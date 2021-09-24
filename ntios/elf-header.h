
#pragma once

#include <stdint.h>


#define ELF_MAGIC "\177ELF"

#define BUS_WIDTH_32_BITS 1
#define BUS_WIDTH_64_BITS 2

#define ENDIANNESS_LITTLE 1
#define ENDIANNESS_BIG 2

#define HEADER_VERSION ???

#define OS_ABI_SYSTEM_V 0

#define TYPE_RELOCATABLE 1
#define TYPE_EXECUTABLE 2
#define TYPE_SHARED 3
#define TYPE_CORE 4

#define INSTRUCTION_SET_NONE 0x00
#define INSTRUCTION_SET_SPARC 0x02
#define INSTRUCTION_SET_X86 0x03
#define INSTRUCTION_SET_MIPS 0x08
#define INSTRUCTION_SET_POWERPC 0x14
#define INSTRUCTION_SET_ARM 0x28
#define INSTRUCTION_SET_SUPERH 0x2A
#define INSTRUCTION_SET_IA64 0x32
#define INSTRUCTION_SET_X64 0x3E
#define INSTRUCTION_SET_AARCH64 0xB7

#define PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMIC 2
#define PT_INTERP 3
#define PT_NOTE 4
#define PT_SHLIB 5
#define PT_PHDR 6

#ifdef __cplusplus
extern "C" {
#endif
	const char* instruction_set_name(int isa);
	void program_header_flags_to_str(char* dst, uint32_t flags);
#ifdef __cplusplus
}
#endif

typedef struct elf_header_s {
    char magic[4];
    uint8_t bus_width;
    uint8_t endianness;
    uint8_t header_version;
    uint8_t os_abi;
    char padding_8_15[8];
    uint16_t type;
    uint16_t instruction_set;
    uint16_t elf_version;
} elf_header_t;

typedef struct elf_header_32_s {
    char magic[4];
    uint8_t bus_width;
    uint8_t endianness;
    uint8_t header_version;
    uint8_t os_abi;
    char padding_8_15[8];
    uint16_t type;
    uint16_t instruction_set;
    uint16_t elf_version;
    uint32_t program_entry_position;
    uint32_t program_header_table_position;
    uint32_t section_header_table_position;
    uint32_t flags;
    uint16_t header_size;
    uint16_t program_header_table_entry_size;
    uint16_t program_header_table_entry_count;
    uint16_t section_header_table_entry_size;
    uint16_t section_header_table_entry_count;
    uint16_t string_table_index;
} elf_header_32_t;

typedef struct elf_header_64_s {
    char magic[4];
    uint8_t bus_width;
    uint8_t endianness;
    uint8_t header_version;
    uint8_t os_abi;
    char padding_8_15[8];
    uint16_t type;
    uint16_t instruction_set;
    uint16_t elf_version;
    uint64_t program_entry_position;
    uint64_t program_header_table_position;
    uint64_t section_header_table_position;
    uint32_t flags;
    uint16_t header_size;
    uint16_t program_header_table_entry_size;
    uint16_t program_header_table_entry_count;
    uint16_t section_header_table_entry_size;
    uint16_t section_header_table_entry_count;
    uint16_t string_table_index;
} elf_header_64_t;

typedef struct  {
    uint32_t segment_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    char padding[4];
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t flags;
    uint32_t alignment;
} elf_program_header_32_t;

typedef struct elf_program_header_64_s {
    uint32_t segment_type;
    uint32_t flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    char padding[4];
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t alignment;
} elf_program_header_64_t;

// Yup, you guessed it.  This is the point I discovered that elf.h exists.
typedef struct {
	uint32_t   sh_name;
	uint32_t   sh_type;
	uint32_t   sh_flags;
	uint32_t   sh_addr;
	uint32_t    sh_offset;
	uint32_t   sh_size;
	uint32_t   sh_link;
	uint32_t   sh_info;
	uint32_t   sh_addralign;
	uint32_t   sh_entsize;
} elf_section_header_32_t;

typedef struct {
	uint32_t   sh_name;
	uint32_t   sh_type;
	uint64_t   sh_flags;
	uint64_t   sh_addr;
	uint64_t   sh_offset;
	uint64_t   sh_size;
	uint32_t   sh_link;
	uint32_t   sh_info;
	uint64_t   sh_addralign;
	uint64_t   sh_entsize;
} elf_section_header_64_t;

