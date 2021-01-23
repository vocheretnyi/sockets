cmake_minimum_required(VERSION 3.15)
project(sockets)

set(CMAKE_CXX_STANDARD 17)

add_executable(my_client my_client.c)
add_executable(my_server my_server.c)

