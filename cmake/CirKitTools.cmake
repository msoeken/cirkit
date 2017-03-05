include( CMakeParseArguments )

set(
  CirKitTools_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
  CACHE PATH "Where CirKitTools finds its own files"
  FORCE
)
mark_as_advanced( CirKitTools_DIRECTORY )

###
# add_cirkit_library(
#     NAME <name>         - the name of the library prefixed with "cirkit_"
#     SOURCES <file> ...  - list of source files for this library (optional)
#     AUTO_DIRS <dir> ... - list of directories to search for cpp files (optional)
# )
#
# Creates a circuit library. the targets will be called "<name>"
# is will  be static or shared depending on 'cirkit_BUILD_SHARED'
###
function( add_cirkit_library )
  cmake_parse_arguments(
    "arg"
    ""
    "NAME"
    "SOURCES;AUTO_DIRS;USE;INCLUDE;DEFINE;COMMANDS;OPTIONS"
    ${ARGN}
  )

  if( DEFINED arg_UNPARSED_ARGUMENTS )
    message( FATAL_ERROR "invalid arguments passed to cirkit_add_library: ${arg_UNPARSED_ARGUMENTS}" )
  endif( )

  if( NOT DEFINED arg_NAME )
    message( FATAL_ERROR "cirkit_add_library requires a NAME: cirkit_add_library( NAME abc ...)" )
  endif( )

  if( (NOT arg_AUTO_DIRS) AND (NOT DEFINED arg_SOURCES) )
    message( FATAL_ERROR "library ${arg_NAME} specifies neither SOURCES nor AUTO_DIRS." )
  endif( )

  if( DEFINED arg_AUTO_DIRS )
    foreach( dir ${arg_AUTO_DIRS} )
      file( GLOB_RECURSE files ${dir}/*.cpp )
      list( APPEND arg_SOURCES ${files} )
    endforeach( )
  endif( )

  if( cirkit_BUILD_SHARED )
    set( type SHARED )
  else( )
    set( type STATIC )
  endif( )

  add_library( ${arg_NAME} ${type}
    ${CirKitTools_DIRECTORY}/nothing.cpp
    ${arg_SOURCES}
  )

  set_property( TARGET ${arg_NAME} PROPERTY POSITION_INDEPENDENT_CODE on )

  if( DEFINED arg_USE )
    target_link_libraries( ${arg_NAME} PUBLIC ${arg_USE} )
  endif( )

  if( DEFINED arg_INCLUDE )
    target_include_directories( ${arg_NAME} ${arg_INCLUDE} )
  endif( )

  if( DEFINED arg_DEFINE )
    target_compile_definitions( ${arg_NAME} ${arg_DEFINE} )
  endif( )

  if( DEFINED arg_OPTIONS )
    target_compile_options( ${arg_NAME} ${arg_OPTIONS} )
  endif( )

  if( DEFINED arg_COMMANDS )
    if(cirkit_ENABLE_PROGRAMS)
      set(cirkit_addon_command_libraries ${cirkit_addon_command_libraries} ${arg_NAME} CACHE INTERNAL "" FORCE )
      set(cirkit_addon_command_includes "${cirkit_addon_command_includes}#include <${arg_COMMANDS}>;" CACHE INTERNAL "" FORCE )

      string( TOUPPER ${arg_NAME} arg_NAME_UC )
      set(cirkit_addon_command_defines "${cirkit_addon_command_defines}${arg_NAME_UC}_COMMANDS;" CACHE INTERNAL "" FORCE )
    endif()
  endif( )

endfunction( )


function( add_cirkit_program )
  cmake_parse_arguments(
    "arg"
    ""
    "NAME"
    "SOURCES;AUTO_DIRS;USE;INCLUDE"
    ${ARGN}
  )

  if( DEFINED arg_UNPARSED_ARGUMENTS )
    message( FATAL_ERROR "invalid arguments passed to cirkit_add_library: ${arg_UNPARSED_ARGUMENTS}" )
  endif( )

  if( NOT DEFINED arg_NAME )
    message( FATAL_ERROR "cirkit_add_library requires a NAME: cirkit_add_library( NAME abc ...)" )
  endif( )

  if( (NOT arg_AUTO_DIRS) AND (NOT DEFINED arg_SOURCES) )
    message( FATAL_ERROR "library ${arg_NAME} specifies neither SOURCES nor AUTO_DIRS." )
  endif( )

  if( DEFINED arg_AUTO_DIRS )
    foreach( dir ${arg_AUTO_DIRS} )
      file( GLOB_RECURSE files ${dir}/*.cpp )
      list( APPEND arg_SOURCES ${files} )
    endforeach( )
  endif( )


  add_executable( ${arg_NAME} ${arg_SOURCES} )

  if( DEFINED arg_USE )
    target_link_libraries( ${arg_NAME} ${arg_USE} )
  endif( )

  if( DEFINED arg_INCLUDE )
    target_include_directories( ${arg_NAME} ${arg_INCLUDE} )
  endif( )

endfunction( )


function( add_cirkit_test_program )
  cmake_parse_arguments(
    "arg"
    ""
    "NAME"
    ""
    ${ARGN}
  )

  set( name "test_${arg_NAME}" )
  add_cirkit_program( NAME ${name} ${arg_UNPARSED_ARGUMENTS} )
  add_test( NAME ${name} COMMAND $<TARGET_FILE:${name}> )
endfunction( )


function( find_cirkit_addon_dirs OUTPUT_VARIABLE name )
  set( _addon_dirs "")
  foreach(dir ${addon_directories})
    list( APPEND _addon_dirs ${CMAKE_SOURCE_DIR}/addons/${dir}/src/${name} )
  endforeach( )

  set( ${OUTPUT_VARIABLE} ${_addon_dirs} PARENT_SCOPE )
endfunction( )
