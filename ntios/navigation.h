
#ifndef NAVIGATION_H

#include "drivers.h"
#include "ntios.h"
#include "ntios_math.h"

double get_longitude();
double get_latitude();
double get_position_certainty();

double get_position_x();
double get_position_y();
double get_altitude();

Vector3 get_position_m();
Vector3 get_velocity_ms();
Quaternion get_attitude_quat();

double get_roll_rad();
double get_pitch_rad();
double get_yaw_rad();

double get_rollrate_rad();
double get_pitchrate_rad();
double get_yawrate_rad();

void _ntios_nav_updater_thread(void* nothing);

class NavigationDevice;

typedef void (*nav_callback_function)(NavigationDevice*);

int nav_listener_program(StreamDevice* io);
int nav_cal_program(int argc, char** argv, StreamDevice* io);

class NavigationDevice: public Device {
public:
	int getType() { return DEV_TYPE_NAVIGATION; }

	virtual double latitude() = 0;
	virtual double longitude() = 0;
	virtual double position_certainty() = 0;
	virtual Vector2 get_velocity() = 0;
	virtual double velocity_certainty() = 0;
	virtual bool add_callback(nav_callback_function callback) = 0;
	virtual void rm_callback(nav_callback_function callback) = 0;
};

class NMEARawGPS: public NavigationDevice {
private:
	StreamDevice* stream;
	nav_callback_function callbacks[8];
	int num_cbs = 0;
	volatile double last_lat = -1;
	volatile double last_lon = -1;
	volatile double certainty = -1;

	int gps_vel_i;

	uint32_t vel_timer = 0;

	Vector2 vel;

	char name[20];
public:
	NMEARawGPS(StreamDevice* s, const char* name);

	int getType() { return DEV_TYPE_GPS_RAW; }
	const char* getName() { return name; }

	double latitude() { return last_lat; }
	double longitude() { return last_lon; }
	double position_certainty() { return certainty; }
	Vector2 get_velocity() { return vel; }
	double velocity_certainty() { if (vel_timer > millis()) return certainty / 10; else return -1; }

	bool add_callback(nav_callback_function callback);
	void rm_callback(nav_callback_function callback);

	void update();
};

class IMUDevice: public Device {
public:
	int getType() { return DEV_TYPE_IMU; }

	virtual bool isMagCalibrated() = 0;
	virtual bool isAccelCalibrated() = 0;
	virtual bool isGyroCalibrated() = 0;

	virtual bool getQuatAttitude(Quaternion& quat) = 0;

	// get relative frame acceleration without gravity
	virtual bool getRelativeAcceleration(Vector3& vec) = 0;

	// get global frame acceleration without gravity
	virtual bool getGlobalAcceleration(Vector3& vec);

	// get euler rotation speed
	virtual bool getGyroEuler(Vector3& vec) = 0;

	virtual bool saveAccelCal() = 0;
};

#endif
