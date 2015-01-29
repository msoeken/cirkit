include(ExternalProject)

set(ext_FUNCTIONS ${CMAKE_SOURCE_DIR}/ext/functions)
set(ext_CIRCUITS ${CMAKE_SOURCE_DIR}/ext/circuits)

file(MAKE_DIRECTORY ${ext_FUNCTIONS})
file(MAKE_DIRECTORY ${ext_CIRCUITS})

externalProject_add(revlib_functions
  DOWNLOAD_DIR .
  URL "http://revlib.org/doc/func/func.tgz"
  CONFIGURE_COMMAND cp . ${ext_FUNCTIONS} -R
  BUILD_IN_SOURCE 1
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  LOG_DOWNLOAD 1
  LOG_BUILD 1
  LOG_INSTALL 1)

externalProject_add(revlib_circuits
  DOWNLOAD_DIR .
  URL "http://revlib.org/doc/real/reals.tgz"
  CONFIGURE_COMMAND cp . ${ext_CIRCUITS} -R
  BUILD_IN_SOURCE 1
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  LOG_DOWNLOAD 1
  LOG_BUILD 1
  LOG_INSTALL 1)

file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/src/reversible)
