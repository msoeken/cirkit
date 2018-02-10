set(ABC_USE_NAMESPACE "abc")
add_subdirectory(abc)
target_compile_definitions(libabc-pic PUBLIC "LIN64")

set(CUDD_BUILD_TESTS OFF CACHE BOOL "Build the tests" FORCE)
add_subdirectory(cudd)
target_include_directories(cudd PUBLIC "cudd/cudd")
target_include_directories(cudd PUBLIC ${CMAKE_BINARY_DIR}/lib/cudd/)
target_include_directories(epd PUBLIC "cudd/epd")
target_include_directories(mtr PUBLIC "cudd/mtr")
target_include_directories(st PUBLIC "cudd/st")
target_include_directories(cudd_util PUBLIC "cudd/util")
target_include_directories(obj PUBLIC "cudd/cplusplus")
foreach(target cudd epd mtr st cudd_util obj)
set_property(TARGET ${target} PROPERTY POSITION_INDEPENDENT_CODE ON)
endforeach()

add_subdirectory(alice)
add_subdirectory(kitty)
add_subdirectory(fmt EXCLUDE_FROM_ALL)

# add_cirkit_library(
#   NAME extra
#   SOURCES
#     extra/extraAddMisc.c
#     extra/extraAddSpectra.c
#     extra/extraBddAuto.c
#     extra/extraBddBoundSet.c
#     extra/extraBddDistance.c
#     extra/extraBddKmap.c
#     extra/extraBddMisc.c
#     extra/extraBddPermute.c
#     extra/extraBddSupp.c
#     extra/extraBddSymm.c
#     extra/extraBddUnate.c
#     extra/extraBddWidth.c
#     extra/extraDdMinterm.c
#     extra/extraDdMisc.c
#     extra/extraDdNodePath.c
#     extra/extraDdPrint.c
#     extra/extraDdShift.c
#     extra/extraDdSigma.c
#     extra/extraDdTimed.c
#     extra/extraDdTransfer.c
#     extra/extraUtilFile.c
#     extra/extraUtilMemory.c
#     extra/extraUtilMisc.c
#     extra/extraUtilProgress.c
#     extra/extraZddCover.c
#     extra/extraZddExor.c
#     extra/extraZddFactor.c
#     extra/extraZddGraph.c
#     extra/extraZddIsop.c
#     extra/extraZddLitCount.c
#     extra/extraZddMaxMin.c
#     extra/extraZddMisc.c
#     extra/extraZddPermute.c
#     extra/extraZddSubSup.c
#   USE
#     cudd
#   INCLUDE
#     SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/extra
#   OPTIONS
#     PRIVATE -w
# )

set(CMAKE_MACOSX_RPATH 1)
add_subdirectory(minisat EXCLUDE_FROM_ALL)
set_property(TARGET minisat-lib-static PROPERTY POSITION_INDEPENDENT_CODE ON)

add_cirkit_library(
  NAME puma
  SOURCES
    puma/puma.cpp
    puma/tc_time.cpp
  INCLUDE
    SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/puma
  OPTIONS
    PRIVATE -Wno-sign-compare
)

