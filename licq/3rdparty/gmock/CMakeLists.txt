include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(gmock STATIC src/gmock-all.cc)
add_library(gmock_main STATIC src/gmock_main.cc)
