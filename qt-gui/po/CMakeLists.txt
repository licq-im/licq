set(po_SRCS
  be.ts
  bg_BG.ts
  cs_CZ.ts
  de.ts
  es.ts
  fi.ts
  fr.ts
  hu_HU.ts
  it.ts
  ja_JP.eucJP.ts
  pl.ts
  pt_BR.ts
  pt.ts
  ru.ts
  sr.ts
  sv.ts
  tr.ts
  uk.ts
)

# Translatable source files
file(GLOB_RECURSE translate_files
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/*.h)

if (WITH_QT5)
  set(QT_LUPDATE ${Qt5_LUPDATE_EXECUTABLE})
else (WITH_QT5)
  set(QT_LUPDATE ${QT_LUPDATE_EXECUTABLE})
endif (WITH_QT5)

# Update translations (note that [clean-]merge targets write to source directory)
# merge: update all translations
add_custom_target(${licq_target_prefix}merge
  ${QT_LUPDATE} ${translate_files} -ts ${po_SRCS}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# clean-merge: update all translations and remove obsolete strings
add_custom_target(${licq_target_prefix}clean-merge
  ${QT_LUPDATE} -noobsolete ${translate_files} -ts ${po_SRCS}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# Create template.ts to be used for new translations
add_custom_command(OUTPUT template.ts
  COMMAND ${QT_LUPDATE} -noobsolete ${translate_files} -ts template.ts
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${translate_files})

# template: create po/template.ts
add_custom_target(${licq_target_prefix}template DEPENDS template.ts)

if (WITH_QT5)
  qt5_add_translation(qms ${po_SRCS})
else (WITH_QT5)
  qt4_add_translation(qms ${po_SRCS})
endif (WITH_QT5)

# translations: build all *.qm files
add_custom_target(${licq_target_prefix}translations ALL DEPENDS ${qms})

# Install *.qm to <prefix>/share/licq/qt-gui/locale
install(FILES ${qms}
  DESTINATION ${Licq_SHARE_DIR}/${INSTALL_QTGUI_DIR}/locale)
