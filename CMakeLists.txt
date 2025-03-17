cmake_minimum_required(VERSION 3.30)
project(console_engine C)

set(CMAKE_C_STANDARD 11)

add_library(console_engine STATIC src/console.c
)
add_executable(console_test src/main.c)
target_link_libraries(console_test console_engine)
