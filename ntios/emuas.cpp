
#include "ntios.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>



bool isint(const char* itm) {
	if (itm[0] == '-') itm++;
	return *itm <= '9' && *itm >= '0';
}

int64_t parseint(const char* s) {
	if (!isint(s)) return -100000;

	int base = 10;
	bool isneg = s[0] == '-';
	if (isneg) s++;

	if (s[0] == '0') {
		if (s[1] == 'x') {
			base = 16;
			s += 2;
		} else if (s[1] == 'b') {
			base = 2;
			s += 2;
		} else if (s[1] <= '9' && s[1] >= '0'){
			base = 8;
			s++;
		}
	}
	return strtol(s, NULL, base);
}

class AssemblyOutputObject {
private:
	uint64_t* lbl_ptrs = (uint64_t*)malloc(0);
	const char** lbl_names = (const char**)malloc(0);
	int num_lbls = 0;

	NTIOSFile* out;

public:
	void setOutput(NTIOSFile* n) {out = n;}

	// returns false on failure (label already defined)
	bool addLabel(const char* name, unsigned long loc) {
		if (hasLabel(name)) return false;

		lbl_ptrs = (uint64_t*)realloc(lbl_ptrs, (num_lbls + 1) * sizeof(uint64_t));
		lbl_names = (const char**)realloc(lbl_names, (num_lbls + 1) * sizeof(char*));
		lbl_ptrs[num_lbls] = loc;
		lbl_names[num_lbls] = name;
		num_lbls++;
		return true;
	}

	bool hasLabel(const char* name) {
		for (int i = 0; i < num_lbls; i++)
			if (!strcmp(name, lbl_names[i])) return true;

		return false;
	}

	uint64_t getLabel(const char* name) {
		for (int i = 0; i < num_lbls; i++)
			if (!strcmp(name, lbl_names[i]))
				return lbl_ptrs[i];
		return -1;
	}

	size_t write8(uint8_t num) {
		return out->write(num);
	}

	size_t write16(uint16_t num) {
		size_t total = out->write(num & 255);
		total += out->write(num >> 8);
		return total;
	}

	size_t write32(uint32_t num) {
		size_t total = write16(num & 65535);
		total += write16(num >> 16);
		return total;
	}

	size_t write64(uint64_t num) {
		size_t total = write32(num & 0xFFFFFFFFUL);
		total += write32(num >> 32);
		return total;
	}

	size_t writeString(char* s) {
		return out->print(s);
	}

	size_t write16bigendian(uint16_t num) {
		size_t total = out->write(num >> 8);
		total += out->write(num & 255);
		return total;
	}

	// returns true on error
	bool interpret8(const char* s) {
		if (isint(s)) {
			write8(parseint(s));
		} else if (hasLabel(s)) {
			write8(getLabel(s));
		} else return true;
		return false;
	}

	// returns true on error
	bool interpret16(const char* s) {
		if (isint(s)) {
			write16(parseint(s));
		} else if (hasLabel(s)) {
			write16(getLabel(s));
		} else return true;
		return false;
	}

	// returns true on error
	bool interpret32(const char* s) {
		if (isint(s)) {
			write32(parseint(s));
		} else if (hasLabel(s)) {
			write32(getLabel(s));
		} else return true;
		return false;
	}

	// returns true on error
	bool interpret64(const char* s) {
		if (isint(s)) {
			write64(parseint(s));
		} else if (hasLabel(s)) {
			write64(getLabel(s));
		} else return true;
		return false;
	}
};

bool streq_to_space(const char* a, const char* b) {
	while (true) {
		char ac = *a;
		char bc = *b;
		if (ac == ' ' || ac == '\n' || ac == '\r' || ac == ';' || ac == '\t') ac = 0;
		if (bc == ' ' || bc == '\n' || bc == '\r' || bc == ';' || bc == '\t') bc = 0;
		if (ac != bc) return false;
		if (ac == 0) return true;
		a++;
		b++;
	}
}

