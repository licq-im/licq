# - Try to find gpgme
# Once done this will define
#
#  GPGME_FOUND - system has gpgme
#  GPGME_INCLUDE_DIRS - the gpgme include directory
#  GPGME_LIBRARIES - Link these to use gpgme
#  GPGME_DEFINITIONS - Compiler switches required for using gpgme
#
#  Copyright (c) 2007 Arne Schmitz <arne.schmitz@gmx.net>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (GPGME_LIBRARIES AND GPGME_INCLUDE_DIRS)
  # in cache already
  set(GPGME_FOUND TRUE)
else (GPGME_LIBRARIES AND GPGME_INCLUDE_DIRS)
  find_path(GPGME_INCLUDE_DIR
    NAMES
      gpgme.h
    PATHS
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
  )

  find_library(GPGME_LIBRARY
    NAMES
      gpgme
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  set(GPGME_INCLUDE_DIRS
    ${GPGME_INCLUDE_DIR}
  )
  set(GPGME_LIBRARIES
    ${GPGME_LIBRARY}
)

  if (GPGME_INCLUDE_DIRS AND GPGME_LIBRARIES)
     set(GPGME_FOUND TRUE)
  endif (GPGME_INCLUDE_DIRS AND GPGME_LIBRARIES)

  if (GPGME_FOUND)
    if (NOT gpgme_FIND_QUIETLY)
      message(STATUS "Found gpgme: ${GPGME_LIBRARIES}")
    endif (NOT gpgme_FIND_QUIETLY)
  else (GPGME_FOUND)
    if (gpgme_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find gpgme")
    endif (gpgme_FIND_REQUIRED)
  endif (GPGME_FOUND)

  # show the GPGME_INCLUDE_DIRS and GPGME_LIBRARIES variables only in the advanced view
  mark_as_advanced(GPGME_INCLUDE_DIRS GPGME_LIBRARIES)

endif (GPGME_LIBRARIES AND GPGME_INCLUDE_DIRS)

