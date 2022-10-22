
#include <stdlib.h>

#include "csvlog.h"
#include "ntios.h"
#include "emu.h"
#include "texteditor.h"
#include "navigation.h"
#include "emuas.h"
#include "actuation.h"
#include "actuator_control/control_system.h"
#include "exec.h"
#include "blockdevice.h"

#include "drivers/graphics/graphics.h"
#include "drivers/device_cli_utils.h"
#include "drivers/disks.h"
#include "crypto/sha256.h"

bool check_os_stall() {
	static uint32_t last_millis = 0;
	static uint32_t last_heartbeat = 0;

	if (get_heartbeat() != last_heartbeat) {
		last_millis = millis();
		last_heartbeat = get_heartbeat();
		return false;
	}
	if (last_millis + 1000 < millis())
		return true;
	return false;
}

static void do_login(StreamDevice* io, char* login_sha256_hash) {
	char keyboard_buffer[64];
	uint8_t hash_output[SIZE_OF_SHA_256_HASH];
	uint16_t i;

	if (login_sha256_hash == nullptr)
		return;

	io->println("NTIOS Login");
	while (true) {
		io->print("password > ");

		// Read a password from the keyboard
		// This loop will exit (submit the password) if another
		// character would overflow the buffer.
		i = 0;
		while (i < sizeof(keyboard_buffer)) {
			// Wait for a character
			while (!io->available())
				yield();

			char c = io->read();
			if (c == '\n' || c == '\r') {
				// Break, password is complete
				io->write('\n');
				io->write('\n');
				break;
			} else {
				// Got a character
				keyboard_buffer[i++] = c;
				io->write('*');
			}
		}

		// Check the entered password
		calc_sha_256(hash_output, keyboard_buffer, i);
		if (0 == memcmp(hash_output, login_sha256_hash, SIZE_OF_SHA_256_HASH))
			break;

		// Hash didn't match - invalid login.
		// Wait and alert that the password is wrong.
		delay(1000);
		io->println("Password incorrect.");
	}
}

int ntios_shell(StreamDevice* io, char* login_sha256_hash) {
	char c;
	String input;
	//bool did_os_stall = false;

	// Check if we need a login
	do_login(io, login_sha256_hash);

	while (true) {
		input = "";
		io->print(" > ");
		while (true) {
			/*if (!did_os_stall) {
				did_os_stall = check_os_stall();
				if (did_os_stall) {
					io->println("\n***ERROR: OS STOPPED***");
					io->println(" A stall was detected in the main OS thread.");
					//if (get_last_updating_device() != -1)
						io->printf("Device %i (%s) has likely stalled the system.\n",
									get_last_updating_device(),
									get_device(get_last_updating_device())->getName());
				}
			}*/
			if (io->available()){
				c = io->read();
				if (c == 8) {
					if (input.length() == 1)
						input = "";
					else if (input.length() > 1)
						input = input.substring(0, input.length() - 1);
					else continue;
				} else if (c == '\n' || c == '\r')
					break;
				else
					input = input + c;
				io->write(c);
			} else
				yield();
		}
		io->println();
		if (!strcmp(input.c_str(), "exit"))
			return 0;
		ntios_system(input.c_str(), io);
	}
	return -1;
}

int b_ls(StreamDevice* io, int argc, const char** argv){
	const char* path;
	NTIOSFile* entry;
	NTIOSFile* dir;
	char buffer[12];

	if (argc < 2){
		path = "/";
	} else {
		path = argv[argc - 1];
	}

	dir = fsopen(path);

	if (!dir){
		io->println("ls: directory not found");
		return -1;
	}

	if (!dir->isDirectory()){
		io->println("ls: is a file");
		return -2;
	}

	while((entry = dir->openNextFile())) {
		io->print(entry->name());
		if (entry->isDirectory()) {
			io->print("/");
		} else {
			// files have sizes, directories do not
			io->print("\t");
			io->print(itoa(entry->size(), buffer, 10));
		}
		io->println();
		entry->close();
		delete entry;
	}
	dir->close();
	delete dir;
	return 0;
}

