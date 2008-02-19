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
  DESTINATION ${INSTALL_DATA_DIR}/licq/${INSTALL_QTGUI_DIR}/icons
  PATTERN ".svn" EXCLUDE
)
