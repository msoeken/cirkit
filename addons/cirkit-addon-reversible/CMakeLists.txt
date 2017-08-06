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

if (metaSMT_FOUND)
  add_definitions(-DHAS_METASMT=1)
endif ()

include_directories(lib/xtensor-0.10.9/include)
include_directories(lib/xtensor-blas-0.1.0/include)
include_directories(lib/xtensor-blas-0.1.0/include/xtensor-blas/flens)

add_subdirectory(src)
add_subdirectory(programs)
add_subdirectory(test)
