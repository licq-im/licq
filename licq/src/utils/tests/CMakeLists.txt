set(test_SRCS
  ../dynamiclibrary.cpp
  dynamiclibrarytest.cpp
)

add_executable(testutils ${test_SRCS})
include_directories(${GTEST_INCLUDE_DIRS})

target_link_libraries(testutils ${GTEST_BOTH_LIBRARIES})
target_link_libraries(testutils ${DLOPEN_LIBRARIES})

add_test(utils testutils)
add_dependencies(tests testutils)
