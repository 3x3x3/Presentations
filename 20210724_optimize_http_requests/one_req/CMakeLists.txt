cmake_minimum_required(VERSION 3.0)
 
project(latency_test)

find_package(CURL REQUIRED)

add_executable(latency_test
    main.cpp
)

target_link_libraries(latency_test
    ${CURL_LIBRARIES}
)
