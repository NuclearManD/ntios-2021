
#include "drivers.h"
#include "ntios.h"
#include "drivers/imu_drivers.h"
#include "../csvlog.h"

/*
 * Based on code from Adafruit BNO055 library
 *
*/

/*
 * operation mode 0x0C
 * sensorid -1
 * sensor address 0x28 (address A, address B is 0x29)
 * BNO05 ID is 0xA0
*/

#define DEV_ID_REG 0x00
#define ACCELEROMETER_REG 0x08
#define GYRO_REG 0X14
#define QUATERNION_REG 0x20
#define LINEARACCEL_REG 0x28
#define CAL_STATE_REG 0x35
#define MODE_REG 0x3D
#define SYS_TRIGGER_REG 0x3F


/* Accelerometer Offset registers */
#define ACCEL_OFFSET_X_LSB_ADDR 0x55
#define ACCEL_OFFSET_X_MSB_ADDR 0x56
#define ACCEL_OFFSET_Y_LSB_ADDR 0x57
#define ACCEL_OFFSET_Y_MSB_ADDR 0x58
#define ACCEL_OFFSET_Z_LSB_ADDR 0x59
#define ACCEL_OFFSET_Z_MSB_ADDR 0x5A

/* Magnetometer Offset registers */
#define MAG_OFFSET_X_LSB_ADDR 0x5B
#define MAG_OFFSET_X_MSB_ADDR 0x5C
#define MAG_OFFSET_Y_LSB_ADDR 0x5D
#define MAG_OFFSET_Y_MSB_ADDR 0x5E
#define MAG_OFFSET_Z_LSB_ADDR 0x5F
#define MAG_OFFSET_Z_MSB_ADDR 0x60

/* Gyroscope Offset register s*/
#define GYRO_OFFSET_X_LSB_ADDR 0x61
#define GYRO_OFFSET_X_MSB_ADDR 0x62
#define GYRO_OFFSET_Y_LSB_ADDR 0x63
#define GYRO_OFFSET_Y_MSB_ADDR 0x64
#define GYRO_OFFSET_Z_LSB_ADDR 0x65
#define GYRO_OFFSET_Z_MSB_ADDR 0x66

/* Radius registers */
#define ACCEL_RADIUS_LSB_ADDR 0x67
#define ACCEL_RADIUS_MSB_ADDR 0x68
#define MAG_RADIUS_LSB_ADDR 0x69
#define MAG_RADIUS_MSB_ADDR 0x6A

#define BNO055_ID 0xA0

BNO055::BNO055(I2CBusDevice& device, uint8_t imuadr, const char* calinfo_dir) : 
	i2c(device),
	bno055_adr(imuadr)
{

	strncpy(caldir, calinfo_dir, 31);
	caldir[31] = 0;

	acc_i = internal_logger.markVector("bno055_rel_acc");
}

void BNO055::configure() {

	if (configurator_lock) return; // already configuring
	configurator_lock = true;

	/* Reset */
	write8(SYS_TRIGGER_REG, 0x20);

	/* Delay incrased to 30ms due to power issues https://tinyurl.com/y375z699 */
	delay(30);
	if (read8(DEV_ID_REG) != BNO055_ID) {
		return;
	}
	delay(10);

	/* attempt to load calibrations */
	if (fsexists(caldir)) {
		char cal_fn[36];
		NTIOSFile* file;
		char buf[8];

		strcpy(cal_fn, caldir);
		strcat(cal_fn, "acc");

		//Serial.println(cal_fn);
		if (fsexists(cal_fn)) {
			// load accelerometer calibrations
			file = fsopen(cal_fn);
			if (file->size() == 8) {
				// FORMAT: 6 bytes for 3 16-bit accel offsets then 2 bytes for a 16-bit accel radius
				for (int i = 0; i < 8; i++)
					buf[i] = file->read();
				writeLen(ACCEL_OFFSET_X_LSB_ADDR, (const uint8_t*)buf, 6);
				writeLen(ACCEL_RADIUS_LSB_ADDR, (const uint8_t*)(buf + 6), 2);
				DEBUG_PRINTF("Loaded accelerometer calibration data.\n");
			} else
				DEBUG_PRINTF("Broken BNO055 accelerometer calibration!\n");
			file->close();
		} else
			DEBUG_PRINTF("No BNO055 accelerometer calibration.\n");

		strcpy(cal_fn, caldir);
		strcat(cal_fn, "mag");

		if (fsexists(cal_fn)) {
			// load magnetometer calibrations
			file = fsopen(cal_fn);
			if (file->size() == 8) {
				// FORMAT: 6 bytes for 3 16-bit mag offsets then 2 bytes for a 16-bit mag radius
				for (int i = 0; i < 8; i++)
					buf[i] = file->read();
				writeLen(MAG_OFFSET_X_LSB_ADDR, (const uint8_t*)buf, 6);
				writeLen(MAG_RADIUS_LSB_ADDR, (const uint8_t*)(buf + 6), 2);
				DEBUG_PRINTF("Loaded magnetometer calibration data.\n");
			}
			file->close();
		}

		strcpy(cal_fn, caldir);
		strcat(cal_fn, "gyro");

		if (fsexists(cal_fn)) {
			// load gyroscope calibrations
			file = fsopen(cal_fn);
			if (file->size() == 6) {
				// FORMAT: 6 bytes for 3 16-bit gyro offsets
				for (int i = 0; i < 6; i++)
					buf[i] = file->read();
				writeLen(GYRO_OFFSET_X_LSB_ADDR, (const uint8_t*)buf, 6);
				DEBUG_PRINTF("Loaded gyroscope calibration data.\n");
				have_gyro_cal = true;
			}
			file->close();
		}
	}

	/* Set the requested operating mode (see section 3.3) */
	write8(MODE_REG, 0x0C);

	configurator_lock = false;
}


