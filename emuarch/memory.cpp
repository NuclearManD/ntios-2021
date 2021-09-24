#include "Arduino.h"
#include <stdlib.h>
#include "memory.h"

uint8_t* global_code;
uint8_t* global_ram;

char* adr_to_str(int64_t address){
	return (char*)DECODE_ADR(address);
}

void setup_memory(void* code, uint32_t ram_size){
	global_ram = (uint8_t*)malloc(ram_size);
	global_code = (uint8_t*)code;
}

int8_t		ram_read_byte(uint64_t address){
	return DECODE_ADR(address)[0];
}

int16_t	ram_read_word(uint64_t address){
	int16_t data;
	uint8_t* ptr = DECODE_ADR(address);
	data = ptr[0] | (((uint16_t)ptr[1]) << 8);
	return data;
}

int32_t	ram_read_dword(uint64_t address){
	int32_t data;
	uint8_t* ptr = DECODE_ADR(address);
	data = ptr[0] | (((uint32_t)ptr[1]) << 8) | (((uint32_t)ptr[2]) << 16) | (((uint32_t)ptr[3]) << 24);
	return data;
}

int64_t	ram_read_qword(uint64_t address){
	int32_t data;
	uint8_t* ptr = DECODE_ADR(address);
	data = ptr[0] | (((uint64_t)ptr[1]) << 8) | (((uint64_t)ptr[2]) << 16) | (((uint64_t)ptr[3]) << 24);
	data |= (((uint64_t)ptr[4]) << 32) | (((uint64_t)ptr[5]) << 8);
	data |= (((uint64_t)ptr[6]) << 16) | (((uint64_t)ptr[7]) << 24);
	return data;
}

int64_t	ram_read_size(uint64_t address, char size){
	int64_t data;
	switch (size){
		case 0:
			data = ram_read_qword(address);
			break;
		case 1:
			data = ram_read_dword(address);
			break;
		case 2:
			data = ram_read_word(address);
			break;
		case 3:
			data = ram_read_byte(address);
			break;
		default:
			data = -1;
			break;
	}
	return data;
}


void	ram_write_byte(uint64_t address, int8_t data){
	*(int8_t*)DECODE_ADR(address) = data;
}

void	ram_write_word(uint64_t address, int16_t data){
	*(int16_t*)DECODE_ADR(address) = data;
}

void	ram_write_dword(uint64_t address, int32_t data){
	*(int32_t*)DECODE_ADR(address) = data;
}

void	ram_write_qword(uint64_t address, int64_t data){
	*(int64_t*)DECODE_ADR(address) = data;
}

void	ram_write_size(uint64_t address, int64_t data, char size){
	switch (size){
		case 0:
			*(int64_t*)DECODE_ADR(address) = data;
			break;
		case 1:
			*(int32_t*)DECODE_ADR(address) = data;
			break;
		case 2:
			*(int16_t*)DECODE_ADR(address) = data;
			break;
		case 3:
			*(int8_t*)DECODE_ADR(address) = data;
	}
}
