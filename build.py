
import os, json, subprocess, sys


'''
For the future, if anyone is wondering what goon would force -Wall -Wextra -Werror
for every file, on every platform, in every language, it was me.  And here's why:
It produces better code by forcing us (I hate it too) to clarify and not make potential blunders.
Ah yes, the project is run by someone from 42SV :)
No I'm not going to remove it.  I've added -fpermissive too (that one I PARTICULARLY hate)
'''

BASE_CPP_FLAGS = "-Wall -Wextra -Werror -fmax-errors=5"
BASE_CXX_FLAGS = "-std=gnu++14 -felide-constructors -fpermissive -fno-rtti"
BASE_C_FLAGS = ""  # None for now

def build_platform(name):
	path = os.path.join('platforms', name)
	if not os.path.isdir(path):
		raise FileNotFoundError(f'No platform found matching {name}')

	with open(os.path.join(path, 'config.json')) as f:
		conf = json.load(f)

	mcu = conf['mcu']
	executable_type = conf['executable_type']

	defines = f'-D__{mcu}__ -DNTIOS_{mcu} -DEXEC_TYPE={executable_type} -DNATIVE_NTIOS=0x20210000'

	cpu_flags = conf['cpu_flags']
	cpp_flags = BASE_CPP_FLAGS + ' ' + conf['cpp_flags'] + ' ' + cpu_flags + ' ' + defines
	cxx_flags = BASE_CXX_FLAGS + ' ' + conf['cxx_flags'] + ' ' + cpp_flags
	c_flags   = BASE_C_FLAGS + ' ' + conf['c_flags']   + ' ' + cpp_flags
	ld_flags  = f'{conf["ld_flags"]} {cpu_flags} -T{conf["mcu_ld"]}'

	cc = conf['cc']
	ld = conf['ld']
	cxx = conf['cxx']
	objcopy = conf['objcopy']
	size = conf['size']

	target = f'ntios_{name}'

	os.chdir(path)
	os.makedirs('build', exist_ok=True)

	c_files = []
	cpp_files = []
	o_files = []
	for filename in os.listdir('.'):
		if filename.endswith('.c'):
			c_files.append(filename)
		if filename.endswith('.cpp'):
			cpp_files.append(filename)

	did_error = False
	for i in c_files:
		o_file = 'build/' + i[:-2] + '.o'
		cmd = f'{cc} {i} -o {o_file} {c_flags}'
		print(cmd)
		proc = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
		proc.wait()
		print(proc.stdout.read().decode() + '\n')
		sys.stderr.write(proc.stderr.read().decode() + '\n')
		if proc.returncode != 0:
			did_error = True

	if did_error:
		print("Failure.  Fix errors.")
		return

	os.chdir('../..')
'''
# automatically create lists of the sources and objects
# TODO: this does not handle Arduino libraries yet...
C_FILES := $(wildcard *.c)
CPP_FILES := $(wildcard *.cpp)
OBJS := $(C_FILES:.c=.o) $(CPP_FILES:.cpp=.o)


# the actual makefile rules (all .o files built by GNU make's default implicit rules)

all: $(TARGET).hex

$(TARGET).elf: $(OBJS) $(MCU_LD)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

%.hex: %.elf
	$(SIZE) $<
	$(OBJCOPY) -O ihex -R .eeprom $< $@
ifneq (,$(wildcard $(TOOLSPATH)))
	$(TOOLSPATH)/teensy_post_compile -file=$(basename $@) -path=$(shell pwd) -tools=$(TOOLSPATH)
	-$(TOOLSPATH)/teensy_reboot
endif

# compiler generated dependency info
-include $(OBJS:.o=.d)

clean:
	rm -f *.o *.d $(TARGET).elf $(TARGET).hex
'''
build_platform('teensy4')
