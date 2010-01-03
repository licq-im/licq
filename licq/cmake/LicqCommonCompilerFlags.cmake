include(TestCXXAcceptsFlag)

# Pthreads is required for all plugins
find_package(Threads REQUIRED)
if (NOT CMAKE_USE_PTHREADS_INIT)
  message(FATAL_ERROR "Could not find POSIX threads")
endif (NOT CMAKE_USE_PTHREADS_INIT)
add_definitions(-D_REENTRANT)

# Boost is required for all plugins as it is used in Licq includes
find_package(Boost 1.36.0 REQUIRED)
link_directories(${Boost_LIBRARY_DIRS})
include_directories(${Boost_INCLUDE_DIR})

# Add -Wall if supported by compiler
check_cxx_accepts_flag(-Wall CXX_ACCEPTS_WALL)
if (CXX_ACCEPTS_WALL)
  add_definitions(-Wall)
endif (CXX_ACCEPTS_WALL)

# Add -Wextra if supported by compiler
check_cxx_accepts_flag(-Wextra CXX_ACCEPTS_WEXTRA)
if (CXX_ACCEPTS_WEXTRA)
  add_definitions(-Wextra)
endif (CXX_ACCEPTS_WEXTRA)
