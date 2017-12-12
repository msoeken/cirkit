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
# MiniSAT                                                                      #
################################################################################

# set(minisat_SRC ${CMAKE_BINARY_DIR}/ext/minisat-prefix/src/minisat)
# set(minisat_utils_HEADERS
#   ${minisat_SRC}/utils/System.h
#   ${minisat_SRC}/utils/ParseUtils.h
#   ${minisat_SRC}/utils/Options.h)
# set(minisat_mtl_HEADERS
#   ${minisat_SRC}/mtl/XAlloc.h
#   ${minisat_SRC}/mtl/Vec.h
#   ${minisat_SRC}/mtl/Sort.h
#   ${minisat_SRC}/mtl/Queue.h
#   ${minisat_SRC}/mtl/Map.h
#   ${minisat_SRC}/mtl/IntTypes.h
#   ${minisat_SRC}/mtl/Heap.h
#   ${minisat_SRC}/mtl/Alloc.h
#   ${minisat_SRC}/mtl/Alg.h)
# set(minisat_core_HEADERS
#   ${minisat_SRC}/core/SolverTypes.h
#   ${minisat_SRC}/core/Solver.h
#   ${minisat_SRC}/core/Dimacs.h)
# externalProject_add( minisat
#   DOWNLOAD_DIR .
#   URL "http://msoeken.github.io/tools/minisat-2.2.0-clang.tar.gz"
#   CONFIGURE_COMMAND ""
#   BUILD_COMMAND make -C core libr MROOT=${minisat_SRC} COPTIMIZE=-fPIC\ -O3
#   BUILD_IN_SOURCE 1
#   INSTALL_COMMAND mkdir -p ${ext_INCLUDE}/utils ${ext_INCLUDE}/mtl ${ext_INCLUDE}/core COMMAND cp ${minisat_utils_HEADERS} ${ext_INCLUDE}/utils COMMAND cp ${minisat_mtl_HEADERS} ${ext_INCLUDE}/mtl COMMAND cp ${minisat_core_HEADERS} ${ext_INCLUDE}/core COMMAND cp ${minisat_SRC}/core/lib.a ${ext_LIB}/libminisat.a
#   LOG_DOWNLOAD 1
#   LOG_BUILD 1
#   LOG_INSTALL 1)
# add_dependencies(external minisat)

# set(MINISAT_LIBRARIES
#   ${ext_LIB}/libminisat.a

#   PARENT_SCOPE
# )
