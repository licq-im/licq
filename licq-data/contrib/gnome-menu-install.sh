#!/bin/sh
cp ../plugins/qt-gui-0.65/src/xpm/iconLicq.xpm `gnome-config --datadir`/pixmaps
echo "" > `gnome-config --datadir`/gnome/apps/Internet/Licq.desktop
echo "[Desktop Entry]" >> `gnome-config --datadir`/gnome/apps/Internet/Licq.desktop
echo "Name=Licq" >> `gnome-config --datadir`/gnome/apps/Internet/Licq.desktop
echo "Comment=Licq - ICQ client" >> `gnome-config --datadir`/gnome/apps/Internet/Licq.desktop
echo "Exec=licq -p qt-gui" >> `gnome-config --datadir`/gnome/apps/Internet/Licq.desktop
echo "Icon=/usr/local/share/pixmaps/iconLicq.xpm" >> `gnome-config --datadir`/gnome/apps/Internet/Licq.desktop
echo "Terminal=false" >> `gnome-config --datadir`/gnome/apps/Internet/Licq.desktop
echo "MultipleArgs=false" >> `gnome-config --datadir`/gnome/apps/Internet/Licq.desktop
echo "Type=Application" >> `gnome-config --datadir`/gnome/apps/Internet/Licq.desktop

