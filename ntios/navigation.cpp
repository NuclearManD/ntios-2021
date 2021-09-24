
#include <stdlib.h>

#include "navigation.h"
#include "ntios.h"
#include "estimators.h"
#include "csvlog.h"

NuclaenFilter _ntios_positioning_filter;

bool _nav_has_attitude_data = false;
double _nav_roll_rad, _nav_pitch_rad, _nav_yaw_rad;
double _nav_rollrate_rad, _nav_pitchrate_rad, _nav_yawrate_rad;
Quaternion _nav_att_quat;

BootloaderMutex* _nav_mutex;

uint64_t last_unix_satellite_time = 0;
long last_millis_satellite_time = -1;

// This function is in navigation because GPS is the primary way for NTIOS devices to get the time.
uint64_t get_unix_millis() {
	if (last_millis_satellite_time == -1)
		return 0;
	return (unsigned long)(millis() - last_millis_satellite_time) + last_unix_satellite_time;
}

void process_date_time_input(int year, int month, int day, int hour, int minute, double second) {
	// Set this first because this is the reference of when the time was recorded.
	// The time recorded will be relative to the system millis clock.
	last_millis_satellite_time = millis();

	static const int secs_through_month[] = {
		0, 31*86400, 59*86400, 90*86400,
		120*86400, 151*86400, 181*86400, 212*86400,
		243*86400, 273*86400, 304*86400, 334*86400 };
	uint64_t secs;
	int is_leap;

	// Largely modified code from http://git.musl-libc.org/cgit/musl/tree/src/time/
	if (year <= 2038) {
		int y = year - 1900;
		int leaps = (y - 68) >> 2;
		if (!((y - 68) & 3)) {
			leaps--;
			is_leap = 1;
		} else 
			is_leap = 0;

		secs = 31536000*(y - 70) + 86400*leaps;

	} else {
		int cycles, centuries, leaps, rem;

		cycles = (year - 2000) / 400;
		rem = (year - 2000) % 400;
		if (rem < 0) {
			cycles--;
			rem += 400;
		}
		if (!rem) {
			centuries = 0;
			leaps = 0;
			is_leap = true;
		} else {
			if (rem >= 200) {
				if (rem >= 300) centuries = 3, rem -= 300;
				else centuries = 2, rem -= 200;
			} else {
				if (rem >= 100) centuries = 1, rem -= 100;
				else centuries = 0;
			}
			if (!rem) {
				leaps = 0;
				is_leap = false;
			} else {
				leaps = rem / 4U;
				rem %= 4U;
				is_leap = !rem;
			}
		}

		leaps += 97*cycles + 24*centuries - is_leap;

		secs = (year-2000) * 31536000ULL + leaps * 86400ULL + 946684800ULL + 86400ULL;
	}

	secs += secs_through_month[month - 1];
	if (is_leap && month >= 3) secs += 86400;

	secs += 86400ULL * (day - 1);
	secs += 3600ULL * hour;
	secs += 60ULL * minute;
	//t += tm->tm_sec;

	last_unix_satellite_time = (secs * 1000ULL) + (uint64_t)(second * 1000);
}

