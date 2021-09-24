
#ifndef IMU_DRIVERS_H
#define IMU_DRIVERS_H

#include "../navigation.h"
#include "../ntios_math.h"


class BNO055: public IMUDevice {
private:
	I2CBusDevice& i2c;
	bool sensor_detected = false;
	uint8_t bno055_adr;

	bool have_mag_cal;
	bool have_acc_cal;
	bool have_gyro_cal;

	uint8_t read8(uint8_t adr);
	bool readLen(uint8_t adr, uint8_t *buffer, uint8_t len);
	bool write8(uint8_t adr, uint8_t value);
	bool writeLen(uint8_t adr, const uint8_t *buffer, uint8_t len);

	void configure();

	char caldir[32];

	unsigned long update_timer = 0;
	int acc_i;

	bool configurator_lock = false;

public:

	const char* getName() { return "BNO055"; };

	BNO055(I2CBusDevice& device, uint8_t imuadr = 0x28, const char* calinfo_dir = "/etc/bno055cal/");

	bool isMagCalibrated();
	bool isAccelCalibrated();
	bool isGyroCalibrated();

	bool getQuatAttitude(Quaternion& quat);
	bool getRelativeAcceleration(Vector3& vec);
	bool getGyroEuler(Vector3& vec);

	void update();

	bool saveAccelCal();
};


#endif
