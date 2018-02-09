# Configuration
add_definitions(-Wall)

add_cirkit_library(
  NAME cirkit_core
  AUTO_DIRS core
  USE
    cudd
    obj
    fmt::fmt-header-only
)

add_cirkit_library(
  NAME cirkit_classical
  AUTO_DIRS classical
  USE
    ${Boost_FILESYSTEM_LIBRARIES}  
    ${Boost_SYSTEM_LIBRARIES}
    cirkit_core
    libabc-pic
    kitty
    minisat-lib-static
)

add_cirkit_library(
  NAME cirkit_cli
  AUTO_DIRS cli
  USE
    alice
    cirkit_core
    cirkit_classical
    ${READLINE_LIBRARY}
  DEFINE
    PUBLIC ALICE_ENV_HOME=CIRKIT_HOME
)