int nav_listener_program(StreamDevice* io) {
	io->println("Press any key to exit.");
	io->flush();
	while(io->available())io->read();
	io->flush();
	while (true) {
		if (io->available())
			break;
		if (_ntios_positioning_filter.has_2d_position()) {
			double lat = get_latitude();
			double lon = get_longitude();
			double err = get_position_certainty();
			io->printf("GPS:          %.10f %.10f +/- %.3fm\n", lat, lon, err);
		}
		if (_ntios_positioning_filter.has_altitude()) {
			double alt = _ntios_positioning_filter.get_altitude_meters();
			double err = _ntios_positioning_filter.error_altitude();
			io->printf("Altitude:     %.2f +/- %.3fm\n", alt, err);
		}
		if (_ntios_positioning_filter.has_velocity()) {
			Vector3 v = _ntios_positioning_filter.get_velocity();
			double err = _ntios_positioning_filter.error_velocity();
			io->printf("Velocity:     %.3f %.3f %.3f +/- %.3fm/2\n", v.x, v.y, v.z, err);
		}
		if (_ntios_positioning_filter.has_acceleration()) {
			Vector3 v = _ntios_positioning_filter.get_acceleration();
			double err = _ntios_positioning_filter.error_acceleration();
			io->printf("Acceleration: %.3f %.3f %.3f +/- %.3fm/2\n", v.x, v.y, v.z, err);
		}
		if (_nav_has_attitude_data) {
			io->printf("Attitude: %.1f %.1f %.1f\n", 
					  _nav_roll_rad * 57.2957795, 
					  _nav_pitch_rad * 57.2957795, 
					  _nav_yaw_rad * 57.2957795);
			io->printf("Attitude rate: %.1f %.1f %.1f\n", 
					  _nav_rollrate_rad * 57.2957795, 
					  _nav_pitchrate_rad * 57.2957795, 
					  _nav_yawrate_rad * 57.2957795);
		}
		io->println();
		delay(200);
	}
	return 0;
}


int nav_cal_program(int argc, char** argv, StreamDevice* io) {
	if (argc < 3) {
		io->println("usage: cal DEVID reconf/accel");
		return -1;
	}
	if (!strcmp(argv[2], "accel")) {
		char* echk;
		int port = (int)strtol(argv[1], &echk, 10);
		if (*echk != 0 || argv[1][0] == 0 || port < 0 || port >= num_devices()) {
			io->printf("Error: bad device id '%s'\n", argv[1]);
			return -2;
		} else if ((get_device(port)->getType() & 0xFF80) != DEV_TYPE_IMU) {
			io->println("Error: not an IMU.");
			return -3;
		} else {
			IMUDevice* imu = (IMUDevice*)get_device(port);
			io->printf("Do calibration procedure for %s, settings will be captured automatically\n", imu->getName());
			while (true) {
				if (!imu->isAccelCalibrated()) {
					io->println("Waiting for IMU to have calibration data...");
					while (!imu->isAccelCalibrated())
						delay(20);
				}
				io->println("Keep IMU perfectly still in level position, checking cal in 5 seconds!");
				delay(5000);
				io->println("Checking cal...");
				Vector3 acc;
				Vector3 tmp;
				for (int i = 0; i < 100; i++) {
					if (imu->getRelativeAcceleration(tmp))
						acc = acc + tmp;
					else
						i--;
					delay(10);
				}
				if (abs(acc.x) < 30 && abs(acc.y) < 30 && abs(acc.z) < 30) {
					io->println("Good cal detected, saving...");
					if (imu->saveAccelCal()) {
						io->println("Success!");
						return 0;
					} else {
						io->println("IMU Driver failed to save accelerometer calibrations :(");
						return -20;
					}
				}
				io->println("Bad cal detected, retry procedure, settings will be captured automatically.");
				io->println("Checking again in 10 seconds.");
				delay(10000);
			}
		}
	} else if (!strcmp(argv[2], "reconf")) {
		io->println("reconf not yet supported.");
		return 500;
	} else {
		io->printf("Bad option '%s'\n", argv[2]);
		return -1;
	}
}

void _ntios_nav_cb(NavigationDevice* dev) {
	double certainty;

	_nav_mutex->lock();
	if ((certainty = dev->position_certainty()) != -1) {
		_ntios_positioning_filter.feed_gps_position(dev->latitude(), dev->longitude(), certainty);
	}

	if ((certainty = dev->velocity_certainty()) != -1) {
		_ntios_positioning_filter.feed_2d_velocity(dev->get_velocity(), certainty);
	}
	_nav_mutex->unlock();
}

