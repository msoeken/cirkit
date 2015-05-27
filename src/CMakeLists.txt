add_cirkit_library(
  NAME cirkit_reversible
  AUTO_DIRS reversible
  USE
    cirkit_core
    cirkit_classical
    ${PUMA_LIBRARIES}
    ${CUDD_LIBRARIES}
    gmpxx
  INCLUDE
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)
