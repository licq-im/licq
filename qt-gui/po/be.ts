<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS><TS version="1.1" language="be_BY">
<defaultcodec></defaultcodec>
<context>
    <name>@default</name>
    <message>
        <location filename="" line="0"/>
        <source>KDE default</source>
        <translation>KDE па-змоўчаньню</translation>
    </message>
</context>
<context>
    <name>AddUserDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>Protocol:</source>
        <translation>Пратакол:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>New User ID:</source>
        <translation>Новы нумар</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Ok</source>
        <translation>&amp;Добра</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Licq - Add User</source>
        <translation>Licq - Дадаць карыстальніка</translation>
    </message>
</context>
<context>
    <name>AuthUserDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>Licq - Grant Authorisation</source>
        <translation>Licq - Дазволіць аўтарызацыю</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Licq - Refuse Authorisation</source>
        <translation>Licq - Адмовіць у аўтарызацыі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Authorize which user (Id):</source>
        <translation>Аўтарызаваць карыстальніка:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Grant authorization to %1</source>
        <translation>Аўтарызаваць %1</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Refuse authorization to %1</source>
        <translation>Адказаць у аўтарызацыі %1</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Response</source>
        <translation>Адказ</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Ok</source>
        <translation>&amp;Добра</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Authorize which user (UIN):</source>
        <translation>Аўтарызаваць карыстальніка:</translation>
    </message>
