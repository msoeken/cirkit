add_executable(cirkit cirkit.cpp)
target_link_libraries(cirkit PRIVATE alice mockturtle)

add_executable(revkit revkit.cpp)
target_link_libraries(revkit PRIVATE alice td)
