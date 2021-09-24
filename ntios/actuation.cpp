
#include "actuation.h"

PWMActuator::PWMActuator(PWMPin* dev, const char* name) {
	pwm = dev;
	pwm->setFreq(50);
	pwm->setMicros(900);
	strlcpy(this->name, name, 16);
}

void PWMActuator::set(double val) {
	pwm->setMicros((long)(val * 1000 + 900));
}
