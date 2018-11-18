add_executable(cirkit cirkit.cpp)
target_link_libraries(cirkit PRIVATE alice mockturtle)

add_executable(revkit revkit.cpp)
target_link_libraries(revkit PRIVATE alice tweedledum mockturtle caterpillar)

if(WIN32)
target_compile_options(cirkit PRIVATE /bigobj)
target_compile_options(revkit PRIVATE /bigobj)
elseif(UNIX)
target_compile_options(cirkit PRIVATE -Wno-pragmas)
target_compile_options(revkit PRIVATE -Wno-pragmas)
endif()
