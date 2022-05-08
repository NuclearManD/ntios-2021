
project(ntios2021)

add_library(arduino
		${CMAKE_CURRENT_SOURCE_DIR}/IPAddress.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/Print.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/Stream.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/WMath.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/WString.cpp
)

target_include_directories(arduino PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/includes)
