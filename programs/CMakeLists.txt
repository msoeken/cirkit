add_cirkit_program(
  NAME aig_to_smt2
  SOURCES
    classical/aig_to_smt2.cpp
  USE
    cirkit_classical
)

add_cirkit_program(
  NAME pretty_print_aig
  SOURCES
    classical/pretty_print_aig.cpp
  USE
    cirkit_classical
)

add_cirkit_program(
  NAME bdd_info
  SOURCES
    core/bdd_info.cpp
  USE
    cirkit_core
)

add_cirkit_program(
  NAME approximate_bdd
  SOURCES
    classical/approximate_bdd.cpp
  USE
    cirkit_classical
)

add_cirkit_program(
  NAME abc_cli
  SOURCES
    core/abc_cli.cpp
  USE
    ${abc_LIBRARIES}
    ${Boost_SYSTEM_LIBRARIES}
    ${Boost_FILESYSTEM_LIBRARIES}
    ${Boost_PROGRAM_OPTIONS_LIBRARIES}
    ${READLINE_LIBRARY}
  INCLUDE
   PUBLIC ${CMAKE_BINARY_DIR}/ext/abc-prefix/src/abc/src
)

add_cirkit_program(
  NAME cirkit
  SOURCES
    core/cirkit.cpp
  USE
    cirkit_core
    cirkit_classical
    linenoise
)

target_compile_definitions( cirkit PUBLIC USE_LINENOISE )
target_link_libraries( cirkit ${cirkit_addon_command_libraries} )

file( WRITE ${CMAKE_BINARY_DIR}/programs/addon_commands.hpp "" )
foreach( inc ${cirkit_addon_command_includes} )
  file( APPEND ${CMAKE_BINARY_DIR}/programs/addon_commands.hpp "${inc}\n" )
endforeach()

file( WRITE ${CMAKE_BINARY_DIR}/programs/addon_defines.hpp "" )
foreach( def ${cirkit_addon_command_defines} )
  file( APPEND ${CMAKE_BINARY_DIR}/programs/addon_defines.hpp "${def}\n" )
endforeach()
target_include_directories( cirkit PUBLIC ${CMAKE_BINARY_DIR}/programs )

# Python API
find_package( PythonLibs 3 )
if( PYTHONLIBS_FOUND AND cirkit_ENABLE_PYTHON_API )
  find_package(pybind11 REQUIRED)
  add_library( cirkit_python SHARED core/cirkit.cpp )
  target_compile_definitions( cirkit_python PUBLIC ALICE_PYTHON )
  set_target_properties( cirkit_python PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED YES )
  set_target_properties( cirkit_python PROPERTIES PREFIX "" )
  set_target_properties( cirkit_python PROPERTIES SUFFIX "" )
  set_target_properties( cirkit_python PROPERTIES OUTPUT_NAME "cirkit.so" )
  target_include_directories( cirkit_python PUBLIC ${PYTHON_INCLUDE_DIRS} ${CMAKE_BINARY_DIR}/programs )
  target_link_libraries( cirkit_python cirkit_core cirkit_classical ${cirkit_addon_command_libraries} ${PYTHON_LIBRARIES} )
endif()
