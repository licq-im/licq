set(licq_HEADERS
  chat.h
  filetransfer.h
  icq.h
  icqdata.h
  owner.h
  user.h
)

install(FILES ${licq_HEADERS} DESTINATION "${Licq_INCLUDE_DIR}/licq/icq")
