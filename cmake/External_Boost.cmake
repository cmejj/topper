find_package(Boost 1.55.0)

# We only use the Boost headers
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()
