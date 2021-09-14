
#ifndef DIGITAL_H__
#define DIGITAL_H__

#include <stdint.h>
#include "core_pins.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void digitalWrite(uint8_t pin, uint8_t val);
extern char digitalRead(uint8_t pin);
extern void pinMode(uint8_t pin, uint8_t mode);

static inline void digitalWriteFast(uint8_t pin, uint8_t val) __attribute__((always_inline, unused));
static inline void digitalWriteFast(uint8_t pin, uint8_t val)
{
	if (__builtin_constant_p(pin)) {
		if (val) {
			if (pin == 0) {
				CORE_PIN0_PORTSET = CORE_PIN0_BITMASK;
			} else if (pin == 1) {
				CORE_PIN1_PORTSET = CORE_PIN1_BITMASK;
			} else if (pin == 2) {
				CORE_PIN2_PORTSET = CORE_PIN2_BITMASK;
			} else if (pin == 3) {
				CORE_PIN3_PORTSET = CORE_PIN3_BITMASK;
			} else if (pin == 4) {
				CORE_PIN4_PORTSET = CORE_PIN4_BITMASK;
			} else if (pin == 5) {
				CORE_PIN5_PORTSET = CORE_PIN5_BITMASK;
			} else if (pin == 6) {
				CORE_PIN6_PORTSET = CORE_PIN6_BITMASK;
			} else if (pin == 7) {
				CORE_PIN7_PORTSET = CORE_PIN7_BITMASK;
			} else if (pin == 8) {
				CORE_PIN8_PORTSET = CORE_PIN8_BITMASK;
			} else if (pin == 9) {
				CORE_PIN9_PORTSET = CORE_PIN9_BITMASK;
			} else if (pin == 10) {
				CORE_PIN10_PORTSET = CORE_PIN10_BITMASK;
			} else if (pin == 11) {
				CORE_PIN11_PORTSET = CORE_PIN11_BITMASK;
			} else if (pin == 12) {
				CORE_PIN12_PORTSET = CORE_PIN12_BITMASK;
			} else if (pin == 13) {
				CORE_PIN13_PORTSET = CORE_PIN13_BITMASK;
			} else if (pin == 14) {
				CORE_PIN14_PORTSET = CORE_PIN14_BITMASK;
			} else if (pin == 15) {
				CORE_PIN15_PORTSET = CORE_PIN15_BITMASK;
			} else if (pin == 16) {
				CORE_PIN16_PORTSET = CORE_PIN16_BITMASK;
			} else if (pin == 17) {
				CORE_PIN17_PORTSET = CORE_PIN17_BITMASK;
			} else if (pin == 18) {
				CORE_PIN18_PORTSET = CORE_PIN18_BITMASK;
			} else if (pin == 19) {
				CORE_PIN19_PORTSET = CORE_PIN19_BITMASK;
			} else if (pin == 20) {
				CORE_PIN20_PORTSET = CORE_PIN20_BITMASK;
			} else if (pin == 21) {
				CORE_PIN21_PORTSET = CORE_PIN21_BITMASK;
			} else if (pin == 22) {
				CORE_PIN22_PORTSET = CORE_PIN22_BITMASK;
			} else if (pin == 23) {
				CORE_PIN23_PORTSET = CORE_PIN23_BITMASK;
			} else if (pin == 24) {
				CORE_PIN24_PORTSET = CORE_PIN24_BITMASK;
			} else if (pin == 25) {
				CORE_PIN25_PORTSET = CORE_PIN25_BITMASK;
			} else if (pin == 26) {
				CORE_PIN26_PORTSET = CORE_PIN26_BITMASK;
			} else if (pin == 27) {
				CORE_PIN27_PORTSET = CORE_PIN27_BITMASK;
			} else if (pin == 28) {
				CORE_PIN28_PORTSET = CORE_PIN28_BITMASK;
			} else if (pin == 29) {
				CORE_PIN29_PORTSET = CORE_PIN29_BITMASK;
			} else if (pin == 30) {
				CORE_PIN30_PORTSET = CORE_PIN30_BITMASK;
			} else if (pin == 31) {
				CORE_PIN31_PORTSET = CORE_PIN31_BITMASK;
			} else if (pin == 32) {
				CORE_PIN32_PORTSET = CORE_PIN32_BITMASK;
			} else if (pin == 33) {
				CORE_PIN33_PORTSET = CORE_PIN33_BITMASK;
			} else if (pin == 34) {
				CORE_PIN34_PORTSET = CORE_PIN34_BITMASK;
			} else if (pin == 35) {
				CORE_PIN35_PORTSET = CORE_PIN35_BITMASK;
			} else if (pin == 36) {
				CORE_PIN36_PORTSET = CORE_PIN36_BITMASK;
			} else if (pin == 37) {
				CORE_PIN37_PORTSET = CORE_PIN37_BITMASK;
			} else if (pin == 38) {
				CORE_PIN38_PORTSET = CORE_PIN38_BITMASK;
			} else if (pin == 39) {
				CORE_PIN39_PORTSET = CORE_PIN39_BITMASK;
#if CORE_NUM_DIGITAL >= 55
			} else if (pin == 40) {
				CORE_PIN40_PORTSET = CORE_PIN40_BITMASK;
			} else if (pin == 41) {
				CORE_PIN41_PORTSET = CORE_PIN41_BITMASK;
			} else if (pin == 42) {
				CORE_PIN42_PORTSET = CORE_PIN42_BITMASK;
			} else if (pin == 43) {
				CORE_PIN43_PORTSET = CORE_PIN43_BITMASK;
			} else if (pin == 44) {
				CORE_PIN44_PORTSET = CORE_PIN44_BITMASK;
			} else if (pin == 45) {
				CORE_PIN45_PORTSET = CORE_PIN45_BITMASK;
			} else if (pin == 46) {
				CORE_PIN46_PORTSET = CORE_PIN46_BITMASK;
			} else if (pin == 47) {
				CORE_PIN47_PORTSET = CORE_PIN47_BITMASK;
			} else if (pin == 48) {
				CORE_PIN48_PORTSET = CORE_PIN48_BITMASK;
			} else if (pin == 49) {
				CORE_PIN49_PORTSET = CORE_PIN49_BITMASK;
			} else if (pin == 50) {
				CORE_PIN50_PORTSET = CORE_PIN50_BITMASK;
			} else if (pin == 51) {
				CORE_PIN51_PORTSET = CORE_PIN51_BITMASK;
			} else if (pin == 52) {
				CORE_PIN52_PORTSET = CORE_PIN52_BITMASK;
			} else if (pin == 53) {
				CORE_PIN53_PORTSET = CORE_PIN53_BITMASK;
			} else if (pin == 54) {
				CORE_PIN54_PORTSET = CORE_PIN54_BITMASK;
#endif
			}
		} else {
			if (pin == 0) {
				CORE_PIN0_PORTCLEAR = CORE_PIN0_BITMASK;
			} else if (pin == 1) {
				CORE_PIN1_PORTCLEAR = CORE_PIN1_BITMASK;
			} else if (pin == 2) {
				CORE_PIN2_PORTCLEAR = CORE_PIN2_BITMASK;
			} else if (pin == 3) {
				CORE_PIN3_PORTCLEAR = CORE_PIN3_BITMASK;
			} else if (pin == 4) {
				CORE_PIN4_PORTCLEAR = CORE_PIN4_BITMASK;
			} else if (pin == 5) {
				CORE_PIN5_PORTCLEAR = CORE_PIN5_BITMASK;
			} else if (pin == 6) {
				CORE_PIN6_PORTCLEAR = CORE_PIN6_BITMASK;
			} else if (pin == 7) {
				CORE_PIN7_PORTCLEAR = CORE_PIN7_BITMASK;
			} else if (pin == 8) {
				CORE_PIN8_PORTCLEAR = CORE_PIN8_BITMASK;
			} else if (pin == 9) {
				CORE_PIN9_PORTCLEAR = CORE_PIN9_BITMASK;
			} else if (pin == 10) {
				CORE_PIN10_PORTCLEAR = CORE_PIN10_BITMASK;
			} else if (pin == 11) {
				CORE_PIN11_PORTCLEAR = CORE_PIN11_BITMASK;
			} else if (pin == 12) {
				CORE_PIN12_PORTCLEAR = CORE_PIN12_BITMASK;
			} else if (pin == 13) {
				CORE_PIN13_PORTCLEAR = CORE_PIN13_BITMASK;
			} else if (pin == 14) {
				CORE_PIN14_PORTCLEAR = CORE_PIN14_BITMASK;
			} else if (pin == 15) {
				CORE_PIN15_PORTCLEAR = CORE_PIN15_BITMASK;
			} else if (pin == 16) {
				CORE_PIN16_PORTCLEAR = CORE_PIN16_BITMASK;
			} else if (pin == 17) {
				CORE_PIN17_PORTCLEAR = CORE_PIN17_BITMASK;
			} else if (pin == 18) {
				CORE_PIN18_PORTCLEAR = CORE_PIN18_BITMASK;
			} else if (pin == 19) {
				CORE_PIN19_PORTCLEAR = CORE_PIN19_BITMASK;
			} else if (pin == 20) {
				CORE_PIN20_PORTCLEAR = CORE_PIN20_BITMASK;
			} else if (pin == 21) {
				CORE_PIN21_PORTCLEAR = CORE_PIN21_BITMASK;
			} else if (pin == 22) {
				CORE_PIN22_PORTCLEAR = CORE_PIN22_BITMASK;
			} else if (pin == 23) {
				CORE_PIN23_PORTCLEAR = CORE_PIN23_BITMASK;
			} else if (pin == 24) {
				CORE_PIN24_PORTCLEAR = CORE_PIN24_BITMASK;
			} else if (pin == 25) {
				CORE_PIN25_PORTCLEAR = CORE_PIN25_BITMASK;
			} else if (pin == 26) {
				CORE_PIN26_PORTCLEAR = CORE_PIN26_BITMASK;
			} else if (pin == 27) {
				CORE_PIN27_PORTCLEAR = CORE_PIN27_BITMASK;
			} else if (pin == 28) {
				CORE_PIN28_PORTCLEAR = CORE_PIN28_BITMASK;
			} else if (pin == 29) {
				CORE_PIN29_PORTCLEAR = CORE_PIN29_BITMASK;
			} else if (pin == 30) {
				CORE_PIN30_PORTCLEAR = CORE_PIN30_BITMASK;
			} else if (pin == 31) {
				CORE_PIN31_PORTCLEAR = CORE_PIN31_BITMASK;
			} else if (pin == 32) {
				CORE_PIN32_PORTCLEAR = CORE_PIN32_BITMASK;
			} else if (pin == 33) {
				CORE_PIN33_PORTCLEAR = CORE_PIN33_BITMASK;
			} else if (pin == 34) {
				CORE_PIN34_PORTCLEAR = CORE_PIN34_BITMASK;
			} else if (pin == 35) {
				CORE_PIN35_PORTCLEAR = CORE_PIN35_BITMASK;
			} else if (pin == 36) {
				CORE_PIN36_PORTCLEAR = CORE_PIN36_BITMASK;
			} else if (pin == 37) {
				CORE_PIN37_PORTCLEAR = CORE_PIN37_BITMASK;
			} else if (pin == 38) {
				CORE_PIN38_PORTCLEAR = CORE_PIN38_BITMASK;
			} else if (pin == 39) {
				CORE_PIN39_PORTCLEAR = CORE_PIN39_BITMASK;
#if CORE_NUM_DIGITAL >= 55
			} else if (pin == 40) {
				CORE_PIN40_PORTCLEAR = CORE_PIN40_BITMASK;
			} else if (pin == 41) {
				CORE_PIN41_PORTCLEAR = CORE_PIN41_BITMASK;
			} else if (pin == 42) {
				CORE_PIN42_PORTCLEAR = CORE_PIN42_BITMASK;
			} else if (pin == 43) {
				CORE_PIN43_PORTCLEAR = CORE_PIN43_BITMASK;
			} else if (pin == 44) {
				CORE_PIN44_PORTCLEAR = CORE_PIN44_BITMASK;
			} else if (pin == 45) {
				CORE_PIN45_PORTCLEAR = CORE_PIN45_BITMASK;
			} else if (pin == 46) {
				CORE_PIN46_PORTCLEAR = CORE_PIN46_BITMASK;
			} else if (pin == 47) {
				CORE_PIN47_PORTCLEAR = CORE_PIN47_BITMASK;
			} else if (pin == 48) {
				CORE_PIN48_PORTCLEAR = CORE_PIN48_BITMASK;
			} else if (pin == 49) {
				CORE_PIN49_PORTCLEAR = CORE_PIN49_BITMASK;
			} else if (pin == 50) {
				CORE_PIN50_PORTCLEAR = CORE_PIN50_BITMASK;
			} else if (pin == 51) {
				CORE_PIN51_PORTCLEAR = CORE_PIN51_BITMASK;
			} else if (pin == 52) {
				CORE_PIN52_PORTCLEAR = CORE_PIN52_BITMASK;
			} else if (pin == 53) {
				CORE_PIN53_PORTCLEAR = CORE_PIN53_BITMASK;
			} else if (pin == 54) {
				CORE_PIN54_PORTCLEAR = CORE_PIN54_BITMASK;
#endif
			}
		}
	} else {
		if(val) *portSetRegister(pin) = digitalPinToBitMask(pin);
		else *portClearRegister(pin) = digitalPinToBitMask(pin);
	}
}

