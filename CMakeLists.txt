cmake_minimum_required(VERSION 2.8)
project(StepicServer)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)
add_executable(StepicServer ${SOURCE_FILES})