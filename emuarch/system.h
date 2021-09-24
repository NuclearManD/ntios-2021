
#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>
#include "drivers.h"

#define SYSCALL_MALLOC	0x0000
#define SYSCALL_FREE	0x0001
#define SYSCALL_PUTCHAR	0x0002
#define SYSCALL_PRINTF	0x0003
#define SYSCALL_PUTSTR	0x0004
#define SYSCALL_MILLIS	0x0005
#define SYSCALL_MICROS	0x0006
#define SYSCALL_OPEN	0x0007
#define SYSCALL_READ	0x0008
#define SYSCALL_WRITE	0x0009
#define SYSCALL_TELL	0x000A
#define SYSCALL_SEEK	0x000B
#define SYSCALL_CLOSE	0x000C
#define SYSCALL_TCP_CONNECT	0x000D
#define SYSCALL_TCP_LISTEN	0x000E
#define SYSCALL_TCP_ACCEPT	0x000F

#define SYSCALL_GPIO_PINCOUNT	0x0400
#define SYSCALL_GPIO_PINMODE	0x0401
#define SYSCALL_GPIO_WRITE		0x0402
#define SYSCALL_GPIO_READ		0x0403

/* NAV_READ - get GPS coordinates as four integers
 * 
 * nav_read() -> f2 = latitude, f3 = longitude, rax = accuracy (in mm)
 */
#define SYSCALL_NAV_READ		0x8600

#define EMUARCH_FILE_MODE_WRITE 1

void emuarch_syscall(t_emuarch_cpu* cpu, uint16_t call_num, StreamDevice* io);

#endif
