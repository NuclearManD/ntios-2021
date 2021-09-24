
#include "elf-header.h"
#include "exec.h"
#include "ntios.h"

#include <stdint.h>

#define EXPECTED_BUS_WIDTH BUS_WIDTH_32_BITS
#define EXPECTED_ELF_HEADER_T elf_header_32_t
#define EXPECTED_PROGRAM_HEADER_T elf_program_header_32_t
#define EXPECTED_SECTION_HEADER_T elf_section_header_32_t
#define BUS_SIZE_T uint32_t

#if CPU_ISA == ARM32
	#define EXPECTED_ISA INSTRUCTION_SET_ARM
#else
	#define CANNOT_LOAD_ELF
#endif

#ifdef __IMXRT1062__
	#define STATIC_ELF_BUFFER 0x20000
#endif


#ifdef STATIC_ELF_BUFFER
// The type of BS we need to do when we don't have virtualization.  I seriously tried.
// Seems the T4.1 doesn't have the coprocessor needed.
__attribute__ ((used, aligned(65536)))
uint8_t _ntios_elf_buffer[STATIC_ELF_BUFFER];
#endif



int get_bus_width(NTIOSFile* file) {
	uint8_t buffer[5];

	file->seek(0);
	file->read(buffer, 5);

	if (memcmp((char*)buffer, ELF_MAGIC, 4))
		return -1;

	return buffer[4];
}

#ifndef CANNOT_LOAD_ELF

int _ntios_dynamic_link_elf(StreamDevice* io, uint8_t* elf_ram_loc, NTIOSFile* file, EXPECTED_ELF_HEADER_T& elf_header) {
	
	(void)elf_ram_loc;

	// Step one: load the string table

	uint32_t section_header_pos = elf_header.section_header_table_position;
	uint32_t section_header_size = elf_header.section_header_table_entry_size;
	uint32_t string_table_offset = elf_header.string_table_index * section_header_size;
	uint32_t string_table_header_offset = section_header_pos + string_table_offset;
	EXPECTED_SECTION_HEADER_T section_header;
	char* string_table;

	file->seek(string_table_header_offset);
	file->read(&section_header, sizeof(EXPECTED_SECTION_HEADER_T));

	string_table = (char*)malloc(section_header.sh_size);
	file->seek(section_header.sh_offset);
	file->read(string_table, section_header.sh_size);

	// Step 2: read the sections to get:
	//          1. the GOT location
	//          2. ????

	io->println("Sections:");
	for (int i = 0; i < elf_header.section_header_table_entry_count; i++) {
		file->seek(section_header_pos + i * section_header_size);
		file->read(&section_header, sizeof(EXPECTED_SECTION_HEADER_T));
		char* section_name = &(string_table[section_header.sh_name]);

		if ((!strcmp(section_name, ".got")) || (!strcmp(section_name, ".rel.plt")) || (!strcmp(section_name, ".dynamic"))) {
			io->printf("  % 2i: % -20s (%i bytes)\n", i, section_name, section_header.sh_size);
			io->printf("    : vaddr   = 0x%08x\n", section_header.sh_addr);
			io->printf("    : offset  = 0x%08x\n", section_header.sh_offset);
			
		}
	}

	free(string_table);
	return 0;
}

int _ntios_do_file_exec(StreamDevice* io, NTIOSFile* file, int argc, char** argv) {
	(void)argc;
	(void)argv;

	int bus_width = get_bus_width(file);

	if (bus_width == -1) {
		io->println("ELF file is not valid.");
		return 10255;
	}

	if (bus_width != EXPECTED_BUS_WIDTH) {
		io->println("ELF file is not compatible with this processor (bus width mismatch).");
		return 10256;
	}

	EXPECTED_ELF_HEADER_T elf_header;
	file->seek(0);
	file->read(&elf_header, sizeof(EXPECTED_ELF_HEADER_T));

	if (EXPECTED_ISA != elf_header.instruction_set) {
		io->println("ELF file is not compatible with this processor (ISA mismatch).");
		io->printf("Needed %s, got %s.\n", EXPECTED_ISA, instruction_set_name(elf_header.instruction_set));
		return 10256;
	}

	if (elf_header.os_abi != 0) {
		io->println("Unknown ELF ABI (not SystemV), cannot execute.");
		return 10256;
	}

	if (elf_header.endianness == ENDIANNESS_BIG) {
		io->println("ELF endianness is big endian, which is not supported.");
		return 10256;
	}

	// Oh god, now we're actually gonna load it.
	EXPECTED_PROGRAM_HEADER_T program_header;
	BUS_SIZE_T program_header_offset = elf_header.program_header_table_position;
	BUS_SIZE_T lowest_start = 0xFFFFFFFF;
	BUS_SIZE_T highest_end = 0;
	for (int i = 0; i < elf_header.program_header_table_entry_count; i++) {
		file->seek(program_header_offset);
		file->read(&program_header, sizeof(EXPECTED_PROGRAM_HEADER_T));
		if (program_header.segment_type == PT_LOAD) {
			// The segment can be loaded
			io->printf("PT_LOAD 0x%08x -> 0x%08x (%i bytes)\n", program_header.p_offset, program_header.p_vaddr, program_header.p_filesz);
			if (program_header.p_vaddr < lowest_start)
				lowest_start = program_header.p_vaddr;
			if (program_header.p_vaddr + program_header.p_memsz > highest_end)
				highest_end = program_header.p_vaddr + program_header.p_memsz;
		}
		program_header_offset += elf_header.program_header_table_entry_size;
	}

	BUS_SIZE_T msize = highest_end - lowest_start;
	io->printf("Address space size: 0x%08x bytes (%i KiB)\n", msize, msize / 1024);
	io->printf("Lowest start: 0x%08x\n", lowest_start);
	io->printf("Highest end:  0x%08x\n", highest_end);
	io->printf("Entry point:  0x%08x\n", elf_header.program_entry_position);

	uint8_t* elf_ram_location;

	#ifdef STATIC_ELF_BUFFER
		if (msize <= STATIC_ELF_BUFFER) {
			// Load into the buffer - we have room
			// Go through the list of program headers again
			program_header_offset = elf_header.program_header_table_position;
			for (int i = 0; i < elf_header.program_header_table_entry_count; i++) {
				file->seek(program_header_offset);
				file->read(&program_header, sizeof(EXPECTED_PROGRAM_HEADER_T));
				if (program_header.segment_type == PT_LOAD) {
					file->seek(program_header.p_offset);
					uint8_t* memory_loc = &(_ntios_elf_buffer[program_header.p_vaddr - lowest_start]);
					file->read(memory_loc, program_header.p_filesz);
				}
				program_header_offset += elf_header.program_header_table_entry_size;
			}
			elf_ram_location = _ntios_elf_buffer;
		} else {
			// We don't have enough allocated RAM.  Abort.
			io->printf("ELF memory requirements too large.\n");
			return 10257;
		}
	#endif

	// Invoke the dynamic linker
	int res = _ntios_dynamic_link_elf(io, elf_ram_location, file, elf_header);
	if (res != 0) {
		io->println("Dynamic linking failed.");
		return res;
	}

	return 0;
}


#else

int _ntios_do_file_exec(StreamDevice* io, NTIOSFile* file, int argc, char** argv) {
	(void)file;
	(void)argc;
	(void)argv;
	io->println("NTIOS does not support ELF loading for this architecture.");
	return 10256;
}

#endif

