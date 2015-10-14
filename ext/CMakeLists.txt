include(ExternalProject)

set(ext_INCLUDE ${CMAKE_SOURCE_DIR}/ext/include)
set(ext_LIB ${CMAKE_SOURCE_DIR}/ext/lib)

file(MAKE_DIRECTORY ${ext_INCLUDE})
file(MAKE_DIRECTORY ${ext_LIB})


add_custom_target(
  external ALL
  ${CMAKE_COMMAND} -E echo_append ""
  COMMENT "Build external dependencies"
)

externalProject_add(cudd
  DOWNLOAD_DIR .
  URL "http://www.informatik.uni-bremen.de/revkit/files/cudd-2.5.0-cirkit.tar.gz"
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/ext
  LOG_DOWNLOAD 1
  LOG_BUILD 1
  LOG_INSTALL 1)
add_dependencies(external cudd)

set(CUDD_LIBRARIES
  ${ext_LIB}/libcudd_obj${CMAKE_SHARED_LIBRARY_SUFFIX}
  ${ext_LIB}/libcudd_cudd${CMAKE_SHARED_LIBRARY_SUFFIX}
  ${ext_LIB}/libcudd_mtr${CMAKE_SHARED_LIBRARY_SUFFIX}
  ${ext_LIB}/libcudd_st${CMAKE_SHARED_LIBRARY_SUFFIX}
  ${ext_LIB}/libcudd_epd${CMAKE_SHARED_LIBRARY_SUFFIX}
  ${ext_LIB}/libcudd_util${CMAKE_SHARED_LIBRARY_SUFFIX}
  ${ext_LIB}/libcudd_dddmp${CMAKE_SHARED_LIBRARY_SUFFIX}

  PARENT_SCOPE
)
