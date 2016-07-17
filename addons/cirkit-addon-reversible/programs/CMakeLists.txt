set(reversible_programs
  adding_lines_optimization
  benchmark_table
  circuit_info
  circuit_to_truth_table
  dd_synthesis
  embed_pla
  example1
  example2
  example3
  example4
  is_identity
  pidd_debugging
  window_optimization)

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
    reversible/calculate_required_lines.cpp
  USE
    cirkit_reversible
    pthread
)

add_cirkit_program(
  NAME revkit
  SOURCES
    reversible/revkit.cpp
  USE
    cirkit_reversible
    cirkit_classical
    ${READLINE_LIBRARY}
    linenoise
)

target_compile_definitions( revkit PUBLIC USE_LINENOISE )

if( enable_cirkit-addon-experimental )
  target_link_libraries( revkit cirkit_experimental_reversible )
  target_compile_definitions( revkit PUBLIC USE_EXPERIMENTAL_REVERSIBLE_COMMANDS )
endif()
