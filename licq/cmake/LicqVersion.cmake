#
# Generates include/licq/licqversion.h
#

# Licq version
set(LICQ_VERSION_MAJOR 1)          # 0 <= major   <= 64
set(LICQ_VERSION_MINOR 8)          # 0 <= minor   <= 99
set(LICQ_VERSION_RELEASE 1)        # 0 <= release <=  9
set(LICQ_VERSION_EXTRA "-dev")     # Any string

# When building from a git clone, set the extra version to the HEAD revision,
# replacing any existing value.
find_program(licq_git git)
if (licq_git)
  if (NOT LICQ_VERSION_SOURCE_DIR)
    set(LICQ_VERSION_SOURCE_DIR ${PROJECT_SOURCE_DIR})
  endif (NOT LICQ_VERSION_SOURCE_DIR)

  execute_process(COMMAND ${licq_git} rev-parse HEAD
    WORKING_DIRECTORY ${LICQ_VERSION_SOURCE_DIR}
    RESULT_VARIABLE licq_git_result
    OUTPUT_VARIABLE licq_git_output
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  if (${licq_git_result} EQUAL 0)
    string(SUBSTRING ${licq_git_output} 0 7 licq_git_short)
    set(LICQ_VERSION_EXTRA "-${licq_git_short}")
  endif (${licq_git_result} EQUAL 0)
endif (licq_git)

# licqversion.h content
set(licq_version_file "${CMAKE_CURRENT_BINARY_DIR}/include/licq/licqversion.h")
set(licq_old_version "")
set(licq_new_version 
  "// Autogenerated by CMake. Do not edit, changes will be lost.
#ifndef LICQ_LICQVERSION_H
#define LICQ_LICQVERSION_H

#define LICQ_VERSION_MAJOR ${LICQ_VERSION_MAJOR}
#define LICQ_VERSION_MINOR ${LICQ_VERSION_MINOR}
#define LICQ_VERSION_RELEASE ${LICQ_VERSION_RELEASE}
#define LICQ_VERSION_EXTRA \"${LICQ_VERSION_EXTRA}\"
#define LICQ_VERSION_STRING \"${LICQ_VERSION_MAJOR}.${LICQ_VERSION_MINOR}.${LICQ_VERSION_RELEASE}${LICQ_VERSION_EXTRA}\"

#endif
")

# Read in the old file (if it exists)
if (EXISTS ${licq_version_file})
  file(READ ${licq_version_file} licq_old_version)
endif (EXISTS ${licq_version_file})

# Only write the file if the content has changed
string(COMPARE NOTEQUAL
  "${licq_old_version}" "${licq_new_version}"
  licq_update_version_file)

if (${licq_update_version_file})
  message(STATUS "Creating ${licq_version_file}")
  file(WRITE ${licq_version_file} ${licq_new_version})
endif (${licq_update_version_file})
