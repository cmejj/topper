project(examples CXX)

# Set includes
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

# Hello world server
add_executable(hello_server
    hello_server.cc
)

target_link_libraries(hello_server
    topper
    pthread
)