int b_timeit(StreamDevice* io, int argc, char** argv){
	int result;
	long time;

	if (argc < 2){
		io->print(F("timeit: usage: timeit ...\n"));
		return -1;
	}

	time = micros();
	result = ntios_system(argc - 1, &(argv[1]), io);
	time = micros() - time;

	io->printf("\nTime taken: %i us (%.3f ms) result: %i\n", time, (double)(time / 1000.0), result);
	return 0;
}

int b_emu(StreamDevice* io, int argc, const char** argv){
	int result;
	long time;
	long file_load_time;
	uint8_t* buffer;
	NTIOSFile* file;

	if (argc < 2){
		io->print(F("emu: usage: emu file [args]\n"));
		return -1;
	}
	
	file_load_time = micros();
	file = fsopen(argv[1]);
	if (!file){
		io->printf("emu: file not found '%s'\n", argv[1]);
		return -1;
	}

	// Speed up the CPU if we're about to start the emulator.  Start in burst mode.
	set_cpu_hz(MAX_BURST_SPEED);

	buffer = (uint8_t*)malloc(file->size());
	file->readBytes((char*)buffer, file->size());
	file->close();
	delete file;
	file_load_time = micros() - file_load_time;
	
	setup_memory(buffer, MEMORY_SIZE);
	t_emuarch_cpu* cpu = make_cpu(0, (MEMORY_SIZE - 1) | RAM_OFFSET);
	
	time = micros();
	run(cpu, io);
	time = micros() - time;
	
	cleanup_cpu(cpu);
	
	result = cpu->reg_set_0[0];
	free(buffer);
	free(global_ram);
	free(cpu);

	io->printf("\nFile load time: %i us (%f ms)\n", 
										file_load_time, (double)(file_load_time / 1000.0));
	io->printf("Execution time: %i us (%f ms)\n", 
										time, (double)(time / 1000.0));
	io->printf("Executed instructions: %i (%f IPS)\n", 
										cpu->total_operations, cpu->total_operations * 1000000.0 / time);
	io->printf("result: %i\n", result);
	return 0;
}

int b_hexdump(StreamDevice* io, int argc, const char** argv) {
	uint8_t buffer[16];
	NTIOSFile* file;
	const int pagelines = 25;

	if (argc < 2){
		io->print("usage: hexdump file\n");
		io->println("This is very similar to the Unix command: hexdump -C [filename] | more");
		return -1;
	}

	file = fsopen(argv[1]);
	if (!file){
		io->printf("hexdump: file not found '%s'\n", argv[1]);
		return -2;
	}

	if (!file->available()) {
		io->println("File empty.");
		return 0;
	}

	if (file->size() / 16 > pagelines)
		io->println("Enter for next line, q to quit.");

	uint32_t position = 0;
	int line = 0;

	while(file->available()) {

		// Read some data from the file
		int readsize = min(16, file->available());
		file->read(buffer, readsize);

		// Print out the hex data
		io->printf("%08x  ", position);
		for (int i = 0; i < readsize; i++) {
			io->printf("%02x ", buffer[i]);
			if (i == 7)
				io->write(' ');
		}

		// Padding for any misaligned data (ex: file ends with only 5 bytes
		for (int i = 0; i < (16 - readsize) * 3 + 1; i++)
			io->write(' ');
		if (readsize < 8)
			io->write(' ');

		// Print character column
		io->write('|');
		for (int i = 0; i < readsize; i++) {
			char c = buffer[i];
			if (c >= ' ' && c < 127)
				io->write(c);
			else
				io->write('.');
		}
		io->println("|");

		line++;
		position += readsize;

		if (line >= pagelines && file->available()) {
			// Wait for user input
			while (true) {
				while (!io->available())
					delay(16); // 60Hz = no visible delay to human input

				char c = io->read();
				if (c == '\n' || c == '\r')
					break;
				if (c == 'q')
					return 0;
			}
		}
	}
	return 0;
}

