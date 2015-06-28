project(tests CXX)

# Set includes
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/test
    ${gtest_INCLUDE_DIRS}
)

add_executable(test
    driver.cc
    resource_test.cc
    resource_matcher_test.cc
    server_test.cc
    util.cc
    util_test.cc
)

target_link_libraries(test
    gtest
    topper
    pthread
)
