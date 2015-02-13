# Some user defined macros

set(ext_libraries "" CACHE STRING "external libraries" FORCE)
mark_as_advanced(ext_libraries)
function(add_ext_library)
  if ("${ext_libraries}" STREQUAL "")
    set(ext_libraries "${ARGN}" CACHE STRING "external libraries" FORCE)
  else()
    set(ext_libraries "${ext_libraries};${ARGN}" CACHE STRING "external libraries" FORCE)
  endif()
endfunction(add_ext_library)
