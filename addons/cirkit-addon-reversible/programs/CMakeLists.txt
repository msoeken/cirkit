# Small programs

set(reversible_programs
  dd_synthesis
  embed_pla
  example1
  example2
  example3
  example4
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

# RevKit

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