void _ntios_nav_updater_thread(void* nothing) {
	(void) nothing;

	_nav_mutex = bootloader_make_mutex();
	int i_roll = internal_logger.markDouble("attitude.roll");
	int i_pitch = internal_logger.markDouble("attitude.pitch");
	int i_yaw = internal_logger.markDouble("attitude.yaw");

	int i_rollrate = internal_logger.markDouble("attitude_rate.roll");
	int i_pitchrate = internal_logger.markDouble("attitude_rate.pitch");
	int i_yawrate = internal_logger.markDouble("attitude_rate.yaw");

	int i_acc_imu = internal_logger.markVector("imu_acceleration");
	int i_acc = internal_logger.markVector("acceleration");
	int i_vel = internal_logger.markVector("velocity");
	int i_pos = internal_logger.markVector("position");

	int i_pos_err = internal_logger.markDouble("position_error");
	int i_vel_err = internal_logger.markDouble("velocity_error");
	int i_acc_err = internal_logger.markDouble("acceleration_error");

	uint32_t next_publish = millis();

	while (true) {
		_nav_mutex->lock();
		_ntios_positioning_filter.simulate();
		_nav_mutex->unlock();

		bool got_imu = false;
		for (int i = 0; i < num_devices(); i++) {
			Device* dev = get_device(i);
			if (dev->getType() == DEV_TYPE_IMU) {
				Vector3 vec;
				if (((IMUDevice*)dev)->getGlobalAcceleration(vec)) {
					internal_logger.publishVector(i_acc_imu, vec);
					_nav_mutex->lock();
					if (vec.magnitude() > .8)
						_ntios_positioning_filter.feed_linear_global_acceleration(vec, 1);
					else
						_ntios_positioning_filter.feed_linear_global_acceleration(vec, .2);
					_nav_mutex->unlock();
				}
				if (!got_imu) {
					_nav_has_attitude_data = true;
					// For now just use the first IMU we find
					if (((IMUDevice*)dev)->getGyroEuler(vec)) {
						_nav_rollrate_rad = vec.x;
						_nav_pitchrate_rad = vec.y;
						_nav_yawrate_rad = vec.z;
					} else
						_nav_has_attitude_data = false;
					Quaternion q;
					if (((IMUDevice*)dev)->getQuatAttitude(q)) {
						_nav_att_quat = q;
						_nav_roll_rad = atan2(q.x * q.w + q.y * q.z / 2, 0.5 - q.x * q.x - q.y * q.y);
						_nav_pitch_rad = asin(-2.0 * (q.x*q.z - q.w*q.y));
						_nav_yaw_rad = atan2(q.x*q.y + q.w*q.z, 0.5 - q.y*q.y - q.z*q.z);
					} else
						_nav_has_attitude_data = false;
					got_imu = true;
				}
			}
		}
		
		bool did_publish = false;

		if (_ntios_positioning_filter.has_3d_position()) {
			internal_logger.publishVector(i_pos, _ntios_positioning_filter.get_3d_position_meters());
			internal_logger.publishDouble(i_pos_err, _ntios_positioning_filter.error_2d_position());
			did_publish = true;
		} else if (_ntios_positioning_filter.has_2d_position()) {
			Vector2 vec = _ntios_positioning_filter.get_2d_position_meters();
			internal_logger.publishVector(i_pos, Vector3(vec.x, vec.y, 0));
			internal_logger.publishDouble(i_pos_err, _ntios_positioning_filter.error_2d_position());
			did_publish = true;
		}
		if (_ntios_positioning_filter.has_velocity()) {
			internal_logger.publishVector(i_vel, _ntios_positioning_filter.get_velocity());
			internal_logger.publishDouble(i_vel_err, _ntios_positioning_filter.error_velocity());
			did_publish = true;
		}
		if (_ntios_positioning_filter.has_acceleration()) {
			internal_logger.publishVector(i_acc, _ntios_positioning_filter.get_acceleration());
			internal_logger.publishDouble(i_acc_err, _ntios_positioning_filter.error_acceleration());
			did_publish = true;
		}
		if (_nav_has_attitude_data) {
			internal_logger.publishDouble(i_roll, _nav_roll_rad * 57.2957795);
			internal_logger.publishDouble(i_pitch, _nav_pitch_rad * 57.2957795);
			internal_logger.publishDouble(i_yaw, _nav_yaw_rad * 57.2957795);
			internal_logger.publishDouble(i_rollrate, _nav_rollrate_rad * 57.2957795);
			internal_logger.publishDouble(i_pitchrate, _nav_pitchrate_rad * 57.2957795);
			internal_logger.publishDouble(i_yawrate, _nav_yawrate_rad * 57.2957795);
			did_publish = true;
		}
		
		if (next_publish < millis() && !did_publish) {
			internal_logger.flush();
			did_publish = true;
		}
		
		if (did_publish)
			next_publish = millis() + 1000;

		delay(8);
	}
}

