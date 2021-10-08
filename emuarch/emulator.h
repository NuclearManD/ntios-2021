
#ifndef EMULATOR_H
#define EMULATOR_H

#include <stdint.h>

#include "ntios.h"
#include "emu.h"

// I am not yet implementing support for floats
#define EMULATOR_FEATURES 0x00000

#define PC	reg_set_0[7]
#define SP	reg_set_0[6]
#define SI	reg_set_0[4]
#define DI	reg_set_0[5]
#define CNT	reg_set_1[6]
#define CR0	reg_set_1[7]

#define ERROR_INVALID_INSTRUCTION -1121

t_emuarch_cpu* make_cpu(int64_t pc, int64_t sp);
void cleanup_cpu(t_emuarch_cpu* cpu);

int64_t pop_qword(t_emuarch_cpu* cpu);
int64_t pop_size(t_emuarch_cpu* cpu, uint8_t size);
void push_size(t_emuarch_cpu* cpu, int64_t data, uint8_t size);
void push_qword(t_emuarch_cpu* cpu, int64_t data);

void write_reg(t_emuarch_cpu* cpu, uint8_t size, uint8_t regid, int64_t data);

void throw_exception(t_emuarch_cpu* cpu, uint16_t exception_id);
void call_immediate(t_emuarch_cpu* cpu, uint64_t function, StreamDevice* io);

int step(t_emuarch_cpu* cpu, StreamDevice* io);
void run(t_emuarch_cpu* cpu, StreamDevice* io);

#endif
