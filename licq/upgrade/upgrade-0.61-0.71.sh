#!/bin/sh
#
# Script for upgrading old 0.61 Licq installations to a newer
# format.  Takes as an argument the old licq config dierctory
# Graham Roff <graham@licq.org>
#


if [ ! -d "~/.licq" -o -z "$1" -o ! -d "$1" ]; then
  echo "upgrade-0.61-0.71.sh <old .licq dir>"
  echo "First rename ~/.licq and then run licq to recreate it."
  echo "Then run this script with the name of your old .licq dir."
  echo "ie: upgrade-0.61-0.71.sh ~/.licq-old"
  exit
fi

echo "Licq 0.61 Upgrade Script"

echo "Grabbing old files..."
#    Old file                       New File
cp  -f $1/conf/users.conf ~/.licq/users.conf
cp  -f $1/conf/owner.uin  ~/.licq/owner.uin
cp  -f $1/conf/*.uin      ~/.licq/users/
cp  -f $1/history/*       ~/.licq/history/

echo "Updating user config files..."
for i in $HOME/.licq/users/*.uin; do \
  awk 'BEGIN { s = 0 } \
       /Group/ { printf "Groups.User = %s\n", $3 } \
       /Online/ { s += 1 * $3 } \
       /Visible/ { s += 2 * $3 } \
       /Invisible/ { s += 4 * $3 } \
       !/Group/ && !/Online/ && !/Visible/ && !/Invisible/ { print $0 } \
       END { printf "Groups.System = %d\n", s } ' \
       $i > $i.new
  mv -f $i.new $i
done
echo "Done."

echo "--> Now upgrade your history files by running \"upgrade-history-0.71.pl\"."
