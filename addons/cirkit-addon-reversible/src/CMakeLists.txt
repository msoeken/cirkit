add_cirkit_library(
  NAME cirkit_reversible
  AUTO_DIRS reversible
  USE
    cirkit_core
    cirkit_classical
    puma
    ${CUDD_LIBRARIES}
    ${GMPXX_LIBRARIES}
    ${GMP_LIBRARIES}
  INCLUDE
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

add_cirkit_library(
  NAME cirkit_reversible_cli
  AUTO_DIRS cli
  USE
    cirkit_reversible
    cirkit_cli
)

if(enable_cirkit-addon-formal)
target_link_libraries(cirkit_reversible PUBLIC cirkit_formal_z3)
target_link_libraries(cirkit_reversible_cli PUBLIC cirkit_formal_z3_cli)
endif()

