
# The default build should be optimized with proper stack traces and debug symbols
# Therefore we override RELWITHDEBINFO and set this compile mode when noone is selected.

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -fno-omit-frame-pointer -g -DNODEBUG"
  CACHE STRING "Flags used by the compiler during Release with Debug Info builds." FORCE)

set(CMAKE_CXX_FLAGS_DISABLE_WARNINGS "-Wno-unknown-pragmas -Wno-unused-result -Wno-char-subscripts")
set(CMAKE_CXX_FLAGS_ENABLE_WARNINGS "-Wsign-compare")
set(CMAKE_CXX_FLAGS_WARNINGS_AS_ERRORS "-Werror=return-type")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 ${CMAKE_CXX_FLAGS_DISABLE_WARNINGS} ${CMAKE_CXX_FLAGS_ENABLE_WARNINGS} ${CMAKE_CXX_FLAGS_WARNINGS_AS_ERRORS}")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "RELWITHDEBINFO" CACHE STRING "" FORCE)
endif()

# compiler definitions
add_definitions(-DBOOST_RESULT_OF_USE_DECLTYPE -D__extern_always_inline=inline -DDATA_PATH="${CMAKE_SOURCE_DIR}")
add_definitions(-D__STDC_LIMIT_MACROS -D__STDC_FORMAT_MACROS)

# clang specific compiler definitions
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  add_definitions(-fcolor-diagnostics)
endif()
