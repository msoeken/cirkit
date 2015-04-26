project("CirKit" CXX)
cmake_minimum_required(VERSION 2.8.9)

include(cmake/CCache.cmake)
include(cmake/CompilerFlags.cmake)
include(cmake/ListDir.cmake)
include(cmake/AddExtLibrary.cmake)
include(cmake/Addons.cmake)
include(cmake/CirKitTools.cmake)

include(CTest)


# Dependencies
set( MAIN_Boost_LIBRARIES
  unit_test_framework
  regex filesystem
  graph
  program_options
  system
  timer
  thread
)
find_package(Boost REQUIRED COMPONENTS ${MAIN_Boost_LIBRARIES} )

include_directories(${Boost_INCLUDE_DIR} ext/include)
add_subdirectory(ext)

# Options
option(cirkit_ENABLE_PROGRAMS "build programs" on)
option(cirkit_BUILD_SHARED "build shared libraries" off)
option(cirkit_BUILD_STATIC "build static libraries" on)
option(cirkit_LINK_PROGRAMS_TO_STATIC "links programs to static libraries" on)
set(cirkit_PACKAGES "" CACHE STRING "if non-empty, then only the packages in the semicolon-separated lists are build")

# Libraries (include)
include_directories(src)

# Addons
add_subdirectory(addons)
add_addon_flags()

# Libraries
add_subdirectory(src)

# Programs
if(cirkit_ENABLE_PROGRAMS)
  add_subdirectory(programs)
endif()

# Tests
if(BUILD_TESTING)
  add_subdirectory(test)
endif()



