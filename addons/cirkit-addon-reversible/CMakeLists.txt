include_directories(lib/xtl-0.4.0/include/)
include_directories(lib/xtensor-0.13.2/include)
include_directories(lib/xtensor-blas-0.6.0/include)
include_directories(lib/xtensor-blas-0.6.0/include/xtensor-blas/flens)

add_subdirectory(src)
add_subdirectory(programs)

if(BUILD_TESTING)
add_subdirectory(test)
endif()

