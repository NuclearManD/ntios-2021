cmake_minimum_required (VERSION 2.8.11)

set(PLATFORM "teensy41")

if ("${PLATFORM}" STREQUAL "teensy41")
	set(CMAKE_SYSTEM_NAME Generic) # Or name of your OS if you have one
	set(CMAKE_SYSTEM_PROCESSOR arm) # Or whatever
	set(CMAKE_CROSSCOMPILING 1)
	set(CMAKE_C_COMPILER arm-none-eabi)
	set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
endif()

project (ntios-2021)

# For the future, if anyone is wondering what goon would force -Wall -Wextra -Werror
# for every file, on every platform, in every language, it was me.  And here's why:
# It produces better code by forcing us (I hate it too) to clarify and not make potential blunders.
# Ah yes, the project is run by someone from 42SV :)
# No I'm not going to remove it.  I've added -fpermissive too (that one I PARTICULARLY hate)

set(CPP_FLAGS "-Wall -Wextra -Werror -Wno-implicit-fallthrough -Wno-nonnull-compare -Wno-narrowing -Wno-comment -Wno-unused-parameter -fmax-errors=5")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${CPP_FLAGS} -std=gnu++14 -felide-constructors -fpermissive -fno-rtti")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CPP_FLAGS}")

# Set compiler and other platform dependent variables
add_subdirectory("platforms/${PLATFORM}")

include_directories(ntios_include)
add_subdirectory(neon-libc)
add_subdirectory(arduino_libcpp)
add_subdirectory(emuarch)
add_subdirectory(ntios)

add_executable(ntios)
target_link_libraries(ntios PUBLIC neon-libc arduino ntios_core emuarch ${PLATFORM})
