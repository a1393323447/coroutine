cmake_minimum_required(VERSION 3.20)
project(coroutine C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(coroutine
        coroutine.c
        coroutine.h
        main.c
        regs.h)
