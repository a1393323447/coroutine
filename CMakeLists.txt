PROJECT(coroutine)
cmake_minimum_required(VERSION 3.20)
set(CMAKE_C_STANDARD 99)

add_library(coroutine STATIC
        predule.h
    core/core.h core/core.c
    core/regs.h
    pipe/pipe.h
    sync/comutex.h sync/comutex.c sync/corwlock.h sync/corwlock.c sync/sync.h)

add_subdirectory(example)