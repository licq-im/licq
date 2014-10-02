# Icons
set(icons_SHARE
  ami
  computer
  icq
  icq2
  kde
  mark
  thomas
)

install(DIRECTORY ${icons_SHARE}
  DESTINATION ${Licq_SHARE_DIR}/${INSTALL_QTGUI_DIR}/icons
  PATTERN ".svn" EXCLUDE
)
