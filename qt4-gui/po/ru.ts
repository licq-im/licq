<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="ru_RU">
<context>
    <name>LicqQtGui::AboutDlg</name>
    <message>
        <location filename="../src/dialogs/aboutdlg.cpp" line="44"/>
        <source>Licq - About</source>
        <translation>Licq - О программе</translation>
    </message>
    <message>
        <location filename="../src/dialogs/aboutdlg.cpp" line="69"/>
        <source>Version</source>
        <translation>Версия</translation>
    </message>
    <message>
        <location filename="../src/dialogs/aboutdlg.cpp" line="78"/>
        <source>Compiled on</source>
        <translation>Скомпилировано</translation>
    </message>
    <message>
        <location filename="../src/dialogs/aboutdlg.cpp" line="80"/>
        <source>Credits</source>
        <translation>Благодарность</translation>
    </message>
    <message>
        <location filename="../src/dialogs/aboutdlg.cpp" line="81"/>
        <source>Maintainer</source>
        <translation>Ведущий разработчик</translation>
    </message>
    <message>
        <location filename="../src/dialogs/aboutdlg.cpp" line="82"/>
        <source>Contributions</source>
        <translation>При содействии</translation>
    </message>
    <message>
        <location filename="../src/dialogs/aboutdlg.cpp" line="83"/>
        <source>Original author</source>
        <translation>Первоначальный автор</translation>
    </message>
    <message>
        <location filename="../src/dialogs/aboutdlg.cpp" line="84"/>
        <source>Contact us</source>
        <translation>Обратная связь</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::AddGroupDlg</name>
    <message>
        <location filename="../src/dialogs/addgroupdlg.cpp" line="43"/>
        <source>Licq - Add group</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/dialogs/addgroupdlg.cpp" line="48"/>
        <source>New &amp;Group Name:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/dialogs/addgroupdlg.cpp" line="56"/>
        <source>&amp;Position:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>LicqQtGui::AddUserDlg</name>
    <message>
        <location filename="../src/dialogs/adduserdlg.cpp" line="46"/>
        <source>Licq - Add user</source>
        <translation>Licq - добавление пользователя</translation>
    </message>
    <message>
        <location filename="../src/dialogs/adduserdlg.cpp" line="51"/>
        <source>&amp;Protocol:</source>
        <translation>&amp;Протокол:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/adduserdlg.cpp" line="61"/>
        <source>&amp;Group:</source>
        <translation>&amp;Группа:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/adduserdlg.cpp" line="72"/>
        <source>New &amp;User ID:</source>
        <translation>&amp;Идентификатор нового пользователя:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/adduserdlg.cpp" line="83"/>
        <source>&amp;Notify User</source>
        <translation>И&amp;звестить пользователя</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::AuthUserDlg</name>
    <message>
        <location filename="../src/dialogs/authuserdlg.cpp" line="76"/>
        <source>%1 authorization to %2</source>
        <translation>%1 в авторизации %2</translation>
    </message>
    <message>
        <location filename="../src/dialogs/authuserdlg.cpp" line="51"/>
        <location filename="../src/dialogs/authuserdlg.cpp" line="77"/>
        <source>Grant</source>
        <translation>Согласие</translation>
    </message>
    <message>
        <location filename="../src/dialogs/authuserdlg.cpp" line="50"/>
        <source>Licq - %1 Authorization</source>
        <translation>Licq - %1 в авторизации</translation>
    </message>
    <message>
        <location filename="../src/dialogs/authuserdlg.cpp" line="51"/>
        <location filename="../src/dialogs/authuserdlg.cpp" line="77"/>
        <source>Refuse</source>
        <translation>Отказ</translation>
    </message>
    <message>
        <location filename="../src/dialogs/authuserdlg.cpp" line="81"/>
        <source>Response</source>
        <translation>Ответ</translation>
    </message>
    <message>
        <location filename="../src/dialogs/authuserdlg.cpp" line="59"/>
        <source>User Id:</source>
        <translation>Идентификатор пользователя:</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::AwayMsgDlg</name>
    <message>
        <location filename="../src/dialogs/awaymsgdlg.cpp" line="279"/>
        <source>(Closing in %1)</source>
        <translation>(Закроется через %1с)</translation>
    </message>
    <message>
        <location filename="../src/dialogs/awaymsgdlg.cpp" line="217"/>
        <source>&lt;h2&gt;Hints for Setting&lt;br&gt;your Auto-Response&lt;/h2&gt;&lt;hr&gt;&lt;ul&gt;&lt;li&gt;You can include any of the % expansions (described in the main hints page).&lt;/li&gt;&lt;li&gt;Any line beginning with a pipe (|) will be treated as a command to be run.  The line will be replaced by the output of the command. The command is parsed by /bin/sh so any shell commands or meta-characters are allowed.  For security reasons, any % expansions are automatically passed to the command surrounded by single quotes to prevent shell parsing of any meta-characters included in an alias.&lt;br&gt;Examples of popular uses include:&lt;ul&gt;&lt;li&gt;&lt;tt&gt;|date&lt;/tt&gt;: Will replace that line by the current date&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|fortune&lt;/tt&gt;: Show a fortune, as a tagline for example&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|myscript.sh %u %a&lt;/tt&gt;: Run a script, passing the uin and alias&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|myscript.sh %u %a &gt; /dev/null&lt;/tt&gt;: Run the same script but ignore the output (for tracking auto response checks or something)&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|if [ %u -lt 100000 ]; then echo &quot;You are special&quot;; fi&lt;/tt&gt;: Useless, but shows how you can use shell script.&lt;/li&gt;&lt;/ul&gt;Of course, multiple &quot;|&quot; can appear in the auto response, and commands and regular text can be mixed line by line.&lt;/li&gt;&lt;hr&gt;&lt;p&gt; For more information, see the Licq webpage (&lt;tt&gt;http://www.licq.org&lt;/tt&gt;).&lt;/p&gt;</source>
        <translation>&lt;h2&gt;Подсказка по настройке&lt;br&gt;cообщения автоответа&lt;/h2&gt;&lt;hr&gt;&lt;ul&gt;&lt;li&gt;Вы можете использовать любую из %-переменных (описание на странице &quot;Подсказки&quot;).&lt;/li&gt;&lt;li&gt;Любая строка, начинающаяся с символа &quot;|&quot;, обрабатывается как команда, и заменяется на текст, полученный с ее стандартного потока вывода (stdout). В качестве интерпретатора команд используется &lt;b&gt;/bin/sh&lt;/b&gt;, допускается использование любых команд и метасимволов. Из соображений безопасности %-переменные заключаются в одинарные кавычки при передаче интерпретатору команд, во избежание обработки им метасимволов.&lt;br&gt;Примеры:&lt;ul&gt;&lt;li&gt;&lt;tt&gt;|date&lt;/tt&gt;: заменяется на текущую дату;&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|fortune&lt;/tt&gt;: отображает цитату программы fortune;&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|myscript.sh %u %a&lt;/tt&gt;: запускает скрипт и передает ему в качестве аргументов UIN и псевдоним;&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|myscript.sh %u %a &gt; /dev/null&lt;/tt&gt;: запускает скрипт без вывода чего-либо (для каких-нибудь своих целей);&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|if [ %u -lt 100000 ]; then echo &quot;Вы особенны&quot;; fi&lt;/tt&gt;: простой пример использования данной возможности.&lt;/li&gt;&lt;/ul&gt;Допускается так же и многократное использование &quot;|&quot; команд, при этом они будут обрабатываться построчно.&lt;/li&gt;&lt;hr&gt;&lt;p&gt;Более полную информацию можно найти на сайте Licq (&lt;tt&gt;http://www.licq.org&lt;/tt&gt;).&lt;/p&gt;</translation>
    </message>
    <message>
        <source>&amp;Edit Items</source>
        <translation type="obsolete">&amp;Редактировать сообщение</translation>
    </message>
    <message>
        <source>&amp;Hints</source>
        <translation type="obsolete">&amp;Подсказки</translation>
    </message>
    <message>
        <location filename="../src/dialogs/awaymsgdlg.cpp" line="159"/>
        <source>Set %1 Response for all accounts</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/dialogs/awaymsgdlg.cpp" line="196"/>
        <source>I&apos;m currently %1, %a.
You can leave me a message.
(%m messages pending from you).</source>
        <translation>Я сейчас в режиме &apos;%1&apos;, %a.
Вы можете оставить мне сообщение.
(%m не просмотренных сообщений от Вас).</translation>
    </message>
    <message>
        <location filename="../src/dialogs/awaymsgdlg.cpp" line="100"/>
        <source>&amp;Select</source>
        <translation>&amp;Выбрать</translation>
    </message>
    <message>
        <location filename="../src/dialogs/awaymsgdlg.cpp" line="99"/>
        <source>&amp;Hints...</source>
        <translation type="unfinished">&amp;Подсказки...</translation>
    </message>
    <message>
        <location filename="../src/dialogs/awaymsgdlg.cpp" line="150"/>
        <source>&amp;Edit Items...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/dialogs/awaymsgdlg.cpp" line="182"/>
        <source>Set %1 Response for %2</source>
        <translation>Установить авто-ответ %1 для %2</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::ChatDlg</name>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="793"/>
        <source>
&lt;--BEEP--&gt;
</source>
        <translation>
&lt;--BEEP--&gt;
</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="738"/>
        <source>%1 closed connection.</source>
        <translation>%1 закрыл соединение.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="1049"/>
        <source>/%1.chat</source>
        <translation>/%1.чат</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="794"/>
        <source>&gt; &lt;--BEEP--&gt;
</source>
        <translation>&gt; &lt;--BEEP--&gt;
</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="162"/>
        <source>&amp;Audio</source>
        <translation>&amp;Звук</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="208"/>
        <source>Background color</source>
        <translation>Цвет фона</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="195"/>
        <source>Beep</source>
        <translation>Сигнал</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="234"/>
        <source>Bold</source>
        <translation>Полужирный</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="209"/>
        <source>Changes the background color</source>
        <translation>Изменяет цвет фона</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="202"/>
        <source>Changes the foreground color</source>
        <translation>Изменяет цвет текста</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="161"/>
        <source>Chat</source>
        <translation>Чат</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="167"/>
        <source>&amp;Close Chat</source>
        <translation>&amp;Закрыть</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="987"/>
        <source>Do you want to save the chat session?</source>
        <translation>Желаете сохранить разговор?</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="1063"/>
        <source>Failed to open file:
%1</source>
        <translation>Ошибка при открытии файла:
%1</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="201"/>
        <source>Foreground color</source>
        <translation>Цвет текста</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="174"/>
        <source>&amp;IRC Mode</source>
        <translation>Режим &amp;IRC</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="190"/>
        <source>Ignore user settings</source>
        <translation>Игнорировать установки пользователя</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="191"/>
        <source>Ignores user color settings</source>
        <translation>Игнорировать цвета пользователя</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="239"/>
        <source>Italic</source>
        <translation>Наклонный</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="119"/>
        <source>Licq - Chat</source>
        <translation>Licq - Чат</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="1015"/>
        <source>Licq - Chat %1</source>
        <translation>Licq - Чат %1</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="374"/>
        <source>Local - %1</source>
        <translation>Локальный - %1</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="168"/>
        <source>Mode</source>
        <translation>Режим</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="170"/>
        <source>&amp;Pane Mode</source>
        <translation>&amp;Оконный режим</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="612"/>
        <source>Remote - Connecting...</source>
        <translation>Соединение - установка соединения...</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="126"/>
        <location filename="../src/dialogs/chatdlg.cpp" line="978"/>
        <source>Remote - Not connected</source>
        <translation>Соединение - Нет соединения</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="602"/>
        <source>Remote - Waiting for joiners...</source>
        <translation>Соединение - ожидаем участников...</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="165"/>
        <source>&amp;Save Chat</source>
        <translation>&amp;Сохранить</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="196"/>
        <source>Sends a Beep to all recipients</source>
        <translation>Послать сигнал всем участникам</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="320"/>
        <source>Set Encoding</source>
        <translation>Установить кодировку</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="249"/>
        <source>StrikeOut</source>
        <translation>Перечеркнутый</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="235"/>
        <source>Toggles Bold font</source>
        <translation>Включает/выключает полужирный шрифт</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="240"/>
        <source>Toggles Italic font</source>
        <translation>Включает/выключает наклонный шрифт</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="250"/>
        <source>Toggles StrikeOut font</source>
        <translation>Включает/выключает перечеркнутый шрифт</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="245"/>
        <source>Toggles Underline font</source>
        <translation>Включает/Выключает подчёркнутый шрифт</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="709"/>
        <source>Unable to bind to a port.
See Network Window for details.</source>
        <translation>Невозможно выполнить привязку к порту.
Подробности в окне отладки.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="717"/>
        <source>Unable to connect to the remote chat.
See Network Window for details.</source>
        <translation>Невозможно присоединиться к чату.
Подробности в окне отладки.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="725"/>
        <source>Unable to create new thread.
See Network Window for details.</source>
        <translation>Невозможно создать новую нить.
Подробности в окне отладки.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="1103"/>
        <source>Unable to load encoding &lt;b&gt;%1&lt;/b&gt;. Message contents may appear garbled.</source>
        <translation>Невозможно загрузить кодировку &lt;b&gt;%1&lt;/b&gt;. Отображение может быть некорректным.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/chatdlg.cpp" line="244"/>
        <source>Underline</source>
        <translation>Подчеркнутый</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::ContactBar</name>
    <message>
        <location filename="../src/contactlist/contactbar.cpp" line="46"/>
        <source>Not In List</source>
        <translation>Не в списке</translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactbar.cpp" line="42"/>
        <source>Offline</source>
        <translation>Отключен</translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactbar.cpp" line="39"/>
        <source>Online</source>
        <translation>В сети</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::ContactListModel</name>
    <message>
        <location filename="../src/contactlist/contactlist.cpp" line="51"/>
        <source>Online Notify</source>
        <translation type="unfinished">Оповещать о появлении</translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactlist.cpp" line="53"/>
        <source>Visible List</source>
        <translation type="unfinished">В списке видимости</translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactlist.cpp" line="55"/>
        <source>Invisible List</source>
        <translation type="unfinished">В списке невидимости</translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactlist.cpp" line="57"/>
        <source>Ignore List</source>
        <translation type="unfinished">В списке игнорируемых</translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactlist.cpp" line="59"/>
        <source>New Users</source>
        <translation type="unfinished">Новые пользователи</translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactlist.cpp" line="61"/>
        <source>Awaiting Authorization</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactlist.cpp" line="65"/>
        <source>All Users</source>
        <translation type="unfinished">Все пользователи</translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactlist.cpp" line="67"/>
        <source>All Groups (Threaded)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactlist.cpp" line="351"/>
        <source>Other Users</source>
        <translation>Другие пользователи</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::ContactUserData</name>
    <message>
        <location filename="../src/contactlist/contactuserdata.cpp" line="812"/>
        <source> day</source>
        <translation> день</translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactuserdata.cpp" line="812"/>
        <source> days</source>
        <translation> дней</translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactuserdata.cpp" line="818"/>
        <source> hour</source>
        <translation> час</translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactuserdata.cpp" line="818"/>
        <source> hours</source>
        <translation> часов</translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactuserdata.cpp" line="824"/>
        <source> minute</source>
        <translation> минута</translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactuserdata.cpp" line="824"/>
        <source> minutes</source>
        <translation> минут</translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactuserdata.cpp" line="805"/>
        <source> week</source>
        <translation> неделя</translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactuserdata.cpp" line="805"/>
        <source> weeks</source>
        <translation> недель</translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactuserdata.cpp" line="827"/>
        <source>0 minutes</source>
        <translation>0 минут</translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactuserdata.cpp" line="748"/>
        <source>Auto Response:</source>
        <translation>Авто-ответ:</translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactuserdata.cpp" line="715"/>
        <source>Birthday Today!</source>
        <translation>Сегодня День рождения!</translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactuserdata.cpp" line="763"/>
        <source>C: </source>
        <translation>С: </translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactuserdata.cpp" line="739"/>
        <source>Custom Auto Response</source>
        <translation>Персональный авто-ответ</translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactuserdata.cpp" line="756"/>
        <source>E: </source>
        <translation>Е: </translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactuserdata.cpp" line="769"/>
        <source>F: </source>
        <translation>Ф: </translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactuserdata.cpp" line="732"/>
        <source>File Server: Enabled</source>
        <translation>Файловый сервер: Включен</translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactuserdata.cpp" line="727"/>
        <source>ICQphone: Available</source>
        <translation>ICQphone: Доступен</translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactuserdata.cpp" line="729"/>
        <source>ICQphone: Busy</source>
        <translation>ICQphone: Занят</translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactuserdata.cpp" line="839"/>
        <source>ID: </source>
        <translation>Идентификатор: </translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactuserdata.cpp" line="833"/>
        <source>Idle: </source>
        <translation>Отсутствует: </translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactuserdata.cpp" line="776"/>
        <source>Ip: </source>
        <translation>Ip: </translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactuserdata.cpp" line="836"/>
        <source>Local time: </source>
        <translation>Текущее время: </translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactuserdata.cpp" line="829"/>
        <source>Logged In: </source>
        <translation>В сети: </translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactuserdata.cpp" line="788"/>
        <source>O: </source>
        <translation>Был: </translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactuserdata.cpp" line="760"/>
        <source>P: </source>
        <translation>Т: </translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactuserdata.cpp" line="722"/>
        <source>Phone &amp;quot;Follow Me&amp;quot;: Available</source>
        <translation>&amp;quot;Следуй за мной&amp;quot;: Доступен</translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactuserdata.cpp" line="724"/>
        <source>Phone &amp;quot;Follow Me&amp;quot;: Busy</source>
        <translation>&amp;quot;Следуй за мной&amp;quot;: Занят</translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactuserdata.cpp" line="736"/>
        <source>Secure connection</source>
        <translation>Защищенное соединение</translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactuserdata.cpp" line="720"/>
        <source>Typing a message</source>
        <translation>Набирает сообщение</translation>
    </message>
    <message>
        <location filename="../src/contactlist/contactuserdata.cpp" line="742"/>
        <source>Awaiting authorization</source>
        <translation>Ожидаем авторизации</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::CustomAutoRespDlg</name>
    <message>
        <location filename="../src/dialogs/customautorespdlg.cpp" line="64"/>
        <source>Clear</source>
        <translation>Очистить</translation>
    </message>
    <message>
        <location filename="../src/dialogs/customautorespdlg.cpp" line="68"/>
        <source>Hints</source>
        <translation>Подсказки</translation>
    </message>
    <message>
        <location filename="../src/dialogs/customautorespdlg.cpp" line="84"/>
        <source>I am currently %1.
You can leave me a message.</source>
        <translation>Я сейчас в режиме %1.
Вы можете оставить мне сообщение.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/customautorespdlg.cpp" line="77"/>
        <source>Set Custom Auto Response for %1</source>
        <translation>Установить персональный авто-ответ для %1</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::DockIcon</name>
    <message>
        <location filename="../src/dockicons/dockicon.cpp" line="142"/>
        <source>&lt;br&gt;Left click - Show main window&lt;br&gt;Middle click - Show next message&lt;br&gt;Right click - System menu</source>
        <translation>&lt;br&gt;Левая кнопка - основное окно&lt;br&gt;Средняя кнопка - следующее сообщение&lt;br&gt;Правая кнопка - системное меню</translation>
    </message>
    <message>
        <location filename="../src/dockicons/dockicon.cpp" line="135"/>
        <source>%1 system messages</source>
        <translation>%1 сист. сообщений</translation>
    </message>
    <message>
        <location filename="../src/dockicons/dockicon.cpp" line="138"/>
        <source>%1 msgs</source>
        <translation>%1 сообщ</translation>
    </message>
    <message>
        <location filename="../src/dockicons/dockicon.cpp" line="140"/>
        <source>1 msg</source>
        <translation>1 сообщ</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::EditCategoryDlg</name>
    <message>
        <location filename="../src/dialogs/editcategorydlg.cpp" line="61"/>
        <source>Organization, Affiliation, Group</source>
        <translation>Организация, Членство, Группа</translation>
    </message>
    <message>
        <location filename="../src/dialogs/editcategorydlg.cpp" line="67"/>
        <source>Past Background</source>
        <translation>Биография</translation>
    </message>
    <message>
        <location filename="../src/dialogs/editcategorydlg.cpp" line="55"/>
        <source>Personal Interests</source>
        <translation>Личные интересы</translation>
    </message>
    <message>
        <location filename="../src/dialogs/editcategorydlg.cpp" line="83"/>
        <source>Unspecified</source>
        <translation>Не указанно</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::EditFileDlg</name>
    <message>
        <location filename="../src/dialogs/editfiledlg.cpp" line="82"/>
        <location filename="../src/dialogs/editfiledlg.cpp" line="109"/>
        <source>Failed to open file:
%1</source>
        <translation>Ошибка при открытии файла:
%1</translation>
    </message>
    <message>
        <location filename="../src/dialogs/editfiledlg.cpp" line="100"/>
        <source>Licq File Editor - %1</source>
        <translation>Licq - Редактор файлов: %1</translation>
    </message>
    <message>
        <location filename="../src/dialogs/editfiledlg.cpp" line="65"/>
        <source>Revert</source>
        <translation>Восстановить</translation>
    </message>
    <message>
        <location filename="../src/dialogs/editfiledlg.cpp" line="127"/>
        <source>[ Read-Only ]</source>
        <translation>[ Только для чтения ]</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::EditFileListDlg</name>
    <message>
        <location filename="../src/dialogs/editfilelistdlg.cpp" line="58"/>
        <source>D&amp;elete</source>
        <translation>&amp;Удалить</translation>
    </message>
    <message>
        <location filename="../src/dialogs/editfilelistdlg.cpp" line="53"/>
        <source>D&amp;one</source>
        <translation>&amp;Готово</translation>
    </message>
    <message>
        <location filename="../src/dialogs/editfilelistdlg.cpp" line="57"/>
        <source>&amp;Down</source>
        <translation>&amp;Вниз</translation>
    </message>
    <message>
        <location filename="../src/dialogs/editfilelistdlg.cpp" line="41"/>
        <source>Licq - Files to send</source>
        <translation>Licq - Файлы для отправки</translation>
    </message>
    <message>
        <location filename="../src/dialogs/editfilelistdlg.cpp" line="56"/>
        <source>&amp;Up</source>
        <translation>&amp;Вверх</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::EditGrpDlg</name>
    <message>
        <location filename="../src/dialogs/editgrpdlg.cpp" line="51"/>
        <source>Licq - Edit Groups</source>
        <translation>Licq - Редактирование групп</translation>
    </message>
    <message>
        <location filename="../src/dialogs/editgrpdlg.cpp" line="84"/>
        <source>&amp;Save</source>
        <translation>&amp;Сохранить</translation>
    </message>
    <message>
        <location filename="../src/dialogs/editgrpdlg.cpp" line="54"/>
        <source>Groups</source>
        <translation>Группы</translation>
    </message>
    <message>
        <location filename="../src/dialogs/editgrpdlg.cpp" line="67"/>
        <source>Add</source>
        <translation>Добавить</translation>
    </message>
    <message>
        <location filename="../src/dialogs/editgrpdlg.cpp" line="68"/>
        <source>Remove</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <location filename="../src/dialogs/editgrpdlg.cpp" line="69"/>
        <source>Shift Up</source>
        <translation>Вверх</translation>
    </message>
    <message>
        <location filename="../src/dialogs/editgrpdlg.cpp" line="70"/>
        <source>Shift Down</source>
        <translation>Вниз</translation>
    </message>
    <message>
        <location filename="../src/dialogs/editgrpdlg.cpp" line="71"/>
        <location filename="../src/dialogs/editgrpdlg.cpp" line="255"/>
        <location filename="../src/dialogs/editgrpdlg.cpp" line="268"/>
        <source>Edit Name</source>
        <translation>Изменить имя</translation>
    </message>
    <message>
        <location filename="../src/dialogs/editgrpdlg.cpp" line="75"/>
        <source>Edit group name (hit enter to save).</source>
        <translation>Редактировать название группы (нажмите Enter, чтобы сохранить).</translation>
    </message>
    <message>
        <location filename="../src/dialogs/editgrpdlg.cpp" line="86"/>
        <source>Save the name of a group being modified.</source>
        <translation>Сохранить название редактируемой группы.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/editgrpdlg.cpp" line="93"/>
        <source>&amp;Done</source>
        <translation>&amp;Завершить</translation>
    </message>
    <message>
        <location filename="../src/dialogs/editgrpdlg.cpp" line="167"/>
        <source>noname</source>
        <translation>noname</translation>
    </message>
    <message>
        <location filename="../src/dialogs/editgrpdlg.cpp" line="183"/>
        <source>Are you sure you want to remove
the group &apos;%1&apos;?</source>
        <translation>Вы уверены, что хотитеудалить группу&apos;%1&apos;?</translation>
    </message>
    <message>
        <location filename="../src/dialogs/editgrpdlg.cpp" line="170"/>
        <location filename="../src/dialogs/editgrpdlg.cpp" line="237"/>
        <source>Cancel</source>
        <translation>Отмена</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::EditPhoneDlg</name>
    <message>
        <location filename="../src/dialogs/phonedlg.cpp" line="187"/>
        <location filename="../src/dialogs/phonedlg.cpp" line="197"/>
        <location filename="../src/dialogs/phonedlg.cpp" line="223"/>
        <source>@</source>
        <translation>@</translation>
    </message>
    <message>
        <location filename="../src/dialogs/phonedlg.cpp" line="158"/>
        <source>&amp;Cancel</source>
        <translation>О&amp;тмена</translation>
    </message>
    <message>
        <location filename="../src/dialogs/phonedlg.cpp" line="93"/>
        <source>Cellular</source>
        <translation>Сотовый</translation>
    </message>
    <message>
        <location filename="../src/dialogs/phonedlg.cpp" line="94"/>
        <source>Cellular SMS</source>
        <translation>Сотовый СМС</translation>
    </message>
    <message>
        <location filename="../src/dialogs/phonedlg.cpp" line="100"/>
        <source>Country:</source>
        <translation>Страна:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/phonedlg.cpp" line="133"/>
        <source>Custom</source>
        <translation>Другой</translation>
    </message>
    <message>
        <location filename="../src/dialogs/phonedlg.cpp" line="71"/>
        <source>Description:</source>
        <translation>Описание:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/phonedlg.cpp" line="139"/>
        <source>E-mail Gateway:</source>
        <translation>Почтовый шлюз:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/phonedlg.cpp" line="123"/>
        <source>Extension:</source>
        <translation>Донабор:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/phonedlg.cpp" line="95"/>
        <source>Fax</source>
        <translation>Факс</translation>
    </message>
    <message>
        <location filename="../src/dialogs/phonedlg.cpp" line="79"/>
        <source>Home Fax</source>
        <translation>Домашний факс</translation>
    </message>
    <message>
        <location filename="../src/dialogs/phonedlg.cpp" line="75"/>
        <source>Home Phone</source>
        <translation>Домашний</translation>
    </message>
    <message>
        <location filename="../src/dialogs/phonedlg.cpp" line="111"/>
        <source>Network #/Area code:</source>
        <translation>Код сети/местности:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/phonedlg.cpp" line="117"/>
        <source>Number:</source>
        <translation>Номер:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/phonedlg.cpp" line="154"/>
        <source>&amp;OK</source>
        <translation>&amp;Ок</translation>
    </message>
    <message>
        <location filename="../src/dialogs/phonedlg.cpp" line="96"/>
        <source>Pager</source>
        <translation>Пейджер</translation>
    </message>
    <message>
        <location filename="../src/dialogs/phonedlg.cpp" line="92"/>
        <source>Phone</source>
        <translation>Телефон</translation>
    </message>
    <message>
        <location filename="../src/dialogs/phonedlg.cpp" line="235"/>
        <source>Please enter a phone number</source>
        <translation>Пожалуйста, введите номер</translation>
    </message>
    <message>
        <location filename="../src/dialogs/phonedlg.cpp" line="77"/>
        <source>Private Cellular</source>
        <translation>Личный сотовый</translation>
    </message>
    <message>
        <location filename="../src/dialogs/phonedlg.cpp" line="130"/>
        <source>Provider:</source>
        <translation>Оператор:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/phonedlg.cpp" line="146"/>
        <source>Remove leading 0s from Area Code/Network #</source>
        <translation>Удалять нули в начале кода сети/местности</translation>
    </message>
    <message>
        <location filename="../src/dialogs/phonedlg.cpp" line="87"/>
        <source>Type:</source>
        <translation>Тип:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/phonedlg.cpp" line="81"/>
        <source>Wireless Pager</source>
        <translation>Пейджер</translation>
    </message>
    <message>
        <location filename="../src/dialogs/phonedlg.cpp" line="78"/>
        <source>Work Cellular</source>
        <translation>Рабочий сотовый</translation>
    </message>
    <message>
        <location filename="../src/dialogs/phonedlg.cpp" line="80"/>
        <source>Work Fax</source>
        <translation>Рабочий факс</translation>
    </message>
    <message>
        <location filename="../src/dialogs/phonedlg.cpp" line="76"/>
        <source>Work Phone</source>
        <translation>Рабочий</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::Emoticons</name>
    <message>
        <location filename="../src/config/emoticons.cpp" line="50"/>
        <source>Default</source>
        <translation>Стандартные</translation>
    </message>
    <message>
        <location filename="../src/config/emoticons.cpp" line="52"/>
        <source>None</source>
        <translation>Отключить</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::EventDescription</name>
    <message>
        <location filename="../src/helpers/eventdesc.cpp" line="77"/>
        <source>(cancelled)</source>
        <translation>(прервано)</translation>
    </message>
    <message>
        <location filename="../src/helpers/eventdesc.cpp" line="46"/>
        <source>Added to Contact List</source>
        <translation>Добавлен в список контактов</translation>
    </message>
    <message>
        <location filename="../src/helpers/eventdesc.cpp" line="42"/>
        <source>Authorization Granted</source>
        <translation>Авторизация подтверждена</translation>
    </message>
    <message>
        <location filename="../src/helpers/eventdesc.cpp" line="41"/>
        <source>Authorization Refused</source>
        <translation>Отказ в авторизации</translation>
    </message>
    <message>
        <location filename="../src/helpers/eventdesc.cpp" line="40"/>
        <source>Authorization Request</source>
        <translation>Запрос авторизации</translation>
    </message>
    <message>
        <location filename="../src/helpers/eventdesc.cpp" line="36"/>
        <source>Chat Request</source>
        <translation>Запрос чата</translation>
    </message>
    <message>
        <location filename="../src/helpers/eventdesc.cpp" line="53"/>
        <source>Contact List</source>
        <translation>Список контактов</translation>
    </message>
    <message>
        <location filename="../src/helpers/eventdesc.cpp" line="48"/>
        <source>Email Pager</source>
        <translation>Email-пейджер</translation>
    </message>
    <message>
        <location filename="../src/helpers/eventdesc.cpp" line="37"/>
        <source>File Transfer</source>
        <translation>Передача файла</translation>
    </message>
    <message>
        <location filename="../src/helpers/eventdesc.cpp" line="35"/>
        <source>Message</source>
        <translation>Сообщение</translation>
    </message>
    <message>
        <location filename="../src/helpers/eventdesc.cpp" line="34"/>
        <source>Plugin Event</source>
        <translation>Событие модуля</translation>
    </message>
    <message>
        <location filename="../src/helpers/eventdesc.cpp" line="60"/>
        <source>SMS</source>
        <translation>SMS</translation>
    </message>
    <message>
        <location filename="../src/helpers/eventdesc.cpp" line="43"/>
        <source>System Server Message</source>
        <translation>Системное сообщение сервера</translation>
    </message>
    <message>
        <location filename="../src/helpers/eventdesc.cpp" line="38"/>
        <source>URL</source>
        <translation>Ссылка</translation>
    </message>
    <message>
        <location filename="../src/helpers/eventdesc.cpp" line="72"/>
        <source>Unknown Event</source>
        <translation>Неизвестное событие</translation>
    </message>
    <message>
        <location filename="../src/helpers/eventdesc.cpp" line="47"/>
        <source>Web Panel</source>
        <translation>Web-панель</translation>
    </message>
    <message>
        <location filename="../src/helpers/eventdesc.cpp" line="68"/>
        <source>New Email Alert</source>
        <translation>Извещение о новой почте</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::FileDlg</name>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="238"/>
        <source>%1/%2</source>
        <translation>%1/%2</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="97"/>
        <source>Batch:</source>
        <translation>Всего:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="389"/>
        <source>Bind error.
</source>
        <translation>Ошибка привязки.
</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="443"/>
        <source>Byte</source>
        <translation>Байт</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="439"/>
        <source>Bytes</source>
        <translation>Байт</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="481"/>
        <location filename="../src/dialogs/filedlg.cpp" line="505"/>
        <source>Cancel</source>
        <translation>Отмена</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="133"/>
        <source>&amp;Cancel Transfer</source>
        <translation>&amp;Отменить пересылку</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="180"/>
        <location filename="../src/dialogs/filedlg.cpp" line="350"/>
        <location filename="../src/dialogs/filedlg.cpp" line="359"/>
        <location filename="../src/dialogs/filedlg.cpp" line="369"/>
        <location filename="../src/dialogs/filedlg.cpp" line="378"/>
        <location filename="../src/dialogs/filedlg.cpp" line="388"/>
        <location filename="../src/dialogs/filedlg.cpp" line="398"/>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="418"/>
        <source>Connecting to remote...</source>
        <translation>Устанавливается соединение...</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="379"/>
        <source>Connection error.
</source>
        <translation>Ошибка соединения.
</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="72"/>
        <source>Current:</source>
        <translation>Текущий:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="114"/>
        <source>ETA:</source>
        <translation>Завершение:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="362"/>
        <source>File I/O Error:
%1

See Network Window for details.</source>
        <translation>Ошибка ввода/вывода:
%1

Подробности в окне отладки.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="360"/>
        <source>File I/O error: %1.</source>
        <translation>Ошибка ввода/вывода: %1.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="488"/>
        <source>File already exists and appears incomplete.</source>
        <translation>Файл уже есть и, похоже, неполный.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="481"/>
        <source>File already exists and is at least as big as the incoming file.</source>
        <translation>Файл уже есть и, по крайней мере, не меньше передаваемого.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="81"/>
        <source>File name:</source>
        <translation>Имя файла:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="179"/>
        <source>File transfer cancelled.</source>
        <translation>Пересылка файлов отменена.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="336"/>
        <source>File transfer complete.</source>
        <translation>Пересылка файлов завершена.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="89"/>
        <source>File:</source>
        <translation>Файл:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="372"/>
        <source>Handshake Error.
See Network Window for details.</source>
        <translation>Ошибка соединения на стадии приветствия.
Подробности в окне отладки.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="370"/>
        <source>Handshaking error.
</source>
        <translation>Ошибка соединения на стадии приветствия.
</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="435"/>
        <source>KB</source>
        <translation>КБ</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="66"/>
        <location filename="../src/dialogs/filedlg.cpp" line="289"/>
        <source>Licq - File Transfer (%1)</source>
        <translation>Licq - Пересылка файлов (%1)</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="431"/>
        <source>MB</source>
        <translation>МБ</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="399"/>
        <source>Not enough resources.
</source>
        <translation>Недостаточно ресурсов.
</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="337"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="145"/>
        <source>&amp;Open</source>
        <translation>&amp;Открыть</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="146"/>
        <source>O&amp;pen Dir</source>
        <translation>О&amp;ткрыть каталог</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="505"/>
        <source>Open error - unable to open file for writing.</source>
        <translation>Ошибка открытия файла - невозможно открыть файл для записи.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="481"/>
        <location filename="../src/dialogs/filedlg.cpp" line="488"/>
        <source>Overwrite</source>
        <translation>Записать поверх</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="328"/>
        <source>Received %1 from %2 successfully.</source>
        <translation>Файл %1 успешно принят от %2.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="312"/>
        <source>Receiving file...</source>
        <translation>Принимается файл...</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="351"/>
        <location filename="../src/dialogs/filedlg.cpp" line="353"/>
        <source>Remote side disconnected.</source>
        <translation>Удаленный компьютер прервал соединение.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="488"/>
        <source>Resume</source>
        <translation>Продолжить</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="505"/>
        <source>Retry</source>
        <translation>Повторить</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="314"/>
        <source>Sending file...</source>
        <translation>Отправляется файл...</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="330"/>
        <source>Sent %1 to %2 successfully.</source>
        <translation>Файл %1 успешно отправлен %2.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="104"/>
        <source>Time:</source>
        <translation>Время:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="392"/>
        <source>Unable to bind to a port.
See Network Window for details.</source>
        <translation>Невозможно сделать привязку к порту.Подробности в окне отладки.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="402"/>
        <source>Unable to create a thread.
See Network Window for details.</source>
        <translation>Невозможно создать нить.
Подробности в окне отладки.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="382"/>
        <source>Unable to reach remote host.
See Network Window for details.</source>
        <translation>Удаленная машина недостижима.
Подробности в окне отладки.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/filedlg.cpp" line="267"/>
        <source>Waiting for connection...</source>
        <translation>Ожидание соединения...</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::FileNameEdit</name>
    <message>
        <location filename="../src/widgets/filenameedit.cpp" line="53"/>
        <source>Browse...</source>
        <translation>Обзор...</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::FloatyView</name>
    <message>
        <location filename="../src/views/floatyview.cpp" line="56"/>
        <source>%1 Floaty (%2)</source>
        <translation>%1 плавающее окно (%2)</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::FontEdit</name>
    <message>
        <location filename="../src/widgets/fontedit.cpp" line="51"/>
        <source>Choose...</source>
        <translation>Выбор...</translation>
    </message>
    <message>
        <location filename="../src/widgets/fontedit.cpp" line="52"/>
        <source>Select a font from the system list.</source>
        <translation>Выбрать системный шрифт.</translation>
    </message>
    <message>
        <location filename="../src/widgets/fontedit.cpp" line="62"/>
        <source>default (%1)</source>
        <translation>по умолчанию (%1)</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::ForwardDlg</name>
    <message>
        <location filename="../src/dialogs/forwarddlg.cpp" line="96"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Отмена</translation>
    </message>
    <message>
        <location filename="../src/dialogs/forwarddlg.cpp" line="85"/>
        <source>Drag the user to forward to here:</source>
        <translation>Перетащите пользователя, которому хотите переслать сообщение, сюда:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/forwarddlg.cpp" line="92"/>
        <source>&amp;Forward</source>
        <translation>&amp;Переслать</translation>
    </message>
    <message>
        <location filename="../src/dialogs/forwarddlg.cpp" line="81"/>
        <source>Forward %1 To User</source>
        <translation>Переслать %1 пользователю</translation>
    </message>
    <message>
        <location filename="../src/dialogs/forwarddlg.cpp" line="127"/>
        <source>Forwarded URL:
</source>
        <translation>Пересылаемая ссылка:
</translation>
    </message>
    <message>
        <location filename="../src/dialogs/forwarddlg.cpp" line="119"/>
        <source>Forwarded message:
</source>
        <translation>Пересылаемое сообщение:
</translation>
    </message>
    <message>
        <location filename="../src/dialogs/forwarddlg.cpp" line="67"/>
        <source>Message</source>
        <translation>Сообщение</translation>
    </message>
    <message>
        <location filename="../src/dialogs/forwarddlg.cpp" line="71"/>
        <source>URL</source>
        <translation>Ссылка</translation>
    </message>
    <message>
        <location filename="../src/dialogs/forwarddlg.cpp" line="76"/>
        <source>Unable to forward this message type (%d).</source>
        <translation>Невозможно переслать сообщение этого типа (%d).</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::GPGKeyManager</name>
    <message>
        <location filename="../src/dialogs/gpgkeymanager.cpp" line="72"/>
        <source>Active</source>
        <translation>Активно</translation>
    </message>
    <message>
        <location filename="../src/dialogs/gpgkeymanager.cpp" line="86"/>
        <source>&amp;Add</source>
        <translation>&amp;Добавить</translation>
    </message>
    <message>
        <source>&amp;Edit</source>
        <translation type="obsolete">&amp;Редактировать</translation>
    </message>
    <message>
        <location filename="../src/dialogs/gpgkeymanager.cpp" line="72"/>
        <source>Key ID</source>
        <translation>Идентификатор ключа</translation>
    </message>
    <message>
        <location filename="../src/dialogs/gpgkeymanager.cpp" line="94"/>
        <source>&amp;Remove</source>
        <translation>&amp;Удалить</translation>
    </message>
    <message>
        <location filename="../src/dialogs/gpgkeymanager.cpp" line="72"/>
        <source>User</source>
        <translation>Пользователь</translation>
    </message>
    <message>
        <location filename="../src/dialogs/gpgkeymanager.cpp" line="65"/>
        <source>Licq - GPG Key Manager</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/dialogs/gpgkeymanager.cpp" line="93"/>
        <source>&amp;Edit...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/dialogs/gpgkeymanager.cpp" line="155"/>
        <source>Do you want to remove the GPG key binding for the user %1?
The key isn&apos;t deleted from your keyring.</source>
        <translation>Вы хотите удалить привязку GPG ключа к пользователю %1?
Ключ не будет удален из Вашего набора ключей.</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::GPGKeySelect</name>
    <message>
        <location filename="../src/dialogs/gpgkeyselect.cpp" line="75"/>
        <source>Current key: %1</source>
        <translation>Текущий ключ: %1</translation>
    </message>
    <message>
        <location filename="../src/dialogs/gpgkeyselect.cpp" line="73"/>
        <source>Current key: No key selected</source>
        <translation>Текущий ключ: не задан</translation>
    </message>
    <message>
        <location filename="../src/dialogs/gpgkeyselect.cpp" line="85"/>
        <source>Filter:</source>
        <translation>Фильтр:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/gpgkeyselect.cpp" line="105"/>
        <source>&amp;No Key</source>
        <translation>&amp;Без ключа</translation>
    </message>
    <message>
        <location filename="../src/dialogs/gpgkeyselect.cpp" line="67"/>
        <source>Select GPG Key for user %1</source>
        <translation>Выбор GPG ключа для пользователя %1</translation>
    </message>
    <message>
        <location filename="../src/dialogs/gpgkeyselect.cpp" line="70"/>
        <source>Select a GPG key for user %1.</source>
        <translation>Выберите GPG ключ для пользователя %1.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/gpgkeyselect.cpp" line="78"/>
        <source>Use GPG Encryption</source>
        <translation>Использовать GPG шифрование</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::GroupComboBox</name>
    <message>
        <location filename="../src/widgets/groupcombobox.cpp" line="35"/>
        <source>First</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/widgets/groupcombobox.cpp" line="43"/>
        <source>After </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>LicqQtGui::GroupDlg</name>
    <message>
        <location filename="../src/dialogs/groupdlg.cpp" line="48"/>
        <source>Licq - Group </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>LicqQtGui::GroupMenu</name>
    <message>
        <location filename="../src/core/groupmenu.cpp" line="48"/>
        <source>Add Users to</source>
        <translation>Добавить пользователей в</translation>
    </message>
    <message>
        <location filename="../src/core/groupmenu.cpp" line="67"/>
        <source>Move &amp;Up</source>
        <translation>Переместить &amp;вверх</translation>
    </message>
    <message>
        <location filename="../src/core/groupmenu.cpp" line="68"/>
        <source>Move &amp;Down</source>
        <translation>Переместить в&amp;низ</translation>
    </message>
    <message>
        <location filename="../src/core/groupmenu.cpp" line="69"/>
        <source>Rename</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/groupmenu.cpp" line="70"/>
        <source>Sounds...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/groupmenu.cpp" line="72"/>
        <source>Remove Group</source>
        <translation>Удалить группу</translation>
    </message>
    <message>
        <location filename="../src/core/groupmenu.cpp" line="182"/>
        <source>Are you sure you want to remove the group &apos;%1&apos;?</source>
        <translation>Вы уверены, что хотите удалить группу &apos;%1&apos;?</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::HintsDlg</name>
    <message>
        <location filename="../src/dialogs/hintsdlg.cpp" line="39"/>
        <source>Licq - Hints</source>
        <translation>Licq - Подсказки</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::HistoryDlg</name>
    <message>
        <location filename="../src/dialogs/historydlg.cpp" line="86"/>
        <source>&amp;Previous day</source>
        <translation>&amp;Пред. день</translation>
    </message>
    <message>
        <location filename="../src/dialogs/historydlg.cpp" line="90"/>
        <source>&amp;Next day</source>
        <translation>&amp;След. день</translation>
    </message>
    <message>
        <location filename="../src/dialogs/historydlg.cpp" line="101"/>
        <source>Search</source>
        <translation>Поиск</translation>
    </message>
    <message>
        <location filename="../src/dialogs/historydlg.cpp" line="108"/>
        <source>Find:</source>
        <translation>Искать:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/historydlg.cpp" line="119"/>
        <source>Match &amp;case</source>
        <translation>С учетом &amp;регистра</translation>
    </message>
    <message>
        <location filename="../src/dialogs/historydlg.cpp" line="121"/>
        <source>&amp;Regular expression</source>
        <translation>Регулярные &amp;выражения</translation>
    </message>
    <message>
        <location filename="../src/dialogs/historydlg.cpp" line="126"/>
        <source>F&amp;ind previous</source>
        <translation>&amp;Назад</translation>
    </message>
    <message>
        <location filename="../src/dialogs/historydlg.cpp" line="131"/>
        <source>&amp;Find next</source>
        <translation>&amp;Вперед</translation>
    </message>
    <message>
        <location filename="../src/dialogs/historydlg.cpp" line="155"/>
        <source>&amp;Menu</source>
        <translation>&amp;Меню</translation>
    </message>
    <message>
        <location filename="../src/dialogs/historydlg.cpp" line="169"/>
        <source>INVALID USER</source>
        <translation>НЕПРАВИЛЬНЫЙ ПОЛЬЗОВАТЕЛЬ</translation>
    </message>
    <message>
        <location filename="../src/dialogs/historydlg.cpp" line="181"/>
        <source>Licq - History </source>
        <translation>Licq - История </translation>
    </message>
    <message>
        <location filename="../src/dialogs/historydlg.cpp" line="187"/>
        <source>Invalid user requested</source>
        <translation>Запрос по неправильному пользователю</translation>
    </message>
    <message>
        <location filename="../src/dialogs/historydlg.cpp" line="193"/>
        <source>Error loading history file: %1
Description: %2</source>
        <translation>Ошибка загрузки файла истории: %1
Причина: %2</translation>
    </message>
    <message>
        <location filename="../src/dialogs/historydlg.cpp" line="196"/>
        <source>Sorry, history is disabled for this person</source>
        <translation>Извините, история данного пользователя не ведется</translation>
    </message>
    <message>
        <location filename="../src/dialogs/historydlg.cpp" line="201"/>
        <source>History is empty</source>
        <translation>История пуста</translation>
    </message>
    <message>
        <location filename="../src/dialogs/historydlg.cpp" line="220"/>
        <source>server</source>
        <translation>сервер</translation>
    </message>
    <message>
        <location filename="../src/dialogs/historydlg.cpp" line="476"/>
        <source>Search returned no matches</source>
        <translation>Нет совпадений</translation>
    </message>
    <message>
        <location filename="../src/dialogs/historydlg.cpp" line="483"/>
        <source>Search wrapped around</source>
        <translation>Поиск по кругу</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::HistoryView</name>
    <message>
        <location filename="../src/widgets/historyview.cpp" line="45"/>
        <source>Compact</source>
        <translation>Компактный</translation>
    </message>
    <message>
        <location filename="../src/widgets/historyview.cpp" line="44"/>
        <source>Default</source>
        <translation>Стандартный</translation>
    </message>
    <message>
        <location filename="../src/widgets/historyview.cpp" line="48"/>
        <source>Long</source>
        <translation>Длинный</translation>
    </message>
    <message>
        <location filename="../src/widgets/historyview.cpp" line="47"/>
        <source>Table</source>
        <translation>Табличный</translation>
    </message>
    <message>
        <location filename="../src/widgets/historyview.cpp" line="46"/>
        <source>Tiny</source>
        <translation>Крохотный</translation>
    </message>
    <message>
        <location filename="../src/widgets/historyview.cpp" line="49"/>
        <source>Wide</source>
        <translation>Широкий</translation>
    </message>
    <message>
        <location filename="../src/widgets/historyview.cpp" line="356"/>
        <source>from</source>
        <translation>от</translation>
    </message>
    <message>
        <location filename="../src/widgets/historyview.cpp" line="356"/>
        <source>to</source>
        <translation>к</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::InfoField</name>
    <message>
        <location filename="../src/widgets/infofield.cpp" line="62"/>
        <source>Unknown</source>
        <translation>Неизвестно</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::JoinChatDlg</name>
    <message>
        <location filename="../src/dialogs/joinchatdlg.cpp" line="63"/>
        <location filename="../src/dialogs/joinchatdlg.cpp" line="70"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Отмена</translation>
    </message>
    <message>
        <location filename="../src/dialogs/joinchatdlg.cpp" line="62"/>
        <source>&amp;Invite</source>
        <translation>&amp;Пригласить</translation>
    </message>
    <message>
        <location filename="../src/dialogs/joinchatdlg.cpp" line="61"/>
        <source>Invite to Join Chat</source>
        <translation>Пригласить в чат</translation>
    </message>
    <message>
        <location filename="../src/dialogs/joinchatdlg.cpp" line="69"/>
        <source>&amp;Join</source>
        <translation>П&amp;рисоединиться</translation>
    </message>
    <message>
        <location filename="../src/dialogs/joinchatdlg.cpp" line="68"/>
        <source>Join Multiparty Chat</source>
        <translation>Присоединиться к чату</translation>
    </message>
    <message>
        <location filename="../src/dialogs/joinchatdlg.cpp" line="60"/>
        <source>Select chat to invite:</source>
        <translation>Выберите чат для приглашения:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/joinchatdlg.cpp" line="67"/>
        <source>Select chat to join:</source>
        <translation>Выберите чат:</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::KeyListItem</name>
    <message>
        <location filename="../src/dialogs/gpgkeymanager.cpp" line="295"/>
        <source>No</source>
        <translation>Нет</translation>
    </message>
    <message>
        <location filename="../src/dialogs/gpgkeymanager.cpp" line="295"/>
        <source>Yes</source>
        <translation>Да</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::KeyRequestDlg</name>
    <message>
        <location filename="../src/dialogs/keyrequestdlg.cpp" line="114"/>
        <source>Client does not support OpenSSL.
Rebuild Licq with OpenSSL support.</source>
        <translation>Ваш клиент не поддерживает OpenSSL.
Пересоберите Licq с поддержкой OpenSSL.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/keyrequestdlg.cpp" line="144"/>
        <source>Closing secure channel...</source>
        <translation>Закрываем защищенное соединение...</translation>
    </message>
    <message>
        <location filename="../src/dialogs/keyrequestdlg.cpp" line="55"/>
        <source>Licq - Secure Channel with %1</source>
        <translation>Licq - Защищенное соединение с %1</translation>
    </message>
    <message>
        <location filename="../src/dialogs/keyrequestdlg.cpp" line="108"/>
        <source>Ready to close channel</source>
        <translation>Готов закрыть соединение</translation>
    </message>
    <message>
        <location filename="../src/dialogs/keyrequestdlg.cpp" line="110"/>
        <source>Ready to request channel</source>
        <translation>Готов запросить соединение</translation>
    </message>
    <message>
        <location filename="../src/dialogs/keyrequestdlg.cpp" line="139"/>
        <source>Requesting secure channel...</source>
        <translation>Запрашиваем защищенное соединение...</translation>
    </message>
    <message>
        <location filename="../src/dialogs/keyrequestdlg.cpp" line="60"/>
        <source>Secure channel is established using SSL
with Diffie-Hellman key exchange and
the TLS version 1 protocol.

</source>
        <translation>Защищенное соединение будет установлено
при помощи SSL с использованием алгоритма
Диффи-Хеллмана для обмена ключами
и протокола TLS версии 1.
</translation>
    </message>
    <message>
        <location filename="../src/dialogs/keyrequestdlg.cpp" line="97"/>
        <source>&amp;Send</source>
        <translation>&amp;Отправить</translation>
    </message>
    <message>
        <location filename="../src/dialogs/keyrequestdlg.cpp" line="74"/>
        <source>The remote uses Licq %1, however it
has no secure channel support compiled in.
This probably won&apos;t work.</source>
        <translation>Партнер использует Licq %1, но без
поддежки защищенного соединения.
Скорее всего запрос будет отвергнут.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/keyrequestdlg.cpp" line="67"/>
        <source>The remote uses Licq %1/SSL.</source>
        <translation>Партнер использует Licq %1/SSL.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/keyrequestdlg.cpp" line="81"/>
        <source>This only works with other Licq clients &gt;= v0.85
The remote doesn&apos;t seem to use such a client.
This might not work.</source>
        <translation>Это работает только с другими Licq клиентами 
версии старше, чем 0.85. Не похоже, чтобы
партнер использовал подобный клиент.
Скорее всего запрос будет отвергнут.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/keyrequestdlg.cpp" line="170"/>
        <source>Secure channel already established.</source>
        <translation>Защищенное соединение уже установлено.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/keyrequestdlg.cpp" line="172"/>
        <source>Secure channel not established.</source>
        <translation>Защищенное соединение не установлено.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/keyrequestdlg.cpp" line="182"/>
        <source>Remote client does not support OpenSSL.</source>
        <translation>Клиент партнера не поддерживает OpenSSL.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/keyrequestdlg.cpp" line="185"/>
        <source>Could not connect to remote client.</source>
        <translation>Не удалось соединиться с партнером.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/keyrequestdlg.cpp" line="191"/>
        <source>Secure channel established.</source>
        <translation>Защищенное соединение установлено.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/keyrequestdlg.cpp" line="196"/>
        <source>Secure channel closed.</source>
        <translation>Защищенное соединение закрыто.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/keyrequestdlg.cpp" line="200"/>
        <source>Unknown state.</source>
        <translation>Неизвестное состояние.</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::KeyView</name>
    <message>
        <location filename="../src/dialogs/gpgkeyselect.cpp" line="189"/>
        <source>EMail</source>
        <translation>EMail</translation>
    </message>
    <message>
        <location filename="../src/dialogs/gpgkeyselect.cpp" line="189"/>
        <source>ID</source>
        <translation>Идентификатор</translation>
    </message>
    <message>
        <location filename="../src/dialogs/gpgkeyselect.cpp" line="189"/>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::LicqGui</name>
    <message>
        <location filename="../src/core/licqgui.cpp" line="650"/>
        <source>Are you sure you want to remove
%1 (%2)
from your contact list?</source>
        <translation>Вы уверены, что хотите удалить
%1 (%2)
из списка контактов?</translation>
    </message>
    <message>
        <location filename="../src/core/licqgui.cpp" line="274"/>
        <source>There was an error loading the default configuration file.
Would you like to try loading the old one?</source>
        <translation>Произошла ошибка при попытке загрузить файл конфигурации.
Загрузить старый?</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::LogWindow</name>
    <message>
        <location filename="../src/dialogs/logwindow.cpp" line="63"/>
        <source>Licq - Network Log</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/dialogs/logwindow.cpp" line="86"/>
        <source>Save...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/dialogs/logwindow.cpp" line="153"/>
        <location filename="../src/dialogs/logwindow.cpp" line="156"/>
        <source>Licq - Save Network Log</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/dialogs/logwindow.cpp" line="166"/>
        <source>Failed to open file:
%1</source>
        <translation>Ошибка при открытии файла:
%1</translation>
    </message>
    <message>
        <location filename="../src/dialogs/logwindow.cpp" line="91"/>
        <source>Clear</source>
        <translation>Очистить</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::MLEdit</name>
    <message>
        <location filename="../src/widgets/mledit.cpp" line="287"/>
        <source>Allow Tabulations</source>
        <translation>Разрешить табуляцию</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::MLView</name>
    <message>
        <location filename="../src/widgets/mlview.cpp" line="293"/>
        <source>Copy URL</source>
        <translation>Скопировать ссылку</translation>
    </message>
    <message>
        <location filename="../src/widgets/mlview.cpp" line="295"/>
        <source>Quote</source>
        <translation>Цитировать</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::MMSendDlg</name>
    <message>
        <location filename="../src/dialogs/mmsenddlg.cpp" line="143"/>
        <source>&amp;Close</source>
        <translation>&amp;Закрыть</translation>
    </message>
    <message>
        <location filename="../src/dialogs/mmsenddlg.cpp" line="123"/>
        <source>Multiple Recipient Contact List</source>
        <translation>Список контактов нескольким адресатам</translation>
    </message>
    <message>
        <location filename="../src/dialogs/mmsenddlg.cpp" line="96"/>
        <source>Multiple Recipient Message</source>
        <translation>Сообщение нескольким адресатам</translation>
    </message>
    <message>
        <location filename="../src/dialogs/mmsenddlg.cpp" line="110"/>
        <source>Multiple Recipient URL</source>
        <translation>Ссылка нескольким адресатам</translation>
    </message>
    <message>
        <location filename="../src/dialogs/mmsenddlg.cpp" line="250"/>
        <source>Sending mass URL to %1...</source>
        <translation>Отправляется ссылка к %1...</translation>
    </message>
    <message>
        <location filename="../src/dialogs/mmsenddlg.cpp" line="263"/>
        <source>Sending mass list to %1...</source>
        <translation>Отправляется список контактов к %1...</translation>
    </message>
    <message>
        <location filename="../src/dialogs/mmsenddlg.cpp" line="183"/>
        <source>Sending mass message to %1...</source>
        <translation>Отправляется сообщение к %1...</translation>
    </message>
    <message>
        <location filename="../src/dialogs/mmsenddlg.cpp" line="142"/>
        <source>failed</source>
        <translation>неудачно</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::MMUserView</name>
    <message>
        <location filename="../src/views/mmuserview.cpp" line="69"/>
        <source>Add All</source>
        <translation>Добавить всех</translation>
    </message>
    <message>
        <location filename="../src/views/mmuserview.cpp" line="68"/>
        <source>Add Group</source>
        <translation>Добавить группу</translation>
    </message>
    <message>
        <location filename="../src/views/mmuserview.cpp" line="66"/>
        <source>Clear</source>
        <translation>Очистить</translation>
    </message>
    <message>
        <location filename="../src/views/mmuserview.cpp" line="65"/>
        <source>Crop</source>
        <translation>Оставить</translation>
    </message>
    <message>
        <location filename="../src/views/mmuserview.cpp" line="64"/>
        <source>Remove</source>
        <translation>Удалить</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::MainWindow</name>
    <message>
        <location filename="../src/core/mainwin.cpp" line="734"/>
        <location filename="../src/core/mainwin.cpp" line="735"/>
        <source> </source>
        <translation>  </translation>
    </message>
    <message>
        <location filename="../src/core/mainwin.cpp" line="735"/>
        <source>%1 message%2</source>
        <translation>%1 сообщ%2</translation>
    </message>
    <message>
        <location filename="../src/core/mainwin.cpp" line="734"/>
        <source>%1 msg%2</source>
        <translation>%1 сообщ%2</translation>
    </message>
    <message>
        <location filename="../src/core/mainwin.cpp" line="199"/>
        <source>&lt;ul&gt;&lt;li&gt;&lt;tt&gt;%a - &lt;/tt&gt;user alias&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%e - &lt;/tt&gt;email&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%f - &lt;/tt&gt;first name&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%h - &lt;/tt&gt;phone number&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%i - &lt;/tt&gt;user ip&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%l - &lt;/tt&gt;last name&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%L - &lt;/tt&gt;local time&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%m - &lt;/tt&gt;# pending messages&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%M - &lt;/tt&gt;# pending messages (if any)&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%n - &lt;/tt&gt;full name&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%o - &lt;/tt&gt;last seen online&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%O - &lt;/tt&gt;online since&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%p - &lt;/tt&gt;user port&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%s - &lt;/tt&gt;full status&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%S - &lt;/tt&gt;abbreviated status&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%u - &lt;/tt&gt;uin&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%w - &lt;/tt&gt;webpage&lt;/li&gt;&lt;/ul&gt;</source>
        <translation>&lt;ul&gt;&lt;li&gt;&lt;tt&gt;%a - &lt;/tt&gt;псевдоним пользователя&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%e - &lt;/tt&gt;адрес электронной почты&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%f - &lt;/tt&gt;имя&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%h - &lt;/tt&gt;номер телефона&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%i - &lt;/tt&gt;ip адрес пользователя&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%l - &lt;/tt&gt;фамилия&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%m - &lt;/tt&gt;количество непрочитанных сообщений&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%M - &lt;/tt&gt;количество непрочитанных сообщений (если есть)&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%n - &lt;/tt&gt;имя и фамилия&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%o - &lt;/tt&gt;последний раз в сети&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%O - &lt;/tt&gt;с какого времени в сети&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%p - &lt;/tt&gt;порт пользователя&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%s - &lt;/tt&gt;полный статус&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%S - &lt;/tt&gt;сокращенный статус&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%u - &lt;/tt&gt;UIN&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%w - &lt;/tt&gt;домашняя страничка&lt;/li&gt;&lt;/ul&gt;</translation>
    </message>
    <message>
        <location filename="../src/core/mainwin.cpp" line="656"/>
        <source>Error! No owner set</source>
        <translation>Ошибка! Владелец не задан</translation>
    </message>
    <message>
        <location filename="../src/core/mainwin.cpp" line="1062"/>
        <source>Logon failed.
See network window for details.</source>
        <translation>Вход в сеть не удался.
Подробности в окне отладки.</translation>
    </message>
    <message>
        <location filename="../src/core/mainwin.cpp" line="1126"/>
        <source>&lt;h2&gt;Hints for Using&lt;br&gt;the Licq %1 Plugin&lt;/h2&gt;&lt;hr&gt;&lt;ul&gt;&lt;li&gt;Change your status by right clicking on the status label.&lt;/li&gt;&lt;li&gt;Change your auto response by double-clicking on the status label.&lt;/li&gt;&lt;li&gt;View system messages by double clicking on the message label.&lt;/li&gt;&lt;li&gt;Change groups by right clicking on the message label.&lt;/li&gt;&lt;li&gt;Use the following shortcuts from the contact list:&lt;ul&gt;&lt;li&gt;&lt;tt&gt;Ctrl-M : &lt;/tt&gt;Toggle mini-mode&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-O : &lt;/tt&gt;Toggle show offline users&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-X : &lt;/tt&gt;Exit&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-H : &lt;/tt&gt;Hide&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-I : &lt;/tt&gt;View the next message&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-V : &lt;/tt&gt;View message&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-S : &lt;/tt&gt;Send message&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-U : &lt;/tt&gt;Send Url&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-C : &lt;/tt&gt;Send chat request&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-F : &lt;/tt&gt;Send File&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-A : &lt;/tt&gt;Check Auto response&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-P : &lt;/tt&gt;Popup all messages&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-L : &lt;/tt&gt;Redraw user window&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Delete : &lt;/tt&gt;Delete user from current group&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-Delete : &lt;/tt&gt;Delete user from contact list&lt;/li&gt;&lt;/ul&gt;&lt;li&gt;Hold control while clicking on close in the function window to remove   the user from your contact list.&lt;/li&gt;&lt;li&gt;Hit Ctrl-Enter from most text entry fields to select &quot;Ok&quot; or &quot;Accept&quot;.   For example in the send tab of the user function window.&lt;/li&gt;&lt;li&gt;Here is the complete list of user % options, which can be used in &lt;b&gt;OnEvent&lt;/b&gt;   parameters, &lt;b&gt;auto responses&lt;/b&gt;, and &lt;b&gt;utilities&lt;/b&gt;:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/mainwin.cpp" line="752"/>
        <source>No messages</source>
        <translation>Нет сообщений</translation>
    </message>
    <message>
        <location filename="../src/core/mainwin.cpp" line="751"/>
        <source>No msgs</source>
        <translation>Нет сообщений</translation>
    </message>
    <message>
        <location filename="../src/core/mainwin.cpp" line="455"/>
        <source>Right click - Status menu
Double click - Set auto response</source>
        <translation>Правая кнопка - меню статуса
Двойной щелчок - установить авто-ответ</translation>
    </message>
    <message>
        <location filename="../src/core/mainwin.cpp" line="443"/>
        <source>Right click - User groups
Double click - Show next message</source>
        <translation>Правая кнопка - группы пользователей
Двойной щелчок - показать следующее сообщение</translation>
    </message>
    <message>
        <location filename="../src/core/mainwin.cpp" line="725"/>
        <source>SysMsg</source>
        <translation>Сист. сообщение</translation>
    </message>
    <message>
        <location filename="../src/core/mainwin.cpp" line="419"/>
        <source>System</source>
        <translation>Система</translation>
    </message>
    <message>
        <location filename="../src/core/mainwin.cpp" line="409"/>
        <source>&amp;System</source>
        <translation>&amp;Система</translation>
    </message>
    <message>
        <location filename="../src/core/mainwin.cpp" line="726"/>
        <source>System Message</source>
        <translation>Системное сообщение</translation>
    </message>
    <message>
        <location filename="../src/core/mainwin.cpp" line="734"/>
        <location filename="../src/core/mainwin.cpp" line="735"/>
        <source>s</source>
        <translation>ений</translation>
    </message>
    <message>
        <location filename="../src/core/mainwin.cpp" line="684"/>
        <source>is online</source>
        <translation>в сети</translation>
    </message>
    <message>
        <location filename="../src/core/mainwin.cpp" line="1159"/>
        <source>Qt URL handling</source>
        <translation>Обработка URL в Qt</translation>
    </message>
    <message>
        <location filename="../src/core/mainwin.cpp" line="1176"/>
        <source>&lt;p&gt;For more information, see the Licq webpage (&lt;tt&gt;http://www.licq.org&lt;/tt&gt;).&lt;/p&gt;</source>
        <translation>&lt;p&gt;Более подробную информацию можно получить на web-сайте (&lt;tt&gt;http://www.licq.org&lt;/tt&gt;).&lt;/p&gt;</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::MessageBox</name>
    <message>
        <location filename="../src/core/messagebox.cpp" line="120"/>
        <location filename="../src/core/messagebox.cpp" line="373"/>
        <source>Licq</source>
        <translation>Licq</translation>
    </message>
    <message>
        <location filename="../src/core/messagebox.cpp" line="151"/>
        <source>&amp;List</source>
        <translation>&amp;Список</translation>
    </message>
    <message>
        <location filename="../src/core/messagebox.cpp" line="154"/>
        <location filename="../src/core/messagebox.cpp" line="198"/>
        <location filename="../src/core/messagebox.cpp" line="338"/>
        <source>&amp;Next</source>
        <translation>&amp;Далее</translation>
    </message>
    <message>
        <location filename="../src/core/messagebox.cpp" line="156"/>
        <location filename="../src/core/messagebox.cpp" line="203"/>
        <source>&amp;Ok</source>
        <translation>&amp;Ок</translation>
    </message>
    <message>
        <location filename="../src/core/messagebox.cpp" line="209"/>
        <source>&amp;Clear All</source>
        <translation>&amp;Очистить</translation>
    </message>
    <message>
        <location filename="../src/core/messagebox.cpp" line="210"/>
        <location filename="../src/core/messagebox.cpp" line="334"/>
        <source>&amp;Next (%1)</source>
        <translation>&amp;Далее (%1)</translation>
    </message>
    <message>
        <location filename="../src/core/messagebox.cpp" line="359"/>
        <source>Licq Information</source>
        <translation>Licq - Информация</translation>
    </message>
    <message>
        <location filename="../src/core/messagebox.cpp" line="363"/>
        <source>Licq Warning</source>
        <translation>Licq - Предупреждение</translation>
    </message>
    <message>
        <location filename="../src/core/messagebox.cpp" line="367"/>
        <source>Licq Critical</source>
        <translation>Licq - Критическая ошибка</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::MessageList</name>
    <message>
        <location filename="../src/widgets/messagelist.cpp" line="159"/>
        <source>D</source>
        <translation>П</translation>
    </message>
    <message>
        <location filename="../src/widgets/messagelist.cpp" line="159"/>
        <source>Event Type</source>
        <translation>Тип события</translation>
    </message>
    <message>
        <location filename="../src/widgets/messagelist.cpp" line="159"/>
        <source>Options</source>
        <translation>Параметры</translation>
    </message>
    <message>
        <location filename="../src/widgets/messagelist.cpp" line="159"/>
        <source>Time</source>
        <translation>Время</translation>
    </message>
    <message>
        <location filename="../src/widgets/messagelist.cpp" line="251"/>
        <source>Direct</source>
        <translation>Напрямую</translation>
    </message>
    <message>
        <location filename="../src/widgets/messagelist.cpp" line="251"/>
        <source>Server</source>
        <translation>Сервер</translation>
    </message>
    <message>
        <location filename="../src/widgets/messagelist.cpp" line="253"/>
        <source>Urgent</source>
        <translation>Срочно</translation>
    </message>
    <message>
        <location filename="../src/widgets/messagelist.cpp" line="255"/>
        <source>Multiple Recipients</source>
        <translation>Нескольким адресатам</translation>
    </message>
    <message>
        <location filename="../src/widgets/messagelist.cpp" line="257"/>
        <source>Cancelled Event</source>
        <translation>Прерванное событие</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::OnEventBox</name>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="41"/>
        <source>Sound Options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="47"/>
        <source>Sounds enabled:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="48"/>
        <source>Select for which statuses &quot;Command&quot; should be run.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="51"/>
        <source>Never</source>
        <translation type="unfinished">Никогда</translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="52"/>
        <source>Only when online</source>
        <translation type="unfinished">Когда в сети</translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="53"/>
        <source>When online or away</source>
        <translation type="unfinished">Когда в сети или отошел</translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="54"/>
        <source>When online, away or N/A</source>
        <translation type="unfinished">Когда в сети, отошел или недоступен</translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="55"/>
        <source>Always except DND</source>
        <translation type="unfinished">Всегда, кроме &apos;не беспокоить&apos;</translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="56"/>
        <source>Always</source>
        <translation type="unfinished">Всегда</translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="61"/>
        <source>Command:</source>
        <translation type="unfinished">Команда:</translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="62"/>
        <source>Command to execute when an event is received.&lt;br&gt;It will be passed the relevant parameters from below.&lt;br&gt;Parameters can contain the following expressions &lt;br&gt; which will be replaced with the relevant information:</source>
        <translation type="unfinished">Команда, которая будет выполнена при событии.&lt;br&gt;Ей будут переданы параметры, указанные ниже.&lt;br&gt;Также будут выполнены следующие подстановки:</translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="77"/>
        <source>Message:</source>
        <translation type="unfinished">Сообщение:</translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="78"/>
        <source>Parameter for received messages</source>
        <translation type="unfinished">Параметры команды для входящих сообщений</translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="79"/>
        <source>URL:</source>
        <translation type="unfinished">Ссылка:</translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="80"/>
        <source>Parameter for received URLs</source>
        <translation type="unfinished">Параметры команды для ссылок</translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="81"/>
        <source>Chat request:</source>
        <translation type="unfinished">Запрос чата:</translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="82"/>
        <source>Parameter for received chat requests</source>
        <translation type="unfinished">Параматры команды для запросов чата</translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="83"/>
        <source>File transfer:</source>
        <translation type="unfinished">Передача файла:</translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="84"/>
        <source>Parameter for received file transfers</source>
        <translation type="unfinished">Параметры команды для передачи файлов</translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="85"/>
        <source>SMS:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="86"/>
        <source>Parameter for received SMSs</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="87"/>
        <source>Online notify:</source>
        <translation type="unfinished">При появлении:</translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="88"/>
        <source>Parameter for online notification</source>
        <translation type="unfinished">Параматры команды для оповещения о появлении пользователя в сети</translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="89"/>
        <source>System msg:</source>
        <translation type="unfinished">Сист сообщ:</translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="90"/>
        <source>Parameter for received system messages</source>
        <translation type="unfinished">Параметры команды для системных сообщений</translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="91"/>
        <source>Message sent:</source>
        <translation type="unfinished">Отправленное сообщение:</translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="92"/>
        <source>Parameter for sent messages</source>
        <translation type="unfinished">Параметры команды для отправляемых сообщений</translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="103"/>
        <source>Online notify when logging on</source>
        <translation type="unfinished">Оповещать о появлении при заходе в сеть</translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="104"/>
        <source>Perform the online notify OnEvent when logging on (this is different from how the Mirabilis client works)</source>
        <translation type="unfinished">Запускает команду обработки события &apos;При появлении&apos; во время того, как Вы заходите в сеть
(это отличается от поведения клиента от Mirabilis)</translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="111"/>
        <source>Override default setting for enabling sounds</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="117"/>
        <source>Override default command</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="125"/>
        <source>Override parameter</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/oneventbox.cpp" line="132"/>
        <source>Override always online notify setting</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>LicqQtGui::OptionsDlg</name>
    <message>
        <location filename="../src/settings/chat.cpp" line="228"/>
        <source>&lt;p&gt;Available custom date format variables.&lt;/p&gt;&lt;table&gt;&lt;tr&gt;&lt;th&gt;Expression&lt;/th&gt;&lt;th&gt;Output&lt;/th&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;d&lt;/td&gt;&lt;td&gt;the day as number without a leading zero (1-31)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;dd&lt;/td&gt;&lt;td&gt;the day as number with a leading zero (01-31)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;ddd&lt;/td&gt;&lt;td&gt;the abbreviated localized day name (e.g. &apos;Mon&apos;..&apos;Sun&apos;)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;dddd&lt;/td&gt;&lt;td&gt;the long localized day name (e.g. &apos;Monday&apos;..&apos;Sunday&apos;)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;M&lt;/td&gt;&lt;td&gt;the month as number without a leading zero (1-12)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;MM&lt;/td&gt;&lt;td&gt;the month as number with a leading zero (01-12)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;MMM&lt;/td&gt;&lt;td&gt;the abbreviated localized month name (e.g. &apos;Jan&apos;..&apos;Dec&apos;)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;MMMM&lt;/td&gt;&lt;td&gt;the long localized month name (e.g. &apos;January&apos;..&apos;December&apos;)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;yy&lt;/td&gt;&lt;td&gt;the year as two digit number (00-99)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;yyyy&lt;/td&gt;&lt;td&gt;the year as four digit number (1752-8000)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td colspan=2&gt;&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;h&lt;/td&gt;&lt;td&gt;the hour without a leading zero (0..23 or 1..12 if AM/PM display)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;hh&lt;/td&gt;&lt;td&gt;the hour with a leading zero (00..23 or 01..12 if AM/PM display)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;m&lt;/td&gt;&lt;td&gt;the minute without a leading zero (0..59)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;mm&lt;/td&gt;&lt;td&gt;the minute with a leading zero (00..59)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;s&lt;/td&gt;&lt;td&gt;the second without a leading zero (0..59)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;ss&lt;/td&gt;&lt;td&gt;the second with a leading zero (00..59)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;z&lt;/td&gt;&lt;td&gt;the millisecond without leading zero (0..999)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;zzz&lt;/td&gt;&lt;td&gt;the millisecond with leading zero (000..999)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;AP&lt;/td&gt;&lt;td&gt;use AM/PM display. AP will be replaced by either &apos;AM&apos; or &apos;PM&apos;&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;ap&lt;/td&gt;&lt;td&gt;use am/pm display. ap will be replaced by either &apos;am&apos; or &apos;pm&apos;&lt;/td&gt;&lt;/tr&gt;&lt;/table&gt;</source>
        <translation>&lt;p&gt;Переменные для форматирования даты.&lt;/p&gt;&lt;table&gt;&lt;tr&gt;&lt;th&gt;Выражение&lt;/th&gt;&lt;th&gt;Вывод&lt;/th&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;d&lt;/td&gt;&lt;td&gt;дни месяца без нуля в начале (1-31)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;dd&lt;/td&gt;&lt;td&gt;дни месяца с нулем в начале (01-31)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;ddd&lt;/td&gt;&lt;td&gt;сокращенные дни недели (&apos;Пнд&apos;..&apos;Вск&apos;)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;dddd&lt;/td&gt;&lt;td&gt;полные дни недели (&apos;Понедельник&apos;..&apos;Воскресенье&apos;)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;M&lt;/td&gt;&lt;td&gt;номер месяца без нуля в начале (1-12)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;MM&lt;/td&gt;&lt;td&gt;номер месяца с нулем в начале (01-12)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;MMM&lt;/td&gt;&lt;td&gt;сокращенные месяцы (&apos;Янв&apos;..&apos;Дек&apos;)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;MMMM&lt;/td&gt;&lt;td&gt;полные месяцы (&apos;Январь&apos;..&apos;Декабрь&apos;)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;yy&lt;/td&gt;&lt;td&gt;две цифры года (00-99)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;yyyy&lt;/td&gt;&lt;td&gt;четыре цифры года (1752-8000)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td colspan=2&gt;&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;h&lt;/td&gt;&lt;td&gt;часы без нуля в начале (0..23 или 1..12 при AM/PM отображении)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;hh&lt;/td&gt;&lt;td&gt;часы с нулем в начале (00..23 или 01..12 при AM/PM отображении)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;m&lt;/td&gt;&lt;td&gt;минуты без нуля в начале (0..59)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;mm&lt;/td&gt;&lt;td&gt;минуты с нулем в начале (00..59)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;s&lt;/td&gt;&lt;td&gt;секунды без нуля в начале (0..59)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;ss&lt;/td&gt;&lt;td&gt;секунды с нулем в начале (00..59)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;z&lt;/td&gt;&lt;td&gt;миллисекунды без нуля в начале (0..999)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;zzz&lt;/td&gt;&lt;td&gt;миллисекунды с нулем в начале (000..999)&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;AP&lt;/td&gt;&lt;td&gt;отображение AM/PM. AP будет заменено на &apos;AM&apos; либо &apos;PM&apos;&lt;/td&gt;&lt;/tr&gt;&lt;tr&gt;&lt;td&gt;ap&lt;/td&gt;&lt;td&gt;отображение am/pm. ap будет заменено на &apos;am&apos; либо &apos;pm&apos;&lt;/td&gt;&lt;/tr&gt;&lt;/table&gt;</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::OwnerEditDlg</name>
    <message>
        <location filename="../src/dialogs/ownereditdlg.cpp" line="110"/>
        <source>Currently only one account per protocol is supported.</source>
        <translation>Поддерживается только одна учётная запись для протокола.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/ownereditdlg.cpp" line="51"/>
        <source>Edit Account</source>
        <translation>Редактирование учётной записи</translation>
    </message>
    <message>
        <location filename="../src/dialogs/ownereditdlg.cpp" line="75"/>
        <source>Pro&amp;tocol:</source>
        <translation>Про&amp;токол:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/ownereditdlg.cpp" line="76"/>
        <source>&amp;User ID:</source>
        <translation>&amp;Идентификатор:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/ownereditdlg.cpp" line="77"/>
        <source>&amp;Password:</source>
        <translation>&amp;Пароль:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/ownereditdlg.cpp" line="81"/>
        <source>&amp;Save Password</source>
        <translation>&amp;Сохранить пароль</translation>
    </message>
    <message>
        <location filename="../src/dialogs/ownereditdlg.cpp" line="127"/>
        <source>User ID field cannot be empty.</source>
        <translation>Идентификатор не может быть пустым.</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::OwnerManagerDlg</name>
    <message>
        <location filename="../src/dialogs/ownermanagerdlg.cpp" line="152"/>
        <source>(Invalid ID)</source>
        <translation>(Неверный идентификатор)</translation>
    </message>
    <message>
        <location filename="../src/dialogs/ownermanagerdlg.cpp" line="150"/>
        <source>(Invalid Protocol)</source>
        <translation>(Неверный протокол)</translation>
    </message>
    <message>
        <source>&amp;Add</source>
        <translation type="obsolete">&amp;Добавить</translation>
    </message>
    <message>
        <source>D&amp;elete</source>
        <translation type="obsolete">&amp;Удалить</translation>
    </message>
    <message>
        <location filename="../src/dialogs/ownermanagerdlg.cpp" line="96"/>
        <source>&amp;Done</source>
        <translation>&amp;Завершить</translation>
    </message>
    <message>
        <location filename="../src/dialogs/ownermanagerdlg.cpp" line="230"/>
        <source>Do you really want to remove account %1?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/dialogs/ownermanagerdlg.cpp" line="68"/>
        <source>Licq - Account Manager</source>
        <translation>Licq - Менеджер учётных записей</translation>
    </message>
    <message>
        <source>&amp;Modify</source>
        <translation type="obsolete">&amp;Изменить</translation>
    </message>
    <message>
        <location filename="../src/dialogs/ownermanagerdlg.cpp" line="75"/>
        <source>Protocol</source>
        <translation>Протокол</translation>
    </message>
    <message>
        <source>&amp;Register</source>
        <translation type="obsolete">&amp;Зарегистрировать</translation>
    </message>
    <message>
        <location filename="../src/dialogs/ownermanagerdlg.cpp" line="75"/>
        <source>User ID</source>
        <translation>Идентификатор</translation>
    </message>
    <message>
        <location filename="../src/dialogs/ownermanagerdlg.cpp" line="84"/>
        <source>&amp;Add...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/dialogs/ownermanagerdlg.cpp" line="87"/>
        <source>&amp;Register...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/dialogs/ownermanagerdlg.cpp" line="90"/>
        <source>&amp;Modify...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/dialogs/ownermanagerdlg.cpp" line="93"/>
        <source>D&amp;elete...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/dialogs/ownermanagerdlg.cpp" line="118"/>
        <source>From the Account Manager dialog you can add your accounts or register a new account.
Note that only one account per protocol is supported in Licq.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/dialogs/ownermanagerdlg.cpp" line="179"/>
        <source>You are currently registered as
UIN (User ID): %1
Base Directory: %2
Rerun licq with the -b option to select a new
base directory and then register a new user.</source>
        <translation>Ваши регистрационные данные:
Пользователь (UIN): %1
Рабочий каталог: %2
Перезапустите licq с опцией -b чтобы выбрать другой
рабочий каталог и тогда регистрируйте новую запись.</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::PluginDlg</name>
    <message>
        <location filename="../src/dialogs/plugindlg.cpp" line="324"/>
        <location filename="../src/dialogs/plugindlg.cpp" line="364"/>
        <source>(Unloaded)</source>
        <translation>(Выгружен)</translation>
    </message>
    <message>
        <location filename="../src/dialogs/plugindlg.cpp" line="85"/>
        <location filename="../src/dialogs/plugindlg.cpp" line="101"/>
        <source>Description</source>
        <translation>Описание</translation>
    </message>
    <message>
        <location filename="../src/dialogs/plugindlg.cpp" line="117"/>
        <source>Done</source>
        <translation>Завершить</translation>
    </message>
    <message>
        <location filename="../src/dialogs/plugindlg.cpp" line="85"/>
        <source>Enable</source>
        <translation>Включить</translation>
    </message>
    <message>
        <location filename="../src/dialogs/plugindlg.cpp" line="71"/>
        <source>Licq - Plugin Manager</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/dialogs/plugindlg.cpp" line="85"/>
        <location filename="../src/dialogs/plugindlg.cpp" line="101"/>
        <source>Id</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../src/dialogs/plugindlg.cpp" line="85"/>
        <location filename="../src/dialogs/plugindlg.cpp" line="101"/>
        <source>Load</source>
        <translation>Загрузить</translation>
    </message>
    <message>
        <location filename="../src/dialogs/plugindlg.cpp" line="85"/>
        <location filename="../src/dialogs/plugindlg.cpp" line="101"/>
        <source>Name</source>
        <translation>Название</translation>
    </message>
    <message>
        <location filename="../src/dialogs/plugindlg.cpp" line="262"/>
        <source>Plugin %1 has no configuration file</source>
        <translation>У модуля %1 нет файла настроек</translation>
    </message>
    <message>
        <location filename="../src/dialogs/plugindlg.cpp" line="90"/>
        <source>Protocol Plugins</source>
        <translation>Модули протоколов</translation>
    </message>
    <message>
        <location filename="../src/dialogs/plugindlg.cpp" line="113"/>
        <source>Refresh</source>
        <translation>Обновить</translation>
    </message>
    <message>
        <location filename="../src/dialogs/plugindlg.cpp" line="74"/>
        <source>Standard Plugins</source>
        <translation>Стандартные модули</translation>
    </message>
    <message>
        <location filename="../src/dialogs/plugindlg.cpp" line="85"/>
        <location filename="../src/dialogs/plugindlg.cpp" line="101"/>
        <source>Version</source>
        <translation>Версия</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::RandomChatDlg</name>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="75"/>
        <source>20 Something</source>
        <translation>от 20</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="76"/>
        <source>30 Something</source>
        <translation>от 30</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="77"/>
        <source>40 Something</source>
        <translation>от 40</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="78"/>
        <source>50 Plus</source>
        <translation>от 50 и выше</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="73"/>
        <source>Games</source>
        <translation>Игры</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="55"/>
        <source>Licq - Random Chat Search</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="71"/>
        <source>General</source>
        <translation>Общий</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="127"/>
        <source>No random chat user found in that group.</source>
        <translation>Не найдено ни одного пользователя в этой группе.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="133"/>
        <source>Random chat search had an error.</source>
        <translation>В процессе поиска случайного собеседника возникла ошибка.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="130"/>
        <source>Random chat search timed out.</source>
        <translation>Поиск случайного собеседника не удался - время ожидания вышло.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="72"/>
        <source>Romance</source>
        <translation>Романтики</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="64"/>
        <source>&amp;Search</source>
        <translation>&amp;Поиск</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="113"/>
        <source>Searching for Random Chat Partner...</source>
        <translation>Поиск случайного собеседника...</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="80"/>
        <source>Seeking Men</source>
        <translation>Она ищет его</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="79"/>
        <source>Seeking Women</source>
        <translation>Он ищет ее</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="74"/>
        <source>Students</source>
        <translation>Студенты</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::RefuseDlg</name>
    <message>
        <location filename="../src/dialogs/refusedlg.cpp" line="64"/>
        <source>Cancel</source>
        <translation>Отмена</translation>
    </message>
    <message>
        <location filename="../src/dialogs/refusedlg.cpp" line="49"/>
        <source>Refusal message for %1 with </source>
        <translation>Причина отказа в %1 с </translation>
    </message>
    <message>
        <location filename="../src/dialogs/refusedlg.cpp" line="60"/>
        <source>Refuse</source>
        <translation>Отказать</translation>
    </message>
    <message>
        <location filename="../src/dialogs/refusedlg.cpp" line="68"/>
        <source>Licq - %1 Refusal</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>LicqQtGui::RegisterUserDlg</name>
    <message>
        <location filename="../src/dialogs/registeruser.cpp" line="120"/>
        <source>&amp;Remember Password</source>
        <translation>&amp;Запомнить пароль</translation>
    </message>
    <message>
        <location filename="../src/dialogs/registeruser.cpp" line="55"/>
        <source>Register Account</source>
        <translation>Регистрация учётной записи</translation>
    </message>
    <message>
        <location filename="../src/dialogs/registeruser.cpp" line="80"/>
        <source>Introduction</source>
        <translation>Введение</translation>
    </message>
    <message>
        <location filename="../src/dialogs/registeruser.cpp" line="84"/>
        <source>Welcome to the Registration Wizard.

You can register a new ICQ account here.

Press &quot;Next&quot; to proceed.</source>
        <translation>Добро пожаловать в мастер регистрации.

Здесь Вы можете зарегистрировать нового пользователя.

Нажмите &quot;Далее&quot;, чтобы продолжить.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/registeruser.cpp" line="99"/>
        <source>Select password</source>
        <translation>Укажите пароль</translation>
    </message>
    <message>
        <location filename="../src/dialogs/registeruser.cpp" line="100"/>
        <source>Specify a password for your account.
Length must be 1 to 8 characters.</source>
        <translation>Укажите пароль для Вашей учетной записи.
Длина должна быть от 1 до 8 символов.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/registeruser.cpp" line="104"/>
        <source>&amp;Password:</source>
        <translation>&amp;Пароль:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/registeruser.cpp" line="112"/>
        <source>&amp;Verify:</source>
        <translation>&amp;Контроль:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/registeruser.cpp" line="132"/>
        <source>Account Verification</source>
        <translation>Подтверждение учетной записи</translation>
    </message>
    <message>
        <location filename="../src/dialogs/registeruser.cpp" line="133"/>
        <source>Retype the letters shown in the image.</source>
        <translation>Введите буквы, указанные на катринке.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/registeruser.cpp" line="140"/>
        <source>&amp;Verification:</source>
        <translation>&amp;Подтверждение:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/registeruser.cpp" line="154"/>
        <source>Registration Completed</source>
        <translation>Регистрация окончена</translation>
    </message>
    <message>
        <location filename="../src/dialogs/registeruser.cpp" line="158"/>
        <source>Account registration has been successfuly completed.</source>
        <translation>Регистрация учетной записи успешно завершена.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/registeruser.cpp" line="162"/>
        <source>Your new user Id:</source>
        <translation>Идентификатор нового пользователя:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/registeruser.cpp" line="168"/>
        <source>You are now being automatically logged on.
Click Finish to edit your personal details.
After you are online, you can send your personal details to the server.</source>
        <translation>Вы сейчас будете автоматически подключены к сети.
Нажмите &quot;Завершить&quot;, чтобы отредактировать Ваши персональные данные.
После подключения можете их отправить на сервер.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/registeruser.cpp" line="185"/>
        <source>Passwords don&apos;t match.</source>
        <translation>Пароли не совпадают.</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::ReqAuthDlg</name>
    <message>
        <location filename="../src/dialogs/reqauthdlg.cpp" line="52"/>
        <source>Licq - Request Authorization</source>
        <translation>Licq - Запрос авторизации</translation>
    </message>
    <message>
        <location filename="../src/dialogs/reqauthdlg.cpp" line="69"/>
        <source>Request</source>
        <translation>Запрос</translation>
    </message>
    <message>
        <location filename="../src/dialogs/reqauthdlg.cpp" line="58"/>
        <source>Request authorization from (UIN):</source>
        <translation>Запросить авторизацию от (UIN):</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::SearchUserDlg</name>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="396"/>
        <source>%1 more users found. Narrow search.</source>
        <translation>Еще найдено %1 пользователей. Уточните критерии поиска.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="372"/>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="382"/>
        <source>?</source>
        <translation>?</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="428"/>
        <source>&amp;Add %1 Users</source>
        <translation>&amp;Добавить (%1)</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="191"/>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="425"/>
        <source>&amp;Add User</source>
        <translation>&amp;Добавить пользователя</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="139"/>
        <source>Age Range:</source>
        <translation>Возраст:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="173"/>
        <source>Alias</source>
        <translation>Псевдоним</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="136"/>
        <source>Alias:</source>
        <translation>Псевдоним:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="243"/>
        <source>Cancel</source>
        <translation>Отмена</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="146"/>
        <source>City:</source>
        <translation>Город:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="150"/>
        <source>Company Department:</source>
        <translation>Подразделение компании:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="149"/>
        <source>Company Name:</source>
        <translation>Название компании:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="151"/>
        <source>Company Position:</source>
        <translation>Должность:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="148"/>
        <source>Country:</source>
        <translation>Страна:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="173"/>
        <source>Email</source>
        <translation>Email</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="153"/>
        <source>Email Address:</source>
        <translation>E-Mail адрес:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="221"/>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="315"/>
        <source>Enter search parameters and select &apos;Search&apos;</source>
        <translation>Введите параметры и нажмите кнопку &apos;Поиск&apos;</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="376"/>
        <source>F</source>
        <translation>Ж</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="91"/>
        <source>Female</source>
        <translation>Женский</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="137"/>
        <source>First Name:</source>
        <translation>Имя:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="140"/>
        <source>Gender:</source>
        <translation>Пол:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="154"/>
        <source>Keyword:</source>
        <translation>Ключевое слово:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="141"/>
        <source>Language:</source>
        <translation>Язык:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="138"/>
        <source>Last Name:</source>
        <translation>Фамилия:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="64"/>
        <source>Licq - User Search</source>
        <translation>Licq - Поиск пользователей</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="379"/>
        <source>M</source>
        <translation>М</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="92"/>
        <source>Male</source>
        <translation>Мужской</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="394"/>
        <source>More users found. Narrow search.</source>
        <translation>Слишком много пользователей. Уточните критерии поиска.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="173"/>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="386"/>
        <source>No</source>
        <translation>Нет</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="361"/>
        <source>Offline</source>
        <translation>Отключен</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="364"/>
        <source>Online</source>
        <translation>В сети</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="208"/>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="314"/>
        <source>Reset Search</source>
        <translation>Очистить</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="160"/>
        <source>Return Online Users Only</source>
        <translation>Только подключенные</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="203"/>
        <source>&amp;Search</source>
        <translation>&amp;Поиск</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="392"/>
        <source>Search complete.</source>
        <translation>Поиск завершен.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="408"/>
        <source>Search failed.</source>
        <translation>Поиск не удался.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="272"/>
        <source>Searching (this can take awhile)...</source>
        <translation>Идет поиск (это может занять некоторое время)...</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="147"/>
        <source>State:</source>
        <translation>Штат:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="174"/>
        <source>Status</source>
        <translation>Статус</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="173"/>
        <source>UIN</source>
        <translation>UIN</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="368"/>
        <source>Unknown</source>
        <translation>Неизвестно</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="81"/>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="90"/>
        <source>Unspecified</source>
        <translation>Не указан</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="186"/>
        <source>View &amp;Info</source>
        <translation>&amp;Просмотр деталей</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="386"/>
        <source>Yes</source>
        <translation>Да</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="72"/>
        <source>Search Criteria</source>
        <translation>Критерий поиска</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="129"/>
        <source>UIN:</source>
        <translation>UIN:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="168"/>
        <source>Result</source>
        <translation>Результат</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="174"/>
        <source>A/G</source>
        <translation>В/П</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="174"/>
        <source>Auth</source>
        <translation>Авториз</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="214"/>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="280"/>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="401"/>
        <source>New Search</source>
        <translation>Новый поиск</translation>
    </message>
    <message>
        <location filename="../src/dialogs/searchuserdlg.cpp" line="281"/>
        <source>Search interrupted</source>
        <translation>Поиск прерван</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::SecurityDlg</name>
    <message>
        <location filename="../src/dialogs/securitydlg.cpp" line="71"/>
        <source>&amp;Authorization Required</source>
        <translation>&amp;Требуется авторизация</translation>
    </message>
    <message>
        <location filename="../src/dialogs/securitydlg.cpp" line="76"/>
        <source>&amp;Hide IP</source>
        <translation>&amp;Скрыть IP</translation>
    </message>
    <message>
        <location filename="../src/dialogs/securitydlg.cpp" line="168"/>
        <source>Internal error while setting security options.</source>
        <translation>Произошла внутренняя ошибка при сохранении настроек безопасности.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/securitydlg.cpp" line="62"/>
        <source>Options</source>
        <translation>Настройки</translation>
    </message>
    <message>
        <location filename="../src/dialogs/securitydlg.cpp" line="158"/>
        <source>Setting security options failed.</source>
        <translation>Не удалось сохранить настройки безопасности.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/securitydlg.cpp" line="163"/>
        <source>Timeout while setting security options.</source>
        <translation>Время ожидания вышло при сохранении настроек безопасности.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/securitydlg.cpp" line="86"/>
        <source>&amp;Update</source>
        <translation>&amp;Обновить</translation>
    </message>
    <message>
        <location filename="../src/dialogs/securitydlg.cpp" line="74"/>
        <source>&amp;Web Presence</source>
        <translation>&amp;Присутствие в web</translation>
    </message>
    <message>
        <location filename="../src/dialogs/securitydlg.cpp" line="113"/>
        <source>You need to be connected to the
ICQ Network to change the settings.</source>
        <translation>Вы должны сначала войти в сеть ICQ,
чтобы изменить настройки.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/securitydlg.cpp" line="167"/>
        <source>error</source>
        <translation>ошибка</translation>
    </message>
    <message>
        <location filename="../src/dialogs/securitydlg.cpp" line="157"/>
        <source>failed</source>
        <translation>неудачно</translation>
    </message>
    <message>
        <location filename="../src/dialogs/securitydlg.cpp" line="162"/>
        <source>timed out</source>
        <translation>время ожидания вышло</translation>
    </message>
    <message>
        <location filename="../src/dialogs/securitydlg.cpp" line="71"/>
        <source>Determines whether regular ICQ clients require
your authorization to add you to their contact list.</source>
        <translation>Определяет, должны ли пользователи запрашивать Вашу
авторизацию для добавления Вас в свой список контактов.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/securitydlg.cpp" line="74"/>
        <source>Web Presence allows users to see
if you are online through your web indicator.</source>
        <translation>Присутствие в web позволяет пользователям видеть,
в сети Вы или нет, используя web-индикатор.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/securitydlg.cpp" line="76"/>
        <source>Hide IP stops users from seeing your IP address.
It doesn&apos;t guarantee it will be hidden though.</source>
        <translation>Эта опция позволяет скрыть Ваш IP адрес.
Установка этой опции не гарантирует,
что IP адрес невозможно будет узнать.</translation>
    </message>
    <message>
        <location filename="../src/dialogs/securitydlg.cpp" line="132"/>
        <location filename="../src/dialogs/securitydlg.cpp" line="178"/>
        <source>Setting...</source>
        <translation>Устанавливается...</translation>
    </message>
    <message>
        <location filename="../src/dialogs/securitydlg.cpp" line="45"/>
        <source>ICQ Security</source>
        <translation>ICQ Безопасность</translation>
    </message>
    <message>
        <location filename="../src/dialogs/securitydlg.cpp" line="55"/>
        <source>No ICQ owner found.
Please create one first.</source>
        <translation>Не найдена учетная запись ICQ.
Пожалуйста, создайте сначала.</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::SetRandomChatGroupDlg</name>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="176"/>
        <source>(none)</source>
        <translation>(нет)</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="181"/>
        <source>20 Something</source>
        <translation>от 20</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="182"/>
        <source>30 Something</source>
        <translation>от 30</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="183"/>
        <source>40 Something</source>
        <translation>от 40</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="184"/>
        <source>50 Plus</source>
        <translation>от 50 и выше</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="223"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Отмена</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="166"/>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="251"/>
        <source>&amp;Close</source>
        <translation>&amp;Закрыть</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="179"/>
        <source>Games</source>
        <translation>Игры</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="177"/>
        <source>General</source>
        <translation>Общий</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="178"/>
        <source>Romance</source>
        <translation>Романтики</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="186"/>
        <source>Seeking Men</source>
        <translation>Она ищет его</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="185"/>
        <source>Seeking Women</source>
        <translation>Он ищет ее</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="162"/>
        <source>&amp;Set</source>
        <translation>&amp;Установить</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="153"/>
        <source>Set Random Chat Group</source>
        <translation>Установка группы для случайного чата</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="242"/>
        <source>Setting Random Chat Group...</source>
        <translation>Устанавливается группа случайных чатов...</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="180"/>
        <source>Students</source>
        <translation>Студенты</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="266"/>
        <source>done</source>
        <translation>готово</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="263"/>
        <source>error</source>
        <translation>ошибка</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="257"/>
        <source>failed</source>
        <translation>неудачно</translation>
    </message>
    <message>
        <location filename="../src/dialogs/randomchatdlg.cpp" line="260"/>
        <source>timed out</source>
        <translation>время ожидания вышло</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::Settings::Chat</name>
    <message>
        <location filename="../src/settings/chat.cpp" line="63"/>
        <source>Chat</source>
        <translation>Чат</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="65"/>
        <location filename="../src/settings/chat.cpp" line="263"/>
        <source>Chat Display</source>
        <translation>Отображение чата</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="67"/>
        <location filename="../src/settings/chat.cpp" line="378"/>
        <source>History Display</source>
        <translation>Отображение истории</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="78"/>
        <source>General Chat Options</source>
        <translation>Общие настройки чата</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="81"/>
        <source>Chatmode messageview</source>
        <translation>Просмотр сообщений в режиме чата</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="82"/>
        <source>Show the current chat history in Send Window</source>
        <translation>Показывает историю разговора в окне отправки сообщения</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="86"/>
        <source>Tabbed chatting</source>
        <translation>Использовать вкладки</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="87"/>
        <source>Use tabs in Send Window</source>
        <translation>Использовать вкладки в окне отправки сообщения</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="90"/>
        <source>Single line chat mode</source>
        <translation>Однострочный режим ввода</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="91"/>
        <source>Send messages with Return and insert new lines with (Ctrl|Shift)+Return, opposite of the normal mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="94"/>
        <source>Use double return</source>
        <translation>Использовать двойной Return</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="95"/>
        <source>Hitting Return twice will be used instead of Ctrl+Return
to send messages and close input dialogs.
Multiple new lines can be inserted with Ctrl+Return.</source>
        <translation>Двойное нажатие на Return будет использоваться вместо Ctrl+Return
для отправки сообщений и закрытия диалогового окна.
Пустые строки вставляются с помощью Ctrl+Return.</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="100"/>
        <source>Show Send/Close buttons</source>
        <translation>Показывать кнопки Отослать/Закрыть</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="101"/>
        <source>Show Send and Close buttons in the chat dialog.</source>
        <translation>Показывать кнопки Отослать/Закрыть в диалоговом окне.</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="108"/>
        <source>Sticky message window(s)</source>
        <translation>Закрепить окна сообщений</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="109"/>
        <source>Makes the message window(s) visible on all desktops</source>
        <translation>Сделать окно сообщений видимым на всех рабочих столах</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="112"/>
        <source>Auto close function window</source>
        <translation>Авто-закрытие окон функций</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="113"/>
        <source>Auto close the user function window after a successful event</source>
        <translation>Автоматически закрывать окна функций после успешной обработки события</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="116"/>
        <source>Check clipboard For URIs/files</source>
        <translation>Проверять буфер обмена
на наличие файлов/ссылок</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="117"/>
        <source>When double-clicking on a user to send a message check for urls/files in the clipboard</source>
        <translation>При двойном нажатии кнопкой мыши на пользователе чтобы открыть окно отправки сообщения,
автоматически будет произведена проверка буфера обмена на наличие имен файлов или ссылок</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="120"/>
        <source>Auto position the reply window</source>
        <translation>Автоматически располагать окно ответа</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="121"/>
        <source>Position a new reply window just underneath the message view window</source>
        <translation>Окно ответа будет располагаться прямо под окном с сообщением</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="124"/>
        <source>Auto send through server</source>
        <translation>Отправлять сообщения через 
сервер автоматически</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="125"/>
        <source>Automatically send messages through the server if direct connection fails</source>
        <translation>Отправлять сообщения через сервер, если не удалось установить прямое соединение</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="128"/>
        <source>Send typing notifications</source>
        <translation>Посылать уведомление о наборе текста</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="129"/>
        <source>Send a notification to the user so they can see when you are typing a message to them</source>
        <translation>Посылать уведомление удалённому пользователю во время набора текста сообщения</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="148"/>
        <source>Dictionary file:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="149"/>
        <source>Dictionary file to use when checking spelling.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="153"/>
        <source>*.dic|Dictionary files for Hunspell\/Myspell (*.dic)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="164"/>
        <source>Localization</source>
        <translation>Локализация</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="168"/>
        <source>Default encoding:</source>
        <translation>Кодировка по умолчанию:</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="169"/>
        <source>Sets which default encoding should be used for newly added contacts.</source>
        <translation>Устанавливает кодировку по умолчанию для новых пользователей.</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="173"/>
        <source>System default (%1)</source>
        <translation>Системная по умолчанию (%1)</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="188"/>
        <source>Show all encodings</source>
        <translation>Показывать все кодировки</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="189"/>
        <source>Show all available encodings in the User Encoding selection menu.
Normally, this menu shows only commonly used encodings.</source>
        <translation>Отображает все доступные кодировки в меню выбора кодировки пользователя.
Обычно отображаются только наиболее часто используемые кодировки.</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="195"/>
        <source>Extensions</source>
        <translation>Программы</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="199"/>
        <location filename="../src/settings/chat.cpp" line="203"/>
        <source>Terminal:</source>
        <translation>Терминал:</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="200"/>
        <source>The command to run to start your terminal program.</source>
        <translation>Команда для запуска предпочитаемого Вами терминала.</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="267"/>
        <location filename="../src/settings/chat.cpp" line="382"/>
        <source>Style:</source>
        <translation>Стиль:</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="277"/>
        <location filename="../src/settings/chat.cpp" line="392"/>
        <source>Date format:</source>
        <translation>Формат даты:</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="291"/>
        <location filename="../src/settings/chat.cpp" line="407"/>
        <source>Insert vertical spacing</source>
        <translation>Вертикальный пробел</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="293"/>
        <location filename="../src/settings/chat.cpp" line="409"/>
        <source>Insert extra space between messages.</source>
        <translation>Вставлять дополнительное пространство между сообщениями.</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="296"/>
        <source>Insert horizontal line</source>
        <translation>Вставлять горизонтальные линии</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="298"/>
        <source>Insert a line between each message.</source>
        <translation>Вставлять горизонтальную полоску между сообщениями.</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="301"/>
        <source>Show recent messages</source>
        <translation>Показывать предыдущие сообщения</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="302"/>
        <source>Show the last 5 messages when a Send Window is opened</source>
        <translation>Показывать 5 последних сообщений в окне отправки</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="306"/>
        <source>Show join/left notices</source>
        <translation>Показывать приходы/уходы</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="307"/>
        <source>Show a notice in the chat window when a user joins or leaves the conversation.</source>
        <translation>Уведомлять в диалоговом окне о приходе и уходе пользователя.</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="314"/>
        <source>Colors</source>
        <translation>Цвета</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="317"/>
        <source>Message received:</source>
        <translation>Полученное сообщение:</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="323"/>
        <source>Message sent:</source>
        <translation>Отправленное сообщение:</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="329"/>
        <source>History received:</source>
        <translation>История сообщений, полученное:</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="335"/>
        <source>History sent:</source>
        <translation>История сообщений, отправленное:</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="341"/>
        <source>Notice:</source>
        <translation>Примечание:</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="347"/>
        <source>Typing notification color:</source>
        <translation>Цвет уведомления о наборе:</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="353"/>
        <source>Background color:</source>
        <translation>Цвет фона:</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="412"/>
        <source>Reverse history</source>
        <translation>Обратный порядок</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="414"/>
        <source>Put recent messages on top.</source>
        <translation>Добавлять новые сообщения наверх.</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="418"/>
        <source>Preview</source>
        <translation>Предпросмотр</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="451"/>
        <source>This is a received message</source>
        <translation>Это полученное сообщение</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="452"/>
        <source>This is a sent message</source>
        <translation>Это отправленное сообщение</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="453"/>
        <source>Have you gone to the Licq IRC Channel?</source>
        <translation>А ты бывал на IRC канале Licq?</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="454"/>
        <source>No, where is it?</source>
        <translation>Нет, а где он?</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="455"/>
        <source>#Licq on irc.freenode.net</source>
        <translation>#Licq на irc.freenode.net</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="456"/>
        <source>Cool, I&apos;ll see you there :)</source>
        <translation>Отлично, до встречи там :)</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="457"/>
        <source>We&apos;ll be waiting!</source>
        <translation>Будем ждать!</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="458"/>
        <source>Marge has left the conversation.</source>
        <translation>Marge удалилась.</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="573"/>
        <source>none</source>
        <translation>нет</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="132"/>
        <source>Show user picture</source>
        <translation>Показывать иконку</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="133"/>
        <source>Show user picture next to the input area</source>
        <translation>Показывать иконку пользователя возле поля ввода</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="136"/>
        <source>Minimize user picture</source>
        <translation>Скрывать иконку</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="137"/>
        <source>Hide user picture upon opening</source>
        <translation>При открытии иконка будет скрыта</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="141"/>
        <source>Check spelling</source>
        <translation>Проверить правописание</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="104"/>
        <source>Popup auto response</source>
        <translation>Показывать авто-ответ</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="105"/>
        <source>Popup auto responses received when sending to contacts that are away.</source>
        <translation>Показывать авто-ответ, полученный при написании отсутствующим пользователям.</translation>
    </message>
    <message>
        <location filename="../src/settings/chat.cpp" line="142"/>
        <source>Mark misspelled words as you type.</source>
        <translation>Отмечать слова с ошибками во время набора.</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::Settings::ContactList</name>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="59"/>
        <source>Contact List</source>
        <translation>Список контактов</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="61"/>
        <source>Columns</source>
        <translation>Столбцы</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="63"/>
        <source>Contact Info</source>
        <translation>Информация</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="77"/>
        <source>Show grid lines</source>
        <translation>Показывать разделительные линии</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="81"/>
        <source>Show column headers</source>
        <translation>Показывать заголовки столбцов</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="82"/>
        <source>Turns on or off the display of headers above each column in the user list</source>
        <translation>Включает или выключает отображение заголовков над столбцами в списке контактов</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="85"/>
        <source>Show user dividers</source>
        <translation>Показывать разделитель</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="86"/>
        <source>Show the &quot;--online--&quot; and &quot;--offline--&quot; bars in the contact list</source>
        <translation>Добавляет строчки &quot;--в сети--&quot; и &quot;--не в сети--&quot; в список контактов</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="93"/>
        <source>Use font styles</source>
        <translation>Использовать стили шрифтов</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="94"/>
        <source>Use italics and bold in the user list to indicate special characteristics such as online notify and visible list</source>
        <translation>Использовать наклонный и полужирный шрифты в списке пользователей для обозначения специальных
характеристик, таких как оповещение о наличии в сети и присутствие в списке видимости</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="98"/>
        <source>Show extended icons</source>
        <translation>Показывать дополнительные пиктограммы</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="99"/>
        <source>Show birthday, invisible, and custom auto response icons to the right of users in the list</source>
        <translation>Показывать дополнительные пиктограммы (день рождения, невидимость, персональный авто-ответ) справа от имени пользователя</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="108"/>
        <source>Show user display picture</source>
        <translation>Показывать иконку пользователя</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="109"/>
        <source>Show the user&apos;s display picture instead of a status icon, if the user is online and has a display picture</source>
        <translation>Использовать иконку-рисунок пользователя вместо иконки статуса в случаях, когда пользователь в сети и имеет такую иконку</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="168"/>
        <source>Use server side contact list</source>
        <translation>Хранить список контактов на сервере</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="169"/>
        <source>Store your contacts on the server so they are accessible from different locations and/or programs</source>
        <translation>Ваш список контактов будет храниться на сервере и будет доступен на любой машине</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="114"/>
        <source>Always show online notify users</source>
        <translation>Всегда показывать пользователей, 
для которых включено оповещение</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="115"/>
        <source>Show online notify users who are offline even when offline users are hidden.</source>
        <translation>Показывает пользователей, для которых включено оповещение и которые сейчас 
не в сети, даже если стоит запрет на показ пользователей не в сети.</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="172"/>
        <source>Manual &quot;New User&quot; group handling</source>
        <translation>Обрабатывать группу 
&quot;Новые пользователи&quot; вручную</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="173"/>
        <source>If not checked, a user will be automatically removed from &quot;New User&quot; group when you first send an event to them.</source>
        <translation>Если отключено, пользователь будет удален из группы &quot;Новые пользователи&quot; при первой отправке сообщения.</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="177"/>
        <source>Allow dragging main window</source>
        <translation>Разрешить перенос основного окна</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="178"/>
        <source>Lets you drag around the main window with your mouse</source>
        <translation>Позволяет переносить главное окно с помощью мыши</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="181"/>
        <source>Sticky main window</source>
        <translation>Закрепить главное окно</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="182"/>
        <source>Makes the Main window visible on all desktops</source>
        <translation>Сделать Главное окно видимым на всех рабочих столах</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="118"/>
        <source>Transparent when possible</source>
        <translation>Использовать прозрачность</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="89"/>
        <source>Alternate threaded view</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="90"/>
        <source>Separate online and offline users in threaded view</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="119"/>
        <source>Make the user window transparent when there is no scroll bar</source>
        <translation>Делает окно пользователей прозрачным, если это возможно (когда нет полосы прокрутки)</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="122"/>
        <source>Show group name if no messages</source>
        <translation>Показывать название группы
при отсутствии сообщений</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="123"/>
        <source>Show the name of the current group in the messages label when there are no new messages</source>
        <translation>Показывать название текущей группы при отсутствии новых сообщений</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="126"/>
        <source>Use system background color</source>
        <translation>Использовать системный цвет фона</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="129"/>
        <source>Allow scroll bar</source>
        <translation>Разрешить полосу прокрутки</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="130"/>
        <source>Allow the vertical scroll bar in the user list</source>
        <translation>Разрешает вертикальную полосу прокрутки в списке пользователей</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="134"/>
        <source>Frame style:</source>
        <translation>Стиль рамки:</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="136"/>
        <source>Override the skin setting for the frame style of the user window:
   0 (No frame), 1 (Box), 2 (Panel), 3 (WinPanel)
 + 16 (Plain), 32 (Raised), 48 (Sunken), 240 (Shadow)</source>
        <translation>Использовать указанный стиль рамки вместо того, что записан в конфигурации текущей темы.
   0 -- нет рамки; 1 -- коробка; 2 -- панель; 3 -- панель в стиле Windows
+ 16 -- плоская, 32 -- рельефная, 48 -- утопленная
+ 240 -- с тенью</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="149"/>
        <source>GUI Style:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="150"/>
        <source>Select look and feel for the GUI. Available styles may vary between systems.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="191"/>
        <source>Additional sorting:</source>
        <translation>Дополнительная сортировка:</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="192"/>
        <source>&lt;b&gt;none:&lt;/b&gt; - Don&apos;t sort online users by Status&lt;br&gt;
&lt;b&gt;status&lt;/b&gt; - Sort online users by status&lt;br&gt;
&lt;b&gt;status + last event&lt;/b&gt; - Sort online users by status and by last event&lt;br&gt;
&lt;b&gt;status + new messages&lt;/b&gt; - Sort online users by status and number of new messages</source>
        <translation>&lt;b&gt;нет:&lt;/b&gt; - не сортировать пользователей в сети по статусу&lt;br&gt;
&lt;b&gt;статус&lt;/b&gt; - сортировать пользователей в сети по статусу&lt;br&gt;
&lt;b&gt;статус + последнее сообщение&lt;/b&gt; - сортировать пользователей в сети по статусу и времени прихода последнего сообщения&lt;br&gt;
&lt;b&gt;статус + новые сообщения&lt;/b&gt; - сортировать пользователей в сети по статусу и количеству новых сообщений</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="198"/>
        <source>none</source>
        <translation>нет</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="199"/>
        <source>status</source>
        <translation>статус</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="200"/>
        <source>status + last event</source>
        <translation>статус + последнее событие</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="201"/>
        <source>status + new messages</source>
        <translation>статус + новые сообщения</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="221"/>
        <source>Column Configuration</source>
        <translation>Столбцы</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="225"/>
        <source>Title</source>
        <translation>Название</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="228"/>
        <source>Format</source>
        <translation>Формат</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="229"/>
        <source>The format string used to define what will appear in each column.&lt;br&gt;The following parameters can be used:</source>
        <translation>Строка, определяющая что будет показываться в каждом столбце.&lt;br&gt;Могут быть использованы следующие параметры:</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="233"/>
        <source>Width</source>
        <translation>Ширина</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="234"/>
        <source>The width of the column</source>
        <translation>Ширина столбца</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="236"/>
        <source>Alignment</source>
        <translation>Выравнивание</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="237"/>
        <source>The alignment of the column</source>
        <translation>Выравнивание текста в столбце</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="243"/>
        <source>Number of columns</source>
        <translation>Количество столбцов</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="262"/>
        <source>Left</source>
        <translation>Влево</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="263"/>
        <source>Right</source>
        <translation>Вправо</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="264"/>
        <source>Center</source>
        <translation>По центру</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="280"/>
        <source>Popup info</source>
        <translation>Всплывающая подсказка</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="283"/>
        <source>Picture</source>
        <translation>Картинка</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="285"/>
        <source>Alias</source>
        <translation>Псевдоним</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="287"/>
        <source>Full name</source>
        <translation>Имя и фамилия</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="289"/>
        <source>Email</source>
        <translation>Email</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="291"/>
        <source>Phone</source>
        <translation>Телефон</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="293"/>
        <source>Fax</source>
        <translation>Факс</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="295"/>
        <source>Cellular</source>
        <translation>Сотовый</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="297"/>
        <source>IP address</source>
        <translation>IP адрес</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="299"/>
        <source>Last online</source>
        <translation>Последний раз в сети</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="301"/>
        <source>Online time</source>
        <translation>Время в сети</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="303"/>
        <source>Idle time</source>
        <translation>Время бездействия</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="305"/>
        <source>Local time</source>
        <translation>Текущее время</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="307"/>
        <source>Protocol ID</source>
        <translation>Протокол</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="309"/>
        <source>Authorization status</source>
        <translation>Статус авторизации</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="312"/>
        <source>Automatic Update</source>
        <translation>Автообновление</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="315"/>
        <source>Contact information</source>
        <translation>Информация о контактах</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="316"/>
        <source>Automatically update users&apos; server stored information.</source>
        <translation>Автоматически обновлять информацию о пользователях с сервера.</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="319"/>
        <source>Info plugins</source>
        <translation>Инфо-модули</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="320"/>
        <source>Automatically update users&apos; Phone Book and Picture.</source>
        <translation>Автоматически обновлять телефонную книгу и картинку пользователей.</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="323"/>
        <source>Status plugins</source>
        <translation>Модули статуса</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="324"/>
        <source>Automatically update users&apos; Phone &quot;Follow Me&quot;, File Server and ICQphone status.</source>
        <translation>Автоматически обновлять статус &quot;Следуй за мной&quot;, файлового сервера и ICQphone.</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="74"/>
        <source>Contact List Appearance</source>
        <translation>Внешний вид</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="165"/>
        <source>Contact List Behaviour</source>
        <translation>Поведение</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="78"/>
        <source>Draw the box around each square in the user list</source>
        <translation>Рисовать рамку вокруг каждого пользователя в списке</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="226"/>
        <source>The string which will appear in the column header</source>
        <translation>Строка, которая будет отображаться в заголовке столбца</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="102"/>
        <source>Show phone icons</source>
        <translation>Показывать пиктограммы телефонов</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="103"/>
        <source>Show extended icons for phone statuses</source>
        <translation>Показывать ли пиктограммы для телефонов и телефонных статусов</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="185"/>
        <source>Move users when dragging to groups</source>
        <translation>Перемещать пользователя при перетаскивани</translation>
    </message>
    <message>
        <location filename="../src/settings/contactlist.cpp" line="186"/>
        <source>If checked a user will be moved when dragged to another group.
If not checked user will only be added to the new group.</source>
        <translation>Если отмечено, то пользователь будет перемещен в другую группу.
В противном случае, он будет добавлен в группу.</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::Settings::Events</name>
    <message>
        <location filename="../src/settings/events.cpp" line="54"/>
        <source>Events</source>
        <translation>События</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="56"/>
        <source>Sounds</source>
        <translation>Звуки</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="67"/>
        <source>Actions On Incoming Messages</source>
        <translation>Действия при входящем сообщении</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="70"/>
        <source>Bold message label</source>
        <translation>Выделять инфо-поле</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="71"/>
        <source>Show the message info label in bold font if there are incoming messages</source>
        <translation>Если есть непрочитанные сообщения, информационное поле будет выделяться жирным шрифтом</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="74"/>
        <source>Auto-focus message</source>
        <translation>Авто-фокус сообщений</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="75"/>
        <source>Automatically focus opened message windows.</source>
        <translation>Автоматически фокусировать открывающиеся окна сообщений.</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="78"/>
        <source>Auto-raise main window</source>
        <translation>Авто-всплытие главного окна</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="79"/>
        <source>Raise the main window on incoming messages</source>
        <translation>Поднимать основное окно при получении новых входящих сообщений</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="98"/>
        <source>Auto-popup urgent only</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="99"/>
        <source>Only auto-popup urgent messages.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="102"/>
        <source>Flash taskbar</source>
        <translation>Мигающая иконка в панели задач</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="103"/>
        <source>Flash the taskbar on incoming messages</source>
        <translation>Мигать иконкой при входящем сообщении</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="106"/>
        <source>Blink all events</source>
        <translation>Все сообщения мигают</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="107"/>
        <source>All incoming events will blink</source>
        <translation>Все приходящие сообщения будут мигать</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="110"/>
        <source>Blink urgent events</source>
        <translation>Срочные сообщения должны мигать</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="111"/>
        <source>Only urgent events will blink</source>
        <translation>Только срочные сообщения будут мигать</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="115"/>
        <source>Hot key:</source>
        <translation>Горячая клавиша:</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="117"/>
        <source>Hotkey to pop up the next pending message.
Enter the hotkey literally, like &quot;shift+f10&quot;, or &quot;none&quot; for disabling.</source>
        <translation>Горячая клавиша для открытия следующего сообщения.
Вводите добуквенно, например &quot;shift+f10&quot; или &quot;none&quot; для отказа.</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="125"/>
        <source>Paranoia</source>
        <translation>Игнорировать</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="128"/>
        <source>Ignore new users</source>
        <translation>Новых пользователей</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="129"/>
        <source>Determines if new users are automatically added to your list or must first request authorization.</source>
        <translation>Определяет, должны ли новые пользователи запрашивать Вашу авторизацию для добавления,
или они могут быть добавлены в список контактов автоматически.</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="132"/>
        <source>Ignore mass messages</source>
        <translation>Сообщения многим адресатам</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="133"/>
        <source>Determines if mass messages are ignored or not.</source>
        <translation>Определяет, игнорировать ли сообщения, отправленные сразу нескольким адресатам.</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="136"/>
        <source>Ignore web panel</source>
        <translation>Web-панель</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="137"/>
        <source>Determines if web panel messages are ignored or not.</source>
        <translation>Определяет, игнорировать ли сообщения, отправленные через web-панель.</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="140"/>
        <source>Ignore email pager</source>
        <translation>Email-пейджер</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="141"/>
        <source>Determines if email pager messages are ignored or not.</source>
        <translation>Определяет, игнорировать ли сообщения с Email-пейджера.</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="164"/>
        <source>Disable sound for active window</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="165"/>
        <source>Don&apos;t perform OnEvent command if chat window for user is currently active.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="83"/>
        <source>Auto-popup message:</source>
        <translation>Авто-показ сообщения:</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="86"/>
        <source>Never</source>
        <translation>Никогда</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="87"/>
        <source>Only when online</source>
        <translation>Когда в сети</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="88"/>
        <source>When online or away</source>
        <translation>Когда в сети или отошел</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="89"/>
        <source>When online, away or N/A</source>
        <translation>Когда в сети, отошел или недоступен</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="90"/>
        <source>Always except DND</source>
        <translation>Всегда, кроме &apos;не беспокоить&apos;</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="91"/>
        <source>Always</source>
        <translation>Всегда</translation>
    </message>
    <message>
        <location filename="../src/settings/events.cpp" line="92"/>
        <source>Select for which statuses incoming messages should open automatically.
Online also includes Free for chat.</source>
        <translation>Выбор статусов, при которых должны автоматически показываться входящие сообщения.
В сети также подразумевает &apos;свободен для чата&apos;.</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::Settings::General</name>
    <message>
        <location filename="../src/settings/general.cpp" line="49"/>
        <location filename="../src/settings/general.cpp" line="62"/>
        <source>Docking</source>
        <translation>Док</translation>
    </message>
    <message>
        <location filename="../src/settings/general.cpp" line="51"/>
        <location filename="../src/settings/general.cpp" line="124"/>
        <source>Fonts</source>
        <translation>Шрифты</translation>
    </message>
    <message>
        <location filename="../src/settings/general.cpp" line="65"/>
        <source>Use dock icon</source>
        <translation>Использовать док</translation>
    </message>
    <message>
        <location filename="../src/settings/general.cpp" line="66"/>
        <source>Controls whether or not the dockable icon should be displayed.</source>
        <translation>Определяет, должен ли отображаться док.</translation>
    </message>
    <message>
        <location filename="../src/settings/general.cpp" line="69"/>
        <source>Start hidden</source>
        <translation>Скрыть основное окно при запуске</translation>
    </message>
    <message>
        <location filename="../src/settings/general.cpp" line="70"/>
        <source>Start main window hidden. Only the dock icon will be visible.</source>
        <translation>Скрывать главное окно после запуска. Только док будет на экране.</translation>
    </message>
    <message>
        <location filename="../src/settings/general.cpp" line="73"/>
        <source>Default icon</source>
        <translation>Пиктограмма по умолчанию</translation>
    </message>
    <message>
        <location filename="../src/settings/general.cpp" line="76"/>
        <source>64 x 48 dock icon</source>
        <translation>Док 64 x 48</translation>
    </message>
    <message>
        <location filename="../src/settings/general.cpp" line="77"/>
        <source>Selects between the standard 64x64 icon used in the WindowMaker/Afterstep wharf
and a shorter 64x48 icon for use in the Gnome/KDE panel.</source>
        <translation>Выбор между пиктограммой размером 64x64, используемой в WindowMaker/Afterstep
и уменьшенной пиктограммой размером 64x48, используемой для панели Gnome/KDE.</translation>
    </message>
    <message>
        <location filename="../src/settings/general.cpp" line="81"/>
        <source>Themed icon</source>
        <translation>Тема</translation>
    </message>
    <message>
        <location filename="../src/settings/general.cpp" line="84"/>
        <source>Tray icon</source>
        <translation>Системная иконка</translation>
    </message>
    <message>
        <location filename="../src/settings/general.cpp" line="85"/>
        <source>Uses the freedesktop.org standard to dock a small icon into the system tray.
Works with many different window managers.</source>
        <translation>Использовать стандарт freedesktop.org для дока в оконных менеджерах.
Работает с большинством оконных менеджеров.</translation>
    </message>
    <message>
        <location filename="../src/settings/general.cpp" line="89"/>
        <source>Blink on events</source>
        <translation>Мигать при сообщении</translation>
    </message>
    <message>
        <location filename="../src/settings/general.cpp" line="90"/>
        <source>Make tray icon blink on unread incoming events.</source>
        <translation>Мигать иконкой при непрочитанных сообщениях.</translation>
    </message>
    <message>
        <location filename="../src/settings/general.cpp" line="128"/>
        <source>General:</source>
        <translation>Общий:</translation>
    </message>
    <message>
        <location filename="../src/settings/general.cpp" line="129"/>
        <source>Used for normal text.</source>
        <translation>Для обычного текста.</translation>
    </message>
    <message>
        <location filename="../src/settings/general.cpp" line="138"/>
        <source>Editing:</source>
        <translation>Редактирование:</translation>
    </message>
    <message>
        <location filename="../src/settings/general.cpp" line="139"/>
        <source>Used in message editor etc.</source>
        <translation>При наборе сообщения</translation>
    </message>
    <message>
        <location filename="../src/settings/general.cpp" line="94"/>
        <source>Show popup for Online notify</source>
        <translation>Оповещение при появлении в сети</translation>
    </message>
    <message>
        <location filename="../src/settings/general.cpp" line="95"/>
        <source>Show balloon popup message when contacts marked for online notify comes online.</source>
        <translation>Показывать всплывающую подсказку при появлении с сети пользователя, для которого включено оповещение.</translation>
    </message>
    <message>
        <location filename="../src/settings/general.cpp" line="156"/>
        <source>Fixed:</source>
        <translation>Моноширинный:</translation>
    </message>
    <message>
        <location filename="../src/settings/general.cpp" line="157"/>
        <source>Used in file editor and network log.</source>
        <translation>Используется в файловом редакторе и в окне отладки.</translation>
    </message>
    <message>
        <location filename="../src/settings/general.cpp" line="147"/>
        <source>History:</source>
        <translation>История:</translation>
    </message>
    <message>
        <location filename="../src/settings/general.cpp" line="148"/>
        <source>Used in message history.</source>
        <translation>Используется при просмотре истории.</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::Settings::Network</name>
    <message>
        <location filename="../src/settings/network.cpp" line="46"/>
        <source>Network</source>
        <translation>Сеть</translation>
    </message>
    <message>
        <location filename="../src/settings/network.cpp" line="48"/>
        <source>ICQ</source>
        <translation>ICQ</translation>
    </message>
    <message>
        <location filename="../src/settings/network.cpp" line="59"/>
        <source>Firewall</source>
        <translation>Брандмауэр</translation>
    </message>
    <message>
        <location filename="../src/settings/network.cpp" line="62"/>
        <source>I am behind a firewall</source>
        <translation>Сеть защищена брандмауэром</translation>
    </message>
    <message>
        <location filename="../src/settings/network.cpp" line="66"/>
        <source>I can receive direct connections</source>
        <translation>Со мной возможно соединиться напрямую</translation>
    </message>
    <message>
        <location filename="../src/settings/network.cpp" line="72"/>
        <source>Port range:</source>
        <translation>Диапазон портов:</translation>
    </message>
    <message>
        <location filename="../src/settings/network.cpp" line="73"/>
        <source>TCP port range for incoming connections.</source>
        <translation>Диапазон TCP портов для входящих соединений.</translation>
    </message>
    <message>
        <location filename="../src/settings/network.cpp" line="77"/>
        <location filename="../src/settings/network.cpp" line="84"/>
        <source>Auto</source>
        <translation>Автоматически</translation>
    </message>
    <message>
        <location filename="../src/settings/network.cpp" line="80"/>
        <source>to</source>
        <translation>до</translation>
    </message>
    <message>
        <location filename="../src/settings/network.cpp" line="90"/>
        <source>Proxy</source>
        <translation>Прокси сервер</translation>
    </message>
    <message>
        <location filename="../src/settings/network.cpp" line="93"/>
        <source>Use proxy server</source>
        <translation>Использовать прокси сервер</translation>
    </message>
    <message>
        <location filename="../src/settings/network.cpp" line="99"/>
        <source>Proxy type:</source>
        <translation>Тип прокси:</translation>
    </message>
    <message>
        <location filename="../src/settings/network.cpp" line="103"/>
        <source>HTTPS</source>
        <translation>HTTPS</translation>
    </message>
    <message>
        <location filename="../src/settings/network.cpp" line="108"/>
        <source>Proxy server:</source>
        <translation>Прокси сервер:</translation>
    </message>
    <message>
        <location filename="../src/settings/network.cpp" line="115"/>
        <source>Proxy server port:</source>
        <translation>Порт прокси сервера:</translation>
    </message>
    <message>
        <location filename="../src/settings/network.cpp" line="123"/>
        <source>Use authorization</source>
        <translation>Использовать авторизацию</translation>
    </message>
    <message>
        <location filename="../src/settings/network.cpp" line="126"/>
        <source>Username:</source>
        <translation>Имя пользователя:</translation>
    </message>
    <message>
        <location filename="../src/settings/network.cpp" line="133"/>
        <source>Password:</source>
        <translation>Пароль:</translation>
    </message>
    <message>
        <location filename="../src/settings/network.cpp" line="158"/>
        <source>Server Settings</source>
        <translation>Установки сервера</translation>
    </message>
    <message>
        <location filename="../src/settings/network.cpp" line="162"/>
        <source>ICQ server:</source>
        <translation>ICQ сервер:</translation>
    </message>
    <message>
        <location filename="../src/settings/network.cpp" line="169"/>
        <source>ICQ server port:</source>
        <translation>ICQ порт сервера:</translation>
    </message>
    <message>
        <location filename="../src/settings/network.cpp" line="178"/>
        <source>Connection</source>
        <translation>Соединение</translation>
    </message>
    <message>
        <location filename="../src/settings/network.cpp" line="181"/>
        <source>Reconnect after Uin clash</source>
        <translation>Восстанавливать соединение после обрыва</translation>
    </message>
    <message>
        <location filename="../src/settings/network.cpp" line="182"/>
        <source>Licq can reconnect you when you got disconnected because your Uin was used from another location. Check this if you want Licq to reconnect automatically.</source>
        <translation>Licq может восстанавливать соединение в случаях, когда Вас отключило из-за того,
что Ваш UIN начал использоваться на другой машине.
Установите этот флаг для того, чтобы это происходило автоматически.</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::Settings::Shortcuts</name>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="41"/>
        <location filename="../src/settings/shortcuts.cpp" line="43"/>
        <source>Shortcuts</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="54"/>
        <location filename="../src/settings/shortcuts.cpp" line="124"/>
        <source>Contact List Shortcuts</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="62"/>
        <location filename="../src/settings/shortcuts.cpp" line="132"/>
        <source>Action</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="62"/>
        <location filename="../src/settings/shortcuts.cpp" line="132"/>
        <source>Shortcut</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="75"/>
        <source>View message from user</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="76"/>
        <source>Send message to user</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="77"/>
        <source>Send URL to user</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="78"/>
        <source>Send file to user</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="79"/>
        <source>Send chat request to user</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="80"/>
        <source>Check user auto response</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="81"/>
        <source>View user history</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="83"/>
        <source>Open account manager</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="87"/>
        <source>Edit groups</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="88"/>
        <source>Redraw user window</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="89"/>
        <source>Set auto response</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="90"/>
        <source>Show network log</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="91"/>
        <source>Toggle mini mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="92"/>
        <source>Toggle show offline users</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="84"/>
        <source>Open all unread messages</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="85"/>
        <location filename="../src/settings/shortcuts.cpp" line="157"/>
        <source>Open next unread message</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="86"/>
        <source>Add group</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="93"/>
        <source>Toggle empty groups</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="94"/>
        <source>Toggle column headers visible</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="95"/>
        <source>Open settings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="96"/>
        <source>Hide contact list</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="97"/>
        <source>Exit Licq</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="99"/>
        <source>Switch status to online</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="100"/>
        <source>Switch status to away</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="101"/>
        <source>Switch status to N/A</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="102"/>
        <source>Switch status to occupied</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="103"/>
        <source>Switch status to DND</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="104"/>
        <source>Switch status to Free For Chat</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="105"/>
        <source>Switch status to offline</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="106"/>
        <source>Toggle status Invisible</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="145"/>
        <source>Open user menu</source>
        <translation type="unfinished">Открыть меню пользователя</translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="146"/>
        <source>Open history</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="147"/>
        <source>Open user information</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="148"/>
        <source>Select encoding</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="149"/>
        <source>Open / close secure channel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="150"/>
        <source>Select message type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="151"/>
        <source>Toggle send through server</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="152"/>
        <source>Toggle urgent</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="153"/>
        <source>Toggle multiple recipients</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="154"/>
        <source>Insert smiley</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="155"/>
        <source>Change text color</source>
        <translation type="unfinished">Изменить цвет текста</translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="156"/>
        <source>Change background color</source>
        <translation type="unfinished">Изменить цвет фона</translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="159"/>
        <source>Switch to tab 1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="160"/>
        <source>Switch to tab 2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="161"/>
        <source>Switch to tab 3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="162"/>
        <source>Switch to tab 4</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="163"/>
        <source>Switch to tab 5</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="164"/>
        <source>Switch to tab 6</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="165"/>
        <source>Switch to tab 7</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="166"/>
        <source>Switch to tab 8</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="167"/>
        <source>Switch to tab 9</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/shortcuts.cpp" line="168"/>
        <source>Switch to tab 10</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>LicqQtGui::Settings::Skin</name>
    <message>
        <location filename="../src/settings/skin.cpp" line="74"/>
        <source>Skin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/skin.cpp" line="104"/>
        <source>Skin Selection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/skin.cpp" line="106"/>
        <source>Icon Selection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/settings/skin.cpp" line="112"/>
        <source>S&amp;kins:</source>
        <translation type="unfinished">&amp;Внешний вид:</translation>
    </message>
    <message>
        <location filename="../src/settings/skin.cpp" line="121"/>
        <source>Use this combo box to select one of the available skins</source>
        <translation type="unfinished">Используйте это выпадающее меню для выбора внешнего вида</translation>
    </message>
    <message>
        <location filename="../src/settings/skin.cpp" line="132"/>
        <source>Icons:</source>
        <translation type="unfinished">Пиктограммы:</translation>
    </message>
    <message>
        <location filename="../src/settings/skin.cpp" line="140"/>
        <source>Use this combo box to select one of the available icon sets</source>
        <translation type="unfinished">Используйте это выпадающее меню для выбора набора пиктограмм</translation>
    </message>
    <message>
        <location filename="../src/settings/skin.cpp" line="153"/>
        <source>Extended Icons:</source>
        <translation type="unfinished">Дополнительные:</translation>
    </message>
    <message>
        <location filename="../src/settings/skin.cpp" line="161"/>
        <source>Use this combo box to select one of the available extended icon sets</source>
        <translation type="unfinished">Используйте это выпадающее меню для выбора набора дополнительных пиктограмм</translation>
    </message>
    <message>
        <location filename="../src/settings/skin.cpp" line="174"/>
        <source>Emoticons:</source>
        <translation type="unfinished">Эмотиконы:</translation>
    </message>
    <message>
        <location filename="../src/settings/skin.cpp" line="182"/>
        <source>Use this combo box to select one of the available emoticon icon sets</source>
        <translation type="unfinished">Используйте это выпадающее меню для выбора набора эмотиконов</translation>
    </message>
    <message>
        <location filename="../src/settings/skin.cpp" line="194"/>
        <source>&amp;Edit Skin</source>
        <translation type="unfinished">&amp;Редактировать внешний вид</translation>
    </message>
    <message>
        <location filename="../src/settings/skin.cpp" line="214"/>
        <location filename="../src/settings/skin.cpp" line="275"/>
        <source>Error</source>
        <translation type="unfinished">Ошибка</translation>
    </message>
    <message>
        <location filename="../src/settings/skin.cpp" line="289"/>
        <location filename="../src/settings/skin.cpp" line="314"/>
        <source>Unable to open icons file
%1
Iconset &apos;%2&apos; has been disabled.</source>
        <translation type="unfinished">Невозможно открыть файл пиктограммы
%1
Набор пиктограмм &apos;%2&apos; отключен.</translation>
    </message>
    <message>
        <location filename="../src/settings/skin.cpp" line="352"/>
        <source>Unable to load icons
%1.</source>
        <translation type="unfinished">Невозможно загрузить пиктограммы
%1.</translation>
    </message>
    <message>
        <location filename="../src/settings/skin.cpp" line="356"/>
        <source>Unable to load extended icons
%1.</source>
        <translation type="unfinished">Невозможно загрузить дополнительные пиктограммы
%1.</translation>
    </message>
    <message>
        <location filename="../src/settings/skin.cpp" line="405"/>
        <source>Unable to open icons file
%1</source>
        <translation type="unfinished">Ошибка при открытии файла пиктограмм
%1</translation>
    </message>
    <message>
        <location filename="../src/settings/skin.cpp" line="520"/>
        <location filename="../src/settings/skin.cpp" line="527"/>
        <source>System</source>
        <translation type="unfinished">Система</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::Settings::Status</name>
    <message>
        <location filename="../src/settings/status.cpp" line="61"/>
        <source>Status</source>
        <translation>Статус</translation>
    </message>
    <message>
        <location filename="../src/settings/status.cpp" line="63"/>
        <source>Auto Response</source>
        <translation>Авто-ответ</translation>
    </message>
    <message>
        <location filename="../src/settings/status.cpp" line="74"/>
        <source>Startup</source>
        <translation>Запуск</translation>
    </message>
    <message>
        <location filename="../src/settings/status.cpp" line="103"/>
        <source>Automatically log on when first starting up.</source>
        <translation>Автоматически подключаться к сети с заданным статусом при запуске.</translation>
    </message>
    <message>
        <location filename="../src/settings/status.cpp" line="106"/>
        <source>Invisible</source>
        <translation>Невидимый</translation>
    </message>
    <message>
        <location filename="../src/settings/status.cpp" line="115"/>
        <source>Auto Change Status</source>
        <translation>Автоматически сменить статус</translation>
    </message>
    <message>
        <location filename="../src/settings/status.cpp" line="119"/>
        <source>Auto Away:</source>
        <translation>Переключение в режим &apos;отошел&apos;:</translation>
    </message>
    <message>
        <location filename="../src/settings/status.cpp" line="120"/>
        <source>Number of minutes of inactivity after which to automatically be marked &quot;away&quot;.  Set to &quot;0&quot; to disable.</source>
        <translation>Время бездействия (в минутах), после которого Ваш статус автоматически меняется на &apos;отошел&apos;.
&quot;0&quot; отключает эту функцию.</translation>
    </message>
    <message>
        <location filename="../src/settings/status.cpp" line="125"/>
        <location filename="../src/settings/status.cpp" line="137"/>
        <location filename="../src/settings/status.cpp" line="149"/>
        <source>Never</source>
        <translation>Никогда</translation>
    </message>
    <message>
        <location filename="../src/settings/status.cpp" line="131"/>
        <source>Auto N/A:</source>
        <translation>Переключение в режим &apos;недоступен&apos;:</translation>
    </message>
    <message>
        <location filename="../src/settings/status.cpp" line="132"/>
        <source>Number of minutes of inactivity after which to automatically be marked &quot;not available&quot;.  Set to &quot;0&quot; to disable.</source>
        <translation>Время бездействия (в минутах), после которого Ваш статус автоматически меняется на &apos;недоступен&apos;.
&quot;0&quot; отключает эту функцию.</translation>
    </message>
    <message>
        <location filename="../src/settings/status.cpp" line="143"/>
        <source>Auto Offline:</source>
        <translation>Отключение:</translation>
    </message>
    <message>
        <location filename="../src/settings/status.cpp" line="144"/>
        <source>Number of minutes of inactivity after which to automatically go offline.  Set to &quot;0&quot; to disable.</source>
        <translation>Время бездействия (в минутах), после которого Вы автоматически отключаетесь от сети.
&quot;0&quot; отключает эту функцию.</translation>
    </message>
    <message>
        <location filename="../src/settings/status.cpp" line="167"/>
        <source>Default Auto Response Messages</source>
        <translation>Сообщения для авто-ответа</translation>
    </message>
    <message>
        <location filename="../src/settings/status.cpp" line="172"/>
        <source>Status:</source>
        <translation>Статус:</translation>
    </message>
    <message>
        <location filename="../src/settings/status.cpp" line="184"/>
        <source>Preset slot:</source>
        <translation>Заготовки:</translation>
    </message>
    <message>
        <location filename="../src/settings/status.cpp" line="198"/>
        <source>Hints</source>
        <translation>Подсказки</translation>
    </message>
    <message>
        <location filename="../src/settings/status.cpp" line="204"/>
        <source>Save</source>
        <translation>Сохранить</translation>
    </message>
    <message>
        <location filename="../src/settings/status.cpp" line="234"/>
        <location filename="../src/settings/status.cpp" line="242"/>
        <source>Previous Message</source>
        <translation>Предыдущее сообщение</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::SettingsDlg</name>
    <message>
        <location filename="../src/settings/settingsdlg.cpp" line="63"/>
        <source>Licq - Settings</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>LicqQtGui::ShortcutButton</name>
    <message>
        <location filename="../src/widgets/shortcutedit.cpp" line="87"/>
        <source>Input</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/widgets/shortcutedit.cpp" line="89"/>
        <source>Shift+</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/widgets/shortcutedit.cpp" line="91"/>
        <source>Ctrl+</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/widgets/shortcutedit.cpp" line="93"/>
        <source>Alt+</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/widgets/shortcutedit.cpp" line="95"/>
        <source>Meta+</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/widgets/shortcutedit.cpp" line="106"/>
        <source>None</source>
        <translation type="unfinished">Отключить</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::ShortcutEdit</name>
    <message>
        <location filename="../src/widgets/shortcutedit.cpp" line="45"/>
        <source>X</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>LicqQtGui::ShowAwayMsgDlg</name>
    <message>
        <location filename="../src/dialogs/showawaymsgdlg.cpp" line="88"/>
        <source>%1 Response for %2</source>
        <translation>&apos;%1&apos; ответ для &apos;%2&apos;</translation>
    </message>
    <message>
        <location filename="../src/dialogs/showawaymsgdlg.cpp" line="72"/>
        <source>&amp;Show Again</source>
        <translation>&amp;Показывать снова</translation>
    </message>
    <message>
        <location filename="../src/dialogs/showawaymsgdlg.cpp" line="143"/>
        <source>error</source>
        <translation>ошибка</translation>
    </message>
    <message>
        <location filename="../src/dialogs/showawaymsgdlg.cpp" line="137"/>
        <source>failed</source>
        <translation>неудачно</translation>
    </message>
    <message>
        <location filename="../src/dialogs/showawaymsgdlg.cpp" line="131"/>
        <source>refused</source>
        <translation>отказано</translation>
    </message>
    <message>
        <location filename="../src/dialogs/showawaymsgdlg.cpp" line="140"/>
        <source>timed out</source>
        <translation>время ожидания вышло</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::StatsDlg</name>
    <message>
        <location filename="../src/dialogs/statsdlg.cpp" line="46"/>
        <source>Licq - Statistics</source>
        <translation>Licq - Статистика</translation>
    </message>
    <message>
        <location filename="../src/dialogs/statsdlg.cpp" line="86"/>
        <source>Daemon Statistics</source>
        <translation>Статистика демона</translation>
    </message>
    <message>
        <location filename="../src/dialogs/statsdlg.cpp" line="87"/>
        <source>Up since</source>
        <translation>Работает с</translation>
    </message>
    <message>
        <location filename="../src/dialogs/statsdlg.cpp" line="89"/>
        <source>Last reset</source>
        <translation>Последний сброс</translation>
    </message>
    <message>
        <location filename="../src/dialogs/statsdlg.cpp" line="91"/>
        <source>Number of users</source>
        <translation>Количество пользователей</translation>
    </message>
    <message>
        <location filename="../src/dialogs/statsdlg.cpp" line="93"/>
        <source>Event Statistics</source>
        <translation>Статистика событий</translation>
    </message>
    <message>
        <location filename="../src/dialogs/statsdlg.cpp" line="94"/>
        <source>Today</source>
        <translation>Сегодня</translation>
    </message>
    <message>
        <location filename="../src/dialogs/statsdlg.cpp" line="95"/>
        <source>Total</source>
        <translation>Всего</translation>
    </message>
    <message>
        <location filename="../src/dialogs/statsdlg.cpp" line="118"/>
        <source>Do you really want to
reset your statistics?</source>
        <translation>Вы действительно хотите
обнулить статистику?</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::SystemMenu</name>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="187"/>
        <source>&amp;About...</source>
        <translation>О &amp;программе...</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="110"/>
        <source>&amp;Add User...</source>
        <translation>&amp;Добавить пользователя...</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="113"/>
        <source>A&amp;uthorize User...</source>
        <translation>&amp;Авторизовать пользователя...</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="135"/>
        <source>Available</source>
        <translation>Доступен</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="136"/>
        <source>Busy</source>
        <translation>Занят</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="96"/>
        <source>Clear All</source>
        <translation>Очистить все</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="81"/>
        <source>Debug Level</source>
        <translation>Уровень отладки</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="134"/>
        <source>Don&apos;t Show</source>
        <translation>Скрыть</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="118"/>
        <source>Edit &amp;Groups...</source>
        <translation>&amp;Редактировать группы...</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="90"/>
        <source>Errors</source>
        <translation>Ошибки</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="212"/>
        <source>E&amp;xit</source>
        <translation>&amp;Выход</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="165"/>
        <source>&amp;Group</source>
        <translation>&amp;Группа</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="185"/>
        <source>&amp;Help</source>
        <translation>Помо&amp;щь</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="186"/>
        <source>&amp;Hints...</source>
        <translation>&amp;Подсказки...</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="198"/>
        <source>&amp;Mini Mode</source>
        <translation>Режим &apos;&amp;мини&apos;</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="197"/>
        <source>&amp;Network Window...</source>
        <translation>Окно &amp;отладки...</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="205"/>
        <source>&amp;Plugin Manager...</source>
        <translation>Менеджер мо&amp;дулей...</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="117"/>
        <source>&amp;Popup All Messages...</source>
        <translation>&amp;Вывести все сообщения...</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="122"/>
        <source>&amp;Redraw User Window</source>
        <translation>&amp;Обновить окно пользователей</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="114"/>
        <source>Re&amp;quest Authorization...</source>
        <translation>Запросит&amp;ь авторизацию...</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="123"/>
        <source>&amp;Save All Users</source>
        <translation>&amp;Сохранить всех пользователей</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="210"/>
        <source>Sa&amp;ve Settings</source>
        <translation>Сохранить &amp;настройки</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="112"/>
        <source>S&amp;earch for User...</source>
        <translation>&amp;Поиск пользователя...</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="95"/>
        <source>Set All</source>
        <translation>Установить все</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="195"/>
        <source>Set &amp;Auto Response...</source>
        <translation>Установить &amp;авто-ответ...</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="200"/>
        <source>Show Offline &amp;Users</source>
        <translation>Показывать о&amp;тключенных</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="188"/>
        <source>&amp;Statistics...</source>
        <translation>&amp;Статистика...</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="140"/>
        <source>&amp;Status</source>
        <translation>&amp;Статус</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="88"/>
        <source>Status Info</source>
        <translation>Статус</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="89"/>
        <source>Unknown Packets</source>
        <translation>Неизвестные пакеты</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="120"/>
        <source>Update All Users</source>
        <translation>Обновить всех пользователей</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="121"/>
        <source>Update Current Group</source>
        <translation>Обновить текущую группу</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="101"/>
        <source>&amp;View System Messages...</source>
        <translation>&amp;Просмотр системных сообщений...</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="91"/>
        <source>Warnings</source>
        <translation>Предупреждения</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="92"/>
        <source>Debug</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="93"/>
        <source>Raw Packets</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="100"/>
        <source>S&amp;ystem Functions</source>
        <translation>С&amp;истемные операции</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="104"/>
        <source>&amp;Account Manager...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="109"/>
        <source>User &amp;Functions</source>
        <translation>По&amp;льзовательские операции</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="111"/>
        <source>A&amp;dd Group...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="115"/>
        <source>ICQ Ra&amp;ndom Chat...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="126"/>
        <source>ICQ Phone &quot;Follow Me&quot;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="202"/>
        <source>Sh&amp;ow Empty Groups</source>
        <translation>Показывать &amp;пустые группы</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="204"/>
        <source>S&amp;ettings...</source>
        <translation>&amp;Установки...</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="206"/>
        <source>GPG &amp;Key Manager...</source>
        <translation>Менеджер &amp;ключей GPG...</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::SystemMenuPrivate::OwnerData</name>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="615"/>
        <source>&amp;Info...</source>
        <translation>Ин&amp;формация...</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="616"/>
        <source>View &amp;History...</source>
        <translation>Просмотр &amp;истории...</translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="619"/>
        <source>&amp;Security Options...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/systemmenu.cpp" line="620"/>
        <source>&amp;Random Chat Group...</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>LicqQtGui::ThemedDockIcon</name>
    <message>
        <location filename="../src/dockicons/themeddockicon.cpp" line="80"/>
        <source>Unable to load dock theme file:
(%1)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/dockicons/themeddockicon.cpp" line="94"/>
        <source>Dock theme unspecified image: %1</source>
        <translation>Неуказанная картинка в теме: %1</translation>
    </message>
    <message>
        <location filename="../src/dockicons/themeddockicon.cpp" line="100"/>
        <location filename="../src/dockicons/themeddockicon.cpp" line="133"/>
        <source>Unable to load dock theme image: %1</source>
        <translation>Невозможно загрузить картинку из темы: %1</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::TimeZoneEdit</name>
    <message>
        <location filename="../src/widgets/timezoneedit.cpp" line="43"/>
        <source>Unknown</source>
        <translation>Неизвестно</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::TreePager</name>
    <message>
        <location filename="../src/widgets/treepager.cpp" line="38"/>
        <source>Categories</source>
        <translation>Категории</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::UserCodec</name>
    <message>
        <location filename="../src/helpers/usercodec.cpp" line="37"/>
        <location filename="../src/helpers/usercodec.cpp" line="38"/>
        <source>Arabic</source>
        <translation>Арабский</translation>
    </message>
    <message>
        <location filename="../src/helpers/usercodec.cpp" line="40"/>
        <location filename="../src/helpers/usercodec.cpp" line="41"/>
        <source>Baltic</source>
        <translation>Прибалтийские</translation>
    </message>
    <message>
        <location filename="../src/helpers/usercodec.cpp" line="43"/>
        <location filename="../src/helpers/usercodec.cpp" line="44"/>
        <source>Central European</source>
        <translation>Центральная европа</translation>
    </message>
    <message>
        <location filename="../src/helpers/usercodec.cpp" line="46"/>
        <source>Chinese</source>
        <translation>Китайский</translation>
    </message>
    <message>
        <location filename="../src/helpers/usercodec.cpp" line="47"/>
        <source>Chinese Traditional</source>
        <translation>Традиционный китайский</translation>
    </message>
    <message>
        <location filename="../src/helpers/usercodec.cpp" line="49"/>
        <location filename="../src/helpers/usercodec.cpp" line="50"/>
        <location filename="../src/helpers/usercodec.cpp" line="51"/>
        <source>Cyrillic</source>
        <translation>Кириллица</translation>
    </message>
    <message>
        <location filename="../src/helpers/usercodec.cpp" line="53"/>
        <source>Esperanto</source>
        <translation>Эсперанто</translation>
    </message>
    <message>
        <location filename="../src/helpers/usercodec.cpp" line="55"/>
        <location filename="../src/helpers/usercodec.cpp" line="56"/>
        <source>Greek</source>
        <translation>Греческая</translation>
    </message>
    <message>
        <location filename="../src/helpers/usercodec.cpp" line="59"/>
        <location filename="../src/helpers/usercodec.cpp" line="60"/>
        <source>Hebrew</source>
        <translation>Иврит</translation>
    </message>
    <message>
        <location filename="../src/helpers/usercodec.cpp" line="62"/>
        <location filename="../src/helpers/usercodec.cpp" line="63"/>
        <location filename="../src/helpers/usercodec.cpp" line="64"/>
        <source>Japanese</source>
        <translation>Японский</translation>
    </message>
    <message>
        <location filename="../src/helpers/usercodec.cpp" line="66"/>
        <source>Korean</source>
        <translation>Корейский</translation>
    </message>
    <message>
        <location filename="../src/helpers/usercodec.cpp" line="72"/>
        <source>Tamil</source>
        <translation>Тамильский</translation>
    </message>
    <message>
        <location filename="../src/helpers/usercodec.cpp" line="74"/>
        <source>Thai</source>
        <translation>Тайский</translation>
    </message>
    <message>
        <location filename="../src/helpers/usercodec.cpp" line="76"/>
        <location filename="../src/helpers/usercodec.cpp" line="77"/>
        <source>Turkish</source>
        <translation>Турецкий</translation>
    </message>
    <message>
        <location filename="../src/helpers/usercodec.cpp" line="79"/>
        <source>Ukrainian</source>
        <translation>Украинский</translation>
    </message>
    <message>
        <location filename="../src/helpers/usercodec.cpp" line="34"/>
        <source>Unicode</source>
        <translation>Юникод</translation>
    </message>
    <message>
        <location filename="../src/helpers/usercodec.cpp" line="35"/>
        <source>Unicode-16</source>
        <translation>Юникод-16</translation>
    </message>
    <message>
        <location filename="../src/helpers/usercodec.cpp" line="68"/>
        <location filename="../src/helpers/usercodec.cpp" line="69"/>
        <location filename="../src/helpers/usercodec.cpp" line="70"/>
        <source>Western European</source>
        <translation>Восточная европа</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::UserDlg</name>
    <message>
        <location filename="../src/userdlg/userdlg.cpp" line="69"/>
        <source>Menu</source>
        <translation>Меню</translation>
    </message>
    <message>
        <location filename="../src/userdlg/userdlg.cpp" line="88"/>
        <source>Send</source>
        <translation>Отправить</translation>
    </message>
    <message>
        <location filename="../src/userdlg/userdlg.cpp" line="94"/>
        <source>Retrieve</source>
        <translation>Получить</translation>
    </message>
    <message>
        <location filename="../src/userdlg/userdlg.cpp" line="94"/>
        <source>Update</source>
        <translation>Обновить</translation>
    </message>
    <message>
        <location filename="../src/userdlg/userdlg.cpp" line="112"/>
        <location filename="../src/userdlg/userdlg.cpp" line="119"/>
        <source>Licq - Info </source>
        <translation>Licq - Информация </translation>
    </message>
    <message>
        <location filename="../src/userdlg/userdlg.cpp" line="119"/>
        <source>INVALID USER</source>
        <translation>НЕПРАВИЛЬНЫЙ ПОЛЬЗОВАТЕЛЬ</translation>
    </message>
    <message>
        <location filename="../src/userdlg/userdlg.cpp" line="176"/>
        <source>Updating...</source>
        <translation>Обновляется...</translation>
    </message>
    <message>
        <location filename="../src/userdlg/userdlg.cpp" line="189"/>
        <source>Updating server...</source>
        <translation>Обновляется на сервере...</translation>
    </message>
    <message>
        <location filename="../src/userdlg/userdlg.cpp" line="250"/>
        <location filename="../src/userdlg/userdlg.cpp" line="266"/>
        <source>error</source>
        <translation>ошибка</translation>
    </message>
    <message>
        <location filename="../src/userdlg/userdlg.cpp" line="257"/>
        <source>done</source>
        <translation>готово</translation>
    </message>
    <message>
        <location filename="../src/userdlg/userdlg.cpp" line="260"/>
        <source>failed</source>
        <translation>неудачно</translation>
    </message>
    <message>
        <location filename="../src/userdlg/userdlg.cpp" line="263"/>
        <source>timed out</source>
        <translation>истекло время ожидания</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::UserEventCommon</name>
    <message>
        <location filename="../src/userevents/usereventcommon.cpp" line="128"/>
        <source>Encoding</source>
        <translation>Кодировка</translation>
    </message>
    <message>
        <location filename="../src/userevents/usereventcommon.cpp" line="124"/>
        <source>History...</source>
        <translation>История...</translation>
    </message>
    <message>
        <location filename="../src/userevents/usereventcommon.cpp" line="119"/>
        <source>Menu</source>
        <translation>Меню</translation>
    </message>
    <message>
        <location filename="../src/userevents/usereventcommon.cpp" line="258"/>
        <source>Open / Close secure channel</source>
        <translation>Открыть/закрыть безопасное соединение</translation>
    </message>
    <message>
        <location filename="../src/userevents/usereventcommon.cpp" line="254"/>
        <source>Open user menu</source>
        <translation>Открыть меню пользователя</translation>
    </message>
    <message>
        <location filename="../src/userevents/usereventcommon.cpp" line="133"/>
        <source>Secure Channel</source>
        <translation>Защищенный канал</translation>
    </message>
    <message>
        <location filename="../src/userevents/usereventcommon.cpp" line="257"/>
        <source>Select the text encoding used for outgoing messages.</source>
        <translation>Выбрать кодировку для исходящих сообщений.</translation>
    </message>
    <message>
        <location filename="../src/userevents/usereventcommon.cpp" line="255"/>
        <source>Show user history</source>
        <translation>Показать историю</translation>
    </message>
    <message>
        <location filename="../src/userevents/usereventcommon.cpp" line="256"/>
        <source>Show user information</source>
        <translation>Показать информацию о пользователе</translation>
    </message>
    <message>
        <location filename="../src/userevents/usereventcommon.cpp" line="372"/>
        <source>Unable to load encoding &lt;b&gt;%1&lt;/b&gt;.&lt;br&gt;Message contents may appear garbled.</source>
        <translation>Невозможно загрузить кодировку &lt;b&gt;%1&lt;/b&gt;. Отображение может быть некорректным.</translation>
    </message>
    <message>
        <location filename="../src/userevents/usereventcommon.cpp" line="112"/>
        <location filename="../src/userevents/usereventcommon.cpp" line="297"/>
        <source>Unknown</source>
        <translation>Неизвестно</translation>
    </message>
    <message>
        <location filename="../src/userevents/usereventcommon.cpp" line="125"/>
        <source>User Info...</source>
        <translation>Информация о пользователе...</translation>
    </message>
    <message>
        <location filename="../src/userevents/usereventcommon.cpp" line="109"/>
        <source>User&apos;s current local time</source>
        <translation>Текущее время</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::UserEventTabDlg</name>
    <message>
        <location filename="../src/userevents/usereventtabdlg.cpp" line="176"/>
        <source>[UNKNOWN_USER]</source>
        <translation>[НЕИЗВЕСТНЫЙ ПОЛЬЗОВАТЕЛЬ]</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::UserMenu</name>
    <message>
        <location filename="../src/core/usermenu.cpp" line="105"/>
        <source>Accept in Away</source>
        <translation>Принимать, если в режиме &apos;Отошел&apos;</translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="108"/>
        <source>Accept in Do Not Disturb</source>
        <translation>Принимать, если в режиме &apos;Не беспокоить&apos;</translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="106"/>
        <source>Accept in Not Available</source>
        <translation>Принимать, если в режиме &quot;Недоступен&quot;</translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="107"/>
        <source>Accept in Occupied</source>
        <translation>Принимать, если в режиме &apos;Занят&apos;</translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="110"/>
        <source>Auto Accept Chats</source>
        <translation>Автоматически принимать запросы чата</translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="109"/>
        <source>Auto Accept Files</source>
        <translation>Автоматически принимать файлы</translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="79"/>
        <source>Send &amp;Message...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="80"/>
        <source>Send &amp;URL...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="81"/>
        <source>Send &amp;Chat Request...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="82"/>
        <source>Send &amp;File Transfer...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="83"/>
        <source>Send Contact &amp;List...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="84"/>
        <source>Send &amp;Authorization...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="85"/>
        <source>Send Authorization Re&amp;quest...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="86"/>
        <source>Send &amp;SMS...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="111"/>
        <source>Auto Request Secure</source>
        <translation>Автоматически запрашивать защищенное соединение</translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="118"/>
        <source>Away to User</source>
        <translation>В режиме &apos;Отошел&apos; для пользователя</translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="163"/>
        <source>&amp;View Event...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="172"/>
        <source>Add to List</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Check %1 Response</source>
        <translation type="obsolete">Посмотреть авто-ответ %1</translation>
    </message>
    <message>
        <source>Check Auto Response</source>
        <translation type="obsolete">Посмотреть авто-ответ</translation>
    </message>
    <message>
        <source>Close &amp;Secure Channel</source>
        <translation type="obsolete">&amp;Закрыть защищенный канал</translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="169"/>
        <source>Custom Auto Response...</source>
        <translation>Персональный авто-ответ...</translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="121"/>
        <source>Do Not Disturb to User</source>
        <translation>В режиме &apos;Не беспокоить&apos; для пользователя</translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="177"/>
        <source>Set GPG key...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="181"/>
        <source>View &amp;History...</source>
        <translation type="unfinished">Просмотр &amp;истории...</translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="182"/>
        <source>&amp;Info...</source>
        <translation type="unfinished">Ин&amp;формация...</translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="259"/>
        <source>Check %1 Response...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="262"/>
        <source>Check Auto Response...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="290"/>
        <source>Close &amp;Secure Channel...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="295"/>
        <source>Request &amp;Secure Channel...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="624"/>
        <source>Do you really want to add
%1 (%2)
to your ignore list?</source>
        <translation>Вы действительно хотите добавить
%1 (%2)
в список игнорируемых?</translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="133"/>
        <source>Edit User Group</source>
        <translation>Изменить группы</translation>
    </message>
    <message>
        <source>&amp;Info</source>
        <translation type="obsolete">&amp;Информация</translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="98"/>
        <source>Misc Modes</source>
        <translation>Различные режимы</translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="119"/>
        <source>Not Available to User</source>
        <translation>В режиме &apos;Недоступен&apos; для пользователя</translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="120"/>
        <source>Occupied to User</source>
        <translation>В режиме &apos;Занят&apos; для пользователя</translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="117"/>
        <source>Online to User</source>
        <translation>В режиме &apos;В сети&apos; для пользователя</translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="175"/>
        <source>Remove From List</source>
        <translation>Удалить из списка</translation>
    </message>
    <message>
        <source>Request &amp;Secure Channel</source>
        <translation type="obsolete">Зап&amp;росить защищенный канал</translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="73"/>
        <source>Send</source>
        <translation>Отправить</translation>
    </message>
    <message>
        <source>Send &amp;Authorization</source>
        <translation type="obsolete">&amp;Авторизовать пользователя</translation>
    </message>
    <message>
        <source>Send Authorization Re&amp;quest</source>
        <translation type="obsolete">За&amp;просить авторизацию</translation>
    </message>
    <message>
        <source>Send &amp;Chat Request</source>
        <translation type="obsolete">Отправить &amp;запрос чата</translation>
    </message>
    <message>
        <source>Send Contact &amp;List</source>
        <translation type="obsolete">Отправить список &amp;контактов</translation>
    </message>
    <message>
        <source>Send &amp;File Transfer</source>
        <translation type="obsolete">Отправить &amp;файл</translation>
    </message>
    <message>
        <source>Send &amp;Message</source>
        <translation type="obsolete">Отправить &amp;сообщение</translation>
    </message>
    <message>
        <source>Send &amp;SMS</source>
        <translation type="obsolete">Послать &amp;SMS</translation>
    </message>
    <message>
        <source>Send &amp;URL</source>
        <translation type="obsolete">Отправить &amp;ссылку</translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="156"/>
        <source>Server Group</source>
        <translation>Группа на сервере</translation>
    </message>
    <message>
        <source>Set GPG key</source>
        <translation type="obsolete">Установить GPG ключ</translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="173"/>
        <source>Toggle &amp;Floaty</source>
        <translation>&amp;Вкл/Выкл плавающее окно</translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="92"/>
        <source>Update File Server Status</source>
        <translation>Обновить статус файл сервера</translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="91"/>
        <source>Update ICQphone Status</source>
        <translation>Обновить статус ICQphone</translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="88"/>
        <source>Update Info Plugin List</source>
        <translation>Обновить список инфо-модулей</translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="90"/>
        <source>Update Phone &quot;Follow Me&quot; Status</source>
        <translation>Обновить статус &quot;Следуй за мной&quot;</translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="89"/>
        <source>Update Status Plugin List</source>
        <translation>Обновить список статус-модулей</translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="112"/>
        <source>Use GPG Encryption</source>
        <translation>Использовать GPG шифрование</translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="115"/>
        <source>Use Real Ip (LAN)</source>
        <translation>Использовать реальный IP адрес в ЛВС</translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="125"/>
        <source>U&amp;tilities</source>
        <translation>&amp;Утилиты</translation>
    </message>
    <message>
        <source>&amp;View Event</source>
        <translation type="obsolete">&amp;Посмотреть сообщение</translation>
    </message>
    <message>
        <source>View &amp;History</source>
        <translation type="obsolete">Просмотр &amp;истории</translation>
    </message>
    <message>
        <location filename="../src/core/usermenu.cpp" line="180"/>
        <source>&amp;Copy User ID</source>
        <translation>Скопи&amp;ровать идентификатор пользователя</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::UserPages::Info</name>
    <message>
        <location filename="../src/userdlg/info.cpp" line="93"/>
        <source>Info</source>
        <translation>Информация</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="97"/>
        <location filename="../src/userdlg/info.cpp" line="372"/>
        <source>More</source>
        <translation>Еще</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="99"/>
        <location filename="../src/userdlg/info.cpp" line="637"/>
        <source>More II</source>
        <translation>Больше</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="101"/>
        <location filename="../src/userdlg/info.cpp" line="809"/>
        <source>Work</source>
        <translation>Работа</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="103"/>
        <location filename="../src/userdlg/info.cpp" line="961"/>
        <source>About</source>
        <translation>О себе</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="105"/>
        <source>Phone Book</source>
        <translation>Телефонная книга</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="108"/>
        <location filename="../src/userdlg/info.cpp" line="1208"/>
        <source>Picture</source>
        <translation>Картинка</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="110"/>
        <location filename="../src/userdlg/info.cpp" line="1297"/>
        <source>Last</source>
        <translation>Последний раз</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="113"/>
        <source>KDE Adressbook</source>
        <translation>Адресная книга KDE</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="172"/>
        <source>General Information</source>
        <translation>Общая информация</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="176"/>
        <source>Alias:</source>
        <translation>Псевдоним:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="182"/>
        <source>Keep Alias on Update</source>
        <translation>Не менять псевдоним при обновлении</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="183"/>
        <source>Normally Licq overwrites the Alias when updating user details.
Check this if you want to keep your changes to the Alias.</source>
        <translation>Обычно Licq изменяет псевдоним при получении информации о пользователе с сервера.
Эта опция не позволяет изменить псевдоним при обновлении.</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="190"/>
        <source>ID:</source>
        <translation>Идентификатор:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="193"/>
        <source>IP:</source>
        <translation>IP:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="197"/>
        <source>Status:</source>
        <translation>Статус:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="200"/>
        <source>Timezone:</source>
        <translation>Временная зона:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="204"/>
        <location filename="../src/userdlg/info.cpp" line="814"/>
        <location filename="../src/userdlg/info.cpp" line="1360"/>
        <source>Name:</source>
        <translation>Имя:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="210"/>
        <source>EMail 1:</source>
        <translation>EMail 1:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="216"/>
        <source>EMail 2:</source>
        <translation>EMail 2:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="220"/>
        <source>Old Email:</source>
        <translation>Старый Email:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="224"/>
        <location filename="../src/userdlg/info.cpp" line="850"/>
        <source>Address:</source>
        <translation>Адрес:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="227"/>
        <location filename="../src/userdlg/info.cpp" line="872"/>
        <source>Phone:</source>
        <translation>Телефон:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="231"/>
        <location filename="../src/userdlg/info.cpp" line="845"/>
        <source>State:</source>
        <translation>Штат:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="236"/>
        <location filename="../src/userdlg/info.cpp" line="875"/>
        <source>Fax:</source>
        <translation>Факс:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="241"/>
        <location filename="../src/userdlg/info.cpp" line="842"/>
        <source>City:</source>
        <translation>Город:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="245"/>
        <source>Cellular:</source>
        <translation>Сотовый:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="250"/>
        <location filename="../src/userdlg/info.cpp" line="854"/>
        <source>Zip:</source>
        <translation>Индекс:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="254"/>
        <location filename="../src/userdlg/info.cpp" line="857"/>
        <source>Country:</source>
        <translation>Страна:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="322"/>
        <location filename="../src/userdlg/info.cpp" line="594"/>
        <location filename="../src/userdlg/info.cpp" line="918"/>
        <location filename="../src/userdlg/info.cpp" line="924"/>
        <source>Unknown (%1)</source>
        <translation>Неизвестно (%1)</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="376"/>
        <source>Age:</source>
        <translation>Возраст:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="379"/>
        <source>Gender:</source>
        <translation>Пол:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="383"/>
        <location filename="../src/userdlg/info.cpp" line="499"/>
        <location filename="../src/userdlg/info.cpp" line="505"/>
        <location filename="../src/userdlg/info.cpp" line="523"/>
        <source>Unspecified</source>
        <translation>Не указано</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="384"/>
        <location filename="../src/userdlg/info.cpp" line="495"/>
        <source>Female</source>
        <translation>Женский</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="385"/>
        <location filename="../src/userdlg/info.cpp" line="497"/>
        <source>Male</source>
        <translation>Мужской</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="394"/>
        <location filename="../src/userdlg/info.cpp" line="879"/>
        <source>Homepage:</source>
        <translation>Домашняя страница:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="398"/>
        <source>Category:</source>
        <translation>Категория:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="406"/>
        <source>Description:</source>
        <translation>Описание:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="411"/>
        <source>Birthday:</source>
        <translation>День рождения:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="416"/>
        <source> Day:</source>
        <translation>День:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="420"/>
        <source> Month:</source>
        <translation>Месяц:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="424"/>
        <source> Year:</source>
        <translation>Год:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="438"/>
        <location filename="../src/userdlg/info.cpp" line="458"/>
        <source>Language 1:</source>
        <translation>Основной язык:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="441"/>
        <location filename="../src/userdlg/info.cpp" line="461"/>
        <source>Language 2:</source>
        <translation>Второй язык:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="445"/>
        <location filename="../src/userdlg/info.cpp" line="465"/>
        <source>Language 3:</source>
        <translation>Третий язык:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="601"/>
        <source>Authorization Required</source>
        <translation>Требуется авторизация</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="603"/>
        <source>Authorization Not Required</source>
        <translation>Авторизация не требуется</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="609"/>
        <source>User has an ICQ Homepage </source>
        <translation>У пользователя есть домашняя ICQ-страница</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="612"/>
        <source>User has no ICQ Homepage</source>
        <translation>У пользователя нет домашней ICQ-страницы</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="770"/>
        <location filename="../src/userdlg/info.cpp" line="1104"/>
        <location filename="../src/userdlg/info.cpp" line="1108"/>
        <source>Unknown</source>
        <translation>Неизвестно</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="790"/>
        <source>(none)</source>
        <translation>(нет)</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="818"/>
        <source>Department:</source>
        <translation>Отдел:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="822"/>
        <source>Position:</source>
        <translation>Должность:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="826"/>
        <source>Occupation:</source>
        <translation>Род занятий:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="997"/>
        <source>PhoneBook</source>
        <translation>Телефонная книга</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="1003"/>
        <source>Type</source>
        <translation>Тип</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="1004"/>
        <source>Number/Gateway</source>
        <translation>Номер/Шлюз</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="1005"/>
        <source>Country/Provider</source>
        <translation>Страна/Оператор</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="1014"/>
        <source>Currently at:</source>
        <translation>Сейчас на:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="1038"/>
        <source>Add...</source>
        <translation>Добавить...</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="1042"/>
        <location filename="../src/userdlg/info.cpp" line="1223"/>
        <source>Clear</source>
        <translation>Очистить</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="1123"/>
        <location filename="../src/userdlg/info.cpp" line="1125"/>
        <source>(</source>
        <translation>(</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="1123"/>
        <location filename="../src/userdlg/info.cpp" line="1125"/>
        <source>) </source>
        <translation>)</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="1128"/>
        <source>-</source>
        <translation>-</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="1219"/>
        <location filename="../src/userdlg/info.cpp" line="1368"/>
        <source>Browse...</source>
        <translation>Обзор...</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="1245"/>
        <location filename="../src/userdlg/info.cpp" line="1711"/>
        <source>Not Available</source>
        <translation>Отсутствует</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="1253"/>
        <location filename="../src/userdlg/info.cpp" line="1716"/>
        <source>Failed to Load</source>
        <translation>Не удалось загрузить</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="1300"/>
        <source>Last Online:</source>
        <translation>В сети:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="1304"/>
        <source>Last Sent Event:</source>
        <translation>Что-либо отправлено:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="1308"/>
        <source>Last Received Event:</source>
        <translation>Что-либо принято:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="1312"/>
        <source>Last Checked Auto Response:</source>
        <translation>Проверен авто-ответ:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="1316"/>
        <source>Online Since:</source>
        <translation>В сети с:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="1320"/>
        <source>Registration Date:</source>
        <translation>Дата регистрации:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="1335"/>
        <source>Now</source>
        <translation>Сейчас</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="1347"/>
        <source>Offline</source>
        <translation>Отключен</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="1357"/>
        <source>KDE Adress Book</source>
        <translation>Адресная книга KDE</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="1364"/>
        <source>Email:</source>
        <translation>Email:</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="1510"/>
        <source>You need to be connected to the
ICQ Network to retrieve your settings.</source>
        <translation>Вы должны быть в ICQ сети для
получения этих данных.</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="1575"/>
        <source>You need to be connected to the
ICQ Network to change your settings.</source>
        <translation>Вы должны быть в ICQ сети для
изменения этих данных.</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="1681"/>
        <location filename="../src/userdlg/info.cpp" line="1685"/>
        <source>Select your picture</source>
        <translation>Выберите Вашу картинку</translation>
    </message>
    <message>
        <location filename="../src/userdlg/info.cpp" line="1696"/>
        <source> is over %1 bytes.
Select another picture?</source>
        <translation> более %1 байт.
Выбрать другую картинку?</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::UserPages::Settings</name>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="67"/>
        <source>Settings</source>
        <translation>Установки</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="69"/>
        <source>Status</source>
        <translation>Статус</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="71"/>
        <source>Sounds</source>
        <translation type="unfinished">Звуки</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="73"/>
        <location filename="../src/userdlg/settings.cpp" line="239"/>
        <source>Groups</source>
        <translation>Группы</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="82"/>
        <source>Misc Modes</source>
        <translation>Различные режимы</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="85"/>
        <source>Accept in away</source>
        <translation>Принимать, если в режиме &apos;Отошел&apos;</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="86"/>
        <source>Play sounds for this contact when my status is away.</source>
        <translation>Воспроизводить звуки, если в режиме &apos;Отошел&apos;.</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="89"/>
        <source>Accept in not available</source>
        <translation>Принимать, если в режиме &apos;Недоступен&apos;</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="90"/>
        <source>Play sounds for this contact when my status is not available.</source>
        <translation>Воспроизводить звуки, если в режиме &apos;Недоступен&apos;.</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="93"/>
        <source>Accept in occupied</source>
        <translation>Принимать, если в режиме &apos;Занят&apos;</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="94"/>
        <source>Play sounds for this contact when my status is occupied.</source>
        <translation>Воспроизводить звуки, если в режиме &apos;Занят&apos;.</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="97"/>
        <source>Accept in do not disturb</source>
        <translation>Принимать, если в режиме &apos;Не беспокоить&apos;</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="98"/>
        <source>Play sounds for this contact when my status is do not disturb.</source>
        <translation>Воспроизводить звуки, если в режиме &apos;Не беспокоить&apos;.</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="101"/>
        <source>Auto accept files</source>
        <translation>Автоматически принимать файлы</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="102"/>
        <source>Automatically accept file transfers from this contact.</source>
        <translation>Автоматически принимать файлы от этого контакта.</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="105"/>
        <source>Auto accept chats</source>
        <translation>Автоматически принимать запросы чата</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="106"/>
        <source>Automatically accept chat requests from this contact.</source>
        <translation>Автоматически принимать запросы чата от этого контакта.</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="109"/>
        <source>Auto request secure</source>
        <translation>Автоматически запрашивать защищенное соединение</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="110"/>
        <source>Automatically request secure channel to this contact.</source>
        <translation>Автоматически устанавливать защищенное соединение с этим контактом.</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="113"/>
        <source>Use GPG encryption</source>
        <translation>Использовать GPG шифрование</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="114"/>
        <source>Use GPG encryption for messages with this contact.</source>
        <translation>Использовать GPG шифрование с этим контактом.</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="119"/>
        <source>Use real ip (LAN)</source>
        <translation>Использовать реальный IP адрес в ЛВС</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="120"/>
        <source>Use real IP for when sending to this contact.</source>
        <translation>Использовать реальный IP адрес при общении с этим контактом.</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="135"/>
        <source>Status to User</source>
        <translation>Статус для пользователя</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="138"/>
        <source>Not overridden</source>
        <translation>Основной</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="139"/>
        <source>Contact will see your normal status.</source>
        <translation>Контакт будет видеть Ваш текущий статус.</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="142"/>
        <source>Online</source>
        <translation>В сети</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="143"/>
        <source>Contact will always see you as online.</source>
        <translation>Контакт будет видеть, что Вы всегда в сети.</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="146"/>
        <source>Away</source>
        <translation>Отошел</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="147"/>
        <source>Contact will always see your status as away.</source>
        <translation>Контакт будет видеть, что Вы всегда в состоянии &apos;Отошел&apos;.</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="150"/>
        <source>Not available</source>
        <translation>Недоступен</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="151"/>
        <source>Contact will always see your status as not available.</source>
        <translation>Контакт будет видеть, что Вы всегда в состоянии &apos;Недоступен&apos;.</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="154"/>
        <source>Occupied</source>
        <translation>Занят</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="155"/>
        <source>Contact will always see your status as occupied.</source>
        <translation>Контакт будет видеть, что Вы всегда в состоянии &apos;Занят&apos;.</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="158"/>
        <source>Do not disturb</source>
        <translation>Не беспокоить</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="159"/>
        <source>Contact will always see your status as do not disturb.</source>
        <translation>Контакт будет видеть, что Вы всегда в состоянии &apos;Не беспокоить&apos;.</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="165"/>
        <source>System Groups</source>
        <translation>Системные группы</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="169"/>
        <source>Notify when this contact comes online.</source>
        <translation>Оповещать о появлении при заходе в сеть.</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="173"/>
        <source>Contact will see you online even if you&apos;re invisible.</source>
        <translation>Контакт будет видеть Ваш статус если Вы невидимы.</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="177"/>
        <source>Contact will always see you as offline.</source>
        <translation>Ваш статус будет скрываться от контакта.</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="181"/>
        <source>Ignore any events from this contact.</source>
        <translation>Все сообщения от контакта будут игнорироваться.</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="185"/>
        <source>Contact was recently added to the list.</source>
        <translation>Контакт был недавно добавлен в список.</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="190"/>
        <source>Custom Auto Response</source>
        <translation>Персональный авто-ответ</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="199"/>
        <source>Hints</source>
        <translation>Подсказки</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="203"/>
        <source>Clear</source>
        <translation>Очистить</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="249"/>
        <source>Group</source>
        <translation>Группа</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="249"/>
        <source>Local</source>
        <translation>Локальная</translation>
    </message>
    <message>
        <location filename="../src/userdlg/settings.cpp" line="249"/>
        <source>Server</source>
        <translation>Сервер</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::UserSelectDlg</name>
    <message>
        <location filename="../src/dialogs/userselectdlg.cpp" line="83"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Отмена</translation>
    </message>
    <message>
        <location filename="../src/dialogs/userselectdlg.cpp" line="45"/>
        <source>Licq User Select</source>
        <translation>Licq - Выбор пользователя</translation>
    </message>
    <message>
        <location filename="../src/dialogs/userselectdlg.cpp" line="79"/>
        <source>&amp;Ok</source>
        <translation>&amp;Ок</translation>
    </message>
    <message>
        <location filename="../src/dialogs/userselectdlg.cpp" line="63"/>
        <source>&amp;Password:</source>
        <translation>&amp;Пароль:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/userselectdlg.cpp" line="71"/>
        <source>&amp;Save Password</source>
        <translation>&amp;Сохранить пароль</translation>
    </message>
    <message>
        <location filename="../src/dialogs/userselectdlg.cpp" line="54"/>
        <source>&amp;User:</source>
        <translation>&amp;Пользователь:</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::UserSendChatEvent</name>
    <message>
        <location filename="../src/userevents/usersendchatevent.cpp" line="79"/>
        <source> - Chat Request</source>
        <translation> - Запрос чата</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendchatevent.cpp" line="98"/>
        <source>Chat with %1 refused:
%2</source>
        <translation>%1 отказался участвовать в чате:
%2</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendchatevent.cpp" line="140"/>
        <source>Clear</source>
        <translation>Очистить</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendchatevent.cpp" line="75"/>
        <location filename="../src/userevents/usersendchatevent.cpp" line="148"/>
        <source>Invite</source>
        <translation>Пригласить</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendchatevent.cpp" line="69"/>
        <source>Multiparty: </source>
        <translation>Многопользовательский:</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendchatevent.cpp" line="96"/>
        <source>No reason provided</source>
        <translation>Причина не указана</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::UserSendCommon</name>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="1198"/>
        <source> to Contact List</source>
        <translation> в список контактов</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="1180"/>
        <source>%1 is in %2 mode:
%3
Send...</source>
        <translation>%1 в режиме %2:
%3
Отправляется...</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="1198"/>
        <source>Cancel</source>
        <translation>Отмена</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="1059"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Отмена</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="534"/>
        <source>Change background color</source>
        <translation>Изменить цвет фона</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="533"/>
        <source>Change text color</source>
        <translation>Изменить цвет текста</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="158"/>
        <source>Chat Request</source>
        <translation>Запрос чата</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="160"/>
        <source>Contact List</source>
        <translation>Список контактов</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="1163"/>
        <source>Direct send failed,
send through server?</source>
        <translation>Отправить напрямую не удалось.
Отправить через сервер?</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="1353"/>
        <source>Drag Users Here
Right Click for Options</source>
        <translation>Перенесите пользователей сюда
Используйте правую кнопку для выбора опций</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="256"/>
        <source>Error! no owner set</source>
        <translation>Ошибка! Не задан владелец</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="159"/>
        <source>File Transfer</source>
        <translation>Передача файла</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="532"/>
        <source>Insert smileys</source>
        <translation>Вставить смайл</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="156"/>
        <source>Message</source>
        <translation>Сообщение</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="168"/>
        <source>Message type</source>
        <translation>Тип сообщения</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="198"/>
        <source>Multiple Recipients</source>
        <translation>Нескольким адресатам</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="531"/>
        <source>Multiple recipients</source>
        <translation>Нескольким адресатам</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="161"/>
        <source>SMS</source>
        <translation>SMS</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="528"/>
        <source>Select type of message to send</source>
        <translation>Выбeрите тип сообщения для отправки</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="211"/>
        <location filename="../src/userevents/usersendcommon.cpp" line="1136"/>
        <source>&amp;Send</source>
        <translation>&amp;Отправить</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="173"/>
        <location filename="../src/userevents/usersendcommon.cpp" line="529"/>
        <source>Send through server</source>
        <translation>Отправить через сервер</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="1048"/>
        <source>Sending </source>
        <translation>Отправляется </translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="204"/>
        <source>Smileys</source>
        <translation>Смайлы</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="157"/>
        <source>URL</source>
        <translation>Ссылка</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="195"/>
        <location filename="../src/userevents/usersendcommon.cpp" line="530"/>
        <location filename="../src/userevents/usersendcommon.cpp" line="1198"/>
        <source>Urgent</source>
        <translation>Срочно</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="1113"/>
        <source>cancelled</source>
        <translation>прервано</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="1049"/>
        <source>direct</source>
        <translation>напрямую</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="1109"/>
        <source>done</source>
        <translation>выполнено</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="1077"/>
        <location filename="../src/userevents/usersendcommon.cpp" line="1122"/>
        <source>error</source>
        <translation>ошибка</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="1116"/>
        <source>failed</source>
        <translation>неудачно</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="1119"/>
        <source>timed out</source>
        <translation>время ожидания вышло</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="1049"/>
        <source>via server</source>
        <translation>через сервер</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="205"/>
        <source>Text Color...</source>
        <translation>Цвет текста...</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="206"/>
        <source>Background Color...</source>
        <translation>Цвет фона...</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="621"/>
        <source>%1 has joined the conversation.</source>
        <translation>%1 присоединился.</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="648"/>
        <source>%1 has left the conversation.</source>
        <translation>%1 отсоединился.</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcommon.cpp" line="1008"/>
        <source>Message can&apos;t be sent securely through the server!
Send anyway?</source>
        <translation>Cообщение не может быть безопасно отправлено через сервер!
Отправить все равно?</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::UserSendContactEvent</name>
    <message>
        <location filename="../src/userevents/usersendcontactevent.cpp" line="79"/>
        <source> - Contact List</source>
        <translation> - Список контактов</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendcontactevent.cpp" line="74"/>
        <source>Drag Users Here - Right Click for Options</source>
        <translation>Перенесите пользователей сюда - используйте правую кнопку для выбора опций</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::UserSendFileEvent</name>
    <message>
        <location filename="../src/userevents/usersendfileevent.cpp" line="86"/>
        <source> - File Transfer</source>
        <translation> - Передача файла</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendfileevent.cpp" line="117"/>
        <location filename="../src/userevents/usersendfileevent.cpp" line="200"/>
        <source>%1 Files</source>
        <translation>%1 файл(ов)</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendfileevent.cpp" line="77"/>
        <source>Browse</source>
        <translation>Просмотр</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendfileevent.cpp" line="81"/>
        <source>Edit</source>
        <translation>Редактировать</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendfileevent.cpp" line="130"/>
        <source>File transfer with %1 refused:
%2</source>
        <translation>В передаче файла к %1 отказано:
%2</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendfileevent.cpp" line="70"/>
        <source>File(s): </source>
        <translation>Файл(ы): </translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendfileevent.cpp" line="128"/>
        <source>No reason provided</source>
        <translation>Причина не указана</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendfileevent.cpp" line="159"/>
        <location filename="../src/userevents/usersendfileevent.cpp" line="161"/>
        <source>Select files to send</source>
        <translation>Выберите файлы для отправки</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendfileevent.cpp" line="216"/>
        <source>You must specify a file to transfer!</source>
        <translation>Вы должны указать файл для передачи!</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::UserSendMsgEvent</name>
    <message>
        <location filename="../src/userevents/usersendmsgevent.cpp" line="58"/>
        <source> - Message</source>
        <translation> - Сообщение</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendmsgevent.cpp" line="114"/>
        <source>You didn&apos;t edit the message.
Do you really want to send it?</source>
        <translation>Вы не редактировали сообщение.
Вы уверены, что хотите его отправить?</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::UserSendSmsEvent</name>
    <message>
        <location filename="../src/userevents/usersendsmsevent.cpp" line="85"/>
        <source> - SMS</source>
        <translation> - SMS</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendsmsevent.cpp" line="70"/>
        <source>Chars left : </source>
        <translation>Осталось символов: </translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendsmsevent.cpp" line="64"/>
        <source>Phone : </source>
        <translation>Телефон:</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendsmsevent.cpp" line="124"/>
        <source>You didn&apos;t edit the SMS.
Do you really want to send it?</source>
        <translation>Вы не отредактировали SMS сообщение.
Вы действительно хотите его отослать?</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::UserSendUrlEvent</name>
    <message>
        <location filename="../src/userevents/usersendurlevent.cpp" line="68"/>
        <source> - URL</source>
        <translation> - Ссылка</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendurlevent.cpp" line="140"/>
        <source>No URL specified</source>
        <translation>Ссылка не указана</translation>
    </message>
    <message>
        <location filename="../src/userevents/usersendurlevent.cpp" line="62"/>
        <source>URL : </source>
        <translation>Ссылка: </translation>
    </message>
</context>
<context>
    <name>LicqQtGui::UserViewEvent</name>
    <message>
        <location filename="../src/userevents/userviewevent.cpp" line="701"/>
        <source>
--------------------
Request was cancelled.</source>
        <translation>
--------------------
Запрос был прерван.</translation>
    </message>
    <message>
        <location filename="../src/userevents/userviewevent.cpp" line="706"/>
        <source>A&amp;ccept</source>
        <translation>&amp;Принять</translation>
    </message>
    <message>
        <location filename="../src/userevents/userviewevent.cpp" line="769"/>
        <source>A&amp;dd %1 Users</source>
        <translation>&amp;Добавить %1 пользователей</translation>
    </message>
    <message>
        <location filename="../src/userevents/userviewevent.cpp" line="742"/>
        <location filename="../src/userevents/userviewevent.cpp" line="751"/>
        <location filename="../src/userevents/userviewevent.cpp" line="760"/>
        <location filename="../src/userevents/userviewevent.cpp" line="772"/>
        <source>A&amp;dd User</source>
        <translation>&amp;Добавить</translation>
    </message>
    <message>
        <location filename="../src/userevents/userviewevent.cpp" line="738"/>
        <source>A&amp;uthorize</source>
        <translation>&amp;Авторизировать</translation>
    </message>
    <message>
        <location filename="../src/userevents/userviewevent.cpp" line="122"/>
        <source>Aut&amp;o Close</source>
        <translation>Закрывать а&amp;втоматически</translation>
    </message>
    <message>
        <location filename="../src/userevents/userviewevent.cpp" line="498"/>
        <source>Chat</source>
        <translation>Чат</translation>
    </message>
    <message>
        <location filename="../src/userevents/userviewevent.cpp" line="134"/>
        <source>&amp;Close</source>
        <translation>&amp;Закрыть</translation>
    </message>
    <message>
        <location filename="../src/userevents/userviewevent.cpp" line="519"/>
        <source>File Transfer</source>
        <translation>Передача файла</translation>
    </message>
    <message>
        <location filename="../src/userevents/userviewevent.cpp" line="721"/>
        <location filename="../src/userevents/userviewevent.cpp" line="732"/>
        <source>&amp;Forward</source>
        <translation>&amp;Переслать</translation>
    </message>
    <message>
        <location filename="../src/userevents/userviewevent.cpp" line="714"/>
        <source>&amp;Join</source>
        <translation>&amp;Присоединиться</translation>
    </message>
    <message>
        <location filename="../src/userevents/userviewevent.cpp" line="129"/>
        <location filename="../src/userevents/userviewevent.cpp" line="261"/>
        <source>Nex&amp;t</source>
        <translation>&amp;Следующее</translation>
    </message>
    <message>
        <location filename="../src/userevents/userviewevent.cpp" line="259"/>
        <source>Nex&amp;t (%1)</source>
        <translation>&amp;Следующее (%1)</translation>
    </message>
    <message>
        <location filename="../src/userevents/userviewevent.cpp" line="135"/>
        <source>Normal Click - Close Window
&lt;CTRL&gt;+Click - also delete User</source>
        <translation>Обычное нажатие закрывает окно.
Нажатие с удерживанием клавиши &lt;CTRL&gt;
закрывает окно и удаляет пользователя</translation>
    </message>
    <message>
        <location filename="../src/userevents/userviewevent.cpp" line="720"/>
        <location filename="../src/userevents/userviewevent.cpp" line="731"/>
        <source>&amp;Quote</source>
        <translation>&amp;Цитировать</translation>
    </message>
    <message>
        <location filename="../src/userevents/userviewevent.cpp" line="707"/>
        <location filename="../src/userevents/userviewevent.cpp" line="739"/>
        <source>&amp;Refuse</source>
        <translation>О&amp;тказать</translation>
    </message>
    <message>
        <location filename="../src/userevents/userviewevent.cpp" line="699"/>
        <location filename="../src/userevents/userviewevent.cpp" line="719"/>
        <location filename="../src/userevents/userviewevent.cpp" line="730"/>
        <source>&amp;Reply</source>
        <translation>&amp;Ответить</translation>
    </message>
    <message>
        <location filename="../src/userevents/userviewevent.cpp" line="722"/>
        <source>Start Chat</source>
        <translation>Начать чат</translation>
    </message>
    <message>
        <location filename="../src/userevents/userviewevent.cpp" line="733"/>
        <source>&amp;View</source>
        <translation>П&amp;росмотр</translation>
    </message>
    <message>
        <location filename="../src/userevents/userviewevent.cpp" line="777"/>
        <source>&amp;View Email</source>
        <translation>&amp;Показать Email</translation>
    </message>
    <message>
        <location filename="../src/userevents/userviewevent.cpp" line="743"/>
        <location filename="../src/userevents/userviewevent.cpp" line="752"/>
        <location filename="../src/userevents/userviewevent.cpp" line="761"/>
        <source>&amp;View Info</source>
        <translation>Просмотр &amp;деталей</translation>
    </message>
</context>
<context>
    <name>LicqQtGui::UtilityDlg</name>
    <message>
        <location filename="../src/dialogs/utilitydlg.cpp" line="129"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Отмена</translation>
    </message>
    <message>
        <location filename="../src/dialogs/utilitydlg.cpp" line="261"/>
        <source>C&amp;lose</source>
        <translation>&amp;Закрыть</translation>
    </message>
    <message>
        <location filename="../src/dialogs/utilitydlg.cpp" line="223"/>
        <source>Command Window</source>
        <translation>Окно команды</translation>
    </message>
    <message>
        <location filename="../src/dialogs/utilitydlg.cpp" line="64"/>
        <source>Command:</source>
        <translation>Команда:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/utilitydlg.cpp" line="87"/>
        <source>Description:</source>
        <translation>Описание:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/utilitydlg.cpp" line="252"/>
        <source>Done</source>
        <translation>Завершить</translation>
    </message>
    <message>
        <location filename="../src/dialogs/utilitydlg.cpp" line="260"/>
        <source>Done:</source>
        <translation>Завершено:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/utilitydlg.cpp" line="92"/>
        <source>Edit final command</source>
        <translation>Редактировать команду</translation>
    </message>
    <message>
        <location filename="../src/dialogs/utilitydlg.cpp" line="188"/>
        <source>Edit:</source>
        <translation>Редактировать:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/utilitydlg.cpp" line="246"/>
        <source>Failed:</source>
        <translation>Неудача:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/utilitydlg.cpp" line="77"/>
        <source>GUI</source>
        <translation>Графический интерфейс</translation>
    </message>
    <message>
        <location filename="../src/dialogs/utilitydlg.cpp" line="83"/>
        <source>Internal</source>
        <translation>встроенное</translation>
    </message>
    <message>
        <location filename="../src/dialogs/utilitydlg.cpp" line="63"/>
        <source>Licq Utility: %1</source>
        <translation>Licq Утилита: %1</translation>
    </message>
    <message>
        <location filename="../src/dialogs/utilitydlg.cpp" line="126"/>
        <source>&amp;Run</source>
        <translation>&amp;Запустить</translation>
    </message>
    <message>
        <location filename="../src/dialogs/utilitydlg.cpp" line="196"/>
        <source>Running:</source>
        <translation>Выполняется:</translation>
    </message>
    <message>
        <location filename="../src/dialogs/utilitydlg.cpp" line="80"/>
        <source>Terminal</source>
        <translation>Терминал</translation>
    </message>
    <message>
        <location filename="../src/dialogs/utilitydlg.cpp" line="95"/>
        <source>User Fields</source>
        <translation>Пользовательские поля</translation>
    </message>
    <message>
        <location filename="../src/dialogs/utilitydlg.cpp" line="71"/>
        <source>Window:</source>
        <translation>Окно:</translation>
    </message>
</context>
<context>
    <name>QMessageBox</name>
    <message>
        <location filename="../src/core/messagebox.cpp" line="63"/>
        <location filename="../src/core/messagebox.cpp" line="68"/>
        <location filename="../src/core/messagebox.cpp" line="71"/>
        <location filename="../src/core/messagebox.cpp" line="73"/>
        <location filename="../src/core/messagebox.cpp" line="77"/>
        <location filename="../src/core/messagebox.cpp" line="80"/>
        <location filename="../src/core/messagebox.cpp" line="90"/>
        <location filename="../src/core/messagebox.cpp" line="95"/>
        <source>Licq Question</source>
        <translation>Licq - Вопрос</translation>
    </message>
    <message>
        <location filename="../src/core/messagebox.cpp" line="78"/>
        <location filename="../src/core/messagebox.cpp" line="81"/>
        <source>No</source>
        <translation>Нет</translation>
    </message>
    <message>
        <location filename="../src/core/messagebox.cpp" line="78"/>
        <location filename="../src/core/messagebox.cpp" line="81"/>
        <source>Yes</source>
        <translation>Да</translation>
    </message>
</context>
</TS>
