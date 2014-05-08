include(ExternalProject)

set(ext_INCLUDE ${CMAKE_SOURCE_DIR}/ext/include)
set(ext_LIB ${CMAKE_SOURCE_DIR}/ext/lib)

file(MAKE_DIRECTORY ${ext_INCLUDE})
file(MAKE_DIRECTORY ${ext_LIB})

externalProject_add(cudd
  DOWNLOAD_DIR .
  URL "http://www.informatik.uni-bremen.de/revkit/files/cudd-2.5.0-revkit.tar.gz"
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/ext
  LOG_DOWNLOAD 1
  LOG_BUILD 1
  LOG_INSTALL 1)
add_ext_library("libcudd_cudd.so;libcudd_dddmp.so;libcudd_epd.so;libcudd_mtr.so;libcudd_nanotrav.so;libcudd_obj.so;libcudd_st.so;libcudd_util.so")

set(puma_SRC ${CMAKE_BINARY_DIR}/ext/puma-prefix/src/puma)
set(puma_HEADERS
  ${puma_SRC}/include/puma.h
  ${puma_SRC}/include/tc_time.h)
set(puma_LIBS
  ${puma_SRC}/lib/libpuma.so)

externalProject_add(puma
  DOWNLOAD_DIR .
  URL "http://www.informatik.uni-bremen.de/revkit/files/puma-2.24.tar.gz"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND make puma -C bin
  BUILD_IN_SOURCE 1
  INSTALL_COMMAND cp ${puma_HEADERS} ${ext_INCLUDE} COMMAND cp ${puma_LIBS} ${ext_LIB}
  LOG_DOWNLOAD 1
  LOG_BUILD 1
  LOG_INSTALL 1)
add_ext_library("libpuma.so")
