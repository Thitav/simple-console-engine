cmake_minimum_required(VERSION 3.30)
project(simple_console_engine C)

set(CMAKE_C_STANDARD 11)

add_library(simple_console_engine STATIC
        src/console.c
        include/console.h
        include/types.h
        src/audio.c
        include/audio.h
)

add_executable(console_test src/main.c
        src/bitmap.c
        include/bitmap.h)
target_link_libraries(console_test simple_console_engine)
