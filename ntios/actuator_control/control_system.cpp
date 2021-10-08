
#include "actuator_control/control_system.h"
#include "actuator_control/pid.h"
#include "actuation.h"
#include "navigation.h"
#include "ntios.h"
#include "../csvlog.h"
#include <math.h>
#include <stdio.h>
#include <Arduino.h>

// each actuator has 12 mixing signals.  Mixing signals are multiplied with the following (in order):
//  posx posy posz velx vely velz roll pitch yaw rollspeed pitchspeed yawspeed
// the products are summed.  Expected output between 0 and 1.  All inputs are between 0 and 1.
// If the result is outside the range [0, 1] then it will be clipped.


GenericSymmetricController::GenericSymmetricController(ActuatorDevice** actuator_li, double mixing[][13], int num_actuators) : 
num_actuators(num_actuators),

position_x_loop(1, 0, 0.01), // up to 1 m/s
position_y_loop(1, 0, 0.01),
position_z_loop(1, 0, 0.01),

velocity_x_loop(0.1, 0/*.01*/, 0.001),
velocity_y_loop(0.1, 0/*.01*/, 0.001),
velocity_z_loop(0.1, 0/*.01*/, 0.001),

attitude_roll_loop(5, 0/*.01*/, 0),
attitude_pitch_loop(5, 0/*.01*/, 0),
attitude_yaw_loop(3.14, 0/*.01*/, 0),

attitude_roll_rate_loop(0.15, 0/*.01*/, 0.003),
attitude_pitch_rate_loop(0.15, 0/*.01*/, 0.003),
attitude_yaw_rate_loop(0.15, 0/*.01*/, 0.003)

{
	for (int i = 0; i < num_actuators; i++) {
		actuators[i] = actuator_li[i];
		for (int j = 0; j < 13; j++) {
			actuator_mixing[i][j] = mixing[i][j];
			//Serial.printf("%i:%i = %f\n", i, j, mixing[i][j]);
		}
	}
	pos_setpoint_i = internal_logger.markVector("position_setpoint");
	pos_response_i = internal_logger.markVector("position_response");
	vel_setpoint_i = internal_logger.markVector("velocity_setpoint");
	vel_response_i = internal_logger.markVector("velocity_response");
	att_setpoint_i = internal_logger.markVector("attitude_setpoint");
	att_response_i = internal_logger.markVector("attitude_response");
	rate_setpoint_i = internal_logger.markVector("attitude_rate_setpoint");
	rate_response_i = internal_logger.markVector("attitude_rate_response");
}


