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

################################################################################
# Cudd                                                                         #
################################################################################

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
    ${cudd_SRC}/cudd/.libs/libcudd.a
#    ${cudd_SRC}/cudd/.libs/libcudd.dylib
#    ${cudd_SRC}/cudd/.libs/libcudd-3.0.0.0.dylib
  )
  set( cudd_BUILD_TYPE static )
else()
  set( cudd_LIBRARY_FILES
    ${cudd_SRC}/cudd/.libs/libcudd.so
    ${cudd_SRC}/cudd/.libs/libcudd-3.0.0.so.0
    ${cudd_SRC}/cudd/.libs/libcudd-3.0.0.so.0.0.0
  )
  set( cudd_BUILD_TYPE shared )
endif()

include(ProcessorCount)
ProcessorCount(ProcCount)
if( ProcCount EQUAL 0 )
  set( ProcCount 1 )
endif()

externalProject_add(cudd
  DOWNLOAD_DIR .
  URL "http://msoeken.github.io/tools/cudd-3.0.0.tar.gz"
  CONFIGURE_COMMAND ./configure --enable-obj --enable-${cudd_BUILD_TYPE} --enable-dddmp
  BUILD_COMMAND make -j${ProcCount}
  INSTALL_COMMAND cp ${cudd_INCLUDE_FILES} ${ext_INCLUDE} COMMAND cp -L ${cudd_LIBRARY_FILES} ${ext_LIB}
  BUILD_IN_SOURCE 1
  LOG_DOWNLOAD 1
  LOG_BUILD 1
  LOG_INSTALL 1)
add_dependencies(external cudd)

if(APPLE)
  set(CUDD_LIBRARIES
    ${ext_LIB}/libcudd.a
    PARENT_SCOPE
  )
else()
  set(CUDD_LIBRARIES
    ${ext_LIB}/libcudd.so
    PARENT_SCOPE
  )
endif()

################################################################################
# ABC                                                                          #
################################################################################

set(abc_SRC ${CMAKE_BINARY_DIR}/ext/abc-prefix/src/abc)
externalProject_add(abc
  DOWNLOAD_DIR .
  URL "http://msoeken.github.io/tools/alanmi-abc-0b9cf5a21abd.zip"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND make -j${ProcCount} ABC_USE_PIC=1 ABC_USE_NO_READLINE=1 ABC_USE_NAMESPACE=abc OPTFLAGS=-O3 libabc.a
  BUILD_IN_SOURCE 1
  INSTALL_COMMAND cp ${abc_SRC}/libabc.a ${ext_LIB}/libabc.a
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

################################################################################
# MiniSAT                                                                      #
################################################################################

set(minisat_SRC ${CMAKE_BINARY_DIR}/ext/minisat-prefix/src/minisat)
set(minisat_utils_HEADERS
  ${minisat_SRC}/utils/System.h
  ${minisat_SRC}/utils/ParseUtils.h
  ${minisat_SRC}/utils/Options.h)
set(minisat_mtl_HEADERS
  ${minisat_SRC}/mtl/XAlloc.h
  ${minisat_SRC}/mtl/Vec.h
  ${minisat_SRC}/mtl/Sort.h
  ${minisat_SRC}/mtl/Queue.h
  ${minisat_SRC}/mtl/Map.h
  ${minisat_SRC}/mtl/IntTypes.h
  ${minisat_SRC}/mtl/Heap.h
  ${minisat_SRC}/mtl/Alloc.h
  ${minisat_SRC}/mtl/Alg.h)
set(minisat_core_HEADERS
  ${minisat_SRC}/core/SolverTypes.h
  ${minisat_SRC}/core/Solver.h
  ${minisat_SRC}/core/Dimacs.h)
externalProject_add( minisat
  DOWNLOAD_DIR .
  URL "http://msoeken.github.io/tools/minisat-2.2.0-clang.tar.gz"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND make -C core libr MROOT=${minisat_SRC} COPTIMIZE=-fPIC\ -O3
  BUILD_IN_SOURCE 1
  INSTALL_COMMAND mkdir -p ${ext_INCLUDE}/utils ${ext_INCLUDE}/mtl ${ext_INCLUDE}/core COMMAND cp ${minisat_utils_HEADERS} ${ext_INCLUDE}/utils COMMAND cp ${minisat_mtl_HEADERS} ${ext_INCLUDE}/mtl COMMAND cp ${minisat_core_HEADERS} ${ext_INCLUDE}/core COMMAND cp ${minisat_SRC}/core/lib.a ${ext_LIB}/libminisat.a
  LOG_DOWNLOAD 1
  LOG_BUILD 1
  LOG_INSTALL 1)
add_dependencies(external minisat)

set(MINISAT_LIBRARIES
  ${ext_LIB}/libminisat.a

  PARENT_SCOPE
)
