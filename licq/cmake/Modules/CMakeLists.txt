# Dist some modules so that plugins can use them
set(Licq_CMAKE_MODULES
  FindIconv.cmake
)

install(FILES ${Licq_CMAKE_MODULES}
  DESTINATION ${Licq_SHARE_DIR}/cmake/Modules)
