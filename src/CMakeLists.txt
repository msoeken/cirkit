# Configuration
add_definitions(-Wall)

add_cirkit_library(
  NAME cirkit_core
  AUTO_DIRS core
  USE
    ${Boost_TIMER_LIBRARIES}
    ${CUDD_LIBRARIES}
    ${Boost_SYSTEM_LIBRARIES}
    ${Boost_FILESYSTEM_LIBRARIES}
    ${Boost_PROGRAM_OPTIONS_LIBRARIES}
    ${READLINE_LIBRARY}
)

add_cirkit_library(
  NAME cirkit_classical
  AUTO_DIRS classical
  USE
    cirkit_core
    ${abc_LIBRARIES}
    ${MINISAT_LIBRARIES}
  INCLUDE
    SYSTEM PUBLIC ${CMAKE_BINARY_DIR}/ext/abc-prefix/src/abc/src
)
