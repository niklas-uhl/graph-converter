include(FindPackageHandleStandardArgs)

find_path(ZLib_INCLUDE_DIR zlib.h)
find_library(ZLib_LIBRARY z)

find_package_handle_standard_args(
    ZLib
    DEFAULT_MSG
    ZLib_INCLUDE_DIR
    ZLib_LIBRARY
)
if(ZLib_FOUND)
    mark_as_advanced(ZLib_LIBRARY ZLib_INCLUDE_DIR)
    if (NOT TARGET ZLib::ZLib)
        add_library(ZLib::ZLib UNKNOWN IMPORTED)
        set_target_properties(ZLib::ZLib PROPERTIES
        IMPORTED_LOCATION "${ZLib_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${ZLib_INCLUDE_DIR}"
        )
    endif()
endif()