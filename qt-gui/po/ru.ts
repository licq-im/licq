<!DOCTYPE TS><TS>
<context>
    <name>AddUserDlg</name>
    <message>
        <source>Licq - Add User</source>
        <translation>Licq - Добавление пользователя</translation>
    </message>
    <message>
        <source>New User UIN:</source>
        <translation>UIN нового пользователя</translation>
    </message>
    <message>
        <source>&amp;Alert User</source>
        <translation>&amp;Оповестить пользователя</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Отменить</translation>
    </message>
</context>
<context>
    <name>AuthUserDlg</name>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;Да</translation>
    </message>
    <message>
        <source>Refuse authorization to %1</source>
        <translation>Отказать в авторизации %1</translation>
    </message>
    <message>
        <source>Authorize which user (UIN):</source>
        <translation>Авторизовать пользователя (UIN):</translation>
    </message>
    <message>
        <source>Grant authorization to %1</source>
        <translation>Авторизовать %1</translation>
    </message>
    <message>
        <source>Licq - Grant Authorisation</source>
        <translation>Licq - Авторизация</translation>
    </message>
    <message>
        <source>Licq - Refuse Authorisation</source>
        <translation>Licq - Отказ в авторизации</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Отменить</translation>
    </message>
    <message>
        <source>Response</source>
        <translation>Ответ</translation>
    </message>
</context>
<context>
    <name>AwayMsgDlg</name>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;Да</translation>
    </message>
    <message>
        <source>I&apos;m currently %1, %a.
You can leave me a message.
(%m messages pending from you).</source>
        <translation>Я сейчас в режиме &quot;%1&quot;, %a.
