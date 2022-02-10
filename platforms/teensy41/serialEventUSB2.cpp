
#include <Arduino.h>
#include "avr/pgmspace.h"

void serialEventUSB2() __attribute__((weak));
void serialEventUSB2() {}
uint8_t _serialEventUSB2_default PROGMEM = 1;	
