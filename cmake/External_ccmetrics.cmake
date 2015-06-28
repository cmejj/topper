set(_source "${CMAKE_SOURCE_DIR}/third_party/ccmetrics")

ExternalProject_Add(ccmetrics_ext
    SOURCE_DIR ${_source}
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/ccmetrics
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
)

ExternalProject_Get_Property(ccmetrics_ext install_dir)
include_directories(${install_dir}/include)
link_directories(${install_dir}/lib)
