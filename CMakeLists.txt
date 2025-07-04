cmake_minimum_required(VERSION 3.10)
project(keyHearing)

add_subdirectory(portaudio)  
include_directories(portaudio/include)

add_executable(keyHearing main.cpp)

target_link_libraries(keyHearing PRIVATE portaudio)
