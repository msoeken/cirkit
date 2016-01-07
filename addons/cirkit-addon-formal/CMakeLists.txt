include(ExternalProject)

find_package( PythonInterp 2 REQUIRED )

set(ext_INCLUDE ${CMAKE_SOURCE_DIR}/ext/include)
set(ext_LIB ${CMAKE_SOURCE_DIR}/ext/lib)

externalProject_add(z3ep
  DOWNLOAD_DIR .
  URL "http://msoeken.github.io/tools/z3-4.3.2-github-a7db1c9d76.tar.gz"
  CONFIGURE_COMMAND ${PYTHON_EXECUTABLE} scripts/mk_make.py --prefix=${CMAKE_SOURCE_DIR}/ext
  BUILD_COMMAND make -C build
  BUILD_IN_SOURCE 1
  INSTALL_COMMAND make install -C build
)
add_dependencies(external z3ep)

set(Z3_LIBRARIES
  ${ext_LIB}/libz3${CMAKE_SHARED_LIBRARY_SUFFIX}
  )

# minisat
set(minisat_SRC ${CMAKE_BINARY_DIR}/addons/cirkit-addon-formal/minisat-prefix/src/minisat)
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
)

link_directories(${ext_LIB})

add_cirkit_library(
  NAME cirkit_formal
  AUTO_DIRS src
  USE
    cirkit_core
    ${Z3_LIBRARIES}
    ${MINISAT_LIBRARIES}
  INCLUDE
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
)
