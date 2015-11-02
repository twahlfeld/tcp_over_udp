cmake_minimum_required(VERSION 3.3)
project(tcp_over_udp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES packet.cpp packet.h receiver.cpp receiver.h sender.cpp sender.h tcp.cpp tcp.h ftp.cpp ftp.h)
add_executable(tcp_over_udp ${SOURCE_FILES})