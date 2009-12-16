project(msn)
cmake_minimum_required(VERSION 2.6.0)
cmake_policy(VERSION 2.6)

include(CheckFunctionExists)

# Licq
find_package(Licq REQUIRED)
include_directories(${Licq_INCLUDE_DIRS})

# Boost
find_package(Boost 1.31.1 COMPONENTS smart_ptr)
link_directories(${Boost_LIBRARY_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

# strndup
check_function_exists(strndup have_strndup)
if (have_strndup)
  add_definitions(-DHAVE_STRNDUP=1)
endif (have_strndup)

add_subdirectory(src)