// safe to pass unclean input to this function :)
int regname_to_regid(char* regname) {
	if (regname == NULL) return -2;
	if (regname[0] == 'r' && regname[2] == 'x') {
		if (regname[1] >= 'a' && regname[1] <= 'd') return regname[1] - 'a';
		else return -1;
	} else if (regname[0] == 'r') {
		if (regname[1] <= '5' && regname[1] >= '0') return regname[1] - '0' + 8;
		else return -1;
	} else if (regname[0] == 'f') {
		if (regname[1] <= '9' && regname[1] >= '0') return regname[1] - '0' + 16;
		else return -1;
	} else if (streq_to_space(regname, "cr0")) return 15;
	else if (streq_to_space(regname, "cnt")) return 14;
	else if (streq_to_space(regname, "si")) return 4;
	else if (streq_to_space(regname, "di")) return 5;
	else if (streq_to_space(regname, "sp")) return 6;
	else if (streq_to_space(regname, "pc")) return 7;
	else
		return -1;
}

int twoarg_math_op(const char* buffer) {
	if (streq_to_space(buffer, "add")) return 0;
	if (streq_to_space(buffer, "sub")) return 1;
	if (streq_to_space(buffer, "mul")) return 2;
	if (streq_to_space(buffer, "div")) return 3;
	if (streq_to_space(buffer, "and")) return 4;
	if (streq_to_space(buffer, "or")) return 5;
	if (streq_to_space(buffer, "xor")) return 6;
	if (streq_to_space(buffer, "cmp")) return 7;
	if (streq_to_space(buffer, "lsh")) return 12;
	if (streq_to_space(buffer, "rsh")) return 13;
	if (streq_to_space(buffer, "ras")) return 14;
	return -1;
}

int onearg_math_op(const char* buffer) {
	if (streq_to_space(buffer, "inc")) return 0;
	if (streq_to_space(buffer, "dec")) return 1;
	if (streq_to_space(buffer, "neg")) return 2;
	if (streq_to_space(buffer, "not")) return 3;
	if (streq_to_space(buffer, "sqrt")) return 4;
	if (streq_to_space(buffer, "tanh")) return 5;
	return -1;
}

int conditional_jmp(const char* buffer) {
	if (streq_to_space(buffer, "jz")) return 0;
	if (streq_to_space(buffer, "jnz")) return 1;
	if (streq_to_space(buffer, "jl")) return 2;
	if (streq_to_space(buffer, "jg")) return 3;
	return -1;
}

int noarg_onebyte_ops(const char* buffer) {
	if (streq_to_space(buffer, "lodq"))
		return 0xC0;
	else if (streq_to_space(buffer, "lodd"))
		return 0xC4;
	else if (streq_to_space(buffer, "lodw"))
		return 0xC8;
	else if (streq_to_space(buffer, "lodb"))
		return 0xCC;
	return -1;
}

int noarg_twobyte_ops(const char* buffer) {
	if (streq_to_space(buffer, "strcpy"))
		return 0xEC10;
	else if (streq_to_space(buffer, "strcat"))
		return 0xEC08;
	else if (streq_to_space(buffer, "strcmp"))
		return 0xEC20;
	else if (streq_to_space(buffer, "strlen"))
		return 0xEC00;
	else if (streq_to_space(buffer, "strf"))
		return 0xEC18;
	else if (streq_to_space(buffer, "strskip"))
		return 0xEC30;
	else if (streq_to_space(buffer, "strfi"))
		return 0xEC28;
	else if (streq_to_space(buffer, "stoa"))
		return 0xEC02;
	else if (streq_to_space(buffer, "utoa"))
		return 0xEC12;
	else if (streq_to_space(buffer, "ftoa"))
		return 0xEC22;
	return -1;
}

