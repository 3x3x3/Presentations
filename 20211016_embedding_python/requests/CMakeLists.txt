cmake_minimum_required(VERSION 3.1)
project(req_sample)

set (CMAKE_CXX_STANDARD 17)

set(Python_ADDITIONAL_VERSIONS 3.8)
find_package(PythonLibs 3 REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif()

add_executable(req_sample
    main.cpp
)

target_link_libraries(req_sample
    ${PYTHON_LIBRARIES}
)