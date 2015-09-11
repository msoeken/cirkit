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
