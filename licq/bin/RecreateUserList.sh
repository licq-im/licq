#!/bin/sh
# This file is part of Licq, an instant messaging client for UNIX.
# Copyright (C) 2000,2005,2011 Licq developers
#
# Licq is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# Licq is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Licq; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

BASE=${HOME}/.licq
CONF="${BASE}/users.conf"

echo "Licq User List Recreation App"
echo
echo "This script will rebuild your user list from the user config files found"
echo "in ${BASE}.  It will overwrite your old list."
echo "Continue (y/N)?"
read CONTINUE

if [ "$CONTINUE" != "y" -a "$CONTINUE" != "Y" ]; then
  echo "Aborting."
  exit
fi 

echo "Rebuilding user list configuration file in $BASE..."
cd ${BASE}/users
echo "[users]" > $CONF
ls -1 *.Licq *.MSN_ *.XMPP | awk 'BEGIN { i = 0 } { printf "User%d=%s\n", ++i, $1 } END { printf "NumOfUsers=%d\n", i }' >> $CONF
echo "Done"
