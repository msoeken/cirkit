# Configuration
add_definitions(-Wall)
add_ext_library(gmp gmpxx pthread)

srcdirlist(directories ".")
foreach(dir ${directories})
  set( addon_dirs "")
  foreach(adir ${addon_directories})
    list( APPEND addon_dirs ${CMAKE_SOURCE_DIR}/addons/${adir}/src/${dir} )
  endforeach( )

  add_cirkit_library(
    NAME ${dir}
    AUTO_DIRS ${dir} ${addon_dirs}
  )

endforeach()
