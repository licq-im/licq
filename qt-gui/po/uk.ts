<!DOCTYPE TS><TS>
<context>
    <name>AddUserDlg</name>
    <message>
        <source>Licq - Add User</source>
        <translation>Licq - Додання користувача</translation>
    </message>
    <message>
        <source>New User UIN:</source>
        <translation>Ід. номер нового користувача</translation>
    </message>
    <message>
        <source>&amp;Alert User</source>
        <translation>&amp;Попередити користувача</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасувати</translation>
    </message>
</context>
<context>
    <name>AuthUserDlg</name>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;Гаразд</translation>
    </message>
    <message>
        <source>Refuse authorization to %1</source>
        <translation>Відмовити в авторизації %1</translation>
    </message>
    <message>
        <source>Authorize which user (UIN):</source>
        <translation>Авторизувати користувача з номером:</translation>
    </message>
    <message>
        <source>Grant authorization to %1</source>
        <translation>Авторизувати %1</translation>
    </message>
    <message>
        <source>Licq - Grant Authorisation</source>
        <translation>Licq - Авторизація</translation>
    </message>
    <message>
        <source>Licq - Refuse Authorisation</source>
        <translation>Licq - Відмова в авторизації</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасувати</translation>
    </message>
    <message>
        <source>Response</source>
        <translation>Відповідь</translation>
    </message>
</context>
<context>
    <name>AwayMsgDlg</name>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;Гаразд</translation>
    </message>
    <message>
        <source>I&apos;m currently %1, %a.
You can leave me a message.
(%m messages pending from you).</source>
        <translation>Я зараз в стані &quot;%1&quot;, %a.
