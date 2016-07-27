add_cirkit_library(
  NAME cirkit_reversible
  AUTO_DIRS reversible
  USE
    cirkit_core
    cirkit_classical
    cirkit_formal_z3
    puma
    ${CUDD_LIBRARIES}
    ${GMPXX_LIBRARIES}
    ${GMP_LIBRARIES}
  INCLUDE
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)
