set(sound_FILES
  Monty-Python.wav
)

set(sounds_DIR "${Licq_SHARE_DIR}/sounds")

install(FILES ${sound_FILES} DESTINATION ${sounds_DIR})

add_subdirectory(fun)
add_subdirectory(icq)
