add_subdirectory(mockturtle)

add_library(alice INTERFACE) # v0.2+
target_include_directories(alice INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/alice)
if(UNIX)
target_compile_definitions(alice INTERFACE "-DREADLINE_USE_READLINE=1")
target_link_libraries(alice INTERFACE readline)
endif()
target_link_libraries(alice INTERFACE any cli11 fmt json)

# library for Python bindings
add_library(alice_python INTERFACE)
target_link_libraries(alice_python INTERFACE alice)
target_compile_definitions(alice_python INTERFACE ALICE_PYTHON)
target_link_libraries(alice_python INTERFACE pybind11::module)

add_library(any INTERFACE)
target_include_directories(any INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/any)

add_library(cli11 INTERFACE) # v1.3
target_include_directories(cli11 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/cli11)
