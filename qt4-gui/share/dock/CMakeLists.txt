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
  DESTINATION ${Licq_SHARE_DIR}/${INSTALL_QTGUI_DIR}/dock
  PATTERN ".svn" EXCLUDE
)
