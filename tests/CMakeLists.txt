cmake_minimum_required(VERSION 3.16)

# Find Google Test
find_package(GTest QUIET)

if(GTest_FOUND)
    # Create test executable
    add_executable(mi_modeling_tests
        test_main.cpp
        test_dtm.cpp
        test_fitzhugh_nagumo.cpp
        test_cardiac_electrophysiology.cpp
        test_data_processor.cpp
        test_validation_framework.cpp
    )
    
    # Link libraries
    target_link_libraries(mi_modeling_tests GTest::GTest GTest::Main)
    
    # Include directories
    target_include_directories(mi_modeling_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)
    
    # Add tests to CTest
    include(GoogleTest)
    gtest_discover_tests(mi_modeling_tests)
    
    message(STATUS "Google Test found - unit tests enabled")
else()
    message(STATUS "Google Test not found - unit tests disabled")
    
    # Create simple test executable without Google Test
    add_executable(simple_tests
        simple_test_main.cpp
    )
    
    target_include_directories(simple_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)
    
    # Add simple test
    add_test(NAME SimpleTests COMMAND simple_tests)
endif()
