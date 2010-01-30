# This file sets up the default installation paths used by the daemon. It is
# also used by plugins when configuring the plugin to build using headers in
# the source directory.

set(Licq_INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include")
set(Licq_LOCALE_DIR "${CMAKE_INSTALL_PREFIX}/share/locale")
set(Licq_PLUGIN_DIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/licq")
set(Licq_SHARE_DIR "${CMAKE_INSTALL_PREFIX}/share/licq")
