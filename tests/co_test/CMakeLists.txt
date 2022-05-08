PROJECT(coroutine)
cmake_minimum_required(VERSION 3.20)
set(CMAKE_C_STANDARD 99)

add_executable(co-test main.c co-test.h ../../prelude.h ../../core/core.h ../../core/core.c ../../core/regs.h)
#target_link_libraries(co-test coroutine)