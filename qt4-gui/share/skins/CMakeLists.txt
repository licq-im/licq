# Skins
set(skins_SHARE
  basic
  basic-ncb
  bheart
  compact
  dim
  e
  gradient
  icq
  icq-ncb
  Mirabilis
  pli
  shale
  thomas
)

install(DIRECTORY ${skins_SHARE}
  DESTINATION ${INSTALL_DATA_DIR}/licq/${INSTALL_QTGUI_DIR}/skins
  PATTERN ".svn" EXCLUDE
)
