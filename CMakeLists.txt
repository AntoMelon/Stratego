# How to use:
#   mkdir build
#   cd build
#   cmake ..
#   make
#
cmake_minimum_required(VERSION 3.10)

project(STG
        LANGUAGES CXX
        VERSION 0.1.0
)

find_package(gf REQUIRED)

find_package(Threads)

add_executable(stgclient
  src/client/main.cc
        src/common/protocol.cpp src/common/protocol.h
        src/common/piece.cc src/common/piece.h src/client/libs/board.h src/client/libs/board.cpp src/client/libs/board.h)

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

add_executable(stgserver
        src/server/main.cc
        src/common/protocol.cpp src/common/protocol.h
        src/server/server_board.cc src/server/server_board.h
        src/common/piece.cc src/common/piece.h
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
