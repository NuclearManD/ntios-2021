
#ifndef EMU_H
#define EMU_H

//#include "EmuArch.h"
#include "drivers.h"

#define MEMORY_SIZE 32768

#define CODE_OFFSET	0x00000000
#define RAM_OFFSET	0x00800000
#define MAX_RAM_SIZE RAM_OFFSET

extern uint8_t* global_code;
extern uint8_t* global_ram;

int launch_emu(StreamDevice* io, uint8_t* code, int argc, char** argv);

void setup_memory(void* code, uint32_t ram_size);

typedef struct s_emuarch_cpu{
	int64_t	reg_set_0[8];
	int32_t	reg_set_1[8];
	float	reg_set_2[8];
	uint64_t total_operations;

	NTIOSFile** files;
	int num_files;
	TCPConnection** tcps;
	int num_tcps;

}	t_emuarch_cpu;

t_emuarch_cpu* make_cpu(int64_t pc, int64_t sp);
void cleanup_cpu(t_emuarch_cpu* cpu);

int step(t_emuarch_cpu* cpu, StreamDevice* io);
void run(t_emuarch_cpu* cpu, StreamDevice* io);

#endif
