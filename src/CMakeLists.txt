# Configuration
add_definitions(-Wall)
add_ext_library(gmp gmpxx pthread)

srcdirlist(directories ".")
foreach(dir ${directories})

  find_cirkit_addon_dirs( addon_dirs ${dir} )
  add_cirkit_library(
    NAME ${dir}
    AUTO_DIRS ${dir} ${addon_dirs}
  )

endforeach()