Вы можете оставить мне сообщение.
(%m не просмотренных сообщений от Вас).</translation>
    </message>
    <message>
        <source>&amp;Hints</source>
        <translation>&amp;Советы</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Hints for Setting&lt;br&gt;your Auto-Response&lt;/h2&gt;&lt;br&gt;&lt;hr&gt;&lt;br&gt;&lt;ul&gt;&lt;li&gt;You can include any of the % expansions (described in the main hints page).&lt;/li&gt;&lt;li&gt;Any line beginning with a pipe (|) will be treated as a command to be run.  The line will be replaced by the output of the command. The command is parsed by /bin/sh so any shell commands or meta-characters are allowed.  For security reasons, any % expansions are automatically passed to the command surrounded by single quotes to prevent shell parsing of any meta-characters included in an alias.&lt;br&gt;Examples of popular uses include:&lt;ul&gt;&lt;li&gt;&lt;tt&gt;|date&lt;/tt&gt;: Will replace that line by the current date&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|fortune&lt;/tt&gt;: Show a fortune, as a tagline for example&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|myscript.sh %u %a&lt;/tt&gt;: Run a script, passing the uin and alias&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|myscript.sh %u %a &gt; /dev/null&lt;/tt&gt;: Run the same script but ignore the output (for tracking auto response checks or something)&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|if [ %u -lt 100000 ]; then echo &quot;You are special&quot;; fi&lt;/tt&gt;: Useless, but shows how you can use shell script.&lt;/li&gt;&lt;/ul&gt;Of course, multiple &quot;|&quot; can appear in the auto response, and commands and regular text can be mixed line by line.&lt;/li&gt;&lt;hr&gt;&lt;p&gt; For more information, see the Licq webpage (&lt;tt&gt;http://www.licq.org&lt;/tt&gt;).&lt;/p&gt;</source>
        <translation>&lt;h2&gt;Подсказка по настройке&lt;br&gt;Сообщения автоответа&lt;/h2&gt;&lt;br&gt;&lt;br&gt;&lt;br&gt;&lt;ul&gt;&lt;li&gt;Вы можете использовать любую из %-переменных (описание на странице &quot;Советы&quot;).&lt;/li&gt;&lt;li&gt;Любая строка начинающаяся с символа &quot;|&quot; обрабатывается как команда и заменяется на текст полученный с ее стандартного потока вывода (stdout). В качестве интерпретатора команд используется &lt;b&gt;/bin/sh&lt;/b&gt;, допускается использование любых команд и метасимволов. Из соображений безопасности %-переменные автоматически обрабатываются и заключаются в одинарные кавычки при передаче интерпретатору команд во избежание обработки им метасимволов.&lt;br&gt;Примеры:&lt;ul&gt;&lt;li&gt;&lt;tt&gt;|date&lt;/tt&gt;: Заменяется на текущую дату;&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|fortune&lt;/tt&gt;: Отображает цитату использую программу fortune;&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|myscript.sh %u %a&lt;/tt&gt;: Запускает скрипт, и передает ему в качестве аргументов ICQ# и ник;&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|myscript.sh %u %a &gt; /dev/null&lt;/tt&gt;: Запускает скрипт без вывода чего-либо (для каких-нибудь своих целей);&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|if [ %u -lt 100000 ]; then echo &quot;You are special&quot;; fi&lt;/tt&gt;: Простой пример использования данной возможности.&lt;/li&gt;&lt;/ul&gt;Допускается так же и многократное использование префиксированных &quot;|&quot; команд, при этом они будут обрабатываться &quot;строчка за строчкой&quot;.&lt;/li&gt;&lt;hr&gt;&lt;p&gt; Более полную информацию можно найти на сайте Licq (&lt;tt&gt;http://www.licq.org&lt;/tt&gt;).&lt;/p&gt;</translation>
    </message>
    <message>
        <source>&amp;Edit Items</source>
        <translation>&amp;Редактировать</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Отменить</translation>
    </message>
    <message>
        <source>&amp;Select</source>
        <translation>&amp;Выбрать</translation>
    </message>
    <message>
        <source>Set %1 Response for %2</source>
        <translation>Установить ответ %1 для %2</translation>
    </message>
</context>
<context>
    <name>CFileDlg</name>
    <message>
        <source>KB</source>
        <translation>КБ</translation>
    </message>
    <message>
        <source>MB</source>
        <translation>МБ</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>ОК</translation>
    </message>
    <message>
        <source>Byte</source>
        <translation>Байт</translation>
    </message>
    <message>
        <source>ETA:</source>
        <translation>ОВП:</translation>
    </message>
    <message>
        <source>%1/%2</source>
        <translation>%1/%2</translation>
    </message>
    <message>
        <source>Not enough resources.
</source>
        <translation>Недостаточно ресурсов.
</translation>
    </message>
    <message>
        <source>Connecting to remote...</source>
        <translation>Устанавливается соединение...</translation>
    </message>
    <message>
        <source>Bytes</source>
        <translation>Байт</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <source>File:</source>
        <translation>Файл:</translation>
    </message>
    <message>
        <source>Retry</source>
        <translation>Повторить</translation>
    </message>
    <message>
        <source>Time:</source>
        <translation>Время:</translation>
    </message>
    <message>
        <source>Licq - File Transfer (%1)</source>
        <translation>Licq - Передача файла(ов) (%1)</translation>
    </message>
    <message>
        <source>Connection error.
</source>
        <translation>Ошибка соединения
</translation>
    </message>
    <message>
        <source>Remote side disconnected.</source>
        <translation>Удаленная сторона отсоединилась.</translation>
    </message>
    <message>
        <source>Sent %1 to %2 successfully.</source>
        <translation>%1 отправлен к %2 успешно.</translation>
    </message>
    <message>
        <source>Waiting for connection...</source>
        <translation>Ожидание установления соединения...</translation>
    </message>
    <message>
        <source>File already exists and appears incomplete.</source>
        <translation>Файл уже есть и, похоже, неполный.</translation>
    </message>
    <message>
        <source>Batch:</source>
        <translation>Всего:</translation>
    </message>
    <message>
        <source>File already exists and is at least as big as the incoming file.</source>
        <translation>Файл уже есть и, по крайней мере, не меньше передаваемого.</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <source>Resume</source>
        <translation>Возобновить</translation>
    </message>
    <message>
        <source>Unable to create a thread.
See Network Window for details.</source>
        <translation>Невозможно создать нить.
Подробности в окне сети.</translation>
    </message>
    <message>
        <source>Handshaking error.
</source>
        <translation>Ошибка устнановления соединения.
</translation>
    </message>
    <message>
        <source>Handshake Error.
See Network Window for details.</source>
        <translation>Ошибка установления соединения
Подробности в окне сети.</translation>
    </message>
    <message>
        <source>File name:</source>
        <translation>Имя файла:</translation>
    </message>
    <message>
        <source>Bind error.
</source>
        <translation>Ошибка привязки сокета
</translation>
    </message>
    <message>
        <source>Sending file...</source>
        <translation>Отправляется файл...</translation>
    </message>
    <message>
        <source>File I/O error: %1.</source>
        <translation>Ошибка ввода/вывода: %1.</translation>
    </message>
    <message>
        <source>File I/O Error:
%1

See Network Window for details.</source>
        <translation>Ошибка ввода/вывода:
%1

Подробности в Окне Сети</translation>
    </message>
    <message>
        <source>File transfer cancelled.</source>
        <translation>Передача файла(ов) прервана.</translation>
    </message>
    <message>
        <source>Receiving file...</source>
        <translation>Принимается файл...</translation>
    </message>
    <message>
        <source>Current:</source>
        <translation>Текущий:</translation>
    </message>
    <message>
        <source>Open error - unable to open file for writing.</source>
        <translation>Ошибка открытия файла для записи.</translation>
    </message>
    <message>
        <source>Overwrite</source>
        <translation>Перезаписать</translation>
    </message>
    <message>
        <source>&amp;Cancel Transfer</source>
        <translation>&amp;Отменить передачу</translation>
    </message>
    <message>
        <source>Unable to bind to a port.
See Network Window for details.</source>
        <translation>Невозможно сделать привязку к порту.
Подробности в окне сети.</translation>
    </message>
    <message>
        <source>Received %1 from %2 successfully.</source>
        <translation>%1 принят от %2 успешно.</translation>
    </message>
    <message>
        <source>Unable to reach remote host.
See Network Window for details.</source>
        <translation>Удаленная машина недостижима.
Подробности в окне сети.</translation>
    </message>
    <message>
        <source>File transfer complete.</source>
        <translation>Передача файла(ов) закончена</translation>
    </message>
</context>
<context>
    <name>CForwardDlg</name>
    <message>
        <source>URL</source>
        <translation>Ссылка</translation>
    </message>
    <message>
        <source>Message</source>
        <translation>Сообщение</translation>
    </message>
    <message>
        <source>Forward %1 To User</source>
        <translation>Переслать %1 пользователю</translation>
    </message>
    <message>
        <source>Drag the user to forward to here:</source>
        <translation>Перетащите пользователя, которому
хотите переслать сообщения сюда:</translation>
    </message>
    <message>
        <source>Unable to forward this message type (%d).</source>
        <translation>Невозможно переслать сообщение этого типа (%d).</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Отменить</translation>
    </message>
    <message>
        <source>Forwarded message:
</source>
        <translation>Пересылаемое сообщение:
</translation>
    </message>
    <message>
        <source>&amp;Forward</source>
        <translation>&amp;Переслать</translation>
    </message>
    <message>
        <source>Forwarded URL:
</source>
        <translation>Пересылаемый URL:
</translation>
    </message>
</context>
<context>
    <name>CJoinChatDlg</name>
    <message>
        <source>Select chat to join:</source>
        <translation>Выберите разговор:</translation>
    </message>
    <message>
        <source>Invite to Join Chat</source>
        <translation>Пригласить вступить в разговор</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Отменить</translation>
    </message>
    <message>
        <source>Select chat to invite:</source>
        <translation>Выберите разговор:</translation>
    </message>
    <message>
        <source>Join Multiparty Chat</source>
        <translation>Вступить в разговор с несколькими людьми</translation>
    </message>
</context>
<context>
    <name>CMMSendDlg</name>
    <message>
        <source>Sending mass message to %1...</source>
        <translation>Отправляется массовое сообщение к %1...</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Закрыть</translation>
    </message>
    <message>
        <source>Sending mass URL to %1...</source>
        <translation>Отправляется URL к %1...</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>ошибка</translation>
    </message>
    <message>
        <source>Multiple Recipient Contact List</source>
        <translation>Список контактов многим адресатам</translation>
    </message>
    <message>
        <source>Multiple Recipient URL</source>
        <translation>URL многим адресатам</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Отменить</translation>
    </message>
    <message>
        <source>Sending mass list to %1...</source>
        <translation>Отправляется список контактов к %1...</translation>
    </message>
    <message>
        <source>Multiple Recipient Message</source>
        <translation>Сообщение многим адресатам</translation>
    </message>
</context>
<context>
    <name>CMMUserView</name>
    <message>
        <source>Crop</source>
        <translation>Оставить</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation>Очистить</translation>
    </message>
    <message>
        <source>Remove</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <source>Add Group</source>
        <translation>Добавить группу</translation>
    </message>
    <message>
        <source>Add All</source>
        <translation>Добавить всех</translation>
    </message>
</context>
<context>
    <name>CMainWindow</name>
    <message>
        <source> </source>
        <translation> </translation>
    </message>
    <message>
        <source>s</source>
        <translation> </translation>
    </message>
    <message>
        <source>Ok</source>
        <translation>Да</translation>
    </message>
    <message>
        <source>&amp;No</source>
        <translation>&amp;Нет</translation>
    </message>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;Да</translation>
    </message>
    <message>
        <source>&amp;Yes</source>
        <translation>&amp;Да</translation>
    </message>
    <message>
        <source>Send</source>
        <translation>Отправить</translation>
    </message>
    <message>
        <source>Online to User</source>
        <translation>Виден в режиме &quot;В сети&quot;</translation>
    </message>
    <message>
        <source>&amp;Away</source>
        <translation>&amp;Отошел</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>&amp;Помощь</translation>
    </message>
    <message>
        <source>&amp;Info</source>
        <translation>&amp;Информация</translation>
    </message>
    <message>
        <source>&amp;Last</source>
        <translation>&amp;Последний раз</translation>
    </message>
    <message>
        <source>U&amp;tilities</source>
        <translation>&amp;Утилиты</translation>
    </message>
    <message>
        <source>E&amp;xit</source>
        <translation>&amp;Выход</translation>
    </message>
    <message>
        <source>&amp;Options...</source>
        <translation>&amp;Настройки...</translation>
    </message>
    <message>
        <source>R&amp;andom Chat</source>
        <translation>&amp;Случайный разговор</translation>
    </message>
    <message>
        <source>Error sending authorization.</source>
        <translation>Ошибка при отправке авторизации.</translation>
    </message>
    <message>
        <source>&amp;Random Chat Group</source>
        <translation>&amp;Случайные собеседники</translation>
    </message>
    <message>
        <source>&lt;ul&gt;&lt;li&gt;&lt;tt&gt;%a - &lt;/tt&gt;user alias&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%e - &lt;/tt&gt;email&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%f - &lt;/tt&gt;first name&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%h - &lt;/tt&gt;phone number&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%i - &lt;/tt&gt;user ip&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%l - &lt;/tt&gt;last name&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%m - &lt;/tt&gt;# pending messages&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%n - &lt;/tt&gt;full name&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%o - &lt;/tt&gt;last seen online&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%O - &lt;/tt&gt;online since&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%p - &lt;/tt&gt;user port&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%s - &lt;/tt&gt;full status&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%S - &lt;/tt&gt;abbreviated status&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%u - &lt;/tt&gt;uin&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%w - &lt;/tt&gt;webpage&lt;/li&gt;&lt;/ul&gt;</source>
        <translation> &lt;ul&gt;&lt;li&gt;&lt;tt&gt;%a - &lt;/tt&gt;ник пользователя&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%e - &lt;/tt&gt;email&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%f - &lt;/tt&gt;имя&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%h - &lt;/tt&gt;номер телефона&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%i - &lt;/tt&gt;ip адрес пользователя&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%l - &lt;/tt&gt;фамилия&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%m - &lt;/tt&gt;количество непрочитаных сообщений&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%n - &lt;/tt&gt;имя фамилия&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%o - &lt;/tt&gt;последний раз в сети&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%O - &lt;/tt&gt;с какого времени в сети&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%p - &lt;/tt&gt;порт пользователя&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%s - &lt;/tt&gt;полный статус&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%S - &lt;/tt&gt;сокращенный статус&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%u - &lt;/tt&gt;uin&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%w - &lt;/tt&gt;домашняя страничка&lt;/li&gt;&lt;/ul&gt;</translation>
    </message>
    <message>
        <source>Up since %1
</source>
        <translation>В сети с %1
</translation>
    </message>
    <message>
        <source>Re&amp;quest Authorization</source>
        <translation>Запросит&amp;ь авторизацию</translation>
    </message>
    <message>
        <source>Check %1 Response</source>
        <translation>Посмотреть &apos;%1&apos; авто-ответ</translation>
    </message>
    <message>
        <source>You are currently registered as
UIN: %1
Base Directory: %2
Rerun licq with the -b option to select a new
base directory and then register a new user.</source>
        <translation> Вы сейчас зарегистрированы:
UIN: %1
Базовый каталог: %2
Укажите новую базовую директорию, перезапустив licq с опцией -b
После этого Вы сможете зарегистрировать новый UIN.</translation>
    </message>
    <message>
        <source>Auto Request Secure</source>
        <translation>Автоматически запрашивать защищенное соединение</translation>
    </message>
    <message>
        <source>Unable to open icons file
%1.</source>
        <translation>Ошибка при открытии файла иконки
%1.</translation>
    </message>
    <message>
        <source>Send &amp;Authorization</source>
        <translation>&amp;Авторизовать пользователя</translation>
    </message>
    <message>
        <source>Auto Accept Chats</source>
        <translation>Автоматически принимать запрос разговора</translation>
    </message>
    <message>
        <source>Auto Accept Files</source>
        <translation>Автоматически принимать файлы</translation>
    </message>
    <message>
        <source>&amp;Not Available</source>
        <translation>&amp;Недоступен</translation>
    </message>
    <message>
        <source>&amp;About</source>
        <translation>&amp;Информация</translation>
    </message>
    <message>
        <source>&amp;Group</source>
        <translation>&amp;Группа</translation>
    </message>
    <message>
        <source>&amp;Hints</source>
        <translation>&amp;Советы</translation>
    </message>
    <message>
        <source>&amp;Reset</source>
        <translation>&amp;Сбросить</translation>
    </message>
    <message>
        <source>Custom Auto Response...</source>
        <translation>Персональный авто-ответ</translation>
    </message>
    <message>
        <source>Licq version %1%8.
Qt GUI plugin version %2.
Compiled on: %7
%6
Maintainer: Jon Keating
Contributions by Dirk A. Mueller
Original Author: Graham Roff

http://www.licq.org
#licq on irc.openprojects.net

%3 (%4)
%5 contacts.</source>
        <translation>Версия Licq %1%8.
Версия Qt GUI модуля %2.
Дата сборки: %7
%6
Ведущий разработчик: Jon Keating
При содействии Dirk A. Mueller
Автор: Graham Roff

http://www.licq.org
Канал #licq на сервере irc.openprojects.net

%3 (%4)
%5 в списке контактов.</translation>
    </message>
    <message>
        <source>Online Notify</source>
        <translation>Оповещать о появлении</translation>
    </message>
    <message>
        <source>Set &amp;Auto Response...</source>
        <translation>&amp;Установить Авто-ответ</translation>
    </message>
    <message>
        <source>Remove From List</source>
        <translation>Удалить из списка</translation>
    </message>
    <message>
        <source>System Message</source>
        <translation>Системное сообщение</translation>
    </message>
    <message>
        <source>%1 msg%2</source>
        <translation>%1 сообщ%2</translation>
    </message>
    <message>
        <source>&amp;Popup All Messages</source>
        <translation>&amp;Вывести все сообщения</translation>
    </message>
    <message>
        <source>Accept in Not Available</source>
        <translation>Принимать, если в режиме &quot;Недоступен&quot;</translation>
    </message>
    <message>
        <source>View &amp;History</source>
        <translation>&amp;Просмотр истории</translation>
    </message>
    <message>
        <source>Other Users</source>
        <translation>Другие пользователи</translation>
    </message>
    <message>
        <source>Edit User Group</source>
        <translation>Редактировать группы</translation>
    </message>
    <message>
        <source>Successfully registered, your user identification
number (UIN) is %1.
Now set your personal information.</source>
        <translation>Регистрация прошла успешно. Ваш идентификационный номер (UIN) - %1.
Теперь введите информацию о себе.</translation>
    </message>
    <message>
        <source>Unknown Packets</source>
        <translation>Неизвестные пакеты</translation>
    </message>
    <message>
        <source>Send &amp;URL</source>
        <translation>Послать &amp;ссылку</translation>
    </message>
    <message>
        <source>Send &amp;SMS</source>
        <translation>Послать &amp;SMS</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <source>&lt;hr&gt;&lt;p&gt; For more information, see the Licq webpage (&lt;tt&gt;http://www.licq.org&lt;/tt&gt;).&lt;/p&gt;</source>
        <translation>&lt;hr&gt;&lt;p&gt; Более подробную информацию можно получить на web-сайте (&lt;tt&gt;http://www.licq.org&lt;/tt&gt;) или его русском зеркале (&lt;tt&gt;http://www.ru.licq.org&lt;/tt&gt;).&lt;/p&gt;</translation>
    </message>
    <message>
        <source>&amp;Mini Mode</source>
        <translation>&amp;Мини-режим</translation>
    </message>
    <message>
        <source>Errors</source>
        <translation>Ошибки</translation>
    </message>
    <message>
        <source>Close &amp;Secure Channel</source>
        <translation>&amp;Закрыть защищенный канал</translation>
    </message>
    <message>
        <source>No msgs</source>
        <translation>Нет сообщений</translation>
    </message>
    <message>
        <source>Up since %1

</source>
        <translation>В сети с %1

</translation>
    </message>
    <message>
        <source>Misc Modes</source>
        <translation>Различные режимы</translation>
    </message>
    <message>
        <source>Daemon Statistics

</source>
        <translation>Статистика Licq демона

</translation>
    </message>
    <message>
        <source>Daemon Statistics
(Today/Total)
</source>
        <translation>Статистика Licq демона
(Сегодня/Всего)
</translation>
    </message>
    <message>
        <source>SysMsg</source>
        <translation>Сист. сообщ.</translation>
    </message>
    <message>
        <source>System</source>
        <translation>Система</translation>
    </message>
    <message>
        <source>Send Authorization Re&amp;quest</source>
        <translation>Послат&amp;ь запрос авторизации</translation>
    </message>
    <message>
        <source>Last reset %1

</source>
        <translation>Последний сброс %1

</translation>
    </message>
    <message>
        <source>Are you sure you want to remove
%1 (%2)
from the &apos;%3&apos; group?</source>
        <translation>Вы уверены, что хотите удалить
%1 (%2)
из группы &apos;%3&apos;?</translation>
    </message>
    <message>
        <source>Do you really want to add
%1 (%2)
to your ignore list?</source>
        <translation>Вы действительно хотите добавить
%1 (%2)
в список игнорируемых?</translation>
    </message>
    <message>
        <source>Packets</source>
        <translation>Пакеты</translation>
    </message>
    <message>
        <source>&amp;Save All Users</source>
        <translation>&amp;Сохранить всех пользователей</translation>
    </message>
    <message>
        <source>%1: %2 / %3
</source>
        <translation>%1: %2 / %3
</translation>
    </message>
    <message>
        <source>Do Not Disturb to User</source>
        <translation>Виден в режиме &quot;Не беспокоить&quot;</translation>
    </message>
    <message>
        <source>Update All Users</source>
        <translation>Обновить всех пользователей</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Hints for Using&lt;br&gt;the Licq Qt-GUI Plugin&lt;/h2&gt;&lt;br&gt;&lt;hr&gt;&lt;br&gt;&lt;ul&gt;&lt;li&gt;Change your status by right clicking on the status label.&lt;/li&gt;&lt;li&gt;Change your auto response by double-clicking on the status label.&lt;/li&gt;&lt;li&gt;View system messages by double clicking on the message label.&lt;/li&gt;&lt;li&gt;Change groups by right clicking on the message label.&lt;/li&gt;&lt;li&gt;Use the following shortcuts from the contact list:&lt;ul&gt;&lt;li&gt;&lt;tt&gt;Ctrl-M : &lt;/tt&gt;Toggle mini-mode&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-O : &lt;/tt&gt;Toggle show offline users&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-X : &lt;/tt&gt;Exit&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-H : &lt;/tt&gt;Hide&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-I : &lt;/tt&gt;View the next message&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-V : &lt;/tt&gt;View message&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-S : &lt;/tt&gt;Send message&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-U : &lt;/tt&gt;Send Url&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-C : &lt;/tt&gt;Send chat request&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-F : &lt;/tt&gt;Send File&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-A : &lt;/tt&gt;Check Auto response&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-P : &lt;/tt&gt;Popup all messages&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-L : &lt;/tt&gt;Redraw user window&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Delete : &lt;/tt&gt;Delete user from current group&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-Delete : &lt;/tt&gt;Delete user from contact list&lt;/li&gt;&lt;/ul&gt;&lt;li&gt;Hold control while clicking on close in the function window to remove   the user from your contact list.&lt;/li&gt;&lt;li&gt;Hit Ctrl-Enter from most text entry fields to select &quot;Ok&quot; or &quot;Accept&quot;.   For example in the send tab of the user function window.&lt;/li&gt;&lt;li&gt;Here is the complete list of user % options, which can be used in &lt;b&gt;OnEvent&lt;/b&gt;   parameters, &lt;b&gt;auto responses&lt;/b&gt;, and &lt;b&gt;utilities&lt;/b&gt;:
</source>
        <translation>&lt;h2&gt;Советы по использованию&lt;br&gt; QT-модуля для Licq&lt;/h2&gt;&lt;br&gt;&lt;hr&gt;&lt;br&gt;&lt;ul&gt;&lt;li&gt;Статус можно изменить нажатием правой кнопки мыши на поле статуса.&lt;/li&gt;&lt;li&gt;Авто-ответ можно изменить двойным щелчком на поле отображения статуса.&lt;/li&gt;&lt;li&gt;Для просмотра системных сообщений можно использовать двойной щелчок на поле сообщений.&lt;/li&gt;&lt;li&gt;Выбор отображаемой группы можно осуществить нажатием правой кнопки мыши на поле сообщений.&lt;/li&gt;&lt;li&gt;Так же в списке контактов можно воспользоваться следующими &quot;горячими клавишами&quot;:&lt;ul&gt;&lt;li&gt;&lt;tt&gt;Ctrl-M : &lt;/tt&gt;Включение/выключение мини-режима&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-O : &lt;/tt&gt;Показывать/не показывать пользователей не в сети&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-X : &lt;/tt&gt;Выход&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-H : &lt;/tt&gt;Скрыть&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-I : &lt;/tt&gt;Просмотр следующего сообщения&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-V : &lt;/tt&gt;Просмотр сообщения&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-S : &lt;/tt&gt;Отправить сообщение&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-U : &lt;/tt&gt;Отправить Url&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-C : &lt;/tt&gt;Отправить запрос разговора&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-F : &lt;/tt&gt;Отправить файл(ы)&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-A : &lt;/tt&gt;Проверить авто-ответ&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-P : &lt;/tt&gt;Открыть все сообщения&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-L : &lt;/tt&gt;Обновить окно пользователей&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Delete : &lt;/tt&gt;Удалить пользователя из текущей группы&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-Delete : &lt;/tt&gt;Удалить пользователя из списка контактов&lt;/li&gt;&lt;/ul&gt;&lt;li&gt;Если нажать кнопку &quot;закрыть&quot; удерживая клавишу Ctrl,   то можно сразу же удалить пользователя из списка контактов.&lt;/li&gt;&lt;li&gt;Можно использовать Ctrl-Enter во многих полях ввода чтобы вызвать   нажатие &quot;Да&quot; или &quot;Подтвердить&quot;. Например в диалоге отправки сообщения.&lt;/li&gt;&lt;li&gt;Здесь полный список всех %-переменных, которые могут быть использованы    в качестве параметров в &lt;b&gt;команде на событие&lt;/b&gt;, &lt;b&gt;авто-ответах&lt;/b&gt;    и строчке запуска &lt;b&gt;утилит&lt;/b&gt;:
</translation>
    </message>
    <message>
        <source>&amp;Redraw User Window</source>
        <translation>&amp;Обновить окно пользователей</translation>
    </message>
    <message>
        <source>(with KDE support)
</source>
        <translation>(с поддержкой KDE)
</translation>
    </message>
    <message>
        <source>Occupied to User</source>
        <translation>Виден в режиме &quot;Занят&quot;</translation>
    </message>
    <message>
        <source>Logon failed.
See network window for details.</source>
        <translation>Вход в сеть не удался.
Подробности в Окне Сети.</translation>
    </message>
    <message>
        <source>Accept in Away</source>
        <translation>Принимать, если в режиме &quot;Отошел&quot;</translation>
    </message>
    <message>
        <source>Invisible List</source>
        <translation>В списке невидимости</translation>
    </message>
    <message>
        <source>Visible List</source>
        <translation>В списке видимости</translation>
    </message>
    <message>
        <source>Registration failed.  See network window for details.</source>
        <translation>Регистрация не удалась. Подробности в Окне Сети.</translation>
    </message>
    <message>
        <source>Send &amp;Chat Request</source>
        <translation>&amp;Запросить разговор</translation>
    </message>
    <message>
        <source>Reg&amp;ister User</source>
        <translation>&amp;Зарегистрировать пользователя</translation>
    </message>
    <message>
        <source>Show Offline &amp;Users</source>
        <translation>&amp;Показывать пользователей не в сети</translation>
    </message>
    <message>
        <source>%1 message%2</source>
        <translation>%1 сообщ%2</translation>
    </message>
    <message>
        <source>Ignore List</source>
        <translation>В списке игнорируемых</translation>
    </message>
    <message>
        <source>%1: %2
</source>
        <translation>%1: %2
</translation>
    </message>
    <message>
        <source>&amp;More Info</source>
        <translation>&amp;Подробнее</translation>
    </message>
    <message>
        <source>Warnings</source>
        <translation>Предупреждения</translation>
    </message>
    <message>
        <source>&amp;Work Info</source>
        <translation>&amp;Информация о работе</translation>
    </message>
    <message>
        <source>Debug Level</source>
        <translation>Уровень отладки</translation>
    </message>
    <message>
        <source>New Users</source>
        <translation>Новые пользователи</translation>
    </message>
    <message>
        <source>&amp;Network Window</source>
        <translation>&amp;Окно Сети</translation>
    </message>
    <message>
        <source>&amp;View Event</source>
        <translation>&amp;Просмотр событий</translation>
    </message>
    <message>
        <source>&amp;Plugin Manager...</source>
        <translation>&amp;Менеджер модулей...</translation>
    </message>
    <message>
        <source>O&amp;ccupied</source>
        <translation>&amp;Занят</translation>
    </message>
    <message>
        <source>Set All</source>
        <translation>Установить все</translation>
    </message>
    <message>
        <source>&amp;View System Messages</source>
        <translation>&amp;Просмотр системных сообщений</translation>
    </message>
    <message>
        <source>&amp;Thread Group View</source>
        <translation>&amp;По группам</translation>
    </message>
    <message>
        <source>Send &amp;Message</source>
        <translation>&amp;Отправить сообщение</translation>
    </message>
    <message>
        <source>Update Current Group</source>
        <translation>Обновить текущую группу</translation>
    </message>
    <message>
        <source>Licq (%1)</source>
        <translation>Licq (%1)</translation>
    </message>
    <message>
        <source>Right click - Status menu
Double click - Set auto response</source>
        <translation>Правая кнопка - Меню статуса
Двойной щелчок - Установить авто-ответ</translation>
    </message>
    <message>
        <source>&amp;Add User</source>
        <translation>&amp;Добавить пользователя</translation>
    </message>
    <message>
        <source>Accept in Do Not Disturb</source>
        <translation>Принимать, если в режиме &quot;Не беспокоить&quot;</translation>
    </message>
    <message>
        <source>Use Real Ip (LAN)</source>
        <translation>Использовать реальный IP адрес</translation>
    </message>
    <message>
        <source>&amp;Statistics</source>
        <translation>&amp;Статистика</translation>
    </message>
    <message>
        <source>&amp;Online</source>
        <translation>&amp;В сети</translation>
    </message>
    <message>
        <source>Clear All</source>
        <translation>Очистить все</translation>
    </message>
    <message>
        <source>&amp;Status</source>
        <translation>&amp;Статус</translation>
    </message>
    <message>
        <source>&amp;System</source>
        <translation>&amp;Система</translation>
    </message>
    <message>
        <source>Accept in Occupied</source>
        <translation>Принимать, если в режиме &quot;Занят&quot;</translation>
    </message>
    <message>
        <source>Send Contact &amp;List</source>
        <translation>&amp;Отправить список контактов</translation>
    </message>
    <message>
        <source>Status Info</source>
        <translation>Статус</translation>
    </message>
    <message>
        <source>&amp;Do Not Disturb</source>
        <translation>&amp;Не беспокоить</translation>
    </message>
    <message>
        <source>Are you sure you want to remove
%1 (%2)
from your contact list?</source>
        <translation>Вы уверены, что хотите удалить
%1 (%2)
из списка контактов?</translation>
    </message>
    <message>
        <source>O&amp;ffline</source>
        <translation>&amp;Отключен</translation>
    </message>
    <message>
        <source>A&amp;uthorize User</source>
        <translation>&amp;Авторизовать пользователя</translation>
    </message>
    <message>
        <source>Free for C&amp;hat</source>
        <translation>&amp;Свободен для разговора</translation>
    </message>
    <message>
        <source>Right click - User groups
Double click - Show next message</source>
        <translation>Правая кнопка - Группы пользователей
Двойной щелчок - Показать следующее сообщение</translation>
    </message>
    <message>
        <source>No messages</source>
        <translation>Нет сообщений</translation>
    </message>
    <message>
        <source>&amp;Invisible</source>
        <translation>&amp;Невидимый</translation>
    </message>
    <message>
        <source>&amp;General Info</source>
        <translation>&amp;Общая информация</translation>
    </message>
    <message>
        <source>Send &amp;File Transfer</source>
        <translation>&amp;Отправить файл</translation>
    </message>
    <message>
        <source>User Functions</source>
        <translation>Пользовательские функции</translation>
    </message>
    <message>
        <source>Check Auto Response</source>
        <translation>Посмотреть Авто-ответ</translation>
    </message>
    <message>
        <source>Authorization granted.</source>
        <translation>Авторизация подтверждена.</translation>
    </message>
    <message>
        <source>System Functions</source>
        <translation>Системные операции</translation>
    </message>
    <message>
        <source>Edit &amp;Groups</source>
        <translation>&amp;Редактировать Группы</translation>
    </message>
    <message>
        <source>All Users</source>
        <translation>Все пользователи</translation>
    </message>
    <message>
        <source>S&amp;kin Browser...</source>
        <translation>&amp;Выбор внешнего вида</translation>
    </message>
    <message>
        <source>&amp;Security/Password Options</source>
        <translation>Настройки &amp;безопасности/пароля</translation>
    </message>
    <message>
        <source>Request &amp;Secure Channel</source>
        <translation>&amp;Запросить защищенный канал</translation>
    </message>
    <message>
        <source>S&amp;earch for User</source>
        <translation>&amp;Поиск пользователя</translation>
    </message>
    <message>
        <source>Next &amp;Server</source>
        <translation>&amp;Следующий сервер</translation>
    </message>
    <message>
        <source>&amp;History</source>
        <translation>&amp;История</translation>
    </message>
    <message>
        <source>Not Available to User</source>
        <translation>Виден в режиме &quot;Недоступен&quot;</translation>
    </message>
    <message>
        <source>Toggle &amp;Floaty</source>
        <translation>&amp;Вкл/Выкл плавающего окна</translation>
    </message>
    <message>
        <source>Sa&amp;ve Settings</source>
        <translation>&amp;Сохранить настройки</translation>
    </message>
    <message>
        <source>Away to User</source>
        <translation>Виден в режиме &quot;Отошел&quot;</translation>
    </message>
</context>
<context>
    <name>CQtLogWindow</name>
    <message>
        <source>&amp;Save</source>
        <translation>&amp;Сохранить</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Закрыть</translation>
    </message>
    <message>
        <source>Failed to open file:
%1</source>
        <translation>Ошибка при открытии файла:
%1</translation>
    </message>
    <message>
        <source>C&amp;lear</source>
        <translation>&amp;Очистить</translation>
    </message>
    <message>
        <source>Licq Network Log</source>
        <translation>Licq: Протокол сети</translation>
    </message>
</context>
<context>
    <name>CRandomChatDlg</name>
    <message>
        <source>Seeking Men</source>
        <translation>Поиск мужчин</translation>
    </message>
    <message>
        <source>Games</source>
        <translation>Игры</translation>
    </message>
    <message>
        <source>20 Something</source>
        <translation>20+</translation>
    </message>
    <message>
        <source>30 Something</source>
        <translation>30+</translation>
    </message>
    <message>
        <source>40 Something</source>
        <translation>40+</translation>
    </message>
    <message>
        <source>Seeking Women</source>
        <translation>Поиск женщин</translation>
    </message>
    <message>
        <source>Random chat search timed out.</source>
        <translation>Поиск случайного собеседника не удался - вышло время.</translation>
    </message>
    <message>
        <source>Random chat search had an error.</source>
        <translation>Поиск случайного собеседника не удался - ошибка.</translation>
    </message>
    <message>
        <source>No random chat user found in that group.</source>
        <translation>Не найдено ни одного пользователя в этой группе,
желающегоучаствовать в случайном разговоре.</translation>
    </message>
    <message>
        <source>50 Plus</source>
        <translation>50+</translation>
    </message>
    <message>
        <source>Romance</source>
        <translation>Романтики</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Отменить</translation>
    </message>
    <message>
        <source>Searching for Random Chat Partner...</source>
        <translation>Поиск случайного собеседника...</translation>
    </message>
    <message>
        <source>&amp;Search</source>
        <translation>&amp;Поиск</translation>
    </message>
    <message>
        <source>Students</source>
        <translation>Студенты</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Разное</translation>
    </message>
    <message>
        <source>Random Chat Search</source>
        <translation>Поиск случайного собеседника</translation>
    </message>
</context>
<context>
    <name>CRefuseDlg</name>
    <message>
        <source>Licq %1 Refusal</source>
        <translation>Licq: Отказ в %1</translation>
    </message>
    <message>
        <source>Refusal message for %1 with </source>
        <translation>Причина отказа в %1 с </translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <source>Refuse</source>
        <translation>Отказать</translation>
    </message>
</context>
<context>
    <name>CSetRandomChatGroupDlg</name>
    <message>
        <source>&amp;Set</source>
        <translation>&amp;Установить</translation>
    </message>
    <message>
        <source>done</source>
        <translation>готово</translation>
    </message>
    <message>
        <source>Seeking Men</source>
        <translation>Поиск мужчин</translation>
    </message>
    <message>
        <source>Games</source>
        <translation>Игры</translation>
    </message>
    <message>
        <source>error</source>
        <translation>ошибка</translation>
    </message>
    <message>
        <source>20 Something</source>
        <translation>20+</translation>
    </message>
    <message>
        <source>30 Something</source>
        <translation>30+</translation>
    </message>
    <message>
        <source>40 Something</source>
        <translation>40+</translation>
    </message>
    <message>
        <source>Seeking Women</source>
        <translation>Поиск женщин</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Закрыть</translation>
    </message>
    <message>
        <source>(none)</source>
        <translation>(нет)</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>вышло время</translation>
    </message>
    <message>
        <source>Set Random Chat Group</source>
        <translation>Установить группу для случайных разговоров</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>не удалось</translation>
    </message>
    <message>
        <source>50 Plus</source>
        <translation>50+</translation>
    </message>
    <message>
        <source>Romance</source>
        <translation>Романтики</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Отменить</translation>
    </message>
    <message>
        <source>Students</source>
        <translation>Студенты</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Разное</translation>
    </message>
    <message>
        <source>Setting Random Chat Group...</source>
        <translation>Устанавливается группа для случайных разговоров...</translation>
    </message>
</context>
<context>
    <name>CUserView</name>
    <message>
        <source>S</source>
        <translation>С</translation>
    </message>
    <message>
        <source>Thai</source>
        <translation>Тайский</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;nobr&gt;Ip: </source>
        <translation>&lt;br&gt;&lt;nobr&gt;Ip: </translation>
    </message>
    <message>
        <source>Greek</source>
        <translation>Греческий</translation>
    </message>
    <message>
        <source>Tamil</source>
        <translation>Тамильский</translation>
    </message>
    <message>
        <source>Cyrillic</source>
        <translation>Кирилица</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;nobr&gt;Idle: </source>
        <translation>&lt;br&gt;&lt;nobr&gt;Не занят: </translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;u&gt;Auto Response:&lt;/u&gt;</source>
        <translation>&lt;br&gt;&lt;u&gt;Авто-ответ:&lt;/u&gt;</translation>
    </message>
    <message>
        <source>Baltic</source>
        <translation>Балтийский</translation>
    </message>
    <message>
        <source>Arabic</source>
        <translation>Арабский</translation>
    </message>
    <message>
        <source>Hebrew</source>
        <translation>Иврит</translation>
    </message>
    <message>
        <source>Korean</source>
        <translation>Корейский</translation>
    </message>
    <message>
        <source>Online</source>
        <translation>В сети</translation>
    </message>
    <message>
        <source>Offline</source>
        <translation>Отключены</translation>
    </message>
    <message>
        <source>Western European</source>
        <translation>Западная европа</translation>
    </message>
    <message>
        <source>Esperanto</source>
        <translation>Эсперанто</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;nobr&gt;C: </source>
        <translation>&lt;br&gt;&lt;nobr&gt;С: </translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;nobr&gt;F: </source>
        <translation>&lt;br&gt;&lt;nobr&gt;Ф: </translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;nobr&gt;E: </source>
        <translation>&lt;br&gt;&lt;nobr&gt;E: </translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;nobr&gt;P: </source>
        <translation>&lt;br&gt;&lt;nobr&gt;T: </translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;nobr&gt;O: </source>
        <translation>&lt;br&gt;&lt;nobr&gt;Вошел: </translation>
    </message>
    <message>
        <source>Central European</source>
        <translation>Центральная европа</translation>
    </message>
    <message>
        <source>&lt;br&gt;Licq&amp;nbsp;%1/SSL</source>
        <translation>&lt;br&gt;Licq&amp;nbsp;%1/SSL</translation>
    </message>
    <message>
        <source>&lt;br&gt;Licq&amp;nbsp;%1</source>
        <translation>&lt;br&gt;Licq&amp;nbsp;%1</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;b&gt;Birthday&amp;nbsp;Today!&lt;/b&gt;</source>
        <translation>&lt;br&gt;&lt;b&gt;Сегодня&amp;nbsp;День&amp;nbsp;Рождения!&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Japanese</source>
        <translation>Японский</translation>
    </message>
    <message>
        <source>Ukrainian</source>
        <translation>Украинский</translation>
    </message>
    <message>
        <source>%1 Floaty (%2)</source>
        <translation>%1 (%2)</translation>
    </message>
    <message>
        <source>Chinese</source>
        <translation>Китайский</translation>
    </message>
    <message>
        <source>&lt;br&gt;Custom&amp;nbsp;Auto&amp;nbsp;Response</source>
        <translation>&lt;br&gt;Специальный&amp;nbsp;авто-ответ</translation>
    </message>
    <message>
        <source>&lt;br&gt;Secure&amp;nbsp;connection</source>
        <translation>&lt;br&gt;Защищенное&amp;nbsp;соединение</translation>
    </message>
    <message>
        <source>Turkish</source>
        <translation>Турецкий</translation>
    </message>
    <message>
        <source>Unicode</source>
        <translation>Уникод</translation>
    </message>
    <message>
        <source>Chinese Traditional</source>
        <translation>Китайский традиционный</translation>
    </message>
    <message>
        <source>&lt;/nobr&gt;</source>
        <translation>&lt;/nobr&gt;</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;nobr&gt;Logged In: </source>
        <translation>&lt;br&gt;&lt;nobr&gt;В сети: </translation>
    </message>
</context>
<context>
    <name>CUtilityDlg</name>
    <message>
        <source>GUI</source>
        <translation>Графический интерфейс</translation>
    </message>
    <message>
        <source>&amp;Run</source>
        <translation>&amp;Запустить</translation>
    </message>
    <message>
        <source>Done</source>
        <translation>Завершить</translation>
    </message>
    <message>
        <source>Done:</source>
        <translation>Завершено:</translation>
    </message>
    <message>
        <source>Edit:</source>
        <translation>Редактировать:</translation>
    </message>
    <message>
        <source>Edit final command</source>
        <translation>Редактировать команду</translation>
    </message>
    <message>
        <source>Command Window</source>
        <translation>Окно команд</translation>
    </message>
    <message>
        <source>Licq Utility: %1</source>
        <translation>Licq утилита: %1</translation>
    </message>
    <message>
        <source>C&amp;lose</source>
        <translation>&amp;Закрыть</translation>
    </message>
    <message>
        <source>User Fields</source>
        <translation>Пользовательские поля</translation>
    </message>
    <message>
        <source>Description:</source>
        <translation>Описание:</translation>
    </message>
    <message>
        <source>Internal</source>
        <translation>Встроенное</translation>
    </message>
    <message>
        <source>Command:</source>
        <translation>Команда:</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Отменить</translation>
    </message>
    <message>
        <source>Running:</source>
        <translation>Выполняется:</translation>
    </message>
    <message>
        <source>Failed:</source>
        <translation>Неудача:</translation>
    </message>
    <message>
        <source>Terminal</source>
        <translation>Терминал</translation>
    </message>
    <message>
        <source>Window:</source>
        <translation>Окно:</translation>
    </message>
</context>
<context>
    <name>ChangePassDlg</name>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;ОК</translation>
    </message>
    <message>
        <source>&amp;UIN:</source>
        <translation>&amp;UIN</translation>
    </message>
    <message>
        <source>Enter your UIN here.</source>
        <translation>Введите Ваш UIN.</translation>
    </message>
    <message>
        <source>Verify your ICQ password here.</source>
        <translation>Повторно введите Ваш пароль ICQ.</translation>
    </message>
    <message>
        <source>Passwords do not match, try again.</source>
        <translation>Пароли не совпадают, повторите еще.</translation>
    </message>
    <message>
        <source>Invalid password, must be 8 characters or less.</source>
        <translation>Неправильный пароль, должен быть не больше 8 символов.</translation>
    </message>
    <message>
        <source>Login Information</source>
        <translation>Информация входа в систему</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>Отм&amp;ена</translation>
    </message>
    <message>
        <source>&amp;Password:</source>
        <translation>&amp;Пароль:</translation>
    </message>
    <message>
        <source>&amp;Verify:</source>
        <translation>&amp;Проверка:</translation>
    </message>
    <message>
        <source>Enter your ICQ password here.</source>
        <translation>Введите Ваш пароль ICQ.</translation>
    </message>
    <message>
        <source>Licq - Set UIN/Password</source>
        <translation>Licq - Установка UIN/пароля</translation>
    </message>
</context>
<context>
    <name>ChatDlg</name>
    <message>
        <source>No</source>
        <translation>Нет</translation>
    </message>
    <message>
        <source>Yes</source>
        <translation>Да</translation>
    </message>
    <message>
        <source>Beep</source>
        <translation>Звонок</translation>
    </message>
    <message>
        <source>Bold</source>
        <translation>Утолщенный</translation>
    </message>
    <message>
        <source>Chat</source>
        <translation>Разговор</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation>Режим</translation>
    </message>
    <message>
        <source>Toggles Bold font</source>
        <translation>Устанавливает/отменяет утолщенный шрифт</translation>
    </message>
    <message>
        <source>Ignores user color settings</source>
        <translation>Игнорировать установки цвета пользователя</translation>
    </message>
    <message>
        <source>Toggles Italic font</source>
        <translation>Устанавливает/отменяет наклонный шрифт</translation>
    </message>
    <message>
        <source>Remote - Waiting for joiners...</source>
        <translation>Удаленный - ожидаем участников...</translation>
    </message>
    <message>
        <source>Licq - Chat</source>
        <translation>Licq: Разговор</translation>
    </message>
    <message>
        <source>&amp;Audio</source>
        <translation>&amp;Звук</translation>
    </message>
    <message>
        <source>Sends a Beep to all recipients</source>
        <translation>Отправить &quot;звонок&quot; всем</translation>
    </message>
    <message>
        <source>Unable to create new thread.
See Network Window for details.</source>
        <translation>Невозможно создать новую нить.
Подробности в окне сети.</translation>
    </message>
    <message>
        <source>&amp;Pane Mode</source>
        <translation>&amp;Оконный режим</translation>
    </message>
    <message>
        <source>Italic</source>
        <translation>Наклонный</translation>
    </message>
    <message>
        <source>Do you want to save the chat session?</source>
        <translation>Желаете сохранить разговор?</translation>
    </message>
    <message>
        <source>Licq - Chat %1</source>
        <translation>Licq: Разговор с %1</translation>
    </message>
    <message>
        <source>&amp;IRC Mode</source>
        <translation>&amp;IRC режим</translation>
    </message>
    <message>
        <source>Changes the foreground color</source>
        <translation>Изменяет цвет текста</translation>
    </message>
    <message>
        <source>Unable to load encoding &lt;b&gt;%1&lt;/b&gt;. Message contents may appear garbled.</source>
        <translation>Невозможно загрузить кодировку &lt;b&gt;%1&lt;/b&gt;. Отображение сообщения может быть некорректным.</translation>
    </message>
    <message>
        <source>Background color</source>
        <translation>Цвет фона</translation>
    </message>
    <message>
        <source>Local - %1</source>
        <translation>Локальный - %1</translation>
    </message>
    <message>
        <source>/%1.%2.chat</source>
        <translation>/%1.%2.chat</translation>
    </message>
    <message>
        <source>Remote - Not connected</source>
        <translation>Удаленный - Нет соединения</translation>
    </message>
    <message>
        <source>&amp;Close Chat</source>
        <translation>&amp;Завершить разговор</translation>
    </message>
    <message>
        <source>Remote - %1</source>
        <translation>Удаленный - %1</translation>
    </message>
    <message>
        <source>Underline</source>
        <translation>Подчеркнутый</translation>
    </message>
    <message>
        <source>%1 closed connection.</source>
        <translation>%1 закрыл соединение.</translation>
    </message>
    <message>
        <source>Remote - Connecting...</source>
        <translation>Удаленный - Соединяемся...</translation>
    </message>
    <message>
        <source>&amp;Save Chat</source>
        <translation>&amp;Сохранить разговор</translation>
    </message>
    <message>
        <source>Changes the background color</source>
        <translation>Изменяет цвет фона</translation>
    </message>
    <message>
        <source>Foreground color</source>
        <translation>Цвет текста</translation>
    </message>
    <message>
        <source>Unable to bind to a port.
See Network Window for details.</source>
        <translation>Невозможно выполнить привязку сокета к порту.
Подробности в окне сети.</translation>
    </message>
    <message>
        <source>Ignore user settings</source>
        <translation>Игнорировать установки пользователя</translation>
    </message>
    <message>
        <source>Unable to connect to the remote chat.
See Network Window for details.</source>
        <translation>Невозможно присоединиться к разговору.
Подробности в окне сети.</translation>
    </message>
</context>
<context>
    <name>CustomAwayMsgDlg</name>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;Да</translation>
    </message>
    <message>
        <source>&amp;Clear</source>
        <translation>&amp;Очистить</translation>
    </message>
    <message>
        <source>&amp;Hints</source>
        <translation>&amp;Совет</translation>
    </message>
    <message>
        <source>Set Custom Auto Response for %1</source>
        <translation>Установить персональный авто-ответ для %1</translation>
    </message>
    <message>
        <source>I am currently %1.
You can leave me a message.</source>
        <translation>Я сейчас в режиме &quot;%1&quot;
Вы можете оставить мне сообщение.</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Отменить</translation>
    </message>
</context>
<context>
    <name>EditFileDlg</name>
    <message>
        <source>&amp;Save</source>
        <translation>&amp;Сохранить</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Закрыть</translation>
    </message>
    <message>
        <source>Failed to open file:
%1</source>
        <translation>Ошибка открытия файла:
%1</translation>
    </message>
    <message>
        <source>Licq File Editor - %1</source>
        <translation>Licq: Редактор файлов - %1</translation>
    </message>
    <message>
        <source>[ Read-Only ]</source>
        <translation>[ Только для чтения ]</translation>
    </message>
</context>
<context>
    <name>EditGrpDlg</name>
    <message>
        <source>Ok</source>
        <translation>Да</translation>
    </message>
    <message>
        <source>Add</source>
        <translation>Добавить</translation>
    </message>
    <message>
        <source>&amp;Done</source>
        <translation>&amp;Завершить</translation>
    </message>
    <message>
        <source>&amp;Save</source>
        <translation>&amp;Сохранить</translation>
    </message>
    <message>
        <source>Edit Name</source>
        <translation>Изменить имя</translation>
    </message>
    <message>
        <source>Set New Users</source>
        <translation>Для новых пользователей</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <source>Groups</source>
        <translation>Группы</translation>
    </message>
    <message>
        <source>Are you sure you want to remove
the group &apos;%1&apos;?</source>
        <translation>Вы уверены, что хотите удалить
группу &apos;%1&apos;?</translation>
    </message>
    <message>
        <source>Remove</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <source>noname</source>
        <translation>новая</translation>
    </message>
    <message>
        <source>The group to which new users will be automatically added.  All new users will be in the local system group New Users but for server side storage will also be stored in the specified group.</source>
        <translation>Группа, в которую будут добавляться новые пользователи.  Все новые пользователи будут в локальной группе &quot;Новые пользователи&quot;, но для серверного списка контактов они будут в этой группе.</translation>
    </message>
    <message>
        <source>Save the name of a group being modified.</source>
        <translation>Сохранить имя модифицируемой группы.</translation>
    </message>
    <message>
        <source>New User:</source>
        <translation>Группа новых пользователей:</translation>
    </message>
    <message>
        <source>Set Default</source>
        <translation>Группа по умолчанию</translation>
    </message>
    <message>
        <source>Default:</source>
        <translation>По умолчанию:</translation>
    </message>
    <message>
        <source>The default group to start up in.</source>
        <translation>Группа по умолчанию при старте</translation>
    </message>
    <message>
        <source>Edit group name (hit enter to save).</source>
        <translation>Изменение имени группы (Нажмите Enter для сохранения).</translation>
    </message>
    <message>
        <source>Shift Down</source>
        <translation>Вниз</translation>
    </message>
    <message>
        <source>All Users</source>
        <translation>Все пользователи</translation>
    </message>
    <message>
        <source>Licq - Edit Groups</source>
        <translation>Licq - Редактирование групп</translation>
    </message>
    <message>
        <source>Shift Up</source>
        <translation>Вверх</translation>
    </message>
</context>
<context>
    <name>Event</name>
    <message>
        <source>SMS</source>
        <translation>SMS</translation>
    </message>
    <message>
        <source>URL</source>
        <translation>Ссылка</translation>
    </message>
    <message>
        <source>Message</source>
        <translation>Сообщение</translation>
    </message>
    <message>
        <source>Added to Contact List</source>
        <translation>Добавлен в список контактов</translation>
    </message>
    <message>
        <source>Web Panel</source>
        <translation>Web-панель</translation>
    </message>
    <message>
        <source>Plugin Event</source>
        <translation>Событие модуля</translation>
    </message>
    <message>
        <source>File Transfer</source>
        <translation>Передача файла(ов)</translation>
    </message>
    <message>
        <source>(cancelled)</source>
        <translation>(прервано)</translation>
    </message>
    <message>
        <source>Authorization Request</source>
        <translation>Запрос авторизации</translation>
    </message>
    <message>
        <source>Authorization Refused</source>
        <translation>Отказ в авторизации</translation>
    </message>
    <message>
        <source>Unknown Event</source>
        <translation>Неизвестное событие</translation>
    </message>
    <message>
        <source>Chat Request</source>
        <translation>Запрос разговора</translation>
    </message>
    <message>
        <source>Authorization Granted</source>
        <translation>Авторизация подтверждена</translation>
    </message>
    <message>
        <source>Email Pager</source>
        <translation>Email-пейджер</translation>
    </message>
    <message>
        <source>Contact List</source>
        <translation>Список контактов</translation>
    </message>
</context>
<context>
    <name>HintsDlg</name>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Закрыть</translation>
    </message>
    <message>
        <source>Licq - Hints</source>
        <translation>Licq - Советы</translation>
    </message>
</context>
<context>
    <name>IconManager_KDEStyle</name>
    <message>
        <source>&lt;br&gt;1 msg</source>
        <translation>&lt;br&gt;1 сообщ.</translation>
    </message>
    <message>
        <source>&lt;br&gt;%1 msgs</source>
        <translation>&lt;br&gt;%1 сообщ.</translation>
    </message>
    <message>
        <source>&lt;br&gt;Left click - Show main window&lt;br&gt;Middle click - Show next message&lt;br&gt;Right click - System menu</source>
        <translation>&lt;br&gt;Левая кнопка - Основное окно&lt;br&gt;Средняя кнопка - Следующее сообщение&lt;br&gt;Правая кнопка - Системное меню</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;b&gt;%1 system messages&lt;/b&gt;</source>
        <translation>&lt;br&gt;&lt;b&gt;%1 сист. сообщ.&lt;/b&gt;</translation>
    </message>
</context>
<context>
    <name>IconManager_Themed</name>
    <message>
        <source>Unable to load dock theme image
%1</source>
        <translation>Невозможно загрузить файл с изображением для дока
%1</translation>
    </message>
    <message>
        <source>Unable to load dock theme file
(%1)
:%2</source>
        <translation>Невозможно загрузить файл с темой для дока
(%1)
:%2</translation>
    </message>
</context>
<context>
    <name>KeyRequestDlg</name>
    <message>
        <source>&amp;Send</source>
        <translation>&amp;Отправить</translation>
    </message>
    <message>
        <source>Closing secure channel...</source>
        <translation>Закрываем защищенное соединение...</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Закрыть</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;ForestGreen&quot;&gt;Secure channel established.&lt;/font&gt;
</source>
        <translation>&lt;font color=&quot;ForestGreen&quot;&gt;Защищенное соединение установлено.&lt;/font&gt;
</translation>
    </message>
    <message>
        <source>Client does not support OpenSSL.
Rebuild Licq with OpenSSL support.</source>
        <translation>Клиент не поддерживает OpenSSL.
Пересоберите Licq с поддержкой OpenSSL.</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;blue&quot;&gt;Secure channel closed.&lt;/font&gt;
</source>
        <translation>&lt;font color=&quot;blue&quot;&gt;Защищенное соединение закрыто.&lt;/font&gt;
</translation>
    </message>
    <message>
        <source>Licq - Secure Channel with %1</source>
        <translation>Licq - Защищенное соединение с %1</translation>
    </message>
    <message>
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
        <source>&lt;font color=&quot;red&quot;&gt;Could not connect to remote client.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;Невозможно соединиться с удаленной стороной.&lt;/font&gt;</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;yellow&quot;&gt;Secure channel already established.&lt;/font&gt;
</source>
        <translation>&lt;font color=&quot;yellow&quot;&gt;Защищенное соединение уже установлено.&lt;/font&gt;
</translation>
    </message>
    <message>
        <source>The remote uses Licq %1, however it
has no secure channel support compiled in.
This probably won&apos;t work.</source>
        <translation>Удаленная сторона использует Licq %1/SSL.</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;yellow&quot;&gt;Secure channel not established.&lt;/font&gt;
</source>
        <translation>&lt;font color=&quot;yellow&quot;&gt;Защищенное соединение не установлено.&lt;/font&gt;
</translation>
    </message>
    <message>
        <source>The remote uses Licq %1/SSL.</source>
        <translation>Удаленная сторона использует Licq %1/SSL.</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;Remote client does not support OpenSSL.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;Удаленная сторона не поддерживает OpenSSL.&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Requesting secure channel...</source>
        <translation>Запрашиваем защищенное соединение...</translation>
    </message>
    <message>
        <source>This only works with other Licq clients &gt;= v0.85
The remote doesn&apos;t seem to use such a client.
This might not work.</source>
        <translation>Это работает только с другими Licq клиентами 
версии старше, чем 0.85. Удаленная сторона не 
использует подобного клиента.
Соединение установлено не будет.</translation>
    </message>
</context>
<context>
    <name>MLView</name>
    <message>
        <source>Licq is unable to start your browser and open the URL.
You will need to start the browser and open the URL manually.</source>
        <translation>Licq не в состоянии запустить браузер и открыть ссылку.
Вам небходимо запустить браузер вручную.</translation>
    </message>
    <message>
        <source>Licq is unable to find a browser application due to an internal error.</source>
        <translation>Licq не в состоянии найти приложение-браузер в следствии внутренней ошибки.</translation>
    </message>
</context>
<context>
    <name>MsgView</name>
    <message>
        <source>D</source>
        <translation>D</translation>
    </message>
    <message>
        <source>Time</source>
        <translation>Время</translation>
    </message>
    <message>
        <source>Cancelled Event</source>
        <translation>Отмененное событие</translation>
    </message>
    <message>
        <source>Direct</source>
        <translation>Напрямую</translation>
    </message>
    <message>
        <source>Event Type</source>
        <translation>Тип события</translation>
    </message>
    <message>
        <source>Server</source>
        <translation>Через сервер</translation>
    </message>
    <message>
        <source>Urgent</source>
        <translation>Срочно</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>Параметры</translation>
    </message>
    <message>
        <source>Multiple Recipients</source>
        <translation>Несколько адресатов</translation>
    </message>
</context>
<context>
    <name>OptionsDlg</name>
    <message>
        <source>IP</source>
        <translation>IP</translation>
    </message>
    <message>
        <source><byte value="x9"/>to</source>
        <translation><byte value="x9"/>до</translation>
    </message>
    <message>
        <source>&amp;OK</source>
        <translation>&amp;Да</translation>
    </message>
    <message>
        <source>Fax</source>
        <translation>Факс</translation>
    </message>
    <message>
        <source>Auto</source>
        <translation>Автоматически</translation>
    </message>
    <message>
        <source>Away</source>
        <translation>Отошел</translation>
    </message>
    <message>
        <source>Font</source>
        <translation>Шрифт</translation>
    </message>
    <message>
        <source>N/A:</source>
        <translation>Авто-недоступен:</translation>
    </message>
    <message>
        <source>Left</source>
        <translation>Слева</translation>
    </message>
    <message>
        <source>Save</source>
        <translation>Сохранить</translation>
    </message>
    <message>
        <source>URL:</source>
        <translation>Ссылка:</translation>
    </message>
    <message>
        <source>none</source>
        <translation>не перекодировать</translation>
    </message>
    <message>
        <source>Show Extended Icons</source>
        <translation>Показывать дополнительные иконки</translation>
    </message>
    <message>
        <source>Blink All Events</source>
        <translation>Все сообщения должны мигать</translation>
    </message>
    <message>
        <source>Away:</source>
        <translation>Авто-отошел:</translation>
    </message>
    <message>
        <source>Email</source>
        <translation>Email</translation>
    </message>
    <message>
        <source>Font:</source>
        <translation>Шрифт:</translation>
    </message>
    <message>
        <source>HTTPS</source>
        <translation>HTTPS</translation>
    </message>
    <message>
        <source>Never</source>
        <translation>Никогда</translation>
    </message>
    <message>
        <source>Phone</source>
        <translation>Телефон</translation>
    </message>
    <message>
        <source>Proxy</source>
        <translation>Прокси сервер</translation>
    </message>
    <message>
        <source>Right</source>
        <translation>Справа</translation>
    </message>
    <message>
        <source>Text:</source>
        <translation>Текст:</translation>
    </message>
    <message>
        <source>Title</source>
        <translation>Заголовок</translation>
    </message>
    <message>
        <source>Width</source>
        <translation>Ширина</translation>
    </message>
    <message>
        <source>Parameter for received file transfers</source>
        <translation>Параметры для принятых запросов передачи файлов</translation>
    </message>
    <message>
        <source>Firewall</source>
        <translation>Межсетевой экран</translation>
    </message>
    <message>
        <source>Use proxy server</source>
        <translation>Использовать прокси сервер</translation>
    </message>
    <message>
        <source>Show Grid Lines</source>
        <translation>Показывать разделительные линии</translation>
    </message>
    <message>
        <source>Override the skin setting for the frame style of the user window:
   0 (No frame), 1 (Box), 2 (Panel), 3 (WinPanel)
 + 16 (Plain), 32 (Raised), 48 (Sunken)
 + 240 (Shadow)</source>
        <translation>Использовать указанный стиль рамки, взамен того, что записан в конфигурации текущего &quot;Внешнего вида&quot;
Подсказка:
   0 (Нет рамки), 1 (Коробка), 2 (Панель), 3 (Панель в стиле Windows)
 + 16 (Простая), 32 (Рельефная), 48 (Утопленная)
 + 240 (С тенью)</translation>
    </message>
    <message>
        <source>Parameter for received chat requests</source>
        <translation>Параметры для принятых запросов на разговор</translation>
    </message>
    <message>
        <source>Auto Away Messages</source>
        <translation>Сообщения при авто-режимах</translation>
    </message>
    <message>
        <source>Turns on or off the display of headers above each column in the user list</source>
        <translation>Включает и выключает показ названий столбцов в списке пользователей</translation>
    </message>
    <message>
        <source>Auto close the user function window after a succesful event</source>
        <translation>Автоматически закрывать окна сообщений после удачных событий</translation>
    </message>
    <message>
        <source>Start Hidden</source>
        <translation>При запуске скрыть главное окно</translation>
    </message>
    <message>
        <source>Auto Away:</source>
        <translation>Авто-отошел:</translation>
    </message>
    <message>
        <source>Number of minutes of inactivity after which to automatically be marked &quot;not available&quot;.  Set to &quot;0&quot; to disable.</source>
        <translation>Время бездействия (в минутах), после которого Ваш статус автоматически меняется на &quot;Не доступен&quot;. Введите &quot;0&quot; чтобы отключить.</translation>
    </message>
    <message>
        <source>Blink Urgent Events</source>
        <translation>Все срочные сообщения
должны мигать</translation>
    </message>
    <message>
        <source>Ignore Mass Messages</source>
        <translation>Игнорировать массовую рассылку</translation>
    </message>
    <message>
        <source>Show the &quot;--online--&quot; and &quot;--offline--&quot; bars in the contact list</source>
        <translation>Показывать разделители &quot;-- Отключены --&quot;&quot;-- В сети --&quot; в списке контактов</translation>
    </message>
    <message>
        <source>ICQ Server Port:</source>
        <translation>Порт сервера ICQ:</translation>
    </message>
    <message>
        <source>Start main window hidden. Only the dock icon will be visible.</source>
        <translation>Скрывать главное окно после запуска. Только иконка дока будет на экране.</translation>
    </message>
    <message>
        <source>Parameter for online notification</source>
        <translation>Параметры для оповещения о появлении в сети</translation>
    </message>
    <message>
        <source>TCP port range for incoming connections.</source>
        <translation>Диапазон TCP портов для входящих соединений.</translation>
    </message>
    <message>
        <source>Accept Modes</source>
        <translation>Разрешить команды на событие</translation>
    </message>
    <message>
        <source>Online Notify:</source>
        <translation>При появлении:</translation>
    </message>
    <message>
        <source>Idle Time</source>
        <translation>Время бездействия</translation>
    </message>
    <message>
        <source>Not Available</source>
        <translation>Недоступен</translation>
    </message>
    <message>
        <source>&amp;Apply</source>
        <translation>&amp;Применить</translation>
    </message>
    <message>
        <source>Determines if new users are automatically added to your list or must first request authorization.</source>
        <translation>Определяет, должны ли новые пользователи запрашивать Вашу авторизацию,или они могут быть добавлены в список пользователей автоматически.</translation>
    </message>
    <message>
        <source>Show the name of the current group in the messages label when there are no new messages</source>
        <translation>Показывает название текущей группы в случае отсутствия новых сообщений</translation>
    </message>
    <message>
        <source>Ignore Web Panel</source>
        <translation>Игнорировать Web-панель</translation>
    </message>
    <message>
        <source>Sets which default encoding should be used for newly added contacts.</source>
        <translation>Устанавливает кодировку по умолчанию для новых контактов.</translation>
    </message>
    <message>
        <source>Show the current chat history in Send Window</source>
        <translation>Показывает историю разговора в окне отправки сообщения</translation>
    </message>
    <message>
        <source>Use Font Styles</source>
        <translation>Использовать стили шрифтов</translation>
    </message>
    <message>
        <source>Preset slot:</source>
        <translation>Заготовки:</translation>
    </message>
    <message>
        <source>Automatically send messages through the server if direct connection fails</source>
        <translation>Автоматиматически посылать сообщения через сервер если прямое соединение не удалось установить</translation>
    </message>
    <message>
        <source>Licq Options</source>
        <translation>Настройки Licq</translation>
    </message>
    <message>
        <source>Show online notify users who are offline even when offline users are hidden.</source>
        <translation>Разрешает показывать пользователей, для которых включено оповещение и которые сейчас не в сети, даже если стоит запрет на показ пользователей не в сети.</translation>
    </message>
    <message>
        <source>Show Column Headers</source>
        <translation>Показывать заголовки столбцов</translation>
    </message>
    <message>
        <source>Bold Message Label on Incoming Msg</source>
        <translation>Выделять утолщенным шрифтом
при приходе сообщения</translation>
    </message>
    <message>
        <source>Column %1</source>
        <translation>Столбец %1</translation>
    </message>
    <message>
        <source>Selects between the standard 64x64 icon used in the WindowMaker/Afterstep wharf and a shorter 64x48 icon for use in the Gnome/KDE panel.</source>
        <translation>Выбор между иконкой размером 64x64, используемой в доке WindowMaker/Afterstep и уменьшенной иконкой размером 64x48, используемой для панели Gnome/KDE.</translation>
    </message>
    <message>
        <source>Perform OnEvent command in occupied mode</source>
        <translation>Выполнять команды на событие в режиме &quot;Занят&quot;</translation>
    </message>
    <message>
        <source>Auto Offline:</source>
        <translation>Авто-выход из сети:</translation>
    </message>
    <message>
        <source>Online Time</source>
        <translation>Время в сети</translation>
    </message>
    <message>
        <source>Lets you drag around the main window with your mouse</source>
        <translation>Позволяет переносить объекты в главном окнес помощью мыши</translation>
    </message>
    <message>
        <source>OnEvent in DND</source>
        <translation>В режиме &quot;Не беспокоить&quot;</translation>
    </message>
    <message>
        <source>OnEvent in N/A</source>
        <translation>В режиме &quot;Недоступен&quot;</translation>
    </message>
    <message>
        <source>Parameter for received messages</source>
        <translation>Параметры для принятых сообщений</translation>
    </message>
    <message>
        <source>Message Sent:</source>
        <translation>Сообщение отправлено:</translation>
    </message>
    <message>
        <source>Center</source>
        <translation>По центру</translation>
    </message>
    <message>
        <source>Proxy Server Port:</source>
        <translation>Порт прокси сервера:</translation>
    </message>
    <message>
        <source>Allow scroll bar</source>
        <translation>Разрешить полосу прокрутки</translation>
    </message>
    <message>
        <source>OnEvent in Away</source>
        <translation>В режиме &quot;Отошел&quot;</translation>
    </message>
    <message>
        <source>Network</source>
        <translation>Сеть</translation>
    </message>
    <message>
        <source>Only urgent events will blink</source>
        <translation>Только срочные сообщения должны мигать</translation>
    </message>
    <message>
        <source>Format</source>
        <translation>Формат</translation>
    </message>
    <message>
        <source>The fonts used</source>
        <translation>Используемый шрифт</translation>
    </message>
    <message>
        <source>Number of minutes of inactivity after which to automatically go offline.  Set to &quot;0&quot; to disable.</source>
        <translation>Время бездействия (в минутах), после которого Ваш статус автоматически меняется на &quot;Отключен&quot;. Введите &quot;0&quot; чтобы отключить.</translation>
    </message>
    <message>
        <source>Auto send through server</source>
        <translation>Автоотправка через сервер</translation>
    </message>
    <message>
        <source>Use Dock Icon</source>
        <translation>Иконка для дока</translation>
    </message>
    <message>
        <source>Chat Request:</source>
        <translation>Запрос разговора</translation>
    </message>
    <message>
        <source>Online</source>
        <translation>В сети</translation>
    </message>
    <message>
        <source>Server settings</source>
        <translation>Установки сервера</translation>
    </message>
    <message>
        <source>Auto Logon:</source>
        <translation>Авто-логин:</translation>
    </message>
    <message>
        <source>Main Window</source>
        <translation>Главное окно</translation>
    </message>
    <message>
        <source>Status</source>
        <translation>Статус</translation>
    </message>
    <message>
        <source>Theme:</source>
        <translation>Тема:</translation>
    </message>
    <message>
        <source>Always show online notify users</source>
        <translation>Всегда показывать пользователей, 
для которых включено оповещение.</translation>
    </message>
    <message>
        <source>default (%1)</source>
        <translation>по умолчанию (%1)</translation>
    </message>
    <message>
        <source>Offline</source>
        <translation>Отключен</translation>
    </message>
    <message>
        <source>Port Range:</source>
        <translation>Диапазон портов</translation>
    </message>
    <message>
        <source>Allow dragging main window</source>
        <translation>Разрешить перенос объектов для основного окна</translation>
    </message>
    <message>
        <source>Show the message info label in bold font if there are incoming messages</source>
        <translation>Выделять жирным шрифтом входящие сообщения</translation>
    </message>
    <message>
        <source>If not checked, a user will be automatically removed from &quot;New User&quot; group when you first send an event to them</source>
        <translation>Если не установлен, то пользователь будет автоматически удален из группы &quot;Новые пользователи&quot; после первой отправки ему сообщения, файла и т.д.</translation>
    </message>
    <message>
        <source>Default Encoding:</source>
        <translation>Кодировка по умолчанию</translation>
    </message>
    <message>
        <source>OnEvent</source>
        <translation>Команды на событие</translation>
    </message>
    <message>
        <source>Localization</source>
        <translation>Локализация</translation>
    </message>
    <message>
        <source>Command:</source>
        <translation>Команда:</translation>
    </message>
    <message>
        <source>Edit Font:</source>
        <translation>Шрифт в редакторе:</translation>
    </message>
    <message>
        <source>Hot key: </source>
        <translation>Горячая клавиша: </translation>
    </message>
    <message>
        <source>I can receive direct connections</source>
        <translation>Прием прямых соединений разрешен</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>Настройки</translation>
    </message>
    <message>
        <source>Miscellaneous</source>
        <translation>Разное</translation>
    </message>
    <message>
        <source>Perform OnEvent command in do not disturb mode</source>
        <translation>Выполнять команды на событие в режиме &quot;Не беспокоить&quot;</translation>
    </message>
    <message>
        <source>Determines if mass messages are ignored or not.</source>
        <translation>Определяет игнорировать ли сообщения со многими адресатами.</translation>
    </message>
    <message>
        <source>Last online</source>
        <translation>Последний раз в сети</translation>
    </message>
    <message>
        <source>Frame Style: </source>
        <translation>Стиль фреймов: </translation>
    </message>
    <message>
        <source>The alignment of the column</source>
        <translation>Выравнивание столбцов</translation>
    </message>
    <message>
        <source>Auto Position the Reply Window</source>
        <translation>Автоматический выбор места для окна ответа</translation>
    </message>
    <message>
        <source>Make the user window transparent when there is no scroll bar</source>
        <translation>Делает окно пользователей прозрачным, когда нет полосы прокрутки</translation>
    </message>
    <message>
        <source>Default Icon</source>
        <translation>Значок по умолчанию</translation>
    </message>
    <message>
        <source>Check Clipboard For Urls/Files</source>
        <translation>Проверять буфер обмена на наличие файлов/URL</translation>
    </message>
    <message>
        <source>Parameters</source>
        <translation>Параметры</translation>
    </message>
    <message>
        <source>Show birthday, invisible, and custom auto response icons to the right of users in the list</source>
        <translation>Показывать дополнительные иконки (день рождения, невидимость, специальный авто-ответ) справа от имени пользователя.</translation>
    </message>
    <message>
        <source>Transparent when possible</source>
        <translation>Прозрачное, если возможно</translation>
    </message>
    <message>
        <source>Default Auto Response Messages</source>
        <translation>Авто-ответы по умолчанию</translation>
    </message>
    <message>
        <source>File Transfer:</source>
        <translation>Передача файла:</translation>
    </message>
    <message>
        <source>OnEvents Enabled</source>
        <translation>Команды на событие разрешены</translation>
    </message>
    <message>
        <source>Paranoia</source>
        <translation>Паранойя</translation>
    </message>
    <message>
        <source>Auto-Raise on Incoming Msg</source>
        <translation>Авто-всплытие при приходе сообщения</translation>
    </message>
    <message>
        <source>Position a new reply window just underneath the message view window</source>
        <translation>Окно ответа будет располагаться под окном с сообщением</translation>
    </message>
    <message>
        <source>Terminal:</source>
        <translation>Терминал:</translation>
    </message>
    <message>
        <source>Automatically log on when first starting up.</source>
        <translation>Автоматически входить в сеть при старте.</translation>
    </message>
    <message>
        <source>Perform the online notify OnEvent when logging on (this is different from how the Mirabilis client works)</source>
        <translation>Оповещает о появлении при входе пользователя в ICQ-сеть. (Это отличается от поведения клиента от Mirabilis)</translation>
    </message>
    <message>
        <source>Number of minutes of inactivity after which to automatically be marked &quot;away&quot;.  Set to &quot;0&quot; to disable.</source>
        <translation>Время бездействия (в минутах), после которого Ваш статус автоматически меняется на &quot;Отошел&quot;. Введите &quot;0&quot; чтобы отключить.</translation>
    </message>
    <message>
        <source>&lt;p&gt;Command to execute when an event is received.&lt;br&gt;It will be passed the relevant parameters from below.&lt;br&gt;Parameters can contain the following expressions &lt;br&gt; which will be replaced with the relevant information:&lt;/p&gt;</source>
        <translation>&lt;p&gt;Команда, которая будет выполнена по приходу события.&lt;br&gt;Ей будут переданы параметры, указанные ниже.&lt;br&gt;Также будут выполнены следующие подстановки:&lt;/p&gt;</translation>
    </message>
    <message>
        <source>ICQ Server:</source>
        <translation>Сервер ICQ:</translation>
    </message>
    <message>
        <source>Determines if email pager messages are ignored or not.</source>
        <translation>Определяет игнорировать ли сообщения с email-пейджера.</translation>
    </message>
    <message>
        <source>Use authorization</source>
        <translation>Сервер требует авторизацию</translation>
    </message>
    <message>
        <source>Perform OnEvent command in not available mode</source>
        <translation>Выполнять команды на событие в режиме &quot;Недоступен&quot;</translation>
    </message>
    <message>
        <source>Occupied</source>
        <translation>Занят</translation>
    </message>
    <message>
        <source>Show User Dividers</source>
        <translation>Показывать разделители
пользователей</translation>
    </message>
    <message>
        <source>OnEvent in Occupied</source>
        <translation>В режиме &quot;Занят&quot;</translation>
    </message>
    <message>
        <source>The format string used to define what will appear in each column.
The following parameters can be used:
</source>
        <translation>Строка, определяющая что будет показываться в каждом столбце.
Могут быть использованы следующие параметры:
</translation>
    </message>
    <message>
        <source>Parameter for received system messages</source>
        <translation>Параметры для принятых системных сообщений</translation>
    </message>
    <message>
        <source>Manual &quot;New User&quot; group handling</source>
        <translation>Ручная обработка группы &quot;Новые пользователи&quot;</translation>
    </message>
    <message>
        <source>Parameter for sent messages</source>
        <translation>Параметр для отправленных сообщений</translation>
    </message>
    <message>
        <source>Select Font</source>
        <translation>Выбор шрифта</translation>
    </message>
    <message>
        <source>Auto-Popup Incoming Msg</source>
        <translation>Авто-показ приходящих сообщений</translation>
    </message>
    <message>
        <source>64 x 48 Dock Icon</source>
        <translation>64 x 48 иконка для дока</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Отменить</translation>
    </message>
    <message>
        <source>Startup</source>
        <translation>При старте</translation>
    </message>
    <message>
        <source>Status:</source>
        <translation>Статус:</translation>
    </message>
    <message>
        <source>Password:</source>
        <translation>Пароль:</translation>
    </message>
    <message>
        <source>default</source>
        <translation>по умолчанию</translation>
    </message>
    <message>
        <source>Use italics and bold in the user list to indicate special characteristics such as online notify and visible list</source>
        <translation>Использовать наклонный и утолщенный шрифт в списке пользователей для обозначения специальных характеристик, таких как оповещение о наличии в сети и присутствие в списке видимости</translation>
    </message>
    <message>
        <source>Use System Background Color</source>
        <translation>Использовать фон по умолчанию</translation>
    </message>
    <message>
        <source>Proxy Type:</source>
        <translation>Тип прокси сервера:</translation>
    </message>
    <message>
        <source>Auto N/A:</source>
        <translation>Авто-недоступен:</translation>
    </message>
    <message>
        <source>Themed Icon</source>
        <translation>Иконка с темой</translation>
    </message>
    <message>
        <source>Auto Close Function Window</source>
        <translation>Авто-закрытие функциональных окон</translation>
    </message>
    <message>
        <source>Perform OnEvent command in away mode</source>
        <translation>Выполнять команды на событие в режиме &quot;Отошел&quot;</translation>
    </message>
    <message>
        <source>Docking</source>
        <translation>Использование дока</translation>
    </message>
    <message>
        <source>Determines if web panel messages are ignored or not.</source>
        <translation>Определяет игнорировать ли сообщения, отправленные через web-панель.</translation>
    </message>
    <message>
        <source>Enable running of &quot;Command&quot; when the relevant event occurs.</source>
        <translation>Разрешает запуск команды при приходе соответствующего события</translation>
    </message>
    <message>
        <source>System Msg:</source>
        <translation>Сист. сообщ.:</translation>
    </message>
    <message>
        <source>Show group name if no messages</source>
        <translation>Показывать название группы
при отсутствии сообщений</translation>
    </message>
    <message>
        <source>Free for Chat</source>
        <translation>Свободен для разговора</translation>
    </message>
    <message>
        <source>Free For Chat</source>
        <translation>Свободен для разговора</translation>
    </message>
    <message>
        <source>When double-clicking on a user to send a message check for urls/files in the clipboard</source>
        <translation>Проверять буфер обмена на наличие URL или файлов при попытке отправить сообщение пользователю.</translation>
    </message>
    <message>
        <source>Show all encodings</source>
        <translation>Показывать все кодировки</translation>
    </message>
    <message>
        <source>Online Notify when Logging On</source>
        <translation>Оповещение при появлении в сети</translation>
    </message>
    <message>
        <source>Popup info</source>
        <translation>Всплывающая подсказка</translation>
    </message>
    <message>
        <source>Cellular</source>
        <translation>Сотовый</translation>
    </message>
    <message>
        <source>Extensions</source>
        <translation>Расширения</translation>
    </message>
    <message>
        <source>Do Not Disturb</source>
        <translation>Не беспокоить</translation>
    </message>
    <message>
        <source>All incoming events will blink</source>
        <translation>Все входящие сообщения должны мигать</translation>
    </message>
    <message>
        <source>Username:</source>
        <translation>Имя пользователя:</translation>
    </message>
    <message>
        <source>Url Viewer:</source>
        <translation>Программа просмотра URL:</translation>
    </message>
    <message>
        <source>Message:</source>
        <translation>Сообщение:</translation>
    </message>
    <message>
        <source>Draw boxes around each square in the user list</source>
        <translation>Определяет, рисовать ли рамку вокруг каждого пользователя в списке</translation>
    </message>
    <message>
        <source>Show all available encodings in the User Encoding selection menu. Normally, this menu shows only commonly used encodings.</source>
        <translation>Отображает все доступные кодировки в меню выбора кодировки пользователя. Обычно отображаются только наиболее часто используемые кодировки.</translation>
    </message>
    <message>
        <source>Column Configuration</source>
        <translation>Конфигурация столбцов</translation>
    </message>
    <message>
        <source>Invisible</source>
        <translation>Невидимый</translation>
    </message>
    <message>
        <source>Open all incoming messages automatically when received if we are online (or free for chat)</source>
        <translation>Открывать все входящие сообщения автоматически только если мы в сети (или доступны для разговора)</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Внешний вид</translation>
    </message>
    <message>
        <source>Ignore Email Pager</source>
        <translation>Игнорировать Email-пейджер</translation>
    </message>
    <message>
        <source>Chatmode Messageview</source>
        <translation>Просмотр сообщений в режиме разговора</translation>
    </message>
    <message>
        <source>Controls whether or not the dockable icon should be displayed.</source>
        <translation>Управляет отображением иконки для дока</translation>
    </message>
    <message>
        <source>Sort Online Users by Status</source>
        <translation>Сортировать пользователей,
которые в сети, по статусу</translation>
    </message>
    <message>
        <source>Proxy Server:</source>
        <translation>Прокси сервер:</translation>
    </message>
    <message>
        <source>Raise the main window on incoming messages</source>
        <translation>Поднимать основное окно при получении входящих сообщений</translation>
    </message>
    <message>
        <source>Contact List</source>
        <translation>Список контактов</translation>
    </message>
    <message>
        <source>System default (%1)</source>
        <translation>Системная по умолчанию (%1)</translation>
    </message>
    <message>
        <source>The width of the column</source>
        <translation>Ширина столбца</translation>
    </message>
    <message>
        <source>Previous Message</source>
        <translation>Предыдущее сообщение</translation>
    </message>
    <message>
        <source>Sort all online users by their actual status</source>
        <translation>Сортирует всех пользователей, которые в сети, по их фактическому статусу</translation>
    </message>
    <message>
        <source>Select a font from the system list</source>
        <translation>Выбрать шрифт из имеющихся в системе</translation>
    </message>
    <message>
        <source>The string which will appear in the list box column header</source>
        <translation>Строка, которая будет появляться в заголовке столбцов</translation>
    </message>
    <message>
        <source>Store your contacts on the server so they are accessible from different locations and/or programs</source>
        <translation>Ваш список контактов будет храниться на сервере, поэтому будет доступен на любой машине.</translation>
    </message>
    <message>
        <source>Parameter for received URLs</source>
        <translation>Параметры для принятых URL</translation>
    </message>
    <message>
        <source>The command to run to view a URL.  Will be passed the URL as a parameter.</source>
        <translation>Команда для просмотра URL. URL будет передан как параметр.</translation>
    </message>
    <message>
        <source>Allow the vertical scroll bar in the user list</source>
        <translation>Разрешает вертикальную полосу прокрутки в списке пользователей</translation>
    </message>
    <message>
        <source>The command to run to start your terminal program.</source>
        <translation>Команда для запуска вашей терминальной программы.</translation>
    </message>
    <message>
        <source>Font used in message editor etc.</source>
        <translation>Шрифт, используемый редактором сообщений и т.д.</translation>
    </message>
    <message>
        <source>Ignore New Users</source>
        <translation>Игнорировать новых пользователей</translation>
    </message>
    <message>
        <source>Use server side contact list</source>
        <translation>Хранить список контактов на сервере</translation>
    </message>
    <message>
        <source>Alignment</source>
        <translation>Выравнивание</translation>
    </message>
</context>
<context>
    <name>PasswordDlg</name>
    <message>
        <source>error</source>
        <translation>ошибка</translation>
    </message>
    <message>
        <source>Set Password [Setting...</source>
        <translation>Пароль ICQ [Устанавливается...</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>время вышло</translation>
    </message>
    <message>
        <source>Verify your ICQ password here.</source>
        <translation>Введите здесь свой ICQ пароль для проверки.</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>неудачно</translation>
    </message>
    <message>
        <source>Passwords do not match, try again.</source>
        <translation>Пароли не совпадают, попробуйте еще раз.</translation>
    </message>
    <message>
        <source>Invalid password, must be 8 characters or less.</source>
        <translation>Пароль должен быть не больше 8 символов.</translation>
    </message>
    <message>
        <source>Password</source>
        <translation>Пароль</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Отменить</translation>
    </message>
    <message>
        <source>&amp;Password:</source>
        <translation>&amp;Пароль:</translation>
    </message>
    <message>
        <source>&amp;Update</source>
        <translation>&amp;Обновить</translation>
    </message>
    <message>
        <source>&amp;Verify:</source>
        <translation>&amp;Еще раз:</translation>
    </message>
    <message>
        <source>Set Password</source>
        <translation>Установка ICQ пароля</translation>
    </message>
    <message>
        <source>ICQ Password [Setting...]</source>
        <translation>Пароль ICQ [Устанавливается...]</translation>
    </message>
    <message>
        <source>Enter your ICQ password here.</source>
        <translation>Введите здесь ваш ICQ пароль.</translation>
    </message>
    <message>
        <source>Licq - Set Password</source>
        <translation>Licq - Установка пароля</translation>
    </message>
</context>
<context>
    <name>PluginDlg</name>
    <message>
        <source>Id</source>
        <translation>N</translation>
    </message>
    <message>
        <source>Done</source>
        <translation>Завершить</translation>
    </message>
    <message>
        <source>Load</source>
        <translation>Загрузить</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <source>Plugin %1 has no configuration file</source>
        <translation>Модуль %1 не использует файл настройки</translation>
    </message>
    <message>
        <source>Enable</source>
        <translation>Разрешить</translation>
    </message>
    <message>
        <source>Configure</source>
        <translation>Настройка</translation>
    </message>
    <message>
        <source>Loaded</source>
        <translation>Загруженные</translation>
    </message>
    <message>
        <source>Status</source>
        <translation>Статус</translation>
    </message>
    <message>
        <source>Unload</source>
        <translation>Выгрузить</translation>
    </message>
    <message>
        <source>Licq Plugin %1 %2
</source>
        <translation>Licq модуль %1 %2
</translation>
    </message>
    <message>
        <source>Available</source>
        <translation>Доступные</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Обновить</translation>
    </message>
    <message>
        <source>Description</source>
        <translation>Описание</translation>
    </message>
    <message>
        <source>Details</source>
        <translation>Подробности</translation>
    </message>
    <message>
        <source>Disable</source>
        <translation>Запретить</translation>
    </message>
    <message>
        <source>Version</source>
        <translation>Версия</translation>
    </message>
</context>
<context>
    <name>QFileDialog</name>
    <message>
        <source>OK</source>
        <translation>Да</translation>
    </message>
    <message>
        <source>&amp;No</source>
        <translation>&amp;Нет</translation>
    </message>
    <message>
        <source>Dir</source>
        <translation>Каталог</translation>
    </message>
    <message>
        <source>&amp;Yes</source>
        <translation>&amp;Да</translation>
    </message>
    <message>
        <source>Back</source>
        <translation>Назад</translation>
    </message>
    <message>
        <source>Date</source>
        <translation>Дата</translation>
    </message>
    <message>
        <source>File</source>
        <translation>Файл</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <source>Open</source>
        <translation>Открыть</translation>
    </message>
    <message>
        <source>Save</source>
        <translation>Сохранить</translation>
    </message>
    <message>
        <source>Size</source>
        <translation>Размер</translation>
    </message>
    <message>
        <source>Sort</source>
        <translation>Сортировка</translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Тип</translation>
    </message>
    <message>
        <source>&amp;Open</source>
        <translation>&amp;Открыть</translation>
    </message>
    <message>
        <source>&amp;Save</source>
        <translation>&amp;Сохранить</translation>
    </message>
    <message>
        <source>ERROR</source>
        <translation>ОШИБКА</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
    <message>
        <source>Sort by &amp;Size</source>
        <translation>&amp;Сортировать по размеру</translation>
    </message>
    <message>
        <source>Sort by &amp;Date</source>
        <translation>&amp;Сортировать по дате</translation>
    </message>
    <message>
        <source>Sort by &amp;Name</source>
        <translation>&amp;Сортировать по имени</translation>
    </message>
    <message>
        <source>New Folder 1</source>
        <translation>Новый каталог 1</translation>
    </message>
    <message>
        <source>&lt;qt&gt;Do you really want to delete %1 &quot;%2&quot;?&lt;/qt&gt;</source>
        <translation>&lt;qt&gt;Вы действительно хотите удалить %1 &quot;%2&quot;?&lt;/qt&gt;</translation>
    </message>
    <message>
        <source>the directory</source>
        <translation>каталог</translation>
    </message>
    <message>
        <source>File &amp;type:</source>
        <translation>&amp;Тип файла:</translation>
    </message>
    <message>
        <source>File &amp;name:</source>
        <translation>&amp;Имя файла:</translation>
    </message>
    <message>
        <source>Delete %1</source>
        <translation>Удалить %1</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <source>R&amp;eload</source>
        <translation>&amp;Перегрузить</translation>
    </message>
    <message>
        <source>New Folder</source>
        <translation>Новый каталог</translation>
    </message>
    <message>
        <source>&amp;Unsorted</source>
        <translation>&amp;Без сортировки</translation>
    </message>
    <message>
        <source>Look &amp;in:</source>
        <translation>&amp;Смотреть в:</translation>
    </message>
    <message>
        <source>Preview File Contents</source>
        <translation>Предварительный просмотр содержания</translation>
    </message>
    <message>
        <source>New Folder %1</source>
        <translation>Новый каталог %1</translation>
    </message>
    <message>
        <source>Read-write</source>
        <translation>Чтение-запись</translation>
    </message>
    <message>
        <source>Read-only</source>
        <translation>Только для чтения</translation>
    </message>
    <message>
        <source>Find Directory</source>
        <translation>Найти каталог</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation>Атрибуты</translation>
    </message>
    <message>
        <source>Show &amp;hidden files</source>
        <translation>&amp;Показывать скрытые файлы</translation>
    </message>
    <message>
        <source>Save as</source>
        <translation>Сохранить как</translation>
    </message>
    <message>
        <source>Inaccessible</source>
        <translation>Недоступен</translation>
    </message>
    <message>
        <source>%1
File not found.
Check path and filename.</source>
        <translation>%1
Файл не найден.
Проверьте путь и имя файла.</translation>
    </message>
    <message>
        <source>List View</source>
        <translation>Список</translation>
    </message>
    <message>
        <source>Special</source>
        <translation>Специальный файл</translation>
    </message>
    <message>
        <source>Write-only</source>
        <translation>Только для записи</translation>
    </message>
    <message>
        <source>the symlink</source>
        <translation>ссылку</translation>
    </message>
    <message>
        <source>&amp;Delete</source>
        <translation>&amp;Удалить</translation>
    </message>
    <message>
        <source>All files (*)</source>
        <translation>Все файлы (*)</translation>
    </message>
    <message>
        <source>Directories</source>
        <translation>Каталоги</translation>
    </message>
    <message>
        <source>Symlink to Special</source>
        <translation>Ссылка на специальный файл</translation>
    </message>
    <message>
        <source>&amp;Rename</source>
        <translation>&amp;Переименовать</translation>
    </message>
    <message>
        <source>One directory up</source>
        <translation>На один каталог вверх</translation>
    </message>
    <message>
        <source>Preview File Info</source>
        <translation>Предварительный просмотр информации</translation>
    </message>
    <message>
        <source>the file</source>
        <translation>файл</translation>
    </message>
    <message>
        <source>Create New Folder</source>
        <translation>Создать новый каталог</translation>
    </message>
    <message>
        <source>Symlink to File</source>
        <translation>Ссылка на файл</translation>
    </message>
    <message>
        <source>Symlink to Directory</source>
        <translation>Ссылка на каталог</translation>
    </message>
    <message>
        <source>Detail View</source>
        <translation>Подробно</translation>
    </message>
</context>
<context>
    <name>QFontDialog</name>
    <message>
        <source>OK</source>
        <translation>Да</translation>
    </message>
    <message>
        <source>&amp;Font</source>
        <translation>&amp;Шрифт</translation>
    </message>
    <message>
        <source>&amp;Size</source>
        <translation>&amp;Размер</translation>
    </message>
    <message>
        <source>Apply</source>
        <translation>Применить</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <source>&amp;Color</source>
        <translation>&amp;Цвет</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <source>Sample</source>
        <translation>Пример</translation>
    </message>
    <message>
        <source>Font st&amp;yle</source>
        <translation>&amp;Стиль шрифта</translation>
    </message>
    <message>
        <source>Scr&amp;ipt</source>
        <translation>&amp;Набор символов</translation>
    </message>
    <message>
        <source>Select Font</source>
        <translation>Выбор шрифта</translation>
    </message>
    <message>
        <source>&amp;Underline</source>
        <translation>&amp;Подчеркнутый</translation>
    </message>
    <message>
        <source>Effects</source>
        <translation>Эффекты</translation>
    </message>
    <message>
        <source>Stri&amp;keout</source>
        <translation>&amp;Перечеркнутый</translation>
    </message>
</context>
<context>
    <name>QMessageBox</name>
    <message>
        <source>Licq Error</source>
        <translation>Ошибка Licq</translation>
    </message>
    <message>
        <source>Licq Warning</source>
        <translation>Предупреждение Licq</translation>
    </message>
    <message>
        <source>Licq Question</source>
        <translation>Вопрос Licq</translation>
    </message>
    <message>
        <source>Licq Information</source>
        <translation>Информация Licq</translation>
    </message>
</context>
<context>
    <name>QString</name>
    <message>
        <source>Failed to open file:
%1</source>
        <translation>Ошибка открытия файла:
%1</translation>
    </message>
</context>
<context>
    <name>QWizard</name>
    <message>
        <source>Help</source>
        <translation>Помощь</translation>
    </message>
    <message>
        <source>&lt; Back</source>
        <translation>&lt; Назад</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <source>Finish</source>
        <translation>Завершить</translation>
    </message>
    <message>
        <source>Next &gt;</source>
        <translation>Дальше &gt;</translation>
    </message>
</context>
<context>
    <name>RegisterUserDlg</name>
    <message>
        <source>Uin:</source>
        <translation>UIN:</translation>
    </message>
    <message>
        <source>Please enter your password in both input fields.</source>
        <translation>Пожалуйста введите пароль в оба поля ввода.</translation>
    </message>
    <message>
        <source>Registered succesfully.  Now log on and update your personal info.</source>
        <translation>Регистрация прошла успешно. Теперь войдите в сеть и обновите информацию о себе.</translation>
    </message>
    <message>
        <source>You need to enter a valid UIN when you try to register an existing user. </source>
        <translation>Вы должны ввести правильный UIN, если вы хотите зарегистрировать существующего пользователя</translation>
    </message>
    <message>
        <source>Now please click &apos;Finish&apos; to start the registration process.</source>
        <translation>Теперь нажмите  &apos;Закончить&apos; для регистрации на сервере.</translation>
    </message>
    <message>
        <source>User Registration in Progress...</source>
        <translation>Производится регистрация пользователя...</translation>
    </message>
    <message>
        <source>Now please press the &apos;Back&apos; button and try again.</source>
        <translation>Теперь нажмите  &apos;Назад&apos; и попробуйте еще раз.</translation>
    </message>
    <message>
        <source>&amp;Register Existing User</source>
        <translation>&amp;Зарегистрировать существующего пользователя</translation>
    </message>
    <message>
        <source>The passwords don&apos;t seem to match.</source>
        <translation>Пароли не совпадают.</translation>
    </message>
    <message>
        <source>UIN Registration - Step 2</source>
        <translation>Регистрация UIN - Шаг 2</translation>
    </message>
    <message>
        <source>UIN Registration - Step 3</source>
        <translation>Регистрация UIN - Шаг 3</translation>
    </message>
    <message>
        <source>Password:</source>
        <translation>Пароль:</translation>
    </message>
    <message>
        <source>Invalid UIN.  Try again.</source>
        <translation>Неверный UIN. Попробуйте еще раз.</translation>
    </message>
    <message>
        <source>Verify:</source>
        <translation>Еще раз:</translation>
    </message>
    <message>
        <source>Licq User Registration</source>
        <translation>Регистрация пользователя Licq</translation>
    </message>
    <message>
        <source>Welcome to the Registration Wizard.

You can register a new user here, or configure Licq to use an existing UIN.

If you are registering a new uin, choose a password and click &quot;Finish&quot;.
If you already have a uin, then toggle &quot;Register Existing User&quot;,
enter your uin and your password, and click &quot;OK&quot;

Press &quot;Next&quot; to proceed.</source>
        <translation>Добро пожаловать в Мастер Регистрации.

Вы можете зарегистрировать нового пользователя или
сконфигурировать Licq для использования уже существующего UIN.

Если вы регистрируете нового пользователя, то выберите
пароль и нажмите &quot;Завершить&quot;.
Если у вас уже есть UIN, то установите &quot;Зарегистрировать существующего пользователя&quot;,
введите ваш UIN и пароль и нажмите &quot;Да&quot;

Нажмите &quot;Дальше&quot; для продолжения.</translation>
    </message>
    <message>
        <source>UIN Registration</source>
        <translation>Регистрация UIN</translation>
    </message>
</context>
<context>
    <name>ReqAuthDlg</name>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;ОК</translation>
    </message>
    <message>
        <source>Request authorization from (UIN):</source>
        <translation>Запросить авторизацию от (UIN):</translation>
    </message>
    <message>
        <source>Licq - Request Authorization</source>
        <translation>Licq - Запрос авторизации</translation>
    </message>
    <message>
        <source>Request</source>
        <translation>Запрос</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Отменить</translation>
    </message>
</context>
<context>
    <name>SearchUserDlg</name>
    <message>
        <source>69+</source>
        <translation>69+</translation>
    </message>
    <message>
        <source>Male</source>
        <translation>Мужской</translation>
    </message>
    <message>
        <source>Reset Search</source>
        <translation>Очистить все</translation>
    </message>
    <message>
        <source>&amp;Done</source>
        <translation>&amp;Завершить</translation>
    </message>
    <message>
        <source>&amp;Uin#</source>
        <translation>&amp;Uin#</translation>
    </message>
    <message>
        <source>City:</source>
        <translation>Город:</translation>
    </message>
    <message>
        <source>UIN#:</source>
        <translation>UIN#:</translation>
    </message>
    <message>
        <source>Company Name:</source>
        <translation>Название компании:</translation>
    </message>
    <message>
        <source>Enter search parameters and select &apos;Search&apos;</source>
        <translation>Введите параметры поиска и нажмите кнопку &apos;Поиск&apos;</translation>
    </message>
    <message>
        <source>Company Position:</source>
        <translation>Должность:</translation>
    </message>
    <message>
        <source>Searching (this can take awhile)...</source>
        <translation>Идет поиск (это может занять некоторое время)...</translation>
    </message>
    <message>
        <source>%1 more users found. Narrow search.</source>
        <translation>Еще найдено %1 пользователей. Уточните поиск.</translation>
    </message>
    <message>
        <source>More users found. Narrow search.</source>
        <translation>Слишком много найдено. Уточните критерии поиска.</translation>
    </message>
    <message>
        <source>Alias:</source>
        <translation>Ник:</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <source>18 - 22</source>
        <translation>18 - 22</translation>
    </message>
    <message>
        <source>Female</source>
        <translation>Женский</translation>
    </message>
    <message>
        <source>Language:</source>
        <translation>Язык:</translation>
    </message>
    <message>
        <source>23 - 29</source>
        <translation>23 - 29</translation>
    </message>
    <message>
        <source>Age Range:</source>
        <translation>Возраст:</translation>
    </message>
    <message>
        <source>State:</source>
        <translation>Штат:</translation>
    </message>
    <message>
        <source>First Name:</source>
        <translation>Имя:</translation>
    </message>
    <message>
        <source>30 - 39</source>
        <translation>30 - 39</translation>
    </message>
    <message>
        <source>Last Name:</source>
        <translation>Фамилия:</translation>
    </message>
    <message>
        <source>Country:</source>
        <translation>Страна:</translation>
    </message>
    <message>
        <source>40 - 49</source>
        <translation>40 - 49</translation>
    </message>
    <message>
        <source>Search failed.</source>
        <translation>Поиск не удался.</translation>
    </message>
    <message>
        <source>50 - 59</source>
        <translation>50 - 59</translation>
    </message>
    <message>
        <source>Company Department:</source>
        <translation>Подразделение компании:</translation>
    </message>
    <message>
        <source>Return Online Users Only</source>
        <translation>Искать только среди тех, кто сейчас в сети</translation>
    </message>
    <message>
        <source>A&amp;lert User</source>
        <translation>&amp;Оповестить пользователя</translation>
    </message>
    <message>
        <source>&amp;Add %1 Users</source>
        <translation>&amp;Добавить (%1)</translation>
    </message>
    <message>
        <source>Email Address:</source>
        <translation>EMail адрес:</translation>
    </message>
    <message>
        <source>&amp;Add User</source>
        <translation>&amp;Добавить</translation>
    </message>
    <message>
        <source>&amp;Whitepages</source>
        <translation>&amp;Белые страницы</translation>
    </message>
    <message>
        <source>&amp;Search</source>
        <translation>&amp;Поиск</translation>
    </message>
    <message>
        <source>Unspecified</source>
        <translation>Не указано</translation>
    </message>
    <message>
        <source>Keyword:</source>
        <translation>Ключевое слово:</translation>
    </message>
    <message>
        <source>Gender:</source>
        <translation>Пол:</translation>
    </message>
    <message>
        <source>Licq - User Search</source>
        <translation>Licq - Поиск пользователей</translation>
    </message>
</context>
<context>
    <name>SearchUserView</name>
    <message>
        <source>?</source>
        <translation>?</translation>
    </message>
    <message>
        <source>F</source>
        <translation>М</translation>
    </message>
    <message>
        <source>M</source>
        <translation>Ж</translation>
    </message>
    <message>
        <source>UIN</source>
        <translation>UIN</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <source>Alias</source>
        <translation>Ник</translation>
    </message>
    <message>
        <source>Email</source>
        <translation>Email</translation>
    </message>
    <message>
        <source>Online</source>
        <translation>В сети</translation>
    </message>
    <message>
        <source>Status</source>
        <translation>Статус</translation>
    </message>
    <message>
        <source>Offline</source>
        <translation>Отключен</translation>
    </message>
    <message>
        <source>Sex &amp; Age</source>
        <translation>Пол и возраст</translation>
    </message>
    <message>
        <source>Authorize</source>
        <translation>Авторизация</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation>Неизвестно</translation>
    </message>
</context>
<context>
    <name>SecurityDlg</name>
    <message>
        <source>&amp;Uin:</source>
        <translation>&amp;UIN:</translation>
    </message>
    <message>
        <source>Timeout while setting security options.</source>
        <translation>Таймаут при сохранении настроек безопасности.</translation>
    </message>
    <message>
        <source>error</source>
        <translation>ошибка</translation>
    </message>
    <message>
        <source>ICQ Security Options [Setting...]</source>
        <translation>Настройки безопасности ICQ [Устанавливаются...]</translation>
    </message>
    <message>
        <source>Internal error while setting security options.</source>
        <translation>Произошла внутренняя ошибка при сохранении настроек безопасности.</translation>
    </message>
    <message>
        <source>&amp;Local changes only</source>
        <translation>&amp;Сохранять только локально</translation>
    </message>
    <message>
        <source>Password/UIN settings</source>
        <translation>Установки пароля/UIN</translation>
    </message>
    <message>
        <source>ICQ Security Options</source>
        <translation>Настройки безопасности ICQ</translation>
    </message>
    <message>
        <source>You need to be connected to the
ICQ Network to change the settings.</source>
        <translation>Вы должны сначала войти в ICQ-сеть,
чтобы изменить установки.</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>время вышло</translation>
    </message>
    <message>
        <source>Verify your ICQ password here.</source>
        <translation>Введите еще раз Ваш пароль ICQ.</translation>
    </message>
    <message>
        <source>If checked, password/UIN changes will apply only on your local computer.  Useful if your password is incorrectly saved in Licq.</source>
        <translation>Если установленно, изменения пароля/UIN будут применятся только локально.  Полезно, если Ваш пароль неправильно сохранен в файле конфигурации Licq.</translation>
    </message>
    <message>
        <source>Invalid password, must be between 1 and 8 characters.</source>
        <translation>Неправильный пароль, должен быть от 1 до 8 символов.</translation>
    </message>
    <message>
        <source>Timeout while changing password.</source>
        <translation>Таймаут при смене пароля.</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>Настройки</translation>
    </message>
    <message>
        <source>Web Presence allows users to see if you are online through your web indicator.</source>
        <translation>Присутствие в web позволяет пользователям видеть, в сети вы или нет, через web индикатор.</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>неудачно</translation>
    </message>
    <message>
        <source>Passwords do not match, try again.</source>
        <translation>Пароли не совпадают, попробуйте еще раз.</translation>
    </message>
    <message>
        <source>Authorization Required</source>
        <translation>Требуется авторизация</translation>
    </message>
    <message>
        <source>Enter the UIN which you want to use.  Only available if &quot;Local changes only&quot; ischecked.</source>
        <translation>Введите UIN, который Вы хотите использовать.  Доступно только если опция &quot;Сохранять только локально&quot; установлена.</translation>
    </message>
    <message>
        <source>Internal error while changing password.</source>
        <translation>Произошла внутренняя ошибка при смене пароля.</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Отменить</translation>
    </message>
    <message>
        <source>&amp;Password:</source>
        <translation>&amp;Пароль:</translation>
    </message>
    <message>
        <source>&amp;Update</source>
        <translation>&amp;Обновить</translation>
    </message>
    <message>
        <source>Changing password failed.</source>
        <translation>Не удалось сменить пароль.</translation>
    </message>
    <message>
        <source>&amp;Verify</source>
        <translation>&amp;Проверка</translation>
    </message>
    <message>
        <source>Hide IP stops users from seeing your IP address. It doesn&apos;t guarantee it will be hidden though.</source>
        <translation>Эта опция позволяет скрыть ваш IP адрес. Но при этом она не гарантирует, что его невозможно будет узнать.</translation>
    </message>
    <message>
        <source>Setting security options failed.</source>
        <translation>Не удалось сохранить настройки безопасности.</translation>
    </message>
    <message>
        <source>Web Presence</source>
        <translation>Присутствие в web</translation>
    </message>
    <message>
        <source>ICQ Security Options [Setting...</source>
        <translation>Настройки безопасности ICQ [Устанавливаются...</translation>
    </message>
    <message>
        <source>Enter your ICQ password here.</source>
        <translation>Введите Ваш ICQ пароль.</translation>
    </message>
    <message>
        <source>Determines whether regular ICQ clients require your authorization to add you to their contact list.</source>
        <translation>Вынуждает ICQ-клиента запрашивать Ваше подтверждение на добавление Вас в чей-либо список контактов.</translation>
    </message>
    <message>
        <source>Hide IP</source>
        <translation>Скрыть IP</translation>
    </message>
</context>
<context>
    <name>ShowAwayMsgDlg</name>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;Да</translation>
    </message>
    <message>
        <source>error</source>
        <translation>ошибка</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>вышло время</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>неудачно</translation>
    </message>
    <message>
        <source>&amp;Show Again</source>
        <translation>&amp;Показывать снова</translation>
    </message>
    <message>
        <source>%1 Response for %2</source>
        <translation>&apos;%1&apos; ответ для &apos;%2&apos;</translation>
    </message>
</context>
<context>
    <name>SkinBrowserDlg</name>
    <message>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <source>&amp;Done</source>
        <translation>&amp;Завершить</translation>
    </message>
    <message>
        <source>&amp;Edit</source>
        <translation>&amp;Редактировать</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
    <message>
        <source>Icons</source>
        <translation>значки</translation>
    </message>
    <message>
        <source>Skins</source>
        <translation>Вид</translation>
    </message>
    <message>
        <source>&amp;Apply</source>
        <translation>&amp;Применить</translation>
    </message>
    <message>
        <source>A&amp;pply</source>
        <translation>&amp;Применить</translation>
    </message>
    <message>
        <source>Licq Skin Browser</source>
        <translation>Выбор внешнего вида Licq</translation>
    </message>
</context>
<context>
    <name>UserEventCommon</name>
    <message>
        <source>Time:</source>
        <translation>Время:</translation>
    </message>
    <message>
        <source>Change user text encoding</source>
        <translation>Изменить кодировку пользователя</translation>
    </message>
    <message>
        <source>This button selects the text encoding used when communicating with this user. You might need to change the encoding to communicate in a different language.</source>
        <translation>Эта кнопка позволяет выбрать кодировку текста, используемую для общения с этим пользователем. Вам может понадобиться сменить кодировку для общения на другом языке.</translation>
    </message>
    <message>
        <source>Show User Info</source>
        <translation>Показать информацию о пользователе</translation>
    </message>
    <message>
        <source>Unable to load encoding &lt;b&gt;%1&lt;/b&gt;. Message contents may appear garbled.</source>
        <translation>Невозможно загрузить кодировку &lt;b&gt;%1&lt;/b&gt;. Отображение сообщения может быть некорректным.</translation>
    </message>
    <message>
        <source>Status:</source>
        <translation>Статус:</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation>Неизвестно</translation>
    </message>
    <message>
        <source>Show User History</source>
        <translation>Показать историю</translation>
    </message>
    <message>
        <source>Open / Close secure channel</source>
        <translation>Открыть/закрыть безопасное соединение</translation>
    </message>
</context>
<context>
    <name>UserInfoDlg</name>
    <message>
        <source>IP:</source>
        <translation>IP:</translation>
    </message>
    <message>
        <source>Now</source>
        <translation>Сейчас</translation>
    </message>
    <message>
        <source>Age:</source>
        <translation>Возраст:</translation>
    </message>
    <message>
        <source>Fax:</source>
        <translation>Факс:</translation>
    </message>
    <message>
        <source>Male</source>
        <translation>Мужской</translation>
    </message>
    <message>
        <source>UIN:</source>
        <translation>UIN:</translation>
    </message>
    <message>
        <source>Zip:</source>
        <translation>Индекс:</translation>
    </message>
    <message>
        <source>done</source>
        <translation>готово</translation>
    </message>
    <message>
        <source> Day:</source>
        <translation> День:</translation>
    </message>
    <message>
        <source>&amp;Last</source>
        <translation>&amp;Последний раз</translation>
    </message>
    <message>
        <source>&amp;Menu</source>
        <translation>&amp;Меню</translation>
    </message>
    <message>
        <source>&amp;More</source>
        <translation>&amp;Подробнее</translation>
    </message>
    <message>
        <source>&amp;Save</source>
        <translation>&amp;Сохранить</translation>
    </message>
    <message>
        <source>&amp;Work</source>
        <translation>&amp;О Работе</translation>
    </message>
    <message>
        <source>&amp;Filter: </source>
        <translation>&amp;Фильтр: </translation>
    </message>
    <message>
        <source>City:</source>
        <translation>Город:</translation>
    </message>
    <message>
        <source>P&amp;rev</source>
        <translation>&amp;Назад</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation>Имя:</translation>
    </message>
    <message>
        <source>Nex&amp;t</source>
        <translation>&amp;Дальше</translation>
    </message>
    <message>
        <source>error</source>
        <translation>ошибка</translation>
    </message>
    <message>
        <source>Last Online:</source>
        <translation>В сети:</translation>
    </message>
    <message>
        <source>Updating server...</source>
        <translation>Обновляется на сервере...</translation>
    </message>
    <message>
        <source> Year:</source>
        <translation> Год:</translation>
    </message>
    <message>
        <source>Last Sent Event:</source>
        <translation>Что-либо отправлено:</translation>
    </message>
    <message>
        <source>Last Checked Auto Response:</source>
        <translation>Проверен автоответ:</translation>
    </message>
    <message>
        <source>&amp;About</source>
        <translation>&amp;О себе</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Закрыть</translation>
    </message>
    <message>
        <source>You need to be connected to the
ICQ Network to retrieve your settings.</source>
        <translation>Вы должны быть в ICQ сети для
изменения ваших настроек.</translation>
    </message>
    <message>
        <source>Cellular:</source>
        <translation>Сотовый:</translation>
    </message>
    <message>
        <source>%1 to %2</source>
        <translation>%1 к пользователю %2</translation>
    </message>
    <message>
        <source>EMail 1:</source>
        <translation>Основной EMail:</translation>
    </message>
    <message>
        <source>EMail 2:</source>
        <translation>Дополнительный EMail:</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>вышло время</translation>
    </message>
    <message>
        <source>Homepage:</source>
        <translation>Домашняя страница:</translation>
    </message>
    <message>
        <source>Online Since:</source>
        <translation>В сети с:</translation>
    </message>
    <message>
        <source>Timezone:</source>
        <translation>Временная зона:</translation>
    </message>
    <message>
        <source>About:</source>
        <translation>О себе:</translation>
    </message>
    <message>
        <source>Alias:</source>
        <translation>Ник:</translation>
    </message>
    <message>
        <source>Old Email:</source>
        <translation>Старый Email:</translation>
    </message>
    <message>
        <source>Female</source>
        <translation>Женский</translation>
    </message>
    <message>
        <source> Month:</source>
        <translation> Месяц:</translation>
    </message>
    <message>
        <source>Phone:</source>
        <translation>Телефон:</translation>
    </message>
    <message>
        <source>State:</source>
        <translation>Штат:</translation>
    </message>
    <message>
        <source>Offline</source>
        <translation>Оключен</translation>
    </message>
    <message>
        <source>Last Received Event:</source>
        <translation>Что-либо принято:</translation>
    </message>
    <message>
        <source>Authorization Not Required</source>
        <translation>Авторизация не требуется</translation>
    </message>
    <message>
        <source>[&lt;font color=&quot;%1&quot;&gt;Received&lt;/font&gt;] [&lt;font color=&quot;%2&quot;&gt;Sent&lt;/font&gt;] %3 out of %4 matches</source>
        <translation>[&lt;font color=&quot;%1&quot;&gt;Принятые&lt;/font&gt;] [&lt;font color=&quot;%2&quot;&gt;Отправленные&lt;/font&gt;] %3 из %4 подходящих</translation>
    </message>
    <message>
        <source>GMT%1%1%1</source>
        <translation>GMT%1%1%1</translation>
    </message>
    <message>
        <source>[&lt;font color=&quot;%1&quot;&gt;Received&lt;/font&gt;] [&lt;font color=&quot;%2&quot;&gt;Sent&lt;/font&gt;] %3 to %4 of %5</source>
        <translation>[&lt;font color=&quot;%1&quot;&gt;Принятые&lt;/font&gt;] [&lt;font color=&quot;%2&quot;&gt;Отправленные&lt;/font&gt;] с %3 по %4 из %5</translation>
    </message>
    <message>
        <source>Country:</source>
        <translation>Страна:</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>неудачно</translation>
    </message>
    <message>
        <source>server</source>
        <translation>через сервер</translation>
    </message>
    <message>
        <source>Authorization Required</source>
        <translation>Авторизация требуется</translation>
    </message>
    <message>
        <source>INVALID USER</source>
        <translation>НЕПРАВИЛЬНЫЙ ПОЛЬЗОВАТЕЛЬ</translation>
    </message>
    <message>
        <source>Birthday:</source>
        <translation>День рождения:</translation>
    </message>
    <message>
        <source>Unknown (%1)</source>
        <translation>Неизвестно (%1)</translation>
    </message>
    <message>
        <source>Department:</source>
        <translation>Отдел:</translation>
    </message>
    <message>
        <source>Position:</source>
        <translation>Должность:</translation>
    </message>
    <message>
        <source>Sorry, history is disabled for this person.</source>
        <translation>Извините, история запрещена для данного пользователя.</translation>
    </message>
    <message>
        <source>Licq - Info </source>
        <translation>Licq - Информация </translation>
    </message>
    <message>
        <source>Status:</source>
        <translation>Статус:</translation>
    </message>
    <message>
        <source>Address:</source>
        <translation>Адрес:</translation>
    </message>
    <message>
        <source>Updating...</source>
        <translation>Обновляется...</translation>
    </message>
    <message>
        <source>You need to be connected to the
ICQ Network to change your settings.</source>
        <translation>Вы должны быть в ICQ сети для
изменения ваших настроек.</translation>
    </message>
    <message>
        <source>Language 3:</source>
        <translation>Третий язык:</translation>
    </message>
    <message>
        <source>Language 1:</source>
        <translation>Основной язык:</translation>
    </message>
    <message>
        <source>Language 2:</source>
        <translation>Второй язык:</translation>
    </message>
    <message>
        <source>Unspecified</source>
        <translation>Не указано</translation>
    </message>
    <message>
        <source>&amp;Update</source>
        <translation>&amp;Обновить</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation>Неизвестно</translation>
    </message>
    <message>
        <source>Retrieve</source>
        <translation>Извлечь</translation>
    </message>
    <message>
        <source>Rever&amp;se</source>
        <translation>&amp;В обратном порядке</translation>
    </message>
    <message>
        <source>%1 from %2</source>
        <translation>%1 от пользователя %2</translation>
    </message>
    <message>
        <source>Gender:</source>
        <translation>Пол:</translation>
    </message>
    <message>
        <source>&amp;General</source>
        <translation>&amp;Общая</translation>
    </message>
    <message>
        <source>&amp;History</source>
        <translation>&amp;История</translation>
    </message>
    <message>
        <source>Error loading history file: %1
Description: %2</source>
        <translation>Ошибка загрузки файла истории: %1
Описание: %2</translation>
    </message>
</context>
<context>
    <name>UserSendChatEvent</name>
    <message>
        <source>Clear</source>
        <translation>Очистить</translation>
    </message>
    <message>
        <source>Multiparty: </source>
        <translation>Многопользовательский:</translation>
    </message>
    <message>
        <source>Invite</source>
        <translation>Пригласить</translation>
    </message>
    <message>
        <source>Chat with %2 refused:
%3</source>
        <translation>В разговоре с %2 отказано:
%3</translation>
    </message>
    <message>
        <source> - Chat Request</source>
        <translation> - Запрос разговора</translation>
    </message>
</context>
<context>
    <name>UserSendCommon</name>
    <message>
        <source>No</source>
        <translation>Нет</translation>
    </message>
    <message>
        <source>SMS</source>
        <translation>SMS</translation>
    </message>
    <message>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <source>Yes</source>
        <translation>Да</translation>
    </message>
    <message>
        <source>done</source>
        <translation>готово</translation>
    </message>
    <message>
        <source>&amp;Menu</source>
        <translation>&amp;Меню</translation>
    </message>
    <message>
        <source>&amp;Send</source>
        <translation>&amp;Отправить</translation>
    </message>
    <message>
        <source>error</source>
        <translation>ошибка</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Закрыть</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>вышло время</translation>
    </message>
    <message>
        <source>Message</source>
        <translation>Сообщение</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <source>File Transfer</source>
        <translation>Передача файла</translation>
    </message>
    <message>
        <source>%1 refused %2, send through server.</source>
        <translation>%1 отказал %2, посылаю через сервер.</translation>
    </message>
    <message>
        <source>Urgent</source>
        <translation>Срочно</translation>
    </message>
    <message>
        <source>Send anyway</source>
        <translation>Отправить любым способом</translation>
    </message>
    <message>
        <source>Drag Users Here
Right Click for Options</source>
        <translation>Перенесите пользователей сюда
Используйте правую кнопку для изменения списка</translation>
    </message>
    <message>
        <source>direct</source>
        <translation>напрямую</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>неудачно</translation>
    </message>
    <message>
        <source>via server</source>
        <translation>через сервер</translation>
    </message>
    <message>
        <source>U&amp;rgent</source>
        <translation>&amp;Срочно</translation>
    </message>
    <message>
        <source>%1 is in %2 mode:
%3
Send...</source>
        <translation>%1 в режиме %2:
%3
Отправляется...</translation>
    </message>
    <message>
        <source>Direct send failed,
send through server?</source>
        <translation>Отправить напрямую не удалось,
отправить через сервер?</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Отменить</translation>
    </message>
    <message>
        <source>Warning: Message can&apos;t be sent securely
through the server!</source>
        <translation>Предупреждение: сообщение не может быть отослано
через безопасное соединение через сервер!</translation>
    </message>
    <message>
        <source>Sending </source>
        <translation>Отправляется </translation>
    </message>
    <message>
        <source>Chat Request</source>
        <translation>Запрос разговора</translation>
    </message>
    <message>
        <source>Contact List</source>
        <translation>Список контактов</translation>
    </message>
    <message>
        <source> to Contact List</source>
        <translation> в список контактов</translation>
    </message>
    <message>
        <source>M&amp;ultiple recipients</source>
        <translation>&amp;Многим адресатам</translation>
    </message>
    <message>
        <source>Se&amp;nd through server</source>
        <translation>&amp;Отправить через сервер</translation>
    </message>
</context>
<context>
    <name>UserSendContactEvent</name>
    <message>
        <source>Drag Users Here - Right Click for Options</source>
        <translation>Перенесите пользователей сюда - Используйте правую кнопку для изменения списка</translation>
    </message>
    <message>
        <source> - Contact List</source>
        <translation> - Список контактов</translation>
    </message>
</context>
<context>
    <name>UserSendFileEvent</name>
    <message>
        <source>You must specify a file to transfer!</source>
        <translation>Вы должны указать файл для передачи!</translation>
    </message>
    <message>
        <source>File(s): </source>
        <translation>Файл(ы): </translation>
    </message>
    <message>
        <source>Browse</source>
        <translation>Просмотр</translation>
    </message>
    <message>
        <source> - File Transfer</source>
        <translation> - Передача файла</translation>
    </message>
    <message>
        <source>File transfer with %2 refused:
%3</source>
        <translation>В передаче файла к %2 отказано::
%3</translation>
    </message>
    <message>
        <source>Select files to send</source>
        <translation>Выберите файлы для отправки</translation>
    </message>
</context>
<context>
    <name>UserSendMsgEvent</name>
    <message>
        <source>&amp;No</source>
        <translation>&amp;Нет</translation>
    </message>
    <message>
        <source>&amp;Yes</source>
        <translation>&amp;Да</translation>
    </message>
    <message>
        <source> - Message</source>
        <translation> - Сообщение</translation>
    </message>
    <message>
        <source>You didn&apos;t edit the message.
Do you really want to send it?</source>
        <translation>Вы не редактировали сообщение.
Вы уверены, что хотите его отправить?</translation>
    </message>
</context>
<context>
    <name>UserSendSmsEvent</name>
    <message>
        <source>&amp;No</source>
        <translation>&amp;Нет</translation>
    </message>
    <message>
        <source>&amp;Yes</source>
        <translation>&amp;Да</translation>
    </message>
    <message>
        <source> - SMS</source>
        <translation> - SMS</translation>
    </message>
    <message>
        <source>You didn&apos;t edit the SMS.
Do you really want to send it?</source>
        <translation>Вы не отредактировали SMS сообщение.
Вы действительно хотите его отослать?</translation>
    </message>
    <message>
        <source>Chars left : </source>
        <translation>Осталось символов : </translation>
    </message>
    <message>
        <source>Phone : </source>
        <translation>Телефон : </translation>
    </message>
</context>
<context>
    <name>UserSendUrlEvent</name>
    <message>
        <source> - URL</source>
        <translation> - ссылка</translation>
    </message>
    <message>
        <source>URL : </source>
        <translation>Ссылка : </translation>
    </message>
</context>
<context>
    <name>UserViewEvent</name>
    <message>
        <source>Chat</source>
        <translation>Разговор</translation>
    </message>
    <message>
        <source>&amp;Join</source>
        <translation>&amp;Присоединиться</translation>
    </message>
    <message>
        <source>&amp;Menu</source>
        <translation>&amp;Меню</translation>
    </message>
    <message>
        <source>&amp;View</source>
        <translation>&amp;Просмотр</translation>
    </message>
    <message>
        <source>Nex&amp;t</source>
        <translation>&amp;Дальше</translation>
    </message>
    <message>
        <source>A&amp;dd %1 Users</source>
        <translation>&amp;Добавить %1 пользователей</translation>
    </message>
    <message>
        <source>Aut&amp;o Close</source>
        <translation>&amp;Авто-закрытие</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Закрыть</translation>
    </message>
    <message>
        <source>&amp;Quote</source>
        <translation>&amp;Цитировать</translation>
    </message>
    <message>
        <source>&amp;Reply</source>
        <translation>&amp;Ответить</translation>
    </message>
    <message>
        <source>A&amp;ccept</source>
        <translation>&amp;Подтвердить</translation>
    </message>
    <message>
        <source>Normal Click - Close Window
&lt;CTRL&gt;+Click - also delete User</source>
        <translation>Нормальное нажатие - закрывает окно
&lt;CTRL&gt;+Click - также удаляет пользователя</translation>
    </message>
    <message>
        <source>File Transfer</source>
        <translation>Передача файла</translation>
    </message>
    <message>
        <source>Start Chat</source>
        <translation>Начать разговор</translation>
    </message>
    <message>
        <source>
--------------------
Request was cancelled.</source>
        <translation> 
--------------------
Запрос был отменен.</translation>
    </message>
    <message>
        <source>View URL failed</source>
        <translation>Просмотр URL не удался</translation>
    </message>
    <message>
        <source>A&amp;dd User</source>
        <translation>&amp;Добавить</translation>
    </message>
    <message>
        <source>A&amp;uthorize</source>
        <translation>&amp;Авторизовать</translation>
    </message>
    <message>
        <source>&amp;Refuse</source>
        <translation>&amp;Отказать</translation>
    </message>
    <message>
        <source>Nex&amp;t (%1)</source>
        <translation>&amp;Дальше (%1)</translation>
    </message>
    <message>
        <source>&amp;Forward</source>
        <translation>&amp;Переслать</translation>
    </message>
</context>
<context>
    <name>WharfIcon</name>
    <message>
        <source>Left click - Show main window
Middle click - Show next message
Right click - System menu</source>
        <translation>Левая кнопка - Показать главное окно
Средняя кнопка - Показать следующее сообщение
Правая кнопка - Системное меню</translation>
    </message>
</context>
</TS>
