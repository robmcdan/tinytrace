cmake_minimum_required(VERSION 3.14)
project(tinytrace VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Header-only library
add_library(tinytrace INTERFACE)
target_include_directories(tinytrace INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Enable threading
find_package(Threads REQUIRED)
target_link_libraries(tinytrace INTERFACE Threads::Threads)

# Testing
option(TINYTRACE_BUILD_TESTS "Build tests" ON)
if(TINYTRACE_BUILD_TESTS)
    enable_testing()

    # Fetch Catch2
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.2
    )
    FetchContent_MakeAvailable(Catch2)

    add_subdirectory(tests)
endif()

# Examples
option(TINYTRACE_BUILD_EXAMPLES "Build examples" ON)
if(TINYTRACE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Benchmarks
option(TINYTRACE_BUILD_BENCHMARKS "Build benchmarks" OFF)
if(TINYTRACE_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()
