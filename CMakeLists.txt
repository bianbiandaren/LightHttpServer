cmake_minimum_required(VERSION 3.16)

project(LightHttpServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(LightHttpServer
    src/main.cpp
    src/Server.cpp
    src/Config.cpp
    src/HttpRequest.cpp
    src/HttpResponse.cpp
    src/MineType.cpp
    src/Utils.cpp
)

target_include_directories(LightHttpServer
    PRIVATE ${PROJECT_SOURCE_DIR}/include
)

if(WIN32)
    target_link_libraries(LightHttpServer ws2_32)
endif()