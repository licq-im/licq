set(newosd_SRCS
  conf.cpp
  iface.cpp
  plugin.cpp
)

licq_add_plugin(licq_newosd ${newosd_SRCS})
target_link_libraries(licq_newosd ${AOSD_LIBRARIES})
