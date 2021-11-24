# How to use:
#   mkdir build
#   cd build
#   cmake ..
#   make
#
cmake_minimum_required(VERSION 3.10)

project(STGSERVER
        LANGUAGES CXX
        VERSION 0.1.0
        )

find_package(gf REQUIRED)

add_executable(stgserver
        main.cc
        ../common/protocol.cpp ../common/protocol.h
        )

target_link_libraries(stgserver
  PRIVATE
        gf::net
)

target_compile_options(stgserver
        PRIVATE
        "-Wall" "-Wextra" "-pedantic" "-g" "-O2"
        )

set_target_properties(stgserver
        PROPERTIES
        CXX_STANDARD 17
        CXX_EXTENSIONS OFF
        )