
#include "ntios.h"
#include "estimators.h"

ScalarKalmanFilter::ScalarKalmanFilter(double estimate, double error){
	err = error;
	est = estimate;
}

double ScalarKalmanFilter::update(double measurement, double error){
	double kg = err / (err + error);
	est += kg * (measurement - est);
	return est;
}


PositionKalmanFilter::PositionKalmanFilter(double start_pos, double start_vel, double pos_err, double vel_err) {
	pos = start_pos;
	vel = start_vel;
	poserr = pos_err;
	velerr = vel_err;
}

PositionKalmanFilter::PositionKalmanFilter() {
	poserr = -1;
	velerr = -1;
}

void PositionKalmanFilter::update(double new_pos, double new_vel, double Mposerr, double Mvelerr) {
	if (poserr > 0) {
		double pos_kg = poserr / (poserr + Mposerr);
		double vel_kg = velerr / (velerr + Mvelerr);

		pos += pos_kg * (new_pos - pos);
		vel += vel_kg * (new_vel - vel);

		poserr *= 1 - pos_kg;
		velerr *= 1 - vel_kg;
	} else {
		pos = new_pos;
		vel = new_vel;

		poserr = Mposerr;
		velerr = Mvelerr;
	}
}

void PositionKalmanFilter::updatePos(double new_pos, double Mposerr) {
	if (poserr > 0) {
		double pos_kg = poserr / (poserr + Mposerr);

		pos += pos_kg * (new_pos - pos);

		poserr *= 1 - pos_kg;
	} else {
		pos = new_pos;

		poserr = Mposerr;
	}
}

void PositionKalmanFilter::updateVel(double new_vel, double Mvelerr) {
	if (velerr > 0) {
		double vel_kg = velerr / (velerr + Mvelerr);

		vel += vel_kg * (new_vel - vel);

		velerr *= 1 - vel_kg;
	} else {
		vel = new_vel;

		velerr = Mvelerr;
	}
}

void PositionKalmanFilter::predict(double accel, double Maccerr, double dt) {
	pos += (vel + (accel * dt / 2)) * dt;
	vel += accel * dt;

	poserr += (velerr + (Maccerr * dt / 2)) * dt;
	velerr += Maccerr * dt;
}

void PositionKalmanFilter::reset(){
	vel = 0;
	poserr *= 1000;
	velerr = 10;
}



double NuclaenFilter::state_age_seconds() {
	long time = micros();
	if (time > last_update_micros)
		return (time - last_update_micros) / 1000000.0;
	else
		return time / 1000000.0; // underestimate how long it was
}

void NuclaenFilter::simulate() {
	// Only simulate if we have a valid dt
	if (last_update_micros != -1) {

		double dt = state_age_seconds();

		if (has_vel) {

			if (has_acc) {
				// update velocity with acceleration
				vel_ms = vel_ms + (acc_mss * dt);
				vel_err_ms += dt * acc_err_mss;

				if (acc_err_mss < 0.3 && acc_mss.magnitude() < 0.15) {
					// If we're very certain that we're not accelerating, then reduce
					// the estimated velocity
					// Makes the estimates more reliable and stable
					vel_ms = vel_ms * 0.95;
					vel_err_ms *= 0.95;
				}
			} else {
				// no acceleration - dilute accuracy of old velocity
				// loss of 1 m/ss (roughly one tenth of gravity)
				vel_err_ms += dt;
				if (vel_err_ms > 8)
					has_vel = false; // lost velocity!
			}

			// use velocity to update position
			pos_x_m += vel_ms.x * dt;
			pos_y_m += vel_ms.y * dt;
			pos_err_m += vel_err_ms * dt;

			// update altitude if we have it
			if (has_alt) {
				alt_m += vel_ms.z * dt;
				alt_err_m += vel_err_ms * dt;
			}
		} else {
			// dilute accuracy of old position
			// loss of 1 m/s (about 2 mph)
			pos_err_m += 1 * dt;
			
			if (has_alt) {
				// if we have altitude but no velocity, dilute altitude accurracy
				// loss of 0.5 m/s (about 1 mph)
				alt_err_m += 0.5 * dt;
			}
		}

		if (pos_err_m > 100 || pos_err_m < 0)
			has_pos = false; // lost position!

		if (alt_err_m > 30 || alt_err_m < 0)
			has_alt = false; // lost altitude!

		if (has_acc) {
			// dilute accuracy of acceleration by 1 m/s2
			acc_err_mss += dt;
			if (acc_err_mss > 4)
				has_acc = false; // lost acceleration!
		}

		if (has_temp) {
			// dilute accuracy of temperature by 3 degrees per minute
			temp_err_c += dt / 20;

			if (temp_err_c > 10)
				has_temp = false; // lost temperature!
		}
	}

	// Update state age counter
	last_update_micros = micros();
}

