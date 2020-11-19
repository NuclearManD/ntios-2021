cmake_minimum_required(VERSION 2.6)
project(nportable_os)

add_executable(nportable_os main.cpp)

install(TARGETS nportable_os RUNTIME DESTINATION bin)
