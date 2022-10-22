#ifndef NTIOS_H
#define NTIOS_H

#include <stdint.h>

#include "drivers.h"
#include "basics.h"

//#define __CONCAT_MACRO(...) #__VA_ARGS__

#if defined(__IMXRT1062__)
	#define HAS_SET_ARM_CLOCK
	#define MAX_CLOCK_SPEED 780000000
	#define MIN_CLOCK_SPEED 24000000
	#define MAX_BURST_SPEED 804000000 //912000000
	#define MAX_BURST_MILLIS 60
#elif defined(ESP32)
	#define MAX_CLOCK_SPEED 240000000
	#define MIN_CLOCK_SPEED 80000000
	#define MAX_BURST_SPEED 0
	#define MAX_BURST_MILLIS 0
#else
	#define MAX_CLOCK_SPEED 1
	#define MIN_CLOCK_SPEED 1
	#define MAX_BURST_SPEED 1
	#define MAX_BURST_MILLIS 0
#endif

#ifdef __IMXRT1062__
	#define CPU_ARCH_NAME "ARM v7 (imxrt1062 crossover processor)"

	#define CPU_ISA ARM32
	#define CPU_ARCH ARMv7
#elif defined(__ARM_ARCH_3__)
	#define CPU_ARCH_NAME "ARM v3"

	#define CPU_ISA ARM32
	#define CPU_ARCH ARMv3
#elif defined(__ARM_ARCH_4__)
	#define CPU_ARCH_NAME "ARM v4"

	#define CPU_ISA ARM32
	#define CPU_ARCH ARMv4
#elif defined(__ARM_ARCH_7__)
	#define CPU_ARCH_NAME "ARM v7"

	#define CPU_ISA ARM32
	#define CPU_ARCH ARMv7
#elif defined(ESP32)
	#define CPU_ARCH_NAME "ESP32"

	#define CPU_ISA XTENSA
	#define CPU_ARCH XTENSA
#elif defined(__xtensa__)
	#define CPU_ARCH_NAME "Xtensa (Unknown variant)"

	#define CPU_ISA XTENSA
	#define CPU_ARCH XTENSA
#else
	#define CPU_ARCH_NAME "Unknown"

	#define CPU_ISA UNKNOWN
	#define CPU_ARCH UNKNOWN
#endif

#define LI_TO_UINT32(x) (x)[0] | ((uint32_t)(x)[1] << 8) |((uint32_t)(x)[2] << 16) |((uint32_t)(x)[3] << 24)
#define LI_TO_UINT16(x) (x)[0] | ((uint16_t)(x)[1] << 8)

#define DEBUG_PRINTF if(_ntios_debug_port) _ntios_debug_port->printf


extern StreamDevice* _ntios_debug_port;

// initialize driver list so OS can run - will set up 
void ntios_init(Device** devices, int num_devices, StreamDevice* debug = 0);

int add_device(Device* device);
void rm_device(int dev);
Device* get_device(int dev);
int num_devices();

uint64_t set_cpu_hz(uint64_t frequency);
uint64_t get_cpu_hz();
uint64_t check_cpu_clock();
uint64_t set_min_cpu_hz(uint64_t frequency);
double get_cpu_usage_percent();

uint64_t get_free_ram_bytes();
char* get_stack_top();
char* get_heap_top();

TCPConnection* tcpopen(const char* host, int port);

int mount(int dev, const char* dir);
int unmount(int dev);

// OS statistics and diagnostic info
uint32_t get_heartbeat();
uint32_t get_last_updating_device();
uint64_t get_last_update_time();

// Returns zero if date and time are not known
uint64_t get_unix_millis();

// free file objects after closing them (or have memory leaks :wink:)
NTIOSFile* fsopen(const char* dir, int mode = NTIOS_READ);
int fsmkdir(const char* dir);
int fsrmdir(const char* dir);
int fsremove(const char* dir);
bool fsexists(const char* dir);
int fscopy(const char* src, const char* dst);

// Only one thread may call this
void ntios_yield();

class BootloaderMutex {
public:
	virtual void lock() = 0;
	virtual void unlock() = 0;
};

// this should be defined by the code loading/invoking NTIOS for multithreading purposes.
int get_bootloader_thread_slice_us();
BootloaderMutex* bootloader_make_mutex();

// this should be defined by the code loading/invoking NTIOS.  Return true if the function was
// successfully started in a new thread.
bool start_function(void (*f)(void* param), void* param, int stack_size);

// this should be defined by the code loading/invoking NTIOS.  Return true if the command was
// successfully started in a new thread.
bool launch(int argc, char** argv, StreamDevice* io);

// this should be defined by the code loading/invoking NTIOS so device-specific extra programs can be added in firmware
// If you don't want to use this feature then add a builtin_system function that always returns -100
// -100 should be returned if the command is not recognized
int builtin_system(int argc, char** argv, StreamDevice* io);

int ntios_shell(StreamDevice* io, char* login_sha256_hash = nullptr);
int ntios_system(const char* command, StreamDevice* io);
int ntios_system(int argc, char** argv, StreamDevice* io);

char backslash_delimit(char c);


#ifdef __cplusplus
extern "C" {
#endif

/*
 * These functions originally came from Arduino.
 */

uint32_t millis();
uint32_t micros();
void delay(uint32_t milliseconds);
void delayMicroseconds(uint32_t microseconds);

/*
 * Other functions with C linkage
 */

void yield();
void trigger_software_irq();
void __enable_irq();
void __disable_irq();

#ifdef __cplusplus
};
#endif


#endif
