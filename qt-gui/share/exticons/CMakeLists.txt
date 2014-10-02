# Extended icons
set(exticons_SHARE
  basic
  kde
  thomas
)

install(DIRECTORY ${exticons_SHARE}
  DESTINATION ${Licq_SHARE_DIR}/${INSTALL_QTGUI_DIR}/exticons
  PATTERN ".svn" EXCLUDE
)
