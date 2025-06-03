cmake_minimum_required(VERSION 3.16)

project(speedpatch LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ..)

# 判断是32位还是64位
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(SUFFIX "64")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(SUFFIX "32")
else()
    set(SUFFIX "")
endif()

# 设置目标名带后缀
set(TARGET_NAME "speedpatch${SUFFIX}")

add_library(${TARGET_NAME} SHARED
  speedpatch.cpp
  speedpatch.h
  vcpkg.json

)

target_link_libraries(${TARGET_NAME} PRIVATE
  minhook
)
target_compile_definitions(${TARGET_NAME} PRIVATE SPEEDPATCH_LIBRARY)
