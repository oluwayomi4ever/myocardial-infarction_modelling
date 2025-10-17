cmake_minimum_required(VERSION 3.16)
project(MI_Modeling_Cpp_Project)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/main.cpp
    src/DTM.cpp
    src/FitzHughNagumo.cpp
    src/CardiacElectrophysiology.cpp
    src/DataProcessor.cpp
    src/ValidationFramework.cpp
)

# Header files
set(HEADERS
    include/DTM.h
    include/FitzHughNagumo.h
    include/CardiacElectrophysiology.h
    include/DataProcessor.h
    include/ValidationFramework.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Find required packages
find_package(OpenMP QUIET)
if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_CXX)
    message(STATUS "OpenMP found - parallel processing enabled")
endif()

# Compiler flags
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -O3 -march=native)
endif()

# Additional compiler-specific optimizations
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE -funroll-loops -ffast-math)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -funroll-loops -ffast-math)
endif()

# Enable testing
enable_testing()

# Add tests subdirectory if it exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    add_subdirectory(tests)
endif()

# Print configuration summary
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")