void GenericSymmetricController::update() {

	if (timer > millis()) return;

	timer = millis() + 9;

	Vector3 pos_response;

	Vector3 vel_response;
	bool vel_response_valid = false;

	double att_response[3];
	bool att_response_valid = false;

	double rate_response[3];
	bool rate_response_valid = false;

	if (has_position_setpoint) {
		Vector3 position = get_position_m();

		pos_response.x = position_x_loop.run(position.x, position_setpoint.x);
		pos_response.y = position_y_loop.run(position.y, position_setpoint.y);
		pos_response.z = position_z_loop.run(position.z, position_setpoint.z);

		// normalize vector so that a large position difference can only create a response of 1 or -1
		double magnitude = pos_response.magnitude();
		if (magnitude > 1)
			pos_response = pos_response / magnitude;

		internal_logger.publishVector(pos_setpoint_i, position_setpoint);
		internal_logger.publishVector(pos_response_i, pos_response);
	}

	if (has_velocity_setpoint) {
		// if we have a velocity setpoint then generate response based on the setpoint

		Vector3 velocity = get_velocity_ms();

		vel_response.x = velocity_x_loop.run(velocity.x, velocity_setpoint.x);
		vel_response.y = velocity_y_loop.run(velocity.y, velocity_setpoint.y);
		vel_response.z = velocity_z_loop.run(velocity.z, velocity_setpoint.z);

		vel_response_valid = true;
	} else if (has_position_setpoint) {
		// if there is no velocity setpoint but we have a position response then use that instead

		Vector3 velocity = get_velocity_ms();

		vel_response.x = velocity_x_loop.run(velocity.x, pos_response.x);
		vel_response.y = velocity_y_loop.run(velocity.y, pos_response.y);
		vel_response.z = velocity_z_loop.run(velocity.z, pos_response.z);

		vel_response_valid = true;
	}
	
	if (vel_response_valid) {
		internal_logger.publishVector(vel_setpoint_i, velocity_setpoint);
		internal_logger.publishVector(vel_response_i, vel_response);
	}

	// convert to vehicle relative frame
	Quaternion attitude = get_attitude_quat();
	attitude.x *= -1;
	attitude.y *= -1;
	attitude.z *= -1;

	pos_response = attitude.rotateVector(pos_response);
	vel_response = attitude.rotateVector(vel_response);

	if (has_attitude_setpoint) {
		double roll = get_roll_rad();
		double pitch = get_pitch_rad();
		double yaw = get_yaw_rad();

		att_response[0] = attitude_roll_loop.run(roll, attitude_setpoint[0]);
		att_response[1] = attitude_pitch_loop.run(pitch, attitude_setpoint[1]);
		att_response[2] = attitude_yaw_loop.run(yaw, attitude_setpoint[2]);

		att_response_valid = true;
	} else if (vel_response_valid) {
		double roll = get_roll_rad();
		double pitch = get_pitch_rad();
		double yaw = get_yaw_rad();

		att_response[0] = attitude_roll_loop.run(roll, vel_response.y);
		att_response[1] = attitude_pitch_loop.run(pitch, vel_response.x);
		att_response[2] = attitude_yaw_loop.run(yaw, yaw);

		att_response_valid = true;
	}

	if (att_response_valid) {
		Vector3 att_info(att_response[0], att_response[1], att_response[2]);
		internal_logger.publishVector(att_response_i, att_info);
	}

	if (has_rate_setpoint) {
		double rollrate = get_roll_rad();
		double pitchrate = get_pitch_rad();
		double yawrate = get_yaw_rad();

		rate_response[0] = attitude_roll_rate_loop.run(rollrate, rate_setpoint[0]);
		rate_response[1] = attitude_pitch_rate_loop.run(pitchrate, rate_setpoint[1]);
		rate_response[2] = attitude_yaw_rate_loop.run(yawrate, rate_setpoint[2]);

		rate_response_valid = true;
	} else if (att_response_valid) {
		double rollrate = get_rollrate_rad();
		double pitchrate = get_pitchrate_rad();
		double yawrate = get_yawrate_rad();

		rate_response[0] = attitude_roll_rate_loop.run(rollrate, att_response[0]);
		rate_response[1] = attitude_pitch_rate_loop.run(pitchrate, att_response[1]);
		rate_response[2] = attitude_yaw_rate_loop.run(yawrate, att_response[2]);

		rate_response_valid = true;
	}

	if (rate_response_valid) {
		Vector3 rate_info(rate_response[0], rate_response[1], rate_response[2]);
		internal_logger.publishVector(rate_response_i, rate_info);
	}

	for (int i = 0; i < num_actuators; i++) {
		double signal = 0;

		if (has_position_setpoint) {
			signal += pos_response.x * actuator_mixing[i][0];
			signal += pos_response.y * actuator_mixing[i][1];
			signal += pos_response.z * actuator_mixing[i][2];
		}
		if (vel_response_valid) {
			signal += vel_response.x * actuator_mixing[i][3];
			signal += vel_response.y * actuator_mixing[i][4];
			signal += vel_response.z * actuator_mixing[i][5];
		}
		if (att_response_valid) {
			signal += att_response[0] * actuator_mixing[i][6];
			signal += att_response[1] * actuator_mixing[i][7];
			signal += att_response[2] * actuator_mixing[i][8];
		}
		if (rate_response_valid) {
			signal += rate_response[0] * actuator_mixing[i][9];
			signal += rate_response[1] * actuator_mixing[i][10];
			signal += rate_response[2] * actuator_mixing[i][11];
		}

		signal += actuator_mixing[i][12];

		if (signal > 1) signal = 1;
		if (signal < 0) signal = 0;
		
		//Serial.printf("%i = %f\n", i, signal);

		actuators[i]->set(signal);
	}
}

