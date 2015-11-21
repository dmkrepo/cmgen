cmake_minimum_required(VERSION 3.0)

project(cmgen)

set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE TYPE INTERNAL FORCE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/../cmgen-debug  CACHE STRING "" FORCE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/../cmgen CACHE STRING "" FORCE)

add_definitions(
	-DNOMINMAX
	-D_CRT_SECURE_NO_WARNINGS
)

include_directories(dmk)

add_executable(cmgen 
	cmgen.cpp
	cmgen.h
	configurers.h
	expressions.h
	fetchers.h
	project.h
	
	dmk/dmk.h
	dmk/dmk_assert.h
	dmk/dmk_console.h
	dmk/dmk_fraction.h
	dmk/dmk_json.cpp
	dmk/dmk_json.h
	dmk/dmk_memory.h
	dmk/dmk_path.h
	dmk/dmk_result.h
	dmk/dmk_string.h
	dmk/dmk_time.h
	dmk/cppformat/format.cc
	dmk/cppformat/format.h
)
