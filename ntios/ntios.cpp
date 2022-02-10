
#include "strings.h"
#include "ntios.h"
#include "drivers.h"
#include "navigation.h"

#if defined(ESP32)
	#include "esp32-hal-cpu.h"
#endif

#define LOAD_MON_UPDATE_RATE_MS 75

void _ntios_nav_cb(NavigationDevice* dev);

Device** _ntios_devices;
int _ntios_device_count;

StreamDevice* _ntios_debug_port;

int last_updating_device = -1;
uint32_t ntios_heartbeat = 0;
uint64_t ntios_last_update_unix = 0;

double __ntios_cpu_usage = -1;
volatile char* __ntios_ls_stack_top;

uint64_t __ntios_minimum_clock_hz_conf = MIN_CLOCK_SPEED;

uint32_t get_heartbeat() {
	return ntios_heartbeat;
}
uint32_t get_last_updating_device() {
	return last_updating_device;
}

uint64_t get_last_update_time() {
	return ntios_last_update_unix;
}

// initialize driver list so OS can run - will set up 
void ntios_init(Device** devices, int num_devices, StreamDevice* debug) {

	_ntios_devices = devices;
	_ntios_device_count = num_devices;

	_ntios_debug_port = debug;
	DEBUG_PRINTF("NTIOS Initialized.\n");
	
	if(start_function(_ntios_nav_updater_thread, NULL, 8192)) {
		DEBUG_PRINTF("Started navigation thread.\n");
	} else {
		DEBUG_PRINTF("Failed to start navigation thread - navigation will not function.\n");
	}
}

int add_device(Device* device) {
	int i = ++_ntios_device_count;
	_ntios_devices = (Device**)realloc(_ntios_devices, sizeof(Device*) * i);
	_ntios_devices[--i] = device;
	if (device->getType() == DEV_TYPE_GPS_RAW)
		((NavigationDevice*)device)->add_callback(_ntios_nav_cb);
	return i;
}

void rm_device(int dev) {
	if (dev < 0 || dev >= _ntios_device_count)
		return;
	int i = --_ntios_device_count;
	_ntios_devices[dev] = _ntios_devices[i];
	_ntios_devices = (Device**)realloc(_ntios_devices, sizeof(Device*) * i);
}

Device* get_device(int dev) {
	if (dev < 0)
		return NULL;
	if (dev < _ntios_device_count)
		return _ntios_devices[dev];
	return NULL;
}

int num_devices() {
	return _ntios_device_count;
}

double get_cpu_usage_percent() {
	return __ntios_cpu_usage;
}

void update_load_monitor() {
	static long timer = -1;
	static long yield_time_us = 0;
	static long num_yields = 0;

	long time = millis();
	if (timer == -1)
		timer = time + LOAD_MON_UPDATE_RATE_MS;

	if (timer < time) {
		double usage = 100;
		double avg_yield_us = yield_time_us / (double)(num_yields);

		if (avg_yield_us < get_bootloader_thread_slice_us())
			usage = 100 * avg_yield_us / get_bootloader_thread_slice_us();

		if (MAX_CLOCK_SPEED != 0) {
			uint64_t cpu_hz = get_cpu_hz();
			if (usage > 95) {
				// Increase clock speed
				if (cpu_hz < MAX_CLOCK_SPEED)
					cpu_hz = set_cpu_hz(MAX_CLOCK_SPEED);
			} else if (usage < 85) {
				// Decrease clock speed
				uint64_t target_hz = cpu_hz * usage;
				if (cpu_hz != MIN_CLOCK_SPEED)
					cpu_hz = set_cpu_hz(target_hz > MIN_CLOCK_SPEED ? target_hz : MIN_CLOCK_SPEED);
			}

			// Calculate actual usage based on clock speed.
			// If the clock is slow then we are using more CPU time than
			// is really representitive of how fast it *could* be done.
			__ntios_cpu_usage = usage * cpu_hz / MAX_CLOCK_SPEED;
		}

		timer = time + LOAD_MON_UPDATE_RATE_MS;
		yield_time_us = 0;
		num_yields = 0;
	}

	yield_time_us -= micros();
	yield();
	num_yields++;
	yield_time_us += micros();
}

