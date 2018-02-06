include_directories(lib/xtensor-0.10.9/include)
include_directories(lib/xtensor-blas-0.1.0/include)
include_directories(lib/xtensor-blas-0.1.0/include/xtensor-blas/flens)

add_subdirectory(src)
add_subdirectory(programs)

if(BUILD_TESTING)
add_subdirectory(test)
endif()