long b_emuas_assemble(StreamDevice* io, AssemblyOutputObject& asmout, const char* ifilename, long loc) {
	const char* syscall_err_fmt = "%s:%i: Syscall ID not uint16\n";
	const char* syntax_err_fmt = "%s:%i: Syntax error\n";
	const char* not_an_instruction_err_fmt = "%s:%i: Invalid Opcode\n";
	const char* not_a_register_fmt = "%s:%i: Not a register '%s'\n";
	const char* undefined_fmt = "%s:%i: Undefined: '%s'\n";
	const char* invalid_reg_fmt = "%s:%i: Invalid register selection\n";
	NTIOSFile* ifile = fsopen(ifilename, NTIOS_READ);
	char buffer[128];
	int space_indexes[3];
	int i;
	char c;
	bool incomment;
	int line = 0;
	char lastchar;
	bool haderr = false;
	int opid;

	if (ifile == NULL) {
		io->printf("Error: unable to open '%s'\n", ifilename);
		return -1801;
	}

	while (ifile->available()) {
		i = 0;
		incomment = false;

		bool inbracket = false;
		bool inquote = false;
		while (ifile->available()) {
			c = ifile->read();
			if (c == '\n' || c == '\r') break;
			else if (c == ';') incomment = true;
			else if ((c =='\t' || c == ' ' || c == ',')) {
				if (!inbracket) {
					if (!inquote) {
						if ((i == 0 || buffer[i - 1] == ' ') && !inquote)
							continue;
						c = ' ';
					}
				} else
					continue;
			} else if (c == '"')
				inquote = !inquote;
			else if (c == '[')
				inbracket = true;
			else if (c == ']')
				inbracket = false;
			else if (c == '-' && inbracket)
				buffer[i++] = ' ';
			else if (c == '+' && inbracket)
				c = ' ';
			if (!incomment)
				buffer[i++] = c;
		}

		line++;
		lastchar = buffer[i - 1];
		buffer[i] = 0;

		int j = 0;
		for (i = 0; i < 3; i++) {
			space_indexes[i] = 0;
			for (; buffer[j]; j++)
				if (buffer[j] == ' ') {
					space_indexes[i] = j;
					j++;
					break;
				}
		}

		if (streq_to_space(buffer, "db")) {
			if (buffer[3] == '"') {
				// check length of string literal
				for (i = 0; buffer[i + 4] != '"'; i++) {
					char c = buffer[i + 4];
					if (c == '\\') {
						i++;
						asmout.write8(backslash_delimit(buffer[i + 4]));
					} else
						asmout.write8(c);
					if (c == 0) {
						io->printf(syntax_err_fmt, ifilename, line);
						return -1802;
					}
				}
				loc += i;
			} else if (isint(buffer + 3)) {
				asmout.write8(parseint(buffer + 3));
				loc++;
			}
			else {
				io->printf(syntax_err_fmt, ifilename, line);
				return -1802;
			}
		} else if (streq_to_space(buffer, ".org")) {
			loc = parseint(buffer + 5);
		} else if (lastchar == ':') {
			int len = strlen(buffer);
			char* str = (char*)malloc(len);
			strncpy(str, buffer, len - 1);
			str[len - 1] = 0;
			asmout.addLabel(str, loc);
		} else if ((opid = noarg_onebyte_ops(buffer)) != -1) {
			asmout.write8(opid);
			loc += 1;
		} else if ((opid = noarg_twobyte_ops(buffer)) != -1) {
			asmout.write16bigendian(opid);
			loc += 2;
		} else if ((opid = conditional_jmp(buffer)) != -1) {
			bool interp_err;
			if (opid < 2 && (space_indexes[1] != 0 || buffer[space_indexes[1] + 1] == 0)) {
				int reg = regname_to_regid(buffer + space_indexes[0] + 1);
				asmout.write8(0x90);
				asmout.write8(((1 - opid) << 4) | reg);
				interp_err = asmout.interpret64(buffer + space_indexes[1] + 1);
				loc += 10;
			} else {
				asmout.write8(0x30 | opid);
				interp_err = asmout.interpret64(buffer + space_indexes[0] + 1);
				loc += 9;
			}
			if (interp_err) {
				io->printf(undefined_fmt, ifilename, line, buffer + space_indexes[1] + 1);
				haderr = true;
			}
		} else if ((opid = onearg_math_op(buffer)) != -1) {
			int r1 = regname_to_regid(buffer + space_indexes[0] + 1);

			asmout.write8(0x40 | opid);
			asmout.write8(r1);
			loc += 2;
		} else if ((opid = twoarg_math_op(buffer)) != -1) {
			int r1 = regname_to_regid(buffer + space_indexes[0] + 1);
			int r2 = regname_to_regid(buffer + space_indexes[1] + 1);

			if (r2 != -1) {
				if ((r2 & 16) != (r1 & 16)) {
					io->printf(syntax_err_fmt, ifilename, line);
					haderr = true;
				} else {
					asmout.write8(0x50 | opid | ((r1 >> 4) << 3));
					asmout.write8(r2 | (r1 << 4));
				}
			} else {
				bool interpret_fail;
				if (r1 < 8) {
					asmout.write8(0x60 | opid);
					asmout.write8(r1);
					interpret_fail = asmout.interpret64(buffer + space_indexes[1] + 1);
					loc += 8;
				} else {
					asmout.write8(0x60 | opid);
					asmout.write8(r1);
					interpret_fail = asmout.interpret32(buffer + space_indexes[1] + 1);
					loc += 4;
				}
				if (interpret_fail) {
					io->printf(undefined_fmt, ifilename, line, buffer + space_indexes[1] + 1);
					haderr = true;
				}
			}
			loc += 2;
		} else if (strncmp(buffer, "cmp", 3) == 0) {
			bool interpret_fail = false;
			if (buffer[3] == 'b') {
				asmout.write8(0x17);
				interpret_fail = asmout.interpret8(buffer + space_indexes[1] + 1);
				loc += 2;
			} else if (buffer[3] == 'w') {
				asmout.write8(0x16);
				interpret_fail = asmout.interpret16(buffer + space_indexes[1] + 1);
				loc += 3;
			} else if (buffer[3] == 'd') {
				asmout.write8(0x15);
				interpret_fail = asmout.interpret32(buffer + space_indexes[1] + 1);
				loc += 5;
			} else if (buffer[3] == 'q') {
				asmout.write8(0x14);
				interpret_fail = asmout.interpret64(buffer + space_indexes[1] + 1);
				loc += 9;
			}/* else {
				// re
			}*/

			if (interpret_fail) {
				io->printf(undefined_fmt, ifilename, line, buffer + space_indexes[1] + 1);
				haderr = true;
			}
		} else if (strncmp(buffer, "mov", 3) == 0) {
			int movsz = 0;
			int szid = 0;
			if (buffer[3] == 'b') {
				movsz = 1;
				szid = 3;
			} else if (buffer[3] == 'w') {
				movsz = 2;
				szid = 2;
			} else if (buffer[3] == 'd') {
				movsz = 4;
				szid = 1;
			} else if (buffer[3] == 'q') {
				movsz = 8;
				szid = 0;
			}

			if (buffer[space_indexes[1]] == '[') {
				// mov[s] r1, [r2+**]
				int r1 = regname_to_regid(buffer + 5);
				int r2 = regname_to_regid(buffer + space_indexes[1] + 2);
				int offset = parseint(buffer + space_indexes[2] + 1);

				if (r1 == -1 || r2 == -1 || !isint(buffer + space_indexes[2] + 1)) {
					io->printf(syntax_err_fmt, ifilename, line);
					haderr = true;
				} else if (r1 >= 16 || r2 >= 16) {
					io->printf(invalid_reg_fmt, ifilename, line);
					haderr = true;
				} else {
					asmout.write8(0b00100000 | szid);
					asmout.write8((r1 << 4) | r2);
					asmout.write16(offset);
				}
				loc += 4;
			} else if (buffer[5] == '[') {
				// mov[s] [r2+**], r1
				int r1 = regname_to_regid(buffer + space_indexes[2] + 1);
				int r2 = regname_to_regid(buffer + space_indexes[0] + 2);
				int offset = parseint(buffer + space_indexes[1] + 1);

				if (r1 == -1 || r2 == -1 || !isint(buffer + space_indexes[1] + 1)) {
					io->printf(syntax_err_fmt, ifilename, line);
					haderr = true;
				} else if (r1 >= 16 || r2 >= 16) {
					io->printf(invalid_reg_fmt, ifilename, line);
					haderr = true;
				} else {
					asmout.write8(0b00100100 | szid);
					asmout.write8((r1 << 4) | r2);
					asmout.write16(offset);
				}
				loc += 4;
			} else if (regname_to_regid(buffer + space_indexes[1] + 1) != -1) {
				int r1 = regname_to_regid(buffer + 5);
				int r2 = regname_to_regid(buffer + space_indexes[1] + 1);

				if (r1 == -1 || r2 == -1) {
					io->printf(syntax_err_fmt, ifilename, line);
					haderr = true;
				} else {
					asmout.write8(0b00000000 | (szid << 2) | ((r1 >> 3) & 2) | (r1 >> 4));
					asmout.write8(((r1 << 4) & 0xF0) | (r2 & 15));
				}
				loc += 2;
			} else {
				int reg = regname_to_regid(buffer + 5);
				bool interpret_err = false;
				asmout.write8(0x81);
				asmout.write8((szid << 5) | reg);
				if (buffer[3] == 'b')
					interpret_err = asmout.interpret8(buffer + space_indexes[1] + 1);
				else if (buffer[3] == 'w')
					interpret_err = asmout.interpret16(buffer + space_indexes[1] + 1);
				else if (buffer[3] == 'd')
					interpret_err = asmout.interpret32(buffer + space_indexes[1] + 1);
				else if (buffer[3] == 'q')
					interpret_err = asmout.interpret64(buffer + space_indexes[1] + 1);
				if (interpret_err) {
					io->printf(undefined_fmt, ifilename, line, buffer + space_indexes[1] + 1);
					haderr = true;
				}
				loc += 2 + movsz;
			}
		} else if (streq_to_space(buffer, "exx")) {
			int r1 = regname_to_regid(buffer + 4);
			int r2 = regname_to_regid(buffer + space_indexes[1] + 1);

			if (r1 == -1 || r2 == -1) {
				io->printf(syntax_err_fmt, ifilename, line);
				haderr = true;
			} else {
				asmout.write8(0b00010000 | ((r1 >> 3) & 2) | (r1 >> 4));
				asmout.write8(((r1 << 4) & 0xF0) | (r2 & 15));
			}
			loc += 2;
		} else if (streq_to_space(buffer, "syscall")) {
			uint64_t callid = parseint(buffer + 8);
			if (callid > 65535) {
				io->printf(syscall_err_fmt, ifilename, line);
				haderr = true;
			} else {
				asmout.write8(0x80);
				asmout.write16((uint16_t)callid);
			}
			loc += 3;
		} else if (streq_to_space(buffer, "l32")) {
			int reg = regname_to_regid(buffer + 4);
			if (reg == -1 || reg >= 4) {
				io->printf(not_a_register_fmt, ifilename, line, buffer + 4);
				haderr = true;
			} else {
				asmout.write8(0x18 | reg);
				if(asmout.interpret32(buffer + space_indexes[1] + 1)) {
					io->printf(undefined_fmt, ifilename, line, buffer + space_indexes[1] + 1);
					haderr = true;
				}
			}
			loc += 5;
		} else if (streq_to_space(buffer, "halt")) {
			asmout.write8(0xFF);
			loc += 1;
		} else if (streq_to_space(buffer, "jmp")) {
			asmout.write8(0xE0);
			if (asmout.interpret64(buffer + 4)) {
				io->printf(undefined_fmt, ifilename, line, buffer + 4);
				haderr = true;
			}
			loc += 9;
		} else if (streq_to_space(buffer, "call")) {
			asmout.write8(0xA2);
			if (asmout.interpret64(buffer + 5)) {
				io->printf(undefined_fmt, ifilename, line, buffer + 5);
				haderr = true;
			}
			loc += 9;
		} else if (streq_to_space(buffer, "ret")) {
			asmout.write16bigendian(0xA007);
			loc += 2;
		} else if (streq_to_space(buffer, "pop")) {
			int reg;
			if ((reg = regname_to_regid(buffer + 4)) >= 0) {
				asmout.write8(0xA0);
				asmout.write8(regname_to_regid(buffer + 4));
			} else {
				io->printf(not_a_register_fmt, ifilename, line, buffer + 4);
				haderr = true;
			}
			loc += 2;
		} else if (streq_to_space(buffer, "push")) {
			int reg;
			if ((reg = regname_to_regid(buffer + 5)) >= 0) {
				asmout.write8(0xA1);
				asmout.write8(regname_to_regid(buffer + 5));
			} else {
				io->printf(not_a_register_fmt, ifilename, line, buffer + 5);
				haderr = true;
			}
			loc += 2;
		} else if (buffer[0] == 0) {
		} else {
			io->printf(not_an_instruction_err_fmt, ifilename, line);
			return -1802;
		}
	}
	if (haderr) return -1;
	return loc;
}


