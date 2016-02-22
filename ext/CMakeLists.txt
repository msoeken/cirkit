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

set( cudd_SRC ${CMAKE_BINARY_DIR}/ext/cudd-prefix/src/cudd)
set( cudd_INCLUDE_FILES
  ${cudd_SRC}/config.h
  ${cudd_SRC}/cudd/cudd.h
  ${cudd_SRC}/cudd/cuddInt.h
  ${cudd_SRC}/cplusplus/cuddObj.hh
  ${cudd_SRC}/epd/epd.h
  ${cudd_SRC}/epd/epdInt.h
  ${cudd_SRC}/mtr/mtr.h
  ${cudd_SRC}/mtr/mtrInt.h
  ${cudd_SRC}/st/st.h
  ${cudd_SRC}/util/cstringstream.h
  ${cudd_SRC}/util/util.h
  )

if(APPLE)
  set( cudd_LIBRARY_FILES
    ${cudd_SRC}/cudd/.libs/libcudd.dylib
    ${cudd_SRC}/cudd/.libs/libcudd-3.0.0.0.dylib
  )
else()
  set( cudd_LIBRARY_FILES
    ${cudd_SRC}/cudd/.libs/libcudd.so
    ${cudd_SRC}/cudd/.libs/libcudd-3.0.0.so.0
    ${cudd_SRC}/cudd/.libs/libcudd-3.0.0.so.0.0.0
  )
endif()

include(ProcessorCount)
ProcessorCount(ProcCount)
if( ProcCount EQUAL 0 )
  set( ProcCount 1 )
endif()

externalProject_add(cudd
  DOWNLOAD_DIR .
  URL "http://msoeken.github.io/tools/cudd-3.0.0.tar.gz"
  CONFIGURE_COMMAND ./configure --enable-obj --enable-shared --enable-dddmp
  BUILD_COMMAND make -j${ProcCount}
  INSTALL_COMMAND cp ${cudd_INCLUDE_FILES} ${ext_INCLUDE} COMMAND cp -L ${cudd_LIBRARY_FILES} ${ext_LIB}
  BUILD_IN_SOURCE 1
  LOG_DOWNLOAD 1
  LOG_BUILD 1
  LOG_INSTALL 1)
add_dependencies(external cudd)

set(CUDD_LIBRARIES
  ${ext_LIB}/libcudd${CMAKE_SHARED_LIBRARY_SUFFIX}

  PARENT_SCOPE
)

# abc
set(abc_SRC ${CMAKE_BINARY_DIR}/ext/abc-prefix/src/abc)
externalProject_add(abc
  DOWNLOAD_DIR .
  URL "http://msoeken.github.io/tools/alanmi-abc-4128aecc95d3.zip"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND make -j${ProcCount} ABC_USE_NO_CUDD=1 ABC_USE_PIC=1 ABC_USE_NO_READLINE=1 ABC_USE_NAMESPACE=abc OPTFLAGS=-O3 libabc.a
  BUILD_IN_SOURCE 1
  INSTALL_COMMAND rsync --include "*.h" --filter "hide,! */" -avm ${abc_SRC}/src/ ${ext_INCLUDE}/abc COMMAND cp ${abc_SRC}/libabc.a ${ext_LIB}/libabc.a
  LOG_DOWNLOAD 1
  LOG_BUILD 1
  LOG_INSTALL 1)
add_dependencies(external abc)

find_library(DL_LIBRARY dl)
set(abc_LIBRARIES
  ${ext_LIB}/libabc.a
  ${DL_LIBRARY}
  pthread
  ${READLINE_LIBRARY}

  PARENT_SCOPE
)
