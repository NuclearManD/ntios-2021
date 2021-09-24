
#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define CODE_OFFSET	0x00000000
#define RAM_OFFSET	0x00800000
#define MAX_RAM_SIZE RAM_OFFSET

extern uint8_t* global_code;
extern uint8_t* global_ram;

#define DECODE_ADR(x) (((x) < RAM_OFFSET) ? (global_code + (x)) : (global_ram + ((x) - RAM_OFFSET)))

void setup_memory(void* code, uint32_t ram_size);

char* adr_to_str(int64_t address);

int8_t	ram_read_byte(uint64_t address);
int16_t	ram_read_word(uint64_t address);
int32_t	ram_read_dword(uint64_t address);
int64_t	ram_read_qword(uint64_t address);
int64_t	ram_read_size(uint64_t address, char size);

void	ram_write_byte(uint64_t address, int8_t data);
void	ram_write_word(uint64_t address, int16_t data);
void	ram_write_dword(uint64_t address, int32_t data);
void	ram_write_qword(uint64_t address, int64_t data);
void	ram_write_size(uint64_t address, int64_t data, char size);

#endif
