set(_source "${CMAKE_SOURCE_DIR}/third_party/google-glog")
set(_build "${CMAKE_CURRENT_BINARY_DIR}/google-glog")

ExternalProject_Add(glog_ext
    SOURCE_DIR ${_source}
    BINARY_DIR ${_build}
    CONFIGURE_COMMAND ${_source}/configure --prefix=${_build} --with-pic
    BUILD_COMMAND make
    INSTALL_COMMAND make install
)

include_directories("${_build}/include")
link_directories("${_build}")

set(glog_STATIC_LIB ${_build}/lib/libglog.a)
