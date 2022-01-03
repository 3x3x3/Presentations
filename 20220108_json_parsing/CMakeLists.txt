cmake_minimum_required(VERSION 3.1)
project(json_parsing)

set (CMAKE_CXX_STANDARD 17)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif()

find_package(OpenSSL REQUIRED)
find_package(Threads)
find_package(websocketpp REQUIRED)

add_executable(upbit_watching
    main.cpp
)

target_link_libraries(upbit_watching
    OpenSSL::SSL
    websocketpp::websocketpp
)
