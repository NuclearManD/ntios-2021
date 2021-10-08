
import os, json, subprocess, sys


'''
For the future, if anyone is wondering what goon would force -Wall -Wextra -Werror
for every file, on every platform, in every language, it was me.  And here's why:
It produces better code by forcing us (I hate it too) to clarify and not make potential blunders.
Ah yes, the project is run by someone from 42SV :)
No I'm not going to remove it.  I've added -fpermissive too (that one I PARTICULARLY hate)
'''

BASE_CPP_FLAGS = "-Wall -Wextra -Werror -Wno-implicit-fallthrough -Wno-nonnull-compare -Wno-narrowing -Wno-comment -fmax-errors=5"
BASE_CXX_FLAGS = "-std=gnu++14 -felide-constructors -fpermissive -fno-rtti"
BASE_C_FLAGS = ""  # None for now
BASE_LD_FLAGS = '-Lbuild/libs'

LIBRARY_FLAGS = '-lneon'

ntios_include_dir = os.path.abspath('ntios_include')
arduino_include_dir = os.path.abspath('arduino_libcpp/includes')
neon_include_dir = os.path.abspath('neon-libc/include')
include_flags = f' -I{ntios_include_dir} -I{arduino_include_dir} -I{neon_include_dir}'

def compile_current_directory(build_dst: str, conf: dict, extra_flags: str = '') -> (int, list):
    os.makedirs(build_dst, exist_ok=True)

    mcu = conf['mcu']
    executable_type = conf['executable_type']

    defines = f'-D__{mcu}__ -DNTIOS_{mcu} -DEXEC_TYPE={executable_type} -DNATIVE_NTIOS=0x20210000'

    cpu_flags = conf['cpu_flags']
    cpp_flags = BASE_CPP_FLAGS + ' ' + conf['cpp_flags'] + ' ' + cpu_flags + ' ' + defines + include_flags + ' ' + extra_flags
    cxx_flags = BASE_CXX_FLAGS + ' ' + conf['cxx_flags'] + ' ' + cpp_flags
    c_flags   = BASE_C_FLAGS + ' ' + conf['c_flags']   + ' ' + cpp_flags

    cc = conf['cc']
    cxx = conf['cxx']

    c_files = []
    cpp_files = []
    o_files = []
    errors = 0
    for filename in os.listdir('.'):
        if filename.endswith('.c'):
            c_files.append(filename)
        if filename.endswith('.cpp'):
            cpp_files.append(filename)
        if os.path.isdir(filename):
            new_dst = os.path.join(build_dst, filename)
            os.chdir(filename)
            new_errs, new_o_files = compile_current_directory(new_dst, conf, extra_flags)
            os.chdir('..')
            o_files += new_o_files
            errors += new_errs

    for i in c_files:
        o_file = build_dst + '/' + i[:-2] + '.o'
        o_files.append(o_file)

        # Skip O files that have already been generated
        if os.path.isfile(o_file):
            last_compiled_time = os.path.getmtime(o_file)
            if last_compiled_time > os.path.getmtime(i):
                continue

        cmd = f'{cc} {i} -c -o {o_file} {c_flags}'
        proc = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        proc.wait()
        if proc.returncode != 0:
            print('\n' + cmd)
            print(proc.stdout.read().decode() + '\n')
            sys.stderr.write(proc.stderr.read().decode() + '\n')
            errors += 1
        else:
            print(i)

        if errors >= 5:
            break

    if errors > 0:
        return errors, o_files

    errors = 0
    for i in cpp_files:
        o_file = build_dst + '/' + i[:-4] + '.o'
        o_files.append(o_file)

        # Skip O files that have already been generated
        if os.path.isfile(o_file):
            last_compiled_time = os.path.getmtime(o_file)
            if last_compiled_time > os.path.getmtime(i):
                continue

        cmd = f'{cxx} {i} -c -o {o_file} {c_flags}'
        proc = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        proc.wait()
        if proc.returncode != 0:
            print('\n' + cmd)
            print(proc.stdout.read().decode() + '\n')
            sys.stderr.write(proc.stderr.read().decode() + '\n')
            errors += 1
        else:
            print(i)

        if errors >= 5:
            break

    return errors, o_files

