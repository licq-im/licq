set(licq_HEADERS
  licq_buffer.h
  licq_chat.h
  licq_color.h
  licq_constants.h
  licq_events.h
  licq_file.h
  licq_filetransfer.h
  licq_history.h
  licq_icq.h
  licq_icqd.h
  licq_log.h
  licq_message.h
  licq_mutex.h
  licq_onevent.h
  licq_oscarservice.h
  licq_packets.h
  licq_plugin.h
  licq_protoplugin.h
  licq_proxy.h
  licq_socket.h
  licq_translate.h
  licq_types.h
  licq_user.h
  licq_utility.h
)

install(FILES ${licq_HEADERS} DESTINATION "${Licq_INCLUDE_DIR}/licq")

add_subdirectory(licq)
