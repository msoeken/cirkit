# Configuration
add_definitions(-Wall)
add_ext_library(gmp gmpxx pthread)


add_cirkit_library( NAME core AUTO_DIRS core )
add_cirkit_library( NAME classical AUTO_DIRS classical )
add_cirkit_library( NAME formal AUTO_DIRS formal )
