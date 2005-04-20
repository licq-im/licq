#!/bin/sh

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
NumOfUsers=`ls *.uin *.Licq *.MSN_ | wc -l`
echo "NumOfUsers = ${NumOfUsers}" >> $CONF
ls *.Licq *.MSN_ *.uin | awk 'BEGIN { i = 1 } { printf "User%d = %s\n", i++, $1 }' >> $CONF
echo "Done"

