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
# Creates a circuit library. the targets will be called "cirkit_<name>" for the shared library
# and "cirkit_<name>_static" for the static library (if the respective mode is enabled)
#
###
function( add_cirkit_library )
  cmake_parse_arguments(
    "arg"
    ""
    "NAME"
    "SOURCES;AUTO_DIRS;USE"
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

  set( libs_static "" )
  set( libs_shared "" )
  foreach( item ${arg_USE} )

    if( TARGET ${item}_static)
      list( APPEND libs_static ${item}_static )
    else( )
      list( APPEND libs_static ${item} )
    endif( )

    list( APPEND libs_shared ${item} )
  endforeach( )

  set( objlib cirkit_${arg_NAME}_objlib )
  set( shared cirkit_${arg_NAME} )
  set( static cirkit_${arg_NAME}_static )

  add_library( ${objlib} OBJECT
    ${CirKitTools_DIRECTORY}/nothing.cpp
    ${arg_SOURCES}
  )

  set_property( TARGET ${objlib} PROPERTY POSITION_INDEPENDENT_CODE on )

  set( link_libs )

  if( cirkit_BUILD_SHARED )
    add_library( ${shared} SHARED $<TARGET_OBJECTS:${objlib}> )
    target_link_libraries( ${shared} ${link_libs} ${libs_shared} )
  endif( )

  if(cirkit_BUILD_STATIC)
    add_library( ${static} STATIC $<TARGET_OBJECTS:${objlib}> )
    target_link_libraries( ${static} ${link_libs} ${libs_static} )
  endif( )

endfunction( )


function( add_cirkit_program )
  cmake_parse_arguments(
    "arg"
    ""
    "NAME"
    "SOURCES;AUTO_DIRS;USE"
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

  set( libs "" )
  foreach( item ${arg_USE} )
    if( ${cirkit_LINK_PROGRAMS_TO_STATIC} AND TARGET ${item}_static )
      list( APPEND libs ${item}_static )
    else( )
      list( APPEND libs ${item} )
    endif( )
  endforeach( )

  add_executable( ${arg_NAME} ${arg_SOURCES} )
  target_link_libraries( ${arg_NAME} ${libs} )

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
  add_test( ${name} ${name})
endfunction( )


function( find_cirkit_addon_dirs OUTPUT_VARIABLE name )
  set( _addon_dirs "")
  foreach(dir ${addon_directories})
    list( APPEND _addon_dirs ${CMAKE_SOURCE_DIR}/addons/${dir}/src/${name} )
  endforeach( )

  set( ${OUTPUT_VARIABLE} ${_addon_dirs} PARENT_SCOPE )
endfunction( )