// Only one thread may call this
void ntios_yield() {

	// Get the top of the stack
	// This thread has to do this because all the others have
	// their stacks in the wrong place for memory free calculations.
	char top;
	__ntios_ls_stack_top = &top;

	ntios_last_update_unix = get_unix_millis();
	for (int i = 0; i < _ntios_device_count; i++) {
		last_updating_device = i;
		_ntios_devices[i]->update();
	}
	check_cpu_clock();
	update_load_monitor();
	ntios_heartbeat += 1;
}

TCPConnection* tcpopen(const char* host, int port) {
	TCPConnection* tcp;

	// process virtual first in case we ever add VPNs or anything like that
	for (int i = 0; i < _ntios_device_count; i++) {
		if (_ntios_devices[i]->getType() >> 8 == 4) {
			tcp = ((NetworkDevice*)_ntios_devices[i])->connect(host, port);
			if (tcp != NULL) return tcp;
		}
	}
	return NULL;
}

#ifdef HAS_SET_ARM_CLOCK
	extern "C" uint32_t set_arm_clock(uint32_t frequency);
#endif

#ifndef F_CPU
	#define F_CPU 0
#endif

uint64_t __ntios_internal_clock_speed = F_CPU;
long __ntios_internal_next_clock_check_ms = 0;

uint64_t set_min_cpu_hz(uint64_t frequency) {
	__ntios_minimum_clock_hz_conf = min(frequency, (uint64_t)MAX_CLOCK_SPEED);
	return __ntios_minimum_clock_hz_conf;
}

uint64_t set_cpu_hz(uint64_t frequency) {
	if (frequency < __ntios_minimum_clock_hz_conf)
		frequency = __ntios_minimum_clock_hz_conf;

	if (frequency == get_cpu_hz())
		return frequency;

	#ifdef HAS_SET_ARM_CLOCK
		if (frequency > 4294967295U) frequency = 4294967295U;
		if (frequency > MAX_BURST_SPEED) frequency = MAX_BURST_SPEED;

		// Flush all streams before changing CPU speed, otherwise we may lose bytes
		for (int i = 0; i < _ntios_device_count; i++) {
			if ((_ntios_devices[i]->getType() & 0xFF00) == DEV_TYPE_STREAM)
				((SerialDevice*)_ntios_devices[i])->flush();
		}

		__ntios_internal_clock_speed = set_arm_clock(frequency);
	#elif defined(__xtensa__)
		__ntios_internal_clock_speed = ESP.getCpuFreqMHz();
	#else
		__ntios_internal_clock_speed = F_CPU;
	#endif

	if (frequency > MAX_CLOCK_SPEED)
		__ntios_internal_next_clock_check_ms = get_unix_millis() + MAX_BURST_MILLIS;
	else
		__ntios_internal_next_clock_check_ms = -1;

	return __ntios_internal_clock_speed;
}

uint64_t get_cpu_hz() {
	return __ntios_internal_clock_speed;
}

uint64_t check_cpu_clock() {
	if (__ntios_internal_next_clock_check_ms != -1) {
		if (__ntios_internal_next_clock_check_ms < (long)get_unix_millis()) {
			__ntios_internal_next_clock_check_ms = -1;
			
			// Check CPU frequency for burst mode
			// We can divide before multiplying here because our numbers
			// are huge (10s or 100s of millions) and usually divisible by
			// 10.  Even if they aren't, the answer is close enough.
			if (__ntios_internal_clock_speed > MAX_CLOCK_SPEED)
				return set_cpu_hz((MAX_CLOCK_SPEED / 10) * 9);
		}
	}
	return __ntios_internal_clock_speed;
}

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

char* get_heap_top() {
	#ifdef __arm__
	return reinterpret_cast<char*>(sbrk(0));
	#elif defined(__xtensa__)
	return nullptr;
	#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
	return __brkval;
	#elif defined(__arm__)
	return __brkval ? __brkval : __malloc_heap_start;
	#else
	return nullptr;
	#endif  // __arm__
}

char* get_stack_top() {
	char* result = (char*)__ntios_ls_stack_top;
	return result;
}
 
uint64_t get_free_ram_bytes() {
	#ifdef ESP32
	return xPortGetFreeHeapSize();
	#elif defined(__IMXRT1062__)
	return (char*)0x20280000 - get_heap_top();
	#else
	return get_stack_top() - get_heap_top();
	#endif
}
