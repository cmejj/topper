set(_source "${CMAKE_SOURCE_DIR}/third_party/what-the-event")

ExternalProject_Add(wte_ext
    SOURCE_DIR ${_source}
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/wte
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    BUILD_COMMAND ${CMAKE_COMMAND} --build . --target wte
)

ExternalProject_Get_Property(wte_ext install_dir)
include_directories(${install_dir}/include)
link_directories(${install_dir}/lib)