bool BNO055::isMagCalibrated() {
	if (configurator_lock) return false;

	uint8_t cal_state = read8(CAL_STATE_REG);
	return ((cal_state >> 0) & 0x03) == 3;
}

bool BNO055::isAccelCalibrated() {
	if (configurator_lock) return false;

	uint8_t cal_state = read8(CAL_STATE_REG);
	return ((cal_state >> 2) & 0x03) == 3;
}

bool BNO055::isGyroCalibrated() {
	if (configurator_lock) return false;

	uint8_t cal_state = read8(CAL_STATE_REG);
	return ((cal_state >> 4) & 0x03) == 3;
}

void BNO055::update() {
	char cal_fn[36];
	uint8_t buf[8];
	NTIOSFile* file;

	if (update_timer > millis() || configurator_lock)
		return;
	update_timer = millis() + 50;

	if (!sensor_detected) {

		/* Make sure we have the right device */
		uint8_t id = read8(DEV_ID_REG);

		if (id != BNO055_ID) {
			// ERROR!
			// If it didn't finish booting or isn't plugged in yet, then we'll find it later.
			sensor_detected = false;
			update_timer = millis() + 500;
		} else {
			configure();
			sensor_detected = true;
			DEBUG_PRINTF("BNO055 configured.\n");
		}
		have_mag_cal = isMagCalibrated();
		have_acc_cal = isAccelCalibrated();
		//have_gyro_cal = isGyroCalibrated();
	} else {
		//bool acc = isAccelCalibrated();
		//bool mag = isMagCalibrated();
		bool gyro = isGyroCalibrated();

		if (gyro && !have_gyro_cal) {
			if (!fsexists(caldir))
				fsmkdir(caldir);

			strcpy(cal_fn, caldir);
			strcat(cal_fn, "gyro");

			write8(MODE_REG, 0x00);
			readLen(GYRO_OFFSET_X_LSB_ADDR, buf, 6);
			write8(MODE_REG, 0x0C);
			file = fsopen(cal_fn, NTIOS_WRITE);
			if (file) {
				// FORMAT: 6 bytes for 3 16-bit accel offsets then 2 bytes for a 16-bit accel radius
				for (int i = 0; i < 6; i++)
					file->write(buf[i]);
				file->close();
			}
			DEBUG_PRINTF("Saved new gyro calibration data.\n");
			have_gyro_cal = true;
		}
	}
}

bool BNO055::saveAccelCal() {
	char cal_fn[36];
	uint8_t buf[8];
	NTIOSFile* file;

	if (isAccelCalibrated()) {
		if (!fsexists(caldir))
			fsmkdir(caldir);

		strcpy(cal_fn, caldir);
		strcat(cal_fn, "acc");

		write8(MODE_REG, 0x00);
		readLen(ACCEL_OFFSET_X_LSB_ADDR, buf, 6);
		readLen(ACCEL_RADIUS_LSB_ADDR, buf + 6, 2);
		write8(MODE_REG, 0x0C);
		file = fsopen(cal_fn, NTIOS_WRITE);
		if (file) {
			// FORMAT: 6 bytes for 3 16-bit accel offsets then 2 bytes for a 16-bit accel radius
			for (int i = 0; i < 8; i++)
				file->write(buf[i]);
			file->close();
			return true;
		}
	}
	return false;
}


