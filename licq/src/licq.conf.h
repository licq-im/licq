static const char LICQ_CONF[] =
"
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
";

