cmake_minimum_required(VERSION 3.0)
 
project(latency_test)

find_package(CURL REQUIRED)
find_package (Threads)

add_executable(latency_test
    main.cpp
)

target_link_libraries(latency_test
    ${CURL_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)