long b_emuas_preprocessor(StreamDevice* io, AssemblyOutputObject& asmout, const char* ifilename, long loc) {
	const char* syntax_err_fmt = "%s:%i: Syntax error\n";
	const char* not_an_instruction_err_fmt = "%s:%i: Invalid Opcode\n";
	const char* not_a_reg_err_fmt = "%s:%i: Not a register\n";

	NTIOSFile* ifile = fsopen(ifilename, NTIOS_READ);
	char buffer[128];
	int space_indexes[3];
	int i;
	char c;
	bool incomment;
	int line = 0;
	char lastchar;

	int opid;

	if (ifile == NULL) {
		io->printf("Error: unable to open '%s'\n", ifilename);
		return -1801;
	}

	while (ifile->available()) {
		i = 0;
		incomment = false;
		while (ifile->available()) {
			c = ifile->read();
			if (c == '\n' || c == '\r') break;
			if (c == ';') incomment = true;
			if (c =='\t' || c == ' ' || c == ',') {
				if (i == 0 || buffer[i - 1] == ' ')
					continue;
				c = ' ';
			}

			if (!incomment)
				buffer[i++] = c;
		}

		line++;
		lastchar = buffer[i - 1];
		buffer[i] = 0;

		int j = 0;
		for (i = 0; i < 3; i++) {
			space_indexes[i] = 0;
			for (; buffer[j]; j++)
				if (buffer[j] == ' ') {
					space_indexes[i] = j;
					j++;
					break;
				}
		}

		if (streq_to_space(buffer, "db")) {
			if (buffer[2] != ' ') {
				io->printf(syntax_err_fmt, ifilename, line);
				return -1802;
			}
			if (buffer[3] == '"') {
				// check length of string literal
				for (i = 0; buffer[i + 4] != '"'; i++) {
					char c = buffer[i + 4];
					if (c == '\\')
						i++;
					if (c == 0) {
						io->printf(syntax_err_fmt, ifilename, line);
						return -1802;
					}
				}
				loc += i;
			} else if (isint(buffer + 3))
				loc++;
			else {
				io->printf(syntax_err_fmt, ifilename, line);
				return -1802;
			}
		} else if (streq_to_space(buffer, ".org")) {
			if (!isint(buffer + 5)) {
				io->printf(syntax_err_fmt, ifilename, line);
				return -1802;
			}
			loc = parseint(buffer + 5);
		} else if (lastchar == ':') {
			if (space_indexes[0] != 0) {
				io->printf(syntax_err_fmt, ifilename, line);
			} else {
				int len = strlen(buffer);
				char* str = (char*)malloc(len);
				strncpy(str, buffer, len - 1);
				str[len - 1] = 0;
				asmout.addLabel(str, loc);
			}
		} else if (noarg_onebyte_ops(buffer) != -1) {
			loc += 1;
		} else if (noarg_twobyte_ops(buffer) != -1) {
			loc += 1;
		} else if ((opid = conditional_jmp(buffer)) != -1) {
			if (opid < 2 && (space_indexes[1] != 0 || buffer[space_indexes[1] + 1] == 0)) {
				int reg = regname_to_regid(buffer + space_indexes[0] + 1);
				if (reg == -1 || reg >= 16) {
					io->printf(not_an_instruction_err_fmt, ifilename, line);
					return -1802;
				}
				loc += 10;
			} else {
				loc += 9;
			}
		} else if (twoarg_math_op(buffer) != -1) {
			if (regname_to_regid(buffer + space_indexes[1] + 1) != -1) {
				loc += 2;
			} else {
				loc += 2;
				if (regname_to_regid(buffer + space_indexes[0] + 1) < 8) loc += 8;
				else loc += 4;
			}
		} else if (onearg_math_op(buffer) != -1) {
			if (regname_to_regid(buffer + space_indexes[0] + 1) == -1) {
				io->printf(not_a_reg_err_fmt, ifilename, line);
				return -1802;
			}
			loc += 2;
		} else if (strncmp(buffer, "cmp", 3) == 0) {
			int movsz = 0;
			if (buffer[3] == 'b') movsz = 1;
			else if (buffer[3] == 'w') movsz = 2;
			else if (buffer[3] == 'd') movsz = 4;
			else if (buffer[3] == 'q') movsz = 8;
			if (movsz == 0 || buffer[4] != ' ' || regname_to_regid(buffer + 5) != 0) {
				io->printf(not_an_instruction_err_fmt, ifilename, line);
				return -1802;
			}
			loc += 1 + movsz;
		} else if (strncmp(buffer, "mov", 3) == 0) {
			int movsz = 0;
			if (buffer[3] == 'b') movsz = 1;
			else if (buffer[3] == 'w') movsz = 2;
			else if (buffer[3] == 'd') movsz = 4;
			else if (buffer[3] == 'q') movsz = 8;
			if (movsz == 0 || buffer[4] != ' ') {
				io->printf(not_an_instruction_err_fmt, ifilename, line);
				return -1802;
			}

			if (space_indexes[1] == 0) {
				io->printf(syntax_err_fmt, ifilename, line);
				return -1802;
			}

			if (buffer[5] == '[') {
				// mov[s] r1, [r2+**]
				loc += 4;
			} else if (buffer[space_indexes[1]] == '[') {
				// mov[s] [r2+**], r1
				loc += 4;
			} else if (regname_to_regid(buffer + space_indexes[1] + 1) != -1) {
				loc += 2;
			} else {
				loc += 2 + movsz;
			}
		} else if (streq_to_space(buffer, "exx")) {
			loc += 2;
		} else if (streq_to_space(buffer, "syscall")) {
			loc += 3;
		} else if (streq_to_space(buffer, "l32")) {
			loc += 5;
		} else if (streq_to_space(buffer, "halt")) {
			loc += 1;
		} else if (streq_to_space(buffer, "jmp")) {
			loc += 9;
		} else if (streq_to_space(buffer, "call")) {
			loc += 9;
		} else if (streq_to_space(buffer, "ret")) {
			loc += 2;
		} else if (streq_to_space(buffer, "pop")) {
			loc += 2;
		} else if (streq_to_space(buffer, "push")) {
			loc += 2;
		} else if (buffer[0] == 0) {
		} else {
			io->printf(not_an_instruction_err_fmt, ifilename, line);
			return -1802;
		}
	}
	return loc;
}

int b_emuas(StreamDevice* io, int argc, const char** argv) {
	if (argc < 2) {
		io->println("emuas: usage: emuas infile [outfile = o.bin]");
		return -1;
	}

	const char* ifilename = argv[1];
	const char* ofilename = "o.bin";
	if (argc > 2)
		ofilename = argv[2];

	AssemblyOutputObject asmout;

	if (b_emuas_preprocessor(io, asmout, ifilename, 0) < 0) return -901;

	NTIOSFile* out = fsopen(ofilename, NTIOS_WRITE);
	asmout.setOutput(out);
	if (b_emuas_assemble(io, asmout, ifilename, 0) < 0) {
		out->close();
		return -902;
	}

	out->close();

	return 0;
}
