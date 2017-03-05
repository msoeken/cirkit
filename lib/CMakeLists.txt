add_cirkit_library(
  NAME extra
  SOURCES
    extra/extraAddMisc.c
    extra/extraAddSpectra.c
    extra/extraBddAuto.c
    extra/extraBddBoundSet.c
    extra/extraBddDistance.c
    extra/extraBddKmap.c
    extra/extraBddMisc.c
    extra/extraBddPermute.c
    extra/extraBddSupp.c
    extra/extraBddSymm.c
    extra/extraBddUnate.c
    extra/extraBddWidth.c
    extra/extraDdMinterm.c
    extra/extraDdMisc.c
    extra/extraDdNodePath.c
    extra/extraDdPrint.c
    extra/extraDdShift.c
    extra/extraDdSigma.c
    extra/extraDdTimed.c
    extra/extraDdTransfer.c
    extra/extraUtilFile.c
    extra/extraUtilMemory.c
    extra/extraUtilMisc.c
    extra/extraUtilProgress.c
    extra/extraZddCover.c
    extra/extraZddExor.c
    extra/extraZddFactor.c
    extra/extraZddGraph.c
    extra/extraZddIsop.c
    extra/extraZddLitCount.c
    extra/extraZddMaxMin.c
    extra/extraZddMisc.c
    extra/extraZddPermute.c
    extra/extraZddSubSup.c
  USE
    ${CUDD_LIBRARIES}
  INCLUDE
    SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/extra
  OPTIONS
    PRIVATE -Wno-comment -Wno-parentheses -Wno-incompatible-pointer-types -Wno-implicit-int -Wno-int-to-pointer-cast
)
add_cirkit_library(
  NAME linenoise
  SOURCES
    linenoise/linenoise.c
  INCLUDE
    SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/linenoise
)

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
