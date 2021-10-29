# How to use:
#   mkdir build
#   cd build
#   cmake ..
#   make
#
cmake_minimum_required(VERSION 3.10)

project(STGCLIENT
        LANGUAGES CXX
        VERSION 0.1.0
)

find_package(gf REQUIRED)

find_package(Threads)

add_executable(stgclient
  main.cc
)

target_link_libraries(stgclient
  PRIVATE
        Threads::Threads
        gf::graphics
        gf::net
)

target_compile_options(stgclient
  PRIVATE
    "-Wall" "-Wextra" "-pedantic" "-g" "-O2"
)

set_target_properties(stgclient
  PROPERTIES
    CXX_STANDARD 17
    CXX_EXTENSIONS OFF
)
