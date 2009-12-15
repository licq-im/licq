set(licq_CMAKE
  FindLicq.cmake
  LicqCommonCompilerFlags.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/LicqPaths.cmake
)

configure_file(
  LicqPaths.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/LicqPaths.cmake
  @ONLY
)

install(FILES ${licq_CMAKE} DESTINATION ${Licq_SHARE_DIR}/cmake)
