# ccache if available
find_program(ccache_cmd NAMES ccache)
if (ccache_cmd)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
  message(STATUS "Using ccache for building")
endif()

