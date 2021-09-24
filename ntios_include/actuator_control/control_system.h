
#ifndef CONTROL_SYSTEM_H
#define CONTROL_SYSTEM_H

#include "pid.h"
#include "../actuation.h"
#include "../ntios_math.h"

class GenericSymmetricController: public Device {
private:

	ActuatorDevice* actuators[10];
	double actuator_mixing[10][13];
	int num_actuators;

	int pos_setpoint_i;
	int pos_response_i;
	PIDControlLimit position_x_loop;
	PIDControlLimit position_y_loop;
	PIDControlLimit position_z_loop;

	int vel_response_i;
	int vel_setpoint_i;
	PIDControl velocity_x_loop;
	PIDControl velocity_y_loop;
	PIDControl velocity_z_loop;

	int att_setpoint_i;
	int att_response_i;
	PIDControlLimit attitude_roll_loop;
	PIDControlLimit attitude_pitch_loop;
	PIDControlLimit attitude_yaw_loop;

	int rate_setpoint_i;
	int rate_response_i;
	PIDControl attitude_roll_rate_loop;
	PIDControl attitude_pitch_rate_loop;
	PIDControl attitude_yaw_rate_loop;

	unsigned long timer = 0;

public:

	const char* getName() { return "Control"; }
	int getType() { return DEV_TYPE_CONTROL_SYS_SYMMETRIC; }

	bool has_position_setpoint = false;
	Vector3 position_setpoint;

	bool has_velocity_setpoint = false;
	Vector3 velocity_setpoint;

	bool has_attitude_setpoint = false;
	double attitude_setpoint[3];

	bool has_rate_setpoint = false;
	double rate_setpoint[3];

	// each actuator has 13 mixing signals.  Mixing signals are multiplied with the following (in order):
	//  posx posy posz velx vely velz roll pitch yaw rollspeed pitchspeed yawspeed offset
	// the products are summed.  Expected output between 0 and 1.  All inputs are between 0 and 1.
	// If the result is outside the range [0, 1] then it will be clipped.

	GenericSymmetricController(ActuatorDevice** actuator_li, double mixing[][13], int num_actuators);
	void update();
};

int b_make_controller(int argc, char** argv, StreamDevice* io);
int b_ctrl(int argc, char** argv, StreamDevice* io);

#endif
