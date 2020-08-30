cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)

project(chronos)

# Enable test coverage for Debug build
if (CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_CXX_FLAGS "-fprofile-arcs -ftest-coverage")
endif()

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_run(
        REQUIRES
            boost/1.73.0
            catch2/2.13.0
            fmt/7.0.2
            spdlog/[>=1.4.1]
        OPTIONS
            boost:header_only=True
            fmt:header_only=True
            spdlog:header_only=True
        BASIC_SETUP)

find_package(Boost 1.73 REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories(src)
include_directories(${CONAN_INCLUDE_DIRS})

# Executables
if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_executable(simulation tests/simulation.cpp)
endif()

add_executable(chronos src/Chronos.cpp)
add_executable(tests tests/tests.cpp)
target_link_libraries(chronos PRIVATE Threads::Threads stdc++fs)