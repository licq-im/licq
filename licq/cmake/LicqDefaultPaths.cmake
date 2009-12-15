# This file sets up the default installation paths used by the daemon. It is
# also used by plugins when configuring the plugin to build using headers in
# the source directory.

set(Licq_INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include"      CACHE PATH "Licq include directory")
set(Licq_LOCALE_DIR  "${CMAKE_INSTALL_PREFIX}/share/locale" CACHE PATH "Licq locale directory")
set(Licq_PLUGIN_DIR  "${CMAKE_INSTALL_PREFIX}/lib/licq"     CACHE PATH "Licq plugin directory")
set(Licq_SHARE_DIR   "${CMAKE_INSTALL_PREFIX}/share/licq"   CACHE PATH "Licq share directory")
