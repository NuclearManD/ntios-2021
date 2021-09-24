
#ifndef DRIVERS_H
#define DRIVERS_H

#include <stdbool.h>
#include <stdint.h>

#include "Stream.h"

/*
 * Basic device classes can be found here.
 * All type macros declared here.
 * Some device classes in other files to prevent clutter.
*/
 
// can add more here later
#define NTIOS_READ 1
#define NTIOS_WRITE 2
#define NTIOS_APPEND 4

#define ERR_NO_DEVICE -10
#define ERR_NO_DIR -11
#define ERR_NOT_FS -12
#define ERR_NOT_MOUNTED -13
#define ERR_UNKNOWN -14
#define ERR_NOT_CONNECTED -15
#define ERR_UNSUPPORTED -16
#define ERR_OUT_OF_BOUNDS -17

#define DEV_TYPE_STREAM 0x100
#define DEV_TYPE_FILESYSTEM 0x200
#define DEV_TYPE_GPIO 0x300
#define DEV_TYPE_BUS  0x380
#define DEV_TYPE_NETWORK 0x400
#define DEV_TYPE_USB_HUB 0x500
#define DEV_TYPE_NAVIGATION 0x600
#define DEV_TYPE_IMU 0x680
#define DEV_TYPE_AUDIO_IN 0x700
#define DEV_TYPE_AUDIO_OUT 0x780
#define DEV_TYPE_ACTUATOR 0x800
#define DEV_TYPE_CONTROL_SYS 0x880
#define DEV_TYPE_TOUCH_PANEL 0x900

#define DEV_TYPE_KEYBOARD 0x101
#define DEV_TYPE_GRAPHICS 0x102
#define DEV_TYPE_SERIAL 0x103
#define DEV_TYPE_JOINED_STREAM 0x104

#define DEV_TYPE_PWM_PIN 0x301

#define DEV_TYPE_BUS_I2C 0x380
#define DEV_TYPE_BUS_SPI 0x381
#define DEV_TYPE_BUS_CAN 0x382

#define DEV_TYPE_WIFI 0x420
#define DEV_TYPE_CELLULAR 0x440
#define DEV_TYPE_ETHERNET 0x460

#define DEV_TYPE_GPS_RAW 0x601
#define DEV_TYPE_POS_KALMAN 0x602

//#define DEV_TYPE_IMU_xx 0x681

#define DEV_TYPE_CONTROL_SYS_SYMMETRIC 0x881

#define DEV_TYPE_CAPACITIVE_TOUCH 0x901

#define GPIO_PIN_MODE_INPUT 0
#define GPIO_PIN_MODE_OUTPUT 1
#define GPIO_PIN_MODE_INPUT_PULLUP 2
#define GPIO_PIN_MODE_INPUT_PULLDOWN 3
#define GPIO_PIN_MODE_HIGH_Z 4

const char* devTypeToStr(int type);

class Device {
public:
	virtual const char* getName() = 0;
	int sendCommand(int command, char* buffer);
	virtual int getType() = 0;
	virtual void update() {};
};

class StreamDevice: public Device, public Stream {
public:
	int getType();
};

class NTIOSFile: public Stream {
public:
	virtual size_t write(uint8_t d) = 0;
	virtual size_t write(const uint8_t *buf, size_t size) = 0;
	virtual int availableForWrite() = 0;
	virtual int read() = 0;
	virtual int peek() = 0;
	virtual int available() = 0;
	virtual void flush() = 0;
	virtual int read(void *buf, uint16_t nbyte) = 0;
	virtual bool seek(uint32_t pos) = 0;
	virtual uint32_t position() = 0;
	virtual uint32_t size() = 0;
	virtual void close() = 0;
	virtual operator bool() = 0;
	virtual char* name() = 0;

	virtual bool isDirectory(void) = 0;
	virtual NTIOSFile* openNextFile(uint8_t mode = NTIOS_READ) = 0;
	virtual void rewindDirectory(void) = 0;

	virtual ~NTIOSFile() = 0;

	using Print::write;
};

class FileSystemDevice: public Device {
public:
	int getType();
	virtual void mount() = 0;
	virtual void unmount() = 0;
	virtual NTIOSFile* open(const char *filename, uint8_t mode = NTIOS_READ) = 0;
	NTIOSFile* open(const String &filename, uint8_t mode = NTIOS_READ) {
		return open(filename.c_str(), mode);
	}
	// Methods to determine if the requested file path exists.
	virtual bool exists(const char *filepath) = 0;
	bool exists(const String &filepath) {
		return exists(filepath.c_str());
	}

	// Create the requested directory heirarchy--if intermediate directories
	// do not exist they will be created.
	virtual bool mkdir(const char *filepath) = 0;
	bool mkdir(const String &filepath) {
		return mkdir(filepath.c_str());
	}

	// Delete the file.
	virtual bool remove(const char *filepath) = 0;
	bool remove(const String &filepath) {
		return remove(filepath.c_str());
	}

