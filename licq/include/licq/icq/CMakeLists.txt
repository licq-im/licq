set(licq_HEADERS
  chat.h
  codes.h
  filetransfer.h
  icq.h
  owner.h
  user.h
)

install(FILES ${licq_HEADERS} DESTINATION "${Licq_INCLUDE_DIR}/licq/icq")
