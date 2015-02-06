project("CirKit" CXX)
cmake_minimum_required(VERSION 2.8.9)

# ccache if available
find_program(ccache_cmd NAMES ccache)
if (ccache_cmd)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
  message(STATUS "Using ccache for building")
endif()

# compiler definitions
add_definitions(-std=c++11 -O3 -fno-omit-frame-pointer -g -DBOOST_RESULT_OF_USE_DECLTYPE -D__extern_always_inline=inline -DDATA_PATH="${CMAKE_CURRENT_SOURCE_DIR}")

# clang specific compiler definitions
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  add_definitions(-fcolor-diagnostics)
endif()

include(CTest)

# Some user defined macros (INFO: maybe they can be moved somewhere else)
set(ext_libraries "" CACHE STRING "external libraries" FORCE)
function(add_ext_library library)
  if ("${ext_libraries}" STREQUAL "")
    set(ext_libraries "${library}" CACHE STRING "external libraries" FORCE)
  else()
    set(ext_libraries "${ext_libraries};${library}" CACHE STRING "external libraries" FORCE)
  endif()
endfunction(add_ext_library)

macro(subdirlist result curdir)
  file(GLOB children ${curdir} ${curdir}/*)
  set(dirlist "")
  foreach(child ${children})
    if(IS_DIRECTORY ${child})
      get_filename_component(base_name ${child} NAME_WE)
      set(dirlist ${dirlist} ${base_name})
    endif()
  endforeach()
  set(${result} ${dirlist})
endmacro()

macro(srcdirlist result curdir)
  if("${cirkit_PACKAGES}" STREQUAL "")
    subdirlist(${result} ${curdir})
  else()
    set(${result} ${cirkit_PACKAGES})
  endif()
endmacro()

# Dependencies
set( MAIN_Boost_LIBRARIES unit_test_framework regex filesystem graph program_options system timer )
find_package(Boost REQUIRED COMPONENTS ${MAIN_Boost_LIBRARIES} )

include_directories(${Boost_INCLUDE_DIR} ext/include)
link_directories(${CMAKE_SOURCE_DIR}/ext/lib)
add_subdirectory(ext)

# Options
option(cirkit_ENABLE_PROGRAMS "build programs" on)
option(cirkit_ENABLE_TESTS "build tests" on)
option(cirkit_BUILD_SHARED "build shared libraries" off)
option(cirkit_BUILD_STATIC "build static libraries" on)
option(cirkit_LINK_PROGRAMS_TO_STATIC "links programs to static libraries" on)
set(cirkit_PACKAGES "" CACHE STRING "if non-empty, then only the packages in the semicolon-separated lists are build")

# Libraries (include)
include_directories(src)

# Addons
add_subdirectory(addons)
foreach (dir ${addon_directories})
  string(REGEX REPLACE "cirkit-" "" def ${dir})
  string(REGEX REPLACE "-" "_" def ${def})
  string(TOUPPER ${def} def)
  add_definitions(-D${def})
  include_directories(addons/${dir}/src)
endforeach()
foreach (dir ${addon_include_directories})
  include_directories(${dir})
endforeach()
foreach (dir ${addon_link_directories})
  link_directories(${dir})
endforeach()

# Libraries
add_subdirectory(src)

# Programs
if(cirkit_ENABLE_PROGRAMS)
  add_subdirectory(programs)
endif()

# Tests
if(cirkit_ENABLE_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()



