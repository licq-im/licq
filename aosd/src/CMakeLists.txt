set(aosd_SRCS
  conf.cpp
  iface.cpp
  plugin.cpp
)

licq_add_plugin(licq_aosd ${aosd_SRCS})
target_link_libraries(licq_aosd ${AOSD_LIBRARIES})
