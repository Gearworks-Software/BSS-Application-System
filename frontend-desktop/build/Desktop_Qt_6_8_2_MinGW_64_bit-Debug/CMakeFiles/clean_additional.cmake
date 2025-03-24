# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\frontend-desktop_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\frontend-desktop_autogen.dir\\ParseCache.txt"
  "frontend-desktop_autogen"
  )
endif()
