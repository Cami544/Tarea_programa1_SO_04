cmake_minimum_required(VERSION 3.20)
project(tare_programa2_so_04 C)

set(CMAKE_C_STANDARD 11)

include_directories(include)

add_executable(terminal
        src/main.c
        src/hilos.c
        src/sync.c
        src/scheduler.c
)

target_link_libraries(terminal pthread)
