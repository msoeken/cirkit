# A function to add a unit test based on a name
# The name is the *.cpp basename without an extension
function(add_unit_test name)
  add_executable(test_${name} ${name}.cpp)
  add_test(${name} test_${name})
  file(GLOB ext_libraries "../ext/lib/*.so") # TODO this does not work on first run
  #target_link_libraries(test_${name} ${Boost_LIBRARIES} ${ext_libraries})
  target_link_libraries(test_${name} ${Boost_LIBRARIES} libcudd_cudd.so libcudd_dddmp.so libcudd_epd.so libcudd_mtr.so libcudd_nanotrav.so libcudd_obj.so libcudd_st.so libcudd_util.so libpuma.so)
  subdirlist(directories "../src/")
  foreach(dir ${directories})
    target_link_libraries(test_${name} cirkit_${dir})
  endforeach()
endfunction(add_unit_test)

# Find all test source files and build unit tests from them
file(GLOB test_files *.cpp)
foreach(test_file ${test_files})
  get_filename_component(test_name ${test_file} NAME_WE)
  add_unit_test(${test_name})
endforeach()
