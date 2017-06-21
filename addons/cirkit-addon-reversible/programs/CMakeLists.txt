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

# Python API
find_package( PythonLibs 3 )
if( PYTHONLIBS_FOUND AND cirkit_ENABLE_PYTHON_API )
  find_package(pybind11 REQUIRED)
  add_library( revkit_python SHARED reversible/revkit.cpp )
  target_compile_definitions( revkit_python PUBLIC ALICE_PYTHON )
  set_target_properties( revkit_python PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED YES )
  set_target_properties( revkit_python PROPERTIES PREFIX "" )
  set_target_properties( revkit_python PROPERTIES SUFFIX "" )
  set_target_properties( revkit_python PROPERTIES OUTPUT_NAME "revkit.so" )
  target_include_directories( revkit_python PUBLIC ${PYTHON_INCLUDE_DIRS} )
  target_link_libraries( revkit_python cirkit_reversible cirkit_classical ${PYTHON_LIBRARIES} )
endif()
