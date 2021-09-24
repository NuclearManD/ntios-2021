
#include "elf-header.h"


const char* instruction_set_name(int isa) {
	if (isa == INSTRUCTION_SET_NONE) {
		return "[none]";
	} else if (isa == INSTRUCTION_SET_AARCH64) {
		return "AArch64";
	} else if (isa == INSTRUCTION_SET_ARM) {
		return "ARM";
	} else if (isa == INSTRUCTION_SET_IA64) {
		return "IA64";
	} else if (isa == INSTRUCTION_SET_MIPS) {
		return "MIPS";
	} else if (isa == INSTRUCTION_SET_POWERPC) {
		return "PowerPC";
	} else if (isa == INSTRUCTION_SET_SPARC) {
		return "SPARC";
	} else if (isa == INSTRUCTION_SET_SUPERH) {
		return "SuperH";
	} else if (isa == INSTRUCTION_SET_X64) {
		return "x86-64";
	} else if (isa == INSTRUCTION_SET_X86) {
		return "x86";
	}

	return "[unknown]";
}

void program_header_flags_to_str(char* dst, uint32_t flags) {
	if (flags & 1)
		dst[2] = 'x';
	else
		dst[2] = ' ';

	if (flags & 2)
		dst[1] = 'w';
	else
		dst[1] = ' ';

	if (flags & 4)
		dst[0] = 'r';
	else
		dst[0] = ' ';

	dst[3] = 0;
}
