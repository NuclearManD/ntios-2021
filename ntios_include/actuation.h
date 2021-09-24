
#ifndef ACTUATION_H
#define ACTUATION_H

#include "drivers.h"

class ActuatorDevice: public Device {
public:
	int getType() { return DEV_TYPE_ACTUATOR; };

	virtual void set(double val) = 0;
};

class PWMActuator: public ActuatorDevice {
private:
	PWMPin* pwm;
	char name[16];
public:
	const char* getName() { return name; };

	PWMActuator(PWMPin* dev, const char* name);
	
	void set(double val);
};

#endif
