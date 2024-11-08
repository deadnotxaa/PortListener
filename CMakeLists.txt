cmake_minimum_required(VERSION 3.12)
project(PortKnocker)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall")

option(BUILD_SHARED_LIBS "Build without shared libraries" OFF)

if (APPLE)
    set(CMAKE_OSX_ARCHITECTURES arm64)
elseif (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -lstdc++fs -lwsock32 -lws2_32")
    set(CMAKE_OSX_ARCHITECTURES x86_64)
endif ()

# Boost library | Boost.Asio
set (Boost_NO_SYSTEM_PATHS ON)
set (Boost_USE_MULTITHREADED ON)
set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_STATIC_RUNTIME ON)
set (BOOST_ALL_DYN_LINK OFF)

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

add_subdirectory(bin)
add_subdirectory(lib)
