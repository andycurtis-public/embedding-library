
include(CMakeFindDependencyMacro)
set(_deps "")
foreach(_d IN LISTS _deps)
  if(NOT _d STREQUAL "")
    find_dependency(${_d} REQUIRED)
  endif()
endforeach()
include("${CMAKE_CURRENT_LIST_DIR}/embedding-libraryTargets.cmake")
