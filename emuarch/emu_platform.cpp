
#include "emu.h"
#include "ntios.h"
#include "drivers.h"
#include "navigation.h"

#define TCP_OFFSET 0x1000

int64_t heap_top = RAM_OFFSET;

int launch_emu(StreamDevice* io, uint8_t* code, int argc, char** argv){
	int result;

	setup_memory(code, MEMORY_SIZE);
	t_emuarch_cpu* cpu = make_cpu(0, (MEMORY_SIZE - 1) | RAM_OFFSET);
	run(cpu, io);
	cleanup_cpu(cpu);
	free(global_ram);

	result = cpu->reg_set_0[0];
	free(cpu);

	return result;
}

NTIOSFile* getEmuFile(t_emuarch_cpu* cpu, int fd) {
	if (fd < 0 || fd >= cpu->num_files)
		return NULL;
	return cpu->files[fd];
}

TCPConnection* getEmuTcp(t_emuarch_cpu* cpu, int fd) {
	fd -= TCP_OFFSET;
	if (fd < 0 || fd >= cpu->num_tcps)
		return NULL;
	return cpu->tcps[fd];
}

void emuarch_syscall(t_emuarch_cpu* cpu, uint16_t id, StreamDevice* io){
	int64_t tmp;
	char* tmp_str;
	char	buffer[16];

	if (id == SYSCALL_PUTCHAR) {
		io->write((char)(cpu->reg_set_0[0]));
	} else if (id == SYSCALL_MALLOC) {
		tmp = cpu->reg_set_0[0];
		cpu->reg_set_0[0] = heap_top;
		heap_top += tmp;
	} else if (id == SYSCALL_FREE) {
		
	} else if (id == SYSCALL_PRINTF) {
		// si is format string
		// arguments used are poped off the stack
		// to prevent stack corruption and/or undefined behavior, 
		//	 a qword is popped for each argument, regardless of argument size.
		tmp_str = adr_to_str(cpu->reg_set_0[4]);
		buffer[0] = '%';
		buffer[2] = 0;
		while (*tmp_str) {
			if (*(tmp_str) == '%') {
				tmp_str++;
				buffer[1] = *tmp_str;
				io->printf(buffer, (int)pop_qword(cpu));
				tmp_str++;
			}else
				io->write(*(tmp_str++));
		}

	} else if (id == SYSCALL_PUTSTR) {
		io->print(adr_to_str(cpu->reg_set_0[4]));
	} else if (id == SYSCALL_MILLIS) {
		cpu->reg_set_0[0] = millis();
	} else if (id == SYSCALL_MICROS) {
		cpu->reg_set_0[0] = micros();
	} else if (id == SYSCALL_OPEN) {
		int mode;
		if (cpu->reg_set_0[0] & EMUARCH_FILE_MODE_WRITE)
			mode = NTIOS_WRITE;
		else
			mode = NTIOS_READ;

		NTIOSFile* file = fsopen(adr_to_str(cpu->reg_set_0[4]), mode);
		if (file == NULL)
			cpu->reg_set_0[3] = -1;
		else {
			cpu->reg_set_0[3] = cpu->num_files;
			cpu->files = (NTIOSFile**)realloc(cpu->files, (cpu->num_files + 1) * sizeof(NTIOSFile*));
			cpu->files[cpu->num_files] = file;
			cpu->num_files++;
		}
	} else if (id == SYSCALL_READ) {
		NTIOSFile* file;
		TCPConnection* tcp;
		char* dst = adr_to_str(cpu->reg_set_0[5]);
		
		if (!dst)
			cpu->reg_set_0[0] = 0;
		else {
			if (file = getEmuFile(cpu, cpu->reg_set_0[3])) {
				int i = 0;
				while (file->available() && i < cpu->reg_set_1[6]) {
					dst[i++] = file->read();
				}
				cpu->reg_set_0[0] = i;
			} else if (tcp = getEmuTcp(cpu, cpu->reg_set_0[3])) {
				int i = 0;
				while (tcp->available() && i < cpu->reg_set_1[6]) {
					dst[i++] = tcp->read();
				}
				cpu->reg_set_0[0] = i;
			}else
				cpu->reg_set_0[0] = 0;
		}
	} else if (id == SYSCALL_WRITE) {
		NTIOSFile* file;
		TCPConnection* tcp;
		char* src = adr_to_str(cpu->reg_set_0[4]);
		
		if (!src)
			cpu->reg_set_0[0] = 0;
		else {
			if (file = getEmuFile(cpu, cpu->reg_set_0[3])) {
				int i = 0;
				while (i < cpu->reg_set_1[6]) {
					file->write(src[i++]);
				}
				cpu->reg_set_0[0] = i;
			} else if (tcp = getEmuTcp(cpu, cpu->reg_set_0[3])) {
				int i = 0;
				while (i < cpu->reg_set_1[6]) {
					tcp->write(src[i++]);
				}
				cpu->reg_set_0[0] = i;
			}else
				cpu->reg_set_0[0] = 0;
		}
	} else if (id == SYSCALL_TELL) {
		NTIOSFile* file;

		if (file = getEmuFile(cpu, cpu->reg_set_0[3]))
			cpu->reg_set_0[0] = file->position();
		else
			cpu->reg_set_0[0] = -1;
	} else if (id == SYSCALL_SEEK) {
		NTIOSFile* file;

		if (file = getEmuFile(cpu, cpu->reg_set_0[3]))
			file->seek(cpu->reg_set_0[0]);
	} else if (id == SYSCALL_CLOSE) {
		NTIOSFile* file;
		TCPConnection* tcp;

		if (file = getEmuFile(cpu, cpu->reg_set_0[3])) {
			file->close();
			free(file);
			cpu->files[cpu->reg_set_0[3]] = NULL;
		} else if (tcp = getEmuTcp(cpu, cpu->reg_set_0[3])) {
			tcp->close();
			free(tcp);
			cpu->tcps[cpu->reg_set_0[3] + TCP_OFFSET] = NULL;
		}else
			cpu->reg_set_0[0] = 0;
	} else if (id == SYSCALL_TCP_CONNECT) {
		TCPConnection* tcp = tcpopen(adr_to_str(cpu->reg_set_0[4]), cpu->reg_set_0[0]);
		if (tcp == NULL)
			cpu->reg_set_0[3] = -1;
		else {
			cpu->reg_set_0[3] = cpu->num_tcps + TCP_OFFSET;
			cpu->tcps = (TCPConnection**)realloc(cpu->tcps, (cpu->num_tcps + 1) * sizeof(TCPConnection*));
			cpu->tcps[cpu->num_files] = tcp;
			cpu->num_tcps++;
		}
	} else if (id == SYSCALL_NAV_READ) {
		double certainty = get_position_certainty();
		cpu->reg_set_2[2] = get_latitude();
		cpu->reg_set_2[3] = get_longitude();
		cpu->reg_set_0[0] = certainty == -1 ? -1 : (long) (certainty * 1000);
	}
}