int b_make_controller(int argc, char** argv, StreamDevice* io) {
	NTIOSFile* file;
	char c;
	int result = 0;

	if (argc < 2) {
		io->println("Usage: mkctrl MIXERFILE");
		return -1;
	}

	if (!(file = fsopen(argv[1]))) {
		io->printf("Error opening mixer file %s\n", argv[1]);
		return -2;
	}
	/*if (file->size() > 1024) {
		io->printf("Error: mixer file %s is too big (%.1f KiB of 1KiB)\n", argv[1], file->size() / 1024.0);
		return -3;
	}*/
	char* buffer = (char*)malloc(128); // 1024 is a large buffer for the stack in a thread, use malloc
	ActuatorDevice* actuators[10];
	double mixinfo[10][13];
	int line = 0;
	while (file->available()) {
		int i;
		char* tmp;
		for (i = 0; file->available(); i++) {
			c = file->read();
			if (c == ' ') break;
			buffer[i] = c;
		}
		buffer[i] = 0;
		if (!file->available()) {
			result = -4;
			io->println("Invalid mixer file");
			break;
		}
		tmp = &(buffer[++i]);
		for (; file->available(); i++) {
			c = file->read();
			if (c == '\n' || c == '\r') break;
			buffer[i] = c;
		}
		buffer[i] = 0;
		float mixtmp[13];
		int num_specified = sscanf(tmp, "%f %f %f %f %f %f %f %f %f %f %f %f %f", 
								   &(mixtmp[0]), 
								   &(mixtmp[1]), 
								   &(mixtmp[2]), 
								   &(mixtmp[3]), 
								   &(mixtmp[4]), 
								   &(mixtmp[5]), 
								   &(mixtmp[6]), 
								   &(mixtmp[7]), 
								   &(mixtmp[8]), 
								   &(mixtmp[9]), 
								   &(mixtmp[10]), 
								   &(mixtmp[11]), 
								   &(mixtmp[12]));
		int num_missing = 13 - num_specified;
		if (num_missing) {
			for (int j = num_specified - 1; j >= 0; j--)
				mixinfo[line][j + num_missing] = mixtmp[j];
			for (int j = 0; j < num_missing; j++)
				mixinfo[line][j] = 0;
		}
		// At this point we have all of the data for the actuator mixer info, and buffer has the name of the actuator device to find.
		ActuatorDevice* actuator = NULL;
		for (int j = 0; j < num_devices(); j++) {
			Device* dev = get_device(j);
			if ((dev->getType() & 0xF80) == DEV_TYPE_ACTUATOR) {
				if (!strcmp(dev->getName(), buffer)) {
					actuator = (ActuatorDevice*) dev;
					break;
				}
			}
		}
		if (actuator == NULL) {
			io->printf("Unable to find actuator named '%s'\n", buffer);
			result = -5;
		}
		actuators[line] = actuator;
		line++;
	}
	free(buffer);
	file->close();
	if (result != -1) {
		// create control system
		GenericSymmetricController* controller = new GenericSymmetricController(actuators, mixinfo, line);
		add_device(controller);
	}
	return result;
}

int b_ctrl(int argc, char** argv, StreamDevice* io) {
	int result = 0;

	if (argc < 4)
		result = -1;
	else {
		Device* dev = get_device(atoi(argv[1]));
		if (dev == NULL) {
			io->println("DEVID out of range.");
			result = -2;
		} else if ((dev->getType() & 0xF80) != DEV_TYPE_CONTROL_SYS) {
			io->println("Invalid device.");
			result = -3;
		} else {
			GenericSymmetricController* controller = (GenericSymmetricController*)dev;

			if (!strcmp(argv[2], "set")) {
				if (argc < 7)
					result = -1;
				else {
					char* echk;
					double x, y, z;

					x = strtod(argv[4], &echk);
					if (echk == argv[4])
						result = -10;

					y = strtod(argv[5], &echk);
					if (echk == argv[5])
						result = -10;

					z = strtod(argv[6], &echk);
					if (echk == argv[6])
						result = -10;

					if (result == 0) {
						if (!strcmp(argv[3], "pos")) {
							controller->position_setpoint.x = x;
							controller->position_setpoint.y = y;
							controller->position_setpoint.z = z;
							controller->has_position_setpoint = true;
						} else if (!strcmp(argv[3], "vel")) {
							controller->velocity_setpoint.x = x;
							controller->velocity_setpoint.y = y;
							controller->velocity_setpoint.z = z;
							controller->has_velocity_setpoint = true;
						} else if (!strcmp(argv[3], "att")) {
							controller->attitude_setpoint[0] = x;
							controller->attitude_setpoint[1] = y;
							controller->attitude_setpoint[2] = z;
							controller->has_attitude_setpoint = true;
						} else if (!strcmp(argv[3], "rate")) {
							controller->rate_setpoint[0] = x;
							controller->rate_setpoint[1] = y;
							controller->rate_setpoint[2] = z;
							controller->has_rate_setpoint = true;
						} else
							result = -1;
					}
				}
			} else if (!strcmp(argv[2], "reset")) {
				if (!strcmp(argv[3], "pos"))
					controller->has_position_setpoint = false;
				else if (!strcmp(argv[3], "vel"))
					controller->has_velocity_setpoint = false;
				else if (!strcmp(argv[3], "att"))
					controller->has_attitude_setpoint = false;
				else if (!strcmp(argv[3], "rate"))
					controller->has_rate_setpoint = false;
				else
					result = -1;
			} else
				result = -1;
		}
	}
	
	if (result == -1) {
		io->println("Usage: ctrl DEVID reset SETPOINT");
		io->println("       ctrl DEVID set SETPOINT X/ROLL Y/PITCH Z/YAW");
		io->println(" SETPOINT is one of pos, vel, att, rate");
	} else if (result == -10) {
		io->println("ABORT: Error parsing new setpoint");
	}
	return result;
}
