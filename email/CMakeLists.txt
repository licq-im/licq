project(forwarder)
cmake_minimum_required(VERSION 2.6.0)
cmake_policy(VERSION 2.6)

# Licq
find_package(Licq REQUIRED)
include_directories(${Licq_INCLUDE_DIRS})

add_subdirectory(src)
