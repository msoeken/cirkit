# Configuration
add_definitions(-Wall)

add_cirkit_library(
  NAME cirkit_core
  AUTO_DIRS core
  AUTI DIRS lscli
  USE
    ${Boost_REGEX_LIBRARIES}
    ${Boost_TIMER_LIBRARIES}
    ${Boost_SYSTEM_LIBRARIES}
    ${Boost_FILESYSTEM_LIBRARIES}
    ${Boost_PROGRAM_OPTIONS_LIBRARIES}
    ${CUDD_LIBRARIES}
    ${READLINE_LIBRARY}
)

add_cirkit_library(
  NAME cirkit_classical
  AUTO_DIRS classical
  USE
    cirkit_core
    ${Boost_FILESYSTEM_LIBRARIES}
)

add_cirkit_library(
  NAME cirkit_abc
  AUTO_DIRS abc
  USE
    cirkit_core
    cirkit_classical
    ${abc_LIBRARIES}
  INCLUDE
    PUBLIC ${CMAKE_SOURCE_DIR}/ext/include/abc
)
