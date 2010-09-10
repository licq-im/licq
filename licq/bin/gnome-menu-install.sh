#!/bin/sh
cp ../plugins/qt4-gui/src/xpm/licq.xpm `gnome-config --datadir`/pixmaps
echo "" > `gnome-config --datadir`/gnome/apps/Internet/Licq.desktop
echo "[Desktop Entry]" >> `gnome-config --datadir`/gnome/apps/Internet/Licq.desktop
echo "Name=Licq" >> `gnome-config --datadir`/gnome/apps/Internet/Licq.desktop
echo "Comment=Licq - ICQ client" >> `gnome-config --datadir`/gnome/apps/Internet/Licq.desktop
echo "Exec=licq -p qt4-gui" >> `gnome-config --datadir`/gnome/apps/Internet/Licq.desktop
echo "Icon=/usr/local/share/pixmaps/iconLicq.xpm" >> `gnome-config --datadir`/gnome/apps/Internet/Licq.desktop
echo "Terminal=false" >> `gnome-config --datadir`/gnome/apps/Internet/Licq.desktop
echo "MultipleArgs=false" >> `gnome-config --datadir`/gnome/apps/Internet/Licq.desktop
echo "Type=Application" >> `gnome-config --datadir`/gnome/apps/Internet/Licq.desktop

