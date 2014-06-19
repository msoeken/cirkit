include(ExternalProject)

find_package( PythonInterp 2 REQUIRED )

externalProject_add(z3ep
  DOWNLOAD_DIR .
  URL "http://www.informatik.uni-bremen.de/revkit/files/z3-4.3.1.tar.gz"
  CONFIGURE_COMMAND autoconf COMMAND ./configure --with-python=${PYTHON_EXECUTABLE} --prefix=${CMAKE_SOURCE_DIR}/ext COMMAND ${PYTHON_EXECUTABLE} scripts/mk_make.py
  BUILD_COMMAND make -j5 -C build
  BUILD_IN_SOURCE 1
  INSTALL_COMMAND sed "/usr/d" -i build/Makefile COMMAND make install -C build
)

add_ext_library("z3")
