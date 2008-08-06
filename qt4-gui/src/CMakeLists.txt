set(SOURCE_DIRS
  config
  contactlist
  core
  dialogs
  dockicons
  helpers
  settings
  userdlg
  userevents
  views
  widgets
)

set(objs)
foreach (dir ${SOURCE_DIRS})
  add_subdirectory(${dir})

  # CMake doesn't support "convenience libraries" (yet). This works
  # around that by getting a list of all object files from each
  # subdirectory and then linking them directly to the target. Depends
  # on internal CMake structure and may break in the future. Hopefully
  # CMake will get "convenience libraries" before that.
  set(objdir ${CMAKE_CURRENT_BINARY_DIR}/${dir}${CMAKE_FILES_DIRECTORY}/${dir}.dir) 
  get_directory_property(${dir} DIRECTORY ${dir} DEFINITION ${dir})
  foreach (file ${${dir}})
    get_filename_component(basename ${file} NAME_WE)
    list(APPEND objs ${objdir}/${basename}${CMAKE_C_OUTPUT_EXTENSION})
  endforeach (file)
endforeach (dir)
set_source_files_properties(${objs} PROPERTIES GENERATED true)

if (WITH_KDE)
  set(target licq_kde4-gui)
else (WITH_KDE)
  set(target licq_qt4-gui)
endif (WITH_KDE)

add_library(${target} MODULE ${objs})
add_dependencies(${target} ${SOURCE_DIRS})
target_link_libraries(${target} ${LIBRARIES})
set_target_properties(${target} PROPERTIES PREFIX "")
set_target_properties(${target} PROPERTIES LINKER_LANGUAGE CXX)

install(TARGETS ${target} DESTINATION ${INSTALL_LIB_DIR}/licq)

if (APPLE)
  set_target_properties(${target} PROPERTIES
    LINK_FLAGS "-flat_namespace -undefined suppress")
endif (APPLE)
