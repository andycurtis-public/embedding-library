#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "embedding-library::static" for configuration ""
set_property(TARGET embedding-library::static APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(embedding-library::static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "C"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libembedding-library_static.a"
  )

list(APPEND _cmake_import_check_targets embedding-library::static )
list(APPEND _cmake_import_check_files_for_embedding-library::static "${_IMPORT_PREFIX}/lib/libembedding-library_static.a" )

# Import target "embedding-library::debug" for configuration ""
set_property(TARGET embedding-library::debug APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(embedding-library::debug PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "C"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libembedding-library_debug.a"
  )

list(APPEND _cmake_import_check_targets embedding-library::debug )
list(APPEND _cmake_import_check_files_for_embedding-library::debug "${_IMPORT_PREFIX}/lib/libembedding-library_debug.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
