project(libtopper-library CXX)

# Define the public include directories
set(LIBTOPPER_PUBLIC_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/include
    CACHE INTERNAL "libtopper public include directories"
)

# The shared library target
set(LIBTOPPER_SHARED_LIBRARY
    topper
    CACHE INTERNAL "topper shared library"
)

# Source translation units
set(libtopper_SRCS
    entity.cc
    parameter.cc
    request.cc
    resource_matcher.cc
    response.cc
    server.cc
)

# Set the include directories
include_directories(
    ${LIBTOPPER_PUBLIC_INCLUDE_DIRS}
)

# Main library targets
add_library(${LIBTOPPER_SHARED_LIBRARY} SHARED ${libtopper_SRCS})

# Apple frameworks
IF(APPLE)
    INCLUDE_DIRECTORIES ( /Developer/Headers/FlatCarbon )
    FIND_LIBRARY(CARBON_LIBRARY Carbon)
    FIND_LIBRARY(APP_SERVICES_LIBRARY ApplicationServices )
    MARK_AS_ADVANCED (CARBON_LIBRARY
                    APP_SERVICES_LIBRARY)
    SET(EXTRA_LIBS
        ${CARBON_LIBRARY}
        ${APP_SERVICES_LIBRARY})
ENDIF (APPLE)

# Link dependencies
target_link_libraries(${LIBTOPPER_SHARED_LIBRARY}
    http_parser
    ${glog_STATIC_LIB}
    wte
    ${EXTRA_LIBS}
)

# Installation
install(TARGETS
    ${LIBTOPPER_SHARED_LIBRARY}
    DESTINATION lib
    EXPORT topper-targets)
install(EXPORT topper-targets DESTINATION lib)

install(DIRECTORY
    ${CMAKE_SOURCE_DIR}/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
