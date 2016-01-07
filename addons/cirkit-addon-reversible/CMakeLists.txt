include(ExternalProject)

set(ext_LIB ${CMAKE_SOURCE_DIR}/ext/lib)
set(ext_INCLUDE ${CMAKE_SOURCE_DIR}/ext/include)
set(ext_FUNCTIONS ${CMAKE_SOURCE_DIR}/ext/functions)
set(ext_CIRCUITS ${CMAKE_SOURCE_DIR}/ext/circuits)

file(MAKE_DIRECTORY ${ext_FUNCTIONS})
file(MAKE_DIRECTORY ${ext_CIRCUITS})

externalProject_add(revlib_functions
  DOWNLOAD_DIR .
  URL "http://revlib.org/doc/func/func.tgz"
  CONFIGURE_COMMAND cp -R . ${ext_FUNCTIONS}
  BUILD_IN_SOURCE 1
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  LOG_DOWNLOAD 1
  LOG_BUILD 1
  LOG_INSTALL 1)
add_dependencies(external revlib_functions)

externalProject_add(revlib_circuits
  DOWNLOAD_DIR .
  URL "http://revlib.org/doc/real/reals.tgz"
  CONFIGURE_COMMAND cp -R . ${ext_CIRCUITS}
  BUILD_IN_SOURCE 1
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  LOG_DOWNLOAD 1
  LOG_BUILD 1
  LOG_INSTALL 1)
add_dependencies(external revlib_circuits)

set(puma_SRC ${CMAKE_BINARY_DIR}/addons/cirkit-addon-reversible/puma-prefix/src/puma)
set(puma_HEADERS
  ${puma_SRC}/include/puma.h
  ${puma_SRC}/include/tc_time.h)
set(puma_LIBS
  ${puma_SRC}/lib/libpuma.so)

externalProject_add(puma
  DOWNLOAD_DIR .
  URL "http://msoeken.github.io/tools/puma-2.24.tar.gz"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND make puma -C bin
  BUILD_IN_SOURCE 1
  INSTALL_COMMAND cp ${puma_HEADERS} ${ext_INCLUDE} COMMAND cp ${puma_LIBS} ${ext_LIB}
  LOG_DOWNLOAD 1
  LOG_BUILD 1
  LOG_INSTALL 1)
add_dependencies(external puma)

set(PUMA_LIBRARIES
  ${ext_LIB}/libpuma.so)

if (metaSMT_FOUND)
  add_definitions(-DHAS_METASMT=1)
endif ()

add_subdirectory(src)
add_subdirectory(programs)
add_subdirectory(test)
