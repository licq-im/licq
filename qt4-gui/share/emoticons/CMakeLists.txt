# Emoticons
set(emoticons_SHARE
  bugeyes
  ccmathteam.com
  Default
  FeltTip4
  Mirabilis4
  Mirabilis5
  Mirabilis5.1
  msn
  phpBB
  thomas
)

install(DIRECTORY ${emoticons_SHARE}
  DESTINATION ${INSTALL_DATA_DIR}/licq/${INSTALL_QTGUI_DIR}/emoticons
  PATTERN ".svn" EXCLUDE
)
