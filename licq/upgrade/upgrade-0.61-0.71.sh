#!/bin/sh
#
# Script for upgrading old 0.61 Licq installations to a newer
# format.  Takes as an argument the old licq config dierctory
# Graham Roff <graham@licq.org>
#


if [ ! -d "$HOME/.licq" -o ! -z "$1" ]; then
  echo "upgrade-0.61-0.71.sh"
  echo "Upgrades a 0.61 Licq config directory to 0.71."
  exit
fi

if [ ! -d "$HOME/.licq/conf" ]; then
  echo "No previous version of Licq found."
  exit
fi

echo "Licq 0.61 Upgrade Script"

echo "Updating old locations..."
cd $HOME/.licq
mv conf/users.conf .
mv conf/owner.uin .
mv conf users

echo "Creating new licq.conf file..."
echo "
[licq]
Version = 710

[plugins]
NumPlugins = 1
Plugin1 = qt-gui

[network]
AllowNewUsers = 1
NumOfServers = 3
DefaultServerPort = 4000
Server1 = icq.mirabilis.com
Server2 = icq2.mirabilis.com
Server3 = icq3.mirabilis.com
TCPServerPort = 0
MaxUsersPerPacket = 100
Errors = log.errors
Rejects = log.rejects
Translation = none
UrlViewer = viewurl-netscape.sh
Terminal = xterm -T Licq -e 

[onevent]
Enable = 1
Command = play
Message = ~/wavs/message.wav
Chat = ~/wavs/chat.wav
File = ~/wavs/file.wav
Url = ~/wavs/url.wav
OnlineNotify = ~/wavs/notify.wav
SysMsg = ~/wavs/sysmsg.wav

[groups]
NumOfGroups = 3
DefaultGroup = 0
NewUserGroup = 1
Group1.name = New Users
Group2.name = Friends
Group3.name = Family
" > licq.conf

echo "Updating user config files..."
for i in users/*.uin owner.uin; do \
  TEST=`grep Groups.System $i`
  if [ -z  "$TEST" ]; then
    awk 'BEGIN { s = 0 } \
         /Online/ { s += 1 * $3 } \
         /Visible/ { s += 2 * $3 } \
         /Invisible/ { s += 4 * $3 } \
         !/Online/ && !/Visible/ && !/Invisible/ { print $0 } \
         END { printf "Groups.System = %d\nGroups.User = 0\n", s } ' \
         $i > $i.new
    mv -f $i.new $i
  fi
done
echo "Done."

echo "--> Now upgrade your history files by running \"upgrade-history-0.71.pl\"."
