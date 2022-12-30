cmake_minimum_required(VERSION 3.22)
project(emulator C)

set(CMAKE_C_STANDARD 99)

add_executable(emulator CPU.c emulator.h pswandbus.c loader.c decoder.c fetch.c execute.c Register_display.c)
