
project (ntios-2021)

add_library(ntios_core
	actuator_control/control_system.cpp
	actuator_control/pid.cpp
	crypto/sha256.cpp
	drivers/filesystems/exfat.cpp
	drivers/filesystems/fat32.cpp
	drivers/gpio_expanders/MCP23017.cpp
	drivers/graphics/canvas.cpp
	drivers/graphics/font.cpp
	drivers/graphics/graphics.cpp
	drivers/graphics/ili9481.cpp
	drivers/graphics/ra8875.cpp
	drivers/touch/FT5436.cpp
	drivers/touch/GT911.cpp
	drivers/arm_mmu.cpp
	drivers/bno055.cpp
	drivers/cellular.cpp
	drivers/device_cli_utils.cpp
	drivers/disks.cpp
	drivers/imu_util.cpp
	fileapi/audio_decoder.cpp
	fileapi/audio_encoder.cpp
	fileapi/image_codec.cpp
	actuation.cpp
	csvlog.cpp
	drivers.cpp
	elf-header.c
	emuas.cpp
	estimators.cpp
	exec.cpp
	fs.cpp
	main.cpp
	navigation.cpp
	ntios.cpp
	ntios_math.cpp
	shell.cpp
	texteditor.cpp
)

target_link_libraries(ntios_core PUBLIC neon_libc emuarch arduino)
target_include_directories(ntios_core PUBLIC ../ntios_include)
