
#include "drivers/graphics/graphics.h"
#include "drivers/gpio_expanders/MCP23017.h"
#include "drivers.h"
#include "ntios.h"
#include "navigation.h"
#include "actuation.h"

// TODO: not use these globals in this file directly

extern Device** _ntios_virtual_devices;
extern int _ntios_virtual_device_count;

extern Device** _ntios_hw_devices;
extern int _ntios_hw_device_count;

int __ntios_device_cli_utils(int argc, const char** argv, StreamDevice* io) {
	const char* cmd = argv[0];
	int result = -100;
	if (strcmp(cmd, "lsdev") == 0) {
		if (num_devices() == 0) {
			io->println("Warning: No devices listed (this shouldn't be possible)");
		}
		for (int i = 0; i < num_devices(); i++) {
			Device* dev = get_device(i);
			io->printf("%i %p\n", i, dev);
			io->printf("%i\n", strlen(dev->getName()));
			io->printf("%s\n", dev->getName());
			io->printf("%03x\n", (int)dev->getType());

			if (dev != NULL) {
				int type = dev->getType();
				io->printf("%p\n", dev->getName());
				io->printf("%02i type %03x %-20s \"%s\"\n", i, type, devTypeToStr(type), dev->getName());
			} else
				io->printf("[ERROR: DEVICE %i IS NULL]\n", i);
		}
		result = 0;
	} else if (strcmp(cmd, "rmdev") == 0) {
		if (argc > 1) {
			result = 0;
			int i = atoi(argv[1]);
			if (i < num_devices()) {
				result = -2;
				io->printf("Cannot remove device %i\n", i);
			} else if (i >= num_devices()) {
				result = -3;
				io->printf("No device %i\n", i);
			} else
				rm_device(i);
		} else {
			io->println(F("usage: rmdev DEV"));
			result = -1;
		}
	} else if (strcmp(cmd, "serterm") == 0) {
		if (argc < 2) {
			io->println("serterm: usage: serterm DEVICE");
			result = -1;
		} else {
			char* echk;
			int port = (int)strtol(argv[1], &echk, 10);
			Device* raw = get_device(port);
			if (*echk != 0 || argv[1][0] == 0 || raw == NULL) {
				io->printf("Error: bad device id '%s'\n", argv[2]);
				result = -2;
			} else if ((raw->getType() & 0xFF00) != DEV_TYPE_STREAM) {
				io->println("Error: Not a serial port.");
				result = -3;
			} else {
				SerialDevice* dev = (SerialDevice*)raw;
				io->println("Press ` to exit.");
				while (true) {
					if (io->available()) {
						int c = io->read();
						if (c == '`') {
							break;
						} else {
							dev->write(c);
						}
					}
					while (dev->available()) {
						char c = dev->read();
						io->write(c);
					}
					yield();
				}
				result = 0;
			}
		}
	} else if (strcmp(cmd, "serbaud") == 0) {
		if (argc < 3) {
			io->println("Usage: serbaud DEV BAUD");
			result = -1;
		} else {
			char* echk;
			int port = (int)strtol(argv[1], &echk, 10);
			if (*echk != 0 || argv[2][0] == 0 || port < 0 || port >= num_devices()) {
				io->printf("Error: bad device id '%s'\n", argv[2]);
				result = -2;
			} else if (get_device(port)->getType() != DEV_TYPE_SERIAL) {
				io->println("Error: Not a serial port.");
				result = -3;
			} else {
				int baud = (int)strtol(argv[2], &echk, 10);
				if (*echk != 0 || argv[2][0] == 0 || baud < 0) {
					io->printf("Error: bad baud '%s'\n", argv[2]);
					result = -4;
				} else {
					SerialDevice* dev = (SerialDevice*)get_device(port);
					dev->setBaud(baud);
					result = 0;
				}
			}
		}
	} else if (strcmp(cmd, "mkdev") == 0) {
		if (argc < 2) {
			io->println("Usage: mkdev TYPE [args]");
			io->println("Types: nmea pwmactuator mcp23017 ");
			result = -1;
		} else if (strcmp(argv[1], "nmea") == 0) {
			if (argc < 3) {
				io->println("Usage: mkdev nmea PORT_ID [devname]");
				result = -1;
			} else {
				char* echk;
				int port = (int)strtol(argv[2], &echk, 10);
				if (*echk != 0 || argv[2][0] == 0 || port < 0 || port >= num_devices()) {
					io->printf("Error: bad device id '%s'\n", argv[2]);
					result = -2;
				} else if ((get_device(port)->getType() & 0xFF00) != DEV_TYPE_STREAM) {
					io->println("Error: cannot make NMEA GPS device from non-stream device.");
					result = -3;
				} else {
					const char* name = "NMEA GPS";
					if (argc > 3)
						name = argv[3];
					StreamDevice* dev = (StreamDevice*)get_device(port);
					add_device(new NMEARawGPS(dev, name));
					result = 0;
				}
			}
		} else if (strcmp(argv[1], "pwmactuator") == 0) {
			if (argc < 3) {
				io->println("Usage: mkdev pwmactuator PORT_ID [devname]");
				result = -1;
			} else {
				char* echk;
				int port = (int)strtol(argv[2], &echk, 10);
				if (*echk != 0 || argv[2][0] == 0 || port < 0 || port >= num_devices()) {
					io->printf("Error: bad device id '%s'\n", argv[2]);
					result = -2;
				} else if (get_device(port)->getType() != DEV_TYPE_PWM_PIN) {
					io->println("Error: cannot make PWM actuator device from non-PWM device.");
					result = -3;
				} else {
					const char* name = "PWM Actuator";
					if (argc > 3)
						name = argv[3];
					PWMPin* dev = (PWMPin*)get_device(port);
					add_device(new PWMActuator(dev, name));
					result = 0;
				}
			}
		} else if (strcmp(argv[1], "mcp23017") == 0) {
			if (argc < 3) {
				io->println("Usage: mkdev mcp23017 I2C_DEVICE_ID");
				result = -1;
			} else {
				char* echk;
				int port = (int)strtol(argv[2], &echk, 10);
				if (*echk != 0 || argv[2][0] == 0 || port < 0 || port >= num_devices()) {
					io->printf("Error: bad device id '%s'\n", argv[2]);
					result = -2;
				} else if (get_device(port)->getType() != DEV_TYPE_BUS_I2C) {
					io->println("Error: cannot make MCP23017 device from non-I2C device.");
					result = -3;
				} else {
					I2CBusDevice* dev = (I2CBusDevice*)get_device(port);
					add_device(new MCP23017(dev));
					result = 0;
				}
			}
		}
	} else if (strcmp(cmd, "gpio") == 0) {
		if (argc < 2) {
			io->println("Usage: gpio device [pin [on, off, in, out, in_pullup, in_pulldown, high_z]]");
			result = -1;
		} else {
			char* echk;
			int port = (int)strtol(argv[1], &echk, 10);
			if (*echk != 0 || argv[1][0] == 0 || port < 0 || port >= num_devices()) {
				io->printf("Error: bad device id '%s'\n", argv[2]);
				result = -2;
			} else if ((get_device(port)->getType() & 0xFF80) != DEV_TYPE_GPIO) {
				io->printf("Error: device %i is not a GPIO device.\n", port);
				result = -3;
			} else {
				GPIODevice* dev = (GPIODevice*)get_device(port);
				if (argc == 2) {
					io->printf("%i/%s: %i IO Pins\n", port, dev->getName(), dev->pinCount());
					result = 0;
				} else {
					int pin = (int)strtol(argv[2], &echk, 10);
					if (*echk != 0 || argv[2][0] == 0 || port < 0 || port >= num_devices()) {
						io->printf("Error: Does not have a pin %s.", argv[2]);
						io->printf("  Valid pins are 0-%i.\n", dev->pinCount() - 1);
						result = -4;
					} else {
						if (argc == 3) {
							// Read the pin
							io->printf("Device %i (%s) pin %i: %s\n",
										port,
										dev->getName(),
										pin,
										dev->readPin(pin) ? "HIGH" : "LOW");
							result = 0;
						} else {
							// Here we either set the pin mode or set the pin output
							const char* function = argv[3];
							bool did_succeed = true;
							result = 0;
							if (strcmp(function, "on") == 0) {
								did_succeed = dev->writePin(pin, true);
							} else if (strcmp(function, "off") == 0) {
								did_succeed = dev->writePin(pin, false);
							} else if (strcmp(function, "in") == 0) {
								did_succeed = dev->pinMode(pin, GPIO_PIN_MODE_INPUT);
							} else if (strcmp(function, "out") == 0) {
								did_succeed = dev->pinMode(pin, GPIO_PIN_MODE_OUTPUT);
							} else if (strcmp(function, "in_pullup") == 0) {
								did_succeed = dev->pinMode(pin, GPIO_PIN_MODE_INPUT_PULLUP);
							} else if (strcmp(function, "in_pulldown") == 0) {
								did_succeed = dev->pinMode(pin, GPIO_PIN_MODE_INPUT_PULLDOWN);
							} else if (strcmp(function, "high_z") == 0) {
								did_succeed = dev->pinMode(pin, GPIO_PIN_MODE_HIGH_Z);
							} else {
								io->printf("Invalid operation: %s\n", function);
								result = -5;
							}
							
							if (did_succeed == false) {
								// Something went wrong
								io->println("Failure. Unknown cause, from driver.\n");
								result = -6;
							}
						}
					}
				}
			}
		}
	}
	return result;
}
