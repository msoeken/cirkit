add_executable(cirkit cirkit.cpp)
target_link_libraries(cirkit PRIVATE alice mockturtle)

add_alice_python_module(cirkit cirkit.cpp)
target_link_libraries(cirkit_python PRIVATE alice mockturtle)
