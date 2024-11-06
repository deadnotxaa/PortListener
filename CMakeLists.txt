cmake_minimum_required(VERSION 3.12)
project(PortKnocker)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (APPLE)
    set(CMAKE_OSX_ARCHITECTURES arm64)
elseif (WIN32)
#    TODO: add boost root for win32
#    set(BOOST_ROOT /usr/include/boost)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lstdc++fs -lwsock32")
    set(CMAKE_OSX_ARCHITECTURES x86_64)
endif ()

# Boost library | Boost.Asio
set(Boost_USE_STATIC_LIBS ON)

find_package(Boost)
include_directories(${Boost_INCLUDE_DIR})

add_subdirectory(bin)
add_subdirectory(lib)
