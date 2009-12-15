set(licq_HEADERS
  ${CMAKE_CURRENT_BINARY_DIR}/licqversion.h
  version.h
)

install(FILES ${licq_HEADERS} DESTINATION "${Licq_INCLUDE_DIR}/licq")