double get_longitude() {
	return _ntios_positioning_filter.get_longitude();
}

double get_latitude() {
	return _ntios_positioning_filter.get_latitude();
}

double get_position_certainty() {
	if (_ntios_positioning_filter.has_2d_position())
		return _ntios_positioning_filter.error_2d_position();
	else
		return -1;
}

Vector3 get_position_m() { return _ntios_positioning_filter.get_3d_position_meters(); }
Vector3 get_velocity_ms() { return _ntios_positioning_filter.get_velocity(); }


double get_position_x() { return _ntios_positioning_filter.get_3d_position_meters().x; }
double get_position_y() { return _ntios_positioning_filter.get_3d_position_meters().y; }
double get_altitude() { return _ntios_positioning_filter.get_altitude_meters(); }

double get_roll_rad() { return _nav_roll_rad; }
double get_pitch_rad() { return _nav_pitch_rad; }
double get_yaw_rad() { return _nav_yaw_rad; }

double get_rollrate_rad() { return _nav_rollrate_rad; }
double get_pitchrate_rad() { return _nav_pitchrate_rad; }
double get_yawrate_rad() { return _nav_yawrate_rad; }

Quaternion get_attitude_quat() { return _nav_att_quat; }


NMEARawGPS::NMEARawGPS(StreamDevice* s, const char* name) {
	stream = s;
	strlcpy(this->name, name, 20);
	gps_vel_i = internal_logger.markVector("gps_velocity");
}

bool NMEARawGPS::add_callback(nav_callback_function callback) {
	if (num_cbs >= 8) return false;
	if (callback == NULL) {
		return false;
	}

	callbacks[num_cbs] = callback;
	num_cbs++;

	return true;
}

void NMEARawGPS::rm_callback(nav_callback_function callback) {
	for (int i = 0; i < num_cbs; i++) {
		if (callback == callbacks[i]) {
			--num_cbs;
			callbacks[i] = callbacks[num_cbs];
		}
	}
}

double read_nmea_coord(char** ptr) {
	char buffer[8];
	int i = 0;
	for (; (*ptr)[i] != '.'; i++)
		buffer[i] = (*ptr)[i];

	i -= 2;
	buffer[i] = 0;
	
	double degrees = atoi(buffer);
	double minutes = strtod(*ptr + i, ptr);
	return degrees + minutes / 60.0;
}

