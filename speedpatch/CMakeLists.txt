cmake_minimum_required(VERSION 3.16)

project(speedpatch LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ..)

add_library(speedpatch SHARED
  speedpatch.cpp
  speedpatch.h
  vcpkg.json
)

target_link_libraries(speedpatch PRIVATE
  minhook
)
target_compile_definitions(speedpatch PRIVATE SPEEDPATCH_LIBRARY)

add_subdirectory(minhook/)
