# Dock icons
set(dock_SHARE
  console
  flower
  flower-ns
  glicq
  kde2
  pli
)

install(DIRECTORY ${dock_SHARE}
  DESTINATION ${INSTALL_DATA_DIR}/licq/${INSTALL_QTGUI_DIR}/dock
  PATTERN ".svn" EXCLUDE
)
