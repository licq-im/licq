#!/bin/sh

BASE=~/.licq
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
echo "NumOfUsers = `ls *.uin | wc -l`" >> $CONF
ls *.uin | awk 'BEGIN { i = 1 } { printf "User%d = %d\n", i++, $1 }' >> $CONF
echo "Done"

