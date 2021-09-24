
#include "../ntios.h"
#include "cellular.h"

#include <stdarg.h>

#define CTRL_Z 26
#define UART_LOCK	while(is_uart_busy)yield();\
					is_uart_busy = true;

#define UART_UNLOCK	is_uart_busy = false;

#define UNKNOWN 0
#define UBLOX 1


/*
private:
	bool sendCommand(const char* fmt, ...);

	StreamDevice& uart;
	bool is_being_called = false;
	bool is_in_call = false;
	bool is_connected = false;
	bool got_no_carrier = false;

	bool is_uart_busy = false;

	float quality = -1;

	char phone_number[12];
	char caller_number[12];
	char called_id[64];
*/

void UARTCellularDevice::update() {
	uartRead();
}

#include "Arduino.h"

TCPConnection* UARTCellularDevice::connect(const char* host, int port) {
	if (manufacturer == UBLOX) {
		if (!sendCommand("AT+USOCR=6"))
			return NULL;
		if (strncmp(uart_buffer, "+USOCR: ", 8))
			return NULL;

		// Save socket ID
		int socket_id = atoi(&(uart_buffer[8]));

		// Get rid of the extra OK
		uartRead(100);
		Serial.println(socket_id);

		// Attempt a connection
		if (!sendCommand("AT+USOCO=%i,%s,%i", socket_id, host, port))
			if (!uartRead(10000))
				// No response in more than 10 seconds - assume our device died.
				return NULL;

		if (uartRead(100))
			if (strncmp(uart_buffer, "+UUSOCO: ", 9)) {
				// Failed to connect.  Delete the socket.
				sendCommand("AT+USOCL=%i,1", socket_id);
				return NULL;
			}

		//UARTCellularSocket* socket = new UARTCellularSocket(this, 
	}
	return NULL;
}

void UARTCellularDevice::disconnect() {
	if (!is_connected) return;
	is_connected = false;
	// TODO: Proper cleanup
}

bool UARTCellularDevice::connect() {
	UART_LOCK

	// clear buffer of junk data
	while (uart.available()) uart.read();

	for (int i = 0; i < 10; i++) {
		if (sendCommand("AT")) {
			Serial.println(uart_buffer);
			if (strcmp(uart_buffer, "OK") == 0)
				break;
		}
		else
			Serial.println("[no reply]");

		if (i==9) {
			is_connected = false;
			UART_UNLOCK
			return false;
		}
	}

	Serial.println("Device detected");
	Serial.flush();

	// We checked the serial link, but we already connected, so assume it still works.
	// To force reconnection, call disconnect() first.
	if (is_connected)
		return true;

	sendCommand("ATE0");

	// clear buffer of junk data (again)
	while (uart.available()) uart.read();

	// Check the manufacturer so we can use the right commands
	if (sendCommand("AT+CGMI")) {
		if (!strcmp(uart_buffer, "u-blox")) {
			manufacturer = UBLOX;
		} else {
			manufacturer = UNKNOWN;
		}

		uartRead(100); // Clear out the OK that comes after the manufacturer
	}

	if (manufacturer == UNKNOWN) {
		int cnt = 0;
		while (true) {
			if (sendCommand("AT+CIPSHUT"))
				if (strcmp(uart_buffer, "SHUT OK"))
					break;
			if(cnt==3) {
				is_connected = false;
				UART_UNLOCK
				return false;
			}
			cnt++;
		}
		cnt = 0;
		sendCommand("AT+CIPMUX=0");
		while (true) {
			if (sendCommand("AT+CIPSTATUS"))
				if (strcmp(uart_buffer, "IP INITIAL")) // FYI this is wrong
					break;
			if(cnt == 15) {
				is_connected = false;
				UART_UNLOCK
				return false;
			}
			cnt++;
		}
		cnt = 0;
		if ((!sendCommand("AT+CSTT= \"hologram\", \"\", \"\"")) || 0 != strcmp(uart_buffer, "OK")) {
			is_connected = false;
			UART_UNLOCK
			return false;
		}
		while (true) {
			if (sendCommand("AT+CIPSTATUS"))
				if (strcmp(uart_buffer, "IP START")) // FYI this is wrong
					break;
			if(cnt == 15) {
				is_connected = false;
				UART_UNLOCK
				return false;
			}
			cnt++;
		}
		cnt = 0;
		sendCommand("AT+CIICR");
		while (true) {
			if (sendCommand("AT+CIPSTATUS"))
				if (strcmp(uart_buffer, "ACTIVE")) // FYI this is wrong
					break;
			if(cnt == 15) {
				is_connected = false;
				UART_UNLOCK
				return false;
			}
			cnt++;
		}
		cnt = 0;
		//sendCommand("AT+CIPSHUT");
		if (sendCommand("AT+CIFSR")) {
			strncpy(ip_address, uart_buffer, 15);
			ip_address[15] = 0;
		} else {
			strcpy(ip_address, "Unknown");
		}

		// set communication parameters
		sendCommand("AT+CIPHEAD=1");
	} else if (manufacturer == UBLOX) {
		
	}

	//gprsClean();
	UART_UNLOCK
	is_connected = true;
	return true;
}

bool UARTCellularDevice::call(const char* number) {
	if (!is_connected) return false;
	UART_LOCK
	sendCommand("ATD + +%s;", number);
	UART_UNLOCK
}

void UARTCellularDevice::hangup() {
	if (!is_connected) return;
	UART_LOCK
	sendCommand("ATH");
	UART_UNLOCK
}

bool UARTCellularDevice::sendTextMessage(const char* number, const char* message) {
	const char* check_str = message;

	// Must be connected to the cell network before a text can be sent.
	if (!is_connected) return false;

	// Do not allow transmission of empty message
	if (*check_str == 0) return false;

	while (*check_str) {
		// Do not allow messages that have CTRL_Z (ASCII 26) in them
		if (*check_str == CTRL_Z) return false;
		check_str++;
	}

	UART_LOCK
	sendCommand("AT+CMGF=1");
	uart.println("AT+CMGS=\"+%s\"");
	delay(100);
	uart.println(message);
	uart.write(CTRL_Z);
	UART_UNLOCK
}

bool UARTCellularDevice::uartRead(long timeout_millis) {
	int i = 0;
	bool is_reading_yet = false;

	while (true) {
		if (!uart.available()) {
			unsigned long timer = millis() + timeout_millis;
			while (!uart.available()) {
				if (timer < millis()) return false;
				yield();
			}
		}
		char c = uart.read();
		if (c=='\r' || c=='\n'/* || (c==':' && result.startsWith("+IPD"))*/) {
			if (is_reading_yet) {
				uart_buffer[i] = 0;
				return true;
			}
		}else{
			uart_buffer[i++] = (char)c;
			is_reading_yet = true;
			if (i + 1 == CELLULAR_UART_BUFFER_SIZE)
				return true;
		}
	}
}

bool UARTCellularDevice::sendCommand(const char* fmt, ...) {
	va_list ap;
	int i = 0;

	va_start(ap, fmt);
	Serial.println(fmt);
	Serial.flush();
	while (fmt[i]) {
		if (fmt[i] == '%' && fmt[i + 1] == 's') {
			uart.write(fmt, (size_t)i);
			uart.print(va_arg(ap, const char*));
			fmt = &(fmt[i + 2]);
			i = 0;
		} else
			i++;
	}
	uart.print(fmt);
	va_end(ap);
	uart.print("\r\n");
	uart.flush();

	return uartRead(1200);
}