static inline uint8_t digitalReadFast(uint8_t pin) __attribute__((always_inline, unused));
static inline uint8_t digitalReadFast(uint8_t pin)
{
	if (__builtin_constant_p(pin)) {
		if (pin == 0) {
			return (CORE_PIN0_PINREG & CORE_PIN0_BITMASK) ? 1 : 0;
		} else if (pin == 1) {
			return (CORE_PIN1_PINREG & CORE_PIN1_BITMASK) ? 1 : 0;
		} else if (pin == 2) {
			return (CORE_PIN2_PINREG & CORE_PIN2_BITMASK) ? 1 : 0;
		} else if (pin == 3) {
			return (CORE_PIN3_PINREG & CORE_PIN3_BITMASK) ? 1 : 0;
		} else if (pin == 4) {
			return (CORE_PIN4_PINREG & CORE_PIN4_BITMASK) ? 1 : 0;
		} else if (pin == 5) {
			return (CORE_PIN5_PINREG & CORE_PIN5_BITMASK) ? 1 : 0;
		} else if (pin == 6) {
			return (CORE_PIN6_PINREG & CORE_PIN6_BITMASK) ? 1 : 0;
		} else if (pin == 7) {
			return (CORE_PIN7_PINREG & CORE_PIN7_BITMASK) ? 1 : 0;
		} else if (pin == 8) {
			return (CORE_PIN8_PINREG & CORE_PIN8_BITMASK) ? 1 : 0;
		} else if (pin == 9) {
			return (CORE_PIN9_PINREG & CORE_PIN9_BITMASK) ? 1 : 0;
		} else if (pin == 10) {
			return (CORE_PIN10_PINREG & CORE_PIN10_BITMASK) ? 1 : 0;
		} else if (pin == 11) {
			return (CORE_PIN11_PINREG & CORE_PIN11_BITMASK) ? 1 : 0;
		} else if (pin == 12) {
			return (CORE_PIN12_PINREG & CORE_PIN12_BITMASK) ? 1 : 0;
		} else if (pin == 13) {
			return (CORE_PIN13_PINREG & CORE_PIN13_BITMASK) ? 1 : 0;
		} else if (pin == 14) {
			return (CORE_PIN14_PINREG & CORE_PIN14_BITMASK) ? 1 : 0;
		} else if (pin == 15) {
			return (CORE_PIN15_PINREG & CORE_PIN15_BITMASK) ? 1 : 0;
		} else if (pin == 16) {
			return (CORE_PIN16_PINREG & CORE_PIN16_BITMASK) ? 1 : 0;
		} else if (pin == 17) {
			return (CORE_PIN17_PINREG & CORE_PIN17_BITMASK) ? 1 : 0;
		} else if (pin == 18) {
			return (CORE_PIN18_PINREG & CORE_PIN18_BITMASK) ? 1 : 0;
		} else if (pin == 19) {
			return (CORE_PIN19_PINREG & CORE_PIN19_BITMASK) ? 1 : 0;
		} else if (pin == 20) {
			return (CORE_PIN20_PINREG & CORE_PIN20_BITMASK) ? 1 : 0;
		} else if (pin == 21) {
			return (CORE_PIN21_PINREG & CORE_PIN21_BITMASK) ? 1 : 0;
		} else if (pin == 22) {
			return (CORE_PIN22_PINREG & CORE_PIN22_BITMASK) ? 1 : 0;
		} else if (pin == 23) {
			return (CORE_PIN23_PINREG & CORE_PIN23_BITMASK) ? 1 : 0;
		} else if (pin == 24) {
			return (CORE_PIN24_PINREG & CORE_PIN24_BITMASK) ? 1 : 0;
		} else if (pin == 25) {
			return (CORE_PIN25_PINREG & CORE_PIN25_BITMASK) ? 1 : 0;
		} else if (pin == 26) {
			return (CORE_PIN26_PINREG & CORE_PIN26_BITMASK) ? 1 : 0;
		} else if (pin == 27) {
			return (CORE_PIN27_PINREG & CORE_PIN27_BITMASK) ? 1 : 0;
		} else if (pin == 28) {
			return (CORE_PIN28_PINREG & CORE_PIN28_BITMASK) ? 1 : 0;
		} else if (pin == 29) {
			return (CORE_PIN29_PINREG & CORE_PIN29_BITMASK) ? 1 : 0;
		} else if (pin == 30) {
			return (CORE_PIN30_PINREG & CORE_PIN30_BITMASK) ? 1 : 0;
		} else if (pin == 31) {
			return (CORE_PIN31_PINREG & CORE_PIN31_BITMASK) ? 1 : 0;
		} else if (pin == 32) {
			return (CORE_PIN32_PINREG & CORE_PIN32_BITMASK) ? 1 : 0;
		} else if (pin == 33) {
			return (CORE_PIN33_PINREG & CORE_PIN33_BITMASK) ? 1 : 0;
		} else if (pin == 34) {
			return (CORE_PIN34_PINREG & CORE_PIN34_BITMASK) ? 1 : 0;
		} else if (pin == 35) {
			return (CORE_PIN35_PINREG & CORE_PIN35_BITMASK) ? 1 : 0;
		} else if (pin == 36) {
			return (CORE_PIN36_PINREG & CORE_PIN36_BITMASK) ? 1 : 0;
		} else if (pin == 37) {
			return (CORE_PIN37_PINREG & CORE_PIN37_BITMASK) ? 1 : 0;
		} else if (pin == 38) {
			return (CORE_PIN38_PINREG & CORE_PIN38_BITMASK) ? 1 : 0;
		} else if (pin == 39) {
			return (CORE_PIN39_PINREG & CORE_PIN39_BITMASK) ? 1 : 0;
#if CORE_NUM_DIGITAL >= 55
		} else if (pin == 40) {
			return (CORE_PIN40_PINREG & CORE_PIN40_BITMASK) ? 1 : 0;
		} else if (pin == 41) {
			return (CORE_PIN41_PINREG & CORE_PIN41_BITMASK) ? 1 : 0;
		} else if (pin == 42) {
			return (CORE_PIN42_PINREG & CORE_PIN42_BITMASK) ? 1 : 0;
		} else if (pin == 43) {
			return (CORE_PIN43_PINREG & CORE_PIN43_BITMASK) ? 1 : 0;
		} else if (pin == 44) {
			return (CORE_PIN44_PINREG & CORE_PIN44_BITMASK) ? 1 : 0;
		} else if (pin == 45) {
			return (CORE_PIN45_PINREG & CORE_PIN45_BITMASK) ? 1 : 0;
		} else if (pin == 46) {
			return (CORE_PIN46_PINREG & CORE_PIN46_BITMASK) ? 1 : 0;
		} else if (pin == 47) {
			return (CORE_PIN47_PINREG & CORE_PIN47_BITMASK) ? 1 : 0;
		} else if (pin == 48) {
			return (CORE_PIN48_PINREG & CORE_PIN48_BITMASK) ? 1 : 0;
		} else if (pin == 49) {
			return (CORE_PIN49_PINREG & CORE_PIN49_BITMASK) ? 1 : 0;
		} else if (pin == 50) {
			return (CORE_PIN50_PINREG & CORE_PIN50_BITMASK) ? 1 : 0;
		} else if (pin == 51) {
			return (CORE_PIN51_PINREG & CORE_PIN51_BITMASK) ? 1 : 0;
		} else if (pin == 52) {
			return (CORE_PIN52_PINREG & CORE_PIN52_BITMASK) ? 1 : 0;
		} else if (pin == 53) {
			return (CORE_PIN53_PINREG & CORE_PIN53_BITMASK) ? 1 : 0;
		} else if (pin == 54) {
			return (CORE_PIN54_PINREG & CORE_PIN54_BITMASK) ? 1 : 0;
#endif
		} else {
			return 0;
		}
	} else {
		return (*portInputRegister(pin) & digitalPinToBitMask(pin)) ? 1 : 0;
	}
}


#endif

#ifdef __cplusplus
}
#endif
