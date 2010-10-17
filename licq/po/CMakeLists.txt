set(po_FILES
  be
  de
  es
)

find_package(Gettext REQUIRED)

if (NOT GETTEXT_MSGMERGE_EXECUTABLE AND NOT GETTEXT_MSGFMT_EXECUTABLE)
  message(FATAL_ERROR "Gettext message catalog tools NOT found")
endif (NOT GETTEXT_MSGMERGE_EXECUTABLE AND NOT GETTEXT_MSGFMT_EXECUTABLE)

find_program(GETTEXT_XGETTEXT_EXECUTABLE xgettext)
if (GETTEXT_XGETTEXT_EXECUTABLE)
  # Get list of all source files
  file(GLOB_RECURSE po_source
    RELATIVE ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/src/*.c
    ${PROJECT_SOURCE_DIR}/src/*.cpp
  )

  add_custom_target(translations_update
    ${GETTEXT_XGETTEXT_EXECUTABLE}
      --directory ${PROJECT_SOURCE_DIR}
      --output="${CMAKE_CURRENT_SOURCE_DIR}/licq.pot"
      --default-domain=licq
      --add-comments=TRANSLATORS:
      --keyword=tr
      --keyword=tr_NOOP
      --copyright-holder='Licq Developers'
      --msgid-bugs-address="licq-dev@googlegroups.com"
      --sort-output
      ${po_source}
    COMMENT "Updating licq.pot"
  )
endif (GETTEXT_XGETTEXT_EXECUTABLE)

foreach(lang ${po_FILES})
  set(po  "${CMAKE_CURRENT_SOURCE_DIR}/${lang}.po")
  set(gmo "${CMAKE_CURRENT_BINARY_DIR}/${lang}.gmo")

  # Update po files after pot file
  add_custom_command(TARGET translations_update POST_BUILD
    COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --quiet --update --backup=none
      --sort-output ${po} ${CMAKE_CURRENT_SOURCE_DIR}/licq.pot
    COMMENT "Updating ${lang}.po"
  )

  # Add command to build X.gmo from X.po
  add_custom_command(OUTPUT ${gmo}
    COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} -o ${gmo} ${po}
    DEPENDS ${po}
  )

  install(FILES ${gmo}
    DESTINATION "${Licq_LOCALE_DIR}/${lang}/LC_MESSAGES"
    RENAME licq.mo
  )

  set(gmoFiles ${gmoFiles} ${gmo})
endforeach(lang)

add_custom_target(translations ALL DEPENDS ${gmoFiles})
