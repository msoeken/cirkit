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
    PUBLIC ${CMAKE_SOURCE_DIR}/ext/include/abc
)

# cirkit
add_cirkit_program(
  NAME cirkit
  SOURCES
    core/cirkit.cpp
  USE
    cirkit_core
    cirkit_classical
)

if( enable_cirkit-addon-experimental )
  target_link_libraries( cirkit cirkit_experimental_core cirkit_experimental_classical )
  target_compile_definitions( cirkit PUBLIC USE_EXPERIMENTAL_CLASSICAL_COMMANDS )

  if ( enable_cirkit-addon-formal )
    target_link_libraries( cirkit cirkit_experimental_formal )
    target_compile_definitions( cirkit PUBLIC USE_EXPERIMENTAL_FORMAL_COMMANDS )
  endif()
endif()
