
#ifndef ESTIMATORS_H
#define ESTIMATORS_H

#include "ntios_math.h"

class ScalarKalmanFilter {
  public:
    ScalarKalmanFilter(double estimate, double error);
    
    double update(double measurement, double error);
  private:
    double est, err;
};

class PositionKalmanFilter {
  public:
    PositionKalmanFilter(double start_pos, double start_vel, double pos_err, double vel_err);
    PositionKalmanFilter();

    void update(double new_pos, double new_vel, double Mposerr, double Mvelerr);
    void updatePos(double new_pos, double Mposerr);
    void updateVel(double new_vel, double Mvelerr);
    void predict(double accel, double Maccerr, double dt);
    void reset();
	double position() { return pos; };
	double positionError() { return poserr; };
	double velocity() { return vel; };
	double velocityError() { return velerr; };
  private:
    double poserr, velerr;
    double pos, vel;
};


class NuclaenFilter {
private:
	double pos_x_m;
	double pos_y_m;
	double pos_err_m;
	bool has_pos = false;

	double alt_m;
	double alt_err_m;
	bool has_alt = false;

	Vector3 vel_ms;
	double vel_err_ms;
	bool has_vel = false;

	Vector3 acc_mss;
	double acc_err_mss;
	bool has_acc = false;

	double temp_c;
	double temp_err_c;
	bool has_temp = false;

	long last_update_micros = -1;
public:

	bool has_2d_position() { return has_pos; };
	bool has_3d_position() { return has_pos && has_alt; };
	bool has_altitude() { return has_alt; };
	bool has_velocity() { return has_vel; };
	bool has_acceleration() { return has_acc; };
	bool has_temperature() { return has_temp; };

	double error_2d_position() { return pos_err_m; };
	double error_altitude() { return alt_err_m; };
	double error_velocity() { return vel_err_ms; };
	double error_acceleration() { return acc_err_mss; };
	double error_temperature() { return temp_err_c; };

	double state_age_seconds();

	Vector3 get_3d_position_meters() { return Vector3(pos_x_m, pos_y_m, alt_m); };
	Vector2 get_2d_position_meters() { return Vector2(pos_x_m, pos_y_m); };
	double get_latitude() { return pos_x_m / 111320.0; };
	double get_longitude() { return pos_y_m / (cos(pos_x_m / 6378166.175053201) * 111320.0); };
	

	double get_altitude_meters() { return alt_m; };

	Vector3 get_velocity() { return vel_ms; };

	Vector3 get_acceleration() { return acc_mss; };

	double get_temperature() { return temp_c; };

	void simulate();

	void feed_gps_position(double latitude, double longitude, double error_m);
	void feed_gps_velocity(double course, double speed_ms, double error_ms);
	void feed_2d_velocity(Vector2 mps, double error_ms);
	void feed_linear_global_acceleration(Vector3 accel, double error_mss);
	void feed_altitude(double altitude, double error_m);
	void feed_temperature(double temperature, double error_c);
};

#endif
