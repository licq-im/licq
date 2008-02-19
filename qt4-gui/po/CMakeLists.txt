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

find_program(QT_LUPDATE
  NAMES lupdate-qt4 lupdate
  PATHS $ENV{QTDIR}/bin
  DOC "Path to the lupdate program"
)

find_program(QT_LRELEASE
  NAMES lrelease-qt4 lrelease
  PATHS $ENV{QTDIR}/bin
  DOC "Path to the lrelease program"
)

# Update translations (note that [clean-]merge targets write to source directory)
if (QT_LUPDATE)
  # merge: update all translations
  add_custom_target(merge
    ${QT_LUPDATE} ${translate_files} -ts ${po_SRCS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

  # clean-merge: update all translations and remove obsolete strings
  add_custom_target(clean-merge
    ${QT_LUPDATE} -noobsolete ${translate_files} -ts ${po_SRCS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

  # Create template.ts to be used for new translations
  add_custom_command(OUTPUT template.ts
    COMMAND ${QT_LUPDATE} -noobsolete ${translate_files} -ts template.ts
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${translate_files})

  # template: create po/template.ts
  add_custom_target(template DEPENDS template.ts)
else (QT_LUPDATE)
  message(STATUS "lupdate could not be found. You will not be able to update the translations.")
endif (QT_LUPDATE)


# Build translations
if (QT_LRELEASE)
  # Clear list
  set(qms)

  # Add command to build X.qm from X.po
  foreach (po ${po_SRCS})
    get_filename_component(qm ${po} NAME_WE)
    set(qm "${CMAKE_CURRENT_BINARY_DIR}/${qm}.qm")
    set(po "${CMAKE_CURRENT_SOURCE_DIR}/${po}")
    add_custom_command(OUTPUT ${qm}
      COMMAND ${QT_LRELEASE} ${po} -qm ${qm}
      DEPENDS ${po})
    set(qms ${qms} ${qm})
  endforeach(po)

  # translations: build all *.qm files
  add_custom_target(translations ALL DEPENDS ${qms})

  # Install *.qm to <prefix>/share/licq/qt4-gui/locale
  install(FILES ${qms}
    DESTINATION ${INSTALL_DATA_DIR}/licq/${INSTALL_QTGUI_DIR}/locale)
else (QT_LRELEASE)
  message(STATUS "lrelease could not be found. You're losing the localisation.")
endif (QT_LRELEASE)
