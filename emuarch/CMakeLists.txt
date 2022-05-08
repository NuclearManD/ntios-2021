
project (ntios-2021)

add_library(emuarch
		emu_platform.cpp
		emuarch_strings.cpp
		emulator.cpp
		memory.cpp
)

target_include_directories(emuarch PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
