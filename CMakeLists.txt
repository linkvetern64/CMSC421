cmake_minimum_required(VERSION 3.6)
project(CMSC421)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    hw1/hw1.c
    hw2/player.c
    hw2/team.c)

add_executable(CMSC421 ${SOURCE_FILES})