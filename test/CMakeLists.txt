# Find all test source files and build unit tests from them
srcdirlist(directories ".")
foreach(dir ${directories})
  file(GLOB test_files "${dir}/*.cpp")
  foreach(test_file ${test_files})
    get_filename_component(test_name ${test_file} NAME_WE)

    add_cirkit_test_program(
       NAME ${test_name}
       SOURCES ${test_file}
       USE
         cirkit_${dir}
         ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES}
    )
  endforeach()
endforeach()
