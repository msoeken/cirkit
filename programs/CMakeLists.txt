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
    lscli
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

