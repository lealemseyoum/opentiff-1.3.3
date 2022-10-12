# build test files
set(TEST_SRCS test.cpp)
add_executable(test_app ${TEST_SRCS})
target_link_libraries(test_app PUBLIC Opentiff)
target_include_directories(test_app PUBLIC ${OPENTIFF_INCLUDE_DIRS})

# use ctest
enable_testing()
add_test(NAME MainTest COMMAND test_app 
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})