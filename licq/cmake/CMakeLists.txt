set(licq_CMAKE
  cmake_uninstall.cmake.in
  FindLicq.cmake
  LicqCommonCompilerFlags.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/LicqConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/LicqPaths.cmake
)

configure_file(
  LicqConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/LicqConfig.cmake
  @ONLY
)

configure_file(
  LicqPaths.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/LicqPaths.cmake
  @ONLY
)

install(FILES ${licq_CMAKE} DESTINATION ${Licq_SHARE_DIR}/cmake)

add_subdirectory(Modules)