	virtual bool rmdir(const char *filepath) = 0;
	bool rmdir(const String &filepath) {
		return rmdir(filepath.c_str());
	}
};

class I2CBusDevice: public Device {
public:
	int getType() { return DEV_TYPE_BUS_I2C; };


	virtual bool read(int address, int size, char* data) = 0;

	// This will try to read up to `size` bytes.  Returns the number of bytes actually read.
	virtual int readSome(int address, int size, char* data) = 0;

	virtual bool write(int address, int size, const char* data) = 0;

	// Lock/unlock an internal mutex
	virtual void lock() = 0;
	virtual void unlock() = 0;
};

// Note about the SPI bus:  it is expected that a GPIO device be used to assert the CS line.
// This SPI driver API does NOT provide CS control!
class SPIBusDevice: public Device {
public:
	int getType() { return DEV_TYPE_BUS_SPI; };

	// writes out to the SPI bus while reading to in
	virtual void exchange(int size, const char* out, char* in) = 0;

	// repeates out to the SPI bus while reading to in
	virtual void exchange(int size, char out, char* in) = 0;

	virtual void lock() = 0;
	virtual void unlock() = 0;
};

class GPIODevice: public Device {
public:
	int getType() { return DEV_TYPE_GPIO; }

	void update() {} // GPIO usually doesn't need an update function

	// Returns the number of pins on the device
	virtual int pinCount() = 0;

	// Sets the mode of the pin specified.  Returns true on success.
	// On failure: mode is unchanged and function returns false.
	virtual bool pinMode(int pin, int mode) = 0;

	// Returns true if the pin is HIGH and false if the pin is LOW.
	virtual bool readPin(int pin) = 0;

	// Attempt to write to a pin,  Returns true on success, false on error.
	virtual bool writePin(int pin, bool state) = 0;
};

class SerialDevice: public StreamDevice {
public:
	int getType() { return DEV_TYPE_SERIAL; }
	
	virtual void setBaud(uint32_t baud) = 0;
};

class KeyboardDevice: public StreamDevice {
public:

  // Print implementation
	size_t write(uint8_t val);
	using Print::write; // pull in write(str) and write(buf, size) from Print
	void flush();
	int getType();

	virtual int setOnPress(void (*event)(KeyboardDevice*, int)) { (void)event; return ERR_UNSUPPORTED; };
	virtual int setOnRelease(void (*event)(KeyboardDevice*, int)) { (void)event; return ERR_UNSUPPORTED; };
};

class JoinedStreamDevice: public StreamDevice {
private:
	StreamDevice* input;
	StreamDevice* output;
	char* name;
public:
	JoinedStreamDevice(StreamDevice* in, StreamDevice* out);
	
	size_t write(uint8_t val);
	using Print::write; // pull in write(str) and write(buf, size) from Print
	void flush();
	int getType();
	const char* getName();

	int read();
	int available();
	int peek();
	
	void update();
	
	StreamDevice* getOutput();
	StreamDevice* getInput();
};

class PWMPin: public Device {
public:
	int getType() { return DEV_TYPE_PWM_PIN; };

	virtual void setMicros(long us) = 0;
	virtual void setDuty(float duty) = 0;
	virtual void setFreq(double freq) = 0;
};

class TCPConnection: public Stream {
public:
	using Print::write; // pull in write(str) and write(buf, size) from Print
	
	virtual void close() = 0;
	virtual bool isClosed() = 0;
};

class NetworkDevice: public Device {
public:
	int getType() { return DEV_TYPE_NETWORK; };
	virtual TCPConnection* connect(const char* host, int port);
};

class WiFiDevice: public NetworkDevice {
public:
	int getType() { return DEV_TYPE_WIFI; };
	virtual bool isConnected() = 0;
	virtual int associate(const char* ssid, const char* password = "") = 0;
	virtual bool disassociate() = 0;

	virtual int scanNetworks() = 0;
	virtual const char* netName(int idx) = 0;
	virtual const uint8_t* netMac(int idx) = 0; // 6-byte array for MAC
	virtual int netChannel(int idx) = 0;

	virtual int deauth(const uint8_t* mac, int ch) { (void) mac; (void) ch; return ERR_UNSUPPORTED; }
	virtual int undeauth(const uint8_t* mac) { (void) mac; return ERR_UNSUPPORTED; }

	virtual int fakeAP(const char* name) { (void) name; return ERR_UNSUPPORTED; }
	virtual int unfakeAP(const char* name) { (void) name; return ERR_UNSUPPORTED; }
};

#if defined(TEENSYDUINO)
class AudioInputDevice: public Device {
public:
	int getType() { return DEV_TYPE_AUDIO_IN; };
};

class AudioOutputDevice: public Device {
public:
	int getType() { return DEV_TYPE_AUDIO_OUT; };

	virtual int setVolume(float volume) = 0;
	virtual AudioStream& getAudioStream() = 0;
};
#endif // teensy

#endif
