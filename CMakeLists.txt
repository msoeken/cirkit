# CirKit: A circuit toolkit
# Copyright (C) 2009-2015  University of Bremen
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Authors of Build system: Finn Haedicke, Stefan Frehse, Heinz Riener, Mathias Soeken

project("CirKit" C CXX)
cmake_minimum_required(VERSION 3.0)

include(cmake/CCache.cmake)
include(cmake/CompilerFlags.cmake)
include(cmake/ListDir.cmake)
include(cmake/AddExtLibrary.cmake)
include(cmake/CirKitTools.cmake)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/modules/")

include(CTest)


# Dependencies
set( MAIN_Boost_LIBRARIES
  unit_test_framework
  filesystem
  graph
  program_options
  system
  timer
)
find_package(Boost REQUIRED COMPONENTS ${MAIN_Boost_LIBRARIES} )
find_package(GMP REQUIRED)

include_directories(SYSTEM /usr/include ${Boost_INCLUDE_DIR} ${GMP_INCLUDE_DIR} ext/include)
add_subdirectory(ext)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/programs )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/programs )

# Options
option(cirkit_ENABLE_PROGRAMS "build programs" on)
option(cirkit_BUILD_SHARED "build shared libraries" on)
option(cirkit_ENABLE_PYTHON_API "build Python APIs (experimental)" off)
set(cirkit_PACKAGES "" CACHE STRING "if non-empty, then only the packages in the semicolon-separated lists are build")
set(cirkit_addon_command_libraries "" CACHE INTERNAL "" FORCE )
set(cirkit_addon_command_includes "" CACHE INTERNAL "" FORCE )
set(cirkit_addon_command_defines "" CACHE INTERNAL "" FORCE )

# Readline
include(CheckCXXSourceRuns)

set(CMAKE_REQUIRED_LIBRARIES readline)
check_cxx_source_runs("
#include <stdio.h>
#include <readline/readline.h>
void foo() { readline(\"foo\"); }
int main() { return 0; }
" READLINE_WORKS)
unset(CMAKE_REQUIRED_LIBRARIES)

if( READLINE_WORKS )
  set(USE_READLINE 1)
  set(READLINE_LIBRARY readline)
  add_definitions(-DUSE_READLINE=1)
else( )
  set(USE_READLINE 0)
  set(READLINE_LIBRARY)
endif( )

# ABC
add_definitions(-DABC_NAMESPACE=abc)

# Libraries (include)
include_directories(src)


# 3rd party libraries
add_subdirectory( lib )

# Main code
add_subdirectory(src)

# Addons
add_subdirectory(addons)

# Programs
if(cirkit_ENABLE_PROGRAMS)
  add_subdirectory(programs)
endif()

# Tests
if(BUILD_TESTING)
  add_subdirectory(test)
endif()
