include(ExternalProject)

find_package( PythonInterp 2 REQUIRED )

set(ext_LIB ${CMAKE_SOURCE_DIR}/ext/lib)

externalProject_add(z3ep
  DOWNLOAD_DIR .
  URL "http://www.informatik.uni-bremen.de/revkit/files/z3-4.3.2-github-a7db1c9d76.tar.gz"
  CONFIGURE_COMMAND ${PYTHON_EXECUTABLE} scripts/mk_make.py --prefix=${CMAKE_SOURCE_DIR}/ext
  BUILD_COMMAND make -j5 -C build
  BUILD_IN_SOURCE 1
  INSTALL_COMMAND sed "/usr/d" -i build/Makefile COMMAND make install -C build
)
add_dependencies(external z3ep)

set(Z3_LIBRARIES
  ${ext_LIB}/libz3.so
)

link_directories(${ext_LIB})

add_cirkit_library(
  NAME cirkit_formal
  AUTO_DIRS src
  USE
    cirkit_core
    ${Z3_LIBRARIES}
  INCLUDE
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
)