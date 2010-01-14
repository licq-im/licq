set(licq_HEADERS
  condition.h
  mutex.h
  mutexlocker.h
)

install(FILES ${licq_HEADERS} DESTINATION "${Licq_INCLUDE_DIR}/licq/thread")
