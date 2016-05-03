find_path(Clang_Libs_INCLUDE_DIR clang/Config/config.h)
find_library(Clang_AST_LIBRARY clangAST)
find_library(Clang_Format_LIBRARY clangFormat)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ClangLibs DEFAULT_MSG
  Clang_Libs_INCLUDE_DIR
  Clang_AST_LIBRARY
  Clang_Format_LIBRARY
)
mark_as_advanced(
  Clang_Libs_INCLUDE_DIR
  Clang_AST_LIBRARY
  Clang_Format_LIBRARY
)

if (CLANGLIBS_FOUND)
  add_library(clang::AST UNKNOWN IMPORTED)
  set_target_properties(clang::AST PROPERTIES
    IMPORTED_LOCATION "${Clang_AST_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${Clang_Libs_INCLUDE_DIR}")

  add_library(clang::Format UNKNOWN IMPORTED)
  set_target_properties(clang::Format PROPERTIES
    IMPORTED_LOCATION "${Clang_Format_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${Clang_Libs_INCLUDE_DIR}")
endif()