def build_platform(name: str):
    path = os.path.join('platforms', name)
    if not os.path.isdir(path):
        raise FileNotFoundError(f'No platform found matching {name}')

    with open(os.path.join(path, 'config.json')) as f:
        conf = json.load(f)

    os.makedirs('build/libs', exist_ok=True)

    cpu_flags = conf['cpu_flags']
    mcu_ld = f'platforms/{name}/' + conf["mcu_ld"]
    ld_flags  = f'{conf["ld_flags"]} {cpu_flags} -T{mcu_ld} {BASE_LD_FLAGS}'

    ld = conf['ld']
    objcopy = conf['objcopy']
    objdump = conf['objdump']
    size = conf['size']
    ar = conf['ar']

    target = f'ntios_{name}'

    errors = 0
    o_files = []

    platform_build_dir = os.path.abspath(f'build/{name}')
    ntios_build_dir = os.path.abspath(f'build/ntios')
    arduino_build_dir = os.path.abspath(f'build/arduino')
    neon_libc_build_dir = os.path.abspath(f'build/neon-libc')
    emuarch_build_dir = os.path.abspath(f'build/emuarch')

    print(f"\nCompiling platform {name}\n")

    os.chdir(path)
    new_errors, new_o_files = compile_current_directory(platform_build_dir, conf)
    errors += new_errors
    o_files += new_o_files

    if errors > 0:
        print("Failure.  Fix errors.")
        return -1

    print("\nCompiling NTIOS")
    os.chdir('../../ntios')

    new_errors, new_o_files = compile_current_directory(ntios_build_dir, conf)
    errors += new_errors
    o_files += new_o_files

    if errors > 0:
        print("Failure.  Fix errors.")
        return -1

    print("\nCompiling arduino_libcpp")
    os.chdir('../arduino_libcpp')

    new_errors, new_o_files = compile_current_directory(arduino_build_dir, conf)
    errors += new_errors
    o_files += new_o_files

    if errors > 0:
        print("Failure.  Fix errors.")
        return -1

    print("\nCompiling neon-libc")
    os.chdir('../neon-libc')

    new_errors, neon_o_files = compile_current_directory(neon_libc_build_dir, conf, '-fno-tree-loop-distribute-patterns')
    errors += new_errors

    if errors > 0:
        print("Failure.  Fix errors.")
        return -1

    print("\nCompiling emuarch")
    os.chdir('../emuarch')

    new_errors, neon_o_files = compile_current_directory(emuarch_build_dir, conf, '-fno-tree-loop-distribute-patterns')
    errors += new_errors

    if errors > 0:
        print("Failure.  Fix errors.")
        return -1

    os.chdir('..')

    cmd = f'{ar} rcs build/libs/libneon.a ' + ' '.join(neon_o_files)
    proc = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    proc.wait()
    if proc.returncode != 0:
        print('\n' + cmd)
        print(proc.stdout.read().decode() + '\n')
        sys.stderr.write(proc.stderr.read().decode() + '\n')
        print('Failure.')
        return -2
    else:
        print('build/libs/neon-libc.a')

    print("\nLinking...")

    elf_file = f'build/{name}.elf'
    cmd = f'{ld} {ld_flags} -o {elf_file} {" ".join(o_files)} {LIBRARY_FLAGS}'
    proc = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    proc.wait()
    if proc.returncode != 0:
        print('\n' + cmd)
        print(proc.stdout.read().decode() + '\n')
        sys.stderr.write(proc.stderr.read().decode() + '\n')
        print('Failure.')
        return -2
    else:
        print(elf_file)

    print("\nDone.\n")

    cmd = f'{size} {elf_file}'
    proc = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    proc.wait()
    print(proc.stdout.read().decode() + '\n')
    sys.stderr.write(proc.stderr.read().decode() + '\n')

    hex_file = f'build/{name}.hex'
    cmd = f'{objcopy} -O ihex -R .eeprom {elf_file} {hex_file}'
    proc = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    proc.wait()
    print(proc.stdout.read().decode() + '\n')
    sys.stderr.write(proc.stderr.read().decode() + '\n')

    os.system(f'{objdump} -d -S -C build/{name}.elf > build/{name}.lst')


    return 0

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
