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
    libabc-pic
    ${Boost_SYSTEM_LIBRARIES}
    ${Boost_FILESYSTEM_LIBRARIES}
    ${Boost_PROGRAM_OPTIONS_LIBRARIES}
    ${READLINE_LIBRARY}
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
target_link_libraries( cirkit cirkit_cli ${cirkit_addon_command_libraries} )

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
if( cirkit_ENABLE_PYTHON_API )
  find_package(pybind11 REQUIRED)
  add_library( cirkit_python MODULE core/cirkit.cpp )
  target_compile_definitions( cirkit_python PUBLIC ALICE_PYTHON )
  set_target_properties(cirkit_python PROPERTIES PREFIX ""
                                                 SUFFIX ""
                                                 OUTPUT_NAME "cirkit.so")
  target_include_directories( cirkit_python PUBLIC ${CMAKE_BINARY_DIR}/programs )
  target_link_libraries( cirkit_python PUBLIC cirkit_cli ${cirkit_addon_command_libraries} PRIVATE pybind11::module )
endif()
