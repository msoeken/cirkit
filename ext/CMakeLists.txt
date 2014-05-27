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

set(aiger_SRC ${CMAKE_BINARY_DIR}/ext/aiger-prefix/src/aiger)
set(aiger_HEADERS ${aiger_SRC}/aiger.h)
set(aiger_LIBS ${aiger_SRC}/libaiger.so.1.0.1)

externalProject_add(aiger
  DOWNLOAD_DIR .
  URL "http://www.informatik.uni-bremen.de/revkit/files/aiger-1.9.4.tar.gz"
  CONFIGURE_COMMAND sed -i -e "s/CFLAGS=\"/CFLAGS=\"-fPIC /g" configure COMMAND ./configure
  BUILD_COMMAND make COMMAND gcc -shared -Wl,-soname,libaiger.so.1 -o libaiger.so.1.0.1 aiger.o -lc
  BUILD_IN_SOURCE 1
  INSTALL_COMMAND cp ${aiger_HEADERS} ${ext_INCLUDE} COMMAND cp ${aiger_LIBS} ${ext_LIB} COMMAND ldconfig -n ${ext_LIB} COMMAND ln -s ${ext_LIB}/libaiger.so.1.0.1 ${ext_LIB}/libaiger.so
  LOG_DOWNLOAD 1
  LOG_BUILD 1
  LOG_INSTALL 1
)
add_ext_library("libaiger.so")

