cmake_minimum_required(VERSION 3.16)

project(LightHttpServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB SRC_FILES src/*.cpp)

add_executable(LightHttpServer ${SRC_FILES})

target_include_directories(
    LightHttpServer
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_compile_options(
    LightHttpServer
    PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
