cmake_minimum_required(VERSION 2.6)

project(artetra)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall")

set(SOURCE_FILES
    board.h
    board.cpp
    max_heap_board.h
    max_heap_board.cpp
    max_heap_board_move.h
    max_heap_board_move.cpp
    board_insertion_sorter.h
    board_insertion_sorter.cpp
    move_insertion_sorter.h
    move_insertion_sorter.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES} artetra.cpp)