void NMEARawGPS::update() {
	static char buffer[74]; // maximum NMEA sentence length with delimiters is 74
	static int bufloc;

	while (stream->available()) {
		char c = stream->read();

		if (c == '$')
			bufloc = 0;
		else if (c == '\r' || c == '\n') {
			buffer[bufloc] = 0;
			if (bufloc == 0) continue;
			if (!memcmp(buffer + 2, "GGA,", 4)) {
				// position and fix data
				
				int i = 7;
				for (; i < bufloc && buffer[i] != ','; i++); // skip time
				i++;
				
				if (buffer[i] == ',') continue;

				char* ptr = &(buffer[i]);
				char* end = &(buffer[bufloc]);

				// get latitude
				double tlat = read_nmea_coord(&ptr);
				
				if (ptr[2] == 'N') tlat = -tlat;
				ptr += 3;

				double tlon = read_nmea_coord(&ptr);
				
				if (ptr[2] != 'W') tlon = -tlon;
				ptr += 3;

				for (; ptr < end && *ptr != ','; ptr++); // skip fix quality
				ptr++;
				for (; ptr < end && *ptr != ','; ptr++); // skip num satellites
				ptr++;
				
				double hdop = strtod(ptr, &ptr);
				
				if (hdop != 0 && hdop != 99.99) {
					last_lat = tlat;
					last_lon = tlon;
					certainty = 9 * hdop;
					//Serial.println(hdop);
					//Serial.println(buffer);
					//Serial.println(num_cbs);
					for (int i = 0; i < num_cbs; i++) {
						callbacks[i](this);
					}
				}
			} else if (!memcmp(buffer + 2, "RMC,", 4)) {
				// position and velocity data

				int i = 6;

				int hour = (buffer[i] - '0') * 10 + buffer[i + 1] - '0';
				i+=2;

				int minute = (buffer[i] - '0') * 10 + buffer[i + 1] - '0';
				i+=2;

				char* ptr = &(buffer[i]);
				char* end = &(buffer[bufloc]);

				double second = strtod(ptr, &ptr);

				ptr++;
				for (; ptr < end && *ptr != ','; ptr++); // skip status
				ptr++;

				if (*ptr != ',') {

					// get latitude
					double tlat = read_nmea_coord(&ptr);
					
					if (ptr[2] == 'N') tlat = -tlat;
					ptr += 3;

					double tlon = read_nmea_coord(&ptr);
					
					if (ptr[2] != 'W') tlon = -tlon;
					ptr += 3;

					// get and calculate speed

					char* errdetect;

					double meters_per_second = 0.514444444 * strtod(ptr, &errdetect);


					if (errdetect != ptr) {
						ptr = errdetect + 1;
						double course = strtod(ptr, &errdetect);
						if (errdetect != ptr) {
							ptr = errdetect + 1;
							vel.x = cos(course / 57.29577951) * meters_per_second;
							vel.y = sin(course / 57.29577951) * meters_per_second;
							internal_logger.publishVector(gps_vel_i, Vector3(vel.x, vel.y, 0));
							vel_timer = millis() + 1000;
						} else {
							ptr = errdetect + 1;
						}
					} else {
						ptr = errdetect + 1;
					}
				} else {
					ptr += 6;
				}

				int day = (*ptr - '0') * 10 + ptr[1] - '0';
				ptr+=2;

				int month = (*ptr - '0') * 10 + ptr[1] - '0';
				ptr+=2;

				int year = (*ptr - '0') * 10 + ptr[1] - '0' + 2000;
				ptr+=2;

				process_date_time_input(year, month, day, hour, minute, second);

				/*for (; ptr < end && *ptr != ','; ptr++); // skip fix quality
				ptr++;
				for (; ptr < end && *ptr != ','; ptr++); // skip num satellites
				ptr++;
				
				double hdop = strtod(ptr, &ptr);*/
				
				/*if (hdop != 0 && hdop != 99.99) {
					last_lat = tlat;
					last_lon = tlon;
					certainty = 9 * hdop;
					//Serial.println(hdop);
					//Serial.println(buffer);
					//Serial.println(num_cbs);
					for (int i = 0; i < num_cbs; i++) {
						callbacks[i](this);
					}
				}*/
				
			}
		} else if (bufloc < 74)
			buffer[bufloc++] = c;
		
	}
}

bool IMUDevice::getGlobalAcceleration(Vector3& vec) {
	Quaternion quat;
	Vector3 acc;

	if (!getQuatAttitude(quat))
		return false;
	if (!getRelativeAcceleration(acc))
		return false;

	vec = quat.rotateVector(acc);
	return true;
}
