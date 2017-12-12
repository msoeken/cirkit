# Configuration
add_definitions(-Wall)

add_cirkit_library(
  NAME cirkit_core
  AUTO_DIRS core
  USE
    ${Boost_TIMER_LIBRARIES}
    cudd
    obj
    fmt::fmt-header-only
    ${Boost_PROGRAM_OPTIONS_LIBRARIES}
    ${Boost_SYSTEM_LIBRARIES}
)

add_cirkit_library(
  NAME cirkit_classical
  AUTO_DIRS classical
  USE
    ${Boost_FILESYSTEM_LIBRARIES}  
    cirkit_core
    libabc-pic
    kitty
    minisat-lib-static
)

add_cirkit_library(
  NAME cirkit_cli
  AUTO_DIRS cli
  USE
    cirkit_core
    cirkit_classical
    ${READLINE_LIBRARY}
)

