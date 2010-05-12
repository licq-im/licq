set(licq_HEADERS
  group.h
  owner.h
  user.h
  userhistory.h
  usermanager.h
)

install(FILES ${licq_HEADERS} DESTINATION "${Licq_INCLUDE_DIR}/licq/contactlist")
