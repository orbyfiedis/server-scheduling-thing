cmake_minimum_required(VERSION 3.22)
project(server)

set(CMAKE_CXX_STANDARD 20)

add_executable(server main.cpp server-scheduling.cpp server-scheduling.h server.cpp server.h utilh.h lua-util.h)

target_link_libraries(server ${CMAKE_SOURCE_DIR}/lua/lua54.dll)