Ви можете залиши мені повідомлення.
(%m не прочитаних повідомлень від Вас).</translation>
    </message>
    <message>
        <source>&amp;Hints</source>
        <translation>&amp;Поради</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Hints for Setting&lt;br&gt;your Auto-Response&lt;/h2&gt;&lt;br&gt;&lt;hr&gt;&lt;br&gt;&lt;ul&gt;&lt;li&gt;You can include any of the % expansions (described in the main hints page).&lt;/li&gt;&lt;li&gt;Any line beginning with a pipe (|) will be treated as a command to be run.  The line will be replaced by the output of the command. The command is parsed by /bin/sh so any shell commands or meta-characters are allowed.  For security reasons, any % expansions are automatically passed to the command surrounded by single quotes to prevent shell parsing of any meta-characters included in an alias.&lt;br&gt;Examples of popular uses include:&lt;ul&gt;&lt;li&gt;&lt;tt&gt;|date&lt;/tt&gt;: Will replace that line by the current date&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|fortune&lt;/tt&gt;: Show a fortune, as a tagline for example&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|myscript.sh %u %a&lt;/tt&gt;: Run a script, passing the uin and alias&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|myscript.sh %u %a &gt; /dev/null&lt;/tt&gt;: Run the same script but ignore the output (for tracking auto response checks or something)&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|if [ %u -lt 100000 ]; then echo &quot;You are special&quot;; fi&lt;/tt&gt;: Useless, but shows how you can use shell script.&lt;/li&gt;&lt;/ul&gt;Of course, multiple &quot;|&quot; can appear in the auto response, and commands and regular text can be mixed line by line.&lt;/li&gt;&lt;hr&gt;&lt;p&gt; For more information, see the Licq webpage (&lt;tt&gt;http://www.licq.org&lt;/tt&gt;).&lt;/p&gt;</source>
        <translation>&lt;h2&gt;Поради, що до встановлення&lt;br&gt; автовідповідей&lt;/h2&gt;&lt;br&gt;&lt;hr&gt;&lt;br&gt;&lt;ul&gt;&lt;li&gt;Ви можете користуватись %-комбінаціями в автовідповіді (ці комбінації описані на головній сторінці порад).&lt;/li&gt;&lt;li&gt;Будь який рядок, що починається з (|), буде оброблено як команду. Замість рядка буде вставлено вивід команди. Команда виконується за допомогою /bin/sh, тому можна використовувати будь-які команди або зарезервовані символи оболонки. З причин безпеки, будь-які вирази, що буди отримані за допомогою %-комбінацій, автоматично обгортаються одинарними лапками, щоб уникнути аналіз будь-яких спеціальних символів оболонкою, які можуть бути присутні у іменах.&lt;br&gt;Серед найбільш часто використовуються наступні команди:&lt;ul&gt;&lt;li&gt;&lt;tt&gt;|date&lt;/tt&gt;: рядок буде замінено поточною датою&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|fortune&lt;/tt&gt;: буде відображено пораду від fortune, як приклад власної команди&lt;/li&gt; &lt;li&gt;&lt;tt&gt;|myscript.sh %u %a&lt;/tt&gt;: виконає myscript.sh, передав ідентифікаційний номер та прізвисько у параметрах&lt;/li&gt; &lt;li&gt;&lt;tt&gt;|myscript.sh %u %a &gt; /dev/null&lt;/tt&gt;: виконає ту саму програму, але її вивід буде проігнорований (це може використовуватись для стеження за автовідповідями або чогось іншого)&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|if [ %u -lt 100000 ]; then echo &quot;Так, ти особливий&quot;; fi&lt;/tt&gt;: Малокорисний приклад, але він показує, як Ви можете користуватись командами оболонки.&lt;/li&gt; &lt;/ul&gt;В автовідповіді може бути декілька &quot;|&quot;, також команди та звичайний текст можна змішувати по рядкам.&lt;/li&gt;&lt;hr&gt;&lt;p&gt; Додаткову інформацію Ви можете отримати на домашній сторінці Licq (&lt;tt&gt;http://www.licq.org&lt;/tt&gt;).&lt;/p&gt;</translation>
    </message>
    <message>
        <source>&amp;Edit Items</source>
        <translation>&amp;Редагування</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасувати</translation>
    </message>
    <message>
        <source>&amp;Select</source>
        <translation>&amp;Вибрати</translation>
    </message>
    <message>
        <source>Set %1 Response for %2</source>
        <translation>Встановити відповідь %1 для %2</translation>
    </message>
</context>
<context>
    <name>CFileDlg</name>
    <message>
        <source>KB</source>
        <translation>кБ</translation>
    </message>
    <message>
        <source>MB</source>
        <translation>МБ</translation>
    </message>
    <message>
        <source>Ok</source>
        <translation type="obsolete">Гаразд</translation>
    </message>
    <message>
        <source>Byte</source>
        <translation>Байт</translation>
    </message>
    <message>
        <source>ETA:</source>
        <translation>ОВЗ:</translation>
    </message>
    <message>
        <source>%1/%2</source>
        <translation>%1/%2</translation>
    </message>
    <message>
        <source>Bytes</source>
        <translation>Байт</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Закрити</translation>
    </message>
    <message>
        <source>File:</source>
        <translation>Файл:</translation>
    </message>
    <message>
        <source>Retry</source>
        <translation>Повторити</translation>
    </message>
    <message>
        <source>Time:</source>
        <translation>Час:</translation>
    </message>
    <message>
        <source>Remote side disconnected</source>
        <translation type="obsolete">Віддалена сторона від&apos;єдналася</translation>
    </message>
    <message>
        <source>Licq - File Transfer (%1)</source>
        <translation>Licq - Передача файлу(ів) (%1)</translation>
    </message>
    <message>
        <source>Remote side disconnected
</source>
        <translation type="obsolete">Віддалена сторона від&apos;єдналася
</translation>
    </message>
    <message>
        <source>Sent
%1
to %2 successfully
</source>
        <translation type="obsolete">%1
відіслано к %2 успішно
</translation>
    </message>
    <message>
        <source>Connecting to remote...
</source>
        <translation type="obsolete">Встановлення з&apos;єднання...
</translation>
    </message>
    <message>
        <source>Received
%1
from %2 successfully
</source>
        <translation type="obsolete">%1
прийнято від %2 успішно
</translation>
    </message>
    <message>
        <source>File already exists and appears incomplete.</source>
        <translation>Файл вже існує і схоже він неповний.</translation>
    </message>
    <message>
        <source>Batch:</source>
        <translation>Всього:</translation>
    </message>
    <message>
        <source>File already exists and is at least as big as the incoming file.</source>
        <translation>Файл вже існує і, він не менше файлу, що отримується.</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Скасувати</translation>
    </message>
    <message>
        <source>Waiting for connection...
</source>
        <translation type="obsolete">Чекаю з&apos;єднання...
</translation>
    </message>
    <message>
        <source>Sending file...
</source>
        <translation type="obsolete">Відсилаю файл...
</translation>
    </message>
    <message>
        <source>Resume</source>
        <translation>Повернути</translation>
    </message>
    <message>
        <source>Done %1
</source>
        <translation type="obsolete">Завершено (%1)
</translation>
    </message>
    <message>
        <source>File name:</source>
        <translation>Ім&apos;я файлу:</translation>
    </message>
    <message>
        <source>Receiving file...
</source>
        <translation type="obsolete">Отримую файл...
</translation>
    </message>
    <message>
        <source>File I/O error: %1
</source>
        <translation type="obsolete">Помилка в/в файлу: %1
</translation>
    </message>
    <message>
        <source>Handshake Error
See Network Window for Details</source>
        <translation type="obsolete">Помилка протоколу
Подробиці в Вікні мережі</translation>
    </message>
    <message>
        <source>File transfer cancelled
</source>
        <translation type="obsolete">Передача файлу(ів) скасована
</translation>
    </message>
    <message>
        <source>File I/O Error:
%1
See Network Window for Details</source>
        <translation type="obsolete">Помилка в/в:
%1
Подробиці у Вікні мережі</translation>
    </message>
    <message>
        <source>Handshaking error
</source>
        <translation type="obsolete">Помилка протоколу
</translation>
    </message>
    <message>
        <source>Current:</source>
        <translation>Поточний:</translation>
    </message>
    <message>
        <source>Open error - unable to open file for writing.</source>
        <translation>Помилка відкриття файлу для запису.</translation>
    </message>
    <message>
        <source>Overwrite</source>
        <translation>Перезаписати</translation>
    </message>
    <message>
        <source>&amp;Cancel Transfer</source>
        <translation>&amp;Скасувати передачу</translation>
    </message>
    <message>
        <source>File transfer complete
</source>
        <translation type="obsolete">Передачу файлу(ів) завершено
</translation>
    </message>
    <message>
        <source>File transfer cancelled.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Waiting for connection...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Receiving file...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Sending file...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Received %1 from %2 successfully.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Sent %1 to %2 successfully.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>File transfer complete.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>OK</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Remote side disconnected.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>File I/O error: %1.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>File I/O Error:
%1

See Network Window for details.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Handshaking error.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Handshake Error.
See Network Window for details.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Connection error.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unable to reach remote host.
See Network Window for details.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Bind error.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unable to bind to a port.
See Network Window for details.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Not enough resources.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unable to create a thread.
See Network Window for details.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Connecting to remote...</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>CForwardDlg</name>
    <message>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <source>Message</source>
        <translation>Повідомлення</translation>
    </message>
    <message>
        <source>Forward %1 To User</source>
        <translation>Переслати %1 користувачу</translation>
    </message>
    <message>
        <source>Drag the user to forward to here:</source>
        <translation>Пересуньте користувача, якому
бажаєте переслати повідомлення сюди:</translation>
    </message>
    <message>
        <source>Unable to forward this message type (%d).</source>
        <translation>Неможливо переслати повідомлення цього типу (%d).</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасувати</translation>
    </message>
    <message>
        <source>Forwarded message:
</source>
        <translation>Перенаправлене повідомлення:
</translation>
    </message>
    <message>
        <source>&amp;Forward</source>
        <translation>&amp;Переслати</translation>
    </message>
    <message>
        <source>Forwarded URL:
</source>
        <translation>Адреса, що пересилаєте:
</translation>
    </message>
</context>
<context>
    <name>CJoinChatDlg</name>
    <message>
        <source>Select chat to join:</source>
        <translation>Виберіть розмову:</translation>
    </message>
    <message>
        <source>Invite to Join Chat</source>
        <translation>Запросити до розмови</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасувати</translation>
    </message>
    <message>
        <source>Select chat to invite:</source>
        <translation>Виберіть розмову:</translation>
    </message>
    <message>
        <source>Join Multiparty Chat</source>
        <translation>Приєднатися до багатосторонньої розмови</translation>
    </message>
</context>
<context>
    <name>CMMSendDlg</name>
    <message>
        <source>Sending mass message to %1...</source>
        <translation>Відсилання повідомлення для багатьох людей до %1...</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Закрити</translation>
    </message>
    <message>
        <source>Sending mass URL to %1...</source>
        <translation>Відсилаю URL до %1...</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>Помилка</translation>
    </message>
    <message>
        <source>Multiple Recipient Contact List</source>
        <translation>Список контактів з декількими адресатами</translation>
    </message>
    <message>
        <source>Multiple Recipient URL</source>
        <translation>URL декільким адресатам</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасувати</translation>
    </message>
    <message>
        <source>Sending mass list to %1...</source>
        <translation>Відсилаю список контактів до %1...</translation>
    </message>
    <message>
        <source>Multiple Recipient Message</source>
        <translation>Повідомлення декільким адресатам</translation>
    </message>
</context>
<context>
    <name>CMMUserView</name>
    <message>
        <source>Crop</source>
        <translation>Частина</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation>Очистити</translation>
    </message>
    <message>
        <source>Remove</source>
        <translation>Видалити</translation>
    </message>
    <message>
        <source>Add Group</source>
        <translation>Додати групу</translation>
    </message>
    <message>
        <source>Add All</source>
        <translation>Додати всіх</translation>
    </message>
</context>
<context>
    <name>CMainWindow</name>
    <message>
        <source>s</source>
        <translation>с</translation>
    </message>
    <message>
        <source>Ok</source>
        <translation>Гаразд</translation>
    </message>
    <message>
        <source>&amp;No</source>
        <translation>&amp;Ні</translation>
    </message>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;Гаразд</translation>
    </message>
    <message>
        <source>&amp;Yes</source>
        <translation>&amp;Так</translation>
    </message>
    <message>
        <source>Send</source>
        <translation>Відіслати</translation>
    </message>
    <message>
        <source>Online to User</source>
        <translation>Видимий у стані &quot;В мережі&quot;</translation>
    </message>
    <message>
        <source>&amp;Away</source>
        <translation>&amp;Відійшов</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>&amp;Довідка</translation>
    </message>
    <message>
        <source>&amp;Info</source>
        <translation>&amp;Інформація</translation>
    </message>
    <message>
        <source>U&amp;tilities</source>
        <translation>&amp;Утиліти</translation>
    </message>
    <message>
        <source>E&amp;xit</source>
        <translation>В&amp;ихід</translation>
    </message>
    <message>
        <source>&amp;Options...</source>
        <translation>&amp;Параметри...</translation>
    </message>
    <message>
        <source>R&amp;andom Chat</source>
        <translation>&amp;Випадкова розмова</translation>
    </message>
    <message>
        <source>Error sending authorization.</source>
        <translation>Помилка під час відправлення авторизації.</translation>
    </message>
    <message>
        <source>&amp;Random Chat Group</source>
        <translation>В&amp;ипадкова розмова</translation>
    </message>
    <message>
        <source>&lt;ul&gt;&lt;li&gt;&lt;tt&gt;%a - &lt;/tt&gt;user alias&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%e - &lt;/tt&gt;email&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%f - &lt;/tt&gt;first name&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%h - &lt;/tt&gt;phone number&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%i - &lt;/tt&gt;user ip&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%l - &lt;/tt&gt;last name&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%m - &lt;/tt&gt;# pending messages&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%n - &lt;/tt&gt;full name&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%o - &lt;/tt&gt;last seen online&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%O - &lt;/tt&gt;online since&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%p - &lt;/tt&gt;user port&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%s - &lt;/tt&gt;full status&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%S - &lt;/tt&gt;abbreviated status&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%u - &lt;/tt&gt;uin&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%w - &lt;/tt&gt;webpage&lt;/li&gt;&lt;/ul&gt;</source>
        <translation>&lt;ul&gt;&lt;li&gt;&lt;tt&gt;%a - &lt;/tt&gt;прізвисько&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%e - &lt;/tt&gt;електронна пошта&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%f - &lt;/tt&gt;ім&apos;я&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%h - &lt;/tt&gt;номер телефону&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%i - &lt;/tt&gt;ip адреса&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%l - &lt;/tt&gt;прізвище&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%m - &lt;/tt&gt;кількість повідомлень&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%n - &lt;/tt&gt;ПІБ&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%o - &lt;/tt&gt;останній раз в мережі&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%O - &lt;/tt&gt;з якого часу в мережі&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%p - &lt;/tt&gt;порт&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%s - &lt;/tt&gt;повний стан&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%S - &lt;/tt&gt;скорочений стан&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%u - &lt;/tt&gt;uin&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%w - &lt;/tt&gt;сторінка у Тенетах&lt;/li&gt;&lt;/ul&gt;</translation>
    </message>
    <message>
        <source>Up since %1
</source>
        <translation>В мережі з %1
</translation>
    </message>
    <message>
        <source>Check %1 Response</source>
        <translation>Переглянути %1 відповідь</translation>
    </message>
    <message>
        <source>You are currently registered as
UIN: %1
Base Directory: %2
Rerun licq with the -b option to select a new
base directory and then register a new user.</source>
        <translation>Ви зараз зареєстровані
Ваш ідентифікаційний номер: %1
Основний каталог: %2
Вкажіть новий основний каталог, стартуйте licq с опцією -b
Після цього Ви зможете зареєструвати новий ідентифікаційний номер.</translation>
    </message>
    <message>
        <source>Auto Request Secure</source>
        <translation>Автоматично встановлювати безпечне з&apos;єднання</translation>
    </message>
    <message>
        <source>Unable to open icons file
%1.</source>
        <translation>Помилка під час відкриття файлу з піктограмами
%1.</translation>
    </message>
    <message>
        <source>Send &amp;Authorization</source>
        <translation>&amp;Відіслати авторизацію</translation>
    </message>
    <message>
        <source>Auto Accept Chats</source>
        <translation>Автоматично дозволяти розмови</translation>
    </message>
    <message>
        <source>Auto Accept Files</source>
        <translation>Автоматично дозволяти передачу файлів</translation>
    </message>
    <message>
        <source>&amp;Not Available</source>
        <translation>Не на&amp;явний</translation>
    </message>
    <message>
        <source>&amp;About</source>
        <translation>Пр&amp;о</translation>
    </message>
    <message>
        <source>&amp;Group</source>
        <translation>&amp;Група</translation>
    </message>
    <message>
        <source>&amp;Hints</source>
        <translation>&amp;Поради</translation>
    </message>
    <message>
        <source>&amp;Reset</source>
        <translation>&amp;Скинути</translation>
    </message>
    <message>
        <source>Custom Auto Response...</source>
        <translation>Нетипова авто відповідь</translation>
    </message>
    <message>
        <source>Online Notify</source>
        <translation>Повідомляти про з&apos;явлення в мережі</translation>
    </message>
    <message>
        <source>Set &amp;Auto Response...</source>
        <translation>Встановити &amp;автовідповідь</translation>
    </message>
    <message>
        <source>Remove From List</source>
        <translation>Видалити зі списку</translation>
    </message>
    <message>
        <source>System Message</source>
        <translation>Системне повідомлення</translation>
    </message>
    <message>
        <source>%1 msg%2</source>
        <translation>%1 повідомлення%2</translation>
    </message>
    <message>
        <source>&amp;Popup All Messages</source>
        <translation>П&amp;оказати всі повідомлення</translation>
    </message>
    <message>
        <source>Accept in Not Available</source>
        <translation>Приймати, у стані &quot;Не наявний&quot;</translation>
    </message>
    <message>
        <source>View &amp;History</source>
        <translation>Переглянути &amp;історію</translation>
    </message>
    <message>
        <source>Other Users</source>
        <translation>Інші користувачі</translation>
    </message>
    <message>
        <source>Edit User Group</source>
        <translation>Редагувати групи</translation>
    </message>
    <message>
        <source>Successfully registered, your user identification
number (UIN) is %1.
Now set your personal information.</source>
        <translation>Реєстрацію пройдено успішно. Ваш ідентифікаційний номер - %1.
Зараз введіть інформацію про себе.</translation>
    </message>
    <message>
        <source>Unknown Packets</source>
        <translation>Невідомі пакети</translation>
    </message>
    <message>
        <source>Send &amp;URL</source>
        <translation>Надіслати &amp;URL</translation>
    </message>
    <message>
        <source>Send &amp;SMS</source>
        <translation>Відіслати &amp;SMS</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Скасувати</translation>
    </message>
    <message>
        <source>&lt;hr&gt;&lt;p&gt; For more information, see the Licq webpage (&lt;tt&gt;http://www.licq.org&lt;/tt&gt;).&lt;/p&gt;</source>
        <translation>&lt;hr&gt;&lt;p&gt; Додаткову інформацію можна отримати на домашній сторінці Licq (&lt;tt&gt;http://www.licq.org&lt;/tt&gt;).&lt;/p&gt;</translation>
    </message>
    <message>
        <source>&amp;Mini Mode</source>
        <translation>&amp;Міні-режим</translation>
    </message>
    <message>
        <source>Errors</source>
        <translation>Помилки</translation>
    </message>
    <message>
        <source>Close &amp;Secure Channel</source>
        <translation>Закрити &amp;безпечний канал</translation>
    </message>
    <message>
        <source>No msgs</source>
        <translation>Нема повідомлень</translation>
    </message>
    <message>
        <source>Licq version %1%8.
Qt GUI plugin version %2.
Compiled on: %7
%6
Author: Graham Roff
Contributions by Dirk A. Mueller
http://www.licq.org

%3 (%4)
%5 contacts.</source>
        <translation type="obsolete">Версія Licq: %1%8.
Версія модуля Qt GUI: %2.
Дата компіляції: %7
%6
Автор: Graham Roff
Участь Dirk Mueller
http://www.licq.org

%3 (%4)
%5 у списку контактів.</translation>
    </message>
    <message>
        <source>Up since %1

</source>
        <translation>В мережі з %1

</translation>
    </message>
    <message>
        <source>Misc Modes</source>
        <translation>Режими</translation>
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
(Сьогодні/Всього)
</translation>
    </message>
    <message>
        <source>SysMsg</source>
        <translation>Сист. повід.</translation>
    </message>
    <message>
        <source>System</source>
        <translation>Система</translation>
    </message>
    <message>
        <source>Last reset %1

</source>
        <translation>Час останнього скидання %1

</translation>
    </message>
    <message>
        <source>Are you sure you want to remove
%1 (%2)
from the &apos;%3&apos; group?</source>
        <translation>Ви впевнені, що бажаєте видалити
%1 (%2)
з групи &apos;%3&apos;?</translation>
    </message>
    <message>
        <source>Do you really want to add
%1 (%2)
to your ignore list?</source>
        <translation>Ви впевнені, що хочете додати
%1 (%2)
до списку тих, що ігноруються?</translation>
    </message>
    <message>
        <source>Packets</source>
        <translation>Пакети</translation>
    </message>
    <message>
        <source>&amp;Save All Users</source>
        <translation>&amp;Зберегти всіх користувачів</translation>
    </message>
    <message>
        <source>%1: %2 / %3
</source>
        <translation>%1: %2 / %3
</translation>
    </message>
    <message>
        <source>Do Not Disturb to User</source>
        <translation>Видимий у стані &quot;Не турбувати&quot;</translation>
    </message>
    <message>
        <source>Update All Users</source>
        <translation>Поновити всіх користувачів</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Hints for Using&lt;br&gt;the Licq Qt-GUI Plugin&lt;/h2&gt;&lt;br&gt;&lt;hr&gt;&lt;br&gt;&lt;ul&gt;&lt;li&gt;Change your status by right clicking on the status label.&lt;/li&gt;&lt;li&gt;Change your auto response by double-clicking on the status label.&lt;/li&gt;&lt;li&gt;View system messages by double clicking on the message label.&lt;/li&gt;&lt;li&gt;Change groups by right clicking on the message label.&lt;/li&gt;&lt;li&gt;Use the following shortcuts from the contact list:&lt;ul&gt;&lt;li&gt;&lt;tt&gt;Ctrl-M : &lt;/tt&gt;Toggle mini-mode&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-O : &lt;/tt&gt;Toggle show offline users&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-X : &lt;/tt&gt;Exit&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-H : &lt;/tt&gt;Hide&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-I : &lt;/tt&gt;View the next message&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-V : &lt;/tt&gt;View message&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-S : &lt;/tt&gt;Send message&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-U : &lt;/tt&gt;Send Url&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-C : &lt;/tt&gt;Send chat request&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-F : &lt;/tt&gt;Send File&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-A : &lt;/tt&gt;Check Auto response&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-P : &lt;/tt&gt;Popup all messages&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-L : &lt;/tt&gt;Redraw user window&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Delete : &lt;/tt&gt;Delete user from current group&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-Delete : &lt;/tt&gt;Delete user from contact list&lt;/li&gt;&lt;/ul&gt;&lt;li&gt;Hold control while clicking on close in the function window to remove   the user from your contact list.&lt;/li&gt;&lt;li&gt;Hit Ctrl-Enter from most text entry fields to select &quot;Ok&quot; or &quot;Accept&quot;.   For example in the send tab of the user function window.&lt;/li&gt;&lt;li&gt;Here is the complete list of user % options, which can be used in &lt;b&gt;OnEvent&lt;/b&gt;   parameters, &lt;b&gt;auto responses&lt;/b&gt;, and &lt;b&gt;utilities&lt;/b&gt;:
</source>
        <translation>&lt;h2&gt;Поради, що до використовування&lt;br&gt; QT-модуля для Licq&lt;/h2&gt;&lt;br&gt;&lt;hr&gt;&lt;br&gt;&lt;ul&gt;&lt;li&gt;Стан можна змінити натисканням правої кнопки на полі стану.&lt;/li&gt;&lt;li&gt;Автовідповідь можна змінити подвійним клацанням на полі стану.&lt;/li&gt;&lt;li&gt;Для перегляду системних повідомлень можна використовувати подвійний клацання на полі повідомлень.&lt;/li&gt;&lt;li&gt;Змінити групи можна натисканням правої кнопки миша на полі повідомлень.&lt;/li&gt;&lt;li&gt;Ви можете у списку контактів використовувати наступні прив&apos;язки клавіш:&lt;ul&gt;&lt;li&gt;&lt;tt&gt;Ctrl-M : &lt;/tt&gt;Перемикання міні-режиму&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-O : &lt;/tt&gt;Показувати/не показувати користувачів не в мережі&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-X : &lt;/tt&gt;Вихід&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-H : &lt;/tt&gt;Cховати&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-I : &lt;/tt&gt;Перегляд наступного повідомлення&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-V : &lt;/tt&gt;Перегляд повідомлення&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-S : &lt;/tt&gt;Відіслати повідомлення&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-U : &lt;/tt&gt;Відіслати URL&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-C : &lt;/tt&gt;Відіслати виклик на розмову&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-F : &lt;/tt&gt;Відіслати файл(и)&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-A : &lt;/tt&gt;Перевірити авто відповідь&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-P : &lt;/tt&gt;Відкрити всі повідомлення&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-L : &lt;/tt&gt;Перерисувати вікно користувачів&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Delete : &lt;/tt&gt;Видалити користувача з поточної групи&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-Delete : &lt;/tt&gt;Видалити користувача зі списку контактів&lt;/li&gt;&lt;/ul&gt;&lt;li&gt;Якщо натиснути кнопку &quot;закрити&quot; утримуючі клавішу Ctrl натиснутою,   то можна видалити користувача з списку контактів.&lt;/li&gt;&lt;li&gt;Можна використовувати Ctrl-Enter в багатьох полях вводу щоб вибрати   &quot;Гаразд&quot; або &quot;Прийняти&quot;. Наприклад у вікні відсилання повідомлення.&lt;/li&gt;&lt;li&gt;Тут повний список всіх %-параметрів, які можна використовувати    у &lt;b&gt;дії на повідомлення&lt;/b&gt;, &lt;b&gt;автоматичні відповіді&lt;/b&gt;    і &lt;b&gt;утиліти&lt;/b&gt;:
</translation>
    </message>
    <message>
        <source>&amp;Redraw User Window</source>
        <translation>&amp;Поновити вікно користувачів</translation>
    </message>
    <message>
        <source>(with KDE support)
</source>
        <translation>(з підтримкою KDE)
</translation>
    </message>
    <message>
        <source>Occupied to User</source>
        <translation>Видимий у стані &quot;Зайнято&quot;</translation>
    </message>
    <message>
        <source>Logon failed.
See network window for details.</source>
        <translation>Вхід до мережі зазнав невдачу.
Подробиці у Вікні мережі.</translation>
    </message>
    <message>
        <source>Accept in Away</source>
        <translation>Приймати, у стані &quot;Відійшов&quot;</translation>
    </message>
    <message>
        <source>Registration failed.  See network window for details.</source>
        <translation>Невдача під час реєстрації. Подробиці у вікні мережі.</translation>
    </message>
    <message>
        <source>Send &amp;Chat Request</source>
        <translation>&amp;Відіслати запрошення на розмову</translation>
    </message>
    <message>
        <source>Reg&amp;ister User</source>
        <translation>&amp;Зареєструвати користувача</translation>
    </message>
    <message>
        <source>Show Offline &amp;Users</source>
        <translation>Показувати користувачів, що не в м&amp;ережі</translation>
    </message>
    <message>
        <source>%1 message%2</source>
        <translation>%1 повідомлення%2</translation>
    </message>
    <message>
        <source>Ignore List</source>
        <translation>Ті, що ігноруються</translation>
    </message>
    <message>
        <source>%1: %2
</source>
        <translation>%1: %2
</translation>
    </message>
    <message>
        <source>&amp;More Info</source>
        <translation>&amp;Додаткова інформація</translation>
    </message>
    <message>
        <source>Warnings</source>
        <translation>Попередження</translation>
    </message>
    <message>
        <source>&amp;Work Info</source>
        <translation>&amp;Інформація про роботу</translation>
    </message>
    <message>
        <source>Debug Level</source>
        <translation>Рівень зневадження</translation>
    </message>
    <message>
        <source>New Users</source>
        <translation>Нові користувачі</translation>
    </message>
    <message>
        <source>&amp;Network Window</source>
        <translation>&amp;Журнал мережі</translation>
    </message>
    <message>
        <source>&amp;View Event</source>
        <translation>&amp;Перегляд подій</translation>
    </message>
    <message>
        <source>&amp;Plugin Manager...</source>
        <translation>М&amp;енеджер модулів...</translation>
    </message>
    <message>
        <source>O&amp;ccupied</source>
        <translation>&amp;Зайнято</translation>
    </message>
    <message>
        <source>Set All</source>
        <translation>Встановити всі</translation>
    </message>
    <message>
        <source>&amp;View System Messages</source>
        <translation>Пере&amp;гляд системних повідомлень</translation>
    </message>
    <message>
        <source>&amp;Thread Group View</source>
        <translation>По &amp;групам</translation>
    </message>
    <message>
        <source>Send &amp;Message</source>
        <translation>Відіслати п&amp;овідомлення</translation>
    </message>
    <message>
        <source>Update Current Group</source>
        <translation>Поновити поточну групу</translation>
    </message>
    <message>
        <source>Licq (%1)</source>
        <translation>Licq (%1)</translation>
    </message>
    <message>
        <source>Right click - Status menu
Double click - Set auto response</source>
        <translation>Права кнопка - Меню стану
Подвійне клацання - Встановити автовідповідь</translation>
    </message>
    <message>
        <source>&amp;Add User</source>
        <translation>&amp;Додати користувача</translation>
    </message>
    <message>
        <source>Accept in Do Not Disturb</source>
        <translation>Приймати, у стані &quot;Не тривожити&quot;</translation>
    </message>
    <message>
        <source>&amp;Security Options</source>
        <translation type="obsolete">&amp;Параметри безпеки</translation>
    </message>
    <message>
        <source>&amp;Statistics</source>
        <translation>&amp;Статистика</translation>
    </message>
    <message>
        <source>&amp;Online</source>
        <translation>&amp;В мережі</translation>
    </message>
    <message>
        <source>Clear All</source>
        <translation>Очистити все</translation>
    </message>
    <message>
        <source>&amp;Status</source>
        <translation>&amp;Стан</translation>
    </message>
    <message>
        <source>&amp;System</source>
        <translation>&amp;Система</translation>
    </message>
    <message>
        <source>Accept in Occupied</source>
        <translation>Приймати, у стані &quot;Зайнято&quot;</translation>
    </message>
    <message>
        <source>Send Contact &amp;List</source>
        <translation>Відіслати список &amp;контактів</translation>
    </message>
    <message>
        <source>Status Info</source>
        <translation>Інформація про стан</translation>
    </message>
    <message>
        <source>&amp;Do Not Disturb</source>
        <translation>&amp;Не тривожити (зайнятий)</translation>
    </message>
    <message>
        <source>Change &amp;Password</source>
        <translation type="obsolete">Змінити &amp;пароль</translation>
    </message>
    <message>
        <source>Are you sure you want to remove
%1 (%2)
from your contact list?</source>
        <translation>Ви впевнені, що бажаєте видалити
%1 (%2)
з списку контактів?</translation>
    </message>
    <message>
        <source>O&amp;ffline</source>
        <translation>Не в &amp;мережі</translation>
    </message>
    <message>
        <source>A&amp;uthorize User</source>
        <translation>&amp;Авторизувати користувача</translation>
    </message>
    <message>
        <source>Free for C&amp;hat</source>
        <translation>Вільний для &amp;спілкування</translation>
    </message>
    <message>
        <source>Right click - User groups
Double click - Show next message</source>
        <translation>Права кнопка - Групи користувачів
Подвійне клацання - Показати наступне повідомлення</translation>
    </message>
    <message>
        <source>No messages</source>
        <translation>Нема повідомлень</translation>
    </message>
    <message>
        <source>&amp;Invisible</source>
        <translation>&amp;Невидимий</translation>
    </message>
    <message>
        <source>&amp;General Info</source>
        <translation>&amp;Загальна інформація</translation>
    </message>
    <message>
        <source>Send &amp;File Transfer</source>
        <translation>&amp;Відіслати файл</translation>
    </message>
    <message>
        <source>User Functions</source>
        <translation>Функції користувача</translation>
    </message>
    <message>
        <source>Check Auto Response</source>
        <translation>Переглянути авто відповідь</translation>
    </message>
    <message>
        <source>Authorization granted.</source>
        <translation>Авторизацію підтверджено.</translation>
    </message>
    <message>
        <source>System Functions</source>
        <translation>Системні операції</translation>
    </message>
    <message>
        <source>Edit &amp;Groups</source>
        <translation>Редагувати &amp;групи</translation>
    </message>
    <message>
        <source>All Users</source>
        <translation>Всі користувачі</translation>
    </message>
    <message>
        <source>S&amp;kin Browser...</source>
        <translation>&amp;Вибір зовнішнього вигляду</translation>
    </message>
    <message>
        <source>Request &amp;Secure Channel</source>
        <translation>Безпечний &amp;канал</translation>
    </message>
    <message>
        <source>S&amp;earch for User</source>
        <translation>По&amp;шук користувача</translation>
    </message>
    <message>
        <source>Next &amp;Server</source>
        <translation>&amp;Наступний сервер</translation>
    </message>
    <message>
        <source>&amp;History</source>
        <translation>&amp;Історія</translation>
    </message>
    <message>
        <source>Not Available to User</source>
        <translation>Видимий у стані &quot;Не наявний&quot;</translation>
    </message>
    <message>
        <source>Toggle &amp;Floaty</source>
        <translation>&amp;Ввімкнути/Вимкнути спливаюче вікно</translation>
    </message>
    <message>
        <source>Sa&amp;ve Settings</source>
        <translation>З&amp;берегти параметри</translation>
    </message>
    <message>
        <source>Away to User</source>
        <translation>Видимий у стані &quot;Відійшов&quot;</translation>
    </message>
    <message>
        <source> </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Visible List</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Invisible List</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Send Authorization Re&amp;quest</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Last</source>
        <translation type="unfinished">&amp;Останні події</translation>
    </message>
    <message>
        <source>&amp;Security/Password Options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Re&amp;quest Authorization</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Use Real Ip (LAN)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>CQtLogWindow</name>
    <message>
        <source>&amp;Save</source>
        <translation>&amp;Зберегти</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Закрити</translation>
    </message>
    <message>
        <source>Failed to open file:
%1</source>
        <translation>Не вдалося відкрити файл:
%1</translation>
    </message>
    <message>
        <source>C&amp;lear</source>
        <translation>О&amp;чистити</translation>
    </message>
    <message>
        <source>Licq Network Log</source>
        <translation>Licq: Журнал мережі</translation>
    </message>
</context>
<context>
    <name>CRandomChatDlg</name>
    <message>
        <source>Games</source>
        <translation>Ігри</translation>
    </message>
    <message>
        <source>Women Seeking Men</source>
        <translation type="obsolete">Жінки, що шукають чоловіків</translation>
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
        <source>Men Seeking Women</source>
        <translation type="obsolete">Чоловіки, що шукають жінок</translation>
    </message>
    <message>
        <source>40 Something</source>
        <translation>40+</translation>
    </message>
    <message>
        <source>Random chat search timed out.</source>
        <translation>Пошук випадкового співрозмовника зазнав невдачі - перевищено максимальний час очікування.</translation>
    </message>
    <message>
        <source>Random chat search had an error.</source>
        <translation>Пошук випадкового співрозмовника зазнав невдачі.</translation>
    </message>
    <message>
        <source>No random chat user found in that group.</source>
        <translation>Не можу знайти жодного користувача в цій групі,
який бажає прийняти участь в випадковому спілкуванні.</translation>
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
        <translation>&amp;Скасувати</translation>
    </message>
    <message>
        <source>Searching for Random Chat Partner...</source>
        <translation>Пошук випадкового співрозмовника...</translation>
    </message>
    <message>
        <source>&amp;Search</source>
        <translation>&amp;Шукати</translation>
    </message>
    <message>
        <source>Students</source>
        <translation>Студенти</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Загальне</translation>
    </message>
    <message>
        <source>Random Chat Search</source>
        <translation>Пошук випадкового співрозмовника</translation>
    </message>
    <message>
        <source>Seeking Women</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Seeking Men</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>CRefuseDlg</name>
    <message>
        <source>Licq %1 Refusal</source>
        <translation>Відмова Licq %1</translation>
    </message>
    <message>
        <source>Refusal message for %1 with </source>
        <translation>Причина відмови в %1 з </translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Скасувати</translation>
    </message>
    <message>
        <source>Refuse</source>
        <translation>Відмовити</translation>
    </message>
</context>
<context>
    <name>CSetRandomChatGroupDlg</name>
    <message>
        <source>&amp;Set</source>
        <translation>&amp;Встановити</translation>
    </message>
    <message>
        <source>done</source>
        <translation>завершено</translation>
    </message>
    <message>
        <source>Games</source>
        <translation>Ігри</translation>
    </message>
    <message>
        <source>Women Seeking Men</source>
        <translation type="obsolete">Жінки, що шукають чоловіків</translation>
    </message>
    <message>
        <source>error</source>
        <translation>помилка</translation>
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
        <source>Men Seeking Women</source>
        <translation type="obsolete">Чоловіки, що шукають жінок</translation>
    </message>
    <message>
        <source>40 Something</source>
        <translation>40+</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Закрити</translation>
    </message>
    <message>
        <source>(none)</source>
        <translation>(нема)</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>таймаут</translation>
    </message>
    <message>
        <source>Set Random Chat Group</source>
        <translation>Встановити групу для випадкових розмов</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>невдача</translation>
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
        <translation>&amp;Скасувати</translation>
    </message>
    <message>
        <source>Students</source>
        <translation>Студенти</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Загальні розмови</translation>
    </message>
    <message>
        <source>Setting Random Chat Group...</source>
        <translation>Встановлюється група для випадкових розмов...</translation>
    </message>
    <message>
        <source>Seeking Women</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Seeking Men</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>CUserView</name>
    <message>
        <source>S</source>
        <translation>С</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;nobr&gt;Ip: </source>
        <translation>&lt;br&gt;&lt;nobr&gt;Ip: </translation>
    </message>
    <message>
        <source>Greek</source>
        <translation type="obsolete">Грецьке</translation>
    </message>
    <message>
        <source>Tamil</source>
        <translation type="obsolete">Тамільське</translation>
    </message>
    <message>
        <source>Cyrillic</source>
        <translation type="obsolete">Кирилиця</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;u&gt;Auto Response:&lt;/u&gt;</source>
        <translation>&lt;br&gt;&lt;u&gt;Автовідповідь:&lt;/u&gt;</translation>
    </message>
    <message>
        <source>Baltic</source>
        <translation type="obsolete">Балтійське</translation>
    </message>
    <message>
        <source>Arabic</source>
        <translation type="obsolete">Арабське</translation>
    </message>
    <message>
        <source>Hebrew</source>
        <translation type="obsolete">Єврейське</translation>
    </message>
    <message>
        <source>Korean</source>
        <translation type="obsolete">Корейське</translation>
    </message>
    <message>
        <source>Online</source>
        <translation>В мережі</translation>
    </message>
    <message>
        <source>Offline</source>
        <translation>Не в мережі</translation>
    </message>
    <message>
        <source>Western European</source>
        <translation type="obsolete">Західноєвропейське</translation>
    </message>
    <message>
        <source>Esperanto</source>
        <translation type="obsolete">Есперанто</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;nobr&gt;C: </source>
        <translation>&lt;br&gt;&lt;nobr&gt;Моб.: </translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;nobr&gt;F: </source>
        <translation>&lt;br&gt;&lt;nobr&gt;Факс: </translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;nobr&gt;E: </source>
        <translation>&lt;br&gt;&lt;nobr&gt;Ел. адр.: </translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;nobr&gt;P: </source>
        <translation>&lt;br&gt;&lt;nobr&gt;Тел.: </translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;nobr&gt;O: </source>
        <translation>&lt;br&gt;&lt;nobr&gt;Останній раз: </translation>
    </message>
    <message>
        <source>Central European</source>
        <translation type="obsolete">Центральноєвропейське</translation>
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
        <translation>&lt;br&gt;&lt;b&gt;Сьогодні&amp;nbsp;День&amp;nbsp;народження!&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Japanese</source>
        <translation type="obsolete">Японське</translation>
    </message>
    <message>
        <source>Ukrainian</source>
        <translation type="obsolete">Українське</translation>
    </message>
    <message>
        <source>%1 Floaty (%2)</source>
        <translation>%1 (%2)</translation>
    </message>
    <message>
        <source>Chinese</source>
        <translation type="obsolete">Китайське</translation>
    </message>
    <message>
        <source>&lt;br&gt;Custom&amp;nbsp;Auto&amp;nbsp;Response</source>
        <translation>&lt;br&gt;Нетипова&amp;nbsp;автовідповідь</translation>
    </message>
    <message>
        <source>&lt;br&gt;Secure&amp;nbsp;connection</source>
        <translation>&lt;br&gt;Безпечне&amp;nbsp;з&apos;єднання</translation>
    </message>
    <message>
        <source>Turkish</source>
        <translation type="obsolete">Турецьке</translation>
    </message>
    <message>
        <source>Unicode</source>
        <translation type="obsolete">Юнікод</translation>
    </message>
    <message>
        <source>Chinese Traditional</source>
        <translation type="obsolete">Традиційне китайське</translation>
    </message>
    <message>
        <source>&lt;/nobr&gt;</source>
        <translation>&lt;/nobr&gt;</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;nobr&gt;Logged In: </source>
        <translation>&lt;br&gt;&lt;nobr&gt;В мережі: </translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;nobr&gt;Idle: </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>CUtilityDlg</name>
    <message>
        <source>GUI</source>
        <translation>Графічний інтерфейс</translation>
    </message>
    <message>
        <source>&amp;Run</source>
        <translation>&amp;Виконати</translation>
    </message>
    <message>
        <source>Done</source>
        <translation>Завершено</translation>
    </message>
    <message>
        <source>Done:</source>
        <translation>Завершено:</translation>
    </message>
    <message>
        <source>Edit:</source>
        <translation>Редагування:</translation>
    </message>
    <message>
        <source>Edit final command</source>
        <translation>Редагування команди</translation>
    </message>
    <message>
        <source>Command Window</source>
        <translation>Вікно команд</translation>
    </message>
    <message>
        <source>Licq Utility: %1</source>
        <translation>Утиліта Licq: %1</translation>
    </message>
    <message>
        <source>C&amp;lose</source>
        <translation>&amp;Закрити</translation>
    </message>
    <message>
        <source>User Fields</source>
        <translation>Поля користувача</translation>
    </message>
    <message>
        <source>Description:</source>
        <translation>Опис:</translation>
    </message>
    <message>
        <source>Internal</source>
        <translation>Внутрішнє</translation>
    </message>
    <message>
        <source>Command:</source>
        <translation>Команда:</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасувати</translation>
    </message>
    <message>
        <source>Running:</source>
        <translation>Виконується:</translation>
    </message>
    <message>
        <source>Failed:</source>
        <translation>Невдача:</translation>
    </message>
    <message>
        <source>Terminal</source>
        <translation>Термінал</translation>
    </message>
    <message>
        <source>Window:</source>
        <translation>Вікно:</translation>
    </message>
</context>
<context>
    <name>ChangePassDlg</name>
    <message>
        <source>Licq - Set UIN/Password</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Login Information</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;UIN:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Password:</source>
        <translation type="unfinished">&amp;Пароль:</translation>
    </message>
    <message>
        <source>&amp;Verify:</source>
        <translation type="unfinished">&amp;Ще раз:</translation>
    </message>
    <message>
        <source>Enter your UIN here.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Enter your ICQ password here.</source>
        <translation type="unfinished">Введіть тут ваш пароль ICQ.</translation>
    </message>
    <message>
        <source>Verify your ICQ password here.</source>
        <translation type="unfinished">Введіть тут свій пароль ICQ для перевірки.</translation>
    </message>
    <message>
        <source>&amp;Ok</source>
        <translation type="unfinished">&amp;Гаразд</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation type="unfinished">&amp;Скасувати</translation>
    </message>
    <message>
        <source>Invalid password, must be 8 characters or less.</source>
        <translation type="unfinished">Пароль повинен бути не більше 8 символів.</translation>
    </message>
    <message>
        <source>Passwords do not match, try again.</source>
        <translation type="unfinished">Паролі не збігаються, спробуйте ще раз.</translation>
    </message>
</context>
<context>
    <name>ChatDlg</name>
    <message>
        <source>No</source>
        <translation>Ні</translation>
    </message>
    <message>
        <source>Yes</source>
        <translation>Так</translation>
    </message>
    <message>
        <source>Beep</source>
        <translation>Гудок</translation>
    </message>
    <message>
        <source>Bold</source>
        <translation>Жирний</translation>
    </message>
    <message>
        <source>Chat</source>
        <translation>Розмова</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation>Режим</translation>
    </message>
    <message>
        <source>Toggles Bold font</source>
        <translation>Перемикає використання жирного шрифту</translation>
    </message>
    <message>
        <source>Ignores user color settings</source>
        <translation>ігнорує кольори, що встановлені користувачем</translation>
    </message>
    <message>
        <source>Toggles Italic font</source>
        <translation>Перемикає використання курсиву</translation>
    </message>
    <message>
        <source>Remote - Waiting for joiners...</source>
        <translation>Віддалений - Чекаю на з&apos;єднання...</translation>
    </message>
    <message>
        <source>Licq - Chat</source>
        <translation>Licq: Розмова</translation>
    </message>
    <message>
        <source>&amp;Audio</source>
        <translation>&amp;Звук</translation>
    </message>
    <message>
        <source>Sends a Beep to all recipients</source>
        <translation>Відіслати &quot;Гудок&quot; всім</translation>
    </message>
    <message>
        <source>&amp;Pane Mode</source>
        <translation>&amp;Віконний режим</translation>
    </message>
    <message>
        <source>Italic</source>
        <translation>Курсив</translation>
    </message>
    <message>
        <source>Do you want to save the chat session?</source>
        <translation>Бажаєте зберегти розмову?</translation>
    </message>
    <message>
        <source>Licq - Chat %1</source>
        <translation>Licq: Розмова з %1</translation>
    </message>
    <message>
        <source>&amp;IRC Mode</source>
        <translation>&amp;Режим IRC</translation>
    </message>
    <message>
        <source>Changes the foreground color</source>
        <translation>Змінює колір тексту</translation>
    </message>
    <message>
        <source>Unable to load encoding &lt;b&gt;%1&lt;/b&gt;. Message contents may appear garbled.</source>
        <translation>Не вдається завантажити кодування &lt;b&gt;%1&lt;/b&gt;. Зміст повідомлення може бути перекрученим.</translation>
    </message>
    <message>
        <source>Background color</source>
        <translation>Колір тла</translation>
    </message>
    <message>
        <source>Local - %1</source>
        <translation>Локальний - %1</translation>
    </message>
    <message>
        <source>/%1.%2.chat</source>
        <translation>ChatDlg::/%1.%2.розмова</translation>
    </message>
    <message>
        <source>Remote - Not connected</source>
        <translation>Віддалений - Нема з&apos;єднання</translation>
    </message>
    <message>
        <source>&amp;Close Chat</source>
        <translation>&amp;Завершити розмову</translation>
    </message>
    <message>
        <source>Remote - %1</source>
        <translation>Віддалений - %1</translation>
    </message>
    <message>
        <source>Underline</source>
        <translation>Підкреслений</translation>
    </message>
    <message>
        <source>%1 closed connection.</source>
        <translation>%1 закрив з&apos;єднання.</translation>
    </message>
    <message>
        <source>Remote - Connecting...</source>
        <translation>Віддалений - З&apos;єднуюсь...</translation>
    </message>
    <message>
        <source>&amp;Save Chat</source>
        <translation>&amp;Зберегти розмову</translation>
    </message>
    <message>
        <source>Changes the background color</source>
        <translation>Змінює колір тла</translation>
    </message>
    <message>
        <source>Foreground color</source>
        <translation>Колір тексту</translation>
    </message>
    <message>
        <source>Ignore user settings</source>
        <translation>Ігнорувати параметри користувача</translation>
    </message>
    <message>
        <source>Unable to bind to a port.
See Network Window for details.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unable to connect to the remote chat.
See Network Window for details.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unable to create new thread.
See Network Window for details.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Failed to open file:
%1</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>CustomAwayMsgDlg</name>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;Гаразд</translation>
    </message>
    <message>
        <source>&amp;Clear</source>
        <translation>&amp;Очистити</translation>
    </message>
    <message>
        <source>&amp;Hints</source>
        <translation>&amp;Підказка</translation>
    </message>
    <message>
        <source>Set Custom Auto Response for %1</source>
        <translation>Встановити персональну автовідповідь для %1</translation>
    </message>
    <message>
        <source>I am currently %1.
You can leave me a message.</source>
        <translation>Я зараз у стані &quot;%1&quot;.
Ви можете залишити мені повідомлення.</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасувати</translation>
    </message>
</context>
<context>
    <name>EditFileDlg</name>
    <message>
        <source>&amp;Save</source>
        <translation>&amp;Зберегти</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Закрити</translation>
    </message>
    <message>
        <source>Failed to open file:
%1</source>
        <translation>Не вдалося відкрити файл:
%1</translation>
    </message>
    <message>
        <source>Licq File Editor - %1</source>
        <translation>Licq: Редактор файлів - %1</translation>
    </message>
    <message>
        <source>[ Read-Only ]</source>
        <translation>[ тільки для читання ]</translation>
    </message>
</context>
<context>
    <name>EditGrpDlg</name>
    <message>
        <source>Ok</source>
        <translation>Гаразд</translation>
    </message>
    <message>
        <source>Add</source>
        <translation>Додати</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation type="obsolete">Редагування</translation>
    </message>
    <message>
        <source>&amp;Done</source>
        <translation>&amp;Закрити</translation>
    </message>
    <message>
        <source>Edit Name</source>
        <translation>Змінити ім&apos;я</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Скасувати</translation>
    </message>
    <message>
        <source>Groups</source>
        <translation>Групи</translation>
    </message>
    <message>
        <source>Are you sure you want to remove
the group &apos;%1&apos;?</source>
        <translation>Ви впевнені, що бажаєте видалити
групу &apos;%1&apos;?</translation>
    </message>
    <message>
        <source>Remove</source>
        <translation>Видалити</translation>
    </message>
    <message>
        <source>noname</source>
        <translation>без імені</translation>
    </message>
    <message>
        <source>Set Default</source>
        <translation>Типова група</translation>
    </message>
    <message>
        <source>Default:</source>
        <translation>По замовчуванню:</translation>
    </message>
    <message>
        <source>The default group to start up in.</source>
        <translation>Типова група при запуску</translation>
    </message>
    <message>
        <source>Edit group name (hit enter to save).</source>
        <translation>Змінити ім&apos;я групи (Натисніть Enter, щоб зберегти).</translation>
    </message>
    <message>
        <source>Shift Down</source>
        <translation>Вниз</translation>
    </message>
    <message>
        <source>All Users</source>
        <translation>Всі користувачі</translation>
    </message>
    <message>
        <source>Licq - Edit Groups</source>
        <translation>Licq - Редагування груп</translation>
    </message>
    <message>
        <source>Shift Up</source>
        <translation>Вгору</translation>
    </message>
    <message>
        <source>Set New Users</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The group to which new users will be automatically added.  All new users will be in the local system group New Users but for server side storage will also be stored in the specified group.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>New User:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Save</source>
        <translation type="unfinished">&amp;Зберегти</translation>
    </message>
    <message>
        <source>Save the name of a group being modified.</source>
        <translation type="unfinished"></translation>
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
        <translation>URL</translation>
    </message>
    <message>
        <source>Message</source>
        <translation>Повідомлення</translation>
    </message>
    <message>
        <source>Added to Contact List</source>
        <translation>Доданий до списку контактів</translation>
    </message>
    <message>
        <source>Web Panel</source>
        <translation>Панель Web</translation>
    </message>
    <message>
        <source>Plugin Event</source>
        <translation>Подія додатку</translation>
    </message>
    <message>
        <source>File Transfer</source>
        <translation>Передача файла(ів)</translation>
    </message>
    <message>
        <source>(cancelled)</source>
        <translation>(скасовано)</translation>
    </message>
    <message>
        <source>Authorization Request</source>
        <translation>Запит на авторизацію</translation>
    </message>
    <message>
        <source>Authorization Refused</source>
        <translation>Відмова в авторизації</translation>
    </message>
    <message>
        <source>Unknown Event</source>
        <translation>Невідома подія</translation>
    </message>
    <message>
        <source>Chat Request</source>
        <translation>Запит на розмову</translation>
    </message>
    <message>
        <source>Authorization Granted</source>
        <translation>Авторизацію підтверджено</translation>
    </message>
    <message>
        <source>Email Pager</source>
        <translation>Email-пейджер</translation>
    </message>
    <message>
        <source>Contact List</source>
        <translation>Список контактів</translation>
    </message>
</context>
<context>
    <name>HintsDlg</name>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Закрити</translation>
    </message>
    <message>
        <source>Licq - Hints</source>
        <translation>Підказки Licq</translation>
    </message>
</context>
<context>
    <name>IconManager_KDEStyle</name>
    <message>
        <source>&lt;br&gt;1 msg</source>
        <translation>&lt;br&gt;1 повід.</translation>
    </message>
    <message>
        <source>&lt;br&gt;%1 msgs</source>
        <translation>&lt;br&gt;%1 повід.</translation>
    </message>
    <message>
        <source>&lt;br&gt;Left click - Show main window&lt;br&gt;Middle click - Show next message&lt;br&gt;Right click - System menu</source>
        <translation>&lt;br&gt;Ліва кнопка - Головне вікно&lt;br&gt;Середня кнопка - наступне повідомлення&lt;br&gt;Права кнопка - Системне меню</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;b&gt;%1 system messages&lt;/b&gt;</source>
        <translation>&lt;br&gt;&lt;b&gt;%1 системних повідомлень.&lt;/b&gt;</translation>
    </message>
</context>
<context>
    <name>IconManager_Themed</name>
    <message>
        <source>Unable to load dock theme image
%1</source>
        <translation>Не вдається завантажити файл с образом для дока
%1</translation>
    </message>
    <message>
        <source>Unable to load dock theme file
(%1)
:%2</source>
        <translation>Не вдається завантажити файл с темою для дока
(%1)
:%2</translation>
    </message>
</context>
<context>
    <name>KeyRequestDlg</name>
    <message>
        <source>&amp;Send</source>
        <translation>&amp;Відіслати</translation>
    </message>
    <message>
        <source>Closing secure channel...</source>
        <translation>Закриваю безпечне з&apos;єднання...</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Закрити</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;ForestGreen&quot;&gt;Secure channel established.&lt;/font&gt;
</source>
        <translation>&lt;font color=&quot;ForestGreen&quot;&gt;Встановлено безпечне з&apos;єднання.&lt;/font&gt;
</translation>
    </message>
    <message>
        <source>Client does not support OpenSSL.
Rebuild Licq with OpenSSL support.</source>
        <translation>Клієнт не підтримує OpenSSL.
Зберіть Licq з підтримкою OpenSSL.</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;blue&quot;&gt;Secure channel closed.&lt;/font&gt;
</source>
        <translation>&lt;font color=&quot;blue&quot;&gt;Безпечне з&apos;єднання закрито.&lt;/font&gt;
</translation>
    </message>
    <message>
        <source>Licq - Secure Channel with %1</source>
        <translation>Licq - Безпечне з&apos;єднання з %1</translation>
    </message>
    <message>
        <source>Secure channel is established using SSL
with Diffie-Hellman key exchange and
the TLS version 1 protocol.

</source>
        <translation>Безпечне з&apos;єднання буде встановлюватись
за допомогою SSL з використанням алгоритму
Diffie-Hellman для обміну ключами
та протоколу TLS версії 1.

</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;Could not connect to remote client.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;Не можу з&apos;єднатися з віддаленим клієнтом.&lt;/font&gt;</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;yellow&quot;&gt;Secure channel already established.&lt;/font&gt;
</source>
        <translation>&lt;font color=&quot;yellow&quot;&gt;Безпечне з&apos;єднання вже встановлено.&lt;/font&gt;
</translation>
    </message>
    <message>
        <source>The remote uses Licq %1, however it
has no secure channel support compiled in.
This probably won&apos;t work.</source>
        <translation>Віддалена сторона використовує Licq %1, однак цей клієнт
було зібрано без підтримки безпечних з&apos;єднань.
Можливо, безпечне з&apos;єднання не вдасться встановити.</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;yellow&quot;&gt;Secure channel not established.&lt;/font&gt;
</source>
        <translation>&lt;font color=&quot;yellow&quot;&gt;Безпечне з&apos;єднання не встановлено.&lt;/font&gt;
</translation>
    </message>
    <message>
        <source>The remote uses Licq %1/SSL.</source>
        <translation>Віддалена сторона використовує Licq %1/SSL.</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;Remote client does not support OpenSSL.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;Віддалений клієнт не підтримує OpenSSL.&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Requesting secure channel...</source>
        <translation>Запитую на безпечне з&apos;єднання...</translation>
    </message>
    <message>
        <source>This only works with other Licq clients &gt;= v0.85
The remote doesn&apos;t seem to use such a client.
This might not work.</source>
        <translation>Це повинно працювати з іншими клієнтами Licq &gt;= 0.85.
Але віддалена сторона не 
використовує такого клієнта.
Цілком можливо, що безпечне з&apos;єднання не вдасться встановити.</translation>
    </message>
</context>
<context>
    <name>MLView</name>
    <message>
        <source>Licq is unable to find a browser application due to an internal error.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Licq is unable to start your browser and open the URL.
You will need to start the browser and open the URL manually.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>MsgView</name>
    <message>
        <source>D</source>
        <translation>П</translation>
    </message>
    <message>
        <source>Time</source>
        <translation>Час</translation>
    </message>
    <message>
        <source>Cancelled Event</source>
        <translation>Скасована подія</translation>
    </message>
    <message>
        <source>Direct</source>
        <translation>Пряме</translation>
    </message>
    <message>
        <source>Event Type</source>
        <translation>Тип події</translation>
    </message>
    <message>
        <source>Server</source>
        <translation>Через сервер</translation>
    </message>
    <message>
        <source>Urgent</source>
        <translation>Термінове</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>Параметри</translation>
    </message>
    <message>
        <source>Multiple Recipients</source>
        <translation>Декілька адресатів</translation>
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
        <translation>&amp;Гаразд</translation>
    </message>
    <message>
        <source>Fax</source>
        <translation>Факс</translation>
    </message>
    <message>
        <source>Auto</source>
        <translation>Автоматично</translation>
    </message>
    <message>
        <source>Away</source>
        <translation>Відійшов</translation>
    </message>
    <message>
        <source>Font</source>
        <translation>Шрифт</translation>
    </message>
    <message>
        <source>N/A:</source>
        <translation>Не наявний:</translation>
    </message>
    <message>
        <source>Left</source>
        <translation>Ліворуч</translation>
    </message>
    <message>
        <source>Save</source>
        <translation>Зберегти</translation>
    </message>
    <message>
        <source>URL:</source>
        <translation>URL:</translation>
    </message>
    <message>
        <source>none</source>
        <translation>без перекодування</translation>
    </message>
    <message>
        <source>Show Extended Icons</source>
        <translation>Показувати додаткові піктограми</translation>
    </message>
    <message>
        <source>Blink All Events</source>
        <translation>Блимання подій</translation>
    </message>
    <message>
        <source>Away:</source>
        <translation>Авто-відійшов:</translation>
    </message>
    <message>
        <source>Email</source>
        <translation>Адреса ел. пошти</translation>
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
        <translation>Ніколи</translation>
    </message>
    <message>
        <source>Phone</source>
        <translation>Телефон</translation>
    </message>
    <message>
        <source>Proxy</source>
        <translation>Параметри проксі сервера:</translation>
    </message>
    <message>
        <source>Right</source>
        <translation>Праворуч</translation>
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
        <translation>Параметри для отриманих запитів на передачу файлів</translation>
    </message>
    <message>
        <source>Firewall</source>
        <translation>Фаєрвол</translation>
    </message>
    <message>
        <source>Use proxy server</source>
        <translation>Користуватись проксі сервером</translation>
    </message>
    <message>
        <source>Show Grid Lines</source>
        <translation>Показувати ґратку</translation>
    </message>
    <message>
        <source>Override the skin setting for the frame style of the user window:
   0 (No frame), 1 (Box), 2 (Panel), 3 (WinPanel)
 + 16 (Plain), 32 (Raised), 48 (Sunken)
 + 240 (Shadow)</source>
        <translation>Користуватись вказаним стилем рамки
   0 (рамки немає), 1 (коробка), 2 (панель), 3 (панель, як у Windows)
 + 16 (проста), 32 (піднята), 48 (занурена)
 + 240 (з тінню)</translation>
    </message>
    <message>
        <source>Parameter for received chat requests</source>
        <translation>Параметри для отриманих запитів на розмову</translation>
    </message>
    <message>
        <source>Auto Away Messages</source>
        <translation>Повідомлення при авто-режимах</translation>
    </message>
    <message>
        <source>Turns on or off the display of headers above each column in the user list</source>
        <translation>Вмикає та вимикає відображення назв стовпчиків у списку користувачів</translation>
    </message>
    <message>
        <source>Start Hidden</source>
        <translation>Запускатись у доку</translation>
    </message>
    <message>
        <source>Auto Away:</source>
        <translation>Авто-відійшов:</translation>
    </message>
    <message>
        <source>Number of minutes of inactivity after which to automatically be marked &quot;not available&quot;.  Set to &quot;0&quot; to disable.</source>
        <translation>Тривалість часу не активності (у хвилинах), після якого Ваш стан автоматично змінюється на &quot;Не наявний&quot;. Встановіть &quot;0&quot;, щоб вимкнути цю можливість.</translation>
    </message>
    <message>
        <source>Blink Urgent Events</source>
        <translation>Блимання термінових подій</translation>
    </message>
    <message>
        <source>Ignore Mass Messages</source>
        <translation>Ігнорувати повідомлення до всіх користувачів</translation>
    </message>
    <message>
        <source>Show the &quot;--online--&quot; and &quot;--offline--&quot; bars in the contact list</source>
        <translation>Показувати роздільники &quot;-- Не в мережі --&quot; та &quot;-- В мережі --&quot; у списку контактів</translation>
    </message>
    <message>
        <source>ICQ Server Port:</source>
        <translation>Порт:</translation>
    </message>
    <message>
        <source>The main window will start hidden. Only the dock icon will be visible.</source>
        <translation type="obsolete">Під час запуску головне вікно буде сховано. Буде відображено тільки піктограму у доці.</translation>
    </message>
    <message>
        <source>Parameter for online notification</source>
        <translation>Параметри для сповіщення о з&apos;явленні в мережі</translation>
    </message>
    <message>
        <source>TCP port range for incoming connections.</source>
        <translation>Діапазон TCP портів для вхідних повідомлень.</translation>
    </message>
    <message>
        <source>Accept Modes</source>
        <translation>У яких станах виконувати команди</translation>
    </message>
    <message>
        <source>Online Notify:</source>
        <translation>Сповістити:</translation>
    </message>
    <message>
        <source>Not Available</source>
        <translation>Не наявний</translation>
    </message>
    <message>
        <source>&amp;Apply</source>
        <translation>&amp;Застосувати</translation>
    </message>
    <message>
        <source>Determines if new users are automatically added to your list or must first request authorization.</source>
        <translation>Визначає повинні чи ні нові користувачі запитувати Вашої авторизації, або вони можуть бути додані до списку користувачів автоматично.</translation>
    </message>
    <message>
        <source>Show the name of the current group in the messages label when there are no new messages</source>
        <translation>Показувати назву поточної групи, якщо немає нових повідомлень</translation>
    </message>
    <message>
        <source>Ignore Web Panel</source>
        <translation>Ігнорувати панель Тенет</translation>
    </message>
    <message>
        <source>Sets which default encoding should be used for newly added contacts.</source>
        <translation>Встановлює кодування, яке буде використовуватись для нових користувачів.</translation>
    </message>
    <message>
        <source>Show the current chat history in Send Window</source>
        <translation>Показує попередні повідомлення у вікні для надсилання повідомлень</translation>
    </message>
    <message>
        <source>Use Font Styles</source>
        <translation>Використовувати стилі шрифтів</translation>
    </message>
    <message>
        <source>Preset slot:</source>
        <translation>Заготовки:</translation>
    </message>
    <message>
        <source>Licq Options</source>
        <translation>Licq - Параметри</translation>
    </message>
    <message>
        <source>Show online notify users who are offline even when offline users are hidden.</source>
        <translation>Показувати користувачів, для яких ввімкнено сповіщення про з&apos;явлення в мережі та які зараз не в мережі, навіть якщо ввімкнено заборону на відображення користувачів, які не в мережі.</translation>
    </message>
    <message>
        <source>Show Column Headers</source>
        <translation>Показувати заголовки стовпчиків</translation>
    </message>
    <message>
        <source>Bold Message Label on Incoming Msg</source>
        <translation>Виділяти жирним шрифтом повідомлення, що надійшли</translation>
    </message>
    <message>
        <source>Column %1</source>
        <translation>Стовпчик %1</translation>
    </message>
    <message>
        <source>Selects between the standard 64x64 icon used in the WindowMaker/Afterstep wharf and a shorter 64x48 icon for use in the Gnome/KDE panel.</source>
        <translation>Вибір між піктограмою розміром 64x64, що використовується в доку WindowMaker/Afterstep та зменшеною піктограмою розміром 64x48, що використовується у панелі Gnome/KDE.</translation>
    </message>
    <message>
        <source>Perform OnEvent command in occupied mode</source>
        <translation>Виконувати команди на події у стані &quot;Зайнято&quot;</translation>
    </message>
    <message>
        <source>Auto Offline:</source>
        <translation>Автоматичний вихід з мережі:</translation>
    </message>
    <message>
        <source>Online Time</source>
        <translation>Час в мережі</translation>
    </message>
    <message>
        <source>Lets you drag around the main window with your mouse</source>
        <translation>Дозволяє пересування вікна за допомогою миша</translation>
    </message>
    <message>
        <source>OnEvent in DND</source>
        <translation>у стані &quot;Не турбувати&quot;</translation>
    </message>
    <message>
        <source>OnEvent in N/A</source>
        <translation>у стані &quot;Не наявний&quot;</translation>
    </message>
    <message>
        <source>Parameter for received messages</source>
        <translation>Параметри для отриманих повідомлень</translation>
    </message>
    <message>
        <source>Message Sent:</source>
        <translation>Повідомлення відіслано:</translation>
    </message>
    <message>
        <source>Center</source>
        <translation>По центру</translation>
    </message>
    <message>
        <source>Proxy Server Port:</source>
        <translation>Порт проксі сервера:</translation>
    </message>
    <message>
        <source>Allow scroll bar</source>
        <translation>Дозволити панель прокрутки</translation>
    </message>
    <message>
        <source>OnEvent in Away</source>
        <translation>у стані &quot;Відійшов&quot;</translation>
    </message>
    <message>
        <source>Network</source>
        <translation>Мережа</translation>
    </message>
    <message>
        <source>Only urgent events will blink</source>
        <translation>Тільки термінові події будуть блимати</translation>
    </message>
    <message>
        <source>Format</source>
        <translation>Формат</translation>
    </message>
    <message>
        <source>The fonts used</source>
        <translation>Шрифти, що використовуються</translation>
    </message>
    <message>
        <source>Number of minutes of inactivity after which to automatically go offline.  Set to &quot;0&quot; to disable.</source>
        <translation>Тривалість часу не активності (у хвилинах), після якого Ваш стан автоматично змінюється на &quot;Не в мережі&quot;. Встановіть &quot;0&quot;, щоб вимкнути цю можливість.</translation>
    </message>
    <message>
        <source>Auto send through server</source>
        <translation>Автоматично відсилати через сервер</translation>
    </message>
    <message>
        <source>Use Dock Icon</source>
        <translation>Використовувати піктограмою у доку</translation>
    </message>
    <message>
        <source>Chat Request:</source>
        <translation>Запит на розмову:</translation>
    </message>
    <message>
        <source>Online</source>
        <translation>В мережі</translation>
    </message>
    <message>
        <source>Server settings</source>
        <translation>Параметри сервера</translation>
    </message>
    <message>
        <source>Auto Logon:</source>
        <translation>Початковий стан:</translation>
    </message>
    <message>
        <source>Main Window</source>
        <translation>Головне вікно</translation>
    </message>
    <message>
        <source>Status</source>
        <translation>Стан</translation>
    </message>
    <message>
        <source>Theme:</source>
        <translation>Тема:</translation>
    </message>
    <message>
        <source>Always show online notify users</source>
        <translation>Завжди показувати користувачів, для яких
ввімкнено сповіщення по з&apos;явлення в мережі</translation>
    </message>
    <message>
        <source>default (%1)</source>
        <translation>типовий (%1)</translation>
    </message>
    <message>
        <source>Offline</source>
        <translation>Не в мережі</translation>
    </message>
    <message>
        <source>Port Range:</source>
        <translation>Діапазон портів</translation>
    </message>
    <message>
        <source>Allow dragging main window</source>
        <translation>Дозволити перетягування головного вікна</translation>
    </message>
    <message>
        <source>Default Encoding:</source>
        <translation>Типове кодування:</translation>
    </message>
    <message>
        <source>OnEvent</source>
        <translation>Команди на події</translation>
    </message>
    <message>
        <source>Localization</source>
        <translation>Локалізація</translation>
    </message>
    <message>
        <source>Command:</source>
        <translation>Команда:</translation>
    </message>
    <message>
        <source>Edit Font:</source>
        <translation>Шрифт редактора:</translation>
    </message>
    <message>
        <source>Hot key: </source>
        <translation>Наступне повідомлення: </translation>
    </message>
    <message>
        <source>I can receive direct connections</source>
        <translation>Дозволити отримання прямих з&apos;єднань</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>Параметри</translation>
    </message>
    <message>
        <source>Miscellaneous</source>
        <translation>Різне</translation>
    </message>
    <message>
        <source>All incoming messages automatically open when received, if we are online (or free for chat)</source>
        <translation type="obsolete">Всі повідомлення, що надійшли, автоматично відкриваються, як тільки надійшли, якщо ваш стан &quot;В мережі&quot; (або &quot;Вільний для розмови&quot;)</translation>
    </message>
    <message>
        <source>Automatically send messages through server if direct connection fails</source>
        <translation type="obsolete">Повідомлення автоматично буде відіслано через сервер, якщо не можна встановити пряме з&apos;єднання</translation>
    </message>
    <message>
        <source>Perform OnEvent command in do not disturb mode</source>
        <translation>Виконувати команди на події у стані &quot;Не турбувати&quot;</translation>
    </message>
    <message>
        <source>Determines if mass messages are ignored or not.</source>
        <translation>Визначає ігнорувати чи ні повідомлення до багатьох адресатів.</translation>
    </message>
    <message>
        <source>Last online</source>
        <translation>Коли останній раз був в мережі</translation>
    </message>
    <message>
        <source>Frame Style: </source>
        <translation>Стиль рамки: </translation>
    </message>
    <message>
        <source>The alignment of the column</source>
        <translation>Вирівнювання стовпчиків</translation>
    </message>
    <message>
        <source>Auto Position the Reply Window</source>
        <translation>Автоматичний вибір місця для вікна відповіді</translation>
    </message>
    <message>
        <source>Make the user window transparent when there is no scroll bar</source>
        <translation>Вікно користувачів стає прозорим, якщо немає стовпчика прокрутки</translation>
    </message>
    <message>
        <source>Default Icon</source>
        <translation>Типова піктограма</translation>
    </message>
    <message>
        <source>If not checked, a user will be automatically removed from &quot;New User&quot; group when you firstsend an event to them</source>
        <translation type="obsolete">Якщо вимкнено, то користувача буде автоматично витерто з групи &quot;Нові користувачі&quot; після першого відсилання йому повідомлення будь-якого типу.</translation>
    </message>
    <message>
        <source>The main window will raise on incoming messages</source>
        <translation type="obsolete">Головне вікно буде &quot;виринати&quot;, коли прийшло повідомлення</translation>
    </message>
    <message>
        <source>Check Clipboard For Urls/Files</source>
        <translation>Перевіряти буфер обміну на наявність файлів/URL</translation>
    </message>
    <message>
        <source>Parameters</source>
        <translation>Параметри</translation>
    </message>
    <message>
        <source>Show birthday, invisible, and custom auto response icons to the right of users in the list</source>
        <translation>Показувати додаткові піктограми (день народження, не видимий, нетипова автовідповідь) праворуч від ім&apos;я користувача.</translation>
    </message>
    <message>
        <source>Transparent when possible</source>
        <translation>Прозоре, якщо можливо</translation>
    </message>
    <message>
        <source>Default Auto Response Messages</source>
        <translation>Типові автовідповіді</translation>
    </message>
    <message>
        <source>File Transfer:</source>
        <translation>Передача файлу:</translation>
    </message>
    <message>
        <source>OnEvents Enabled</source>
        <translation>Команди на події ввімкнено</translation>
    </message>
    <message>
        <source>Paranoia</source>
        <translation>Параноя</translation>
    </message>
    <message>
        <source>Auto-Raise on Incoming Msg</source>
        <translation>Автоматичне виринання, коли прийшло повідомлення</translation>
    </message>
    <message>
        <source>Position a new reply window just underneath the message view window</source>
        <translation>Вікно відповіді буде знаходитись під вікном з повідомленням</translation>
    </message>
    <message>
        <source>Terminal:</source>
        <translation>Термінал:</translation>
    </message>
    <message>
        <source>Automatically log on when first starting up.</source>
        <translation>Автоматично входити до мережі.</translation>
    </message>
    <message>
        <source>Perform the online notify OnEvent when logging on (this is different from how the Mirabilis client works)</source>
        <translation>Сповіщає про з&apos;явлення користувача в мережі (Поведінка відрізняється від поведінки клієнту від Mirabilis).</translation>
    </message>
    <message>
        <source>Number of minutes of inactivity after which to automatically be marked &quot;away&quot;.  Set to &quot;0&quot; to disable.</source>
        <translation>Тривалість часу не активності (у хвилинах), після якого Ваш стан автоматично змінюється на &quot;Відійшов&quot;. Встановіть &quot;0&quot;, щоб вимкнути цю можливість.</translation>
    </message>
    <message>
        <source>&lt;p&gt;Command to execute when an event is received.&lt;br&gt;It will be passed the relevant parameters from below.&lt;br&gt;Parameters can contain the following expressions &lt;br&gt; which will be replaced with the relevant information:&lt;/p&gt;</source>
        <translation>&lt;p&gt;Команда, яку буде виконано, коли одержано повідомлення.&lt;br&gt;Їй буде передано параметри, які описано нижче.&lt;br&gt;Також будуть зроблені наступні заміни:&lt;/p&gt;</translation>
    </message>
    <message>
        <source>ICQ Server:</source>
        <translation>Сервер ICQ:</translation>
    </message>
    <message>
        <source>Determines if email pager messages are ignored or not.</source>
        <translation>Визначає ігнорувати чи ні повідомлення email-пейджера.</translation>
    </message>
    <message>
        <source>Use authorization</source>
        <translation>Вживати авторизацію</translation>
    </message>
    <message>
        <source>Perform OnEvent command in not available mode</source>
        <translation>Виконувати команди на події у стані &quot;Не наявний&quot;</translation>
    </message>
    <message>
        <source>Occupied</source>
        <translation>Зайнято</translation>
    </message>
    <message>
        <source>Show User Dividers</source>
        <translation>Показувати подільники користувачів</translation>
    </message>
    <message>
        <source>OnEvent in Occupied</source>
        <translation>у стані &quot;Зайнято&quot;</translation>
    </message>
    <message>
        <source>The format string used to define what will appear in each column.
The following parameters can be used:
</source>
        <translation>Рядок, який визначає, що буде відображатись в кожному стовпчику.
Можуть бути використані наступні значення:
</translation>
    </message>
    <message>
        <source>Parameter for received system messages</source>
        <translation>Параметри для отриманих системних повідомлень</translation>
    </message>
    <message>
        <source>Manual &quot;New User&quot; group handling</source>
        <translation>Ручна обробка групи &quot;Нові користувачі&quot;</translation>
    </message>
    <message>
        <source>Parameter for sent messages</source>
        <translation>Параметри для відісланих повідомлень</translation>
    </message>
    <message>
        <source>Select Font</source>
        <translation>Виберіть шрифт</translation>
    </message>
    <message>
        <source>The message info label will be bold if there are incoming messages</source>
        <translation type="obsolete">Користувача буде виділено жирним шрифтом, якщо є повідомлення від нього</translation>
    </message>
    <message>
        <source>Auto-Popup Incoming Msg</source>
        <translation>Автоматично показувати нові повідомлення</translation>
    </message>
    <message>
        <source>64 x 48 Dock Icon</source>
        <translation>64 x 48 піктограма для дока</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасувати</translation>
    </message>
    <message>
        <source>Startup</source>
        <translation>При старті</translation>
    </message>
    <message>
        <source>Status:</source>
        <translation>Стан:</translation>
    </message>
    <message>
        <source>Password:</source>
        <translation>Пароль:</translation>
    </message>
    <message>
        <source>default</source>
        <translation>типовий</translation>
    </message>
    <message>
        <source>Use italics and bold in the user list to indicate special characteristics such as online notify and visible list</source>
        <translation>Використовувати жирний шрифт або курсив у списку користувачів, щоб вказувати спеціальні характеристики, такі як сповіщення про з&apos;явлення в мережі та знаходження в списку видимості</translation>
    </message>
    <message>
        <source>Use System Background Color</source>
        <translation>Використовувати типове тло</translation>
    </message>
    <message>
        <source>Proxy Type:</source>
        <translation>Тип проксі сервера:</translation>
    </message>
    <message>
        <source>Auto N/A:</source>
        <translation>Авто не наявний:</translation>
    </message>
    <message>
        <source>Themed Icon</source>
        <translation>Піктограма з темою</translation>
    </message>
    <message>
        <source>Auto Close Function Window</source>
        <translation>Авто-закриття вікон</translation>
    </message>
    <message>
        <source>Perform OnEvent command in away mode</source>
        <translation>Виконувати команди на події у стані &quot;Відійшов&quot;</translation>
    </message>
    <message>
        <source>Docking</source>
        <translation>Використовувати док</translation>
    </message>
    <message>
        <source>Determines if web panel messages are ignored or not.</source>
        <translation>Визначає ігнорувати чи ні повідомлення, відіслані через панель Тенет.</translation>
    </message>
    <message>
        <source>Enable running of &quot;Command&quot; when the relevant event occurs.</source>
        <translation>Ввімкнути виконання команди, коли відбувається відповідна подія.</translation>
    </message>
    <message>
        <source>System Msg:</source>
        <translation>Сист. повідомлення:</translation>
    </message>
    <message>
        <source>Show group name if no messages</source>
        <translation>Показувати назву групи
коли нема повідомлень</translation>
    </message>
    <message>
        <source>Free for Chat</source>
        <translation>Вільний для розмови</translation>
    </message>
    <message>
        <source>Free For Chat</source>
        <translation>Вільний для розмови</translation>
    </message>
    <message>
        <source>When double-clicking on a user to send a message check for urls/files in the clipboard</source>
        <translation>Перевіряти буфер обміну на наявність URL або файлів, коли користувач намагається відіслати повідомлення.</translation>
    </message>
    <message>
        <source>Show all encodings</source>
        <translation>Відображати всі кодування</translation>
    </message>
    <message>
        <source>Online Notify when Logging On</source>
        <translation>Сповіщати при з&apos;явленні в мережі</translation>
    </message>
    <message>
        <source>Popup info</source>
        <translation>Спливаюча інформація</translation>
    </message>
    <message>
        <source>Cellular</source>
        <translation>Мобільний</translation>
    </message>
    <message>
        <source>Extensions</source>
        <translation>Розширення</translation>
    </message>
    <message>
        <source>Do Not Disturb</source>
        <translation>Не турбувати</translation>
    </message>
    <message>
        <source>All incoming events will blink</source>
        <translation>Піктограми всіх подій, що надійшли, будуть блимати</translation>
    </message>
    <message>
        <source>Username:</source>
        <translation>Ім&apos;я користувача:</translation>
    </message>
    <message>
        <source>Url Viewer:</source>
        <translation>Переглядач URL-ів:</translation>
    </message>
    <message>
        <source>Message:</source>
        <translation>Повідомлення:</translation>
    </message>
    <message>
        <source>Draw boxes around each square in the user list</source>
        <translation>Визначає малювати чи ні рамку навколо кожного користувача у списку</translation>
    </message>
    <message>
        <source>Sets the default behavior for auto closing the user function window after a succesful event</source>
        <translation type="obsolete">Встановлює типову поведінку для автоматичного закриття вікон, якщо не виникло ніяких помилок.</translation>
    </message>
    <message>
        <source>Show all available encodings in the User Encoding selection menu. Normally, this menu shows only commonly used encodings.</source>
        <translation>У випадаючому меню з кодуваннями будуть відображені всі кодування, що наявні в системі. Звичайно, в цьому меню знаходяться тільки ті кодування, що вживаються найчастіше.</translation>
    </message>
    <message>
        <source>Column Configuration</source>
        <translation>Конфігурація стовпчиків</translation>
    </message>
    <message>
        <source>Invisible</source>
        <translation>Невидимий</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Загальні</translation>
    </message>
    <message>
        <source>Ignore Email Pager</source>
        <translation>Ігнорувати Email-пейджер</translation>
    </message>
    <message>
        <source>Chatmode Messageview</source>
        <translation>Перегляд повідомлень у вікні розмови</translation>
    </message>
    <message>
        <source>Controls whether or not the dockable icon should be displayed.</source>
        <translation>Контролює наявність піктограми у доку</translation>
    </message>
    <message>
        <source>Sort Online Users by Status</source>
        <translation>Упорядковувати користувачів,
що в мережі, за станом</translation>
    </message>
    <message>
        <source>Proxy Server:</source>
        <translation>Проксі сервер:</translation>
    </message>
    <message>
        <source>Contact List</source>
        <translation>Список контактів</translation>
    </message>
    <message>
        <source>System default (%1)</source>
        <translation>Системне кодування (%1)</translation>
    </message>
    <message>
        <source>The width of the column</source>
        <translation>Ширина стовпчика</translation>
    </message>
    <message>
        <source>Previous Message</source>
        <translation>Попереднє повідомлення</translation>
    </message>
    <message>
        <source>Sort all online users by their actual status</source>
        <translation>Упорядковує всіх користувачів, що в мережі, за їх фактичним станом</translation>
    </message>
    <message>
        <source>Select a font from the system list</source>
        <translation>Виберіть шрифт з списку</translation>
    </message>
    <message>
        <source>The string which will appear in the list box column header</source>
        <translation>Текст, який буде надруковано у заголовку стовпчиків</translation>
    </message>
    <message>
        <source>Store your contacts on the server so they are accessible from different locations and/or programs</source>
        <translation>Список ваших контактів буде зберігатися на сервері, це надасть можливість працювати зі списком контактів з різних місць та/або програм.</translation>
    </message>
    <message>
        <source>Parameter for received URLs</source>
        <translation>Параметри для URL, було отримано</translation>
    </message>
    <message>
        <source>The command to run to view a URL.  Will be passed the URL as a parameter.</source>
        <translation>Команда для перегляду URL. URL буде передано як параметр.</translation>
    </message>
    <message>
        <source>Allow the vertical scroll bar in the user list</source>
        <translation>Дозволити вертикальну панель прокрутки у списку користувачів</translation>
    </message>
    <message>
        <source>The command to run to start your terminal program.</source>
        <translation>Команда, що стартує вашу програму термінала.</translation>
    </message>
    <message>
        <source>Font used in message editor etc.</source>
        <translation>Шрифт, що використовується у редакторі повідомлень та ін.</translation>
    </message>
    <message>
        <source>Ignore New Users</source>
        <translation>Ігнорувати нових користувачів</translation>
    </message>
    <message>
        <source>Use server side contact list</source>
        <translation>Вживати віддалений список контактів</translation>
    </message>
    <message>
        <source>Alignment</source>
        <translation>Вирівнювання</translation>
    </message>
    <message>
        <source>Auto close the user function window after a succesful event</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Open all incoming messages automatically when received if we are online (or free for chat)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Raise the main window on incoming messages</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Start main window hidden. Only the dock icon will be visible.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Show the message info label in bold font if there are incoming messages</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>If not checked, a user will be automatically removed from &quot;New User&quot; group when you first send an event to them</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Automatically send messages through the server if direct connection fails</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Hotkey pops up the next pending message
Enter the hotkey literally, like &quot;shift+f10&quot;, &quot;none&quot; for disabling
changes here require a Restart to take effect!
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Idle Time</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>PasswordDlg</name>
    <message>
        <source>error</source>
        <translation>помилка</translation>
    </message>
    <message>
        <source>Set Password [Setting...</source>
        <translation>Пароль ICQ [Встановлюється...</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>час очікування було перевищено</translation>
    </message>
    <message>
        <source>Verify your ICQ password here.</source>
        <translation>Введіть тут свій пароль ICQ для перевірки.</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>невдача</translation>
    </message>
    <message>
        <source>Passwords do not match, try again.</source>
        <translation>Паролі не збігаються, спробуйте ще раз.</translation>
    </message>
    <message>
        <source>Invalid password, must be 8 characters or less.</source>
        <translation>Пароль повинен бути не більше 8 символів.</translation>
    </message>
    <message>
        <source>Password</source>
        <translation>Пароль</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасувати</translation>
    </message>
    <message>
        <source>&amp;Password:</source>
        <translation>&amp;Пароль:</translation>
    </message>
    <message>
        <source>&amp;Verify:</source>
        <translation>&amp;Ще раз:</translation>
    </message>
    <message>
        <source>Set Password</source>
        <translation>Встановити пароль ICQ</translation>
    </message>
    <message>
        <source>ICQ Password [Setting...]</source>
        <translation>Пароль ICQ [Встановлюється...]</translation>
    </message>
    <message>
        <source>Enter your ICQ password here.</source>
        <translation>Введіть тут ваш пароль ICQ.</translation>
    </message>
    <message>
        <source>Licq - Set Password</source>
        <translation>Licq - Встановлення пароля</translation>
    </message>
    <message>
        <source>&amp;Update</source>
        <translation type="unfinished">&amp;Поновити</translation>
    </message>
</context>
<context>
    <name>PluginDlg</name>
    <message>
        <source>Id</source>
        <translation>Id</translation>
    </message>
    <message>
        <source>Done</source>
        <translation>Завершено</translation>
    </message>
    <message>
        <source>Load</source>
        <translation>Завантажити</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Ім&apos;я</translation>
    </message>
    <message>
        <source>Plugin %1 has no configuration file</source>
        <translation>Додаток %1 не використовує конфігураційний файл</translation>
    </message>
    <message>
        <source>Enable</source>
        <translation>Ввімкнути</translation>
    </message>
    <message>
        <source>Configure</source>
        <translation>Налаштування</translation>
    </message>
    <message>
        <source>Loaded</source>
        <translation>Завантажені</translation>
    </message>
    <message>
        <source>Status</source>
        <translation>Стан</translation>
    </message>
    <message>
        <source>Unload</source>
        <translation>Вивантажити</translation>
    </message>
    <message>
        <source>Licq Plugin %1 %2
</source>
        <translation>Додаток Licq %1 %2
</translation>
    </message>
    <message>
        <source>Available</source>
        <translation>Наявні</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Поновити</translation>
    </message>
    <message>
        <source>Description</source>
        <translation>Опис</translation>
    </message>
    <message>
        <source>Details</source>
        <translation>Подробиці</translation>
    </message>
    <message>
        <source>Disable</source>
        <translation>Вимкнути</translation>
    </message>
    <message>
        <source>Version</source>
        <translation>Версія</translation>
    </message>
</context>
<context>
    <name>QFileDialog</name>
    <message>
        <source>OK</source>
        <translation type="obsolete">Так</translation>
    </message>
    <message>
        <source>&amp;No</source>
        <translation type="obsolete">&amp;Hi</translation>
    </message>
    <message>
        <source>Dir</source>
        <translation type="obsolete">Каталог</translation>
    </message>
    <message>
        <source>&amp;Yes</source>
        <translation type="obsolete">&amp;Так</translation>
    </message>
    <message>
        <source>Back</source>
        <translation type="obsolete">Назад</translation>
    </message>
    <message>
        <source>Date</source>
        <translation type="obsolete">Дата</translation>
    </message>
    <message>
        <source>File</source>
        <translation type="obsolete">Файл</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="obsolete">Ім&apos;я</translation>
    </message>
    <message>
        <source>Open</source>
        <translation type="obsolete">Відкрити</translation>
    </message>
    <message>
        <source>Save</source>
        <translation type="obsolete">Зберегти</translation>
    </message>
    <message>
        <source>Size</source>
        <translation type="obsolete">Розмір</translation>
    </message>
    <message>
        <source>Sort</source>
        <translation type="obsolete">Упорядкувати</translation>
    </message>
    <message>
        <source>Type</source>
        <translation type="obsolete">Тип</translation>
    </message>
    <message>
        <source>&amp;Open</source>
        <translation type="obsolete">&amp;Відкрити</translation>
    </message>
    <message>
        <source>&amp;Save</source>
        <translation type="obsolete">&amp;Зберегти</translation>
    </message>
    <message>
        <source>ERROR</source>
        <translation type="obsolete">ПОМИЛКА</translation>
    </message>
    <message>
        <source>Error</source>
        <translation type="obsolete">Помилка</translation>
    </message>
    <message>
        <source>Sort by &amp;Size</source>
        <translation type="obsolete">Упорядкувати за &amp;розміром</translation>
    </message>
    <message>
        <source>Sort by &amp;Date</source>
        <translation type="obsolete">Упорядкувати за &amp;датою</translation>
    </message>
    <message>
        <source>Sort by &amp;Name</source>
        <translation type="obsolete">Упорядкувати за &amp;ім&apos;ям</translation>
    </message>
    <message>
        <source>New Folder 1</source>
        <translation type="obsolete">Новий каталог 1</translation>
    </message>
    <message>
        <source>&lt;qt&gt;Do you really want to delete %1 &quot;%2&quot;?&lt;/qt&gt;</source>
        <translation type="obsolete">&lt;qt&gt;Ви дійсно хочете видалити %1 &quot;%2&quot;?&lt;/qt&gt;</translation>
    </message>
    <message>
        <source>the directory</source>
        <translation type="obsolete">каталог</translation>
    </message>
    <message>
        <source>File &amp;type:</source>
        <translation type="obsolete">&amp;Тип файлу:</translation>
    </message>
    <message>
        <source>File &amp;name:</source>
        <translation type="obsolete">&amp;Ім&apos;я файлу:</translation>
    </message>
    <message>
        <source>Delete %1</source>
        <translation type="obsolete">Видалити %1</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation type="obsolete">Скасувати</translation>
    </message>
    <message>
        <source>R&amp;eload</source>
        <translation type="obsolete">П&amp;еревантажити</translation>
    </message>
    <message>
        <source>New Folder</source>
        <translation type="obsolete">Новий каталог</translation>
    </message>
    <message>
        <source>&amp;Unsorted</source>
        <translation type="obsolete">&amp;Не впорядковувати</translation>
    </message>
    <message>
        <source>Look &amp;in:</source>
        <translation type="obsolete">Д&amp;ивитися в:</translation>
    </message>
    <message>
        <source>Preview File Contents</source>
        <translation type="obsolete">Переглянути зміст файлу</translation>
    </message>
    <message>
        <source>New Folder %1</source>
        <translation type="obsolete">Новий каталог %1</translation>
    </message>
    <message>
        <source>Read-write</source>
        <translation type="obsolete">Для читання та запису</translation>
    </message>
    <message>
        <source>Read-only</source>
        <translation type="obsolete">Тільки для читання</translation>
    </message>
    <message>
        <source>Find Directory</source>
        <translation type="obsolete">Знайти каталог</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation type="obsolete">Атрибути</translation>
    </message>
    <message>
        <source>Show &amp;hidden files</source>
        <translation type="obsolete">Показувати &amp;сховані файли</translation>
    </message>
    <message>
        <source>Save as</source>
        <translation type="obsolete">Зберегти як</translation>
    </message>
    <message>
        <source>Inaccessible</source>
        <translation type="obsolete">Недоступний</translation>
    </message>
    <message>
        <source>%1
File not found.
Check path and filename.</source>
        <translation type="obsolete">%1
Файл не знайдено.
Перевірте, будь ласка, шлях та ім&apos;я файлу.</translation>
    </message>
    <message>
        <source>List View</source>
        <translation type="obsolete">Листовий вигляд</translation>
    </message>
    <message>
        <source>Special</source>
        <translation type="obsolete">Спеціальний файл</translation>
    </message>
    <message>
        <source>Write-only</source>
        <translation type="obsolete">Тільки для запису</translation>
    </message>
    <message>
        <source>the symlink</source>
        <translation type="obsolete">символічне посилання</translation>
    </message>
    <message>
        <source>&amp;Delete</source>
        <translation type="obsolete">В&amp;идалити</translation>
    </message>
    <message>
        <source>All files (*)</source>
        <translation type="obsolete">Всі файли (*)</translation>
    </message>
    <message>
        <source>Directories</source>
        <translation type="obsolete">Каталоги</translation>
    </message>
    <message>
        <source>Symlink to Special</source>
        <translation type="obsolete">Посилання до спеціального файлу</translation>
    </message>
    <message>
        <source>&amp;Rename</source>
        <translation type="obsolete">&amp;Перейменувати</translation>
    </message>
    <message>
        <source>One directory up</source>
        <translation type="obsolete">Одним каталогом вище</translation>
    </message>
    <message>
        <source>Preview File Info</source>
        <translation type="obsolete">Переглянути інформацію про файл</translation>
    </message>
    <message>
        <source>the file</source>
        <translation type="obsolete">файл</translation>
    </message>
    <message>
        <source>Create New Folder</source>
        <translation type="obsolete">Створити нову папку</translation>
    </message>
    <message>
        <source>Symlink to File</source>
        <translation type="obsolete">Посилання до файлу</translation>
    </message>
    <message>
        <source>Symlink to Directory</source>
        <translation type="obsolete">Посилання до каталогу</translation>
    </message>
    <message>
        <source>Detail View</source>
        <translation type="obsolete">Детальний вигляд</translation>
    </message>
</context>
<context>
    <name>QFontDialog</name>
    <message>
        <source>OK</source>
        <translation type="obsolete">Гаразд</translation>
    </message>
    <message>
        <source>&amp;Font</source>
        <translation type="obsolete">&amp;Шрифт</translation>
    </message>
    <message>
        <source>&amp;Size</source>
        <translation type="obsolete">&amp;Розмір</translation>
    </message>
    <message>
        <source>Apply</source>
        <translation type="obsolete">Застосувати</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="obsolete">Закрити</translation>
    </message>
    <message>
        <source>&amp;Color</source>
        <translation type="obsolete">&amp;Колір</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation type="obsolete">Скасувати</translation>
    </message>
    <message>
        <source>Sample</source>
        <translation type="obsolete">Приклад</translation>
    </message>
    <message>
        <source>Font st&amp;yle</source>
        <translation type="obsolete">&amp;Стиль шрифту</translation>
    </message>
    <message>
        <source>Scr&amp;ipt</source>
        <translation type="obsolete">&amp;Скрипт</translation>
    </message>
    <message>
        <source>Select Font</source>
        <translation type="obsolete">Виберіть шрифт</translation>
    </message>
    <message>
        <source>&amp;Underline</source>
        <translation type="obsolete">П&amp;ідкреслити</translation>
    </message>
    <message>
        <source>Effects</source>
        <translation type="obsolete">Ефекти</translation>
    </message>
    <message>
        <source>Stri&amp;keout</source>
        <translation type="obsolete">Пере&amp;креслити</translation>
    </message>
</context>
<context>
    <name>QMessageBox</name>
    <message>
        <source>Licq Error</source>
        <translation>Помилка Licq</translation>
    </message>
    <message>
        <source>Licq Warning</source>
        <translation>Попередження Licq</translation>
    </message>
    <message>
        <source>Licq Question</source>
        <translation>Запитання Licq</translation>
    </message>
    <message>
        <source>Licq Information</source>
        <translation>Інформація Licq</translation>
    </message>
</context>
<context>
    <name>QString</name>
    <message>
        <source>Failed to open file:
%1</source>
        <translation type="obsolete">Не можу відкрити файл:
%1</translation>
    </message>
</context>
<context>
    <name>QWizard</name>
    <message>
        <source>Help</source>
        <translation type="obsolete">Довідка</translation>
    </message>
    <message>
        <source>&lt; Back</source>
        <translation type="obsolete">&lt; Назад</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation type="obsolete">Скасувати</translation>
    </message>
    <message>
        <source>Finish</source>
        <translation type="obsolete">Завершити</translation>
    </message>
    <message>
        <source>Next &gt;</source>
        <translation type="obsolete">Уперед &gt;</translation>
    </message>
</context>
<context>
    <name>RegisterUserDlg</name>
    <message>
        <source>Uin:</source>
        <translation>Ідентифікаційний номер:</translation>
    </message>
    <message>
        <source>Please enter your password in both input fields.</source>
        <translation>Введіть, будь ласка, пароль в обидва поля вводу.</translation>
    </message>
    <message>
        <source>Registered succesfully.  Now log on and update your personal info.</source>
        <translation>Реєстрація вдалась. Зараз зайдіть до мережі та поновити інформацію про себе.</translation>
    </message>
    <message>
        <source>You need to enter a valid UIN when you try to register an existing user. </source>
        <translation>Ви маєте ввести вірний ідентифікаційний номер, якщо Ви бажаєте зареєструвати вже існуючого користувача. </translation>
    </message>
    <message>
        <source>Now please click &apos;Finish&apos; to start the registration process.</source>
        <translation>Зараз натисніть &apos;Завершити&apos;, щоб почати процес реєстрації.</translation>
    </message>
    <message>
        <source>User Registration in Progress...</source>
        <translation>Реєстрація користувача...</translation>
    </message>
    <message>
        <source>Now please press the &apos;Back&apos; button and try again.</source>
        <translation>Зараз натисніть &apos;Назад&apos; та спробуйте ще раз.</translation>
    </message>
    <message>
        <source>&amp;Register Existing User</source>
        <translation>&amp;Зареєструвати вже існуючого користувача</translation>
    </message>
    <message>
        <source>The passwords don&apos;t seem to match.</source>
        <translation>Паролі не співпадають.</translation>
    </message>
    <message>
        <source>UIN Registration - Step 2</source>
        <translation>Реєстрація ідентифікаційного номера - Крок 2</translation>
    </message>
    <message>
        <source>UIN Registration - Step 3</source>
        <translation>Реєстрація ідентифікаційного номера - Крок 3</translation>
    </message>
    <message>
        <source>Password:</source>
        <translation>Пароль:</translation>
    </message>
    <message>
        <source>Invalid UIN.  Try again.</source>
        <translation>Невірний ідентифікаційний номер. Спробуйте ще раз.</translation>
    </message>
    <message>
        <source>Verify:</source>
        <translation>Ще раз:</translation>
    </message>
    <message>
        <source>Licq User Registration</source>
        <translation>Licq :: Реєстрація користувача</translation>
    </message>
    <message>
        <source>Welcome to the Registration Wizard.

You can register a new user here, or configure Licq to use an existing UIN.

If you are registering a new uin, choose a password and click &quot;Finish&quot;.
If you already have a uin, then toggle &quot;Register Existing User&quot;,
enter your uin and your password, and click &quot;OK&quot;

Press &quot;Next&quot; to proceed.</source>
        <translation>Ласкаво просимо до майстра реєстрації.

Тут ви можете зареєструвати нового користувача або
налаштувати Licq для використання вже зареєстрованого користувача.

Виберіть та введіть пароль та натисніть &quot;Закінчити&quot;, якщо Ви реєструєте нового користувача.
А якщо Ви вже маєте ідентифікаційний номер, то ввімкніть &quot;Зареєструвати вже існуючого користувача&quot;,
введіть ваш номер, пароль та натисніть &quot;Гаразд&quot;

Натисніть &quot;Наступний&quot;, щоб продовжити.</translation>
    </message>
    <message>
        <source>UIN Registration</source>
        <translation>Реєстрація ідентифікаційного номера</translation>
    </message>
</context>
<context>
    <name>ReqAuthDlg</name>
    <message>
        <source>Licq - Request Authorization</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Request authorization from (UIN):</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Request</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Ok</source>
        <translation type="unfinished">&amp;Гаразд</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation type="unfinished">&amp;Скасувати</translation>
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
        <translation>Чоловіча</translation>
    </message>
    <message>
        <source>Reset Search</source>
        <translation>Очистити все</translation>
    </message>
    <message>
        <source>&amp;Done</source>
        <translation>&amp;Закрити</translation>
    </message>
    <message>
        <source>&amp;Uin#</source>
        <translation>За &amp;ідентифікаційним номером</translation>
    </message>
    <message>
        <source>City:</source>
        <translation>Місто:</translation>
    </message>
    <message>
        <source>UIN#:</source>
        <translation>Ідентифікаційний номер:</translation>
    </message>
    <message>
        <source>Company Name:</source>
        <translation>Назва компанії:</translation>
    </message>
    <message>
        <source>Enter search parameters and select &apos;Search&apos;</source>
        <translation>Введіть параметри пошуку та натисніть &apos;Пошук&apos;</translation>
    </message>
    <message>
        <source>Company Position:</source>
        <translation>Посада:</translation>
    </message>
    <message>
        <source>Searching (this can take awhile)...</source>
        <translation>Шукаю (це може зайняти деякий час)...</translation>
    </message>
    <message>
        <source>%1 more users found. Narrow search.</source>
        <translation>Знайдено %1 користувачів. Обмежте пошук.</translation>
    </message>
    <message>
        <source>More users found. Narrow search.</source>
        <translation>Знайдено занадто багато користувачів. Обмежить пошук.</translation>
    </message>
    <message>
        <source>Alias:</source>
        <translation>Прізвисько:</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Скасувати</translation>
    </message>
    <message>
        <source>18 - 22</source>
        <translation>18 - 22</translation>
    </message>
    <message>
        <source>Female</source>
        <translation>Жіноча</translation>
    </message>
    <message>
        <source>Language:</source>
        <translation>Мова:</translation>
    </message>
    <message>
        <source>23 - 29</source>
        <translation>23 - 29</translation>
    </message>
    <message>
        <source>Age Range:</source>
        <translation>Вік:</translation>
    </message>
    <message>
        <source>State:</source>
        <translation>Штат:</translation>
    </message>
    <message>
        <source>First Name:</source>
        <translation>Ім&apos;я:</translation>
    </message>
    <message>
        <source>30 - 39</source>
        <translation>30 - 39</translation>
    </message>
    <message>
        <source>Last Name:</source>
        <translation>Прізвище:</translation>
    </message>
    <message>
        <source>Country:</source>
        <translation>Країна:</translation>
    </message>
    <message>
        <source>40 - 49</source>
        <translation>40 - 49</translation>
    </message>
    <message>
        <source>Search failed.</source>
        <translation>Пошук невдалий.</translation>
    </message>
    <message>
        <source>50 - 59</source>
        <translation>50 - 59</translation>
    </message>
    <message>
        <source>Company Department:</source>
        <translation>Відділ компанії:</translation>
    </message>
    <message>
        <source>Return Online Users Only</source>
        <translation>Шукати тільки серед користувачів, які зараз в мережі</translation>
    </message>
    <message>
        <source>A&amp;lert User</source>
        <translation>&amp;Повідомити користувача</translation>
    </message>
    <message>
        <source>&amp;Add %1 Users</source>
        <translation>&amp;Додати (%1)</translation>
    </message>
    <message>
        <source>Email Address:</source>
        <translation>Адреса електронної пошти:</translation>
    </message>
    <message>
        <source>&amp;Add User</source>
        <translation>&amp;Додати користувача</translation>
    </message>
    <message>
        <source>&amp;Whitepages</source>
        <translation>За &amp;параметрами</translation>
    </message>
    <message>
        <source>&amp;Search</source>
        <translation>&amp;Шукати</translation>
    </message>
    <message>
        <source>Unspecified</source>
        <translation>Не вказано</translation>
    </message>
    <message>
        <source>Keyword:</source>
        <translation>Ключове слово:</translation>
    </message>
    <message>
        <source>Gender:</source>
        <translation>Стать:</translation>
    </message>
    <message>
        <source>Licq - User Search</source>
        <translation>Licq - Пошук користувачів</translation>
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
        <translation>Ж</translation>
    </message>
    <message>
        <source>M</source>
        <translation>Ч</translation>
    </message>
    <message>
        <source>UIN</source>
        <translation>Ідентифікаційний номер</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Ім&apos;я</translation>
    </message>
    <message>
        <source>Alias</source>
        <translation>Прізвисько</translation>
    </message>
    <message>
        <source>Email</source>
        <translation>Електронна пошта</translation>
    </message>
    <message>
        <source>Online</source>
        <translation>В мережі</translation>
    </message>
    <message>
        <source>Status</source>
        <translation>Стан</translation>
    </message>
    <message>
        <source>Offline</source>
        <translation>Не в мережі</translation>
    </message>
    <message>
        <source>Sex &amp; Age</source>
        <translation>Стать/вік</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation>Невідомий</translation>
    </message>
    <message>
        <source>Authorize</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No</source>
        <translation type="unfinished">Ні</translation>
    </message>
    <message>
        <source>Yes</source>
        <translation type="unfinished">Так</translation>
    </message>
</context>
<context>
    <name>SecurityDlg</name>
    <message>
        <source>error</source>
        <translation>помилка</translation>
    </message>
    <message>
        <source>ICQ Security Options [Setting...]</source>
        <translation>Параметри безпеки ICQ [Встановлюються...]</translation>
    </message>
    <message>
        <source>ICQ Security Options</source>
        <translation>Параметри безпеки ICQ</translation>
    </message>
    <message>
        <source>You need to be connected to the
ICQ Network to change the settings.</source>
        <translation>Ви повинні ввійти до мережі ICQ,
щоб мати можливість змінити параметри.</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>таймаут</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>Параметри</translation>
    </message>
    <message>
        <source>Web Presence allows users to see if you are online through your web indicator.</source>
        <translation>Наявність через Тенет дозволяє користувачам бачити, що Ви в мережі, через індикатор Тенет.</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>невдача</translation>
    </message>
    <message>
        <source>Authorization Required</source>
        <translation>Вимагати авторизацію</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасувати</translation>
    </message>
    <message>
        <source>&amp;Update</source>
        <translation>&amp;Поновити</translation>
    </message>
    <message>
        <source>Hide IP stops users from seeing your IP address. It doesn&apos;t guarantee it will be hidden though.</source>
        <translation>Приховування IP не дає користувачам бачити вашу IP адресу. Але немає ніякої, що вашу IP адресу не визначать якимось іншим чином.</translation>
    </message>
    <message>
        <source>Web Presence</source>
        <translation>Наявність через Тенет</translation>
    </message>
    <message>
        <source>ICQ Security Options [Setting...</source>
        <translation>Параметри безпеки ICQ [Встановлюються...</translation>
    </message>
    <message>
        <source>Determines whether regular ICQ clients require your authorization to add you to their contact list.</source>
        <translation>Визначає, чи будуть звичайні ICQ-клієнти питати про Вашу згоду, щоб додати Вас до своїх списків контактів.</translation>
    </message>
    <message>
        <source>Hide IP</source>
        <translation>Приховувати IP адресу</translation>
    </message>
    <message>
        <source>Password/UIN settings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Uin:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Enter the UIN which you want to use.  Only available if &quot;Local changes only&quot; ischecked.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Password:</source>
        <translation type="unfinished">&amp;Пароль:</translation>
    </message>
    <message>
        <source>Enter your ICQ password here.</source>
        <translation type="unfinished">Введіть тут ваш пароль ICQ.</translation>
    </message>
    <message>
        <source>&amp;Verify</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Verify your ICQ password here.</source>
        <translation type="unfinished">Введіть тут свій пароль ICQ для перевірки.</translation>
    </message>
    <message>
        <source>&amp;Local changes only</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>If checked, password/UIN changes will apply only on your local computer.  Useful if your password is incorrectly saved in Licq.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Invalid password, must be between 1 and 8 characters.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Passwords do not match, try again.</source>
        <translation type="unfinished">Паролі не збігаються, спробуйте ще раз.</translation>
    </message>
    <message>
        <source>Setting security options failed.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Changing password failed.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Timeout while setting security options.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Timeout while changing password.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Internal error while setting security options.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Internal error while changing password.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ShowAwayMsgDlg</name>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;Гаразд</translation>
    </message>
    <message>
        <source>error</source>
        <translation>помилка</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>таймаут</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>невдача</translation>
    </message>
    <message>
        <source>&amp;Show Again</source>
        <translation>&amp;Показати знову</translation>
    </message>
    <message>
        <source>%1 Response for %2</source>
        <translation>&apos;%1&apos; відповідь для &apos;%2&apos;</translation>
    </message>
</context>
<context>
    <name>SkinBrowserDlg</name>
    <message>
        <source>Name</source>
        <translation>Ім&apos;я</translation>
    </message>
    <message>
        <source>&amp;Done</source>
        <translation>&amp;Закрити</translation>
    </message>
    <message>
        <source>&amp;Edit</source>
        <translation>&amp;Редагувати</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Помилка</translation>
    </message>
    <message>
        <source>Icons</source>
        <translation>Піктограми</translation>
    </message>
    <message>
        <source>Skins</source>
        <translation>Вигляд</translation>
    </message>
    <message>
        <source>&amp;Apply</source>
        <translation>&amp;Застосувати</translation>
    </message>
    <message>
        <source>A&amp;pply</source>
        <translation>З&amp;астосувати</translation>
    </message>
    <message>
        <source>Licq Skin Browser</source>
        <translation>Вибір зовнішнього вигляду Licq</translation>
    </message>
</context>
<context>
    <name>UserCodec</name>
    <message>
        <source>Unicode</source>
        <translation type="unfinished">Юнікод</translation>
    </message>
    <message>
        <source>Arabic</source>
        <translation type="unfinished">Арабське</translation>
    </message>
    <message>
        <source>Baltic</source>
        <translation type="unfinished">Балтійське</translation>
    </message>
    <message>
        <source>Central European</source>
        <translation type="unfinished">Центральноєвропейське</translation>
    </message>
    <message>
        <source>Chinese</source>
        <translation type="unfinished">Китайське</translation>
    </message>
    <message>
        <source>Chinese Traditional</source>
        <translation type="unfinished">Традиційне китайське</translation>
    </message>
    <message>
        <source>Cyrillic</source>
        <translation type="unfinished">Кирилиця</translation>
    </message>
    <message>
        <source>Esperanto</source>
        <translation type="unfinished">Есперанто</translation>
    </message>
    <message>
        <source>Greek</source>
        <translation type="unfinished">Грецьке</translation>
    </message>
    <message>
        <source>Hebrew</source>
        <translation type="unfinished">Єврейське</translation>
    </message>
    <message>
        <source>Japanese</source>
        <translation type="unfinished">Японське</translation>
    </message>
    <message>
        <source>Korean</source>
        <translation type="unfinished">Корейське</translation>
    </message>
    <message>
        <source>Western European</source>
        <translation type="unfinished">Західноєвропейське</translation>
    </message>
    <message>
        <source>Tamil</source>
        <translation type="unfinished">Тамільське</translation>
    </message>
    <message>
        <source>Thai</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Turkish</source>
        <translation type="unfinished">Турецьке</translation>
    </message>
    <message>
        <source>Ukrainian</source>
        <translation type="unfinished">Українське</translation>
    </message>
</context>
<context>
    <name>UserEventCommon</name>
    <message>
        <source>Time:</source>
        <translation>Час:</translation>
    </message>
    <message>
        <source>Change user text encoding</source>
        <translation>Змінити кодування користувача</translation>
    </message>
    <message>
        <source>This button selects the text encoding used when communicating with this user. You might need to change the encoding to communicate in a different language.</source>
        <translation>За допомогою цієї кнопки встановлюється кодування, яке буде використовуватись при спілкуванні з цим користувачем. Можливо вам потрібно буде змінити кодування, щоб спілкуватись на інший мові.</translation>
    </message>
    <message>
        <source>Show User Info</source>
        <translation>Показати інформацію про користувача</translation>
    </message>
    <message>
        <source>Secure channel information</source>
        <translation type="obsolete">Інформація про параметри безпеки</translation>
    </message>
    <message>
        <source>Unable to load encoding &lt;b&gt;%1&lt;/b&gt;. Message contents may appear garbled.</source>
        <translation>Не вдається завантажити кодування &lt;b&gt;%1&lt;/b&gt;. Зміст повідомлення може бути перекрученим.</translation>
    </message>
    <message>
        <source>Status:</source>
        <translation>Стан:</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation>Невідомо</translation>
    </message>
    <message>
        <source>Show User History</source>
        <translation>Показати архів повідомлень</translation>
    </message>
    <message>
        <source>Open / Close secure channel</source>
        <translation type="unfinished"></translation>
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
        <translation>Зараз</translation>
    </message>
    <message>
        <source>Age:</source>
        <translation>Вік:</translation>
    </message>
    <message>
        <source>Fax:</source>
        <translation>Факс:</translation>
    </message>
    <message>
        <source>Male</source>
        <translation>Чоловіча</translation>
    </message>
    <message>
        <source>UIN:</source>
        <translation>Ідентифікаційний номер:</translation>
    </message>
    <message>
        <source>Zip:</source>
        <translation>Індекс:</translation>
    </message>
    <message>
        <source>done</source>
        <translation>виконано</translation>
    </message>
    <message>
        <source> Day:</source>
        <translation> День:</translation>
    </message>
    <message>
        <source>&amp;Last</source>
        <translation>&amp;Останні події</translation>
    </message>
    <message>
        <source>&amp;Menu</source>
        <translation>&amp;Меню</translation>
    </message>
    <message>
        <source>&amp;More</source>
        <translation>&amp;Додаткова</translation>
    </message>
    <message>
        <source>&amp;Save</source>
        <translation>&amp;Зберегти</translation>
    </message>
    <message>
        <source>&amp;Work</source>
        <translation>Про робо&amp;ту</translation>
    </message>
    <message>
        <source>&amp;Filter: </source>
        <translation>&amp;Фільтр: </translation>
    </message>
    <message>
        <source>City:</source>
        <translation>Місто:</translation>
    </message>
    <message>
        <source>P&amp;rev</source>
        <translation>&amp;Попередній</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation>Ім&apos;я:</translation>
    </message>
    <message>
        <source>Nex&amp;t</source>
        <translation>&amp;Наступний</translation>
    </message>
    <message>
        <source>error</source>
        <translation>помилка</translation>
    </message>
    <message>
        <source>Last Online:</source>
        <translation>В мережі:</translation>
    </message>
    <message>
        <source>Updating server...</source>
        <translation>Поновлення інформації на сервері...</translation>
    </message>
    <message>
        <source> Year:</source>
        <translation> Рік:</translation>
    </message>
    <message>
        <source>Last Sent Event:</source>
        <translation>Що-небудь відправлено:</translation>
    </message>
    <message>
        <source>Last Checked Auto Response:</source>
        <translation>Остання перевірена автовідповідь:</translation>
    </message>
    <message>
        <source>&amp;About</source>
        <translation>&amp;Про себе</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Закрити</translation>
    </message>
    <message>
        <source>You need to be connected to the
ICQ Network to retrieve your settings.</source>
        <translation>Ви повинні буди в мережі ICQ,
щоб отримати Ваші параметри.</translation>
    </message>
    <message>
        <source>Cellular:</source>
        <translation>Мобільний:</translation>
    </message>
    <message>
        <source>%1 to %2</source>
        <translation>%1 до %2</translation>
    </message>
    <message>
        <source>EMail 1:</source>
        <translation>Основна адреса ел. пошти:</translation>
    </message>
    <message>
        <source>EMail 2:</source>
        <translation>Додаткова адреса ел. пошти:</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>таймаут</translation>
    </message>
    <message>
        <source>Homepage:</source>
        <translation>Домашня сторінка:</translation>
    </message>
    <message>
        <source>Online Since:</source>
        <translation>В мережі з:</translation>
    </message>
    <message>
        <source>Timezone:</source>
        <translation>Часовий пояс:</translation>
    </message>
    <message>
        <source>About:</source>
        <translation>Про себе:</translation>
    </message>
    <message>
        <source>Alias:</source>
        <translation>Прізвисько:</translation>
    </message>
    <message>
        <source>Old Email:</source>
        <translation>Стара адреса ел. пошти:</translation>
    </message>
    <message>
        <source>Female</source>
        <translation>Жіноча</translation>
    </message>
    <message>
        <source> Month:</source>
        <translation> Місяць:</translation>
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
        <translation>Не в мережі</translation>
    </message>
    <message>
        <source>Last Received Event:</source>
        <translation>Що-небудь прийнято:</translation>
    </message>
    <message>
        <source>Authorization Not Required</source>
        <translation>Авторизація не вимагається</translation>
    </message>
    <message>
        <source>[&lt;font color=&quot;%1&quot;&gt;Received&lt;/font&gt;] [&lt;font color=&quot;%2&quot;&gt;Sent&lt;/font&gt;] %3 out of %4 matches</source>
        <translation>[&lt;font color=&quot;%1&quot;&gt;Прийняті&lt;/font&gt;] [&lt;font color=&quot;%2&quot;&gt;Відіслані&lt;/font&gt;] %3 з %4 співпавших</translation>
    </message>
    <message>
        <source>GMT%1%1%1</source>
        <translation>GMT%1%1%1</translation>
    </message>
    <message>
        <source>[&lt;font color=&quot;%1&quot;&gt;Received&lt;/font&gt;] [&lt;font color=&quot;%2&quot;&gt;Sent&lt;/font&gt;] %3 to %4 of %5</source>
        <translation>[&lt;font color=&quot;%1&quot;&gt;Прийняті&lt;/font&gt;] [&lt;font color=&quot;%2&quot;&gt;Відіслані&lt;/font&gt;] %3 - %4 з %5</translation>
    </message>
    <message>
        <source>Country:</source>
        <translation>Країна:</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>невдача</translation>
    </message>
    <message>
        <source>server</source>
        <translation>через сервер</translation>
    </message>
    <message>
        <source>Authorization Required</source>
        <translation>Необхідна авторизація</translation>
    </message>
    <message>
        <source>INVALID USER</source>
        <translation>НЕВІРНИЙ КОРИСТУВАЧ</translation>
    </message>
    <message>
        <source>Birthday:</source>
        <translation>День народження:</translation>
    </message>
    <message>
        <source>Unknown (%1)</source>
        <translation>Невідомий (%1)</translation>
    </message>
    <message>
        <source>Department:</source>
        <translation>Відділ:</translation>
    </message>
    <message>
        <source>Position:</source>
        <translation>Посада:</translation>
    </message>
    <message>
        <source>Sorry, history is disabled for this person.</source>
        <translation>Перепрошую, але для цього користувача архів вимкнено.</translation>
    </message>
    <message>
        <source>Licq - Info </source>
        <translation>Licq - Інформація </translation>
    </message>
    <message>
        <source>Status:</source>
        <translation>Стан:</translation>
    </message>
    <message>
        <source>Address:</source>
        <translation>Адреса:</translation>
    </message>
    <message>
        <source>Updating...</source>
        <translation>Поновлення інформації...</translation>
    </message>
    <message>
        <source>You need to be connected to the
ICQ Network to change your settings.</source>
        <translation>Ви повинні буди в мережі ICQ,
щоб змінити Ваші параметри.</translation>
    </message>
    <message>
        <source>Language 3:</source>
        <translation>Мова, яку знаєте зі словником:</translation>
    </message>
    <message>
        <source>Language 1:</source>
        <translation>Рідна мова:</translation>
    </message>
    <message>
        <source>Language 2:</source>
        <translation>Мова, яку знаєте без словника:</translation>
    </message>
    <message>
        <source>Unspecified</source>
        <translation>Не вказано</translation>
    </message>
    <message>
        <source>&amp;Update</source>
        <translation>&amp;Поновити</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation>Невідомо</translation>
    </message>
    <message>
        <source>Retrieve</source>
        <translation>Отримати</translation>
    </message>
    <message>
        <source>Rever&amp;se</source>
        <translation>&amp;У зворотньому порядку</translation>
    </message>
    <message>
        <source>%1 from %2</source>
        <translation>%1 від %2</translation>
    </message>
    <message>
        <source>Gender:</source>
        <translation>Стать:</translation>
    </message>
    <message>
        <source>&amp;General</source>
        <translation>&amp;Загальна</translation>
    </message>
    <message>
        <source>&amp;History</source>
        <translation>&amp;Архів</translation>
    </message>
    <message>
        <source>Error loading history file: %1
Description: %2</source>
        <translation>Помилка під час завантаження файлу архіву: %1
Опис: %2</translation>
    </message>
</context>
<context>
    <name>UserSendChatEvent</name>
    <message>
        <source>Clear</source>
        <translation>Очистити</translation>
    </message>
    <message>
        <source>Multiparty: </source>
        <translation>Для багатьох користувачів:</translation>
    </message>
    <message>
        <source>Invite</source>
        <translation>Запросити</translation>
    </message>
    <message>
        <source>Chat with %2 refused:
%3</source>
        <translation>В розмові з %2 відмовлено:
%3</translation>
    </message>
    <message>
        <source> - Chat Request</source>
        <translation> - Запит на розмову</translation>
    </message>
</context>
<context>
    <name>UserSendCommon</name>
    <message>
        <source>No</source>
        <translation>Ні</translation>
    </message>
    <message>
        <source>SMS</source>
        <translation>SMS</translation>
    </message>
    <message>
        <source>URL</source>
        <translation>Адреса</translation>
    </message>
    <message>
        <source>Yes</source>
        <translation>Так</translation>
    </message>
    <message>
        <source>done</source>
        <translation>завершено</translation>
    </message>
    <message>
        <source>&amp;Menu</source>
        <translation>&amp;Меню</translation>
    </message>
    <message>
        <source>&amp;Send</source>
        <translation>&amp;Відіслати</translation>
    </message>
    <message>
        <source>error</source>
        <translation>помилка</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Закрити</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>таймаут</translation>
    </message>
    <message>
        <source>Message</source>
        <translation>Повідомлення</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Скасувати</translation>
    </message>
    <message>
        <source>File Transfer</source>
        <translation>Передача файлу</translation>
    </message>
    <message>
        <source>%1 refused %2, send through server.</source>
        <translation>%1 відмовлено %2, відіслати через сервер.</translation>
    </message>
    <message>
        <source>Urgent</source>
        <translation>Термінове</translation>
    </message>
    <message>
        <source>Send anyway</source>
        <translation>Відіслати будь-яким чином</translation>
    </message>
    <message>
        <source>Drag Users Here
Right Click for Options</source>
        <translation>Перетягніть сюди користувачів
Скористуйтесь правою кнопкою для опцій</translation>
    </message>
    <message>
        <source>direct</source>
        <translation>напряму</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>невдача</translation>
    </message>
    <message>
        <source>via server</source>
        <translation>через сервер</translation>
    </message>
    <message>
        <source>U&amp;rgent</source>
        <translation>&amp;Термінове</translation>
    </message>
    <message>
        <source>%1 is in %2 mode:
%3
Send...</source>
        <translation>%1 у стані %2:
%3
Відсилається...</translation>
    </message>
    <message>
        <source>Direct send failed,
send through server?</source>
        <translation>Не вдається відіслати повідомлення напряму,
відіслати через сервер?</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Скасувати</translation>
    </message>
    <message>
        <source>Warning: Message can&apos;t be sent securely
through the server!</source>
        <translation>Попередження: Через сервер неможливо
безпечно відіслати повідомлення!</translation>
    </message>
    <message>
        <source>Sending </source>
        <translation>Відсилається </translation>
    </message>
    <message>
        <source>Chat Request</source>
        <translation>Запит на розмову</translation>
    </message>
    <message>
        <source>Contact List</source>
        <translation>Список контактів</translation>
    </message>
    <message>
        <source> to Contact List</source>
        <translation> до списку контактів</translation>
    </message>
    <message>
        <source>M&amp;ultiple recipients</source>
        <translation>&amp;Багатьом адресатам</translation>
    </message>
    <message>
        <source>Se&amp;nd through server</source>
        <translation>&amp;Відіслати через сервер</translation>
    </message>
</context>
<context>
    <name>UserSendContactEvent</name>
    <message>
        <source>Drag Users Here - Right Click for Options</source>
        <translation>Перетягніть сюди користувачів
Скористуйтесь правою кнопкою для опцій</translation>
    </message>
    <message>
        <source> - Contact List</source>
        <translation> - Список контактів</translation>
    </message>
</context>
<context>
    <name>UserSendFileEvent</name>
    <message>
        <source>You must specify a file to transfer!</source>
        <translation>Ви повинні вказати файл, який хочете відіслати!</translation>
    </message>
    <message>
        <source>File(s): </source>
        <translation>Файл(и): </translation>
    </message>
    <message>
        <source>Browse</source>
        <translation>Перегляд</translation>
    </message>
    <message>
        <source> - File Transfer</source>
        <translation> - Передача файлу</translation>
    </message>
    <message>
        <source>File transfer with %2 refused:
%3</source>
        <translation>У передачі файлу до %2 відмовлено::
%3</translation>
    </message>
    <message>
        <source>Select files to send</source>
        <translation>Виберіть файли, які потрібно відіслати</translation>
    </message>
</context>
<context>
    <name>UserSendMsgEvent</name>
    <message>
        <source>&amp;No</source>
        <translation>&amp;Ні</translation>
    </message>
    <message>
        <source>&amp;Yes</source>
        <translation>&amp;Так</translation>
    </message>
    <message>
        <source> - Message</source>
        <translation> - Повідомлення</translation>
    </message>
    <message>
        <source>You didn&apos;t edit the message.
Do you really want to send it?</source>
        <translation>Ви не відредагували повідомлення.
Ви впевнені, що бажаєте відіслати його?</translation>
    </message>
</context>
<context>
    <name>UserSendSmsEvent</name>
    <message>
        <source>&amp;No</source>
        <translation>&amp;Ні</translation>
    </message>
    <message>
        <source>&amp;Yes</source>
        <translation>&amp;Так</translation>
    </message>
    <message>
        <source> - SMS</source>
        <translation> - SMS</translation>
    </message>
    <message>
        <source>You didn&apos;t edit the SMS.
Do you really want to send it?</source>
        <translation>Ви не ввели SMS повідомлення.
Ви дійсно бажаєте відіслати його?</translation>
    </message>
    <message>
        <source>Phone : </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Chars left : </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>UserSendUrlEvent</name>
    <message>
        <source> - URL</source>
        <translation> - Адреса</translation>
    </message>
    <message>
        <source>URL : </source>
        <translation>Адреса : </translation>
    </message>
</context>
<context>
    <name>UserViewEvent</name>
    <message>
        <source>Chat</source>
        <translation>Розмова</translation>
    </message>
    <message>
        <source>&amp;Join</source>
        <translation>&amp;Приєднатися</translation>
    </message>
    <message>
        <source>&amp;Menu</source>
        <translation>&amp;Меню</translation>
    </message>
    <message>
        <source>&amp;View</source>
        <translation>&amp;Переглянути</translation>
    </message>
    <message>
        <source>Nex&amp;t</source>
        <translation>Дал&amp;і</translation>
    </message>
    <message>
        <source>A&amp;dd %1 Users</source>
        <translation>&amp;Додати %1 користувачів</translation>
    </message>
    <message>
        <source>Aut&amp;o Close</source>
        <translation>&amp;Авто-закриття</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Закрити</translation>
    </message>
    <message>
        <source>&amp;Quote</source>
        <translation>&amp;Цитувати</translation>
    </message>
    <message>
        <source>&amp;Reply</source>
        <translation>&amp;Відповісти</translation>
    </message>
    <message>
        <source>A&amp;ccept</source>
        <translation>&amp;Прийняти</translation>
    </message>
    <message>
        <source>Normal Click - Close Window
&lt;CTRL&gt;+Click - also delete User</source>
        <translation>Просте клацання - закриває вікно
&lt;CTRL&gt;+Клацання - також видалить користувача</translation>
    </message>
    <message>
        <source>File Transfer</source>
        <translation>Передача файла</translation>
    </message>
    <message>
        <source>Start Chat</source>
        <translation>Почати розмову</translation>
    </message>
    <message>
        <source>
--------------------
Request was cancelled.</source>
        <translation> 
--------------------
Запит було скасовано.</translation>
    </message>
    <message>
        <source>View URL failed</source>
        <translation>Перегляд адреси не вдався</translation>
    </message>
    <message>
        <source>A&amp;dd User</source>
        <translation>Д&amp;одати користувача</translation>
    </message>
    <message>
        <source>A&amp;uthorize</source>
        <translation>&amp;Авторизувати</translation>
    </message>
    <message>
        <source>&amp;Refuse</source>
        <translation>&amp;Відмовити</translation>
    </message>
    <message>
        <source>Nex&amp;t (%1)</source>
        <translation>Дал&amp;і (%1)</translation>
    </message>
    <message>
        <source>&amp;Forward</source>
        <translation>&amp;Перенаправити</translation>
    </message>
</context>
<context>
    <name>WharfIcon</name>
    <message>
        <source>Left click - Show main window
Middle click - Show next message
Right click - System menu</source>
        <translation>Ліва кнопка - показати головне вікно
Середня кнопка - показати наступне повідомлення
Права кнопка - меню</translation>
    </message>
</context>
</TS>
