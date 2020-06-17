cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)

project(chronos)

# Enable test coverage for Debug build
if (CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_CXX_FLAGS "-fprofile-arcs -ftest-coverage")
endif()

# Conan setup
if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
else()
    message(WARNING "The file conanbuildinfo.cmake doesn't exist")
endif()

find_package(Boost 1.73 REQUIRED)

include_directories(include)
include_directories(src)

# Executables
if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_executable(tests tests/tests.cpp)
endif()