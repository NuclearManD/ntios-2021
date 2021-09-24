
#ifndef CELLULAR_H
#define CELLULAR_H

#include "../drivers.h"

#define CELLULAR_UART_BUFFER_SIZE 256

#if CELLULAR_UART_BUFFER_SIZE < 32
	#warning "The cellular driver may start malfunctioning if CELLULAR_UART_BUFFER_SIZE is less than 32."
#endif

class CellularDevice: public NetworkDevice {
public:
	int getType() { return DEV_TYPE_CELLULAR; };
	virtual bool isConnected() = 0;
	virtual void disconnect() = 0;
	
	// Connects to cell network
	// THIS IS A BLOCKING FUNCTION.
	// Some devices require a time-intensive setup sequence taking several
	// seconds.  connect() returns when either the connection is established
	// or when the device fails to connect.
	// Returns true if the connection succeeds, false otherwise.
	virtual bool connect() = 0;
	
	// This will call the given phone number.
	// THIS IS A BLOCKING FUNCTION.
	// Returns true if the call is answered, false otherwise.
	virtual bool call(const char* number) = 0;

	// Will hang up the phone.  Not a blocking function.
	virtual void hangup() = 0;

	// Returns true if the device is receiving a call, false otherwise.
	virtual bool callAvailable() = 0;

	// Returns the number of the phone calling this device, if it is being called.
	// Calling this function when callAvailable() returns false creates undefined behavior.
	virtual const char* getCallerNumber() = 0;

	// Returns the caller ID of the phone calling this device, if it is being called.
	// Calling this function when callAvailable() returns false creates undefined behavior.
	virtual const char* getCallerID() = 0;

	// Returns the number of this device, if it is connected.
	// Calling this function when isConnected() returns false creates undefined behavior.
	virtual const char* getDevicePhoneNumber() = 0;

	// Returns the ip address of this device, if it is connected.
	// Calling this function when isConnected() returns false creates undefined behavior.
	virtual const char* getIPAddress() = 0;

	virtual bool sendTextMessage(const char* number, const char* message) = 0;
	//virtual bool textMessageAvailable() = 0;
	//virtual TextMessage readTextMessage() = 0;

	virtual float getConnectionQuality() { return -1; }
	// TODO: need way to get the audio I/O
};

class UARTCellularDevice: public CellularDevice {
public:

	UARTCellularDevice(StreamDevice* uart) : uart(*uart) {
		//this->uart = *uart;
	}

	//int getType() { return DEV_TYPE_UARTCELLULAR; };
	const char* getName() { return "UART Cellular Modem"; }
	void update();

	TCPConnection* connect(const char* host, int port);

	bool isConnected() { return is_connected; }
	void disconnect();
	bool connect();
	bool call(const char* number);
	void hangup();
	bool sendTextMessage(const char* number, const char* message);
	bool callAvailable() { return is_being_called; }
	bool isInCall() { return is_in_call; }
	const char* getCallerNumber() { return caller_number; }
	const char* getCallerID() { return caller_id; }
	const char* getDevicePhoneNumber() { return phone_number; }
	float getConnectionQuality() { return is_connected ? quality : -2; }
	const char* getIPAddress() { return ip_address; }

private:
	bool sendCommand(const char* fmt, ...);
	bool uartRead(long timeout_millis = 1200);

	StreamDevice& uart;
	bool is_being_called = false;
	bool is_in_call = false;
	bool is_connected = false;
	bool got_no_carrier = false;

	bool is_uart_busy = false;

	float quality = -1;

	char phone_number[12];
	char caller_number[12];

	// NOTE: This only supports IPv4!
	// I'm writing this code on an airplane and I don't know how long ipv6 addresses are! LMAO
	char ip_address[16];
	char caller_id[64];
	char uart_buffer[CELLULAR_UART_BUFFER_SIZE];

	int manufacturer;
};

#endif