</context>
<context>
    <name>AwayMsgDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>&lt;h2&gt;Hints for Setting&lt;br&gt;your Auto-Response&lt;/h2&gt;&lt;br&gt;&lt;hr&gt;&lt;br&gt;&lt;ul&gt;&lt;li&gt;You can include any of the % expansions (described in the main hints page).&lt;/li&gt;&lt;li&gt;Any line beginning with a pipe (|) will be treated as a command to be run.  The line will be replaced by the output of the command. The command is parsed by /bin/sh so any shell commands or meta-characters are allowed.  For security reasons, any % expansions are automatically passed to the command surrounded by single quotes to prevent shell parsing of any meta-characters included in an alias.&lt;br&gt;Examples of popular uses include:&lt;ul&gt;&lt;li&gt;&lt;tt&gt;|date&lt;/tt&gt;: Will replace that line by the current date&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|fortune&lt;/tt&gt;: Show a fortune, as a tagline for example&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|myscript.sh %u %a&lt;/tt&gt;: Run a script, passing the uin and alias&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|myscript.sh %u %a &gt; /dev/null&lt;/tt&gt;: Run the same script but ignore the output (for tracking auto response checks or something)&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|if [ %u -lt 100000 ]; then echo &quot;You are special&quot;; fi&lt;/tt&gt;: Useless, but shows how you can use shell script.&lt;/li&gt;&lt;/ul&gt;Of course, multiple &quot;|&quot; can appear in the auto response, and commands and regular text can be mixed line by line.&lt;/li&gt;&lt;hr&gt;&lt;p&gt; For more information, see the Licq webpage (&lt;tt&gt;http://www.licq.org&lt;/tt&gt;).&lt;/p&gt;</source>
        <translation>&lt;h2&gt;Парады дзеля наладак&lt;br&gt;Паведамленьняў аўтаматычнага адказу&lt;/h2&gt;&lt;br&gt;&lt;hr&gt;&lt;br&gt;&lt;ul&gt;&lt;li&gt;Вы маеце магчымасць выкарыстоўваць любую з %-зьменных (Апісаньне іх магчыма знайсці на старонцы &quot;Парады&quot;).&lt;/li&gt;&lt;li&gt;Любы з радкоў, які пачынаецца з рысы (|) будзе апрацаваны як каманда,а вывад каманды будзе ўстаўлены ў тэкст паведамленьня. Усе каманды інтэрпрэтыруюцца /bin/sh , таму ўсе каманды і мэта-сымбалі дапушчальны. У мэтах бясьпекі ўсе %-зьменныя аўтаматычна апрацоўваюцца і экрануюцца адзінарнымі коскамі і ў гэткім выглядзе передаюцца апрацоўшчыку каманд, каб апошні правільна срэагаваў на мэта-сымбалі.&lt;br&gt;Прыклады найбольш часта ўжываемых варыянтаў:&lt;ul&gt;&lt;li&gt;&lt;tt&gt;|date&lt;/tt&gt;: будзе заменена на цякучую дату і час&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|fortune&lt;/tt&gt;: будзе ўстаўлена выпадковая цытата з праграмы &quot;fortune&quot;(на ангельскай мове)&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|myscript.sh %u %a&lt;/tt&gt;: выканае скрыпт і перадаст яму ў якасці параметраў UIN і мянушку&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|myscript.sh %u %a &gt; /dev/null&lt;/tt&gt;: Тое ж самае, толькі вывад будзе схаваны (напрыклад, дзеля нейкіх сваіх патрэбнасцяў)&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|if [ %u -lt 100000 ]; then echo &quot;You are special&quot;; fi&lt;/tt&gt;: Просты прыклад выкарыстаньня скрыпта.&lt;/li&gt;&lt;/ul&gt;Безумоўна, дапускальна ўжываньне некалькіх каманд, падзеленых сымбалямі &quot;|&quot; ,пры гэтым яны будуць апрацаваны радок за радком.&lt;/li&gt;&lt;hr&gt;&lt;p&gt; Больш падрабязную інфармацыю магчыма знайсьці на старонцы (&lt;tt&gt;http://www.licq.org&lt;/tt&gt;).&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>(Closing in %1)</source>
        <translation>(Зачыніць праз %1)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Select</source>
        <translation>&amp;Абраць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Hints</source>
        <translation>&amp;Парады</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Ok</source>
        <translation>&amp;Добра</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Edit Items</source>
        <translation>&amp;Рэдагаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Set %1 Response for %2</source>
        <translation>Усталяваць адказ дзеля %2: %1</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>I&apos;m currently %1, %a.
You can leave me a message.
(%m messages pending from you).</source>
        <translation>Я зараз %1, %a.
Калі жадаеце, пакіньце мне паведамленьне
(%m паведамленьняў ад Вас)</translation>
    </message>
</context>
<context>
    <name>CEditFileListDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>Licq - Files to send</source>
        <translation>Licq - Файлы дзеля адсылкі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Done</source>
        <translation>&amp;Скончыць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Up</source>
        <translation>&amp;Угару</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Down</source>
        <translation>У&amp;ніз</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Delete</source>
        <translation>&amp;Выдаліць</translation>
    </message>
</context>
<context>
    <name>CEmoticons</name>
    <message>
        <location filename="" line="0"/>
        <source>Default</source>
        <translation>Па-змўчаньню</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>None</source>
        <translation>Адсутнічае</translation>
    </message>
</context>
<context>
    <name>CFileDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>Licq - File Transfer (%1)</source>
        <translation>Licq - Перадача файла (%1)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Current:</source>
        <translation>Цякучы:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>File name:</source>
        <translation>Імя файла:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>File:</source>
        <translation>Файл:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Batch:</source>
        <translation>Прагрэс:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Time:</source>
        <translation>Час:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>ETA:</source>
        <translation>Засталося:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Cancel Transfer</source>
        <translation>&amp;Скасаваць перадачу</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Open</source>
        <translation>А&amp;дчыніць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>O&amp;pen Dir</source>
        <translation>Ад&amp;чыніць каталёг</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>File transfer cancelled.</source>
        <translation>Перадача файла скасавана.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Close</source>
        <translation>Зачыніць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>%1/%2</source>
        <translation>%1/%2</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Waiting for connection...</source>
        <translation>Чаканьне злучэньня...</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Receiving file...</source>
        <translation>Атрыманьне файла...</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Sending file...</source>
        <translation>Адпраўленьне файла...</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Received %1 from %2 successfully.</source>
        <translation>Файл %1 ад %2 пасьпяхова атрыманы.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Sent %1 to %2 successfully.</source>
        <translation>Файл %1 пасьпяхова адасланы да %2.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>File transfer complete.</source>
        <translation>Перадача файлаў скончана.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>OK</source>
        <translation>Добра</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Remote side disconnected.</source>
        <translation>Адлеглы бок зачыніў злучэньне.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>File I/O error: %1.</source>
        <translation>Памылка В/В: %1.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>File I/O Error:
%1

See Network Window for details.</source>
        <translation>Памылка В/В:
%1

Падрабязнасьці ў акенцы сеткі.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Handshaking error.
</source>
        <translation>Памылка ўсталяваньня сувязі.
</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Handshake Error.
See Network Window for details.</source>
        <translation>Памылка ўсталяваньня сувязі.
Падрабязнасьці ў акенцы сеткі.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Connection error.
</source>
        <translation>Памылка злучэньня.
</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Unable to reach remote host.
See Network Window for details.</source>
        <translation>Адлеглы камп&apos;ютар недасягальны.
Падрабязнасьці ў акенцы сеткі.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Bind error.
</source>
        <translation>Памылка парта.
</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Unable to bind to a port.
See Network Window for details.</source>
        <translation>Немагчыма адчыніць порт.
Глядзі акенца сеткі дзеля атрыманьня падрабязнасьцей</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Not enough resources.
</source>
        <translation>Недастаткова рэсурсаў.
</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Unable to create a thread.
See Network Window for details.</source>
        <translation>Немагчыма адчыніць паток.
Падрабязнасьці ў акенцы сеткі.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Connecting to remote...</source>
        <translation>Злучэньне з адлеглым камп&apos;ютарам...</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>MB</source>
        <translation>МБ</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>KB</source>
        <translation>КБ</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Bytes</source>
        <translation>Байтаў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Byte</source>
        <translation>Байт</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>File already exists and is at least as big as the incoming file.</source>
        <translation>Файл ужо йснуе і мае большы памер.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Overwrite</source>
        <translation>Перазапісаць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Cancel</source>
        <translation>Скасаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>File already exists and appears incomplete.</source>
        <translation>Файл ужо йснуе і выглядае няпоўным.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Resume</source>
        <translation>Працягнуць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Open error - unable to open file for writing.</source>
        <translation>Памылка пры адчыненьні файла - немагчыма адчыніць файл дзеля запісу.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Retry</source>
        <translation>Паўтарыць</translation>
    </message>
</context>
<context>
    <name>CForwardDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>Message</source>
        <translation>Паведамленьне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>URL</source>
        <translation>Спасылка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Unable to forward this message type (%d).</source>
        <translation>Немагчыма перанакіраваць данны тып паведамленьня (%d).</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Forward %1 To User</source>
        <translation>Перанакіраваньне %1 да карыстальніка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Drag the user to forward to here:</source>
        <translation>Перацягніце карыстальніка сюды дзеля перанакіраваньня</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Forward</source>
        <translation>&amp;Перанакіраваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Forwarded message:
</source>
        <translation>Перанакіраванае паведамленьне:
</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Forwarded URL:
</source>
        <translation>Перанакіраваная спасылка:
</translation>
    </message>
</context>
<context>
    <name>CInfoField</name>
    <message>
        <location filename="" line="0"/>
        <source>Unknown</source>
        <translation>Не вядома</translation>
    </message>
</context>
<context>
    <name>CJoinChatDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>Select chat to invite:</source>
        <translation>Абярыце размову да якой жадаеце далучыцца:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Invite to Join Chat</source>
        <translation>Папрасіць далучыцца да размовы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Invite</source>
        <translation>&amp;Запрасіць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Select chat to join:</source>
        <translation>Абярыце размову дзеля далучэньня</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Join Multiparty Chat</source>
        <translation>Далучыцца да шматкарыстальніцкай размовы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Join</source>
        <translation>&amp;Далучыцца</translation>
    </message>
</context>
<context>
    <name>CLicqMessageBox</name>
    <message>
        <location filename="" line="0"/>
        <source>Licq</source>
        <translation>Licq</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;List</source>
        <translation>С&amp;ьпіс</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Next</source>
        <translation>&amp;Наступнае</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Ok</source>
        <translation>&amp;Добра</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Clear All</source>
        <translation>Ачысьціць &amp;усё</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Next (%1)</source>
        <translation>&amp;Наступнае (%1)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Licq Information</source>
        <translation>Licq Інфармацыя</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Licq Warning</source>
        <translation>Licq Папярэджваньне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Licq Critical</source>
        <translation>Licq Памылка</translation>
    </message>
</context>
<context>
    <name>CMMSendDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Multiple Recipient Message</source>
        <translation>Паведамленьне да некалькіх карыстальнікаў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Multiple Recipient URL</source>
        <translation>Спасылка да некалькіх карыстальнікаў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Multiple Recipient Contact List</source>
        <translation>Сьпіс кантактаў да некалькіх карыстальнікаў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>failed</source>
        <translation>няўдала</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Close</source>
        <translation>З&amp;ачыніць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Sending mass message to %1...</source>
        <translation>Адпраўленьне шматкарыстальніцкага паведамленьня да %1...</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Sending mass URL to %1...</source>
        <translation>Адпраўленьне шматкарыстальніцкай спасылкі да %1...</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Sending mass list to %1...</source>
        <translation>Адпраўленьне шматкарыстальніцкага сьпісу да %1...</translation>
    </message>
</context>
<context>
    <name>CMMUserView</name>
    <message>
        <location filename="" line="0"/>
        <source>Remove</source>
        <translation>Выдаліць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Crop</source>
        <translation>Сабраць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Clear</source>
        <translation>Ачысціць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Add Group</source>
        <translation>Дадаць гурт</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Add All</source>
        <translation>Дадаць усіх</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Drag&apos;n&apos;Drop didn&apos;t work</source>
        <translation>Drag&apos;n&apos;Drop не працуе</translation>
    </message>
</context>
<context>
    <name>CMainWindow</name>
    <message>
        <location filename="" line="0"/>
        <source>Licq (%1)</source>
        <translation>Лінукс ICQ - Licq (%1)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>System</source>
        <translation>Мяню</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;System</source>
        <translation>&amp;Мяню</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Right click - User groups
Double click - Show next message</source>
        <translation>Клік правай кнопкай мышы - гурты карыстальнікаў
Падвойны клік - праглядзець наступнае паведамленьне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Right click - Status menu
Double click - Set auto response</source>
        <translation>Клік правай кнопкай мышы - мяню стану
Падвойны клік - усталяваць аўтаматычны адказ</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Error! No owner set</source>
        <translation>Памылка! Уладар не вызначаны</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&lt;b&gt;%1&lt;/b&gt; is online</source>
        <translation>&lt;b&gt;%1&lt;/b&gt; у сетцы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Other Users</source>
        <translation>Іншыя карыстальнікі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>SysMsg</source>
        <translation>СістПав</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>System Message</source>
        <translation>Сістэмнае паведамленьне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>%1 msg%2</source>
        <translation>%1 пав.-%2</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source> </source>
        <translation>е</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>s</source>
        <translation>яў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>%1 message%2</source>
        <translation>%1 паведамленьн%2</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>No msgs</source>
        <translation>0 пав.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>No messages</source>
        <translation>Няма паведамленьняў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Server Group</source>
        <translation>Гурты на паслужніку</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Online</source>
        <translation>У &amp;сетцы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Away</source>
        <translation>&amp;Адсутнічаю</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Not Available</source>
        <translation>&amp;Нядасягальны</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>O&amp;ccupied</source>
        <translation>&amp;Заняты</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Do Not Disturb</source>
        <translation>Не &amp;турбаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Free for C&amp;hat</source>
        <translation>З&amp;годны размаўляць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>O&amp;ffline</source>
        <translation>В&amp;ыключаны</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Invisible</source>
        <translation>Ня&amp;бачны</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Info</source>
        <translation>&amp;Дадзеныя</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>View &amp;History</source>
        <translation>&amp;Прагляд гісторыі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Logon failed.
See network window for details.</source>
        <translation>Немагчыма ўсталяваць сувязь.
Падрабязнасьці ў акенцы сеткі.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Are you sure you want to remove
%1 (%2)
from your contact list?</source>
        <translation>Вы ўпэўнены, што жадаеце выдаліць
%1 (%2)
з Вашага сьпісу кантактаў?</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Ok</source>
        <translation>Добра</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Cancel</source>
        <translation>Скасаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Do you really want to add
%1 (%2)
to your ignore list?</source>
        <translation>Вы сапраўды жадаеце дадаць
%1 (%2)
да сьпісу ігнараваных карыстальнікаў?</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Yes</source>
        <translation>&amp;Так</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;No</source>
        <translation>&amp;Не</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Are you sure you want to remove
%1 (%2)
from the &apos;%3&apos; group?</source>
        <translation>Вы ўпэўнены, што жадаеце выдаліць
%1 (%2)
з гурта &apos;%3&apos;?</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>(Error! No owner set)</source>
        <translation>(Памылка! Уладальнік ня вызначаны)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Licq version %1%8.
Qt GUI plugin version %2.
Compiled on: %7
%6
Maintainer: Jon Keating
Contributions by Dirk A. Mueller
Original Author: Graham Roff

http://www.licq.org
#licq on irc.freenode.net

%3 (%4)
%5 contacts.</source>
        <translation>Licq вэрсіі %1%8.
Інтэрфэйс Qt вэрсіі %2.
Сабраны: %7
%6
Мантэйнер: Джон Кіццінг (Jon Keating)
у супрацоўніцтве з Дыркам А. Муллэрам (Dirk A. Mueller)
Арыгінальны аўтар: Грэхам Роф (Graham Roff)

http://www.licq.org
Канал #licq на irc.freenode.net

%3 (%4)
%5 кантактаў.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>(with KDE support)
</source>
        <translation>(з падтрымкай KDE)
</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Unable to open extended icons file
%1.</source>
        <translation>Немагчыма адчыніць файл з дадатковымі выявамі
%1.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Custom Auto Response...</source>
        <translation>Асабісты аўта-адказ...</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Unable to open icons file
%1.</source>
        <translation>Немагчыма адчыніць файл выяваў
%1.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Send &amp;Message</source>
        <translation>Дас&amp;лаць паведамленьне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Send &amp;URL</source>
        <translation>Даслаць спас&amp;ылку</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Send &amp;Chat Request</source>
        <translation>Даслаць &amp;запрашэньне да размовы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Send &amp;File Transfer</source>
        <translation>Запытаць перадачу &amp;файла</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Send Contact &amp;List</source>
        <translation>Даслаць сьпіс &amp;кантактаў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Send &amp;Authorization</source>
        <translation>Даслаць дазвол &amp;аўтарызацыі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Send Authorization Re&amp;quest</source>
        <translation>Запытаць аў&amp;тарызацыю</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Send &amp;SMS</source>
        <translation>Дас&amp;лаць СМС</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Request &amp;Secure Channel</source>
        <translation>Запытаць &amp;бясьпечны канал</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Remove From List</source>
        <translation>Выдаліць з сьпісу</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>S&amp;earch for User</source>
        <translation>Зна&amp;йсці карыстальніка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>A&amp;uthorize User</source>
        <translation>Аўтарызаваць ка&amp;рыстальніка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Re&amp;quest Authorization</source>
        <translation>Запыт аў&amp;тарызацыі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;GPG Key Manager...</source>
        <translation>Наладзі&amp;ць GPG</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Don&apos;t Show</source>
        <translation>Не паказваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Available</source>
        <translation>Даступны</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Busy</source>
        <translation>Заняты</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Phone &quot;Follow Me&quot;</source>
        <translation>Тэлефон &quot;Кроч за мной&quot;</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Status Info</source>
        <translation>Дадзеныя стану</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Unknown Packets</source>
        <translation>Невядомыя пакеты</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Errors</source>
        <translation>Памылкі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Warnings</source>
        <translation>Папярэджваньні</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Packets</source>
        <translation>Пакеты</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Set All</source>
        <translation>Усталяваць усё</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Clear All</source>
        <translation>Зняць усё</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;View System Messages</source>
        <translation>&amp;Прагляд сістэмных паведамленьняў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Owner Manager</source>
        <translation>&amp;Наладкі ўладараў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Security/Password Options</source>
        <translation>Наладкі &amp;бясьпекі ды пароляў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Random Chat Group</source>
        <translation>&amp;Выпадковая размова</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Debug Level</source>
        <translation>Узровень адладкі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Add User</source>
        <translation>&amp;Дадаць карыстальніка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>R&amp;andom Chat</source>
        <translation>&amp;Выпадковая размова</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Popup All Messages</source>
        <translation>&amp;Паказаць усе паведамленьні</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Edit &amp;Groups</source>
        <translation>Рэдагаваць &amp;гурты</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Update All Users</source>
        <translation>Абнавіць усіх карыстальнікаў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Update Current Group</source>
        <translation>Абнавіць цякучы гурт</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Redraw User Window</source>
        <translation>П&amp;ерамаляваць акенца карыстальніка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Save All Users</source>
        <translation>&amp;Захаваць усіх карыстальнікаў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Hints</source>
        <translation>&amp;Парады</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;About</source>
        <translation>Зьв&amp;есткі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Statistics</source>
        <translation>&amp;Статыстыка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>System Functions</source>
        <translation>Сістэмныя наладкі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>User Functions</source>
        <translation>Карыстальніцкія наладкі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Status</source>
        <translation>&amp;Стан</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Group</source>
        <translation>&amp;Гурты</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Set &amp;Auto Response...</source>
        <translation>Усталяваць &amp;аўта-адказ...</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Network Window</source>
        <translation>Акенца с&amp;еткі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Mini Mode</source>
        <translation>Р&amp;эжым &quot;Міні&quot;</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Show Offline &amp;Users</source>
        <translation>&amp;Паказваць усіх карыстальнікаў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Thread Group View</source>
        <translation>Паказваць па г&amp;уртах</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Options...</source>
        <translation>На&amp;ладкі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>S&amp;kin Browser...</source>
        <translation>&amp;Зьнешні выгляд...</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Plugin Manager...</source>
        <translation>П&amp;ашырэньні...</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Sa&amp;ve Settings</source>
        <translation>За&amp;хаваць наладкі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Help</source>
        <translation>Дапа&amp;мога</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>E&amp;xit</source>
        <translation>В&amp;ыйсьце</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Accept in Away</source>
        <translation>Дазволіць калі адсутнічаю</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Accept in Not Available</source>
        <translation>Дазволіць калі недасягальны</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Accept in Occupied</source>
        <translation>Дазволіць калі заняты</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Accept in Do Not Disturb</source>
        <translation>Дазволіць пры &quot;не турбаваць&quot;</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Auto Accept Files</source>
        <translation>Аўтаматычна прымаць файлы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Auto Accept Chats</source>
        <translation>Аўтаматычна прымаць прапановы размовы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Auto Request Secure</source>
        <translation>Аўта запытвыць SSL</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Use GPG Encryption</source>
        <translation>Выкарыстоўваць GPG шыфраваньне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Use Real Ip (LAN)</source>
        <translation>Выкарыстоўваць рэальны IP</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Online to User</source>
        <translation>У сетцы дзеля карыстальніка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Away to User</source>
        <translation>Адсутнічаю дзеля карыстальніка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Not Available to User</source>
        <translation>Недасягальны дзеля карыстальніка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Occupied to User</source>
        <translation>Заняты дзеля карыстальніка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Do Not Disturb to User</source>
        <translation>&quot;Не турбаваць&quot; дзеля карыстальніка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;View Event</source>
        <translation>Праглядз&amp;ець здарэньне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Update Info Plugin List</source>
        <translation>Абнавіць пашырэньне зьвестак</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Update Status Plugin List</source>
        <translation>Абнавіць пашырэньне стану</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Update Phone &quot;Follow Me&quot; Status</source>
        <translation>Абнавіць стан &quot;Кроч за мной&quot;</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Update ICQphone Status</source>
        <translation>Абнавіць стан ICQ-тэлефона</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Update File Server Status</source>
        <translation>Абнавіць стан файлавага сховішча</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Send</source>
        <translation>Даслаць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Misc Modes</source>
        <translation>Дадатковыя рэжымы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>U&amp;tilities</source>
        <translation>Пр&amp;ылады</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Check Auto Response</source>
        <translation>Праверыць аўта-адказ</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Toggle &amp;Floaty</source>
        <translation>&amp;Уключыць плаваючае акенца</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Edit User Group</source>
        <translation>Рэдагаваць гурты</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Set GPG key</source>
        <translation>Усталяваць GPG ключ</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Check %1 Response</source>
        <translation>Праверыць %1-адказ</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Close &amp;Secure Channel</source>
        <translation>За&amp;чыніць бясьпечны канал</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Daemon Statistics
(Today/Total)
</source>
        <translation>Статыстыка дэмана
(Сёньня/Увогуле)
</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Up since %1
</source>
        <translation>У сетцы з %1
</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Last reset %1

</source>
        <translation>Апошняе абнуленьне %1

</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>%1: %2 / %3
</source>
        <translation>%1: %2 / %3
</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Reset</source>
        <translation>&amp;Абнуліць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Ok</source>
        <translation>&amp;Добра</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Do you really want to reset your stats?</source>
        <translation>Вы сапраўды жадаеце абнуліць статыстыку?</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Daemon Statistics

</source>
        <translation>Статыстыка дэмана

</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Up since %1

</source>
        <translation>У сетцы з %1

</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>%1: %2
</source>
        <translation>%1: %2</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&lt;hr&gt;&lt;p&gt; For more information, see the Licq webpage (&lt;tt&gt;http://www.licq.org&lt;/tt&gt;).&lt;/p&gt;</source>
        <translation>&lt;hr&gt;&lt;p&gt; Больш падрабязныя дадзеныя магчыма знайсьці на старонцы (&lt;tt&gt;http://www.licq.org&lt;/tt&gt;).&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Licq is unable to find a browser application due to an internal error.</source>
        <translation>Licq няздолеў знайсьці праглядач з-за ўнутранай памылкі.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Licq is unable to start your browser and open the URL.
You will need to start the browser and open the URL manually.</source>
        <translation>Licq няздолеў запусціць праглядач і адчыніць спасылку.
(Скапіруйце яе і адчыніце ў праглядачы самастойна).</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&lt;ul&gt;&lt;li&gt;&lt;tt&gt;%a - &lt;/tt&gt;user alias&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%e - &lt;/tt&gt;email&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%f - &lt;/tt&gt;first name&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%h - &lt;/tt&gt;phone number&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%i - &lt;/tt&gt;user ip&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%l - &lt;/tt&gt;last name&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%L - &lt;/tt&gt;local time&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%m - &lt;/tt&gt;# pending messages&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%M - &lt;/tt&gt;# pending messages (if any)&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%n - &lt;/tt&gt;full name&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%o - &lt;/tt&gt;last seen online&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%O - &lt;/tt&gt;online since&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%p - &lt;/tt&gt;user port&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%s - &lt;/tt&gt;full status&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%S - &lt;/tt&gt;abbreviated status&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%u - &lt;/tt&gt;uin&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%w - &lt;/tt&gt;webpage&lt;/li&gt;&lt;/ul&gt;</source>
        <translation>&lt;ul&gt;&lt;li&gt;&lt;tt&gt;%a - &lt;/tt&gt;мянушка&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%e - &lt;/tt&gt;пошта&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%f - &lt;/tt&gt;імя&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%h - &lt;/tt&gt;нумар тэлефону&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%i - &lt;/tt&gt;IP-адрас&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%l - &lt;/tt&gt;прозвішча&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%L - &lt;/tt&gt;мясцовы час&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%m - &lt;/tt&gt;колькасьць непрагледжаных паведамленьняў&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%M - &lt;/tt&gt;колькасьць нерагледжаных паведамленьняў (усіх)&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%n - &lt;/tt&gt;поўнае імя&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%o - &lt;/tt&gt;час апошняга з&apos;яўленьня&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%O - &lt;/tt&gt;час ў сетцы&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%p - &lt;/tt&gt;нумар парта&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%s - &lt;/tt&gt;поўны стан&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%S - &lt;/tt&gt;скарочаны стан&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%u - &lt;/tt&gt;нумар&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%w - &lt;/tt&gt;старонка ў сеціве&lt;/li&gt;&lt;/ul&gt;</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&lt;h2&gt;Hints for Using&lt;br&gt;the Licq Qt-GUI Plugin&lt;/h2&gt;&lt;br&gt;&lt;hr&gt;&lt;br&gt;&lt;ul&gt;&lt;li&gt;Change your status by right clicking on the status label.&lt;/li&gt;&lt;li&gt;Change your auto response by double-clicking on the status label.&lt;/li&gt;&lt;li&gt;View system messages by double clicking on the message label.&lt;/li&gt;&lt;li&gt;Change groups by right clicking on the message label.&lt;/li&gt;&lt;li&gt;Use the following shortcuts from the contact list:&lt;ul&gt;&lt;li&gt;&lt;tt&gt;Ctrl-M : &lt;/tt&gt;Toggle mini-mode&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-O : &lt;/tt&gt;Toggle show offline users&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-X : &lt;/tt&gt;Exit&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-H : &lt;/tt&gt;Hide&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-I : &lt;/tt&gt;View the next message&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-V : &lt;/tt&gt;View message&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-S : &lt;/tt&gt;Send message&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-U : &lt;/tt&gt;Send Url&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-C : &lt;/tt&gt;Send chat request&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-F : &lt;/tt&gt;Send File&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-A : &lt;/tt&gt;Check Auto response&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-P : &lt;/tt&gt;Popup all messages&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-L : &lt;/tt&gt;Redraw user window&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Delete : &lt;/tt&gt;Delete user from current group&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-Delete : &lt;/tt&gt;Delete user from contact list&lt;/li&gt;&lt;/ul&gt;&lt;li&gt;Hold control while clicking on close in the function window to remove   the user from your contact list.&lt;/li&gt;&lt;li&gt;Hit Ctrl-Enter from most text entry fields to select &quot;Ok&quot; or &quot;Accept&quot;.   For example in the send tab of the user function window.&lt;/li&gt;&lt;li&gt;Here is the complete list of user % options, which can be used in &lt;b&gt;OnEvent&lt;/b&gt;   parameters, &lt;b&gt;auto responses&lt;/b&gt;, and &lt;b&gt;utilities&lt;/b&gt;:</source>
        <translation>&lt;h2&gt;Парады для карыстаньня&lt;br&gt;пашырэньня Qt-GUI дзеля Licq&lt;/h2&gt;&lt;hr&gt;&lt;ul&gt;&lt;li&gt;Ваш стан зьмяняецца пасьля кліку правай кнопкай на назве стану.&lt;/li&gt;&lt;li&gt;Аўтаматычны адказ зьмяняецца падвойным клікам на назве стану.&lt;/li&gt;&lt;li&gt;Прагляд сістэмных паведамленьняў ажыцяўляецца падвойным клікам на назве паведамленьня.&lt;/li&gt;&lt;li&gt;Клік правай кнопкай па назве паведамленьня  зьмяняе гурт.&lt;/li&gt;&lt;li&gt;Выкарыстоўвайце наступныя скарачэньні ў сьпісе кантактаў:&lt;ul&gt;&lt;li&gt;&lt;tt&gt;Ctrl-M : &lt;/tt&gt;Пераключэньне Міні-рэжыму&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-O : &lt;/tt&gt;Паказваць/непаказваць неактыўных карыстальнікаў &lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-X : &lt;/tt&gt;Выхад&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-H : &lt;/tt&gt;Схаваць&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-I : &lt;/tt&gt;Прагляд наступнага паведамленьня&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-V : &lt;/tt&gt;Прагляд паведамленьня&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-S : &lt;/tt&gt;Даслаць паведамленьне&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-U : &lt;/tt&gt;Даслаць спасылку&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-C : &lt;/tt&gt;Даслаць запыт размовы&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-F : &lt;/tt&gt;Даслаць файл&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-A : &lt;/tt&gt;Праглядзець аўтаматычны адказ&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-P : &lt;/tt&gt;Адчыніць ўсе паведамленьні&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-L : &lt;/tt&gt;Абнавіць акенца сьпісу&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Delete : &lt;/tt&gt;Выдаліць карыстальніка з цякучага гурта&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-Delete : &lt;/tt&gt;Выдаліць карыстальніка з сьпісу кантактаў&lt;/li&gt;&lt;/ul&gt;&lt;li&gt;Утрымлівайце control калі націскаеце на кнопку &quot;зачыніць&quot; у функцыянальным акенцы каб выдаліць карыстальніка з сьпісу кантактаў.&lt;/li&gt;&lt;li&gt;Націск Ctrl-Enter у большасьці выпадкаў прыводзіць да вызначэньня  &quot;Добра&quot; альбо &quot;Ужыць&quot;, напрыклад, у акенцы паведамленьня гэта прыводзіць да адсыланьня паведамленьня.&lt;/li&gt;&lt;li&gt;Ніжэй знаходзіцца поўны сьпіс %-зьменных, якія могуць быць выкарыстаны у якасьці параметраў &lt;b&gt;падзей&lt;/b&gt;,&lt;b&gt;аўтаматычных адказаў&lt;/b&gt; ды &lt;b&gt;дапасаваньнях&lt;/b&gt;:</translation>
    </message>
</context>
<context>
    <name>CMessageViewWidget</name>
    <message>
        <location filename="" line="0"/>
        <source>%1 from %2</source>
        <translation>%1 ад %2</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>%1 to %2</source>
        <translation>%1 да %2</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Default</source>
        <translation>Па-змоўчаньню</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Compact</source>
        <translation>Кампактны</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Tiny</source>
        <translation>Маленькі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Table</source>
        <translation>Табліца</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Long</source>
        <translation>Доўгі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Wide</source>
        <translation>Шырокі</translation>
    </message>
</context>
<context>
    <name>CQtLogWindow</name>
    <message>
        <location filename="" line="0"/>
        <source>Licq Network Log</source>
        <translation>Licq Лог сеткі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Save</source>
        <translation>&amp;Захаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>C&amp;lear</source>
        <translation>Ач&amp;ысціць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Close</source>
        <translation>З&amp;ачыніць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Failed to open file:
%1</source>
        <translation>Немагчыма адчыніць файл:
%1</translation>
    </message>
</context>
<context>
    <name>CRandomChatDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>Random Chat Search</source>
        <translation>Пошук выпадковай размовы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Search</source>
        <translation>&amp;Пошук</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>General</source>
        <translation>Асноўныя</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Romance</source>
        <translation>Рамантычныя</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Games</source>
        <translation>Гульні</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Students</source>
        <translation>Студэнты</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>20 Something</source>
        <translation>Каля 20</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>30 Something</source>
        <translation>Каля 30</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>40 Something</source>
        <translation>Каля 40</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>50 Plus</source>
        <translation>Больш за 50</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Seeking Women</source>
        <translation>Шукаюць жанчыну</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Seeking Men</source>
        <translation>Шуаюць мужчыну</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Searching for Random Chat Partner...</source>
        <translation>Пошук выпадковага суразмоўцы...</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>No random chat user found in that group.</source>
        <translation>У гэтым гурце карыстальнікі ня знойдзены.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Random chat search timed out.</source>
        <translation>Час на пошук скончыўся.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Random chat search had an error.</source>
        <translation>Памылка пры пошуке суразмоўцы.</translation>
    </message>
</context>
<context>
    <name>CRefuseDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>Refusal message for %1 with </source>
        <translation>Паведамленьне аб адмове да %1 </translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Refuse</source>
        <translation>Адмовіць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Cancel</source>
        <translation>Скасаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Licq %1 Refusal</source>
        <translation>Licq Адмова  %1</translation>
    </message>
</context>
<context>
    <name>CSetRandomChatGroupDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>Set Random Chat Group</source>
        <translation>Усталяваць гурт выпадковай размовы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Set</source>
        <translation>&amp;Усталяваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Close</source>
        <translation>З&amp;ачыніць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>(none)</source>
        <translation>(адсутнічае)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>General</source>
        <translation>Асноўныя</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Romance</source>
        <translation>Рамантычныя</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Games</source>
        <translation>Гульні</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Students</source>
        <translation>Студэнты</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>20 Something</source>
        <translation>Каля 20</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>30 Something</source>
        <translation>Каля 30</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>40 Something</source>
        <translation>Каля 40</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>50 Plus</source>
        <translation>Больш за 50</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Seeking Women</source>
        <translation>Шукаюць жанчыну</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Seeking Men</source>
        <translation>Шуаюць мужчыну</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Setting Random Chat Group...</source>
        <translation>Усталяваньне гурта выпадковых суразмоўцаў...</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>failed</source>
        <translation>няўдала</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>timed out</source>
        <translation>скончыўся час</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>error</source>
        <translation>памылка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>done</source>
        <translation>завершана</translation>
    </message>
</context>
<context>
    <name>CTimeZoneField</name>
    <message>
        <location filename="" line="0"/>
        <source>Unknown</source>
        <translation>Не вядома</translation>
    </message>
</context>
<context>
    <name>CUserView</name>
    <message>
        <location filename="" line="0"/>
        <source>%1 Floaty (%2)</source>
        <translation>%1 Плаваючае акенца (%2)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Offline</source>
        <translation>Адключаны</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Online</source>
        <translation>У сетцы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Not In List</source>
        <translation>Не ў сьпісе</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>S</source>
        <translation>С</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source> weeks</source>
        <translation> тыдні(яў)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source> week</source>
        <translation> тыдзень</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source> days</source>
        <translation> дзён/дні</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source> day</source>
        <translation> дзень</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source> hours</source>
        <translation> гадзін(ы)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source> hour</source>
        <translation> гадзіну</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source> minutes</source>
        <translation> хвілін(ы)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source> minute</source>
        <translation> хвіліну</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>0 minutes</source>
        <translation>0 хвілін</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Birthday Today!</source>
        <translation>Сёньня Дзень народзінаў!</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Typing a message</source>
        <translation>Набірае паведамленьне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Phone &amp;quot;Follow Me&amp;quot;: Available</source>
        <translation>Тэлефон &amp;quot;Кроч за мной&amp;quot;: Даступны</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Phone &amp;quot;Follow Me&amp;quot;: Busy</source>
        <translation>Тэлефон &amp;quot;Кроч за мной&amp;quot;: Заняты</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>ICQphone: Available</source>
        <translation>ICQ-тэлефон: даступны</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>ICQphone: Busy</source>
        <translation>ICQ-тэлефон: заняты</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>File Server: Enabled</source>
        <translation>Файлавы паслужнік: Уключаны</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Secure connection</source>
        <translation>Бясьпечны канал</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Custom Auto Response</source>
        <translation>Асабісты аўтаматычны адказ</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Auto Response:</source>
        <translation>Аўтаматычны адказ:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>E: </source>
        <translation>Пошта: </translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>P: </source>
        <translation>Тэлефон: </translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>C: </source>
        <translation>Мабільны: </translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>F: </source>
        <translation>Факс: </translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Ip: </source>
        <translation>IP: </translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>O: </source>
        <translation>У сетцы: </translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Logged In: </source>
        <translation>Увайшоў: </translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Idle: </source>
        <translation>Бяздзейнічае: </translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Local time: </source>
        <translation>Мясцовы час: </translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>ID: </source>
        <translation>Нумар: </translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Awaiting authorization</source>
        <translation>Чакаем аўтарызацыю</translation>
    </message>
</context>
<context>
    <name>CUtilityDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>Licq Utility: %1</source>
        <translation>Licq Прылады: %1</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Command:</source>
        <translation>Каманда:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Window:</source>
        <translation>Акенца:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>GUI</source>
        <translation>Графічны інтэрфейс</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Terminal</source>
        <translation>Тэрмінал</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Internal</source>
        <translation>Унутраны</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Description:</source>
        <translation>Апісаньне:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Edit final command</source>
        <translation>Рэдагаваць фінальную каманду</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>User Fields</source>
        <translation>Карыстальніцкія палі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Run</source>
        <translation>В&amp;ыканаць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Edit:</source>
        <translation>Рэдагаваць:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Running:</source>
        <translation>Выкананьне:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Command Window</source>
        <translation>Акенца выкананьня</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Failed:</source>
        <translation>Няўдала:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Done</source>
        <translation>Скончыць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Done:</source>
        <translation>Скончана:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>C&amp;lose</source>
        <translation>&amp;Зачыніць</translation>
    </message>
</context>
<context>
    <name>ChatDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>Licq - Chat</source>
        <translation>Licq - Размова</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Remote - Not connected</source>
        <translation>Адлеглы бок не падключаны</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Audio</source>
        <translation>&amp;Аўдыё</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Save Chat</source>
        <translation>&amp;Захаваць размову</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Close Chat</source>
        <translation>За&amp;чыніць размову</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Pane Mode</source>
        <translation>&amp;Панэльны рэжым</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;IRC Mode</source>
        <translation>&amp;Рэжым IRC</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Chat</source>
        <translation>Размова</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Mode</source>
        <translation>Рэжым</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Ignore user settings</source>
        <translation>Ігнараваць наладкі карыстальніка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Ignores user color settings</source>
        <translation>Ігнараваць наладкі колеру карыстальніка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Beep</source>
        <translation>Гук</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Sends a Beep to all recipients</source>
        <translation>Дасылаць гук да ўсіх карыстальнікаў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Foreground color</source>
        <translation>Колер друкаваньня</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Changes the foreground color</source>
        <translation>Зьмена колеру друкаваньня</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Background color</source>
        <translation>Колер задняга пляну</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Changes the background color</source>
        <translation>Зьмена колеру задняга пляну</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Bold</source>
        <translation>Патоўшчанае</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Toggles Bold font</source>
        <translation>Патоўшчанае друкаваньне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Italic</source>
        <translation>Курсіў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Toggles Italic font</source>
        <translation>Курсіўнае друкаваньне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Underline</source>
        <translation>Падкрэсьленае</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Toggles Underline font</source>
        <translation>Падкрэсьленае друкаваньне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>StrikeOut</source>
        <translation>Перакрэсьленае</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Toggles StrikeOut font</source>
        <translation>Перакрэсьленае друкаваньне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Set Encoding</source>
        <translation>Усталяваць кадоўку</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Local - %1</source>
        <translation>Лакальная - %1</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Remote - Waiting for joiners...</source>
        <translation>Адлеглы бок - Чаканьне далучаючыхся...</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Remote - Connecting...</source>
        <translation>Адлеглы бок - Злучэньне...</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Unable to bind to a port.
See Network Window for details.</source>
        <translation>Немагчыма адчыніць порт.
Глядзі акенца сеткі дзеля атрыманьня падрабязнасьцей</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Unable to connect to the remote chat.
See Network Window for details.</source>
        <translation>Немагчыма далучыцца да адлеглай размовы.
Глядзі акенца сеткі дзеля атрыманьня падрабязнасьцей</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Unable to create new thread.
See Network Window for details.</source>
        <translation>Немагчыма адчыніць новы паток.
Глядзі акенца сеткі дзеля атрыманьня падрабязнасьцей</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>%1 closed connection.</source>
        <translation>%1 зачыніў злучэньне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>
&lt;--BEEP--&gt;
</source>
        <translation>
&lt;--Гук!--&gt;
</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&gt; &lt;--BEEP--&gt;
</source>
        <translation>&gt; &lt;--Гук!--&gt;
</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Do you want to save the chat session?</source>
        <translation>Жадаеце захаваць гэтую сэсію размовы?</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Yes</source>
        <translation>Так</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>No</source>
        <translation>Не</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Licq - Chat %1</source>
        <translation>Licq - Размова %1</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>/%1.chat</source>
        <translation>/%1.размова</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Failed to open file:
%1</source>
        <translation>Немагчыма адчыніць файл:
%1</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Unable to load encoding &lt;b&gt;%1&lt;/b&gt;. Message contents may appear garbled.</source>
        <translation>Немагчыма загрузіць кадоўку &lt;b&gt;%1&lt;/b&gt;. Магчыма паведамленьне будзе выглядаць пашкоджаным.</translation>
    </message>
</context>
<context>
    <name>CustomAwayMsgDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Hints</source>
        <translation>&amp;Парады</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Ok</source>
        <translation>&amp;Добра</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Clear</source>
        <translation>&amp;Выдаліць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Set Custom Auto Response for %1</source>
        <translation>Усталяваць асабісты аўтаадказ дзеля %1</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>I am currently %1.
You can leave me a message.</source>
        <translation>Я зараз маю стан: %1.
Калі жадаеце, пакіньце паведамленьне.</translation>
    </message>
</context>
<context>
    <name>EditCategoryDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>Unspecified</source>
        <translation>Не вызначаны</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;OK</source>
        <translation>&amp;Добра</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасаваць</translation>
    </message>
</context>
<context>
    <name>EditFileDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>Licq File Editor - %1</source>
        <translation>Licq Рэдактар файлаў - %1</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Save</source>
        <translation>&amp;Захаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Close</source>
        <translation>З&amp;ачыніць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Failed to open file:
%1</source>
        <translation>Немагчыма адчыніць файл:
%1</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>[ Read-Only ]</source>
        <translation>[ Толькі на чытаньне ]</translation>
    </message>
</context>
<context>
    <name>EditGrpDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>Licq - Edit Groups</source>
        <translation>Licq - Рэдагаваньне гуртоў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Groups</source>
        <translation>Гурты</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Add</source>
        <translation>Дадаць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Remove</source>
        <translation>Выдаліць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Shift Up</source>
        <translation>Вышэй</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Shift Down</source>
        <translation>Ніжэй</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Edit Name</source>
        <translation>Рэдагаваць назву</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Edit group name (hit enter to save).</source>
        <translation>Рэдагаваць назву гурта (націсьніце на Ўвод дзеля захаваньня)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Set Default</source>
        <translation>Усталяваць па-змоўчаньню</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>The default group to start up in.</source>
        <translation>Гурт па-змоўчаньню падчас старту Licq</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Set New Users</source>
        <translation>Новыя карыстальнікі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>The group to which new users will be automatically added.  All new users will be in the local system group New Users but for server side storage will also be stored in the specified group.</source>
        <translation>Гурт, у які аўтаматычна дадаюца новыя карыстальнікі.  Усе новыя карыстальнікі будуць дададзены да лакальнага гурта &quot;Новыя карыстальнікі&quot; але на старонцы сервера яны таксама павінны быць дададзены ў адпаведны гурт.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Default:</source>
        <translation>Па-змоўчаньню:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>New User:</source>
        <translation>Новыя карыстальнікі:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Save</source>
        <translation>&amp;Захаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Save the name of a group being modified.</source>
        <translation>Захаваць назвы зьмененых гуртоў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Done</source>
        <translation>&amp;Скончыць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>noname</source>
        <translation>без назвы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Are you sure you want to remove
the group &apos;%1&apos;?</source>
        <translation>Вы ўпэўнены, што жадаеце
выдаліць гурт &apos;%1&apos;?</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Ok</source>
        <translation>Добра</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Cancel</source>
        <translation>Скасаваць</translation>
    </message>
</context>
<context>
    <name>EditPhoneDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>Description:</source>
        <translation>Апісаньне:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Home Phone</source>
        <translation>Хатні тэлефон</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Work Phone</source>
        <translation>Працоўны тэлефон</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Private Cellular</source>
        <translation>Мабільны тэлефон</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Work Cellular</source>
        <translation>Працоўны мабільны тэлефон</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Home Fax</source>
        <translation>Хатні факс</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Work Fax</source>
        <translation>Працоўны факс</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Wireless Pager</source>
        <translation>Пэйджар</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Type:</source>
        <translation>Тып:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Phone</source>
        <translation>Тэлефон</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Cellular</source>
        <translation>Мабільны тэлефон</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Cellular SMS</source>
        <translation>SMS</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Fax</source>
        <translation>Факс</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Pager</source>
        <translation>Пэйджар</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Country:</source>
        <translation>Краіна:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Network #/Area code:</source>
        <translation>Код тэлефоннай сеткі/Код горада</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Number:</source>
        <translation>Нумар:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Extension:</source>
        <translation>Дадатковы:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Provider:</source>
        <translation>Аператар:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Custom</source>
        <translation>Іншы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>E-mail Gateway:</source>
        <translation>Паштовы шлюз:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Remove leading 0s from Area Code/Network #</source>
        <translation>Выдаліць пачатковыя нулі з кода сеткі/горада</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;OK</source>
        <translation>&amp;Добра</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>@</source>
        <translation>@</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Please enter a phone number</source>
        <translation>Калі ласка, увядзіце нумар тэлефону</translation>
    </message>
</context>
<context>
    <name>Event</name>
    <message>
        <location filename="" line="0"/>
        <source>Plugin Event</source>
        <translation>Падзея пашырэньня</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Message</source>
        <translation>Паведамленьне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Chat Request</source>
        <translation>Запрашэньне да размовы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>File Transfer</source>
        <translation>Перадача файла</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>URL</source>
        <translation>Спасылка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source></source>
        <translation></translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Authorization Request</source>
        <translation>Запыт аўтарызацыі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Authorization Refused</source>
        <translation>Адмоўлена ў аўтарызацыі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Authorization Granted</source>
        <translation>Аўтарызацыя дазволена</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>System Server Message</source>
        <translation>Сістэмнае паведамленьне паслужніка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Added to Contact List</source>
        <translation>Дададзены да сьпісу кантактаў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Web Panel</source>
        <translation>Вэб-панэль</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Email Pager</source>
        <translation>Паштовы пэйджар</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Contact List</source>
        <translation>Сьпіс кантактаў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>SMS</source>
        <translation>СМС</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Unknown Event</source>
        <translation>Невядомы выпадак</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>(cancelled)</source>
        <translation>(скасавана)</translation>
    </message>
</context>
<context>
    <name>GPGKeyManager</name>
    <message>
        <location filename="" line="0"/>
        <source>Licq GPG Key Manager</source>
        <translation>Licq Наладкі GPG-ключэй</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>GPG Passphrase</source>
        <translation>GPG пароль</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>No passphrase set</source>
        <translation>Пароль ня ўсталяваны</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Set GPG Passphrase</source>
        <translation>&amp;Усталяваць GPG пароль</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>User Keys</source>
        <translation>Ключ карыстальніка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Add</source>
        <translation>&amp;Дадаць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Edit</source>
        <translation>&amp;Рэдагаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Remove</source>
        <translation>&amp;Выдаліць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&lt;qt&gt;Drag&amp;Drop user to add to list.&lt;/qt&gt;</source>
        <translation>&lt;qt&gt;Перацягніце карыстальніка каб дадаць у сьпіс.&lt;/qt&gt;</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>User</source>
        <translation>Карыстальнік</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Active</source>
        <translation>Актыўны</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Key ID</source>
        <translation>Ключ</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Close</source>
        <translation>З&amp;ачыніць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Set Passphrase</source>
        <translation>Усталяваць пароль</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Not yet implemented. Use licq_gpg.conf.</source>
        <translation>Не падтрымліваецца. Карыстайцеся licq_gpg.conf.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Remove GPG key</source>
        <translation>Выдаліць GPG ключ</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Do you want to remove the GPG key? The key isn&apos;t deleted from your keyring.</source>
        <translation>Вы сапраўды жадаеце выдаліць GPG ключ? Ключ не будзе выдалены з сістэмы.</translation>
    </message>
</context>
<context>
    <name>GPGKeySelect</name>
    <message>
        <location filename="" line="0"/>
        <source>Select GPG Key for user %1</source>
        <translation>Абярыце GPG ключ адпаведна карыстальніка %1</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Select a GPG key for user %1.</source>
        <translation>Абярыце GPG ключ адпаведна карыстальніка %1.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Current key: No key selected</source>
        <translation>Цякучы ключ: не абраны</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Current key: %1</source>
        <translation>Цякучы ключ: %1</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Use GPG Encryption</source>
        <translation>Выкарыстоўваць GPG шыфраваньне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Filter:</source>
        <translation>Фільтр:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;OK</source>
        <translation>&amp;Добра</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;No Key</source>
        <translation>&amp;Без ключа</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасаваць</translation>
    </message>
</context>
<context>
    <name>Groups</name>
    <message>
        <location filename="" line="0"/>
        <source>All Users</source>
        <translation>Усе карыстальнікі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Online Notify</source>
        <translation>Паведаміць аб з&apos;яўленьні</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Visible List</source>
        <translation>У сьпісе бачных</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Invisible List</source>
        <translation>У сьпісе нябачных</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Ignore List</source>
        <translation>У сьпісе ігнараваных</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>New Users</source>
        <translation>Новыя карыстальнікі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Unknown</source>
        <translation>Не вядома</translation>
    </message>
</context>
<context>
    <name>HintsDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>Licq - Hints</source>
        <translation>Licq - Парады</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Close</source>
        <translation>З&amp;ачыніць</translation>
    </message>
</context>
<context>
    <name>IconManager_KDEStyle</name>
    <message>
        <location filename="" line="0"/>
        <source>&lt;br&gt;&lt;b&gt;%1 system messages&lt;/b&gt;</source>
        <translation>&lt;br&gt;&lt;b&gt;%1 сістэмных паведамленьняў&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&lt;br&gt;%1 msgs</source>
        <translation>&lt;br&gt;%1 паведамленьняў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&lt;br&gt;1 msg</source>
        <translation>&lt;br&gt;1 паведамленьне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&lt;br&gt;Left click - Show main window&lt;br&gt;Middle click - Show next message&lt;br&gt;Right click - System menu</source>
        <translation>&lt;br&gt;Клік левай кнопкай - паказаць галоўнае акенца&lt;br&gt;клік сярэдняй кнопкай - паказаць наступнае паведамленьне&lt;br&gt;клік правай кнопкай - сістэмнае мяню</translation>
    </message>
</context>
<context>
    <name>IconManager_Themed</name>
    <message>
        <location filename="" line="0"/>
        <source>Unable to load dock theme file
(%1)
:%2</source>
        <translation>Немагчыма загрузіць тэму дока з файлу 
(%1)
:%2</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Unable to load dock theme image
%1</source>
        <translation>Немагчыма загрузіць малюнак дока з файлу 
(%1)</translation>
    </message>
</context>
<context>
    <name>KeyListItem</name>
    <message>
        <location filename="" line="0"/>
        <source>Yes</source>
        <translation>Так</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>No</source>
        <translation>Не</translation>
    </message>
</context>
<context>
    <name>KeyRequestDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>Licq - Secure Channel with %1</source>
        <translation>Licq - бясьпечны канал з %1</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Secure channel is established using SSL
with Diffie-Hellman key exchange and
the TLS version 1 protocol.

</source>
        <translation>Бясьпечны канал усталяваны з выкарыстаньнем SSL
і алгарытма абмена ключамі Діффі-Хелмана (Diffie-Hellman)
ды пратаколам TLS версіі 1.

</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>The remote uses Licq %1/SSL.</source>
        <translation>На супрацьлеглым баку выкарыстоўваецца Licq %1/SSL.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>The remote uses Licq %1, however it
has no secure channel support compiled in.
This probably won&apos;t work.</source>
        <translation>На супрацьлеглым баку выкарыстоўваецца Licq %1.
Гэты кліент не мае ўбадаванай падтрымкі бясьпечнага каналу.
Магчыма бясьпечны канал не ўсталюецца.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>This only works with other Licq clients &gt;= v0.85
The remote doesn&apos;t seem to use such a client.
This might not work.</source>
        <translation>Гэтая опцыя працуе толькі з іншымі Licq-кліентамі &gt;= v0.85
Адлеглы пэйджэр не падобны на гэткі кліент.
Магчыма бясьпечны канал не ўсталюецца.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Send</source>
        <translation>Да&amp;слаць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Close</source>
        <translation>З&amp;ачыніць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Ready to close channel</source>
        <translation>Гатовы зачыніць каналу</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Ready to request channel</source>
        <translation>Гатовы запытаць канал</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Client does not support OpenSSL.
Rebuild Licq with OpenSSL support.</source>
        <translation>Кліент не падтрымлівае OpenSSL.
Зьбярыце Licq з падтрымкай OpenSSL.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Requesting secure channel...</source>
        <translation>Запытваецца бясьпечны канал...</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Closing secure channel...</source>
        <translation>Зачыняецца бясьпечны канал...</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&lt;font color=&quot;yellow&quot;&gt;Secure channel already established.&lt;/font&gt;
</source>
        <translation>&lt;font color=&quot;yellow&quot;&gt;Бясьпечны канал ужо ўсталяваны.&lt;/font&gt;
</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&lt;font color=&quot;yellow&quot;&gt;Secure channel not established.&lt;/font&gt;
</source>
        <translation>&lt;font color=&quot;yellow&quot;&gt;Бясьпечны канал не ўсталяваны.&lt;/font&gt;
</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&lt;font color=&quot;red&quot;&gt;Remote client does not support OpenSSL.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;Адлеглы кліент не падтрымлівае OpenSSL.&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&lt;font color=&quot;red&quot;&gt;Could not connect to remote client.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;Не магчыма ўсталяваць сувязь з адлеглым кліентам.&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&lt;font color=&quot;ForestGreen&quot;&gt;Secure channel established.&lt;/font&gt;
</source>
        <translation>&lt;font color=&quot;ForestGreen&quot;&gt;Бясьпечны канал усталяваны.&lt;/font&gt;
</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&lt;font color=&quot;blue&quot;&gt;Secure channel closed.&lt;/font&gt;
</source>
        <translation>&lt;font color=&quot;blue&quot;&gt;Бясьпечны канал зачынены.&lt;/font&gt;
</translation>
    </message>
</context>
<context>
    <name>KeyView</name>
    <message>
        <location filename="" line="0"/>
        <source>Name</source>
        <translation>Імя</translation>
    </message>
</context>
<context>
    <name>MLEditWrap</name>
    <message>
        <location filename="" line="0"/>
        <source>Allow Tabulations</source>
        <translation>Дазволіць укладкі</translation>
    </message>
</context>
<context>
    <name>MLView</name>
    <message>
        <location filename="" line="0"/>
        <source>Copy URL</source>
        <translation>Капіяваць спасылку</translation>
    </message>
</context>
<context>
    <name>MsgView</name>
    <message>
        <location filename="" line="0"/>
        <source>D</source>
        <translation>П</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Event Type</source>
        <translation>Тып падзеі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Options</source>
        <translation>Наладкі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Time</source>
        <translation>Час</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Direct</source>
        <translation>Прамое</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Server</source>
        <translation>Паслужнік</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Urgent</source>
        <translation>Хутка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Multiple Recipients</source>
        <translation>Некалькім карыстальнікам</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Cancelled Event</source>
        <translation>Адмененая падзея</translation>
    </message>
</context>
<context>
    <name>OptionsDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>Licq Options</source>
        <translation>Licq Наладкі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;OK</source>
        <translation>&amp;Добра</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Apply</source>
        <translation>&amp;Ужыць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>General</source>
        <translation>Асноўныя</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Contact List</source>
        <translation>Сьпіс кантактаў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>OnEvent</source>
        <translation>Дзеяньні</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Network</source>
        <translation>Сетка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Status</source>
        <translation>Стан</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Miscellaneous</source>
        <translation>Іншыя</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Message Display</source>
        <translation>Выгляд паведамленьняў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>none</source>
        <translation>адсутнічае</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>default</source>
        <translation>па-змоўчаньню</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>default (%1)</source>
        <translation>па-змоўчаньню (%1)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Main Window</source>
        <translation>Галоўнае акенца</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Show group name if no messages</source>
        <translation>Паказваць назву гурта пры адсутнасьці паведамленьняў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Show the name of the current group in the messages label when there are no new messages</source>
        <translation>Паказваць назву цякучага гурта пры адсутнасьці новых паведамленьняў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Auto Close Function Window</source>
        <translation>Аўтаматычна зачыняць акенцы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Auto close the user function window after a succesful event</source>
        <translation>Аўтаматычна зачыняць карыстальніцкія акенцы пасля пасьпяховага завяршэньня дзеяньня</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Auto-Popup Incoming Msg</source>
        <translation>Аўтаматычна адчыняць паведамленьні</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Open all incoming messages automatically when received if we are online (or free for chat)</source>
        <translation>Аўтаматычна адчыняць новыя паведамленьні ў акенцах, калі Вы маеце стан &quot;у сетцы&quot; альбо &quot;згодны размаўляць&quot;</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Auto-Raise on Incoming Msg</source>
        <translation>Аўтаматычнае ўсплыцьцё акенцаў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Raise the main window on incoming messages</source>
        <translation>Паказваць акенца з новым паведамленьнем паверх іншых</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Start Hidden</source>
        <translation>Стартаваць схаваным</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Start main window hidden. Only the dock icon will be visible.</source>
        <translation>Схаваць галоўнае акенца пры старце. Будзе бачна толькі докавая выява.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Bold Message Label on Incoming Msg</source>
        <translation>Патоўшчыць друкаваньне пры новым паведамленьні</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Show the message info label in bold font if there are incoming messages</source>
        <translation>Патоўшчыць друкаванье дзеля адпаведнага карыстальніка калі атрымана новае паведамленьне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Manual &quot;New User&quot; group handling</source>
        <translation>Не апрацоўваць аўтаматычна новых карыстальнікаў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>If not checked, a user will be automatically removed from &quot;New User&quot; group when you first send an event to them</source>
        <translation>Калі гэтая наладка не ўсталявана, новы карыстальнік будзе аўтаматычна перамешчаны ў гурт па-змоўчаньню пры адсылке першага паведамленьня да яго</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Check Clipboard For Urls/Files</source>
        <translation>Назіраць за буферам абмену</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>When double-clicking on a user to send a message check for urls/files in the clipboard</source>
        <translation>Гэтая наладка аўтаматычна абірае тып паведамленьня згодна са зьместам буфера абмену пры наяўнасці ў ім спасылкі альбо назвы файлу</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Auto Position the Reply Window</source>
        <translation>Аўтаматычна абіраць месца дзеля акенца</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Position a new reply window just underneath the message view window</source>
        <translation>Аўтаматычна абіраць месца ў незанятай частцы экрану дзеля новага акенца</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Flash Taskbar on Incoming Msg</source>
        <translation>Мільгаць на панэлі заданьняў пры новым паведамленьні</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Flash the Taskbar on incoming messages</source>
        <translation>Мільгаць выявай на панэлі заданьняў пры з&apos;яўленьні новага паведамленьня</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Auto send through server</source>
        <translation>Аўтаматычна дасылаць праз паслужнік</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Automatically send messages through the server if direct connection fails</source>
        <translation>Аўтаматычна дасылаць паведамленьне праз паслужнік, калі немагчыма ўсталяваць прамое злучэньне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Allow dragging main window</source>
        <translation>Дазволіць перамяшчэньні дзеля асноўнага акенца</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Lets you drag around the main window with your mouse</source>
        <translation>Дазваляе перамяшчаць аб&apos;екты ў галоўным акенцы (drag&apos;n&apos;drop)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Chatmode Messageview</source>
        <translation>Прагляд паведамленьняў у выглядзе размовы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Show the current chat history in Send Window</source>
        <translation>Паказваць гісторыю цякучай размовы ў тым жа самым акенцы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Tabbed Chatting</source>
        <translation>Выкарыстоўваць укладкі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Use tabs in Send Window</source>
        <translation>Размяшчаць у асобных укладках адзінага акенца ўсе тэкучыя дыялёгі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Show recent messages</source>
        <translation>Паказваць апошнія паведамленьні</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Show the last 5 messages when a Send Window is opened</source>
        <translation>Паказваць 5 апошніх паведамленьняў у акенцы паведамленьняў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Send typing notifications</source>
        <translation>Адсылаць інфармацыю аб наборы паведамленьня</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Send a notification to the user so they can see when you are typing a message to them</source>
        <translation>Паказваць адлегламу карыстальніку, што Вы рыхтуеце паведамленьне да яго (typing noification)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Sticky Main Window</source>
        <translation>Замацаваць галоўнае акенца</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Makes the Main window visible on all desktops</source>
        <translation>Зрабіць галоўнае акенца бачным на ўсіх працоўных месцах</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Sticky Message Window</source>
        <translation>Замацаваць акенца паведамленьняў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Makes the Message window visible on all desktops</source>
        <translation>Зрабіць акенца паведамленьняў бачным на ўсіх працоўных месцах</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Localization</source>
        <translation>Моўныя наладкі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Default Encoding:</source>
        <translation>Кадоўка па-змоўчаньню:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Sets which default encoding should be used for newly added contacts.</source>
        <translation>Наладка ўсталёўвае кадоўку па-змоўчаньню дзеля ўсіх новых кантактаў.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>System default (%1)</source>
        <translation>Сістэмная па-змоўчаньню (%1)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Show all encodings</source>
        <translation>Паказваць усе кадоўкі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Show all available encodings in the User Encoding selection menu. Normally, this menu shows only commonly used encodings.</source>
        <translation>Паказваць усе магчымыя кадоўкі ў адпаведным карыстальніцкім мяню. Бяз гэтай наладкі будуць паказаны толькі найбольш ужываемыя.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Docking</source>
        <translation>Выкарыстаньне дока</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Use Dock Icon</source>
        <translation>Выкарыстоўваць докавую выяву</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Controls whether or not the dockable icon should be displayed.</source>
        <translation>Вызначае, ці патрэбна паказваць акенца-док з выявай.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Default Icon</source>
        <translation>Выява па-змоўчаньню</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>64 x 48 Dock Icon</source>
        <translation>Выява памерам 64 x 48</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Selects between the standard 64x64 icon used in the WindowMaker/Afterstep wharf and a shorter 64x48 icon for use in the Gnome/KDE panel.</source>
        <translation>Выбар паміж стандартнай выявай памерам 64x64, ужываемай у Віндаўмэкеры альбо Афтэстэпе, і зменьшанай выявай памерам 64x48 дзеля панэлі Гнома альбо КДЕ.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Themed Icon</source>
        <translation>Выява з тэмы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Small Icon</source>
        <translation>Маленькая выява</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Uses the freedesktop.org standard to dock a small icon into the window manager.  Works with many different window managers.</source>
        <translation>Выкарыстоўваецца выява адпаведная стандарту freedesktop.org на докавыя акенцы графічных мэнэджэраў.  Раіцца выкарыстоўваць з большасьцьцю акенцавых мэнэджэраў.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Theme:</source>
        <translation>Тэма:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Font</source>
        <translation>Друкаваньне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>The fonts used</source>
        <translation>Выкарыстоўваемае друкаваньне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Font:</source>
        <translation>Друкаваньне:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Select Font</source>
        <translation>Абраць друкаваньне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Select a font from the system list</source>
        <translation>Абраць друкаваньне з сістэмнага сьпісу</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Edit Font:</source>
        <translation>Друкаваньне рэдактара:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Font used in message editor etc.</source>
        <translation>Друкаваньне, выкарыстоўваемае ў рэдактары паведамленьняў.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>OnEvents Enabled</source>
        <translation>Дазволіць каманды на здарэньні</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Enable running of &quot;Command&quot; when the relevant event occurs.</source>
        <translation>Дазволіць зьдяйсьненьне каманд, калі мае месца здарэньне.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Command:</source>
        <translation>Каманда:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&lt;p&gt;Command to execute when an event is received.&lt;br&gt;It will be passed the relevant parameters from below.&lt;br&gt;Parameters can contain the following expressions &lt;br&gt; which will be replaced with the relevant information:&lt;/p&gt;</source>
        <translation>&lt;p&gt;Каманда, якая будзе зьдейсьнена пры здарэньні.&lt;br&gt;Каманда мае быць выканана з параметрамі, якія прыведзены ў палях ніжэй.&lt;br&gt;Таксама магчыма ўжываць наступныя %-зьменныя, якія будуць заменены адпаведнымі дадзенымі:&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Parameters</source>
        <translation>Параметры</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Message:</source>
        <translation>Паведамленьне:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Parameter for received messages</source>
        <translation>Параметр, ужываемы пры атрыманьні новага паведамленьня</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>URL:</source>
        <translation>Спасылка:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Parameter for received URLs</source>
        <translation>Параметр, ужываемы пры атрыманьні спасылкі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Chat Request:</source>
        <translation>Запыт размовы:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Parameter for received chat requests</source>
        <translation>Параметр, ужываемы пры атрыманьні запрашэньня да размовы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>File Transfer:</source>
        <translation>Перадача файла:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Parameter for received file transfers</source>
        <translation>Параметр, ужываемы пры атрыманьні запыта на атрыманьне файла</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Online Notify:</source>
        <translation>З&apos;яўленьне ў сетцы:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Parameter for online notification</source>
        <translation>Параметр, ужываемы пры з&apos;яўленьні карыстальніка ў сетцы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>System Msg:</source>
        <translation>Сістэмнае павед.:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Parameter for received system messages</source>
        <translation>Параметр, ужываемы пры атрыманьні сістэмнага паведамленьня</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Message Sent:</source>
        <translation>Адасланае павед.:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Parameter for sent messages</source>
        <translation>Параметр, ужываемы пры адсылке паведамленьня</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Accept Modes</source>
        <translation>Дазволіць выкананьне у рэжымах</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>OnEvent in Away</source>
        <translation>Адсутнічаю</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Perform OnEvent command in away mode</source>
        <translation>Дазволіць выкананьне дзеяньня пры знаходжаньні ў рэжыме &quot;Адсутничаю&quot;</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>OnEvent in N/A</source>
        <translation>Нядасягальны</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Perform OnEvent command in not available mode</source>
        <translation>Дазволіць выкананьне дзеяньня пры знаходжаньні ў рэжыме &quot;Нядасягальны&quot;</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>OnEvent in Occupied</source>
        <translation>Заняты</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Perform OnEvent command in occupied mode</source>
        <translation>Дазволіць выкананьне дзеяньня пры знаходжаньні ў рэжыме &quot;Заняты&quot;</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>OnEvent in DND</source>
        <translation>Не турбаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Perform OnEvent command in do not disturb mode</source>
        <translation>Дазволіць выкананьне дзеяньня пры знаходжаньні ў рэжыме &quot;Не турбаваць&quot;</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Online Notify when Logging On</source>
        <translation>Папярэджваць аб з&apos;яўленьні ў сетцы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Perform the online notify OnEvent when logging on (this is different from how the Mirabilis client works)</source>
        <translation>Дазволіць папярэджваньне пры з&apos;яўленьні карыстальніка ў сетцы. (адрозніваецца ад працы ў Mirabilis-кліентах)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Server settings</source>
        <translation>Наладкі паслужніка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>ICQ Server:</source>
        <translation>паслужнік ICQ:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>ICQ Server Port:</source>
        <translation>Порт на паслужніку:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Firewall</source>
        <translation>Міжсеткавы экран</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>I am behind a firewall</source>
        <translation>Я абаронены міжсеткавым экранам</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Port Range:</source>
        <translation>Дыяпазон партоў:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>TCP port range for incoming connections.</source>
        <translation>Дыяпазон TCP-партоў дзеля ўваходзячых злучэньняў.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Auto</source>
        <translation>Аўтаматычна</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>I can receive direct connections</source>
        <translation>Дазволіць прамыя злучэньні </translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source><byte value="x9"/>to</source>
        <translation><byte value="x9"/>да</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Proxy</source>
        <translation>Проксі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Use proxy server</source>
        <translation>Выкарыстоўваць проксі-паслужнік</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Proxy Type:</source>
        <translation>Тып проксі:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>HTTPS</source>
        <translation>HTTPS</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Proxy Server:</source>
        <translation>Проксі-паслужнік:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Proxy Server Port:</source>
        <translation>Порт на проксі:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Use authorization</source>
        <translation>Выкарыстоўваць аўтарызацыю</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Username:</source>
        <translation>Карыстальнік:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Password:</source>
        <translation>Пароль:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Connection</source>
        <translation>Злучэньне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Reconnect after Uin clash</source>
        <translation>Злучацца аўтаматычна</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Licq can reconnect you when you got disconnected because your Uin was used from another location. Check this if you want Licq to reconnect automatically.</source>
        <translation>Licq мае магчымасць аўтаматычна ўсталёўвацьзлучэньне пасля таго, як яно было згублена (Напрыклад, калі Ваш нумар выкарыстоўваецца на іншым камп&apos;ютары альбо ўзьніклі непаладкі з сецтвам).</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Default Auto Response Messages</source>
        <translation>Аўтаматычнае паведамленьне стану па-змоўчаньню</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Status:</source>
        <translation>Стан:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Away</source>
        <translation>Адсутнічаю</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Not Available</source>
        <translation>Нядасягальны</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Occupied</source>
        <translation>Заняты</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Do Not Disturb</source>
        <translation>Не турбаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Free For Chat</source>
        <translation>Згодзен размаўляць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Preset slot:</source>
        <translation>Нарыхтоўка:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Text:</source>
        <translation>Тэкст:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Save</source>
        <translation>Захаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Startup</source>
        <translation>Пры запуске</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Auto Logon:</source>
        <translation>Стан пры старце:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Automatically log on when first starting up.</source>
        <translation>Аўтаматычна ўсталёўваць стан пры старце.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Offline</source>
        <translation>Адключаны</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Online</source>
        <translation>У сетцы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Free for Chat</source>
        <translation>Згодны размаўляць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Invisible</source>
        <translation>Нябачны</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Auto Away:</source>
        <translation>Зьмяніць на &quot;Адсутнічаю&quot; праз:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Number of minutes of inactivity after which to automatically be marked &quot;away&quot;.  Set to &quot;0&quot; to disable.</source>
        <translation>Колькасьць хвілін бяздзеяньня да ўсталяваньня стану &quot;Адсутнічаю&quot;.  Усталюйце ў &quot;0&quot; каб скасаваць наладку.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Never</source>
        <translation>Ніколі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Auto N/A:</source>
        <translation>Зьмяніць на &quot;Нядасягальны&quot; праз:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Number of minutes of inactivity after which to automatically be marked &quot;not available&quot;.  Set to &quot;0&quot; to disable.</source>
        <translation>Колькасьць хвілін бяздзеяньня да ўсталяваньня стану &quot;Нядасягальны&quot;.  Усталюйце ў &quot;0&quot; каб скасаваць наладку.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Auto Offline:</source>
        <translation>Зьмяніць на &quot;Выключаны&quot; праз:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Number of minutes of inactivity after which to automatically go offline.  Set to &quot;0&quot; to disable.</source>
        <translation>Колькасьць хвілін бяззеяньня да выхада з сетцы.  Усталюйце ў &quot;0&quot; каб скасаваць наладку.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Column Configuration</source>
        <translation>Наладкі слупкоў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Title</source>
        <translation>Заглаў&apos;е</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>The string which will appear in the list box column header</source>
        <translation>Назва, якая будзе бачна ў сьпісе кантактаў як заглаў&apos;е</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Format</source>
        <translation>Фармат</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Width</source>
        <translation>Шырыня</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>The width of the column</source>
        <translation>Шырыня слупка ў пікселах</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Alignment</source>
        <translation>Месцазнаходжаньне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>The alignment of the column</source>
        <translation>Месцазнаходжаньне тэксту ў слупке</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Column %1</source>
        <translation>Слупок %1</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Left</source>
        <translation>З лева</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Right</source>
        <translation>З права</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Center</source>
        <translation>Па цэнтры</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Options</source>
        <translation>Наладкі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Use server side contact list</source>
        <translation>Выкарыстоўваць сьпіс кантактаў на паслужніку</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Store your contacts on the server so they are accessible from different locations and/or programs</source>
        <translation>Захаваньне сьпіса кантактаў на паслужніку дае магчымасць размаўляць з Вашымі суразмоўцамі з любога месца і з любога кліента</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Show Grid Lines</source>
        <translation>Паказваць межы паміж радкамі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Draw boxes around each square in the user list</source>
        <translation>Маляваць межы навокал усіх элементаў у сьпісе кантактаў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Show Column Headers</source>
        <translation>Паказваць загалоў&apos;і слупкоў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Turns on or off the display of headers above each column in the user list</source>
        <translation>Паказваць альбо не паказваць загалоў&apos;е ў сьпісе кантактаў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Show User Dividers</source>
        <translation>Паказваць раздзеляльнікі карыстальнікаў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Show the &quot;--online--&quot; and &quot;--offline--&quot; bars in the contact list</source>
        <translation>Паказваць раздзеляльнікі &quot;--у сетцы--&quot; і &quot;--выключаны--&quot; ў сьпісе кантактаў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Always show online notify users</source>
        <translation>Заўсёды паказваць карыстальнікаў
дзеля якіх уладкавана &quot;Паказваць калі ў сетцы&quot;</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Show online notify users who are offline even when offline users are hidden.</source>
        <translation>Паказваць  карыстальнікаў дзеля якіх усталявана наладка &quot;Паказваць калі ў сетцы&quot; у той час, калі астатнія неактыўныя карыстальнікі будуць схаваны.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Transparent when possible</source>
        <translation>Празрыстасць, калі магчыма</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Make the user window transparent when there is no scroll bar</source>
        <translation>Па магчымасьці зрабіць акенца карыстальнікаў празрыстым калі адсутнічае паласа пракруткі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Use Font Styles</source>
        <translation>Выкарыстоўваць стылі друкаваньня</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Use italics and bold in the user list to indicate special characteristics such as online notify and visible list</source>
        <translation>Выкарыстоўваць курсіўнае і патоўшчанае друкаваньне каб выдзеляць спецыяльныя характэрыстыкі, такія як нябачнасць і іншыя</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Blink All Events</source>
        <translation>Мільгаць пры паведамленьнях</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>All incoming events will blink</source>
        <translation>Усе ўваходзячыя паведамленьні павінны мільгаць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Blink Urgent Events</source>
        <translation>Мільгаць пры хуткіх паведамленьнях</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Only urgent events will blink</source>
        <translation>Мільгаць толькі пры хуткіх паведамленьнях</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Allow scroll bar</source>
        <translation>Дазволіць стужку пракруткі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Allow the vertical scroll bar in the user list</source>
        <translation>Дазволіць вертыкальную стужку пракруткі ў сьпісе карыстальнікаў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Show Extended Icons</source>
        <translation>Паказваць дадатковыя выявы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Show birthday, invisible, and custom auto response icons to the right of users in the list</source>
        <translation>Паказваць выявы дня народзінаў, нябачнасьці, асабістага аўта-адказу, тэлефонаў і інш. з права ад карыстальніка ў спісе кантактаў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Frame Style: </source>
        <translation>Стыль акенца: </translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Override the skin setting for the frame style of the user window:
   0 (No frame), 1 (Box), 2 (Panel), 3 (WinPanel)
 + 16 (Plain), 32 (Raised), 48 (Sunken)
 + 240 (Shadow)</source>
        <translation>Змяніць наладкі выгляду дзеля фрэйма акенца карыстальнікаў:
   0 (бяз рамкі), 1 (каробка), 2 (панэль), 3 (панэль як у Віндаўс)
 + 16 (звычайная), 32 (рэльефная), 48 (утопленая)
 + 240 (з ценьню)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Use System Background Color</source>
        <translation>Выкарыстоўваць сістэмны колер задняга пляну</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Hot key: </source>
        <translation>Гарачая кнопка: </translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Hotkey pops up the next pending message
Enter the hotkey literally, like &quot;shift+f10&quot;, &quot;none&quot; for disabling
changes here require a Restart to take effect!
</source>
        <translation>Гарачая кнопка вызывае наступнае чакаемае вакенца
Увядзіце камбінацыю літэральна, напрыклад, &quot;shift+f10&quot; альбо &quot;none&quot; - каб выключыць
Гэтыя зьмены патрабуюць рэстарту!
</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Additional &amp;sorting:</source>
        <translation>Дадтковая &amp;сартыроўка:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>status</source>
        <translation>стан</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>status + last event</source>
        <translation>стан + апошняя падзея</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>status + new messages</source>
        <translation>стан + новае паведамленьне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&lt;b&gt;none:&lt;/b&gt; - Don&apos;t sort online users by Status&lt;br&gt;
&lt;b&gt;status&lt;/b&gt; - Sort online users by status&lt;br&gt;
&lt;b&gt;status + last event&lt;/b&gt; - Sort online users by status and by last event&lt;br&gt;
&lt;b&gt;status + new messages&lt;/b&gt; - Sort online users by status and number of new messages</source>
        <translation>&lt;b&gt;адсутнічае&lt;/b&gt; -не рабіць дадатковую сартыроўку карыстальнікаў у сетцы&lt;br&gt;
&lt;b&gt;стан&lt;/b&gt; - сартыраваць карытсальнікаў толькі па стану
&lt;b&gt;стан + апошняя падзея&lt;/b&gt; - сартыраваць карыстальнікаў па стану і наяўнасьці падзей&lt;br&gt;
&lt;b&gt;стан + новае паведамленьне&lt;/b&gt; -  сартыраваць карыстальнікаў па стану і колькасьці новых паведамленьняў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Show user display picture</source>
        <translation>Паказваць малюнак карыстальніка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Show the user&apos;s display picture instead of a status icon, if the user is online and has a display picture</source>
        <translation>Паказваць малюнак(выяву) карыстальніка замест статуснай выявы, пры ўмове, што карыстальнік у сетцы і мае гэткі малюнак</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Popup info</source>
        <translation>Усплываючыя дадзеныя</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Email</source>
        <translation>Эл. пошта</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Phone</source>
        <translation>Тэлефон</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Fax</source>
        <translation>Факс</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Cellular</source>
        <translation>Мабільны тэлефон</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>IP</source>
        <translation>IP-адрас</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Last online</source>
        <translation>Апошні раз ў сетцы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Online Time</source>
        <translation>Час у сетцы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Idle Time</source>
        <translation>Час бяздзеяньня</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Protocol ID</source>
        <translation>Ідэнтыфікатар
карыстальніка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Extensions</source>
        <translation>Пашырэньні</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Url Viewer:</source>
        <translation>Праглядач спасылак:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>The command to run to view a URL.  Will be passed the URL as a parameter.</source>
        <translation>Каманда, якая будзе выканана дзеля прагляду спасылкі.  Камандзе будзе перадана спасылка ў якасьці аргументу.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Terminal:</source>
        <translation>Тэрмінал:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>The command to run to start your terminal program.</source>
        <translation>Каманда старту тэрмінальнай праграмы.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Paranoia</source>
        <translation>Параноя</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Ignore New Users</source>
        <translation>Ігнараваць новых карыстальнікаў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Determines if new users are automatically added to your list or must first request authorization.</source>
        <translation>Вызначае, трэба лі абавязковая аўтарызацыя, каб карыстальнік быў дададзены ў сьпіс.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Ignore Mass Messages</source>
        <translation>Ігнараваць масавую разсылку</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Determines if mass messages are ignored or not.</source>
        <translation>Вызначае, ці будзе ігнаравана масавая разсылка.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Ignore Web Panel</source>
        <translation>Ігнараваць Вэб-панэль</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Determines if web panel messages are ignored or not.</source>
        <translation>Вызначае, ці будуць ігнараваны паведамленьні з Вэб-панэлі.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Ignore Email Pager</source>
        <translation>Ігнараваць паштовы пэйджар</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Determines if email pager messages are ignored or not.</source>
        <translation>Вызначае, ці будуць ігнараваны паведамленьні з паштовага пэйджэру.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Auto Away Messages</source>
        <translation>Паведамленьні пры зьмене стану</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Away:</source>
        <translation>Адсутнічаю:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>N/A:</source>
        <translation>Нядасягальны:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Auto Update</source>
        <translation>Аўтаматычнае абнаўленьне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Auto Update Info</source>
        <translation>Абнаўляць дадзеныя карыстальніка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Automatically update users&apos; server stored information.</source>
        <translation>Аўтаматычна абнавіць дадзеныя карыстальніка, захаваныя на паслужніку.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Auto Update Info Plugins</source>
        <translation>Абнавіць інфармацыйныя пашырэньні</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Automatically update users&apos; Phone Book and Picture.</source>
        <translation>Аўтаматычна абнавіць адрасную кнігу і малюнак карыстальніка.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Auto Update Status Plugins</source>
        <translation>Абнавіць пашырэньні стану</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Automatically update users&apos; Phone &quot;Follow Me&quot;, File Server and ICQphone status.</source>
        <translation>Аўтаматычна абнавіць стан ICQ-тэлефону і стан тэлефону &quot;Кроч за мной&quot;.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Style:</source>
        <translation>Стыль:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Insert Horizontal Line</source>
        <translation>Дадаць гарызантальную межу</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Colors</source>
        <translation>Колеры</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Message Received:</source>
        <translation>Атрыманае паведамленьне:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>History Received:</source>
        <translation>Гісторыя - атрыманае:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>History Sent:</source>
        <translation>Гісторыя - адасланае:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Typing Notification Color:</source>
        <translation>Колер паведамленьня
 &quot;Карыстальнік рыхтуе пасланьне&quot;:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Background Color:</source>
        <translation>Колер фону:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Previous Message</source>
        <translation>Папярэдняе паведамленьне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Single line chat mode</source>
        <translation>Рэжым размовы ў адзін радок</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>In single line chat mode you send messages with Enter and insert new lines with Ctrl+Enter, opposite of the normal mode</source>
        <translation>У рэжыме размовы ў адзін радок, націск на ўвод дасылае паведамленьне, а Ctrl+Enter дзейнічае наадварот, устаўляючы новы радок (як у ІРЦ-кліентах)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Alias</source>
        <translation>Мянушка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Name</source>
        <translation>Імя</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Date Format:</source>
        <translation>Фармат часу:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Picture</source>
        <translation>Малюнак</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Notice:</source>
        <translation>Заўвага:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>This is a sent message</source>
        <translation>Гэта адасланае паведамленьне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Have you gone to the Licq IRC Channel?</source>
        <translation>Ты ўжо далучыўся да каналу Licq у ІРЦ?</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>No, where is it?</source>
        <translation>Не, дзе гэта?</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>#Licq on irc.freenode.net</source>
        <translation>#Licq на irc.freenode.net</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Cool, I&apos;ll see you there :)</source>
        <translation>Файна! Убачымся там :)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>We&apos;ll be waiting!</source>
        <translation>Буду чакаць!</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Marge has left the conversation.</source>
        <translation>Мэрдж выйшаў з размовы.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>This is a received message</source>
        <translation>Гэта атрыманае паведамленьне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Use double return</source>
        <translation>Выкарыстоўваць падвойны ўвод</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Hitting Return twice will be used instead of Ctrl+Return to send messages and close input dialogs. Multiple new lines can be inserted with Ctrl+Return.</source>
        <translation>Падвойны націск на Ўвод будзе выкарыстоўвацца замест Ctrl+Return каб адаслаць паведамленьне. Каб уставіць некалькі новых радкоў націсьніце адпаведную колькась разоў Ctrl+Return.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Show Join/Left Notices</source>
        <translation>Паказваць паведамленьне аб уваходзе/выйсьці</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Show a notice in the chat window when a user joins or leaves the conversation.</source>
        <translation>Паказваць у акенцы размовы, што карыстальнік з&apos;явіўся альбо выйшаў.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Local time</source>
        <translation>Мясцовы час</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Chat Options</source>
        <translation>Наладкі размовы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Insert Vertical Spacing</source>
        <translation>Уставіць вертыкальнае раўнаваньне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Insert extra space between messages.</source>
        <translation>Дадаваць дадатковую прастору паміж паведамленьнямі.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Insert a line between each message.</source>
        <translation>Дадаваць радок паміж кожным пваедамленьнем.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&lt;p&gt;Available custom date format variables.&lt;/p&gt;
&lt;table&gt;
&lt;tr&gt;&lt;th&gt;Expression&lt;/th&gt;&lt;th&gt;Output&lt;/th&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;d&lt;/td&gt;&lt;td&gt;the day as number without a leading zero (1-31)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;dd&lt;/td&gt;&lt;td&gt;the day as number with a leading zero (01-31)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;ddd&lt;/td&gt;&lt;td&gt;the abbreviated localized day name (e.g. &apos;Mon&apos;..&apos;Sun&apos;)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;dddd&lt;/td&gt;&lt;td&gt;the long localized day name (e.g. &apos;Monday&apos;..&apos;Sunday&apos;)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;M&lt;/td&gt;&lt;td&gt;the month as number without a leading zero (1-12)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;MM&lt;/td&gt;&lt;td&gt;the month as number with a leading zero (01-12)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;MMM&lt;/td&gt;&lt;td&gt;the abbreviated localized month name (e.g. &apos;Jan&apos;..&apos;Dec&apos;)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;MMMM&lt;/td&gt;&lt;td&gt;the long localized month name (e.g. &apos;January&apos;..&apos;December&apos;)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;yy&lt;/td&gt;&lt;td&gt;the year as two digit number (00-99)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;yyyy&lt;/td&gt;&lt;td&gt;the year as four digit number (1752-8000)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td colspan=2&gt;&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;h&lt;/td&gt;&lt;td&gt;the hour without a leading zero (0..23 or 1..12 if AM/PM display)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;hh&lt;/td&gt;&lt;td&gt;the hour with a leading zero (00..23 or 01..12 if AM/PM display)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;m&lt;/td&gt;&lt;td&gt;the minute without a leading zero (0..59)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;mm&lt;/td&gt;&lt;td&gt;the minute with a leading zero (00..59)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;s&lt;/td&gt;&lt;td&gt;the second without a leading zero (0..59)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;ss&lt;/td&gt;&lt;td&gt;the second with a leading zero (00..59)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;z&lt;/td&gt;&lt;td&gt;the millisecond without leading zero (0..999)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;zzz&lt;/td&gt;&lt;td&gt;the millisecond with leading zero (000..999)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;AP&lt;/td&gt;&lt;td&gt;use AM/PM display. AP will be replaced by either &apos;AM&apos; or &apos;PM&apos;&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;ap&lt;/td&gt;&lt;td&gt;use am/pm display. ap will be replaced by either &apos;am&apos; or &apos;pm&apos;&lt;/td&gt;&lt;/tr&gt;
&lt;/table&gt;</source>
        <translation>&lt;p&gt;Даступныя зьменныя фармату часу.&lt;/p&gt;
&lt;table&gt;
&lt;tr&gt;&lt;th&gt;Выраз&lt;/th&gt;&lt;th&gt;Вывад&lt;/th&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;d&lt;/td&gt;&lt;td&gt;нумар дня месяца без пачатковага нуля (1-31)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;dd&lt;/td&gt;&lt;td&gt;нумар дня месяца з пачатковым нулём (01-31)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;ddd&lt;/td&gt;&lt;td&gt;скарочаная лакалізаваная назва дня тыдня (напрыклад, &apos;Пан&apos;..&apos;Няд&apos;)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;dddd&lt;/td&gt;&lt;td&gt;поўная лакалізаваня назва дня тыдня (напрыклад, &apos;Панядзелак&apos;..&apos;Нядзеля&apos;)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;M&lt;/td&gt;&lt;td&gt;нумар месяца без пачатковага нуля (1-12)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;MM&lt;/td&gt;&lt;td&gt;месяц лічбай з пачатковым нулём (01-12)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;MMM&lt;/td&gt;&lt;td&gt;скарочаная лакалізаваная назва месяца (напрыклад, &apos;Сту&apos;..&apos;Сне&apos;)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;MMMM&lt;/td&gt;&lt;td&gt;поўная лакалізаваная назва месяца (напрыклад, &apos;Студзень&apos;..&apos;Сьнежань&apos;)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;yy&lt;/td&gt;&lt;td&gt;год у выглядзе дзьвух апошніх лічбаў (00-99)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;yyyy&lt;/td&gt;&lt;td&gt;год у выглядзе чатырох лічбаў (1752-8000)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td colspan=2&gt;&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;h&lt;/td&gt;&lt;td&gt;гадзіны без пачаткова нуля (0..23)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;hh&lt;/td&gt;&lt;td&gt;гадзіны з пачатковым нулём (00..23)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;m&lt;/td&gt;&lt;td&gt;хвіліны без пачатковага нуля (0..59)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;mm&lt;/td&gt;&lt;td&gt;хвіліны з пачатковым нулём (00..59)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;s&lt;/td&gt;&lt;td&gt;сякунды без пачатковага нуля (0..59)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;ss&lt;/td&gt;&lt;td&gt;сякунды з пачатковым нулём (00..59)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;z&lt;/td&gt;&lt;td&gt;тысячныя сякунды без пачатковых нулёў (0..999)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;zzz&lt;/td&gt;&lt;td&gt;тысячныя сякунды з пачатковымі нулямі (000..999)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;AP&lt;/td&gt;&lt;td&gt;выкарыстоўваць AM/PM. AP будзе заменена на &apos;AM&apos; альбо на &apos;PM&apos;&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;ap&lt;/td&gt;&lt;td&gt;выкарыстоўваць am/pm. ap будзе заменена на &apos;am&apos; альбо на &apos;pm&apos;&lt;/td&gt;&lt;/tr&gt;
&lt;/table&gt;</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>History Options</source>
        <translation>Наладкі гісторыі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>History</source>
        <translation>Гісторыя</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>The format string used to define what will appear in each column.
The following parameters can be used:</source>
        <translation>Радок фармату вызначае, што будзе з&apos;яўляцца у кожным слупке.
Магчыма выкарыстоўваць наступныя параметры:</translation>
    </message>
</context>
<context>
    <name>OwnerEditDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>Edit Account</source>
        <translation>Рэдагаваньне ўліковага запісу</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>User ID:</source>
        <translation>Ідэнтыфікатар
карыстальніка:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Password:</source>
        <translation>Пароль:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Protocol:</source>
        <translation>Пратакол:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Currently only one account per protocol is supported.</source>
        <translation>Зараз падтрымліваецца толькі адзін уліковы запіс дзеля кожнага пратаколу.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Ok</source>
        <translation>&amp;Добра</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасаваць</translation>
    </message>
</context>
<context>
    <name>OwnerManagerDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>Licq - Account Manager</source>
        <translation>Licq - Наладкі ўладароў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Add</source>
        <translation>&amp;Дадаць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Register</source>
        <translation>&amp;Зарэгістраваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Modify</source>
        <translation>Зь&amp;мяніць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>D&amp;elete</source>
        <translation>В&amp;ыдаліць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Done</source>
        <translation>С&amp;кончыць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>From the Account Manager dialog you are able to add and register your accounts.
Currently, only one account per protocol is supported, but this will be changed in future versions.</source>
        <translation>Праз наладкі ўліковых запісаў магчыма дадаць і зарэгістраваць Вашыя акаунты.
Зараз толькі адзін уліковы запіс дзеля кожнага пратаколу падтрымліваеца Licq, але гэта павінна змяніцца ў будучых версіях.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>You are currently registered as
UIN (User ID): %1
Base Directory: %2
Rerun licq with the -b option to select a new
base directory and then register a new user.</source>
        <translation>Вы зараз ужо зарэгістраваны з
UIN (ідэнтыфікатарам карыстальніка): %1
і працоўным каталёгам  %2
Перазапусціце licq з опцыяй -b каб выбраць новы (альбо іншы) каталёг і пасля гэтага зарэгістрыруйцесь зноўку.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Successfully registered, your user identification
number (UIN) is %1.
Now set your personal information.</source>
        <translation>Зарэгістраваны пасьпяхова. 
Ваш ідэнтыфікатар карыстальніка - %1.
Зараз усталюйце Вашы асабістыя дадзеныя.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Registration failed.  See network window for details.</source>
        <translation>Няўдалая спроба рэгістрацыі.  Глядзіце падрабязнасьці ў акенцы сетцы.</translation>
    </message>
</context>
<context>
    <name>OwnerView</name>
    <message>
        <location filename="" line="0"/>
        <source>User ID</source>
        <translation>Ідэнтыфікатар карыстальніка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Protocol</source>
        <translation>Пратакол</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>(Invalid ID)</source>
        <translation>(Няверны ідэнтыфікатар)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Invalid Protocol</source>
        <translation>Няверны пратакол</translation>
    </message>
</context>
<context>
    <name>PluginDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>Standard Plugins</source>
        <translation>Стандартныя пашырэньні</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Id</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Name</source>
        <translation>Імя</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Version</source>
        <translation>Вэрсія</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Load</source>
        <translation>Загрузіць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Enable</source>
        <translation>Уключыць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Description</source>
        <translation>Апісаньне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Protocol Plugins</source>
        <translation>Пашырэньне пратаколаў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Refresh</source>
        <translation>Абнавіць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Done</source>
        <translation>Скончыць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Licq Plugin %1 %2
</source>
        <translation>Licq Пашырэньне %1 %2
</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Plugin %1 has no configuration file</source>
        <translation>Пашырэньне %1 не мае наладкавых файлаў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>(Unloaded)</source>
        <translation>(Не загружаны)</translation>
    </message>
</context>
<context>
    <name>QMessageBox</name>
    <message>
        <location filename="" line="0"/>
        <source>Licq Question</source>
        <translation>Licq Пытаньне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Yes</source>
        <translation>Так</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>No</source>
        <translation>Не</translation>
    </message>
</context>
<context>
    <name>RegisterUserDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>Welcome to the Registration Wizard.

You can register a new user here.

Press &quot;Next&quot; to proceed.</source>
        <translation>Вітаем у майстры рэгістрацыі.

Тут Вы маеце магчымасць дадаць новага карыстальніка.

Націсьніце &quot;Далей&quot; дзеля працягу.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Account Registration</source>
        <translation>Рэгістрацыя карыстальніка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Enter a password to protect your account.</source>
        <translation>Увядзіце пароль каб абараніць Ваш уліковы запіс.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Password:</source>
        <translation>Пароль:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Verify:</source>
        <translation>Падцьверджаньне:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Remember Password</source>
        <translation>&amp;Захаваць пароль</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Account Registration - Step 2</source>
        <translation>Рэгістрацыя карыстальніка - крок 2</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Account Registration - Step 3</source>
        <translation>Рэгістрацыя карыстальніка - крок 3</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Licq Account Registration</source>
        <translation>Licq Рэгістрацыя карыстальніка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Invalid password, must be between 1 and 8 characters.</source>
        <translation>Няверны пароль, ён павінен быць ад 1 да 8 сымбаляў.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Please enter your password in both input fields.</source>
        <translation>Калі ласка ўвядзіце пароль у абедзьвух палях.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>The passwords don&apos;t seem to match.</source>
        <translation>Здаецца, што пароль не супадае.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Now please press the &apos;Back&apos; button and try again.</source>
        <translation>Зараз націсьніце кнопку &quot;Назад&quot; і паспрабуйце яшчэ раз.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Now please click &apos;Finish&apos; to start the registration process.</source>
        <translation>Зараз націсьніце кнопку &quot;Скончыць&quot; каб пачаць рэгістрацыю на паслужніку.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Account Registration in Progress...</source>
        <translation>Выконваецца рэгістрацыя...</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Account registration has been successfuly completed.
Your new user id is %1.
You are now being automatically logged on.
Click OK to edit your personal details.
After you are online, you can send your personal details to the server.</source>
        <translation>Рэгістрацыя завяршылася пасьпяхова.
Ваш нумар - %1.
Зараз Вы аўтаматычна ўвойдзіце ў сеціва.
Націсьніце &quot;Добра&quot; каб рэдагаваць Вашы ўласныя дадзеныя.
Пасля таго, як Вы войдзеце, Вы здолееце даслаць дадзеныя на паслужнік.</translation>
    </message>
</context>
<context>
    <name>ReqAuthDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>Licq - Request Authorization</source>
        <translation>Licq - Запыт аўтарызацыі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Request authorization from (UIN):</source>
        <translation>Запыт аўтарызацыі ад (UIN):</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Request</source>
        <translation>Запыт</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Ok</source>
        <translation>&amp;Добра</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасаваць</translation>
    </message>
</context>
<context>
    <name>SearchUserDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>Licq - User Search</source>
        <translation>Licq -Пошук карыстальніка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Alias:</source>
        <translation>Мянушка:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>First Name:</source>
        <translation>Імя:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Last Name:</source>
        <translation>Імя па бацьку:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Age Range:</source>
        <translation>Узрост:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Unspecified</source>
        <translation>Ня вызначаны</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>18 - 22</source>
        <translation>18 - 22</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>23 - 29</source>
        <translation>23 - 29</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>30 - 39</source>
        <translation>30 - 39</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>40 - 49</source>
        <translation>40 - 49</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>50 - 59</source>
        <translation>50 - 59</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>60+</source>
        <translation>Больш за 60</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Gender:</source>
        <translation>Род:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Female</source>
        <translation>Жаночы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Male</source>
        <translation>Мужчынскі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Language:</source>
        <translation>Мова:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>City:</source>
        <translation>Горад:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>State:</source>
        <translation>Вобласць/Штат:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Country:</source>
        <translation>Краіна:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Company Name:</source>
        <translation>Месца працы:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Company Department:</source>
        <translation>Аддзел:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Company Position:</source>
        <translation>Пасада:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Email Address:</source>
        <translation>Паштовы адрас:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Keyword:</source>
        <translation>Ключавое слова:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Return Online Users Only</source>
        <translation>Паказваць толькі карыстальнікаў у сетцы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Whitepages</source>
        <translation>&amp;Белыя старонкі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>UIN#:</source>
        <translation>Нумар:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;UIN#</source>
        <translation>&amp;Нумар:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Search</source>
        <translation>&amp;Пошук</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Reset Search</source>
        <translation>Скасаваць усё</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Done</source>
        <translation>&amp;Скончыць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Enter search parameters and select &apos;Search&apos;</source>
        <translation>Увядзіці дадзеныя дзеля пошуку і націсніце &quot;Пошук&quot;</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>A&amp;lert User</source>
        <translation>Па&amp;ведаміць карыстальніка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>View &amp;Info</source>
        <translation>П&amp;раглядзець дадзеныя</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Add User</source>
        <translation>&amp;Дадаць карыстальніка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Cancel</source>
        <translation>Скасаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Searching (this can take awhile)...</source>
        <translation>Пошук (магчыма, патрабуецца некалькі хвілін)...</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Search complete.</source>
        <translation>Пошук завершаны.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>More users found. Narrow search.</source>
        <translation>Знойдзена шмат карыстальнікаў. Удакладніце крытэрыі пошуку.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>%1 more users found. Narrow search.</source>
        <translation>Знойдзена %1 карыстальнікаў. Удакладніце крытэрыі пошуку.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Search failed.</source>
        <translation>Пошук няўдалы.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Add %1 Users</source>
        <translation>&amp;Дадаць %1 карыстальнікаў</translation>
    </message>
</context>
<context>
    <name>SearchUserView</name>
    <message>
        <location filename="" line="0"/>
        <source>Alias</source>
        <translation>Мянушка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>UIN</source>
        <translation>Нумар</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Name</source>
        <translation>Імя</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Email</source>
        <translation>Эл. пошта</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Status</source>
        <translation>Стан</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Sex &amp; Age</source>
        <translation>Род і ўзрост</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Authorize</source>
        <translation>Аўтарызацыя</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Offline</source>
        <translation>Адключаны</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Online</source>
        <translation>У сетцы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Unknown</source>
        <translation>Не вядома</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>F</source>
        <translation>Ж</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>M</source>
        <translation>М</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>?</source>
        <translation>?</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>No</source>
        <translation>Не</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Yes</source>
        <translation>Так</translation>
    </message>
</context>
<context>
    <name>SecurityDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>Options</source>
        <translation>Наладкі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Password/UIN settings</source>
        <translation>Наладкі паролю ды нумара</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Uin:</source>
        <translation>&amp;Нумар:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Enter the UIN which you want to use.  Only available if &quot;Local changes only&quot; is checked.</source>
        <translation>Увядзіце нумар ICQ які Вы жадаеце выкарыстоўваць.  Наладка дасягальна толькі калі ўсталявана &quot;Зьмяніць толькі лакальна&quot;.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Password:</source>
        <translation>&amp;Пароль:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Enter your ICQ password here.</source>
        <translation>Увядзіце тут Ваш ICQ пароль.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Verify:</source>
        <translation>П&amp;адцьверджаньне:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Verify your ICQ password here.</source>
        <translation>Увядзіце яшчэ раз Ваш пароль.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Local changes only</source>
        <translation>&amp;Зьмяніць толькі лакальна</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>If checked, password/UIN changes will apply only on your local computer.  Useful if your password is incorrectly saved in Licq.</source>
        <translation>Калі ўсталявана, то пароль з нумарам будуць зменены толькі на Вашым камп&apos;ютары.  Гэта выкарыстоўваецца, калі Licq памылкова захоўвае пароль.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Authorization Required</source>
        <translation>Неабходна аўтарызацыя</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Determines whether regular ICQ clients require your authorization to add you to their contact list.</source>
        <translation>Вызначае, ці патрэбна Ваша аўтарызацыя каб звычайныя icq-кліенты малі магчымасць дадаць Вас у сьпіс кантактаў.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Web Presence</source>
        <translation>Прысутнасць у сеціве</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Web Presence allows users to see if you are online through your web indicator.</source>
        <translation>Гэтая наладка дазваляе бачыць Ваш статус праз вэб-інтэрфейс.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Hide IP</source>
        <translation>Схаваць IP</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Hide IP stops users from seeing your IP address. It doesn&apos;t guarantee it will be hidden though.</source>
        <translation>Дазваляе схаваць ваш IP-адрас ад іншых карыстальнікаў, але гэта не гарантавана.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Update</source>
        <translation>А&amp;бнавіць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>ICQ Security Options</source>
        <translation>Наладкі бясьпекі ICQ</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Invalid password, must be between 1 and 8 characters.</source>
        <translation>Няспраўны пароль, ён павінен быць ад 1 да 8 сымбаляў.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Passwords do not match, try again.</source>
        <translation>Пароль не супадае, паспрабуйце яшчэ раз.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>You need to be connected to the
ICQ Network to change the settings.</source>
        <translation>Неабходна быць злучаным з ICQ-паслужнікам каб зьмяніць гэтую наладку.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>ICQ Security Options [Setting...]</source>
        <translation>Усталяваньна наладак бясьпекі...</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>failed</source>
        <translation>няўдала</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Setting security options failed.</source>
        <translation>Няўдалае ўсталяваньне наладак бясьпекі.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Changing password failed.</source>
        <translation>Няўдалая зьмена паролю.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>timed out</source>
        <translation>скончыўся час</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Timeout while setting security options.</source>
        <translation>Не хапіла часу каб усталяваць наладкі бясьпекі.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Timeout while changing password.</source>
        <translation>Не хапіла часу каб зьмяніць пароль.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>error</source>
        <translation>памылка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Internal error while setting security options.</source>
        <translation>Унутраная памылка пры ўсталяваньні наладак бясьпекі.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Internal error while changing password.</source>
        <translation>Унутраная памылка пры зьмене паролю.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>ICQ Security Options [Setting...</source>
        <translation>Наладкі бясьпекі ICQ [усталяваньне...</translation>
    </message>
</context>
<context>
    <name>ShowAwayMsgDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Show Again</source>
        <translation>&amp;Паказваць зноўку</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>%1 Response for %2</source>
        <translation>%1 адказ дзеля %2</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Ok</source>
        <translation>&amp;Добра</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>refused</source>
        <translation>адмоўлена</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>failed</source>
        <translation>няўдала</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>timed out</source>
        <translation>скончыўся час</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>error</source>
        <translation>памылка</translation>
    </message>
</context>
<context>
    <name>SkinBrowserDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>Skin selection</source>
        <translation>Выбар зьнешняга выгляду</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Preview</source>
        <translation>Прадпрагляд</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>S&amp;kins:</source>
        <translation>З&amp;ьнешні выгляд:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Use this combo box to select one of the available skins</source>
        <translation>Абярыце адзін з дасягальных выглядаў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Icons:</source>
        <translation>&amp;Выявы:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Use this combo box to select one of the available icon sets</source>
        <translation>Абярыце адзін з дасягальных набораў выяваў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>E&amp;xtended Icons:</source>
        <translation>&amp;Пашыраныя выявы:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Use this combo box to select one of the available extended icon sets</source>
        <translation>Абярыце адзін з дасягальных набораў пашыраных выяваў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Use this combo box to select one of the available emoticon icon sets</source>
        <translation>Абярыце адзін з дасягальных набораў эмацыйных выяваў (смайлаў)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Skin:</source>
        <translation>Выгляд:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Icons:</source>
        <translation>Выявы:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Extended Icons:</source>
        <translation>Пашыраныя выявы:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Emoticons:</source>
        <translation>Выявы эмоцый:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Edit Skin</source>
        <translation>&amp;Рэдагаваць выгляд</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Ok</source>
        <translation>&amp;Добра</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Apply</source>
        <translation>&amp;Ужыць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Error</source>
        <translation>Памылка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Unable to open icons file
%1
Iconset &apos;%2&apos; has been disabled.</source>
        <translation>Немагчыма адчыніць файл выяваў:
%1
 Набор выяваў &apos;%2&apos; будзе нядасягальным.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Unable to open extended icons file
%1
Extended Iconset &apos;%2&apos; has been disabled.</source>
        <translation>Немагчыма адчыніць файл пашыраных выяваў:
%1
 Набор пашыраных выяваў &apos;%2&apos; будзе нядасягальным.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Licq Skin Browser</source>
        <translation>Licq Наладкі зьнешняга выгляду</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Unable to open icons file
%1</source>
        <translation>Немагчыма адчыніць файл выяваў:
%1</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Unable to open extended icons file
%1</source>
        <translation>Немагчыма адчыніць файл пашыраных выяваў:
%1</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>E&amp;moticons:</source>
        <translation>&amp;Эмацыйныя выявы:</translation>
    </message>
</context>
<context>
    <name>Status</name>
    <message>
        <location filename="" line="0"/>
        <source>Offline</source>
        <translation>Адключаны</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Online</source>
        <translation>У сетцы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Away</source>
        <translation>Адсутнічаю</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Do Not Disturb</source>
        <translation>Не турбаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Not Available</source>
        <translation>Нядасягальны</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Occupied</source>
        <translation>Заняты</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Free for Chat</source>
        <translation>Згодны размаўляць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Unknown</source>
        <translation>Не вядома</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Off</source>
        <translation>Адкл</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>On</source>
        <translation>УС</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>DND</source>
        <translation>НТ</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>N/A</source>
        <translation>НД</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Occ</source>
        <translation>Зан</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>FFC</source>
        <translation>ЗР</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>???</source>
        <translation>???</translation>
    </message>
</context>
<context>
    <name>UserCodec</name>
    <message>
        <location filename="" line="0"/>
        <source>Unicode</source>
        <translation>Юнікод</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Unicode-16</source>
        <translation>Юнікод-16</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Arabic</source>
        <translation>Арабскае</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Baltic</source>
        <translation>Балтыйскае</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Central European</source>
        <translation>Цэнтральна Эўрапейскае</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Chinese</source>
        <translation>Кітайскае</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Chinese Traditional</source>
        <translation>Традыцыйнае Кітайскае</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Cyrillic</source>
        <translation>Кірылічнае</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Esperanto</source>
        <translation>Эспэранта</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Greek</source>
        <translation>Грэчаскае</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Hebrew</source>
        <translation>Яўрыт</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Japanese</source>
        <translation>Японскае</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Korean</source>
        <translation>Карэйскае</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Western European</source>
        <translation>Заходне Эўрапэйскае</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Tamil</source>
        <translation>Тамільскае</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Thai</source>
        <translation>Тайскае</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Turkish</source>
        <translation>Турэцкае</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Ukrainian</source>
        <translation>Украінскае</translation>
    </message>
</context>
<context>
    <name>UserEventCommon</name>
    <message>
        <location filename="" line="0"/>
        <source>Status:</source>
        <translation>Стан:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Time:</source>
        <translation>Час:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Open / Close secure channel</source>
        <translation>Адчыніць / зачыніць бясьпечны канал</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Show User History</source>
        <translation>Прагляд гісторыі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Show User Info</source>
        <translation>Прагляд зьвестак аб карыстальніку</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Change user text encoding</source>
        <translation>Зьмяніць кадоўку</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>This button selects the text encoding used when communicating with this user. You might need to change the encoding to communicate in a different language.</source>
        <translation>Гэтая кнопка вызначае, якая кадоўка будзе ўжывацца да гэтага карыстальніка. Выкарыстоўваецца пры выкарыстаньні розны моў.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Unable to load encoding &lt;b&gt;%1&lt;/b&gt;. Message contents may appear garbled.</source>
        <translation>Немагчыма загрузіць кадоўку &lt;b&gt;%1&lt;/b&gt;. Магчыма паведамленьне будзе выглядаць пашкоджаным.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Unknown</source>
        <translation>Не вядома</translation>
    </message>
</context>
<context>
    <name>UserInfoDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Update</source>
        <translation>А&amp;бнавіць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Close</source>
        <translation>З&amp;ачыніць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Save</source>
        <translation>&amp;Захаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Retrieve</source>
        <translation>Атрымаць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Menu</source>
        <translation>&amp;Мяню</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Licq - Info </source>
        <translation>Licq - Дадзеныя </translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>INVALID USER</source>
        <translation>НЯСПРАЎНЫ КАРЫСТАЛЬНІК</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;General</source>
        <translation>А&amp;сноўныя</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Alias:</source>
        <translation>Мянушка:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Keep Alias on Update</source>
        <translation>Захоўваць мянушку пры абнаўленьні</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Normally Licq overwrites the Alias when updating user details.
Check this if you want to keep your changes to the Alias.</source>
        <translation>Звычайна Licq перазапісвае мянушку пры абнаўленьні дадзеных.
Усталюйце гэтую наладку каб захаваць мянушку.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>ID:</source>
        <translation>Нумар:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>IP:</source>
        <translation>IP-адрас:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Status:</source>
        <translation>Стан:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Timezone:</source>
        <translation>Часавая зона: </translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Name:</source>
        <translation>Імя:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>EMail 1:</source>
        <translation>Паштовы адрас 1:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>EMail 2:</source>
        <translation>Паштовы адрас 2:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Old Email:</source>
        <translation>Стары паштовы
адрас:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Address:</source>
        <translation>Адрас:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Phone:</source>
        <translation>Тэлефон:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>State:</source>
        <translation>Вобласць/Штат:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Fax:</source>
        <translation>Факс:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>City:</source>
        <translation>Горад:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Cellular:</source>
        <translation>Мабільны
тэлефон:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Zip:</source>
        <translation>Індэкс:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Country:</source>
        <translation>Краіна:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Unknown</source>
        <translation>Не вядома</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Unknown (%1)</source>
        <translation>Невядомая (%1)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Now</source>
        <translation>Зараз</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;More</source>
        <translation>&amp;Больш</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Age:</source>
        <translation>Узрост:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Gender:</source>
        <translation>Род:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Unspecified</source>
        <translation>Ня вызначаны</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Female</source>
        <translation>Жаночы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Male</source>
        <translation>Мужчынскі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Homepage:</source>
        <translation>Хатняя старонка:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Category:</source>
        <translation>Катэгорыя:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Description:</source>
        <translation>Апісаньне:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Birthday:</source>
        <translation>Дзень народзінаў:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source> Day:</source>
        <translation> Дзень:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source> Month:</source>
        <translation> Месяц:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source> Year:</source>
        <translation> Год:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Language 1:</source>
        <translation>Асноўная мова :</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Language 2:</source>
        <translation>Другая мова:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Language 3:</source>
        <translation>Трэцяя мова:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Authorization Required</source>
        <translation>Неабходна аўтарызацыя</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Authorization Not Required</source>
        <translation>Аўтарызацыя не патрэбна</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>User has an ICQ Homepage </source>
        <translation>Карыстальнік мае старонку на вэб-паслужніку ICQ</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>User has no ICQ Homepage</source>
        <translation>Карыстальнік не мае старонцы на вэб-паслужніку ICQ</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>M&amp;ore II</source>
        <translation>Дада&amp;ткова</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>(none)</source>
        <translation>(адсутнічае)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Work</source>
        <translation>&amp;Праца</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Department:</source>
        <translation>Аддзел:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Position:</source>
        <translation>Пасада:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Occupation:</source>
        <translation>Напрамак
дзейнасьці:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;About</source>
        <translation>Зьв&amp;есткі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>About:</source>
        <translation>Зьвесткі:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Phone</source>
        <translation>Т&amp;элефон</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Type</source>
        <translation>Тып</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Number/Gateway</source>
        <translation>Нумар/Шлюз</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Country/Provider</source>
        <translation>Краіна/Аператар</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Currently at:</source>
        <translation>Зараз у:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>(</source>
        <translation>(</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>) </source>
        <translation>) </translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>-</source>
        <translation>-</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>P&amp;icture</source>
        <translation>Мал&amp;юнак</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Not Available</source>
        <translation>Нядасягальны</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Failed to Load</source>
        <translation>Немагчыма загрузіць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Last</source>
        <translation>Апо&amp;шні раз</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Last Online:</source>
        <translation>У сетцы:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Last Sent Event:</source>
        <translation>Нешта адаслана:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Last Received Event:</source>
        <translation>Нешта прынята:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Last Checked Auto Response:</source>
        <translation>Прагледжаны аўта-адказ:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Online Since:</source>
        <translation>У сетцы з:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Offline</source>
        <translation>Адключаны</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;KDE Addressbook</source>
        <translation>Адрасная кніга &amp;KDE</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Email:</source>
        <translation>Скрынка:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;History</source>
        <translation>&amp;Гісторыя</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Rever&amp;se</source>
        <translation>Наадвар&amp;от</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Filter: </source>
        <translation>&amp;Фільтр: </translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Error loading history file: %1
Description: %2</source>
        <translation>Немагчыма прачытаць файл гісторыі: %1
Апісаньне: %2</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Sorry, history is disabled for this person.</source>
        <translation>Прабачце, але гісторыя не вядзецца дзеля гэтага карыстальніка.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>server</source>
        <translation>паслужнік</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>[&lt;font color=&quot;%1&quot;&gt;Received&lt;/font&gt;] [&lt;font color=&quot;%2&quot;&gt;Sent&lt;/font&gt;] %3 to %4 of %5</source>
        <translation>[&lt;font color=&quot;%1&quot;&gt;Атрымана&lt;/font&gt;] [&lt;font color=&quot;%2&quot;&gt;Адаслана&lt;/font&gt;] %3 па %4 з %5</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>[&lt;font color=&quot;%1&quot;&gt;Received&lt;/font&gt;] [&lt;font color=&quot;%2&quot;&gt;Sent&lt;/font&gt;] %3 out of %4 matches</source>
        <translation>[&lt;font color=&quot;%1&quot;&gt;Атрымана&lt;/font&gt;] [&lt;font color=&quot;%2&quot;&gt;Адаслана&lt;/font&gt;] %3 з %4 знойдзеных</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Retrieve</source>
        <translation>Атры&amp;маць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>S&amp;end</source>
        <translation>Даслац&amp;ь</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Add</source>
        <translation>&amp;Дадаць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Clear</source>
        <translation>&amp;Выдаліць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Browse</source>
        <translation>Прагл&amp;ядзець</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Nex&amp;t</source>
        <translation>&amp;Наступная</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>P&amp;rev</source>
        <translation>Папяр&amp;эдняя</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>You need to be connected to the
ICQ Network to retrieve your settings.</source>
        <translation>Неабходна быць злучаным з паслужнікам
каб атрымаць гэтыя зьвесткі.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Updating...</source>
        <translation>Абнаўленьне...</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>You need to be connected to the
ICQ Network to change your settings.</source>
        <translation>Неабходна быць злучаным з паслужнікам
каб зьмяніць гэтыя зьвесткі.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Select your picture</source>
        <translation>Абярыце Ваш малюнак</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source> is over %1 bytes.
Select another picture?</source>
        <translation> больш за %1 байт.
Абраць іншы малюнак?</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Yes</source>
        <translation>Так</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>No</source>
        <translation>Не</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Updating server...</source>
        <translation>Абнаўленьне на паслужніку...</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>error</source>
        <translation>памылка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>done</source>
        <translation>завершана</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>failed</source>
        <translation>няўдала</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>timed out</source>
        <translation>скончыўся час</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Registration Date:</source>
        <translation>Дата рэгістрацыі:</translation>
    </message>
</context>
<context>
    <name>UserSelectDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>&amp;User:</source>
        <translation>&amp;Карыстальнік:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Password:</source>
        <translation>&amp;Пароль:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Save Password</source>
        <translation>&amp;Захаваць пароль</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Ok</source>
        <translation>&amp;Добра</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Licq User Select</source>
        <translation>Licq Выбар карыстальніка</translation>
    </message>
</context>
<context>
    <name>UserSendChatEvent</name>
    <message>
        <location filename="" line="0"/>
        <source>Multiparty: </source>
        <translation>Шматкарыстальніцкі: </translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Invite</source>
        <translation>Запрасіць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source> - Chat Request</source>
        <translation> - Запыт размовы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Clear</source>
        <translation>Ачысьціць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>No reason provided</source>
        <translation>Нагода ня вызначана</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Chat with %2 refused:
%3</source>
        <translation>Усталяваць размову з %2 не атрымалася:
%3</translation>
    </message>
</context>
<context>
    <name>UserSendCommon</name>
    <message>
        <location filename="" line="0"/>
        <source>Se&amp;nd through server</source>
        <translation>Дас&amp;ылаць праз паслужнік</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>U&amp;rgent</source>
        <translation>&amp;Хутка(UDP)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>M&amp;ultiple recipients</source>
        <translation>Некальк&amp;ім карыстальнікам</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Menu</source>
        <translation>&amp;Мяню</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Message</source>
        <translation>Паведамленьне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>URL</source>
        <translation>Спасылка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Chat Request</source>
        <translation>Запрашэньне да размовы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>File Transfer</source>
        <translation>Перадача файла</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Contact List</source>
        <translation>Сьпіс кантактаў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>SMS</source>
        <translation>SMS</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Send</source>
        <translation>&amp;Даслаць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Close</source>
        <translation>З&amp;ачыніць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Error! no owner set</source>
        <translation>Памылка! Уладальнік ня вызначаны</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Drag Users Here
Right Click for Options</source>
        <translation>Перацягніце карыстальнікаў сюды
Правы клік дзеля варыянтаў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Sending </source>
        <translation>Дасыланьне </translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>via server</source>
        <translation>праз паслужнік</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>direct</source>
        <translation>на прамую</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>error</source>
        <translation>памылка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>done</source>
        <translation>завершана</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>cancelled</source>
        <translation>спынена</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>failed</source>
        <translation>няўдала</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>timed out</source>
        <translation>скончыўся час</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Direct send failed,
send through server?</source>
        <translation>Прамое злучэньне не атрымалася,
 адаслаць праз паслужнік?</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Yes</source>
        <translation>Так</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>No</source>
        <translation>Не</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>%1 is in %2 mode:
%3
Send...</source>
        <translation>%1 у рэжыме %2:
%3
Адсылаем...</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Urgent</source>
        <translation>Хутка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source> to Contact List</source>
        <translation> у сьпіс кантактаў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Cancel</source>
        <translation>Скасаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Warning: Message can&apos;t be sent securely
through the server!</source>
        <translation>Увага: не магчыма адаслаць паведамленьне бяспечна
праз паслужнік!</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Send anyway</source>
        <translation>Даслаць усёроўна</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>%1 has joined the conversation.</source>
        <translation>%1 далучыўся да размовы.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>%1 has left the conversation.</source>
        <translation>%1 пакінуў размову.</translation>
    </message>
</context>
<context>
    <name>UserSendContactEvent</name>
    <message>
        <location filename="" line="0"/>
        <source>Drag Users Here - Right Click for Options</source>
        <translation>Перацягніце карыстальнікаў сюды - правы клік дзеля варыянтаў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source> - Contact List</source>
        <translation> - Сьпіс кантактаў</translation>
    </message>
</context>
<context>
    <name>UserSendFileEvent</name>
    <message>
        <location filename="" line="0"/>
        <source>File(s): </source>
        <translation>Файл(ы): </translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Browse</source>
        <translation>Праглядзець</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Edit</source>
        <translation>Рэдагаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source> - File Transfer</source>
        <translation> - Перадача файла</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Select files to send</source>
        <translation>Аяберыце файлы дзеля адсылкі</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>You must specify a file to transfer!</source>
        <translation>Вы павінны вызначыць файлы дзеля адсылкі!</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>No reason provided</source>
        <translation>Нагода ня вызначана</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>File transfer with %2 refused:
%3</source>
        <translation>Перадача файла да %2 не атрымалася:
%3</translation>
    </message>
</context>
<context>
    <name>UserSendMsgEvent</name>
    <message>
        <location filename="" line="0"/>
        <source> - Message</source>
        <translation> - Паведамленьне</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>You didn&apos;t edit the message.
Do you really want to send it?</source>
        <translation>Вы не рэдагавалі паведамленьне.
Даслаць усёроўна?</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Yes</source>
        <translation>&amp;Так</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;No</source>
        <translation>&amp;Не</translation>
    </message>
</context>
<context>
    <name>UserSendSmsEvent</name>
    <message>
        <location filename="" line="0"/>
        <source>Phone : </source>
        <translation>Тэлефон : </translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Chars left : </source>
        <translation>Засталося : </translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source> - SMS</source>
        <translation> - СМС</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>You didn&apos;t edit the SMS.
Do you really want to send it?</source>
        <translation>Вы не рэдагавалі СМС.
Вы сапраўды жадаеце адаслаць яе?</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Yes</source>
        <translation>&amp;Так</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;No</source>
        <translation>&amp;Не</translation>
    </message>
</context>
<context>
    <name>UserSendUrlEvent</name>
    <message>
        <location filename="" line="0"/>
        <source>URL : </source>
        <translation>Спасылка : </translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source> - URL</source>
        <translation> - Спасылка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>No URL specified</source>
        <translation>Спасылка ня вызначана</translation>
    </message>
</context>
<context>
    <name>UserViewEvent</name>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Menu</source>
        <translation>&amp;Мяню</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Aut&amp;o Close</source>
        <translation>А&amp;ўтаматычна зачыняць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Nex&amp;t</source>
        <translation>&amp;Наступнае</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Close</source>
        <translation>З&amp;ачыніць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Normal Click - Close Window
&lt;CTRL&gt;+Click - also delete User</source>
        <translation>Звычайны клік - зачыняе акенца
клік пры націснутым &lt;CTRL&gt; - выдаляе карыстальніка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Nex&amp;t (%1)</source>
        <translation>&amp;Наступнае (%1)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Reply</source>
        <translation>Ад&amp;казаць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>
--------------------
Request was cancelled.</source>
        <translation>
---------------
Запыт быў спынены.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>A&amp;ccept</source>
        <translation>П&amp;рыняць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Refuse</source>
        <translation>Адк&amp;ланіць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Join</source>
        <translation>&amp;Далучыцца</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Quote</source>
        <translation>&amp;Цытаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Forward</source>
        <translation>&amp;Перанакіраваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Start Chat</source>
        <translation>Пачаць размову</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;View</source>
        <translation>Пра&amp;гляд</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>A&amp;uthorize</source>
        <translation>Аў&amp;тарызаваць</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>A&amp;dd User</source>
        <translation>Дадаць кар&amp;ыстальніка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>A&amp;dd %1 Users</source>
        <translation>Дадаць %1 кар&amp;ыстальнікаў</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;View Email</source>
        <translation>Праглядзець по&amp;шту</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Licq is unable to find a browser application due to an internal error.</source>
        <translation>Licq ня здолеў знайсьці праглядач з-за ўнутранай памылкі.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Licq is unable to start your browser and open the URL.
You will need to start the browser and open the URL manually.</source>
        <translation>Licq ня здолеў запусціць праглядач і адчыніць спасылку.
Вам прыйдзецца гэта зрабіць самастойна.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Chat</source>
        <translation>Размова</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>File Transfer</source>
        <translation>Перадача файла</translation>
    </message>
</context>
<context>
    <name>VerifyDlg</name>
    <message>
        <location filename="" line="0"/>
        <source>Retype the letters shown above:</source>
        <translation>Увядзіце літары, што намаляваны вышэй:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;OK</source>
        <translation>&amp;Добра</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Licq - New Account Verification</source>
        <translation>Licq - Праверка новага ўліковага запісу</translation>
    </message>
</context>
<context>
    <name>WharfIcon</name>
    <message>
        <location filename="" line="0"/>
        <source>Left click - Show main window
Middle click - Show next message
Right click - System menu</source>
        <translation>Клік левай кнопкай - паказаць галоўнае акенца
клік сярэдняй кнопкай - паказаць наступнае паведамленьне
клік правай кнопкай - сістэмнае мяню</translation>
    </message>
</context>
</TS>
