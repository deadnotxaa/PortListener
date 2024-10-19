cmake_minimum_required(VERSION 3.29)
project(PortKnocker)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OSX_ARCHITECTURES x86_64)

# Boost library | Boost.Asio
set(BOOST_ROOT /usr/include/boost)
set(Boost_USE_STATIC_LIBS ON)

find_package(Boost)
include_directories(${Boost_INCLUDE_DIR})

add_subdirectory(bin)
add_subdirectory(lib)
