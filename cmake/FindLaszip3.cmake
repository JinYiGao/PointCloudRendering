# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#.rst:
# FindLaszip3
# --------
#
# Find the Laszip3 library (libLaszip3).
#
# Imported targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` targets:
#
# ``Laszip3::Laszip3``
#   The Laszip3 library, if found.
#
# Result variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``Laszip3_FOUND``
#   true if the Laszip3 headers and libraries were found
# ``Laszip3_INCLUDE_DIR``
#   the directory containing the Laszip3 headers
# ``Laszip3_INCLUDE_DIRS``
#   the directory containing the Laszip3 headers
# ``Laszip3_LIBRARIES``
#   Laszip3 libraries to be linked
#
# Cache variables
# ^^^^^^^^^^^^^^^
#
# The following cache variables may also be set:
#
# ``Laszip3_INCLUDE_DIR``
#   the directory containing the Laszip3 headers
# ``Laszip3_LIBRARY``
#   the path to the Laszip3 library

find_path(Laszip3_INCLUDE_DIR "laszip/laszip_api.h")

set(Laszip3_NAMES ${Laszip3_NAMES} laszip3)

foreach(name ${Laszip3_NAMES})
  list(APPEND Laszip3_NAMES_DEBUG  "${name}" "${name}d")
endforeach()

if(NOT Laszip3_LIBRARY)
  find_library(Laszip3_LIBRARY_RELEASE NAMES ${Laszip3_NAMES})
  find_library(Laszip3_LIBRARY_DEBUG NAMES ${Laszip3_NAMES_DEBUG})
  include(SelectLibraryConfigurations)
  select_library_configurations(Laszip3)
  mark_as_advanced(Laszip3_LIBRARY_RELEASE Laszip3_LIBRARY_DEBUG)
endif()
unset(Laszip3_NAMES)
unset(Laszip3_NAMES_DEBUG)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Laszip3
                                  REQUIRED_VARS Laszip3_LIBRARY Laszip3_INCLUDE_DIR
                                  VERSION_VAR Laszip3_VERSION_STRING)

if(Laszip3_FOUND)
  set(Laszip3_LIBRARIES ${Laszip3_LIBRARY})
  set(Laszip3_INCLUDE_DIRS "${Laszip3_INCLUDE_DIR}")

  if(NOT TARGET Laszip3::Laszip3)
    add_library(Laszip3::Laszip3 UNKNOWN IMPORTED)
    if(Laszip3_INCLUDE_DIRS)
      set_target_properties(Laszip3::Laszip3 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${Laszip3_INCLUDE_DIRS}")
    endif()
    if(EXISTS "${Laszip3_LIBRARY}")
      set_target_properties(Laszip3::Laszip3 PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "C"
        IMPORTED_LOCATION "${Laszip3_LIBRARY}")
    endif()
    if(EXISTS "${Laszip3_LIBRARY_RELEASE}")
      set_property(TARGET Laszip3::Laszip3 APPEND PROPERTY
        IMPORTED_CONFIGURATIONS RELEASE)
      set_target_properties(Laszip3::Laszip3 PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
        IMPORTED_LOCATION_RELEASE "${Laszip3_LIBRARY_RELEASE}")
    endif()
    if(EXISTS "${Laszip3_LIBRARY_DEBUG}")
      set_property(TARGET Laszip3::Laszip3 APPEND PROPERTY
        IMPORTED_CONFIGURATIONS DEBUG)
      set_target_properties(Laszip3::Laszip3 PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
        IMPORTED_LOCATION_DEBUG "${Laszip3_LIBRARY_DEBUG}")
    endif()
  endif()
endif()

mark_as_advanced(Laszip3_INCLUDE_DIR Laszip3_LIBRARY)