int b_disk(StreamDevice* io, int argc, const char** argv) {
	if (argc < 2) {
		// List disks
		BlockDevice* dev;
		for (int i = 0; i < num_devices(); i++) {
			Device* dev_tmp = get_device(i);
			if ((dev_tmp->getType() & 0xFF80) == DEV_TYPE_BLOCK_DEVICE) {
				dev = (BlockDevice*)dev_tmp;
				// Print information about the device
				io->printf("\nDevice %i - %s:\n", i, dev->getName());
				if (dev->begin()) {
					io->printf("  Size: %lu sectors\n", dev->sectorCount());
					PartitionedDisk partitionedDisk;
					int partitionBeginCode = partitionedDisk.begin(dev);
					if (partitionBeginCode == 0) {
						io->printf("  Partitions:\n  id       start         end      length  size(GiB)  type\n");
						for (uint32_t j = 0; j < partitionedDisk.numPartitions(); j++) {
							Partition &partition = partitionedDisk.getPartition(j);
							io->printf("  % 2i % 11llu % 11llu % 11llu % 9f  ", j,
									   partition.getStart(),
									   partition.getEnd(),
									   partition.getLength(),
									   partition.getSizeGiB()
							);
							io->println(partition.getTypeAsStr());
						}
					} else {
						io->printf("  This device opened successfully, but couldn't read partitions.  Code %i\n", partitionBeginCode);
					}
				} else {
					io->printf("  Could not open device.\n");
				}
			}
		}
		io->write('\n');
		return 0;
	}
	printf("Unknown usage.\n");
	return 1;
}

char backslash_delimit(char c){
	switch(c){
		case 'n':
			return '\n';
		case 'r':
			return '\r';
		case 't':
			return '\t';
		//case '"':
		//case '\n':
		//case '\\':
		default:
			return c;
	}
}

int ntios_system(const char* command, StreamDevice* io) {
	int cmd_len = strlen(command) + 1;
	char** argv = (char**)malloc(sizeof(char*));
	int argc = 0;
	char* buf = (char*)malloc(cmd_len * sizeof(char));
	int bufloc = 0;
	bool inquote = false;
	int quote_loc = 0;
	int result;
	
	for (int i = 0; command[i]; i++){
		switch(command[i]){
			case '\\':
				buf[bufloc++] = backslash_delimit(command[++i]);
				break;
			case '"':
				inquote = !inquote;
				quote_loc = i;
				break;
			case ' ':
				if (!inquote){
					buf[bufloc++] = 0;
					argv = (char**)realloc(argv, (++argc) * sizeof(char*));
					argv[argc - 1] = (char*)realloc(buf, bufloc * sizeof(char));
					if(cmd_len>i+1)buf = (char*)malloc((cmd_len - i) * sizeof(char));
					bufloc = 0;
					break;
				}
			default:
				buf[bufloc++] = command[i];
		}
	}
	if (bufloc > 0){
		buf[bufloc++] = 0;
		argv =(char**)realloc(argv, (++argc) * sizeof(char*));
		argv[argc - 1] = (char*)realloc(buf, bufloc * sizeof(char));
	}

	if(inquote){
		io->print(F("Missing endquote:\n "));
		io->println(command);
		for(int i = 0; i <= quote_loc; i++){
			io->write(' ');
		}
		io->println('^');
		return -8192;
	}

	if (argc == 0){
		argc = 1;
		argv[0] = (char*)malloc(1);
		argv[0][0] = 0;
	}
	
	result = ntios_system(argc, argv, io);

	for (int i = 0; i < argc; i++)
		free(argv[i]);
	free(argv);

	return result;
}

