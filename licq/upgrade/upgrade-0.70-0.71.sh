#!/bin/sh

echo "Licq 0.70 Upgrade Script."

# Check version to avoid problems - implamented by security@solarweb.com
#if [ "$verpass" = "0" ];then
#  echo "ERROR - version incorrect"
#  echo "This script was not meant for your Licq version"
#  exit;
#fi

echo "Updating user config files..."
for i in $HOME/.licq/users/*.uin $HOME/.licq/owner.uin; do 
  TEST=`grep Groups.System $i`
  if [ -z  "$TEST" ]; then
    awk 'BEGIN { s = 0 } \
         /Group/ { printf "Groups.User = %s\n", $3 } \
         /Online/ { s += 1 * $3 } \
         /Visible/ { s += 2 * $3 } \
         /Invisible/ { s += 4 * $3 } \
         !/Group/ && !/Online/ && !/Visible/ && !/Invisible/ { print $0 } \
         END { printf "Groups.System = %d\n", s } ' \
         $i > $i.new
    mv -f $i.new $i
  fi
done

echo "Updating licq config file..."
TEST=`grep "Version =" $HOME/.licq/licq.conf`
if [ -z "$TEST" ]; then
  awk 'BEGIN { printf "[licq]\nVersion = 710\n\n" } \
       /NumOfGroups/ { printf "NumOfGroups = %d\nGroup1.name = New Users\nNewUserGroup = 1\n", $3 + 1 } \
       /Group.*name/ { printf "Group%d.name = %s\n", substr($1, 6, 2) + 1, $3 } \
       /DefaultGroup/ { printf "DefaultGroup = %d\n", $3 - 1 } \
       !/NumOfGroups/ && !/Group.*name/ && !/DefaultGroup/ { print $0 } ' \
      $HOME/.licq/licq.conf > $HOME/.licq/licq.conf.new
  mv -f $HOME/.licq/licq.conf.new $HOME/.licq/licq.conf
fi

echo "Done."

echo "--> Now upgrade your history files by running \"upgrade-history-0.71.pl\"."

