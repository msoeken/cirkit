set(reversible_programs
  adding_lines_optimization
  benchmark_table
  circuit_info
  circuit_to_truth_table
  dd_synthesis
  embed_pla
  exact_synthesis
  example1
  example2
  example3
  example4
  is_identity
  negative_controls_to_positive
  permutation_to_pla
  pidd_debugging
  random_reversible_circuit
  rcbdd_synthesis
  reed_muller_synthesis
  transform_circuit
  transformation_based_synthesis
  window_optimization
  young_subgroup_synthesis)

foreach(program ${reversible_programs})
  add_cirkit_program(
    NAME ${program}
    SOURCES
      reversible/${program}.cpp
    USE
      cirkit_reversible
  )
endforeach()

if( metaSMT_FOUND )
  add_cirkit_program(
    NAME equivalence_check
    SOURCES
      reversible/equivalence_check.cpp
    USE
      cirkit_reversible
      cirkit_formal
  )
endif()

add_cirkit_program(
  NAME calculate_required_lines
  SOURCES
    reversible/calculate_required_lines
  USE
    cirkit_reversible
    pthread
)
