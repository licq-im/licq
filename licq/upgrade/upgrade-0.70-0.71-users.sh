#!/bin/sh

echo "Updating user config files."
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

echo "Updating licq config file."
awk '/NumOfGroups/ { printf "NumOfGroups = %d\nGroup1.name = New Users\nNewUserGroup = 1\n", $3 + 1 } \
     /Group.*name/ { printf "Group%d.name = %s\n", substr($1, 6, 2) + 1, $3 } \
     /DefaultGroup/ { printf "DefaultGroup = %d\n", $3 - 1 } \
     !/NumOfGroups/ && !/Group.*name/ && !/DefaultGroup/ { print $0 } ' \
    $HOME/.licq/licq.conf > $HOME/.licq/licq.conf.new
mv -f $HOME/.licq/licq.conf.new $HOME/.licq/licq.conf