// This is not const because hypothetically some bugged user program could modify the program arguments
// The functions before are in NTIOS and we know the arguments will not be modified.
int ntios_system(int argc, char** argv, StreamDevice* io) {
	char* cmd = argv[0];
	int result = -100;
	uint8_t* buffer;
	NTIOSFile* file;

	if (cmd[0] == 0)
		result = 0;
	else if (strcmp(cmd, "ls") == 0)
		result = b_ls(io, argc, (const char**)argv);
	else if (strcmp(cmd, "emu") == 0)
		result = b_emu(io, argc, (const char**)argv);
	else if (strcmp(cmd, "as") == 0)
		result = b_emuas(io, argc, (const char**)argv);
	else if (strcmp(cmd, "timeit") == 0)
		result = b_timeit(io, argc, argv);
	else if (strcmp(cmd, "te") == 0)
		result = b_te(io, argc, argv);
	else if (strcmp(cmd, "hexdump") == 0)
		result = b_hexdump(io, argc, (const char**)argv);
	else if (strcmp(cmd, "disk") == 0)
		result = b_disk(io, argc, (const char**)argv);
	else if (strcmp(cmd, "mkdir") == 0) {
		if (argc == 2) {
			if ((result = fsmkdir(argv[1])))
				io->println("Failure.");
		} else
			io->println(F("mkdir: usage: mkdir [file]"));
	} else if (strcmp(cmd, "rm") == 0) {
		if (argc == 2) {
			if ((result = fsremove(argv[1])))
				io->println("Failure.");
		} else
			io->println(F("rm: usage: rm [file]"));
	} else if (strcmp(cmd, "rmdir") == 0) {
		if (argc == 2) {
			if ((result = fsrmdir(argv[1])))
				io->println(F("Failure."));
		} else
			io->println(F("rmdir: usage: rmdir [directory]"));
	} else if (strcmp(cmd, "cp") == 0) {
		if (argc < 3) {
			io->println(F("cp: usage: cp src dst"));
			result = -1;
		} else {
			if ((result = fscopy(argv[1], argv[2])))
				io->println("Failure.");
		}
	} else if (strcmp(cmd, "cls") == 0) {
		int type = io->getType();
		GraphicsDisplayDevice* dev = NULL;
		if (type == DEV_TYPE_JOINED_STREAM) {
			type = ((JoinedStreamDevice*)io)->getOutput()->getType();
			if (type == DEV_TYPE_GRAPHICS) {
				dev = ((GraphicsDisplayDevice*)(((JoinedStreamDevice*)io)->getOutput()));
			}
		} else if (type == DEV_TYPE_GRAPHICS) {
			dev = (GraphicsDisplayDevice*)io;
		}

		if (dev)
			dev->clearScreen();
		else
			io->println("cls: error: not a graphical interface.");
		
	} else if (strcmp(cmd, "wifi") == 0) {
		if (argc == 1) {
			io->println(F("wifi: usage: wifi command [args...]"));
			io->println(F("      connect network (password entered after)"));
			io->println(F("      disconnect"));
			io->println(F("      scan"));
		} else {
			WiFiDevice* dev = NULL;

			for (int i = num_devices() - 1; i >= 0; i--) {
				Device* dev_tmp = get_device(i);
				if (dev_tmp->getType() == DEV_TYPE_WIFI) {
					dev = (WiFiDevice*)dev_tmp;
					break;
				}
			}

			if (dev == NULL) {
				io->println(F("No WiFi device found."));
			} else {
				cmd = argv[1];
				if (!strcmp(cmd, "scan")) {
					int n = dev->scanNetworks();
					for (int i = 0; i < n; i++) {
						io->println(dev->netName(i));
					}
				} else if (!strcmp(cmd, "connect")) {
					if (argc < 3)
						io->println(F("missing argument"));
					else {
						io->print(F("password, blank for none: "));
						char pwd[64];
						int i = 0;
						while (i < 64) {
							while (!io->available()) yield();
							char c = io->read();
							if (c == '\n' || c == '\r')
								break;
							else if (c == 8) {
								io->write(8);
								i--;
							} else {
								pwd[i++] = c;
								io->write('*');
							}
						}
						io->write('\n');
						pwd[i] = 0;
						result = dev->associate(argv[2], pwd);
						if (result != 0)
							io->printf("Error %i\n", result);
					}
				} else if (!strcmp(cmd, "disconnect")) {
					dev->disassociate();
				} else
					io->printf("%s?\n", cmd);
			}
		}
	} else if (strcmp(cmd, "bg") == 0) {
		if (argc < 2) {
			io->println("Usage: bg [command/args]");
			result = -1;
		} else {
			if (launch(argc - 1, &(argv[1]), io))
				result = 0;
			else {
				io->println("Error: could not launch program.  This system may not support multithreading.");
				result = -100;
			}
		}
	} else if (strcmp(cmd, "lscpu") == 0 || strcmp(cmd, "cpuinfo") == 0) {
		io->printf("CPU Architecture: %s\n", CPU_ARCH_NAME);
		io->printf("\n === Usage Stats ===\n");
		io->printf("CPU Usage (current): %.2f%%\n", get_cpu_usage_percent());
		io->printf("RAM Free (current): %.2f KiB\n", get_free_ram_bytes() / 1024.0);
		io->printf("\n === Clock Info ===\n");
		io->printf("Clock Speed Range: %.2f Mhz - %.2f Mhz\n", 
				MIN_CLOCK_SPEED / 1000000.0,
				MAX_CLOCK_SPEED / 1000000.0);
		io->printf("Burst Clock Speed: %.2f Mhz\n", MAX_BURST_SPEED / 1000000.0);
		io->printf("Current Clock Speed: %.2f Mhz\n", get_cpu_hz() / 1000000.0);
		io->printf("\n === Memory Info ===\n");
		io->printf("Stack Top: %p\n", get_stack_top());
		io->printf("Heap Top: %p\n", get_heap_top());
		
	} else if (strcmp(cmd, "telnet") == 0) {
		if (argc < 3) {
			io->println(F("telnet: usage: telnet host port"));
			result = -1;
		} else {
			int port = atoi(argv[2]);
			TCPConnection* tcp = tcpopen(argv[1], port);
			io->println(F("Press ESC or ` to exit."));
			if (tcp == NULL) {
				io->println(F("telnet: error: connection failure"));
				result = -2;
			} else {
				while (!tcp->isClosed()) {
					if (io->available()) {
						int c = io->read();
						if (c == 27 or c == '`') { // escape key
							tcp->close();
						} else {
							io->write(c);
							tcp->write(c);
						}
					}
					while (tcp->available()) {
						char c = tcp->read();
						io->write(c);
					}
					yield();
				}
			}
		}
	} else if (strcmp(cmd, "cat") == 0) {
		if (argc >= 2) {
			file = fsopen(argv[1]);
			if (file){
				while (file->available() > 0) {
					char c = file->read();
					io->write(c);
				}
				file->close();
				delete file;
			}else
				io->println("Error: file not found");
		}
	} else if (strcmp(cmd, "append") == 0) {
		// append first argument to file that is second argument
		if (argc == 3) {
			NTIOSFile* file;
			if (fsexists(argv[2]))
				file = fsopen(argv[2], NTIOS_APPEND);
			else
				file = fsopen(argv[2], NTIOS_WRITE);
			if (file) {
				//file->seek(file->size()); // don't count on file being at the end!
				file->write(argv[1], strlen(argv[1]));
				file->close();
				delete file;
				result = 0;
			} else {
				io->printf("error opening '%s' for writing or appending.\n", argv[2]);
				result = -8;
			}
		} else {
			io->println("usage: append TEXT FILENAME");
			result = -1;
		}
	} else if (strcmp(cmd, "pwm") == 0) {
		if (argc < 4) {
			io->println("Usage: pwm COMMAND DEV VAL");
			io->println("Commands: freq micros duty");
			result = -1;
		} else {
			char* echk;
			int port = (int)strtol(argv[2], &echk, 10);
			if (*echk != 0 || argv[2][0] == 0 || port < 0 || port >= num_devices()) {
				io->printf("Error: bad device id '%s'\n", argv[2]);
				result = -2;
			} else if (get_device(port)->getType() != DEV_TYPE_PWM_PIN) {
				io->println("Error: not a PWM pin.");
				result = -3;
			} else {
				PWMPin* pwm = (PWMPin*)get_device(port);
				if (!strcmp(argv[1], "freq")) {
					pwm->setFreq(atof(argv[3]));
				} else if (!strcmp(argv[1], "duty")) {
					pwm->setDuty(atof(argv[3]));
				} else if (!strcmp(argv[1], "micros")) {
					pwm->setMicros(atol(argv[3]));
				}
			}
		}
	} else if (strcmp(cmd, "actuate") == 0) {
		if (argc < 3) {
			io->println("Usage: actuate DEV VAL");
			result = -1;
		} else {
			char* echk;
			int port = (int)strtol(argv[1], &echk, 10);
			if (*echk != 0 || argv[1][0] == 0 || port < 0 || port >= num_devices()) {
				io->printf("Error: bad device id '%s'\n", argv[1]);
				result = -2;
			} else if (get_device(port)->getType() != DEV_TYPE_ACTUATOR) {
				io->println("Error: not an actuator.");
				result = -3;
			} else {
				ActuatorDevice* actuator = (ActuatorDevice*)get_device(port);
				actuator->set(atof(argv[2]));
				result = 0;
			}
		}
	} else if (strcmp(cmd, "gps") == 0) {
		double certainty;
		double lat;
		double lon;

		if (argc > 1) {
			char* echk;
			int port = (int)strtol(argv[1], &echk, 10);
			if (*echk != 0 || argv[1][0] == 0 || port < 0 || port >= num_devices()) {
				io->printf("Error: bad device id '%s'\n", argv[1]);
				result = -2;
			} else if ((get_device(port)->getType() & 0xFF00) != DEV_TYPE_NAVIGATION) {
				io->println("Error: not a GPS.");
				result = -3;
			} else {
				NavigationDevice* gps = (NavigationDevice*)get_device(port);
				certainty = gps->position_certainty();
				lat = gps->latitude();
				lon = gps->longitude();
				result = 0;
			}
		} else {
			certainty = get_position_certainty();
			lat = get_latitude();
			lon = get_longitude();
			result = 0;
		}

		if (result >= 0) {
			if (certainty == -1) {
				io->println("No GPS fix :(");
				result = -54112;
			} else {
				io->printf("%.10f %.10f +/- %.3fm\n", lat, lon, certainty);
				result = 0;
			}
		}
	} else if (strcmp(cmd, "mkctrl") == 0) {
		result = b_make_controller(argc, argv, io);
	} else if (strcmp(cmd, "ctrl") == 0) {
		result = b_ctrl(argc, argv, io);
	} else if (strcmp(cmd, "listener") == 0) {
		result = nav_listener_program(io);
	} else if (strcmp(cmd, "cal") == 0) {
		result = nav_cal_program(argc, argv, io);
	} else if (strcmp(cmd, "log") == 0) {
		if (argc < 2) {
			io->println("usage: log on/off");
			result = -1;
		} else if (strcmp(argv[1], "on") == 0) {
			if (!internal_logger.openLog()) {
				io->println("Error opening log file.");
				result = -2;
			}
		} else if (strcmp(argv[1], "off") == 0) {
			const char* name = internal_logger.closeLog();
			if (name == NULL) {
				io->println("Log file not open.");
				result = -3;
			} else
				io->printf("Closed log '%s'\n", name);
		} else {
			io->printf("Bad operation '%s'\n", argv[1]);
			result = -1;
		}
	}
#ifdef TEENSYDUINO // This is never defined, but in the future this code will live again.
	else if (!strcmp(cmd, "vol")) {
		if (argc < 2) {
			io->println("usage: vol [volume]");
			result = -1;
		} else {
			AudioOutputDevice* dev = NULL;

			for (int i = num_devices() - 1; i >= 0; i--) {
				Device* dev_tmp = get_device(i);
				if (dev_tmp->getType() == DEV_TYPE_AUDIO_OUT) {
					dev = (AudioOutputDevice*)dev_tmp;
					break;
				}
			}

			result = dev->setVolume(atof(argv[1]));

			io->printf("Wrote vol=%f\n", atof(argv[1]));
			if (result != 0) {
				io->printf("Device returned code %i (likely a failure)\n", result);
			}
		}
	}
#endif
	else if (!strcmp(cmd, "elf")) {
		if (argc >= 2) {
			NTIOSFile* file = fsopen(argv[1], NTIOS_READ);
			if (file) {
				result = _ntios_do_file_exec(io, file, argc - 2, &(argv[2]));
				file->close();
				delete file;
			} else {
				io->printf("error opening '%s'.\n", argv[1]);
				result = 10254;
			}
		} else {
			io->println("usage: elf FILE");
			result = -1;
		}
	} else if (cmd[0] == '.' && cmd[1] == '/'){
		file = fsopen(cmd + 1);
		if (file){
			int size = file->size();
			buffer = (uint8_t*)malloc(size);
			file->readBytes((char*)buffer, size);
			result = launch_emu(io, buffer, argc, argv);
			free(buffer);
		}else
			io->println("Error: file not found");
	}
	if (result == -100)
		result = __ntios_device_cli_utils(argc, (const char**)argv, io);
	if (result == -100)
		io->printf("Not a command: %s\n", argv[0]);
	return result;
}
