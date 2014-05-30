# A function to add a program based on a name
# The name is the *.cpp basename without an extension
function(add_program filename)
  get_filename_component(name ${filename} NAME_WE)
  add_executable(${name} ${filename})
  target_link_libraries(${name} ${Boost_LIBRARIES})
  foreach(library ${ext_libraries})
    target_link_libraries(${name} ${library})
  endforeach()
  subdirlist(directories "../src/")
  foreach(dir ${directories})
    if (revkit_LINK_PROGRAMS_TO_STATIC)
      target_link_libraries(${name} revkit_${dir}_static)
    else()
      target_link_libraries(${name} revkit_${dir})
    endif()
  endforeach()
endfunction(add_program)

# Find all test source files and build unit tests from them
file(GLOB programs *.cpp ${CMAKE_SOURCE_DIR}/addons/revkit-addon-*/programs/*.cpp)
foreach(program ${programs})
  add_program(${program})
endforeach()
