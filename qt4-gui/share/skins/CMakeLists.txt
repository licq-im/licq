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
  DESTINATION ${Licq_SHARE_DIR}/${INSTALL_QTGUI_DIR}/skins
  PATTERN ".svn" EXCLUDE
)
