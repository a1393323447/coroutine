PROJECT(example)
cmake_minimum_required(VERSION 3.20)
set(CMAKE_C_STANDARD 99)

add_executable(common common.c)
target_link_libraries(common coroutine)

add_executable(co_sync co_sync.c)
target_link_libraries(co_sync coroutine)