set(local_addon_directories "")

subdirlist(directories ".")
foreach(dir ${directories})
  if (${dir} MATCHES "^revkit-addon")
    option(enable_${dir} "Enables ${dir}" ON)
    if(enable_${dir})
      add_subdirectory(${dir})
      set(local_addon_directories "${local_addon_directories};${dir}")
    endif()
  endif()
endforeach()

set(addon_directories ${local_addon_directories} PARENT_SCOPE)
