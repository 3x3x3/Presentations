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
find_package(RapidJSON)

add_executable(json_parsing
    main.cpp
    ws_mng.cpp
)

target_link_libraries(json_parsing
    ${CMAKE_THREAD_LIBS_INIT}
    OpenSSL::SSL
    websocketpp::websocketpp
)
