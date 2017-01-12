include(ExternalProject)

find_package( PythonInterp 2 REQUIRED )

include(ProcessorCount)
ProcessorCount(ProcCount)
if( ProcCount EQUAL 0 )
  set( ProcCount 1 )
endif()

set(ext_INCLUDE ${CMAKE_SOURCE_DIR}/ext/include)
set(ext_LIB ${CMAKE_SOURCE_DIR}/ext/lib)

externalProject_add(z3ep
  DOWNLOAD_DIR .
  URL "http://msoeken.github.io/tools/z3-master.zip"
  CONFIGURE_COMMAND ${PYTHON_EXECUTABLE} scripts/mk_make.py --prefix=${CMAKE_SOURCE_DIR}/ext
  BUILD_COMMAND make -C build -j${ProcCount}
  BUILD_IN_SOURCE 1
  INSTALL_COMMAND make install -C build
)
add_dependencies(external z3ep)

set(Z3_LIBRARIES
  ${ext_LIB}/libz3${CMAKE_SHARED_LIBRARY_SUFFIX}
  )

link_directories(${ext_LIB})

add_cirkit_library(
  NAME cirkit_formal_z3
  AUTO_DIRS src
  USE
    cirkit_classical
    ${Z3_LIBRARIES}
  INCLUDE
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
  DEFINE
    PUBLIC ADDON_FORMAL
  COMMANDS
    formal/cli/commands/commands.hpp
  )
