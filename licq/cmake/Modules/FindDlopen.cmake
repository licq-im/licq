# - Find library containing dlopen()
# The following variables are set if dlopen is found. If dlopen is not
# found, DLOPEN_FOUND is set to false.
#  DLOPEN_FOUND     - System has dlopen.
#  DLOPEN_LIBRARIES - Link these to use dlopen.

# Copyright (c) 2007 Erik Johansson <erijo@licq.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(CheckLibraryExists)

# Assume dlopen is not found.
set(DLOPEN_FOUND FALSE)

foreach (library c c_r dl)
  if (NOT DLOPEN_FOUND)
    check_library_exists(${library} dlopen "" DLOPEN_IN_${library})

    if (DLOPEN_IN_${library})
      set(DLOPEN_LIBRARIES ${library} CACHE STRING "Library containing dlopen")
      set(DLOPEN_FOUND TRUE)
    endif (DLOPEN_IN_${library})

  endif (NOT DLOPEN_FOUND)
endforeach (library)

if (DLOPEN_FOUND)
  if (NOT Dlopen_FIND_QUIETLY)
    message(STATUS "Found dlopen in: ${DLOPEN_LIBRARIES}")
  endif (NOT Dlopen_FIND_QUIETLY)
else (DLOPEN_FOUND)
  if (Dlopen_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find the library containing dlopen")
  endif (Dlopen_FIND_REQUIRED)
endif (DLOPEN_FOUND)

mark_as_advanced(DLOPEN_LIBRARIES)