void NuclaenFilter::feed_gps_position(double latitude, double longitude, double error_m) {
	if (error_m > pos_err_m && has_pos)
		return; // do not use data less reliable than what we have
	if (abs(latitude) > 180 || abs(longitude) > 180)
		return; // if the data is invalid then don't use it

	// convert to x/y in meters
	// This does NOT work well at the poles...
	// At some point this math should be redesigned, the fundamental problem is that it aims to put
	// all coordinates onto a flat plane.  The earth, of course, is not flat.

	double x = latitude * 111320.0;
	double y = longitude * cos(latitude / 57.29577951) * 111320;

	if (has_pos) {
		double magnitude_gate = 10000; // if the new position is 10km away...
		if (has_vel) {
			// check for velocity and error
			// if the new position is farther away than our velocity could ever expect...
			magnitude_gate = (vel_ms.magnitude() + vel_err_ms) * 10;
		}
		
		Vector2 original(pos_x_m, pos_y_m);
		Vector2 gpspos(x, y);

		double mag = (original - gpspos).magnitude();

		if (mag > magnitude_gate)
			error_m *= 10 * mag / magnitude_gate;

		double error_gain = 0.125 * pos_err_m / (pos_err_m + error_m);

		pos_x_m += error_gain * (x - pos_x_m);
		pos_y_m += error_gain * (y - pos_y_m);
		pos_err_m = (pos_err_m + error_m * 8) / 9;
	} else {
		pos_x_m = x;
		pos_y_m = y;
		pos_err_m = error_m;
		has_pos = true;
	}
}

void NuclaenFilter::feed_gps_velocity(double course, double speed_ms, double error_ms) {
	if (error_ms > vel_err_ms && has_vel)
		return; // do not use data less reliable than what we have

	double x = cos(course / 57.29577951) * speed_ms;
	double y = sin(course / 57.29577951) * speed_ms;

	if (has_vel) {
		double error_gain = 0.125 * vel_err_ms / (vel_err_ms + error_ms);

		vel_ms.x += error_gain * (x - vel_ms.x);
		vel_ms.y += error_gain * (y - vel_ms.y);
		vel_err_ms = (vel_err_ms + error_ms * 8) / 9;
	} else {
		vel_ms = Vector3(x, y, 0);
		vel_err_ms = error_ms;
		has_vel = true;
	}
}

void NuclaenFilter::feed_2d_velocity(Vector2 mps, double error_ms) {
	if (error_ms > vel_err_ms && has_vel)
		return; // do not use data less reliable than what we have

	if (has_vel) {
		double error_gain = 0.125 * vel_err_ms / (vel_err_ms + error_ms);

		vel_ms.x += error_gain * (mps.x - vel_ms.x);
		vel_ms.y += error_gain * (mps.y - vel_ms.y);
		vel_err_ms = (vel_err_ms + error_ms * 8) / 9;
	} else {
		vel_ms = Vector3(mps.x, mps.y, 0);
		vel_err_ms = error_ms;
		has_vel = true;
	}
}

void NuclaenFilter::feed_linear_global_acceleration(Vector3 accel, double error_mss) {
	if (error_mss > acc_err_mss && has_acc)
		return; // do not use data less reliable than what we have

	if (accel.magnitude() > 30)
		return; // do not use unrealistic values.

	if (has_acc) {
		double error_gain = 0.5 * acc_err_mss / (acc_err_mss + error_mss);

		acc_mss = acc_mss + ((accel - acc_mss) * error_gain);
		acc_err_mss = (acc_err_mss + error_mss * 2) / 3;

		if (!has_vel) {
			vel_ms = Vector3(0, 0, 0);
			vel_err_ms = 10;
			has_vel = true;
		}
	} else {
		acc_mss = accel;
		acc_err_mss = error_mss;
		has_acc = true;
	}
}

void NuclaenFilter::feed_altitude(double altitude, double error_m) {
	if (error_m > alt_err_m && has_alt)
		return; // do not use data less reliable than what we have

	if (has_alt) {
		double error_gain = 0.25 * alt_err_m / (alt_err_m + error_m);

		alt_m += error_gain * (altitude - alt_m);
		alt_err_m = (alt_err_m + error_m * 4) / 5;
	} else {
		alt_m = altitude;
		alt_err_m = error_m;
		has_alt = true;
	}
}

void NuclaenFilter::feed_temperature(double temperature, double error_c) {
	if (error_c > temp_err_c && has_temp)
		return; // do not use data less reliable than what we have

	if (has_temp) {
		double error_gain = temp_err_c / (temp_err_c + error_c);

		temp_c += error_gain * (temperature - temp_c);
		temp_err_c = (temp_err_c + error_c) / 2;
	} else {
		temp_c = temperature;
		temp_err_c = error_c;
		has_temp = true;
	}
}
