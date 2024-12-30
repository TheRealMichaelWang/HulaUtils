# CMakeList.txt : CMake project for HulaUtils, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("HulaUtils")

# Add source to this project's executable.
#add_executable (HulaUtils "HulaUtils.cpp" "HulaUtils.h" "HulaScript.cpp")
set(BUILD_SHARED_LIBS ON)
add_library(HulaUtils SHARED
	HulaUtils.cpp
	HulaUtils.hpp
	HulaScript.hpp 
	HulaScript.cpp "json.cpp")

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/dynalo)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

# TODO: Add tests and install targets if needed.
