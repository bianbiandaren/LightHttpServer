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

# 新增对方的严格编译警告
target_compile_options(
    LightHttpServer
    PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

# 保留Windows网络兼容
if(WIN32)
    target_link_libraries(LightHttpServer ws2_32)
endif()

# Security test targets
add_executable(handle_flood_test tests/handle_flood_test.cpp)
add_executable(half_packet_test tests/half_packet_test.cpp)
add_executable(slowloris_test tests/slowloris_test.cpp)
add_executable(request_smuggling_test tests/request_smuggling_test.cpp)
add_executable(directory_traversal_test tests/directory_traversal_test.cpp)

set(TEST_TARGETS
    handle_flood_test
    half_packet_test
    slowloris_test
    request_smuggling_test
    directory_traversal_test
)

foreach(target IN LISTS TEST_TARGETS)
    target_compile_features(${target} PRIVATE cxx_std_17)
    if(WIN32)
        target_link_libraries(${target} ws2_32)
    endif()
endforeach()