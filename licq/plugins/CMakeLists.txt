set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

file(GLOB cmake_plugins RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} */CMakeLists.txt)

foreach (plugin ${cmake_plugins})
  get_filename_component(dir ${plugin} PATH)
  set(licq_target_prefix "${dir}_")
  add_subdirectory(${dir})
endforeach (plugin)