bool BNO055::getQuatAttitude(Quaternion& quat) {
	uint8_t buffer[8];
	int16_t x, y, z, w;

	if (configurator_lock || !sensor_detected) return false;

	/* Read quat data (8 bytes) */
	if (!readLen(QUATERNION_REG, buffer, 8)) {
		//sensor_detected = false;
		return false;
	}

	w = (((uint16_t)buffer[1]) << 8) | ((uint16_t)buffer[0]);
	x = (((uint16_t)buffer[3]) << 8) | ((uint16_t)buffer[2]);
	y = (((uint16_t)buffer[5]) << 8) | ((uint16_t)buffer[4]);
	z = (((uint16_t)buffer[7]) << 8) | ((uint16_t)buffer[6]);

	/*!
	* Assign to Quaternion
	* See
	* http://ae-bst.resource.bosch.com/media/products/dokumente/bno055/BST_BNO055_DS000_12~1.pdf
	* 3.6.5.5 Orientation (Quaternion)
	*/
	const double scale = (1.0 / (1 << 14));
	quat = Quaternion(scale * w, scale * x, scale * y, scale * z);
	return true;
}

bool BNO055::getRelativeAcceleration(Vector3& vec) {
	uint8_t buffer[6];
	int16_t x, y, z;

	if (configurator_lock || !sensor_detected) return false;

	/* Read vector data (6 bytes) */
	if (!readLen(LINEARACCEL_REG, buffer, 6)) {
		//sensor_detected = false;
		return false;
	}

	// Invert Y, NTIOS standards use a different axes layout than the BNO055
	x = ((int16_t)buffer[0]) | (((int16_t)buffer[1]) << 8);
	y = -(((int16_t)buffer[2]) | (((int16_t)buffer[3]) << 8));
	z = ((int16_t)buffer[4]) | (((int16_t)buffer[5]) << 8);

	vec = Vector3(x, y, z) / 100.0;
	internal_logger.publishVector(acc_i, vec);
	return true;
}

bool BNO055::getGyroEuler(Vector3& vec) {
	uint8_t buffer[6];
	int16_t x, y, z;

	if (configurator_lock || !sensor_detected) return false;

	/* Read vector data (6 bytes) */
	if (!readLen(GYRO_REG, buffer, 6)) {
		//sensor_detected = false;
		return false;
	}

	x = ((int16_t)buffer[0]) | (((int16_t)buffer[1]) << 8);
	y = ((int16_t)buffer[2]) | (((int16_t)buffer[3]) << 8);
	z = ((int16_t)buffer[4]) | (((int16_t)buffer[5]) << 8);

	vec = Vector3(x, y, z) / 100.0;
	return true;
}


uint8_t BNO055::read8(uint8_t adr) {
	uint8_t val;
	i2c.lock();
	i2c.write(bno055_adr, 1, (char*)&adr);
	i2c.read(bno055_adr, 1, (char*)&val);
	i2c.unlock();
	return val;
}

bool BNO055::readLen(uint8_t adr, uint8_t *buffer, uint8_t len) {
	bool val = false;
	i2c.lock();
	if (i2c.write(bno055_adr, 1, (char*)&adr))
		val = i2c.read(bno055_adr, len, (char*)buffer);
	i2c.unlock();
	return val;
}

bool BNO055::write8(uint8_t adr, uint8_t value) {
	bool val;
	char buf[2] = {adr, value};

	i2c.lock();
	val = i2c.write(bno055_adr, 2, (const char*)buf);
	i2c.unlock();
	return val;
}

bool BNO055::writeLen(uint8_t adr, const uint8_t *buffer, uint8_t len) {
	bool val = true;
	char buf[2] = {adr, 0};

	i2c.lock();
	for (int i = 0; i < len; i++) {
		buf[1] = buffer[i];
		val = val && i2c.write(bno055_adr, 2, (const char*)buf);
		buf[0]++;
	}
	i2c.unlock();
	return val;
}

