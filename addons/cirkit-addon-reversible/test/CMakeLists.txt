set(reversible_tests
  change_polarity
  circuit
  circuit_io
  copy_circuit
  esop_synthesis
  modules
  permutation
  rcbdd_scalability
  redundancy_functions
  restricted_growth_sequence
  synthesis
  truth_table
  truth_table_based_synthesis)

foreach( test ${reversible_tests} )
  add_cirkit_test_program(
    NAME ${test}
    SOURCES
      reversible/${test}.cpp
    USE
      cirkit_reversible
      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES}
  )
endforeach()
