# Extended icons
set(exticons_SHARE
  basic
  kde
  thomas
)

install(DIRECTORY ${exticons_SHARE}
  DESTINATION ${INSTALL_DATA_DIR}/licq/${INSTALL_QTGUI_DIR}/exticons
  PATTERN ".svn" EXCLUDE
)
