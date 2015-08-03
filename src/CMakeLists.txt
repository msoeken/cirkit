# Configuration
add_definitions(-Wall)


add_cirkit_library(
  NAME cirkit_core
  AUTO_DIRS core
  USE
    ${Boost_REGEX_LIBRARIES}
    ${Boost_TIMER_LIBRARIES}
    ${Boost_SYSTEM_LIBRARIES}
    ${Boost_FILESYSTEM_LIBRARIES}
    ${Boost_PROGRAM_OPTIONS_LIBRARIES}
    ${CUDD_LIBRARIES}
)
add_cirkit_library(
  NAME cirkit_classical
  AUTO_DIRS classical
  USE
    cirkit_core
    ${Boost_FILESYSTEM_LIBRARIES}
    ${IGRAPH_LIBRARIES}
)
