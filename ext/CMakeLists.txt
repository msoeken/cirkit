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
  URL "http://www.informatik.uni-bremen.de/revkit/files/cudd-2.5.0-revkit.tar.gz"
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/ext
  LOG_DOWNLOAD 1
  LOG_BUILD 1
  LOG_INSTALL 1)
add_dependencies(external cudd)

set(CUDD_LIBRARIES
  ${ext_LIB}/libcudd_obj.so
  ${ext_LIB}/libcudd_cudd.so
  ${ext_LIB}/libcudd_mtr.so
  ${ext_LIB}/libcudd_st.so
  ${ext_LIB}/libcudd_epd.so
  ${ext_LIB}/libcudd_util.so
  ${ext_LIB}/libcudd_dddmp.so
  ${ext_LIB}/libcudd_nanotrav.so

  PARENT_SCOPE
)

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
add_dependencies(external puma)

set(PUMA_LIBRARIES
  ${ext_LIB}/libpuma.so
  PARENT_SCOPE)

set(aiger_SRC ${CMAKE_BINARY_DIR}/ext/aiger-prefix/src/aiger)
set(aiger_HEADERS ${aiger_SRC}/aiger.h)
set(aiger_LIBS ${aiger_SRC}/libaiger.so.1.0.1)

externalProject_add(aiger
  DOWNLOAD_DIR .
  URL "http://www.informatik.uni-bremen.de/revkit/files/aiger-1.9.4.tar.gz"
  CONFIGURE_COMMAND sed -i -e "s/CFLAGS=\"/CFLAGS=\"-fPIC /g" configure COMMAND ./configure
  BUILD_COMMAND make COMMAND gcc -shared -Wl,-soname,libaiger.so.1 -o libaiger.so.1.0.1 aiger.o -lc
  BUILD_IN_SOURCE 1
  INSTALL_COMMAND cp ${aiger_HEADERS} ${ext_INCLUDE} COMMAND cp ${aiger_LIBS} ${ext_LIB} COMMAND ldconfig -n ${ext_LIB} COMMAND ln -fs ${ext_LIB}/libaiger.so.1.0.1 ${ext_LIB}/libaiger.so
  LOG_DOWNLOAD 1
  LOG_BUILD 1
  LOG_INSTALL 1
)
add_dependencies(external aiger)

externalProject_add(igraphex
  DOWNLOAD_DIR .
  URL "http://igraph.org/nightly/get/c/igraph-0.7.1.tar.gz"
  CONFIGURE_COMMAND ./configure --prefix=${CMAKE_SOURCE_DIR}/ext
  BUILD_COMMAND make -j5
  BUILD_IN_SOURCE 1
  INSTALL_COMMAND make install
  LOG_DOWNLOAD 1
  LOG_BUILD 1
  LOG_INSTALL 1)
add_dependencies(external igraphex)

set(IGRAPH_LIBRARIES "${ext_LIB}/libigraph.so" PARENT_SCOPE)

