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
    cirkit_reversible_cli
    cirkit_classical
    ${READLINE_LIBRARY}
    linenoise
)

target_compile_definitions( revkit PUBLIC USE_LINENOISE )

# Python API
if( cirkit_ENABLE_PYTHON_API )
  add_alice_python_module(revkit reversible/revkit.cpp )
  target_link_libraries( revkit_python PUBLIC cirkit_reversible_cli )
  set_target_properties( revkit_python PROPERTIES PREFIX ""
                                                  SUFFIX ""
                                                  OUTPUT_NAME "revkit.so")
endif()
