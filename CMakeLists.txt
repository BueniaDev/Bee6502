cmake_minimum_required(VERSION 3.10)
project(bee6502)

# Require C++17 (and position independent code)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

option(BUILD_M6502_SDL2 "Enables the SDL2 frontend (requires SDL2)." OFF)

add_subdirectory(Bee6502)

if (BUILD_M6502_SDL2 STREQUAL "ON")
	message(STATUS "Building Bee6502-SDL2...")
	add_subdirectory(Bee6502-SDL2)
endif()

if (WIN32)
    message(STATUS "Operating system is Windows.")
    if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
	target_compile_options(bee6502 PRIVATE -Wall)
	message(STATUS "Compiler is MinGW.")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL MSVC)
	message(WARNING "MSVC support is HIGHLY experimental, and may not even compile correctly, so be EXTREMELY careful here!")
	target_compile_options(bee6502 PRIVATE /W4)
	message(STATUS "Compiler is MSVC.")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL Clang)
	message(WARNING "Clang support on Windows is HIGHLY experimental, and may not even compile correctly, so be EXTREMELY careful here!")
	target_compile_options(bee6502 PRIVATE -Wall)
	message(STATUS "Compiler is Clang.")
    else()
	message(SEND_ERROR "Compiler not supported.")
	return()
    endif()
elseif (UNIX AND NOT APPLE)
    message(STATUS "Operating system is Linux.")
    if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
	target_compile_options(bee6502 PRIVATE -Wall)
	message(STATUS "Compiler is MinGW.")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL Clang)
	target_compile_options(bee6502 PRIVATE -Wall)
	message(STATUS "Compiler is Clang.")
    else()
	message(SEND_ERROR "Compiler not supported.")
	return()
    endif()
elseif (APPLE)
    message(STATUS "Operating system is Mac.")
    if (CMAKE_CXX_COMPILER_ID STREQUAL AppleClang)
	target_compile_options(bee6502 PRIVATE -Wall)
	message(STATUS "Compiler is AppleClang.")
    else()
	message(SEND_ERROR "Compiler not supported.")
	return()
    endif()
else()
    message(SEND_ERROR "Operating system not supported.")
    return()
endif()