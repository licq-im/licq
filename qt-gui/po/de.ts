<!DOCTYPE TS><TS>
<context>
    <name>@default</name>
    <message>
        <source>KDE default</source>
        <translation>KDE-Voreinstellung</translation>
    </message>
</context>
<context>
    <name>AddUserDlg</name>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;Ok</translation>
    </message>
    <message>
        <source>Licq - Add User</source>
        <translation>Licq - Benutzer hinzufügen</translation>
    </message>
    <message>
        <source>Protocol:</source>
        <translation>Protokoll:</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>Ab&amp;brechen</translation>
    </message>
    <message>
        <source>New User ID:</source>
        <translation>Neue Benutzer ID:</translation>
    </message>
</context>
<context>
    <name>AuthUserDlg</name>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;Ok</translation>
    </message>
    <message>
        <source>Authorize which user (Id):</source>
        <translation>Benutzer autorisieren (Id):</translation>
    </message>
    <message>
        <source>Refuse authorization to %1</source>
        <translation>Autorisierung für %1 ablehnen</translation>
    </message>
    <message>
        <source>Grant authorization to %1</source>
        <translation>%1 autorisieren</translation>
    </message>
    <message>
        <source>Licq - Grant Authorisation</source>
        <translation>Licq - Benutzer autorisieren</translation>
    </message>
    <message>
        <source>Licq - Refuse Authorisation</source>
        <translation>Licq - Benutzer nicht autorisieren</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>Ab&amp;brechen</translation>
    </message>
    <message>
        <source>Response</source>
        <translation>Antwort</translation>
    </message>
</context>
<context>
    <name>AwayMsgDlg</name>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;Ok</translation>
    </message>
    <message>
        <source>I&apos;m currently %1, %a.
You can leave me a message.
(%m messages pending from you).</source>
        <translation>Ich bin zur Zeit %1, %a.
Sie können mir eine Nachricht hinterlassen.
(%m Nachrichten von Ihnen warten auf Abruf).</translation>
    </message>
    <message>
        <source>&amp;Hints</source>
        <translation>&amp;Tipps</translation>
    </message>
    <message>
        <source>(Closing in %1)</source>
        <translation>(Schließt in %1s)</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Hints for Setting&lt;br&gt;your Auto-Response&lt;/h2&gt;&lt;br&gt;&lt;hr&gt;&lt;br&gt;&lt;ul&gt;&lt;li&gt;You can include any of the % expansions (described in the main hints page).&lt;/li&gt;&lt;li&gt;Any line beginning with a pipe (|) will be treated as a command to be run.  The line will be replaced by the output of the command. The command is parsed by /bin/sh so any shell commands or meta-characters are allowed.  For security reasons, any % expansions are automatically passed to the command surrounded by single quotes to prevent shell parsing of any meta-characters included in an alias.&lt;br&gt;Examples of popular uses include:&lt;ul&gt;&lt;li&gt;&lt;tt&gt;|date&lt;/tt&gt;: Will replace that line by the current date&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|fortune&lt;/tt&gt;: Show a fortune, as a tagline for example&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|myscript.sh %u %a&lt;/tt&gt;: Run a script, passing the uin and alias&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|myscript.sh %u %a &gt; /dev/null&lt;/tt&gt;: Run the same script but ignore the output (for tracking auto response checks or something)&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|if [ %u -lt 100000 ]; then echo &quot;You are special&quot;; fi&lt;/tt&gt;: Useless, but shows how you can use shell script.&lt;/li&gt;&lt;/ul&gt;Of course, multiple &quot;|&quot; can appear in the auto response, and commands and regular text can be mixed line by line.&lt;/li&gt;&lt;hr&gt;&lt;p&gt; For more information, see the Licq webpage (&lt;tt&gt;http://www.licq.org&lt;/tt&gt;).&lt;/p&gt;</source>
        <translation>&lt;h2&gt;Tipps für das Einstellen der Auto-Antwort&lt;/h2&gt;&lt;br&gt;&lt;hr&gt;&lt;br&gt;&lt;ul&gt;&lt;li&gt;Sie können jede %-Erweiterung, die im Haupt-Tippfenster beschrieben ist, verwenden.&lt;/li&gt;&lt;li&gt;Jede Zeile, die mit einer Pipe (&quot;|&quot;) beginnt, wird als Befehl behandelt. Die Zeile wird dann durch dessen Ausgabe ersetzt. Der Befehl wird durch /bin/sh interpretiert, es sind also alle Shell-Kommandos und Meta-Zeichen erlaubt.&lt;br&gt;Die %-Erweiterungen werden dabei automatisch in einfachen Anführungszeichen an die Shell übergegeben, um die Interpretation von Meta-Zeichen, die z. B. der Alias enthalten kann, zu verhindern. &lt;br&gt; Beliebte Anwendungsbeispiele:&lt;ul&gt;&lt;li&gt;&lt;tt&gt;|date&lt;/tt&gt;: Ersetzt die Zeile durch das aktuelle Datum&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|fortune&lt;/tt&gt;: Zeigt einen Spruch (z. B. als Fußzeile)&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|myscript.sh %u %a&lt;/tt&gt;: Führt ein Script mit UIN und Alias als Parameter aus&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|myscript.sh %u %a &gt; /dev/null&lt;/tt&gt;: Führt dasselbe Script aus, ignoriert jedoch die Ausgabe (z. B. um die Auto-Antwort-Abfragen zu protokollieren, oder ähnliches)&lt;/li&gt; &lt;li&gt;&lt;tt&gt;|if [ %u -lt 100000 ]; then echo &quot;Du bist etwas Besonderes&quot;; fi&lt;/tt&gt;: Sinnlos, aber es zeigt, wie man ein Shellscript benutzen kann.&lt;/li&gt;&lt;/ul&gt;Natürlich können auch mehrere &quot;|&quot; in einer Auto-Antwort auftauchen, Befehle und normaler Text können zeilenweise gemischt werden.&lt;/li&gt;&lt;hr&gt;&lt;p&gt; Für weitere Informationen besuchen Sie die Licq Homepage (&lt;tt&gt;http://www.licq.org&lt;/tt&gt;).&lt;/p&gt;</translation>
    </message>
    <message>
        <source>&amp;Edit Items</source>
        <translation>&amp;Bearbeiten</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>Ab&amp;brechen</translation>
    </message>
    <message>
        <source>&amp;Select</source>
        <translation>Aus&amp;wählen</translation>
    </message>
    <message>
        <source>Set %1 Response for %2</source>
        <translation>%1 Antwort für %2 setzen</translation>
    </message>
</context>
<context>
    <name>CEditFileListDlg</name>
    <message>
        <source>&amp;Up</source>
        <translation>&amp;Hoch</translation>
    </message>
    <message>
        <source>&amp;Done</source>
        <translation>&amp;Fertig</translation>
    </message>
    <message>
        <source>&amp;Down</source>
        <translation>&amp;Runter</translation>
    </message>
    <message>
        <source>Licq - Files to send</source>
        <translation>Licq - Dateien zum senden</translation>
    </message>
    <message>
        <source>&amp;Delete</source>
        <translation>&amp;Entfernen</translation>
    </message>
</context>
<context>
    <name>CEmoticons</name>
    <message>
        <source>Default</source>
        <translation>Voreinstellung</translation>
    </message>
    <message>
        <source>None</source>
        <translation>Nichts</translation>
    </message>
</context>
<context>
    <name>CFileDlg</name>
    <message>
        <source>KB</source>
        <translation>KB</translation>
    </message>
    <message>
        <source>MB</source>
        <translation>MB</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>Ok</translation>
    </message>
    <message>
        <source>Byte</source>
        <translation>Byte</translation>
    </message>
    <message>
        <source>ETA:</source>
        <translation>Verbleibend:</translation>
    </message>
    <message>
        <source>%1/%2</source>
        <translation>%1/%2</translation>
    </message>
    <message>
        <source>&amp;Open</source>
        <translation>Ö&amp;ffnen</translation>
    </message>
    <message>
        <source>Not enough resources.
</source>
        <translation>Nicht genügend Ressourcen.
</translation>
    </message>
    <message>
        <source>Connecting to remote...</source>
        <translation>Verbinden mit Gegenstelle...</translation>
    </message>
    <message>
        <source>Bytes</source>
        <translation>Bytes</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <source>File:</source>
        <translation>Datei:</translation>
    </message>
    <message>
        <source>Retry</source>
        <translation>Wiederholen</translation>
    </message>
    <message>
        <source>Time:</source>
        <translation>Zeit:</translation>
    </message>
    <message>
        <source>Licq - File Transfer (%1)</source>
        <translation>Licq - Dateiübertragung (%1)</translation>
    </message>
    <message>
        <source>Connection error.
</source>
        <translation>Verbindungsfehler.
</translation>
    </message>
    <message>
        <source>Remote side disconnected.</source>
        <translation>Verbindung durch Gegenstelle unterbrochen.</translation>
    </message>
    <message>
        <source>Sent %1 to %2 successfully.</source>
        <translation>%1 von %2 erfolgreich gesendet.</translation>
    </message>
    <message>
        <source>Waiting for connection...</source>
        <translation>Auf Verbindung warten...</translation>
    </message>
    <message>
        <source>File already exists and appears incomplete.</source>
        <translation>Datei existiert bereits und scheint unvollständig zu sein.</translation>
    </message>
    <message>
        <source>Batch:</source>
        <translation>Gesamt:</translation>
    </message>
    <message>
        <source>File already exists and is at least as big as the incoming file.</source>
        <translation>Datei existiert bereits und ist mindestens so groß, wie die zu empfangende Datei.</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <source>Resume</source>
        <translation>Wiederaufnehmen</translation>
    </message>
    <message>
        <source>Unable to create a thread.
See Network Window for details.</source>
        <translation>Es lässt sich kein neuer Thread erzeugen.
Siehe Netzwerkfenster für Details.</translation>
    </message>
    <message>
        <source>Handshaking error.
</source>
        <translation>Fehler bei der Protokollaushandlung.
</translation>
    </message>
    <message>
        <source>O&amp;pen Dir</source>
        <translation>Ordn&amp;er öffnen</translation>
    </message>
    <message>
        <source>Handshake Error.
See Network Window for details.</source>
        <translation>Fehler bei der Protokollaushandlung.
Siehe Netzwerkfenster für Details.</translation>
    </message>
    <message>
        <source>File name:</source>
        <translation>Dateiname:</translation>
    </message>
    <message>
        <source>Bind error.
</source>
        <translation>Fehler beim binden eines Port.
</translation>
    </message>
    <message>
        <source>Sending file...</source>
        <translation>Datei senden...</translation>
    </message>
    <message>
        <source>File I/O error: %1.</source>
        <translation>Datei E/A Fehler: %1.</translation>
    </message>
    <message>
        <source>File I/O Error:
%1

See Network Window for details.</source>
        <translation>Datei E/A Fehler:
%1

Siehe Netzwerkfenster für Details.</translation>
    </message>
    <message>
        <source>File transfer cancelled.</source>
        <translation>Dateiübertragung abgebrochen.</translation>
    </message>
    <message>
        <source>Receiving file...</source>
        <translation>Datei empfangen...</translation>
    </message>
    <message>
        <source>Current:</source>
        <translation>Aktuell:</translation>
    </message>
    <message>
        <source>Open error - unable to open file for writing.</source>
        <translation>Schreibfehler - Datei ließ sich nicht zum Schreiben öffnen.</translation>
    </message>
    <message>
        <source>Overwrite</source>
        <translation>Überschreiben</translation>
    </message>
    <message>
        <source>&amp;Cancel Transfer</source>
        <translation>Übertragung ab&amp;brechen</translation>
    </message>
    <message>
        <source>Unable to bind to a port.
See Network Window for details.</source>
        <translation>Port lässt sich nicht binden.
Siehe Netzwerkfenster für Details.</translation>
    </message>
    <message>
        <source>Received %1 from %2 successfully.</source>
        <translation>%1 von %2 erfolgreich empfangen.</translation>
    </message>
    <message>
        <source>Unable to reach remote host.
See Network Window for details.</source>
        <translation>Gegenstelle ist unerreichbar.
Siehe Netzwerkfenster für Details.</translation>
    </message>
    <message>
        <source>File transfer complete.</source>
        <translation>Dateiübertragung fertig.</translation>
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
        <translation>Nachricht</translation>
    </message>
    <message>
        <source>Forward %1 To User</source>
        <translation>%1 an Benutzer weiterleiten</translation>
    </message>
    <message>
        <source>Drag the user to forward to here:</source>
        <translation>Den Empfänger hier hereinziehen:</translation>
    </message>
    <message>
        <source>Unable to forward this message type (%d).</source>
        <translation>Dieser Nachrichtentyp lässt sich nicht weiterleiten (%d).</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>Ab&amp;brechen</translation>
    </message>
    <message>
        <source>Forwarded message:
</source>
        <translation>Weitergeleitete Nachricht:
</translation>
    </message>
    <message>
        <source>&amp;Forward</source>
        <translation>&amp;Weiterleiten</translation>
    </message>
    <message>
        <source>Forwarded URL:
</source>
        <translation>Weitergeleitete URL:
</translation>
    </message>
</context>
<context>
    <name>CInfoField</name>
    <message>
        <source>Unknown</source>
        <translation>Unbekannt</translation>
    </message>
</context>
<context>
    <name>CJoinChatDlg</name>
    <message>
        <source>&amp;Join</source>
        <translation>&amp;Betreten</translation>
    </message>
    <message>
        <source>Select chat to join:</source>
        <translation>Chat zum teilnehmen auswählen:</translation>
    </message>
    <message>
        <source>Invite to Join Chat</source>
        <translation>In den Chat einladen</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>Ab&amp;brechen</translation>
    </message>
    <message>
        <source>&amp;Invite</source>
        <translation>&amp;Einladen</translation>
    </message>
    <message>
        <source>Select chat to invite:</source>
        <translation>Chat für die Einladung auswählen:</translation>
    </message>
    <message>
        <source>Join Multiparty Chat</source>
        <translation>An Gruppenchat teilnehmen</translation>
    </message>
</context>
<context>
    <name>CLicqMessageBox</name>
    <message>
        <source>Licq</source>
        <translation>Licq</translation>
    </message>
    <message>
        <source>&amp;List</source>
        <translation>&amp;Liste</translation>
    </message>
    <message>
        <source>&amp;Next</source>
        <translation>&amp;Nächste</translation>
    </message>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>&amp;Clear All</source>
        <translation>Alle lös&amp;chen</translation>
    </message>
    <message>
        <source>&amp;Next (%1)</source>
        <translation>&amp;Nächste (%1)</translation>
    </message>
    <message>
        <source>Licq Information</source>
        <translation>Licq Information</translation>
    </message>
    <message>
        <source>Licq Warning</source>
        <translation>Licq Warnung</translation>
    </message>
    <message>
        <source>Licq Critical</source>
        <translation>Licq Kritische Nachricht</translation>
    </message>
</context>
<context>
    <name>CMMSendDlg</name>
    <message>
        <source>Sending mass message to %1...</source>
        <translation>Massen-Nachricht senden an %1...</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>S&amp;chließen</translation>
    </message>
    <message>
        <source>Sending mass URL to %1...</source>
        <translation>Massen-URL senden an %1...</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>fehlgeschlagen</translation>
    </message>
    <message>
        <source>Multiple Recipient Contact List</source>
        <translation>Kontaktliste an mehrere Empfänger</translation>
    </message>
    <message>
        <source>Multiple Recipient URL</source>
        <translation>URL an mehrere Empfänger</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>Ab&amp;brechen</translation>
    </message>
    <message>
        <source>Sending mass list to %1...</source>
        <translation>Massen-Kontaktliste senden an %1...</translation>
    </message>
    <message>
        <source>Multiple Recipient Message</source>
        <translation>Nachricht an mehrere Empfänger</translation>
    </message>
</context>
<context>
    <name>CMMUserView</name>
    <message>
        <source>Crop</source>
        <translation>Benutzer ausschneiden</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation>Alle entfernen</translation>
    </message>
    <message>
        <source>Drag&apos;n&apos;Drop didn&apos;t work</source>
        <translation>Drag und Drop hat nicht funktioniert</translation>
    </message>
    <message>
        <source>Remove</source>
        <translation>Benutzer entfernen</translation>
    </message>
    <message>
        <source>Add Group</source>
        <translation>Gruppe hinzufügen</translation>
    </message>
    <message>
        <source>Add All</source>
        <translation>Alle hinzufügen</translation>
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
        <translation>en</translation>
    </message>
    <message>
        <source>Ok</source>
        <translation>Ok</translation>
    </message>
    <message>
        <source>&amp;No</source>
        <translation>&amp;Nein</translation>
    </message>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;Ok</translation>
    </message>
    <message>
        <source>&amp;Yes</source>
        <translation>&amp;Ja</translation>
    </message>
    <message>
        <source>Busy</source>
        <translation>Besetzt</translation>
    </message>
    <message>
        <source>Send</source>
        <translation>Senden</translation>
    </message>
    <message>
        <source>Online to User</source>
        <translation>Immer &apos;Online&apos;</translation>
    </message>
    <message>
        <source>&amp;Away</source>
        <translation>&amp;Abwesend</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>&amp;Hilfe</translation>
    </message>
    <message>
        <source>&amp;Info</source>
        <translation>&amp;Informationen</translation>
    </message>
    <message>
        <source>U&amp;tilities</source>
        <translation>&amp;Werkzeuge</translation>
    </message>
    <message>
        <source>E&amp;xit</source>
        <translation>&amp;Beenden</translation>
    </message>
    <message>
        <source>&amp;Options...</source>
        <translation>&amp;Optionen...</translation>
    </message>
    <message>
        <source>R&amp;andom Chat</source>
        <translation>Zufälliger &amp;Chat</translation>
    </message>
    <message>
        <source>&amp;Random Chat Group</source>
        <translation>&amp;Zufällige Chat-Gruppe</translation>
    </message>
    <message>
        <source>Licq is unable to start your browser and open the URL.
You will need to start the browser and open the URL manually.</source>
        <translation>Licq kann den Browser nicht starten um die URL zu öffnen
Sie müssen den Browser selbst starten und die URL in die Adressleiste kopieren.</translation>
    </message>
    <message>
        <source>Up since %1
</source>
        <translation>Gestartet seit %1
</translation>
    </message>
    <message>
        <source>Re&amp;quest Authorization</source>
        <translation>Autorisierung an&amp;fordern</translation>
    </message>
    <message>
        <source>&amp;GPG Key Manager...</source>
        <translation>&amp;GPG Schlüsselverwaltung...</translation>
    </message>
    <message>
        <source>Check %1 Response</source>
        <translation>%1-Antwort lesen</translation>
    </message>
    <message>
        <source>Server Group</source>
        <translation>Servergruppe</translation>
    </message>
    <message>
        <source>Auto Request Secure</source>
        <translation>Automatisch Verschlüsseln</translation>
    </message>
    <message>
        <source>Unable to open icons file
%1.</source>
        <translation>Symbole-Datei ließ sich nicht öffnen
%1.</translation>
    </message>
    <message>
        <source>Send &amp;Authorization</source>
        <translation>&amp;Autorisierung senden</translation>
    </message>
    <message>
        <source>&lt;b&gt;%1&lt;/b&gt; is online</source>
        <translation>&lt;b&gt;%1&lt;/b&gt; ist online</translation>
    </message>
    <message>
        <source>Auto Accept Chats</source>
        <translation>Chats automatisch akzeptieren</translation>
    </message>
    <message>
        <source>Auto Accept Files</source>
        <translation>Dateien automatisch akzeptieren</translation>
    </message>
    <message>
        <source>&amp;Not Available</source>
        <translation>&amp;Nicht verfügbar</translation>
    </message>
    <message>
        <source>&amp;About</source>
        <translation>Ü&amp;ber</translation>
    </message>
    <message>
        <source>&amp;Group</source>
        <translation>&amp;Gruppe</translation>
    </message>
    <message>
        <source>&amp;Hints</source>
        <translation>&amp;Tipps</translation>
    </message>
    <message>
        <source>&amp;Reset</source>
        <translation>Zu&amp;rücksetzen</translation>
    </message>
    <message>
        <source>Custom Auto Response...</source>
        <translation>Spezielle Auto-Antwort...</translation>
    </message>
    <message>
        <source>(Error! No owner set)</source>
        <translation>(Fehler! Kein Eigentümer eingerichtet)</translation>
    </message>
    <message>
        <source>Set &amp;Auto Response...</source>
        <translation>&amp;Auto Antwort einstellen...</translation>
    </message>
    <message>
        <source>Remove From List</source>
        <translation>Von Kontaktliste entfernen</translation>
    </message>
    <message>
        <source>System Message</source>
        <translation>Systemnachricht</translation>
    </message>
    <message>
        <source>%1 msg%2</source>
        <translation>%1 Nachricht%2</translation>
    </message>
    <message>
        <source>&amp;Popup All Messages</source>
        <translation>Alle Nachrichten ö&amp;ffnen</translation>
    </message>
    <message>
        <source>Accept in Not Available</source>
        <translation>Bei &apos;Nicht verfügbar&apos; akzeptieren</translation>
    </message>
    <message>
        <source>View &amp;History</source>
        <translation>&amp;Verlauf</translation>
    </message>
    <message>
        <source>Other Users</source>
        <translation>Sonstige</translation>
    </message>
    <message>
        <source>Edit User Group</source>
        <translation>&amp;Gruppenmitgliedschaft...</translation>
    </message>
    <message>
        <source>Unknown Packets</source>
        <translation>&amp;Unbekannte Pakete</translation>
    </message>
    <message>
        <source>Send &amp;URL</source>
        <translation>&amp;Url senden</translation>
    </message>
    <message>
        <source>Send &amp;SMS</source>
        <translation>&amp;SMS senden</translation>
    </message>
    <message>
        <source>Update Status Plugin List</source>
        <translation>Status-Plugin-Liste aktualisieren</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <source>&lt;hr&gt;&lt;p&gt; For more information, see the Licq webpage (&lt;tt&gt;http://www.licq.org&lt;/tt&gt;).&lt;/p&gt;</source>
        <translation>&lt;hr&gt;&lt;p&gt;Für weitere Informationen besuchen Sie bitte die Licq Homepage (&lt;tt&gt;http://www.licq.org&lt;/tt&gt;).&lt;/p&gt;</translation>
    </message>
    <message>
        <source>&amp;Mini Mode</source>
        <translation>&amp;Miniaturansicht</translation>
    </message>
    <message>
        <source>Update File Server Status</source>
        <translation>Dateiserver-Status aktualisieren</translation>
    </message>
    <message>
        <source>Errors</source>
        <translation>&amp;Fehler</translation>
    </message>
    <message>
        <source>Close &amp;Secure Channel</source>
        <translation>Sicheren Kanal &amp;schließeneren Kanal</translation>
    </message>
    <message>
        <source>No msgs</source>
        <translation>Keine Nachrichten</translation>
    </message>
    <message>
        <source>Up since %1

</source>
        <translation>Gestartet seit %1

</translation>
    </message>
    <message>
        <source>Misc Modes</source>
        <translation>Ve&amp;rschiedenes</translation>
    </message>
    <message>
        <source>Update ICQphone Status</source>
        <translation>ICQphone-Status aktualisieren</translation>
    </message>
    <message>
        <source>Daemon Statistics

</source>
        <translation>Daemon-Statistiken

</translation>
    </message>
    <message>
        <source>Update Info Plugin List</source>
        <translation>Info-Plugin-Liste aktualisieren</translation>
    </message>
    <message>
        <source>&amp;Owner Manager</source>
        <translation>&amp;Zugangs-Verwaltung</translation>
    </message>
    <message>
        <source>Daemon Statistics
(Today/Total)
</source>
        <translation>Licq Daemon-Statistiken
(Heute/Gesamt)
</translation>
    </message>
    <message>
        <source>SysMsg</source>
        <translation>Systemnachricht</translation>
    </message>
    <message>
        <source>System</source>
        <translation>System</translation>
    </message>
    <message>
        <source>Send Authorization Re&amp;quest</source>
        <translation>Autorisierungsan&amp;frage senden</translation>
    </message>
    <message>
        <source>Last reset %1

</source>
        <translation>Zuletzt zurückgesetzt: %1

</translation>
    </message>
    <message>
        <source>Phone &quot;Follow Me&quot;</source>
        <translation>Telefon folgen</translation>
    </message>
    <message>
        <source>Are you sure you want to remove
%1 (%2)
from the &apos;%3&apos; group?</source>
        <translation>Soll
%1 (%2)
wirklich aus der Gruppe &apos;%3&apos; entfernt werden?</translation>
    </message>
    <message>
        <source>Do you really want to add
%1 (%2)
to your ignore list?</source>
        <translation>Soll
%1 (%2)
wirklich zur Ignorieren-Gruppe hinzugefügt werden?</translation>
    </message>
    <message>
        <source>Packets</source>
        <translation>&amp;Pakete</translation>
    </message>
    <message>
        <source>&amp;Save All Users</source>
        <translation>A&amp;lle Benutzerdaten speichern</translation>
    </message>
    <message>
        <source>%1: %2 / %3
</source>
        <translation>%1: %2 / %3
</translation>
    </message>
    <message>
        <source>Do Not Disturb to User</source>
        <translation>Immer &apos;Nicht stören&apos;</translation>
    </message>
    <message>
        <source>Update All Users</source>
        <translation>Alle &amp;Benutzer aktualisieren</translation>
    </message>
    <message>
        <source>&amp;Redraw User Window</source>
        <translation>&amp;Hauptfenster auffrischen</translation>
    </message>
    <message>
        <source>(with KDE support)
</source>
        <translation>(mit KDE-Unterstützung)
</translation>
    </message>
    <message>
        <source>Occupied to User</source>
        <translation>Immer &apos;Beschäftigt&apos;</translation>
    </message>
    <message>
        <source>Logon failed.
See network window for details.</source>
        <translation>Die Anmeldung ist fehlgeschlagen.
Siehe Netzwerkfenster für Details.</translation>
    </message>
    <message>
        <source>Accept in Away</source>
        <translation>Bei &apos;Abwesenheit&apos; akzeptieren</translation>
    </message>
    <message>
        <source>Send &amp;Chat Request</source>
        <translation>&amp;Chatanfrage senden</translation>
    </message>
    <message>
        <source>Show Offline &amp;Users</source>
        <translation>O&amp;ffline-Benutzer anzeigen</translation>
    </message>
    <message>
        <source>Available</source>
        <translation>Erreichbar</translation>
    </message>
    <message>
        <source>%1 message%2</source>
        <translation>%1 Nachricht%2</translation>
    </message>
    <message>
        <source>%1: %2
</source>
        <translation>%1: %2
</translation>
    </message>
    <message>
        <source>Update Phone &quot;Follow Me&quot; Status</source>
        <translation>&quot;Telefon folgen&quot;-Status aktualisieren</translation>
    </message>
    <message>
        <source>Warnings</source>
        <translation>Warnun&amp;gen</translation>
    </message>
    <message>
        <source>Debug Level</source>
        <translation>&amp;Debug-Stufe</translation>
    </message>
    <message>
        <source>Use GPG Encryption</source>
        <translation>GPG-Verschlüsselung &amp;aktivieren</translation>
    </message>
    <message>
        <source>&amp;Network Window</source>
        <translation>&amp;Netzwerkfenster</translation>
    </message>
    <message>
        <source>&amp;View Event</source>
        <translation>&amp;Ereignis öffnen</translation>
    </message>
    <message>
        <source>&amp;Plugin Manager...</source>
        <translation>&amp;Plugin-Verwaltung...</translation>
    </message>
    <message>
        <source>O&amp;ccupied</source>
        <translation>Bes&amp;chäftigt</translation>
    </message>
    <message>
        <source>Set All</source>
        <translation>Alle &amp;wählen</translation>
    </message>
    <message>
        <source>&amp;View System Messages</source>
        <translation>S&amp;ystemnachrichten öffnen</translation>
    </message>
    <message>
        <source>&amp;Thread Group View</source>
        <translation>G&amp;ruppenansicht</translation>
    </message>
    <message>
        <source>Send &amp;Message</source>
        <translation>&amp;Nachricht senden</translation>
    </message>
    <message>
        <source>Update Current Group</source>
        <translation>Diese &amp;Gruppe aktualisieren</translation>
    </message>
    <message>
        <source>Licq (%1)</source>
        <translation>Licq (%1)</translation>
    </message>
    <message>
        <source>Right click - Status menu
Double click - Set auto response</source>
        <translation>Rechte Maustaste - Status setzen
Doppelklick - Auto Antwort setzen</translation>
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
#licq on irc.freenode.net

%3 (%4)
%5 contacts.</source>
        <translation>Licq Version %1%8.
Qt GUI Plugin Version %2.
Kompiliert am: %7
%6
Betreuer: Jon Keating
Beiträge von Dirk A. Mueller
Ursprünglicher Autor: Graham Roff
Übersetzung: Thomas Reitelbach
und Arne Schmitz

http://www.licq.org
#licq auf irc.freenode.net

%3 (%4)
%5 Kontake.</translation>
    </message>
    <message>
        <source>&amp;Add User</source>
        <translation>Benutzer &amp;hinzufügen</translation>
    </message>
    <message>
        <source>Accept in Do Not Disturb</source>
        <translation>Bei &apos;Nicht stören&apos; akzeptieren</translation>
    </message>
    <message>
        <source>Use Real Ip (LAN)</source>
        <translation>Echte IP (LAN) verwenden</translation>
    </message>
    <message>
        <source>&amp;Statistics</source>
        <translation>&amp;Statistiken</translation>
    </message>
    <message>
        <source>&amp;Online</source>
        <translation>&amp;Online</translation>
    </message>
    <message>
        <source>Clear All</source>
        <translation>Auswahl &amp;aufheben</translation>
    </message>
    <message>
        <source>&amp;Status</source>
        <translation>&amp;Status</translation>
    </message>
    <message>
        <source>&amp;System</source>
        <translation>&amp;System</translation>
    </message>
    <message>
        <source>Accept in Occupied</source>
        <translation>Bei &apos;Beschäftigt&apos; akzeptieren</translation>
    </message>
    <message>
        <source>Send Contact &amp;List</source>
        <translation>&amp;Kontaktliste senden</translation>
    </message>
    <message>
        <source>Status Info</source>
        <translation>&amp;Status-Info</translation>
    </message>
    <message>
        <source>Set GPG key</source>
        <translation>GPG-Schlüssel setzen</translation>
    </message>
    <message>
        <source>&amp;Do Not Disturb</source>
        <translation>Nicht &amp;stören</translation>
    </message>
    <message>
        <source>Are you sure you want to remove
%1 (%2)
from your contact list?</source>
        <translation>Soll
%1 (%2)
wirklich aus der Kontaktliste entfernt werden?</translation>
    </message>
    <message>
        <source>O&amp;ffline</source>
        <translation>O&amp;ffline</translation>
    </message>
    <message>
        <source>A&amp;uthorize User</source>
        <translation>Benutzer &amp;autorisieren</translation>
    </message>
    <message>
        <source>Free for C&amp;hat</source>
        <translation>Frei für C&amp;hat</translation>
    </message>
    <message>
        <source>Right click - User groups
Double click - Show next message</source>
        <translation>Rechte Maustaste - Benutzergruppen
Doppelklick - Nächste Nachricht</translation>
    </message>
    <message>
        <source>No messages</source>
        <translation>Keine Nachrichten</translation>
    </message>
    <message>
        <source>&amp;Invisible</source>
        <translation>Uns&amp;ichtbar</translation>
    </message>
    <message>
        <source>Error! No owner set</source>
        <translation>Fehler! Kein Eigentümer eingerichtet</translation>
    </message>
    <message>
        <source>Send &amp;File Transfer</source>
        <translation>&amp;Datei senden</translation>
    </message>
    <message>
        <source>User Functions</source>
        <translation>Ben&amp;utzer</translation>
    </message>
    <message>
        <source>Check Auto Response</source>
        <translation>Auto-Antwort &amp;lesen</translation>
    </message>
    <message>
        <source>Unable to open extended icons file
%1.</source>
        <translation>Erweiterte Symbole-Datei ließ sich nicht öffnen
%1.</translation>
    </message>
    <message>
        <source>System Functions</source>
        <translation>&amp;Allgemeines</translation>
    </message>
    <message>
        <source>Edit &amp;Groups</source>
        <translation>Gruppen &amp;bearbeiten</translation>
    </message>
    <message>
        <source>Don&apos;t Show</source>
        <translation>Nicht zeigen</translation>
    </message>
    <message>
        <source>S&amp;kin Browser...</source>
        <translation>&amp;Design-Verwaltung...</translation>
    </message>
    <message>
        <source>&amp;Security/Password Options</source>
        <translation>&amp;Sicherheit / Passwort</translation>
    </message>
    <message>
        <source>Request &amp;Secure Channel</source>
        <translation>&amp;Verschlüsselung aktivieren</translation>
    </message>
    <message>
        <source>S&amp;earch for User</source>
        <translation>Benutzer &amp;suchen</translation>
    </message>
    <message>
        <source>Not Available to User</source>
        <translation>Immer &apos;Nicht verfügbar&apos;</translation>
    </message>
    <message>
        <source>Toggle &amp;Floaty</source>
        <translation>Schwebendes &amp;Fenster</translation>
    </message>
    <message>
        <source>Sa&amp;ve Settings</source>
        <translation>&amp;Einstellungen speichern</translation>
    </message>
    <message>
        <source>Away to User</source>
        <translation>Immer &apos;Abwesend&apos;</translation>
    </message>
    <message>
        <source>Licq is unable to find a browser application due to an internal error.</source>
        <translation>Licq kann aufgrund eines internen Fehlers keine Browseranwendung finden.</translation>
    </message>
    <message>
        <source>Do you really want to reset your stats?</source>
        <translation>Sollen Ihre Statistiken wirklich zurückgesetzt werden?</translation>
    </message>
    <message>
        <source>&lt;ul&gt;&lt;li&gt;&lt;tt&gt;%a - &lt;/tt&gt;user alias&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%e - &lt;/tt&gt;email&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%f - &lt;/tt&gt;first name&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%h - &lt;/tt&gt;phone number&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%i - &lt;/tt&gt;user ip&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%l - &lt;/tt&gt;last name&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%L - &lt;/tt&gt;local time&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%m - &lt;/tt&gt;# pending messages&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%M - &lt;/tt&gt;# pending messages (if any)&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%n - &lt;/tt&gt;full name&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%o - &lt;/tt&gt;last seen online&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%O - &lt;/tt&gt;online since&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%p - &lt;/tt&gt;user port&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%s - &lt;/tt&gt;full status&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%S - &lt;/tt&gt;abbreviated status&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%u - &lt;/tt&gt;uin&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%w - &lt;/tt&gt;webpage&lt;/li&gt;&lt;/ul&gt;</source>
        <translation>&lt;ul&gt;&lt;li&gt;&lt;tt&gt;%a - &lt;/tt&gt;Benutzeralias&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%e - &lt;/tt&gt;Email&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%f - &lt;/tt&gt;Vorname&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%h - &lt;/tt&gt;Telefonnummer&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%i - &lt;/tt&gt;Benutzer IP&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%l - &lt;/tt&gt;Nachname&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%L - &lt;/tt&gt;Ortszeit&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%m - &lt;/tt&gt;# wartende Nachrichten&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%M - &lt;/tt&gt;# wartende Nachrichten (wenn vorhanden)&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%n - &lt;/tt&gt;Voller Name&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%o - &lt;/tt&gt;Letztes mal Online&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%O - &lt;/tt&gt;Online seit&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%p - &lt;/tt&gt;Benutzerport&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%s - &lt;/tt&gt;Voller Status&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%S - &lt;/tt&gt;Abgekürzter Status&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%u - &lt;/tt&gt;UIN&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%w - &lt;/tt&gt;Webseite&lt;/li&gt;&lt;/ul&gt;</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Hints for Using&lt;br&gt;the Licq Qt-GUI Plugin&lt;/h2&gt;&lt;br&gt;&lt;hr&gt;&lt;br&gt;&lt;ul&gt;&lt;li&gt;Change your status by right clicking on the status label.&lt;/li&gt;&lt;li&gt;Change your auto response by double-clicking on the status label.&lt;/li&gt;&lt;li&gt;View system messages by double clicking on the message label.&lt;/li&gt;&lt;li&gt;Change groups by right clicking on the message label.&lt;/li&gt;&lt;li&gt;Use the following shortcuts from the contact list:&lt;ul&gt;&lt;li&gt;&lt;tt&gt;Ctrl-M : &lt;/tt&gt;Toggle mini-mode&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-O : &lt;/tt&gt;Toggle show offline users&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-X : &lt;/tt&gt;Exit&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-H : &lt;/tt&gt;Hide&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-I : &lt;/tt&gt;View the next message&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-V : &lt;/tt&gt;View message&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-S : &lt;/tt&gt;Send message&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-U : &lt;/tt&gt;Send Url&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-C : &lt;/tt&gt;Send chat request&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-F : &lt;/tt&gt;Send File&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-A : &lt;/tt&gt;Check Auto response&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-P : &lt;/tt&gt;Popup all messages&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-L : &lt;/tt&gt;Redraw user window&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Delete : &lt;/tt&gt;Delete user from current group&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-Delete : &lt;/tt&gt;Delete user from contact list&lt;/li&gt;&lt;/ul&gt;&lt;li&gt;Hold control while clicking on close in the function window to remove   the user from your contact list.&lt;/li&gt;&lt;li&gt;Hit Ctrl-Enter from most text entry fields to select &quot;Ok&quot; or &quot;Accept&quot;.   For example in the send tab of the user function window.&lt;/li&gt;&lt;li&gt;Here is the complete list of user % options, which can be used in &lt;b&gt;OnEvent&lt;/b&gt;   parameters, &lt;b&gt;auto responses&lt;/b&gt;, and &lt;b&gt;utilities&lt;/b&gt;:</source>
        <translation>&lt;h2&gt;Hinweise für die Benutzung&lt;br&gt;des Licq Qt-GUI Plugin&lt;/h2&gt;&lt;br&gt;&lt;hr&gt;&lt;br&gt;&lt;ul&gt;&lt;li&gt;Änderung des Status durch Rechtsklick auf das Status-Label.&lt;/li&gt;&lt;li&gt;Änderung der Autoantwort durch Doppelklick auf das Status-Label.&lt;/li&gt;&lt;li&gt;Anzeigen von Nachrichten durch Doppelklick auf das Nachrichten-Label.&lt;/li&gt;&lt;li&gt;Gruppe wechseln durch Rechtsklick auf Nachrichten-Label.&lt;/li&gt;&lt;li&gt;Folgende Tastenkürzel sind in der Kontaktliste aktiv:&lt;ul&gt;&lt;li&gt;&lt;tt&gt;Strg-M : &lt;/tt&gt;Mini-Modus&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Strg-O : &lt;/tt&gt;Offline-Benutzer anzeigen&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Strg-X : &lt;/tt&gt;Beenden&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Strg-H : &lt;/tt&gt;Hauptfenster verstecken&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Strg-I : &lt;/tt&gt;Zeige nächste Nachricht an&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Strg-V : &lt;/tt&gt;Zeige Nachricht an&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Strg-S : &lt;/tt&gt;Sende Nachricht&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Strg-U : &lt;/tt&gt;Sende URL&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Strg-C : &lt;/tt&gt;Sende Chat-Anfrage&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Strg-F : &lt;/tt&gt;Sende Datei&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Strg-A : &lt;/tt&gt;Überprüfe Auto-Antwort&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Strg-P : &lt;/tt&gt;Alle Nachrichten anzeigen&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Strg-L : &lt;/tt&gt;Benutzerfenster neu zeichnen&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Entf : &lt;/tt&gt;Benutzer aus aktueller Gruppe löschen&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Strg-Entf : &lt;/tt&gt;Lösche Benutzer aus Kontaktliste&lt;/li&gt;&lt;/ul&gt;&lt;li&gt;Strg halten während des Klickens festhalten um   den Benutzer von der Kontaktliste zu entfernen.&lt;/li&gt;&lt;li&gt;Strg-Enter selektiert in den meisten Textfeldern &quot;Ok&quot; oder &quot;Anwenden&quot;.   Zum Beispiel im Chatfenster.&lt;/li&gt;&lt;li&gt;Hier ist die komplette Liste von Benutzeroptionen für &lt;b&gt;OnEvent&lt;/b&gt;,    &lt;b&gt;Auto-Antworten&lt;/b&gt; und &lt;b&gt;Werkzeuge&lt;/b&gt; in den Benutzerfunktionen:</translation>
    </message>
</context>
<context>
    <name>CMessageViewWidget</name>
    <message>
        <source>%1 to %2</source>
        <translation>%1 an %2</translation>
    </message>
    <message>
        <source>%1 from %2</source>
        <translation>%1 von %2</translation>
    </message>
    <message>
        <source>Default</source>
        <translation>Voreinstellung</translation>
    </message>
    <message>
        <source>Compact</source>
        <translation>Kompakt</translation>
    </message>
    <message>
        <source>Tiny</source>
        <translation>Kurz</translation>
    </message>
    <message>
        <source>Table</source>
        <translation>Tabelle</translation>
    </message>
    <message>
        <source>Long</source>
        <translation>Lang</translation>
    </message>
    <message>
        <source>Wide</source>
        <translation>Weit</translation>
    </message>
</context>
<context>
    <name>CQtLogWindow</name>
    <message>
        <source>&amp;Save</source>
        <translation>&amp;Speichern</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>S&amp;chließen</translation>
    </message>
    <message>
        <source>Failed to open file:
%1</source>
        <translation>Fehler beim Öffnen der Datei:
%1</translation>
    </message>
    <message>
        <source>C&amp;lear</source>
        <translation>&amp;Löschen</translation>
    </message>
    <message>
        <source>Licq Network Log</source>
        <translation>Licq-Netzwerkprotokoll</translation>
    </message>
</context>
<context>
    <name>CRandomChatDlg</name>
    <message>
        <source>Seeking Men</source>
        <translation>Suche Männer</translation>
    </message>
    <message>
        <source>Games</source>
        <translation>Spiele</translation>
    </message>
    <message>
        <source>20 Something</source>
        <translation>Über 20</translation>
    </message>
    <message>
        <source>30 Something</source>
        <translation>Über 30</translation>
    </message>
    <message>
        <source>40 Something</source>
        <translation>Über 40</translation>
    </message>
    <message>
        <source>Seeking Women</source>
        <translation>Suche Frauen</translation>
    </message>
    <message>
        <source>Random chat search timed out.</source>
        <translation>Zeitüberschreitung bei Zufalls-Chat-Suche.</translation>
    </message>
    <message>
        <source>Random chat search had an error.</source>
        <translation>Fehler bei Zufalls-Chat-Suche.</translation>
    </message>
    <message>
        <source>No random chat user found in that group.</source>
        <translation>Keinen zufälligen Chatpartner in dieser Gruppe gefunden.</translation>
    </message>
    <message>
        <source>50 Plus</source>
        <translation>50 und älter</translation>
    </message>
    <message>
        <source>Romance</source>
        <translation>Romantik</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>Ab&amp;brechen</translation>
    </message>
    <message>
        <source>Searching for Random Chat Partner...</source>
        <translation>Nach zufälligem Chatpartner suchen...</translation>
    </message>
    <message>
        <source>&amp;Search</source>
        <translation>&amp;Suchen</translation>
    </message>
    <message>
        <source>Students</source>
        <translation>Studenten</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Allgemein</translation>
    </message>
    <message>
        <source>Random Chat Search</source>
        <translation>Zufalls-Chat-Suche</translation>
    </message>
</context>
<context>
    <name>CRefuseDlg</name>
    <message>
        <source>Licq %1 Refusal</source>
        <translation>Licq %1 Ablehnen</translation>
    </message>
    <message>
        <source>Refusal message for %1 with </source>
        <translation>Begründung für %1 mit </translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <source>Refuse</source>
        <translation>Ablehnen</translation>
    </message>
</context>
<context>
    <name>CSetRandomChatGroupDlg</name>
    <message>
        <source>&amp;Set</source>
        <translation>&amp;Setzen</translation>
    </message>
    <message>
        <source>done</source>
        <translation>fertig</translation>
    </message>
    <message>
        <source>Seeking Men</source>
        <translation>Suche Männer</translation>
    </message>
    <message>
        <source>Games</source>
        <translation>Spiele</translation>
    </message>
    <message>
        <source>error</source>
        <translation>fehler</translation>
    </message>
    <message>
        <source>20 Something</source>
        <translation>Über 20</translation>
    </message>
    <message>
        <source>30 Something</source>
        <translation>Über 30</translation>
    </message>
    <message>
        <source>40 Something</source>
        <translation>Über 40</translation>
    </message>
    <message>
        <source>Seeking Women</source>
        <translation>Suche Frauen</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>S&amp;chließen</translation>
    </message>
    <message>
        <source>(none)</source>
        <translation>(keine)</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>zeitüberschreitung</translation>
    </message>
    <message>
        <source>Set Random Chat Group</source>
        <translation>Gruppe für zufälligen Chat setzen</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>fehlgeschlagen</translation>
    </message>
    <message>
        <source>50 Plus</source>
        <translation>50 und älter</translation>
    </message>
    <message>
        <source>Romance</source>
        <translation>Romantik</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>Ab&amp;brechen</translation>
    </message>
    <message>
        <source>Students</source>
        <translation>Studenten</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Allgemein</translation>
    </message>
    <message>
        <source>Setting Random Chat Group...</source>
        <translation>Setzen der Gruppe für zufälligen Chat...</translation>
    </message>
</context>
<context>
    <name>CTimeZoneField</name>
    <message>
        <source>Unknown</source>
        <translation>Unbekannt</translation>
    </message>
</context>
<context>
    <name>CUserView</name>
    <message>
        <source>S</source>
        <translation>S</translation>
    </message>
    <message>
        <source> day</source>
        <translation> Tag</translation>
    </message>
    <message>
        <source> days</source>
        <translation> Tage</translation>
    </message>
    <message>
        <source> hour</source>
        <translation> Stunde</translation>
    </message>
    <message>
        <source> week</source>
        <translation> Woche</translation>
    </message>
    <message>
        <source>Not In List</source>
        <translation>Nicht in der Liste</translation>
    </message>
    <message>
        <source> hours</source>
        <translation> Stunden</translation>
    </message>
    <message>
        <source> weeks</source>
        <translation> Wochen</translation>
    </message>
    <message>
        <source> minutes</source>
        <translation> Minuten</translation>
    </message>
    <message>
        <source>0 minutes</source>
        <translation>0 Minuten</translation>
    </message>
    <message>
        <source>Online</source>
        <translation>Online</translation>
    </message>
    <message>
        <source>Offline</source>
        <translation>Offline</translation>
    </message>
    <message>
        <source> minute</source>
        <translation> Minute</translation>
    </message>
    <message>
        <source>%1 Floaty (%2)</source>
        <translation>%1 schwebend (%2)</translation>
    </message>
    <message>
        <source>Birthday Today!</source>
        <translation>Heute Geburtstag!</translation>
    </message>
    <message>
        <source>Typing a message</source>
        <translation>Tippt gerade eine Nachricht</translation>
    </message>
    <message>
        <source>Phone &amp;quot;Follow Me&amp;quot;: Available</source>
        <translation>Telefon &amp;quot;folgen&amp;quot;: Verfügbar</translation>
    </message>
    <message>
        <source>Phone &amp;quot;Follow Me&amp;quot;: Busy</source>
        <translation>Telefon &amp;quot;folgen&amp;quot;: Beschäftigt</translation>
    </message>
    <message>
        <source>ICQphone: Available</source>
        <translation>ICQphone: Verfügbar</translation>
    </message>
    <message>
        <source>ICQphone: Busy</source>
        <translation>ICQphone: Beschäftigt</translation>
    </message>
    <message>
        <source>File Server: Enabled</source>
        <translation>Dateiserver: aktiviert</translation>
    </message>
    <message>
        <source>Secure connection</source>
        <translation>Sichere Verbindung</translation>
    </message>
    <message>
        <source>Custom Auto Response</source>
        <translation>Eigene Auto-Antwort</translation>
    </message>
    <message>
        <source>Auto Response:</source>
        <translation>Auto-Antwort:</translation>
    </message>
    <message>
        <source>E: </source>
        <translation>E: </translation>
    </message>
    <message>
        <source>P: </source>
        <translation>P: </translation>
    </message>
    <message>
        <source>C: </source>
        <translation>C: </translation>
    </message>
    <message>
        <source>F: </source>
        <translation>F: </translation>
    </message>
    <message>
        <source>Ip: </source>
        <translation>lp: </translation>
    </message>
    <message>
        <source>O: </source>
        <translation>O: </translation>
    </message>
    <message>
        <source>Logged In: </source>
        <translation>Eingelogged: </translation>
    </message>
    <message>
        <source>Idle: </source>
        <translation>Untätig: </translation>
    </message>
    <message>
        <source>Local time: </source>
        <translation>Ortszeit: </translation>
    </message>
    <message>
        <source>ID: </source>
        <translation>ID: </translation>
    </message>
    <message>
        <source>Awaiting authorization</source>
        <translation>Erwarte Autorisierung</translation>
    </message>
</context>
<context>
    <name>CUtilityDlg</name>
    <message>
        <source>GUI</source>
        <translation>GUI</translation>
    </message>
    <message>
        <source>&amp;Run</source>
        <translation>&amp;Ausführen</translation>
    </message>
    <message>
        <source>Done</source>
        <translation>Fertig</translation>
    </message>
    <message>
        <source>Done:</source>
        <translation>Fertig:</translation>
    </message>
    <message>
        <source>Edit:</source>
        <translation>Bearbeiten:</translation>
    </message>
    <message>
        <source>Edit final command</source>
        <translation>Endgültigen Befehl bearbeiten</translation>
    </message>
    <message>
        <source>Command Window</source>
        <translation>Befehlsfenster</translation>
    </message>
    <message>
        <source>Licq Utility: %1</source>
        <translation>Licq-Werkzeug: %1</translation>
    </message>
    <message>
        <source>C&amp;lose</source>
        <translation>S&amp;chließen</translation>
    </message>
    <message>
        <source>User Fields</source>
        <translation>Benutzer-Felder</translation>
    </message>
    <message>
        <source>Description:</source>
        <translation>Beschreibung:</translation>
    </message>
    <message>
        <source>Internal</source>
        <translation>Intern</translation>
    </message>
    <message>
        <source>Command:</source>
        <translation>Befehl:</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>Ab&amp;brechen</translation>
    </message>
    <message>
        <source>Running:</source>
        <translation>Wird ausgeführt:</translation>
    </message>
    <message>
        <source>Failed:</source>
        <translation>Fehlgeschlagen:</translation>
    </message>
    <message>
        <source>Terminal</source>
        <translation>Terminal</translation>
    </message>
    <message>
        <source>Window:</source>
        <translation>Fenster:</translation>
    </message>
</context>
<context>
    <name>ChatDlg</name>
    <message>
        <source>No</source>
        <translation>Nein</translation>
    </message>
    <message>
        <source>Yes</source>
        <translation>Ja</translation>
    </message>
    <message>
        <source>Beep</source>
        <translation>Piepton</translation>
    </message>
    <message>
        <source>Bold</source>
        <translation>Fett</translation>
    </message>
    <message>
        <source>Chat</source>
        <translation>Chat</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation>Ansicht</translation>
    </message>
    <message>
        <source>Toggles Bold font</source>
        <translation>Fettschrift</translation>
    </message>
    <message>
        <source>Set Encoding</source>
        <translation>Zeichensatz einstellen</translation>
    </message>
    <message>
        <source>Ignores user color settings</source>
        <translation>Ignoriert die Farbeinstellung des Benutzers</translation>
    </message>
    <message>
        <source>Toggles Italic font</source>
        <translation>Kursivschrift</translation>
    </message>
    <message>
        <source>Remote - Waiting for joiners...</source>
        <translation>Gegenstelle - Auf Teilnehmer warten...</translation>
    </message>
    <message>
        <source>Licq - Chat</source>
        <translation>Licq - Chat</translation>
    </message>
    <message>
        <source>&amp;Audio</source>
        <translation>&amp;Sound</translation>
    </message>
    <message>
        <source>Sends a Beep to all recipients</source>
        <translation>Sendet einen Piepton an alle Teilnehmer</translation>
    </message>
    <message>
        <source>Failed to open file:
%1</source>
        <translation>Fehler beim öffnen der Datei:
%1</translation>
    </message>
    <message>
        <source>Unable to create new thread.
See Network Window for details.</source>
        <translation>Kann keinen neuen Thread erzeugen.
Siehe Netzwerkfenster für Details.</translation>
    </message>
    <message>
        <source>&amp;Pane Mode</source>
        <translation>2-&amp;Fenster-Modus</translation>
    </message>
    <message>
        <source>Italic</source>
        <translation>Kursiv</translation>
    </message>
    <message>
        <source>Do you want to save the chat session?</source>
        <translation>Soll das Protokoll dieser Sitzung gespeichert werden?</translation>
    </message>
    <message>
        <source>Licq - Chat %1</source>
        <translation>Licq - Chat %1</translation>
    </message>
    <message>
        <source>&amp;IRC Mode</source>
        <translation>&amp;IRC-Modus</translation>
    </message>
    <message>
        <source>Changes the foreground color</source>
        <translation>Ändert die Vordergrundfarbe</translation>
    </message>
    <message>
        <source>Unable to load encoding &lt;b&gt;%1&lt;/b&gt;. Message contents may appear garbled.</source>
        <translation>Zeichensatz &lt;b&gt;%1&lt;/b&gt; ließ sich nicht laden. Nachrichteninhalte könnten verstümmelt erscheinen.</translation>
    </message>
    <message>
        <source>Background color</source>
        <translation>Hintergrundfarbe</translation>
    </message>
    <message>
        <source>Local - %1</source>
        <translation>Lokal - %1</translation>
    </message>
    <message>
        <source>/%1.chat</source>
        <translation>/%1.chat</translation>
    </message>
    <message>
        <source>Remote - Not connected</source>
        <translation>Gegenstelle - Nicht verbunden</translation>
    </message>
    <message>
        <source>StrikeOut</source>
        <translation>Durchgestrichen</translation>
    </message>
    <message>
        <source>&amp;Close Chat</source>
        <translation>Chat S&amp;chließen</translation>
    </message>
    <message>
        <source>
&lt;--BEEP--&gt;
</source>
        <translation> 
&lt;--BEEP--&gt;
</translation>
    </message>
    <message>
        <source>&gt; &lt;--BEEP--&gt;
</source>
        <translation>&gt; &lt;--BEEP--&gt;
</translation>
    </message>
    <message>
        <source>Toggles StrikeOut font</source>
        <translation>Schaltet durchgestrichenen Text ein/aus</translation>
    </message>
    <message>
        <source>Underline</source>
        <translation>Unterstrichen</translation>
    </message>
    <message>
        <source>%1 closed connection.</source>
        <translation>%1 hat die Verbindung beendet.</translation>
    </message>
    <message>
        <source>Remote - Connecting...</source>
        <translation>Gegenstelle - Verbinden...</translation>
    </message>
    <message>
        <source>&amp;Save Chat</source>
        <translation>Protokoll &amp;speichern</translation>
    </message>
    <message>
        <source>Changes the background color</source>
        <translation>Ändert die Hintergrundfarbe</translation>
    </message>
    <message>
        <source>Foreground color</source>
        <translation>Vordergrundfarbe</translation>
    </message>
    <message>
        <source>Unable to bind to a port.
See Network Window for details.</source>
        <translation>Kann keinen Port binden.
Siehe Netzwerkfenster für Details.</translation>
    </message>
    <message>
        <source>Ignore user settings</source>
        <translation>Benutzereinstellungen ignorieren</translation>
    </message>
    <message>
        <source>Unable to connect to the remote chat.
See Network Window for details.</source>
        <translation>Kann nicht mit Gegenstelle für Chat verbinden.
Siehe Netzwerkfenster für Details.</translation>
    </message>
    <message>
        <source>Toggles Underline font</source>
        <translation>Schaltet unterstrichenen Text ein/aus</translation>
    </message>
</context>
<context>
    <name>CustomAwayMsgDlg</name>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;Ok</translation>
    </message>
    <message>
        <source>&amp;Clear</source>
        <translation>&amp;Löschen</translation>
    </message>
    <message>
        <source>&amp;Hints</source>
        <translation>&amp;Tipps</translation>
    </message>
    <message>
        <source>Set Custom Auto Response for %1</source>
        <translation>Spezielle Auto-Antwort für %1 einstellen</translation>
    </message>
    <message>
        <source>I am currently %1.
You can leave me a message.</source>
        <translation>Ich bin momentan %1.
Sie können mir eine Nachricht hinterlassen.</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>Ab&amp;brechen</translation>
    </message>
</context>
<context>
    <name>EditCategoryDlg</name>
    <message>
        <source>&amp;OK</source>
        <translation>&amp;Ok</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>Ab&amp;brechen</translation>
    </message>
    <message>
        <source>Unspecified</source>
        <translation>Undefiniert</translation>
    </message>
</context>
<context>
    <name>EditFileDlg</name>
    <message>
        <source>&amp;Save</source>
        <translation>&amp;Speichern</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>S&amp;chließen</translation>
    </message>
    <message>
        <source>Failed to open file:
%1</source>
        <translation>Fehler beim Öffnen der Datei:
%1</translation>
    </message>
    <message>
        <source>Licq File Editor - %1</source>
        <translation>Licq Datei-Editor - %1</translation>
    </message>
    <message>
        <source>[ Read-Only ]</source>
        <translation> [ Nur lesen ]</translation>
    </message>
</context>
<context>
    <name>EditGrpDlg</name>
    <message>
        <source>Ok</source>
        <translation>Ok</translation>
    </message>
    <message>
        <source>Add</source>
        <translation>Hinzufügen</translation>
    </message>
    <message>
        <source>&amp;Done</source>
        <translation>&amp;Fertig</translation>
    </message>
    <message>
        <source>&amp;Save</source>
        <translation>&amp;Speichern</translation>
    </message>
    <message>
        <source>Edit Name</source>
        <translation>Umbenennen</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <source>Groups</source>
        <translation>Gruppen</translation>
    </message>
    <message>
        <source>Are you sure you want to remove
the group &apos;%1&apos;?</source>
        <translation>Soll die Gruppe &apos;%1&apos; wirklich entfernt werden?</translation>
    </message>
    <message>
        <source>Remove</source>
        <translation>Entfernen</translation>
    </message>
    <message>
        <source>noname</source>
        <translation>unbenannt</translation>
    </message>
    <message>
        <source>Save the name of a group being modified.</source>
        <translation>Speichert den Namen der gerade bearbeiteten Gruppe.</translation>
    </message>
    <message>
        <source>Edit group name (hit enter to save).</source>
        <translation>Markierte Gruppe umbenennen (Eingabetaste zum speichern).</translation>
    </message>
    <message>
        <source>Shift Down</source>
        <translation>Nach unten</translation>
    </message>
    <message>
        <source>Licq - Edit Groups</source>
        <translation>Licq - Gruppen bearbeiten</translation>
    </message>
    <message>
        <source>Shift Up</source>
        <translation>Nach oben</translation>
    </message>
</context>
<context>
    <name>EditPhoneDlg</name>
    <message>
        <source>@</source>
        <translation>@</translation>
    </message>
    <message>
        <source>&amp;OK</source>
        <translation>&amp;Ok</translation>
    </message>
    <message>
        <source>Fax</source>
        <translation>Fax</translation>
    </message>
    <message>
        <source>Please enter a phone number</source>
        <translation>Bitte eine Telefonnummer eingeben</translation>
    </message>
    <message>
        <source>Pager</source>
        <translation>Pieper</translation>
    </message>
    <message>
        <source>Phone</source>
        <translation>Telefon</translation>
    </message>
    <message>
        <source>Type:</source>
        <translation>Typ:</translation>
    </message>
    <message>
        <source>Custom</source>
        <translation>Benutzerdefiniert</translation>
    </message>
    <message>
        <source>Description:</source>
        <translation>Beschreibung:</translation>
    </message>
    <message>
        <source>Number:</source>
        <translation>Rufnummer:</translation>
    </message>
    <message>
        <source>Wireless Pager</source>
        <translation>Funkruf</translation>
    </message>
    <message>
        <source>Cellular SMS</source>
        <translation>SMS Mobil</translation>
    </message>
    <message>
        <source>Home Fax</source>
        <translation>Fax privat</translation>
    </message>
    <message>
        <source>Work Cellular</source>
        <translation>Mobiltelefon geschäftlich</translation>
    </message>
    <message>
        <source>Work Fax</source>
        <translation>Fax geschäftlich</translation>
    </message>
    <message>
        <source>Country:</source>
        <translation>Land:</translation>
    </message>
    <message>
        <source>Provider:</source>
        <translation>Anbieter:</translation>
    </message>
    <message>
        <source>Home Phone</source>
        <translation>Telefon privat</translation>
    </message>
    <message>
        <source>Private Cellular</source>
        <translation>Mobiltelefon privat</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>Ab&amp;brechen</translation>
    </message>
    <message>
        <source>E-mail Gateway:</source>
        <translation>E-mail-Gateway:</translation>
    </message>
    <message>
        <source>Cellular</source>
        <translation>Mobil</translation>
    </message>
    <message>
        <source>Extension:</source>
        <translation>Durchwahl:</translation>
    </message>
    <message>
        <source>Work Phone</source>
        <translation>Telefon geschäftlich</translation>
    </message>
    <message>
        <source>Network #/Area code:</source>
        <translation>Ortsvorwahl:</translation>
    </message>
    <message>
        <source>Remove leading 0s from Area Code/Network #</source>
        <translation>Führende Null (0) von der Ortsvorwahl entfernen</translation>
    </message>
</context>
<context>
    <name>Event</name>
    <message>
        <source></source>
        <translation> </translation>
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
        <source>Message</source>
        <translation>Nachricht</translation>
    </message>
    <message>
        <source>Added to Contact List</source>
        <translation>zur Kontaktliste hinzugefügt</translation>
    </message>
    <message>
        <source>Web Panel</source>
        <translation>Web-Panel</translation>
    </message>
    <message>
        <source>Plugin Event</source>
        <translation>Plugin-Ereignis</translation>
    </message>
    <message>
        <source>File Transfer</source>
        <translation>Datei-Übertagung</translation>
    </message>
    <message>
        <source>System Server Message</source>
        <translation>System-Server-Nachricht</translation>
    </message>
    <message>
        <source>(cancelled)</source>
        <translation>(abgebrochen)</translation>
    </message>
    <message>
        <source>Authorization Request</source>
        <translation>Autorisierungs-Anfrage</translation>
    </message>
    <message>
        <source>Authorization Refused</source>
        <translation>Autorisierung abgelehnt</translation>
    </message>
    <message>
        <source>Unknown Event</source>
        <translation>Unbekanntes Ereignis</translation>
    </message>
    <message>
        <source>Chat Request</source>
        <translation>Chat-Anfrage</translation>
    </message>
    <message>
        <source>Authorization Granted</source>
        <translation>Autorisierung gewährt</translation>
    </message>
    <message>
        <source>Email Pager</source>
        <translation>Email-Pager</translation>
    </message>
    <message>
        <source>Contact List</source>
        <translation>Kontaktliste</translation>
    </message>
</context>
<context>
    <name>GPGKeyManager</name>
    <message>
        <source>&amp;Add</source>
        <translation>&amp;Hinzufügen</translation>
    </message>
    <message>
        <source>User</source>
        <translation>Benutzer</translation>
    </message>
    <message>
        <source>Remove GPG key</source>
        <translation>GPG-Schlüssel entfernen</translation>
    </message>
    <message>
        <source>&amp;Edit</source>
        <translation>&amp;Bearbeiten</translation>
    </message>
    <message>
        <source>No passphrase set</source>
        <translation>Kein Passwort festgelegt</translation>
    </message>
    <message>
        <source>&lt;qt&gt;Drag&amp;Drop user to add to list.&lt;/qt&gt;</source>
        <translation>&lt;qt&gt;Ein Benutzer kann durch Ziehen und Ablegen zur Liste hinzugefügt werden.&lt;/qt&gt;</translation>
    </message>
    <message>
        <source>Not yet implemented. Use licq_gpg.conf.</source>
        <translation>Diese Funktion ist noch nicht fertig. Bitte licq_gpg.conf bearbeiten.</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Schließen</translation>
    </message>
    <message>
        <source>Active</source>
        <translation>Aktiv</translation>
    </message>
    <message>
        <source>Key ID</source>
        <translation>Schlüssel-ID</translation>
    </message>
    <message>
        <source>&amp;Set GPG Passphrase</source>
        <translation>GPG-Passwort &amp;festlegen</translation>
    </message>
    <message>
        <source>Licq GPG Key Manager</source>
        <translation>Licq GPG Schlüsselverwaltung</translation>
    </message>
    <message>
        <source>Set Passphrase</source>
        <translation>Passwort festlegen</translation>
    </message>
    <message>
        <source>Do you want to remove the GPG key? The key isn&apos;t deleted from your keyring.</source>
        <translation>Soll der GPG-Schlüssel entfernt werden? Der Schlüssel wird dadurch nicht aus dem Schlüsselbund gelöscht.</translation>
    </message>
    <message>
        <source>&amp;Remove</source>
        <translation>&amp;Entfernen</translation>
    </message>
    <message>
        <source>User Keys</source>
        <translation>Benutzer-Schlüssel</translation>
    </message>
    <message>
        <source>GPG Passphrase</source>
        <translation>GPG-Passwort</translation>
    </message>
</context>
<context>
    <name>GPGKeySelect</name>
    <message>
        <source>&amp;OK</source>
        <translation>&amp;Ok</translation>
    </message>
    <message>
        <source>Select a GPG key for user %1.</source>
        <translation>Bitte einen GPG-Schlüssel für den Benutzer %1 auswählen.</translation>
    </message>
    <message>
        <source>Select GPG Key for user %1</source>
        <translation>Bitte einen GPG-Schlüssel für den Benutzer %1 auswählen</translation>
    </message>
    <message>
        <source>Current key: %1</source>
        <translation>Aktueller Schlüssel: %1</translation>
    </message>
    <message>
        <source>Current key: No key selected</source>
        <translation>Aktueller Schlüssel: Kein Schlüssel ausgewählt</translation>
    </message>
    <message>
        <source>Use GPG Encryption</source>
        <translation>GPG-Verschlüsselung aktivieren</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>A&amp;bbrechen</translation>
    </message>
    <message>
        <source>&amp;No Key</source>
        <translation>&amp;Kein Schlüssel</translation>
    </message>
    <message>
        <source>Filter:</source>
        <translation>Filter:</translation>
    </message>
</context>
<context>
    <name>Groups</name>
    <message>
        <source>All Users</source>
        <translation>Alle Benutzer</translation>
    </message>
    <message>
        <source>Online Notify</source>
        <translation>Online-Benachrichtigung</translation>
    </message>
    <message>
        <source>Visible List</source>
        <translation>Sichtbar</translation>
    </message>
    <message>
        <source>Invisible List</source>
        <translation>Unsichtbar</translation>
    </message>
    <message>
        <source>Ignore List</source>
        <translation>Ignorieren</translation>
    </message>
    <message>
        <source>New Users</source>
        <translation>Neue Benutzer</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation>Unbekannt</translation>
    </message>
</context>
<context>
    <name>HintsDlg</name>
    <message>
        <source>&amp;Close</source>
        <translation>S&amp;chließen</translation>
    </message>
    <message>
        <source>Licq - Hints</source>
        <translation>Licq - Tipps</translation>
    </message>
</context>
<context>
    <name>IconManager_KDEStyle</name>
    <message>
        <source>&lt;br&gt;1 msg</source>
        <translation>&lt;br&gt;1 Nachricht</translation>
    </message>
    <message>
        <source>&lt;br&gt;%1 msgs</source>
        <translation>&lt;br&gt;%1 Nachrichten</translation>
    </message>
    <message>
        <source>&lt;br&gt;Left click - Show main window&lt;br&gt;Middle click - Show next message&lt;br&gt;Right click - System menu</source>
        <translation>&lt;br&gt;linker Mausklick - Hauptfenster&lt;br&gt;mittlerer Mausklick - Nächste Nachricht&lt;br&gt;rechter Mausklick - Systemmenü</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;b&gt;%1 system messages&lt;/b&gt;</source>
        <translation>&lt;br&gt;&lt;b&gt;%1 Systemnachricht(en)&lt;/b&gt;</translation>
    </message>
</context>
<context>
    <name>IconManager_Themed</name>
    <message>
        <source>Unable to load dock theme image
%1</source>
        <translation>Dock-Stil-Bild ließ sich nicht laden
%1</translation>
    </message>
    <message>
        <source>Unable to load dock theme file
(%1)
:%2</source>
        <translation>Dock-Stil-Datei ließ sich nicht laden
(%1)
:%2</translation>
    </message>
</context>
<context>
    <name>KeyListItem</name>
    <message>
        <source>No</source>
        <translation>Nein</translation>
    </message>
    <message>
        <source>Yes</source>
        <translation>Ja</translation>
    </message>
</context>
<context>
    <name>KeyRequestDlg</name>
    <message>
        <source>&amp;Send</source>
        <translation>&amp;Senden</translation>
    </message>
    <message>
        <source>Closing secure channel...</source>
        <translation>Verschlüsselte Verbindung wird geschlossen...</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>S&amp;chließen</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;ForestGreen&quot;&gt;Secure channel established.&lt;/font&gt;
</source>
        <translation>&lt;font color=&quot;ForestGreen&quot;&gt;Verschlüsselte Verbindung aufgebaut.&lt;/font&gt;
</translation>
    </message>
    <message>
        <source>Ready to request channel</source>
        <translation>Bereit zum Anfordern der Verbindung</translation>
    </message>
    <message>
        <source>Client does not support OpenSSL.
Rebuild Licq with OpenSSL support.</source>
        <translation>Ihr Licq-Client unterstützt kein OpenSSL
Bitte kompilieren Sie Licq mit OpenSSL Unterstützung neu.</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;blue&quot;&gt;Secure channel closed.&lt;/font&gt;
</source>
        <translation>&lt;font color=&quot;blue&quot;&gt;Verschlüsselte Verbindung geschlossen.&lt;/font&gt;
</translation>
    </message>
    <message>
        <source>Licq - Secure Channel with %1</source>
        <translation>Licq - Verschlüsselte Verbindung mit %1</translation>
    </message>
    <message>
        <source>Secure channel is established using SSL
with Diffie-Hellman key exchange and
the TLS version 1 protocol.

</source>
        <translation>Es wird eine verschlüsselte Verbindung über SSL mit
Diffie-Hellman Schlüsseltausch und
TLS-Protokoll Version 1 aufgebaut.

</translation>
    </message>
    <message>
        <source>Ready to close channel</source>
        <translation>Bereit zum Schließen der Verbindung</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;Could not connect to remote client.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;Mit der Gegenstelle konnte keine Verbindung hergestellt werden.&lt;/font&gt;</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;yellow&quot;&gt;Secure channel already established.&lt;/font&gt;
</source>
        <translation>&lt;font color=&quot;yellow&quot;&gt;Es besteht bereits eine verschlüsselte Verbindung.&lt;/font&gt;
</translation>
    </message>
    <message>
        <source>The remote uses Licq %1, however it
has no secure channel support compiled in.
This probably won&apos;t work.</source>
        <translation>Der entfernte Benutzer verwendet zwar Licq %1, doch er
hat keine SSL-Unterstützung einkompiliert.
Dies wird nicht funktionieren.</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;yellow&quot;&gt;Secure channel not established.&lt;/font&gt;
</source>
        <translation>&lt;font color=&quot;yellow&quot;&gt;Verschlüsselte Verbindung nicht aufgebaut.&lt;/font&gt;
</translation>
    </message>
    <message>
        <source>The remote uses Licq %1/SSL.</source>
        <translation>Der entfernte Benutzer verwendet Licq %1/SSL.</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;Remote client does not support OpenSSL.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;Die Gegenstelle unterstützt kein OpenSSL.&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Requesting secure channel...</source>
        <translation>Verschlüsselte Verbindung wird angefordert...</translation>
    </message>
    <message>
        <source>This only works with other Licq clients &gt;= v0.85
The remote doesn&apos;t seem to use such a client.
This might not work.</source>
        <translation>Dies funktioniert nur mit Licq-Clients ab Version 0.85
oder höher. Der entfernte Benutzer scheint keinen solchen Client zu
verwenden. Dies wird wahrscheinlich nicht funktionieren.</translation>
    </message>
</context>
<context>
    <name>KeyView</name>
    <message>
        <source>Name</source>
        <translation>Name</translation>
    </message>
</context>
<context>
    <name>MLEditWrap</name>
    <message>
        <source>Allow Tabulations</source>
        <translation>Tabulatoren erlauben</translation>
    </message>
</context>
<context>
    <name>MLView</name>
    <message>
        <source>Copy URL</source>
        <translation>URL kopieren</translation>
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
        <translation>Zeit</translation>
    </message>
    <message>
        <source>Cancelled Event</source>
        <translation>Ereignis abgebrochen</translation>
    </message>
    <message>
        <source>Direct</source>
        <translation>Direkt</translation>
    </message>
    <message>
        <source>Event Type</source>
        <translation>Ereignistyp</translation>
    </message>
    <message>
        <source>Server</source>
        <translation>Server</translation>
    </message>
    <message>
        <source>Urgent</source>
        <translation>Dringend</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>Optionen</translation>
    </message>
    <message>
        <source>Multiple Recipients</source>
        <translation>Mehrere Empfänger</translation>
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
        <translation><byte value="x9"/>bis</translation>
    </message>
    <message>
        <source>&amp;OK</source>
        <translation>&amp;Ok</translation>
    </message>
    <message>
        <source>Fax</source>
        <translation>Fax</translation>
    </message>
    <message>
        <source>Auto</source>
        <translation>Auto</translation>
    </message>
    <message>
        <source>Away</source>
        <translation>Abwesend</translation>
    </message>
    <message>
        <source>Font</source>
        <translation>Schriftart</translation>
    </message>
    <message>
        <source>N/A:</source>
        <translation>Nicht verfügbar:</translation>
    </message>
    <message>
        <source>Left</source>
        <translation>Links</translation>
    </message>
    <message>
        <source>Save</source>
        <translation>Speichern</translation>
    </message>
    <message>
        <source>URL:</source>
        <translation>URL:</translation>
    </message>
    <message>
        <source>none</source>
        <translation>keine</translation>
    </message>
    <message>
        <source>Show Extended Icons</source>
        <translation>Erweiterte Symbole</translation>
    </message>
    <message>
        <source>Blink All Events</source>
        <translation>Alle Ereignisse blinken</translation>
    </message>
    <message>
        <source>Away:</source>
        <translation>Abwesend:</translation>
    </message>
    <message>
        <source>Email</source>
        <translation>E-Mail</translation>
    </message>
    <message>
        <source>Font:</source>
        <translation>Schriftart (Allgemein):</translation>
    </message>
    <message>
        <source>HTTPS</source>
        <translation>HTTPS</translation>
    </message>
    <message>
        <source>Never</source>
        <translation>Niemals</translation>
    </message>
    <message>
        <source>Phone</source>
        <translation>Telefon</translation>
    </message>
    <message>
        <source>Proxy</source>
        <translation>Proxy</translation>
    </message>
    <message>
        <source>Right</source>
        <translation>Rechts</translation>
    </message>
    <message>
        <source>Text:</source>
        <translation>Text:</translation>
    </message>
    <message>
        <source>Title</source>
        <translation>Überschrift</translation>
    </message>
    <message>
        <source>Width</source>
        <translation>Breite</translation>
    </message>
    <message>
        <source>Parameter for received file transfers</source>
        <translation>Parameter für empfangene Dateiübertragung</translation>
    </message>
    <message>
        <source>Firewall</source>
        <translation>Firewall</translation>
    </message>
    <message>
        <source>Use proxy server</source>
        <translation>Proxy-Server verwenden</translation>
    </message>
    <message>
        <source>Show Grid Lines</source>
        <translation>Gitternetzlinien</translation>
    </message>
    <message>
        <source>Override the skin setting for the frame style of the user window:
   0 (No frame), 1 (Box), 2 (Panel), 3 (WinPanel)
 + 16 (Plain), 32 (Raised), 48 (Sunken)
 + 240 (Shadow)</source>
        <translation>Einstellungen für den Rahmen der Kontaktliste:

   0 (kein Rahmen), 1 (Box), 2 (Panel), 3 (WinPanel)
 + 16 (flach), 32 (erhöht), 48 (versunken)
 + 240 (schattiert)</translation>
    </message>
    <message>
        <source>Parameter for received chat requests</source>
        <translation>Parameter für empfangene Chat-Anfragen</translation>
    </message>
    <message>
        <source>Auto Away Messages</source>
        <translation>Automatische Antworten</translation>
    </message>
    <message>
        <source>Turns on or off the display of headers above each column in the user list</source>
        <translation>Schaltet die Spaltenüberschriften in der Kontaktliste ein oder aus.</translation>
    </message>
    <message>
        <source>Auto close the user function window after a successful event</source>
        <translation>Das Funktionsfenster soll nach einem erfolgreichen Ereignis automatisch geschlossen werden</translation>
    </message>
    <message>
        <source>Start Hidden</source>
        <translation>Hauptfenster ausgeblendet starten</translation>
    </message>
    <message>
        <source>Auto Away:</source>
        <translation>Automatisch abwesend:</translation>
    </message>
    <message>
        <source>Number of minutes of inactivity after which to automatically be marked &quot;not available&quot;.  Set to &quot;0&quot; to disable.</source>
        <translation>Anzahl der Minuten, nach denen man automatisch als &apos;Nicht erreichbar&apos; markiert wird. Auf 0 setzen, um diese Funktion auszuschalten.</translation>
    </message>
    <message>
        <source>Blink Urgent Events</source>
        <translation>Wichtige Ereignisse blinken</translation>
    </message>
    <message>
        <source>Ignore Mass Messages</source>
        <translation>Massen-Nachrichten ignorieren</translation>
    </message>
    <message>
        <source>Message Display</source>
        <translation>Nachrichtenanzeige</translation>
    </message>
    <message>
        <source>Show the &quot;--online--&quot; and &quot;--offline--&quot; bars in the contact list</source>
        <translation>Schaltet Trennlinien zwischen Online- und Offline-Benutzern in der Kontaktliste ein.</translation>
    </message>
    <message>
        <source>ICQ Server Port:</source>
        <translation>ICQ-Server-Port:</translation>
    </message>
    <message>
        <source>Start main window hidden. Only the dock icon will be visible.</source>
        <translation>Versteckt das Hauptfenster beim Programmstart. Nur das Dock-Symbol ist nach dem Start sichtbar.</translation>
    </message>
    <message>
        <source>Parameter for online notification</source>
        <translation>Parameter für Online-Benachrichtigung</translation>
    </message>
    <message>
        <source>TCP port range for incoming connections.</source>
        <translation>TCP Portbereich für eingehende Verbindungen.</translation>
    </message>
    <message>
        <source>Insert Horizontal Line</source>
        <translation>Horizontale Trennlinie</translation>
    </message>
    <message>
        <source>Date Format:</source>
        <translation>Datumsformatierung:</translation>
    </message>
    <message>
        <source>Accept Modes</source>
        <translation>Per-Status-Einstellungen für Ereignisse</translation>
    </message>
    <message>
        <source>Flash the Taskbar on incoming messages</source>
        <translation>Lässt die Programmleiste bei einer neuen Nachricht aufblinken.</translation>
    </message>
    <message>
        <source>Online Notify:</source>
        <translation>Online-Benachrichtigung:</translation>
    </message>
    <message>
        <source>Idle Time</source>
        <translation>Leerlaufzeit</translation>
    </message>
    <message>
        <source>Licq can reconnect you when you got disconnected because your Uin was used from another location. Check this if you want Licq to reconnect automatically.</source>
        <translation>Wenn die Verbindung beendet wurde, weil Ihre Uin von einem anderen Ort aus verwendet wurde, kann Licq die Verbindung wiederherstellen. Wenn Sie automatisch wiederverbunden werden möchten, aktivieren Sie diese Option.</translation>
    </message>
    <message>
        <source>Not Available</source>
        <translation>Nicht verfügbar</translation>
    </message>
    <message>
        <source>&amp;Apply</source>
        <translation>Über&amp;nehmen</translation>
    </message>
    <message>
        <source>Determines if new users are automatically added to your list or must first request authorization.</source>
        <translation>Sollen neue Benutzer automatisch zur Liste hinzugefügt werden oder müssen sie erst nach Autorisierung fragen?</translation>
    </message>
    <message>
        <source>Show the name of the current group in the messages label when there are no new messages</source>
        <translation>Name der Benutzergruppe im Info-Bereich anzeigen, während keine neuen Nachrichten vorhanden sind.</translation>
    </message>
    <message>
        <source>Ignore Web Panel</source>
        <translation>Web-Panel ignorieren</translation>
    </message>
    <message>
        <source>Sets which default encoding should be used for newly added contacts.</source>
        <translation>Legt fest, welcher Zeichensatz für neue Benutzer verwendet werden soll.</translation>
    </message>
    <message>
        <source>Show the current chat history in Send Window</source>
        <translation>Nachrichten in einem Chatähnlichen Fenster anzeigen.</translation>
    </message>
    <message>
        <source>Small Icon</source>
        <translation>Kleines Symbol</translation>
    </message>
    <message>
        <source>Use Font Styles</source>
        <translation>Fett-/Kursivschrift</translation>
    </message>
    <message>
        <source>Preset slot:</source>
        <translation>Voreinstellung:</translation>
    </message>
    <message>
        <source>Automatically send messages through the server if direct connection fails</source>
        <translation>Falls keine direkte Verbindung möglich ist, werden Nachrichten automatisch über den Server gesendet.</translation>
    </message>
    <message>
        <source>Licq Options</source>
        <translation>Licq-Optionen</translation>
    </message>
    <message>
        <source>status + new messages</source>
        <translation>Status + neue Nachrichten</translation>
    </message>
    <message>
        <source>Show online notify users who are offline even when offline users are hidden.</source>
        <translation>Benutzer mit Online-Benachrichtigung immer in der Kontaktliste anzeigen, auch wenn Offline-Benutzer ausgeblendet sind.</translation>
    </message>
    <message>
        <source>Show Column Headers</source>
        <translation>Spaltenüberschriften</translation>
    </message>
    <message>
        <source>Bold Message Label on Incoming Msg</source>
        <translation>Fettschrift im Info-Bereich</translation>
    </message>
    <message>
        <source>Column %1</source>
        <translation>Spalte %1</translation>
    </message>
    <message>
        <source>Selects between the standard 64x64 icon used in the WindowMaker/Afterstep wharf and a shorter 64x48 icon for use in the Gnome/KDE panel.</source>
        <translation>Wählt zwischen dem standard 64x64 Pixel Symbol (Windowmaker/Afterstep) und dem kürzeren 64x48 Pixel Symbol (KDE/GNOME).</translation>
    </message>
    <message>
        <source>Perform OnEvent command in occupied mode</source>
        <translation>Kommando auch bei Status &apos;Beschäftigt&apos; ausführen</translation>
    </message>
    <message>
        <source>Auto Offline:</source>
        <translation>Automatisch offline:</translation>
    </message>
    <message>
        <source>Online Time</source>
        <translation>Online-Zeit</translation>
    </message>
    <message>
        <source>Lets you drag around the main window with your mouse</source>
        <translation>Ermöglicht das Ziehen des Hauptfensters mit der Maus.</translation>
    </message>
    <message>
        <source>OnEvent in DND</source>
        <translation>Aktiviert bei Status: &apos;Nicht stören&apos;</translation>
    </message>
    <message>
        <source>OnEvent in N/A</source>
        <translation>Aktiviert bei Status: &apos;Nicht verfügbar&apos;</translation>
    </message>
    <message>
        <source>History Sent:</source>
        <translation>Verlauf, gesendet:</translation>
    </message>
    <message>
        <source>Parameter for received messages</source>
        <translation>Parameter für empfangene Nachrichten</translation>
    </message>
    <message>
        <source>Message Sent:</source>
        <translation>Nachricht gesendet:</translation>
    </message>
    <message>
        <source>Center</source>
        <translation>Zentriert</translation>
    </message>
    <message>
        <source>Colors</source>
        <translation>Farben</translation>
    </message>
    <message>
        <source>Proxy Server Port:</source>
        <translation>Proxy-Server-Port:</translation>
    </message>
    <message>
        <source>Allow scroll bar</source>
        <translation>Bildlaufleiste zulassen</translation>
    </message>
    <message>
        <source>Connection</source>
        <translation>Verbindung</translation>
    </message>
    <message>
        <source>OnEvent in Away</source>
        <translation>Aktiviert bei Status: &apos;Abwesend&apos;</translation>
    </message>
    <message>
        <source>Network</source>
        <translation>Netzwerk</translation>
    </message>
    <message>
        <source>Only urgent events will blink</source>
        <translation>Nur wichtige/dringende Nachrichten blinken</translation>
    </message>
    <message>
        <source>Format</source>
        <translation>Format</translation>
    </message>
    <message>
        <source>The fonts used</source>
        <translation>Die verwendeten Schriftarten</translation>
    </message>
    <message>
        <source>Number of minutes of inactivity after which to automatically go offline.  Set to &quot;0&quot; to disable.</source>
        <translation>Anzahl der Minuten, nach denen automatisch der Status &apos;Offline&apos; gesetzt wird.  Zum Abschalten auf &quot;0&quot; setzen.</translation>
    </message>
    <message>
        <source>Auto send through server</source>
        <translation>Automatisch über Server senden</translation>
    </message>
    <message>
        <source>Use Dock Icon</source>
        <translation>Dock-Symbol benutzen</translation>
    </message>
    <message>
        <source>Chat Request:</source>
        <translation>Chat-Einladung:</translation>
    </message>
    <message>
        <source>Online</source>
        <translation>Online</translation>
    </message>
    <message>
        <source>Server settings</source>
        <translation>Server-Einstellungen</translation>
    </message>
    <message>
        <source>Auto Logon:</source>
        <translation>Automatische Anmeldung:</translation>
    </message>
    <message>
        <source>Main Window</source>
        <translation>Hauptfenster</translation>
    </message>
    <message>
        <source>Auto Update</source>
        <translation>Automatisches aktualisieren</translation>
    </message>
    <message>
        <source>Status</source>
        <translation>Status</translation>
    </message>
    <message>
        <source>Style:</source>
        <translation>Stil:</translation>
    </message>
    <message>
        <source>Theme:</source>
        <translation>Stil:</translation>
    </message>
    <message>
        <source>Always show online notify users</source>
        <translation>Online-Benachrichtungs-
Benutzer immer anzeigen</translation>
    </message>
    <message>
        <source>default (%1)</source>
        <translation>standard (%1)</translation>
    </message>
    <message>
        <source>Offline</source>
        <translation>Offline</translation>
    </message>
    <message>
        <source>Port Range:</source>
        <translation>Portbereich:</translation>
    </message>
    <message>
        <source>Protocol ID</source>
        <translation>Protokoll-ID</translation>
    </message>
    <message>
        <source>Allow dragging main window</source>
        <translation>Hauptfenster ziehen zulassen</translation>
    </message>
    <message>
        <source>Show the message info label in bold font if there are incoming messages</source>
        <translation>Stellt die Schrift im Info-Bereich fett ein, wenn neue Nachrichten empfangen wurden.</translation>
    </message>
    <message>
        <source>If not checked, a user will be automatically removed from &quot;New User&quot; group when you first send an event to them</source>
        <translation>Bei manueller Verwaltung werden neue Benutzer nicht automatisch nach der ersten gesendeten Nachricht aus der Gruppe &quot;Neue Benutzer&quot; entfernt.</translation>
    </message>
    <message>
        <source>Tabbed Chatting</source>
        <translation>Unterfenster</translation>
    </message>
    <message>
        <source>Default Encoding:</source>
        <translation>Standard-Zeichensatz:</translation>
    </message>
    <message>
        <source>OnEvent</source>
        <translation>Ereignisse</translation>
    </message>
    <message>
        <source>Localization</source>
        <translation>Lokalisierung</translation>
    </message>
    <message>
        <source>Command:</source>
        <translation>Befehl:</translation>
    </message>
    <message>
        <source>Edit Font:</source>
        <translation>Schriftart (Editor):</translation>
    </message>
    <message>
        <source>Hot key: </source>
        <translation>Hotkey: </translation>
    </message>
    <message>
        <source>I can receive direct connections</source>
        <translation>Direktverbindungen sind möglich</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>Optionen</translation>
    </message>
    <message>
        <source>Miscellaneous</source>
        <translation>Verschiedenes</translation>
    </message>
    <message>
        <source>Perform OnEvent command in do not disturb mode</source>
        <translation>Kommando auch bei Status &apos;nicht stören&apos; ausführen</translation>
    </message>
    <message>
        <source>Determines if mass messages are ignored or not.</source>
        <translation>Ignoriert Massennachrichten (Nachrichten die an sehr viele Benutzer gleichzeitig verschickt wurden)</translation>
    </message>
    <message>
        <source>Last online</source>
        <translation>Zuletzt online</translation>
    </message>
    <message>
        <source>Frame Style: </source>
        <translation>Rahmenstil: </translation>
    </message>
    <message>
        <source>The alignment of the column</source>
        <translation>Die Ausrichtung der Spalten in der Kontaktliste</translation>
    </message>
    <message>
        <source>status</source>
        <translation>Status</translation>
    </message>
    <message>
        <source>Auto Position the Reply Window</source>
        <translation>Intelligente Antwort-Fenster</translation>
    </message>
    <message>
        <source>Make the user window transparent when there is no scroll bar</source>
        <translation>Macht das Benutzer-Fenster transparent, wenn kein Rollbalken vorhanden ist. Somit scheint der Hintergrund des Hauptfensters durch.</translation>
    </message>
    <message>
        <source>Default Icon</source>
        <translation>Standard-Symbol</translation>
    </message>
    <message>
        <source>Check Clipboard For Urls/Files</source>
        <translation>Intelligente Zwischenablage</translation>
    </message>
    <message>
        <source>Typing Notification Color:</source>
        <translation>Tipp-Benachrichtigung:</translation>
    </message>
    <message>
        <source>Parameters</source>
        <translation>Parameter</translation>
    </message>
    <message>
        <source>Show birthday, invisible, and custom auto response icons to the right of users in the list</source>
        <translation>Erweiterte Symbole für z. B. Geburtstag, unsichtbar und spezielle Auto-Antwort in der Kontaktliste neben den Benutzernamen einblenden.</translation>
    </message>
    <message>
        <source>Additional &amp;sorting:</source>
        <translation>Erweitertes &amp;Sortieren:</translation>
    </message>
    <message>
        <source>Transparent when possible</source>
        <translation>Transparenz</translation>
    </message>
    <message>
        <source>Default Auto Response Messages</source>
        <translation>Standard-Nachricht für automatische Antworten</translation>
    </message>
    <message>
        <source>File Transfer:</source>
        <translation>Dateiübertragung:</translation>
    </message>
    <message>
        <source>OnEvents Enabled</source>
        <translation>Ereignisse aktivieren</translation>
    </message>
    <message>
        <source>Paranoia</source>
        <translation>Paranoia</translation>
    </message>
    <message>
        <source>Auto-Raise on Incoming Msg</source>
        <translation>Hauptfenster nach vorne</translation>
    </message>
    <message>
        <source>Position a new reply window just underneath the message view window</source>
        <translation>Zeigt das Antwort-Fenster möglichst in der Nähe des Nachrichten-Fensters an.</translation>
    </message>
    <message>
        <source>Automatically update users&apos; server stored information.</source>
        <translation>Aktualisiert automatisch die vom Benutzer auf dem Server gespeicherten Informationen.</translation>
    </message>
    <message>
        <source>Terminal:</source>
        <translation>Terminal:</translation>
    </message>
    <message>
        <source>Automatically log on when first starting up.</source>
        <translation>Beim Programmstart automatisch mit diesem Status anmelden</translation>
    </message>
    <message>
        <source>Perform the online notify OnEvent when logging on (this is different from how the Mirabilis client works)</source>
        <translation>Ausführen der Online-Benachrichtigung beim Einloggen.</translation>
    </message>
    <message>
        <source>Number of minutes of inactivity after which to automatically be marked &quot;away&quot;.  Set to &quot;0&quot; to disable.</source>
        <translation>Anzahl der Minuten, nach denen man automatisch als &apos;Abwesend&apos; markiert wird. Auf 0 setzen, um diese Funktion auszuschalten.</translation>
    </message>
    <message>
        <source>&lt;p&gt;Command to execute when an event is received.&lt;br&gt;It will be passed the relevant parameters from below.&lt;br&gt;Parameters can contain the following expressions &lt;br&gt; which will be replaced with the relevant information:&lt;/p&gt;</source>
        <translation>&lt;p&gt;Kommando, das ausgeführt werden soll, wenn ein Ereignis eintritt.&lt;br&gt;Die unten stehenden Parameter werden übergeben.&lt;br&gt;Parameter können folgende Platzhalter enthalten,&lt;br&gt;die durch die jeweilige Information ersetzt werden:&lt;/p&gt;</translation>
    </message>
    <message>
        <source>ICQ Server:</source>
        <translation>ICQ-Server:</translation>
    </message>
    <message>
        <source>Determines if email pager messages are ignored or not.</source>
        <translation>Ignoriert Nachrichten, die über den Email Pager verschickt wurden.</translation>
    </message>
    <message>
        <source>Use authorization</source>
        <translation>Passwort erforderlich</translation>
    </message>
    <message>
        <source>Perform OnEvent command in not available mode</source>
        <translation>Kommando auch bei Status &apos;nicht verfügbar&apos; ausführen</translation>
    </message>
    <message>
        <source>Occupied</source>
        <translation>Beschäftigt</translation>
    </message>
    <message>
        <source>Reconnect after Uin clash</source>
        <translation>Nach Uin-Konflikt erneut verbinden</translation>
    </message>
    <message>
        <source>Show User Dividers</source>
        <translation>Trennlinien (On-/Offline)</translation>
    </message>
    <message>
        <source>OnEvent in Occupied</source>
        <translation>Aktiviert bei Status: &apos;Beschäftigt&apos;</translation>
    </message>
    <message>
        <source>Parameter for received system messages</source>
        <translation>Parameter für empfangene Systemnachrichten</translation>
    </message>
    <message>
        <source>Manual &quot;New User&quot; group handling</source>
        <translation>Neue Benutzer manuell verwalten</translation>
    </message>
    <message>
        <source>Parameter for sent messages</source>
        <translation>Parameter für gesendete Nachrichten</translation>
    </message>
    <message>
        <source>Select Font</source>
        <translation>Schriftart auswählen</translation>
    </message>
    <message>
        <source>Flash Taskbar on Incoming Msg</source>
        <translation>Blinken bei neuer Nachricht</translation>
    </message>
    <message>
        <source>Show recent messages</source>
        <translation>Letzten Verlauf anzeigen</translation>
    </message>
    <message>
        <source>Background Color:</source>
        <translation>Hintergrundfarbe:</translation>
    </message>
    <message>
        <source>Auto-Popup Incoming Msg</source>
        <translation>Nachrichten automatisch öffnen</translation>
    </message>
    <message>
        <source>64 x 48 Dock Icon</source>
        <translation>64 x 48 Dock-Symbol</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>Ab&amp;brechen</translation>
    </message>
    <message>
        <source>Startup</source>
        <translation>Programmstart</translation>
    </message>
    <message>
        <source>Status:</source>
        <translation>Status:</translation>
    </message>
    <message>
        <source>Password:</source>
        <translation>Passwort:</translation>
    </message>
    <message>
        <source>Automatically update users&apos; Phone &quot;Follow Me&quot;, File Server and ICQphone status.</source>
        <translation>Aktualisiert automatisch die auf dem Server gespeicherten Informationen zu &quot;Telefon folgen&quot;, Dateiserver und ICQphone.</translation>
    </message>
    <message>
        <source>default</source>
        <translation>standard</translation>
    </message>
    <message>
        <source>Show the last 5 messages when a Send Window is opened</source>
        <translation>Zeigt in neuen Nachrichtenfenstern die jeweils 5 letzten Nachrichten des Benutzers an.</translation>
    </message>
    <message>
        <source>Use italics and bold in the user list to indicate special characteristics such as online notify and visible list</source>
        <translation>Besondere Benutzereigenschaften wie &apos;Online-Benachrichtigung&apos; oder &apos;Sichtbar&apos; werden durch Kursiv- oder Fettdruck des Aliasnamens dargestellt</translation>
    </message>
    <message>
        <source>Use System Background Color</source>
        <translation>System-Hintergrundfarbe</translation>
    </message>
    <message>
        <source>Proxy Type:</source>
        <translation>Proxy-Typ:</translation>
    </message>
    <message>
        <source>Auto N/A:</source>
        <translation>Automatisch nicht verfügbar:</translation>
    </message>
    <message>
        <source>Auto Update Info Plugins</source>
        <translation>Info-Plugins automatisch aktualisieren</translation>
    </message>
    <message>
        <source>Auto Update Info</source>
        <translation>Info automatisch aktualisieren</translation>
    </message>
    <message>
        <source>Themed Icon</source>
        <translation>Thematisiertes Symbol</translation>
    </message>
    <message>
        <source>Auto Close Function Window</source>
        <translation>Fenster automatisch schließen</translation>
    </message>
    <message>
        <source>Perform OnEvent command in away mode</source>
        <translation>Kommando auch bei Status &apos;Abwesend&apos; ausführen</translation>
    </message>
    <message>
        <source>Docking</source>
        <translation>Andocken</translation>
    </message>
    <message>
        <source>I am behind a firewall</source>
        <translation>Licq ist hinter einer Firewall</translation>
    </message>
    <message>
        <source>Auto Update Status Plugins</source>
        <translation>Status-Plugins automatisch aktualisieren</translation>
    </message>
    <message>
        <source>Determines if web panel messages are ignored or not.</source>
        <translation>Ignoriert Nachrichten, die über eine Web-Oberfläche gesendet wurden.</translation>
    </message>
    <message>
        <source>Enable running of &quot;Command&quot; when the relevant event occurs.</source>
        <translation>Das Ausführen von &quot;Befehl&quot; bei bestimmten Ereignissen zulassen</translation>
    </message>
    <message>
        <source>System Msg:</source>
        <translation>Systemnachricht:</translation>
    </message>
    <message>
        <source>Show group name if no messages</source>
        <translation>Gruppenname im Info-Bereich</translation>
    </message>
    <message>
        <source>Free for Chat</source>
        <translation>Frei für Chat</translation>
    </message>
    <message>
        <source>Free For Chat</source>
        <translation>Frei für Chat</translation>
    </message>
    <message>
        <source>When double-clicking on a user to send a message check for urls/files in the clipboard</source>
        <translation>Überprüft bei Doppelklick auf einen Benutzer die Zwischenablage auf Urls/Dateien und fügt deren Inhalt automatisch in die Nachricht ein.</translation>
    </message>
    <message>
        <source>Show all encodings</source>
        <translation>Alle Zeichensätze</translation>
    </message>
    <message>
        <source>Online Notify when Logging On</source>
        <translation>Online-Benachrichtigung bei Anmeldung</translation>
    </message>
    <message>
        <source>Popup info</source>
        <translation>Popup-Info für Benutzer</translation>
    </message>
    <message>
        <source>Cellular</source>
        <translation>Mobil</translation>
    </message>
    <message>
        <source>Extensions</source>
        <translation>Erweiterungen</translation>
    </message>
    <message>
        <source>Do Not Disturb</source>
        <translation>Nicht stören</translation>
    </message>
    <message>
        <source>All incoming events will blink</source>
        <translation>Alle Ereignisse blinken</translation>
    </message>
    <message>
        <source>Username:</source>
        <translation>Benutzername:</translation>
    </message>
    <message>
        <source>Url Viewer:</source>
        <translation>URL-Betrachter:</translation>
    </message>
    <message>
        <source>Message:</source>
        <translation>Nachricht:</translation>
    </message>
    <message>
        <source>Draw boxes around each square in the user list</source>
        <translation>Rahmen um jedes Feld der Kontaktliste zeichnen.</translation>
    </message>
    <message>
        <source>Show all available encodings in the User Encoding selection menu. Normally, this menu shows only commonly used encodings.</source>
        <translation>Zeigt alle verfügbaren Zeichensätze im Auswahlmenü an. Ansonsten enthält das Menü nur die üblichsten Zeichensätze.</translation>
    </message>
    <message>
        <source>Column Configuration</source>
        <translation>Spalten in der Kontaktliste</translation>
    </message>
    <message>
        <source>Invisible</source>
        <translation>Unsichtbar</translation>
    </message>
    <message>
        <source>&lt;b&gt;none:&lt;/b&gt; - Don&apos;t sort online users by Status&lt;br&gt;
&lt;b&gt;status&lt;/b&gt; - Sort online users by status&lt;br&gt;
&lt;b&gt;status + last event&lt;/b&gt; - Sort online users by status and by last event&lt;br&gt;
&lt;b&gt;status + new messages&lt;/b&gt; - Sort online users by status and number of new messages</source>
        <translation>&lt;b&gt;keine&lt;/b&gt; - Nicht nach Status sortieren&lt;br&gt;
&lt;b&gt;Status&lt;/b&gt; - Nach Status sortieren&lt;br&gt;
&lt;b&gt;Status + letztes Ereignis&lt;/b&gt; - Nach Status und dem letzten empfangenen Ereignis sortieren&lt;br&gt;
&lt;b&gt;Status + Neue Nachrichten&lt;/b&gt; - Nach Status und der Anzahl neuer Nachrichten sortieren</translation>
    </message>
    <message>
        <source>Open all incoming messages automatically when received if we are online (or free for chat)</source>
        <translation>Öffnet neue Nachrichten automatisch (bei Status Online bzw. Frei für Chat)</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Allgemein</translation>
    </message>
    <message>
        <source>Ignore Email Pager</source>
        <translation>E-Mail-Pager ignorieren</translation>
    </message>
    <message>
        <source>Chatmode Messageview</source>
        <translation>Nachrichten in Chatansicht</translation>
    </message>
    <message>
        <source>Controls whether or not the dockable icon should be displayed.</source>
        <translation>Legt fest, ob das Dock-Symbol angezeigt werden soll.</translation>
    </message>
    <message>
        <source>status + last event</source>
        <translation>Status + letztes Ereignis</translation>
    </message>
    <message>
        <source>Uses the freedesktop.org standard to dock a small icon into the window manager.  Works with many different window managers.</source>
        <translation>Verwendet den freedesktop.org-Standard zum Andocken eines kleinen Symbols in den Fenstermanager.  Dies funktioniert mit vielen verschiedenen Fenstermanagern.</translation>
    </message>
    <message>
        <source>Message Received:</source>
        <translation>Nachricht empfangen:</translation>
    </message>
    <message>
        <source>Automatically update users&apos; Phone Book and Picture.</source>
        <translation>Aktualisiert automatisch das auf dem Server gespeicherte Telefonbuch und Bild.</translation>
    </message>
    <message>
        <source>Proxy Server:</source>
        <translation>Proxy-Server:</translation>
    </message>
    <message>
        <source>Raise the main window on incoming messages</source>
        <translation>Holt das Licq-Hauptfenster automatisch nach vorne, sobald eine Nachricht empfangen wurde.</translation>
    </message>
    <message>
        <source>Contact List</source>
        <translation>Kontaktliste</translation>
    </message>
    <message>
        <source>System default (%1)</source>
        <translation>Systemeinstellung (%1)</translation>
    </message>
    <message>
        <source>The width of the column</source>
        <translation>Die Breite der Spalte</translation>
    </message>
    <message>
        <source>Previous Message</source>
        <translation>Vorherige Nachricht</translation>
    </message>
    <message>
        <source>Select a font from the system list</source>
        <translation>Schriftart für die Licq Programmfenster auswählen</translation>
    </message>
    <message>
        <source>The string which will appear in the list box column header</source>
        <translation>Die Spaltenüberschrift, die im Kopf der Kontaktliste erscheint</translation>
    </message>
    <message>
        <source>Store your contacts on the server so they are accessible from different locations and/or programs</source>
        <translation>Aktiviert die Unterstützung für die serverseitige Kontaktliste.
Wenn eingeschaltet, wird die Kontaktliste auch auf dem ICQ-Server gespeichert, sodass sie von verschiedenen Standorten oder Programmen aus verfügbar ist.</translation>
    </message>
    <message>
        <source>Parameter for received URLs</source>
        <translation>Parameter für empfangene URLs</translation>
    </message>
    <message>
        <source>The command to run to view a URL.  Will be passed the URL as a parameter.</source>
        <translation>Das Kommando zum Anzeigen einer URL. Die URL wird als Parameter übergeben.</translation>
    </message>
    <message>
        <source>Allow the vertical scroll bar in the user list</source>
        <translation>Eine vertikale Bildlaufleiste in der Kontaktliste anzeigen wenn nötig.
Wenn aktiviert, wird bei Bedarf eine Bildlaufleiste eingeblendet.</translation>
    </message>
    <message>
        <source>History Received:</source>
        <translation>Verlauf, empfangen:</translation>
    </message>
    <message>
        <source>The command to run to start your terminal program.</source>
        <translation>Das Kommando zum Starten des Terminal-Programms.</translation>
    </message>
    <message>
        <source>Use tabs in Send Window</source>
        <translation>Durch das Aktivieren von Unterfenstern können mehrere Nachrichtenfenster in einem kombiniert werden.</translation>
    </message>
    <message>
        <source>Font used in message editor etc.</source>
        <translation>Schriftart für Nachrichteneditor usw.</translation>
    </message>
    <message>
        <source>Ignore New Users</source>
        <translation>Neue Benutzer ignorieren</translation>
    </message>
    <message>
        <source>Use server side contact list</source>
        <translation>Serverseitige Kontaktliste</translation>
    </message>
    <message>
        <source>Alignment</source>
        <translation>Ausrichtung</translation>
    </message>
    <message>
        <source>Send typing notifications</source>
        <translation>Sende Benachrichtigung beim Tippen</translation>
    </message>
    <message>
        <source>Send a notification to the user so they can see when you are typing a message to them</source>
        <translation>Sendet eine Benachrichtigung an den Benutzer, so dass er sehen kann, wenn Du ihm eine Nachricht schreibst</translation>
    </message>
    <message>
        <source>Sticky Main Window</source>
        <translation>Hauptfenster auf allen Desktops</translation>
    </message>
    <message>
        <source>Makes the Main window visible on all desktops</source>
        <translation>Das Hauptfenster wird auf allen Desktops angezeigt</translation>
    </message>
    <message>
        <source>Sticky Message Window</source>
        <translation>Nachrichtenfenster auf allen Desktops</translation>
    </message>
    <message>
        <source>Makes the Message window visible on all desktops</source>
        <translation>Das Nachrichtenfenster wird auf allen Desktops angezeigt</translation>
    </message>
    <message>
        <source>Hotkey pops up the next pending message
Enter the hotkey literally, like &quot;shift+f10&quot;, &quot;none&quot; for disabling
changes here require a Restart to take effect!
</source>
        <translation>Das Tastenkürzel zeigt die nächste neue Nachricht an
Gib das Kürzel wörtlich ein, wie &quot;shift+f10&quot; gib &quot;none&quot; ein um das Kürzel zu deaktivieren
Bei einer Änderung ist ein Neustart erforderlich!
</translation>
    </message>
    <message>
        <source>Show user display picture</source>
        <translation>Zeige Benutzerbild an</translation>
    </message>
    <message>
        <source>Show the user&apos;s display picture instead of a status icon, if the user is online and has a display picture</source>
        <translation>Zeigt das Benutzerbild anstelle des Satus-Icons an, falls der User online ist und ein Benutzerbild angegeben hat</translation>
    </message>
    <message>
        <source>Single line chat mode</source>
        <translation>Nachricht mit einfachem Enter senden</translation>
    </message>
    <message>
        <source>In single line chat mode you send messages with Enter and insert new lines with Ctrl+Enter, opposite of the normal mode</source>
        <translation>Im einzeiligen Chat-Mode werden Nachrichten mit Enter gesendet und neue Zeilen mit Strg+Enter eingefügt, welches das Gegenteil vom normalen Modus ist</translation>
    </message>
    <message>
        <source>Alias</source>
        <translation>Alias</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Name</translation>
    </message>
    <message>
        <source>Picture</source>
        <translation>Bild</translation>
    </message>
    <message>
        <source>Notice:</source>
        <translation>Benachrichtigung:</translation>
    </message>
    <message>
        <source>This is a sent message</source>
        <translation>Dies ist eine gesendete Nachricht</translation>
    </message>
    <message>
        <source>Have you gone to the Licq IRC Channel?</source>
        <translation>Warst du schon im Licq IRC Kanal?</translation>
    </message>
    <message>
        <source>No, where is it?</source>
        <translation>Nein, wo finde ich den?</translation>
    </message>
    <message>
        <source>#Licq on irc.freenode.net</source>
        <translation>#Licq auf irc.freenode.net</translation>
    </message>
    <message>
        <source>Cool, I&apos;ll see you there :)</source>
        <translation>Cool, wir sehen uns dort! :)</translation>
    </message>
    <message>
        <source>We&apos;ll be waiting!</source>
        <translation>Wir warten auf dich!</translation>
    </message>
    <message>
        <source>Marge has left the conversation.</source>
        <translation>Marge hat die Unterhaltung verlassen.</translation>
    </message>
    <message>
        <source>This is a received message</source>
        <translation>Dies ist eine empfangene Nachricht</translation>
    </message>
    <message>
        <source>Show Join/Left Notices</source>
        <translation>Zeige Betreten/Verlassen Benachrichtigung</translation>
    </message>
    <message>
        <source>Show a notice in the chat window when a user joins or leaves the conversation.</source>
        <translation>Benachrichtigung im Chatfenster zeigen, wenn ein Benutzer die Konversation betritt oder verlässt.</translation>
    </message>
    <message>
        <source>Local time</source>
        <translation>Ortszeit</translation>
    </message>
    <message>
        <source>Chat Options</source>
        <translation>Chat-Optionen</translation>
    </message>
    <message>
        <source>Insert Vertical Spacing</source>
        <translation>Leerzeile einfügen</translation>
    </message>
    <message>
        <source>Insert extra space between messages.</source>
        <translation>Eine Leerzeile nach jeder Nachricht einfügen.</translation>
    </message>
    <message>
        <source>Insert a line between each message.</source>
        <translation>Eine horizontale Linie nach jeder Nachricht einfügen.</translation>
    </message>
    <message>
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
        <translation>&lt;p&gt;Verfügbare Variablen für die benutzerdefinierte Datumsformatierung.&lt;/p&gt;
&lt;table&gt;
&lt;tr&gt;&lt;th&gt;Ausdruck&lt;/th&gt;&lt;th&gt;Ergebnis&lt;/th&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;d&lt;/td&gt;&lt;td&gt;der Tag als Zahl ohne führende Null (1-31)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;dd&lt;/td&gt;&lt;td&gt;der Tag als Zahl mit führender Null (01-31)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;ddd&lt;/td&gt;&lt;td&gt;die lokalisierte Abkürzung des Tages (z.B. &apos;Mon&apos;..&apos;Son&apos;)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;dddd&lt;/td&gt;&lt;td&gt;der lokalisierte Tagesname (z.B. &apos;Montag&apos;..&apos;Sonntag&apos;)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;M&lt;/td&gt;&lt;td&gt;der Monat als Zahl ohne führende Null (1-12)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;MM&lt;/td&gt;&lt;td&gt;der Monat als Zahl mit führender Null (01-12)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;MMM&lt;/td&gt;&lt;td&gt;die lokalisierte Abkürzung des Monats (z.B. &apos;Jan&apos;..&apos;Dez&apos;)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;MMMM&lt;/td&gt;&lt;td&gt;der lokalisierte Monatsname (z.B. &apos;Januar&apos;..&apos;Dezember&apos;)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;yy&lt;/td&gt;&lt;td&gt;das Jahr als zweistellige Zahl (00-99)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;yyyy&lt;/td&gt;&lt;td&gt;das Jahr als vierstellige Zahl (1752-8000)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td colspan=2&gt;&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;h&lt;/td&gt;&lt;td&gt;die Stunde ohne führende Null (0..23 oder 1..12 falls AM/PM-Anzeige)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;hh&lt;/td&gt;&lt;td&gt;die Stunde mit führender Null (00..23 oder 01..12 falls AM/PM-Anzeige)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;m&lt;/td&gt;&lt;td&gt;die Minute ohne führende Null (0..59)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;mm&lt;/td&gt;&lt;td&gt;die Minute mit führender Null (00..59)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;s&lt;/td&gt;&lt;td&gt;die Sekunde ohne führende Null (0..59)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;ss&lt;/td&gt;&lt;td&gt;die Sekunde mit führender Null (00..59)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;z&lt;/td&gt;&lt;td&gt;die Millisekunde ohne führende Nullen (0..999)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;zzz&lt;/td&gt;&lt;td&gt;die Milliesekunden mit führenden Nullen (000..999)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;AP&lt;/td&gt;&lt;td&gt;benutze AM/PM-Anzeige. AP wird entweder durch &apos;AM&apos; oder &apos;PM&apos; ersetzt&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;ap&lt;/td&gt;&lt;td&gt;benutze am/pm-Anzeige. ap wird entweder durch &apos;am&apos; oder &apos;pm&apos; ersetzt&lt;/td&gt;&lt;/tr&gt;
&lt;/table&gt;</translation>
    </message>
    <message>
        <source>History Options</source>
        <translation>Verlauf-Optionen</translation>
    </message>
    <message>
        <source>History</source>
        <translation>Verlauf</translation>
    </message>
    <message>
        <source>Use double return</source>
        <translation>Doppeltes Enter benutzen</translation>
    </message>
    <message>
        <source>Hitting Return twice will be used instead of Ctrl+Return to send messages and close input dialogs. Multiple new lines can be inserted with Ctrl+Return.</source>
        <translation>Statt Strg+Enter wird zweifaches Drücken der Entertaste genutzt, um das Senden einer Nachricht abzuschließen. Neue Zeilen werden mit Strg+Enter eingefügt.</translation>
    </message>
    <message>
        <source>The format string used to define what will appear in each column.
The following parameters can be used:</source>
        <translation>Der Formatstring, der definiert was in jeder Spalte erscheint.
Die folgenden Parameter können benutzt werden:</translation>
    </message>
</context>
<context>
    <name>OwnerEditDlg</name>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;Ok</translation>
    </message>
    <message>
        <source>Edit Account</source>
        <translation>Zugang bearbeiten</translation>
    </message>
    <message>
        <source>Protocol:</source>
        <translation>Protokoll:</translation>
    </message>
    <message>
        <source>User ID:</source>
        <translation>Benutzer-ID:</translation>
    </message>
    <message>
        <source>Currently only one account per protocol is supported.</source>
        <translation>Zur Zeit wird nur ein Zugang pro Protokoll unterstützt.</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>Ab&amp;brechen</translation>
    </message>
    <message>
        <source>Password:</source>
        <translation>Passwort:</translation>
    </message>
</context>
<context>
    <name>OwnerManagerDlg</name>
    <message>
        <source>&amp;Add</source>
        <translation>&amp;Hinzufügen</translation>
    </message>
    <message>
        <source>&amp;Done</source>
        <translation>&amp;Fertig</translation>
    </message>
    <message>
        <source>Successfully registered, your user identification
number (UIN) is %1.
Now set your personal information.</source>
        <translation>Erfolgreich registriert. Ihre Benutzernummer (UIN) ist: %1.
Stellen Sie als nächstes bitte Ihre persönlichen Informationen ein.</translation>
    </message>
    <message>
        <source>D&amp;elete</source>
        <translation>&amp;Löschen</translation>
    </message>
    <message>
        <source>Registration failed.  See network window for details.</source>
        <translation>Die Registrierung ist fehlgeschlagen.
Siehe Netzwerkfenster für Details.</translation>
    </message>
    <message>
        <source>Licq - Account Manager</source>
        <translation>Licq - Zugangsverwaltung</translation>
    </message>
    <message>
        <source>You are currently registered as
UIN (User ID): %1
Base Directory: %2
Rerun licq with the -b option to select a new
base directory and then register a new user.</source>
        <translation>Sie sind registriert als
UIN (Benutzer-ID): %1
Basisordner: %2
Starten Sie Licq mit der Option -b und einem anderen Basisordner
neu. Registrieren Sie danach einen neuen Benutzer.</translation>
    </message>
    <message>
        <source>&amp;Modify</source>
        <translation>&amp;Bearbeiten</translation>
    </message>
    <message>
        <source>&amp;Register</source>
        <translation>&amp;Registrieren</translation>
    </message>
    <message>
        <source>From the Account Manager dialog you are able to add and register your accounts.
Currently, only one account per protocol is supported, but this will be changed in future versions.</source>
        <translation>Die Licq Zugangsverwaltung ermöglicht Ihnen das Hinzufügen und Registrieren
Ihrer Zugänge. Zur Zeit wird nur ein Zugang pro Protokoll unterstützt, aber das wird sich in einer zukünftigen Version ändern.</translation>
    </message>
</context>
<context>
    <name>OwnerView</name>
    <message>
        <source>(Invalid ID)</source>
        <translation>(Ungültige ID)</translation>
    </message>
    <message>
        <source>Invalid Protocol</source>
        <translation>Ungültiges Protokoll</translation>
    </message>
    <message>
        <source>Protocol</source>
        <translation>Protokoll</translation>
    </message>
    <message>
        <source>User ID</source>
        <translation>Benutzer-ID</translation>
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
        <translation>Fertig</translation>
    </message>
    <message>
        <source>Load</source>
        <translation>Laden</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Name</translation>
    </message>
    <message>
        <source>(Unloaded)</source>
        <translation>(Entladen)</translation>
    </message>
    <message>
        <source>Standard Plugins</source>
        <translation>Standard-Plugins</translation>
    </message>
    <message>
        <source>Plugin %1 has no configuration file</source>
        <translation>Plugin %1 hat keine Konfigurationsdatei</translation>
    </message>
    <message>
        <source>Enable</source>
        <translation>Aktivieren</translation>
    </message>
    <message>
        <source>Licq Plugin %1 %2
</source>
        <translation>Licq-Plugin %1 %2
</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Auffrischen</translation>
    </message>
    <message>
        <source>Description</source>
        <translation>Beschreibung</translation>
    </message>
    <message>
        <source>Version</source>
        <translation>Version</translation>
    </message>
    <message>
        <source>Protocol Plugins</source>
        <translation>Protokoll-Plugins</translation>
    </message>
</context>
<context>
    <name>QMessageBox</name>
    <message>
        <source>Licq Question</source>
        <translation>Licq Frage</translation>
    </message>
    <message>
        <source>Yes</source>
        <translation>Ja</translation>
    </message>
    <message>
        <source>No</source>
        <translation>Nein</translation>
    </message>
</context>
<context>
    <name>RegisterUserDlg</name>
    <message>
        <source>Account Registration in Progress...</source>
        <translation>Zugangsregistrierung läuft...</translation>
    </message>
    <message>
        <source>Please enter your password in both input fields.</source>
        <translation>Bitte geben Sie Ihr Passwort in beide Eingabefelder ein.</translation>
    </message>
    <message>
        <source>Account registration has been successfuly completed.
Your new user id is %1.
You are now being automatically logged on.
Click OK to edit your personal details.
After you are online, you can send your personal details to the server.</source>
        <translation>Der Zugang wurde erfolgreich registriert.
Ihre neue Benutzer-ID lautet %1.
Sie werden nun automatisch angemeldet.
Klicken Sie auf Ok um Ihre persönlichen Informationen zu bearbeiten.
Sobald Sie online sind, können Sie diese Details an den Server übermitteln.</translation>
    </message>
    <message>
        <source>Licq Account Registration</source>
        <translation>Licq Zugangsregistrierung</translation>
    </message>
    <message>
        <source>Invalid password, must be between 1 and 8 characters.</source>
        <translation>Ungültiges Passwort, es muss zwischen 1 und 8 Zeichen lang sein.</translation>
    </message>
    <message>
        <source>Now please click &apos;Finish&apos; to start the registration process.</source>
        <translation>Vielen Dank.

Klicken Sie nun auf &apos;Fertigstellen&apos;, um den Registriervorgang zu starten.</translation>
    </message>
    <message>
        <source>Account Registration</source>
        <translation>Zugang registrieren</translation>
    </message>
    <message>
        <source>Welcome to the Registration Wizard.

You can register a new user here.

Press &quot;Next&quot; to proceed.</source>
        <translation>Willkommen beim Registrierungs-Assistenten.

Mit diesem Assistenten können Sie einen neuen Zugang registrieren.

Bitte klicken Sie nun auf &apos;Weiter&apos;.</translation>
    </message>
    <message>
        <source>Now please press the &apos;Back&apos; button and try again.</source>
        <translation>Klicken Sie auf &apos;Zurück&apos; um es erneut zu versuchen.

Achten Sie darauf, dass Sie beide Passwörter in gleicher
Schreibweise eingeben.</translation>
    </message>
    <message>
        <source>Enter a password to protect your account.</source>
        <translation>Geben Sie ein Passwort ein, mit dem Ihr Zugang geschützt wird.</translation>
    </message>
    <message>
        <source>The passwords don&apos;t seem to match.</source>
        <translation>Die Passwörter stimmen nicht überein.</translation>
    </message>
    <message>
        <source>&amp;Remember Password</source>
        <translation>Passwort &amp;beibehalten</translation>
    </message>
    <message>
        <source>Password:</source>
        <translation>Passwort:</translation>
    </message>
    <message>
        <source>Verify:</source>
        <translation>Bestätigen:</translation>
    </message>
    <message>
        <source>Account Registration - Step 2</source>
        <translation>Zugang registrieren - Schritt 2</translation>
    </message>
    <message>
        <source>Account Registration - Step 3</source>
        <translation>Zugang registrieren - Schritt 3</translation>
    </message>
</context>
<context>
    <name>ReqAuthDlg</name>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;Ok</translation>
    </message>
    <message>
        <source>Request authorization from (UIN):</source>
        <translation>Autorisierung anfragen von (UIN):</translation>
    </message>
    <message>
        <source>Licq - Request Authorization</source>
        <translation>Licq - Autorisierung anfragen</translation>
    </message>
    <message>
        <source>Request</source>
        <translation>Mitteilung</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>Ab&amp;brechen</translation>
    </message>
</context>
<context>
    <name>SearchUserDlg</name>
    <message>
        <source>60+</source>
        <translation>60+</translation>
    </message>
    <message>
        <source>Male</source>
        <translation>Männlich</translation>
    </message>
    <message>
        <source>Reset Search</source>
        <translation>Neue Suche</translation>
    </message>
    <message>
        <source>&amp;Done</source>
        <translation>&amp;Fertig</translation>
    </message>
    <message>
        <source>&amp;UIN#</source>
        <translation>&amp;UIN#</translation>
    </message>
    <message>
        <source>City:</source>
        <translation>Stadt:</translation>
    </message>
    <message>
        <source>UIN#:</source>
        <translation>UIN#:</translation>
    </message>
    <message>
        <source>Company Name:</source>
        <translation>Firma:</translation>
    </message>
    <message>
        <source>Enter search parameters and select &apos;Search&apos;</source>
        <translation>Bitte Suchparameter eingeben und dann auch &apos;Suchen&apos; klicken.</translation>
    </message>
    <message>
        <source>Company Position:</source>
        <translation>Position:</translation>
    </message>
    <message>
        <source>Searching (this can take awhile)...</source>
        <translation>Suche läuft (das kann eine Weile dauern)...</translation>
    </message>
    <message>
        <source>%1 more users found. Narrow search.</source>
        <translation>%1 weitere Benutzer gefunden. Suche bitte eingrenzen.</translation>
    </message>
    <message>
        <source>More users found. Narrow search.</source>
        <translation>Mehr Benutzer gefunden. Suche weiter eingrenzen.</translation>
    </message>
    <message>
        <source>Alias:</source>
        <translation>Alias:</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <source>18 - 22</source>
        <translation>18 - 22</translation>
    </message>
    <message>
        <source>Female</source>
        <translation>Weiblich</translation>
    </message>
    <message>
        <source>Language:</source>
        <translation>Sprache:</translation>
    </message>
    <message>
        <source>23 - 29</source>
        <translation>23 - 29</translation>
    </message>
    <message>
        <source>Age Range:</source>
        <translation>Altersbereich:</translation>
    </message>
    <message>
        <source>State:</source>
        <translation>Bundesland:</translation>
    </message>
    <message>
        <source>First Name:</source>
        <translation>Vorname:</translation>
    </message>
    <message>
        <source>30 - 39</source>
        <translation>30 - 39</translation>
    </message>
    <message>
        <source>Last Name:</source>
        <translation>Nachname:</translation>
    </message>
    <message>
        <source>Country:</source>
        <translation>Land:</translation>
    </message>
    <message>
        <source>40 - 49</source>
        <translation>40 - 49</translation>
    </message>
    <message>
        <source>Search failed.</source>
        <translation>Suche fehlgeschlagen.</translation>
    </message>
    <message>
        <source>50 - 59</source>
        <translation>50 - 59</translation>
    </message>
    <message>
        <source>Company Department:</source>
        <translation>Abteilung:</translation>
    </message>
    <message>
        <source>Return Online Users Only</source>
        <translation>Nur &amp;Online-Benutzer zeigen</translation>
    </message>
    <message>
        <source>A&amp;lert User</source>
        <translation>Benutzer &amp;informieren</translation>
    </message>
    <message>
        <source>&amp;Add %1 Users</source>
        <translation>%1 Benutzer &amp;hinzufügen</translation>
    </message>
    <message>
        <source>View &amp;Info</source>
        <translation>&amp;Informationen</translation>
    </message>
    <message>
        <source>Email Address:</source>
        <translation>E-Mail-Adresse:</translation>
    </message>
    <message>
        <source>&amp;Add User</source>
        <translation>Benutzer &amp;hinzufügen</translation>
    </message>
    <message>
        <source>&amp;Whitepages</source>
        <translation>&amp;Whitepages</translation>
    </message>
    <message>
        <source>&amp;Search</source>
        <translation>&amp;Suchen</translation>
    </message>
    <message>
        <source>Unspecified</source>
        <translation>nicht angegeben</translation>
    </message>
    <message>
        <source>Search complete.</source>
        <translation>Suche abgeschlossen.</translation>
    </message>
    <message>
        <source>Keyword:</source>
        <translation>Stichwort:</translation>
    </message>
    <message>
        <source>Gender:</source>
        <translation>Geschlecht:</translation>
    </message>
    <message>
        <source>Licq - User Search</source>
        <translation>Licq - Benutzer Suchen</translation>
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
        <translation>W</translation>
    </message>
    <message>
        <source>M</source>
        <translation>M</translation>
    </message>
    <message>
        <source>UIN</source>
        <translation>UIN</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Name</translation>
    </message>
    <message>
        <source>Alias</source>
        <translation>Alias</translation>
    </message>
    <message>
        <source>Email</source>
        <translation>E-Mail</translation>
    </message>
    <message>
        <source>Online</source>
        <translation>Online</translation>
    </message>
    <message>
        <source>Status</source>
        <translation>Status</translation>
    </message>
    <message>
        <source>Offline</source>
        <translation>Offline</translation>
    </message>
    <message>
        <source>Sex &amp; Age</source>
        <translation>Geschlecht &amp; Alter</translation>
    </message>
    <message>
        <source>Authorize</source>
        <translation>Autorisieren</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation>Unbekannt</translation>
    </message>
    <message>
        <source>No</source>
        <translation>Nein</translation>
    </message>
    <message>
        <source>Yes</source>
        <translation>Ja</translation>
    </message>
</context>
<context>
    <name>SecurityDlg</name>
    <message>
        <source>&amp;Uin:</source>
        <translation>&amp;Uin:</translation>
    </message>
    <message>
        <source>Timeout while setting security options.</source>
        <translation>Zeitüberschreitung beim Setzen der Sicherheitseinstellungen.</translation>
    </message>
    <message>
        <source>error</source>
        <translation>fehler</translation>
    </message>
    <message>
        <source>ICQ Security Options [Setting...]</source>
        <translation>ICQ Sicherheits-Einstellungen setzen... [fertig]</translation>
    </message>
    <message>
        <source>Internal error while setting security options.</source>
        <translation>Interner Fehler beim Setzen der Sicherheitseinstellungen.</translation>
    </message>
    <message>
        <source>&amp;Local changes only</source>
        <translation>&amp;Nur lokale Änderungen</translation>
    </message>
    <message>
        <source>Password/UIN settings</source>
        <translation>Passwort-/UIN-Einstellungen</translation>
    </message>
    <message>
        <source>ICQ Security Options</source>
        <translation>ICQ Sicherheits-Einstellungen</translation>
    </message>
    <message>
        <source>You need to be connected to the
ICQ Network to change the settings.</source>
        <translation>Sie müssen mit dem ICQ-Netzwerk verbunden sein,
um Einstellungen ändern zu können.</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>zeitüberschreitung</translation>
    </message>
    <message>
        <source>Verify your ICQ password here.</source>
        <translation>Geben Sie hier Ihr ICQ-Passwort zur Bestätigung nochmal ein.</translation>
    </message>
    <message>
        <source>If checked, password/UIN changes will apply only on your local computer.  Useful if your password is incorrectly saved in Licq.</source>
        <translation>Wenn &quot;Nur lokale Änderungen&quot; aktiviert ist, werden alle Änderungen nur auf Ihrem Computer vorgenommen; es werden keine Änderungen an den ICQ-Server gesendet. Sehr nützlich, wenn Ihr Passwort in Licq falsch gespeichert ist und Sie den Fehler korrigieren wollen.</translation>
    </message>
    <message>
        <source>Invalid password, must be between 1 and 8 characters.</source>
        <translation>Ungültiges Passwort, es muss zwischen 1 und 8 Zeichen lang sein.</translation>
    </message>
    <message>
        <source>Timeout while changing password.</source>
        <translation>Zeitüberschreitung beim Ändern des Passwortes.</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>Optionen</translation>
    </message>
    <message>
        <source>Web Presence allows users to see if you are online through your web indicator.</source>
        <translation>&apos;Web-Präsenz&apos; ermöglicht es anderen, über die Web-Status-Abfrage zu erkennen ob Sie online sind oder nicht.</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>fehlgeschlagen</translation>
    </message>
    <message>
        <source>Passwords do not match, try again.</source>
        <translation>Passworte stimmen nicht überein. Bitte erneut versuchen.</translation>
    </message>
    <message>
        <source>Authorization Required</source>
        <translation>Autorisierung benötigt</translation>
    </message>
    <message>
        <source>Internal error while changing password.</source>
        <translation>Interner Fehler beim Ändern des Passwortes.</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>Ab&amp;brechen</translation>
    </message>
    <message>
        <source>&amp;Password:</source>
        <translation>&amp;Passwort:</translation>
    </message>
    <message>
        <source>&amp;Update</source>
        <translation>&amp;Aktualisieren</translation>
    </message>
    <message>
        <source>Changing password failed.</source>
        <translation>Passwortänderung ist fehlgeschlagen.</translation>
    </message>
    <message>
        <source>Enter the UIN which you want to use.  Only available if &quot;Local changes only&quot; is checked.</source>
        <translation>Geben Sie die UIN ein, die Sie benutzen möchten. Dies ist nur möglich wenn &quot;Nur lokale Änderungen&quot; aktiviert ist.</translation>
    </message>
    <message>
        <source>Hide IP stops users from seeing your IP address. It doesn&apos;t guarantee it will be hidden though.</source>
        <translation>IP verstecken hindert andere Benutzer daran, Ihre IP-Adresse zu sehen. Es gibt allerdings keine Garantie, dass Ihre IP wirklich ungesehen bleibt.</translation>
    </message>
    <message>
        <source>&amp;Verify:</source>
        <translation>&amp;Wiederholung:</translation>
    </message>
    <message>
        <source>Setting security options failed.</source>
        <translation>Setzen der Sicherheitseinstellungen ist fehlgeschlagen.</translation>
    </message>
    <message>
        <source>Web Presence</source>
        <translation>Web-Präsenz</translation>
    </message>
    <message>
        <source>ICQ Security Options [Setting...</source>
        <translation>ICQ Sicherheits-Einstellungen setzen...</translation>
    </message>
    <message>
        <source>Enter your ICQ password here.</source>
        <translation>Tragen Sie hier Ihr ICQ-Passwort ein.</translation>
    </message>
    <message>
        <source>Determines whether regular ICQ clients require your authorization to add you to their contact list.</source>
        <translation>Bestimmt, ob normale ICQ-Clients Ihre Autorisierung benötigen, um Sie zur Kontaktliste hinzufügen zu können.</translation>
    </message>
    <message>
        <source>Hide IP</source>
        <translation>IP verstecken</translation>
    </message>
</context>
<context>
    <name>ShowAwayMsgDlg</name>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;Ok</translation>
    </message>
    <message>
        <source>error</source>
        <translation>fehler</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>zeitüberschreitung</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>fehlgeschlagen</translation>
    </message>
    <message>
        <source>refused</source>
        <translation>abgelehnt</translation>
    </message>
    <message>
        <source>&amp;Show Again</source>
        <translation>&amp;Erneut zeigen</translation>
    </message>
    <message>
        <source>%1 Response for %2</source>
        <translation>%1-Antwort für %2</translation>
    </message>
</context>
<context>
    <name>SkinBrowserDlg</name>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;Ok</translation>
    </message>
    <message>
        <source>Use this combo box to select one of the available emoticon icon sets</source>
        <translation>Benutzen Sie diese Auswahlbox, um aus den verfügbaren Emoticons auszuwählen.</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Fehler</translation>
    </message>
    <message>
        <source>Skin:</source>
        <translation>Skin:</translation>
    </message>
    <message>
        <source>&amp;Edit Skin</source>
        <translation>Skin b&amp;earbeiten</translation>
    </message>
    <message>
        <source>Use this combo box to select one of the available extended icon sets</source>
        <translation>Benutzen Sie diese Auswahlbox, um aus den verfügbaren erweiterten Symbolen zu wählen.</translation>
    </message>
    <message>
        <source>Skin selection</source>
        <translation>Skin-Auswahl</translation>
    </message>
    <message>
        <source>&amp;Apply</source>
        <translation>Über&amp;nehmen</translation>
    </message>
    <message>
        <source>Unable to open icons file
%1</source>
        <translation>Symboldatei ließ sich nicht öffnen:
%1</translation>
    </message>
    <message>
        <source>Unable to open extended icons file
%1
Extended Iconset &apos;%2&apos; has been disabled.</source>
        <translation>Erweiterte-Symbole-Datei ließ sich nicht öffnen:
%1
Erweiterte-Symbole-Paket &apos;%2&apos; wurde daher deaktiviert.</translation>
    </message>
    <message>
        <source>Icons:</source>
        <translation>Symbole:</translation>
    </message>
    <message>
        <source>S&amp;kins:</source>
        <translation>S&amp;kins:</translation>
    </message>
    <message>
        <source>Use this combo box to select one of the available icon sets</source>
        <translation>Benutzen Sie diese Auswahlbox, um aus den verfügbaren Symbolen zu wählen.</translation>
    </message>
    <message>
        <source>Preview</source>
        <translation>Vorschau</translation>
    </message>
    <message>
        <source>Unable to open extended icons file
%1</source>
        <translation>Erweiterte-Symbole-Datei ließ sich nicht öffnen:
%1</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>Ab&amp;brechen</translation>
    </message>
    <message>
        <source>&amp;Icons:</source>
        <translation>S&amp;ymbole:</translation>
    </message>
    <message>
        <source>Emoticons:</source>
        <translation>Emoticons:</translation>
    </message>
    <message>
        <source>Licq Skin Browser</source>
        <translation>Licq Design-Verwaltung</translation>
    </message>
    <message>
        <source>Unable to open icons file
%1
Iconset &apos;%2&apos; has been disabled.</source>
        <translation>Symboldatei ließ sich nicht öffnen:
%1
Symbol-Paket &apos;%2&apos; wurde daher deaktiviert.</translation>
    </message>
    <message>
        <source>Use this combo box to select one of the available skins</source>
        <translation>Benutzen Sie diese Auswahlbox, um aus den verfügbaren Skins zu wählen.</translation>
    </message>
    <message>
        <source>E&amp;xtended Icons:</source>
        <translation>Er&amp;w. Symbole:</translation>
    </message>
    <message>
        <source>Extended Icons:</source>
        <translation>Erweiterte Symbole:</translation>
    </message>
    <message>
        <source>E&amp;moticons:</source>
        <translation>E&amp;moticons:</translation>
    </message>
</context>
<context>
    <name>Status</name>
    <message>
        <source>Offline</source>
        <translation>Offline</translation>
    </message>
    <message>
        <source>Online</source>
        <translation>Online</translation>
    </message>
    <message>
        <source>Away</source>
        <translation>Abwesend</translation>
    </message>
    <message>
        <source>Do Not Disturb</source>
        <translation>Nicht stören</translation>
    </message>
    <message>
        <source>Not Available</source>
        <translation>Nicht verfügbar</translation>
    </message>
    <message>
        <source>Occupied</source>
        <translation>Beschäftigt</translation>
    </message>
    <message>
        <source>Free for Chat</source>
        <translation>Frei für Chat</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation>Unbekannt</translation>
    </message>
    <message>
        <source>Off</source>
        <translation>Off</translation>
    </message>
    <message>
        <source>On</source>
        <translation>On</translation>
    </message>
    <message>
        <source>DND</source>
        <translation>DND</translation>
    </message>
    <message>
        <source>N/A</source>
        <translation>N/v</translation>
    </message>
    <message>
        <source>Occ</source>
        <translation>Besch</translation>
    </message>
    <message>
        <source>FFC</source>
        <translation>FfC</translation>
    </message>
    <message>
        <source>???</source>
        <translation>???</translation>
    </message>
</context>
<context>
    <name>UserCodec</name>
    <message>
        <source>Unicode</source>
        <translation>Unicode</translation>
    </message>
    <message>
        <source>Unicode-16</source>
        <translation>Unicode-16</translation>
    </message>
    <message>
        <source>Arabic</source>
        <translation>Arabisch</translation>
    </message>
    <message>
        <source>Baltic</source>
        <translation>Baltisch</translation>
    </message>
    <message>
        <source>Central European</source>
        <translation>Zentraleuropäisch</translation>
    </message>
    <message>
        <source>Chinese</source>
        <translation>Chinesisch</translation>
    </message>
    <message>
        <source>Chinese Traditional</source>
        <translation>Chinesisch traditionell</translation>
    </message>
    <message>
        <source>Cyrillic</source>
        <translation>Kyrillisch</translation>
    </message>
    <message>
        <source>Esperanto</source>
        <translation>Spanisch</translation>
    </message>
    <message>
        <source>Greek</source>
        <translation>Grieschisch</translation>
    </message>
    <message>
        <source>Hebrew</source>
        <translation>Hebräisch</translation>
    </message>
    <message>
        <source>Japanese</source>
        <translation>Japanisch</translation>
    </message>
    <message>
        <source>Korean</source>
        <translation>Koreanisch</translation>
    </message>
    <message>
        <source>Western European</source>
        <translation>Westeuropäisch</translation>
    </message>
    <message>
        <source>Tamil</source>
        <translation>Tamil</translation>
    </message>
    <message>
        <source>Thai</source>
        <translation>Thailändisch</translation>
    </message>
    <message>
        <source>Turkish</source>
        <translation>Türkisch</translation>
    </message>
    <message>
        <source>Ukrainian</source>
        <translation>Ukrainisch</translation>
    </message>
</context>
<context>
    <name>UserEventCommon</name>
    <message>
        <source>Time:</source>
        <translation>Ortszeit:</translation>
    </message>
    <message>
        <source>Change user text encoding</source>
        <translation>Zeichensatz für die Anzeige von Nachrichtentext ändern</translation>
    </message>
    <message>
        <source>This button selects the text encoding used when communicating with this user. You might need to change the encoding to communicate in a different language.</source>
        <translation>Dieser Knopf wählt den zu verwendenden Zeichensatz für die Kommunikation mit dem Benutzer. Sie müssen den Zeichensatz möglicherweise wechseln um in einer fremden Sprache zu kommunizieren.</translation>
    </message>
    <message>
        <source>Show User Info</source>
        <translation>Informationen zu diesem Benutzer öffnen</translation>
    </message>
    <message>
        <source>Unable to load encoding &lt;b&gt;%1&lt;/b&gt;. Message contents may appear garbled.</source>
        <translation>Zeichensatz &lt;b&gt;%1&lt;/b&gt; lässt sich nicht laden. Nachrichteninhalte könnten verstümmelt dargestellt werden.</translation>
    </message>
    <message>
        <source>Status:</source>
        <translation>Status:</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation>Unbekannt</translation>
    </message>
    <message>
        <source>Show User History</source>
        <translation>Verlauf für diesen Benutzer zeigen</translation>
    </message>
    <message>
        <source>Open / Close secure channel</source>
        <translation>Öffnen / Schließen einer verschlüsselten Verbindung</translation>
    </message>
</context>
<context>
    <name>UserInfoDlg</name>
    <message>
        <source>(</source>
        <translation>(</translation>
    </message>
    <message>
        <source>-</source>
        <translation>-</translation>
    </message>
    <message>
        <source>) </source>
        <translation>)</translation>
    </message>
    <message>
        <source>No</source>
        <translation>Nein</translation>
    </message>
    <message>
        <source>ID:</source>
        <translation>ID:</translation>
    </message>
    <message>
        <source>IP:</source>
        <translation>IP:</translation>
    </message>
    <message>
        <source>Now</source>
        <translation>Jetzt</translation>
    </message>
    <message>
        <source>Yes</source>
        <translation>Ja</translation>
    </message>
    <message>
        <source>&amp;Add</source>
        <translation>&amp;Hinzufügen</translation>
    </message>
    <message>
        <source>Age:</source>
        <translation>Alter:</translation>
    </message>
    <message>
        <source>Fax:</source>
        <translation>Fax:</translation>
    </message>
    <message>
        <source>Male</source>
        <translation>Männlich</translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Typ</translation>
    </message>
    <message>
        <source>Zip:</source>
        <translation>PLZ:</translation>
    </message>
    <message>
        <source>done</source>
        <translation>fertig</translation>
    </message>
    <message>
        <source> Day:</source>
        <translation>Tag:</translation>
    </message>
    <message>
        <source>&amp;Last</source>
        <translation>Z&amp;uletzt...</translation>
    </message>
    <message>
        <source>&amp;Menu</source>
        <translation>&amp;Menü</translation>
    </message>
    <message>
        <source>&amp;More</source>
        <translation>Me&amp;hr</translation>
    </message>
    <message>
        <source>&amp;Save</source>
        <translation>&amp;Speichern</translation>
    </message>
    <message>
        <source>&amp;Work</source>
        <translation>A&amp;rbeit</translation>
    </message>
    <message>
        <source>&amp;Filter: </source>
        <translation>&amp;Filter: </translation>
    </message>
    <message>
        <source>City:</source>
        <translation>Stadt:</translation>
    </message>
    <message>
        <source>P&amp;rev</source>
        <translation>&amp;Zurück</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation>Name:</translation>
    </message>
    <message>
        <source>Nex&amp;t</source>
        <translation>V&amp;orwärts</translation>
    </message>
    <message>
        <source>S&amp;end</source>
        <translation>S&amp;enden</translation>
    </message>
    <message>
        <source>error</source>
        <translation>fehler</translation>
    </message>
    <message>
        <source>Last Online:</source>
        <translation>Letztes mal Online:</translation>
    </message>
    <message>
        <source>Keep Alias on Update</source>
        <translation>Alias bei Update behalten</translation>
    </message>
    <message>
        <source>Number/Gateway</source>
        <translation>Rufnummer/Gateway</translation>
    </message>
    <message>
        <source>Currently at:</source>
        <translation>Zur Zeit erreichbar unter:</translation>
    </message>
    <message>
        <source>Updating server...</source>
        <translation>Ändern der Information auf dem Server...</translation>
    </message>
    <message>
        <source> Year:</source>
        <translation>Jahr:</translation>
    </message>
    <message>
        <source>Last Sent Event:</source>
        <translation>Letztes Ereignis gesendet:</translation>
    </message>
    <message>
        <source>Last Checked Auto Response:</source>
        <translation>Letzte gelesene Auto-Antwort:</translation>
    </message>
    <message>
        <source>Not Available</source>
        <translation>Nicht verfügbar</translation>
    </message>
    <message>
        <source>&amp;About</source>
        <translation>Ü&amp;ber</translation>
    </message>
    <message>
        <source>&amp;Clear</source>
        <translation>&amp;Löschen</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>S&amp;chließen</translation>
    </message>
    <message>
        <source>&amp;Phone</source>
        <translation>&amp;Telefon</translation>
    </message>
    <message>
        <source>You need to be connected to the
ICQ Network to retrieve your settings.</source>
        <translation>Sie müssen mit dem ICQ-Netzwerk verbunden sein, um Einstellungen empfangen zu können.</translation>
    </message>
    <message>
        <source>(none)</source>
        <translation>(keine)</translation>
    </message>
    <message>
        <source>Failed to Load</source>
        <translation>Fehler beim Laden</translation>
    </message>
    <message>
        <source>Cellular:</source>
        <translation>Mobiltelefon:</translation>
    </message>
    <message>
        <source>EMail 1:</source>
        <translation>Email 1:</translation>
    </message>
    <message>
        <source>EMail 2:</source>
        <translation>Email 2:</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>Zeitüberschreitung</translation>
    </message>
    <message>
        <source>Homepage:</source>
        <translation>Homepage:</translation>
    </message>
    <message>
        <source>Online Since:</source>
        <translation>Online seit:</translation>
    </message>
    <message>
        <source>Timezone:</source>
        <translation>Zeitzone:</translation>
    </message>
    <message>
        <source>About:</source>
        <translation>Über:</translation>
    </message>
    <message>
        <source>Alias:</source>
        <translation>Alias:</translation>
    </message>
    <message>
        <source>Email:</source>
        <translation>E-Mail:</translation>
    </message>
    <message>
        <source>Old Email:</source>
        <translation>Alte Email:</translation>
    </message>
    <message>
        <source>Female</source>
        <translation>Weiblich</translation>
    </message>
    <message>
        <source>Description:</source>
        <translation>Beschreibung:</translation>
    </message>
    <message>
        <source> Month:</source>
        <translation>Monat:</translation>
    </message>
    <message>
        <source>Phone:</source>
        <translation>Telefon:</translation>
    </message>
    <message>
        <source>State:</source>
        <translation>Bundesland:</translation>
    </message>
    <message>
        <source>Offline</source>
        <translation>Offline</translation>
    </message>
    <message>
        <source>Last Received Event:</source>
        <translation>Letztes Ereignis empfangen:</translation>
    </message>
    <message>
        <source>Authorization Not Required</source>
        <translation>Autorisierung nicht erforderlich</translation>
    </message>
    <message>
        <source>[&lt;font color=&quot;%1&quot;&gt;Received&lt;/font&gt;] [&lt;font color=&quot;%2&quot;&gt;Sent&lt;/font&gt;] %3 out of %4 matches</source>
        <translation>[&lt;font color=&quot;%1&quot;&gt;Empfangen&lt;/font&gt;] [&lt;font color=&quot;%2&quot;&gt;Gesendet&lt;/font&gt;] %3 von %4 Übereinstimmungen</translation>
    </message>
    <message>
        <source>[&lt;font color=&quot;%1&quot;&gt;Received&lt;/font&gt;] [&lt;font color=&quot;%2&quot;&gt;Sent&lt;/font&gt;] %3 to %4 of %5</source>
        <translation>[&lt;font color=&quot;%1&quot;&gt;Empfangen&lt;/font&gt;] [&lt;font color=&quot;%2&quot;&gt;Gesendet&lt;/font&gt;] %3 bis %4 von %5</translation>
    </message>
    <message>
        <source>Country:</source>
        <translation>Land:</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>fehlgeschlagen</translation>
    </message>
    <message>
        <source>Occupation:</source>
        <translation>Beruf:</translation>
    </message>
    <message>
        <source>server</source>
        <translation>Server</translation>
    </message>
    <message>
        <source>Select your picture</source>
        <translation>Bitte Ihr Bild auswählen</translation>
    </message>
    <message>
        <source>Authorization Required</source>
        <translation>Autorisierung erforderlich</translation>
    </message>
    <message>
        <source>INVALID USER</source>
        <translation>UNGÜLTIGER BENUTZER</translation>
    </message>
    <message>
        <source>Birthday:</source>
        <translation>Geburtstag:</translation>
    </message>
    <message>
        <source>Unknown (%1)</source>
        <translation>Unbekannt (%1)</translation>
    </message>
    <message>
        <source> is over %1 bytes.
Select another picture?</source>
        <translation>ist größer als %1 Byte.
Ein anderes Bild auswählen?</translation>
    </message>
    <message>
        <source>Department:</source>
        <translation>Abteilung:</translation>
    </message>
    <message>
        <source>Position:</source>
        <translation>Position:</translation>
    </message>
    <message>
        <source>Sorry, history is disabled for this person.</source>
        <translation>Verlauf ist für diesen Benutzer abgeschaltet.</translation>
    </message>
    <message>
        <source>Licq - Info </source>
        <translation>Licq - Info </translation>
    </message>
    <message>
        <source>User has an ICQ Homepage </source>
        <translation>Benutzer hate eine ICQ-Homepage</translation>
    </message>
    <message>
        <source>&amp;Browse</source>
        <translation>&amp;Durchsuchen</translation>
    </message>
    <message>
        <source>Status:</source>
        <translation>Status:</translation>
    </message>
    <message>
        <source>Address:</source>
        <translation>Adresse:</translation>
    </message>
    <message>
        <source>Updating...</source>
        <translation>Ändern...</translation>
    </message>
    <message>
        <source>Category:</source>
        <translation>Kategorie:</translation>
    </message>
    <message>
        <source>You need to be connected to the
ICQ Network to change your settings.</source>
        <translation>Sie müssen mit dem ICQ-Netzwerk verbunden sein, um Einstellungen ändern zu können.</translation>
    </message>
    <message>
        <source>Language 3:</source>
        <translation>Sprache 3:</translation>
    </message>
    <message>
        <source>Language 1:</source>
        <translation>Sprache 1:</translation>
    </message>
    <message>
        <source>Language 2:</source>
        <translation>Sprache 2:</translation>
    </message>
    <message>
        <source>User has no ICQ Homepage</source>
        <translation>Benutzer hat keine ICQ-Homepage</translation>
    </message>
    <message>
        <source>Unspecified</source>
        <translation>Nicht angegeben</translation>
    </message>
    <message>
        <source>&amp;Update</source>
        <translation>&amp;Aktualisieren</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation>Unbekannt</translation>
    </message>
    <message>
        <source>&amp;Retrieve</source>
        <translation>Em&amp;pfangen</translation>
    </message>
    <message>
        <source>Retrieve</source>
        <translation>Empfangen</translation>
    </message>
    <message>
        <source>Rever&amp;se</source>
        <translation>&amp;Rückwärts</translation>
    </message>
    <message>
        <source>P&amp;icture</source>
        <translation>B&amp;ild</translation>
    </message>
    <message>
        <source>M&amp;ore II</source>
        <translation>&amp;Noch mehr</translation>
    </message>
    <message>
        <source>Gender:</source>
        <translation>Geschlecht:</translation>
    </message>
    <message>
        <source>&amp;General</source>
        <translation>A&amp;llgemein</translation>
    </message>
    <message>
        <source>Country/Provider</source>
        <translation>Land/Anbieter</translation>
    </message>
    <message>
        <source>&amp;History</source>
        <translation>&amp;Verlauf</translation>
    </message>
    <message>
        <source>Normally Licq overwrites the Alias when updating user details.
Check this if you want to keep your changes to the Alias.</source>
        <translation>Normalerweise überschreibt Licq den Alias wenn die Benutzerdaten aktualisiert werden.
Aktivieren Sie dieses Kontrollfeld damit der Alias bei einem Update nicht verändert wird.</translation>
    </message>
    <message>
        <source>Error loading history file: %1
Description: %2</source>
        <translation>Fehler beim Laden der Verlaufs-Datei: %1
Beschreibung: %2</translation>
    </message>
    <message>
        <source>&amp;KDE Addressbook</source>
        <translation>&amp;KDE Adressbuch</translation>
    </message>
    <message>
        <source>Registration Date:</source>
        <translation>Registrierungsdatum:</translation>
    </message>
</context>
<context>
    <name>UserSelectDlg</name>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;Ok</translation>
    </message>
    <message>
        <source>&amp;Save Password</source>
        <translation>Passwort &amp;speichern</translation>
    </message>
    <message>
        <source>&amp;User:</source>
        <translation>&amp;Benutzer:</translation>
    </message>
    <message>
        <source>Licq User Select</source>
        <translation>Licq Benutzer-Auswahl</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>Ab&amp;brechen</translation>
    </message>
    <message>
        <source>&amp;Password:</source>
        <translation>&amp;Passwort:</translation>
    </message>
</context>
<context>
    <name>UserSendChatEvent</name>
    <message>
        <source>Clear</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <source>Multiparty: </source>
        <translation>Mehrbenutzer: </translation>
    </message>
    <message>
        <source>Invite</source>
        <translation>Einladen</translation>
    </message>
    <message>
        <source>No reason provided</source>
        <translation>Kein Grund angegeben</translation>
    </message>
    <message>
        <source>Chat with %2 refused:
%3</source>
        <translation>Chat mit %2 abgelehnt:
%3</translation>
    </message>
    <message>
        <source> - Chat Request</source>
        <translation> - Chat-Anfrage</translation>
    </message>
</context>
<context>
    <name>UserSendCommon</name>
    <message>
        <source>No</source>
        <translation>Nein</translation>
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
        <translation>Ja</translation>
    </message>
    <message>
        <source>done</source>
        <translation>fertig</translation>
    </message>
    <message>
        <source>&amp;Menu</source>
        <translation>&amp;Menü</translation>
    </message>
    <message>
        <source>&amp;Send</source>
        <translation>&amp;Senden</translation>
    </message>
    <message>
        <source>error</source>
        <translation>fehler</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>S&amp;chließen</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>zeitüberschreitung</translation>
    </message>
    <message>
        <source>Message</source>
        <translation>Nachricht</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <source>cancelled</source>
        <translation>abgebrochen</translation>
    </message>
    <message>
        <source>File Transfer</source>
        <translation>Dateiübertragung</translation>
    </message>
    <message>
        <source>Urgent</source>
        <translation>Dringend</translation>
    </message>
    <message>
        <source>Send anyway</source>
        <translation>Trotzdem senden</translation>
    </message>
    <message>
        <source>Drag Users Here
Right Click for Options</source>
        <translation>Benutzer hierher ziehen.
Klick mit rechter Maustaste für Einstellungen</translation>
    </message>
    <message>
        <source>direct</source>
        <translation>direkt</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>fehlgeschlagen</translation>
    </message>
    <message>
        <source>via server</source>
        <translation>über Server</translation>
    </message>
    <message>
        <source>U&amp;rgent</source>
        <translation>&amp;Dringend</translation>
    </message>
    <message>
        <source>%1 is in %2 mode:
%3
Send...</source>
        <translation>%1 ist im %2-Modus:
%3
Senden...</translation>
    </message>
    <message>
        <source>Direct send failed,
send through server?</source>
        <translation>Direkter Versand fehlgeschlagen,
über Server senden?</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>Ab&amp;brechen</translation>
    </message>
    <message>
        <source>Warning: Message can&apos;t be sent securely
through the server!</source>
        <translation>Warnung: Nachricht kann nicht verschlüsselt
über den Server gesendet werden!</translation>
    </message>
    <message>
        <source>Sending </source>
        <translation>Sende </translation>
    </message>
    <message>
        <source>Chat Request</source>
        <translation>Chat-Anfrage</translation>
    </message>
    <message>
        <source>Contact List</source>
        <translation>Kontaktliste</translation>
    </message>
    <message>
        <source> to Contact List</source>
        <translation>zur Kontaktliste.</translation>
    </message>
    <message>
        <source>Error! no owner set</source>
        <translation>Fehler! Kein Zugang eingerichtet</translation>
    </message>
    <message>
        <source>M&amp;ultiple recipients</source>
        <translation>&amp;Mehrere Empfänger</translation>
    </message>
    <message>
        <source>Se&amp;nd through server</source>
        <translation>Über S&amp;erver senden</translation>
    </message>
    <message>
        <source>%1 has joined the conversation.</source>
        <translation>%1 ist der Konversation beigetreten.</translation>
    </message>
    <message>
        <source>%1 has left the conversation.</source>
        <translation>%1 hat die Konversation verlassen.</translation>
    </message>
</context>
<context>
    <name>UserSendContactEvent</name>
    <message>
        <source>Drag Users Here - Right Click for Options</source>
        <translation>Benutzer hierher ziehen - Klick mit rechter Maustaste für Einstellungen</translation>
    </message>
    <message>
        <source> - Contact List</source>
        <translation> - Kontaktliste</translation>
    </message>
</context>
<context>
    <name>UserSendFileEvent</name>
    <message>
        <source>Edit</source>
        <translation>Bearbeiten</translation>
    </message>
    <message>
        <source>You must specify a file to transfer!</source>
        <translation>Es muss eine zu sendende Datei angegeben werden!</translation>
    </message>
    <message>
        <source>File(s): </source>
        <translation>Datei(en): </translation>
    </message>
    <message>
        <source>Browse</source>
        <translation>Durchsuchen</translation>
    </message>
    <message>
        <source> - File Transfer</source>
        <translation> - Dateiübertragung</translation>
    </message>
    <message>
        <source>File transfer with %2 refused:
%3</source>
        <translation>Dateiübertragung mit %2 abgelehnt:
%3</translation>
    </message>
    <message>
        <source>No reason provided</source>
        <translation>Kein Grund angegeben</translation>
    </message>
    <message>
        <source>Select files to send</source>
        <translation>Dateien zum Versenden auswählen</translation>
    </message>
</context>
<context>
    <name>UserSendMsgEvent</name>
    <message>
        <source>&amp;No</source>
        <translation>&amp;Nein</translation>
    </message>
    <message>
        <source>&amp;Yes</source>
        <translation>&amp;Ja</translation>
    </message>
    <message>
        <source> - Message</source>
        <translation> - Nachricht</translation>
    </message>
    <message>
        <source>You didn&apos;t edit the message.
Do you really want to send it?</source>
        <translation>Sie haben die Nachricht nicht bearbeitet.
Wollen Sie sie wirklich senden?</translation>
    </message>
</context>
<context>
    <name>UserSendSmsEvent</name>
    <message>
        <source>&amp;No</source>
        <translation>&amp;Nein</translation>
    </message>
    <message>
        <source>&amp;Yes</source>
        <translation>&amp;Ja</translation>
    </message>
    <message>
        <source> - SMS</source>
        <translation> - SMS</translation>
    </message>
    <message>
        <source>You didn&apos;t edit the SMS.
Do you really want to send it?</source>
        <translation>Sie haben die SMS nicht bearbeitet.
Möchten Sie sie wirklich senden?</translation>
    </message>
    <message>
        <source>Chars left : </source>
        <translation>Verbleibende Zeichen : </translation>
    </message>
    <message>
        <source>Phone : </source>
        <translation>Telefon : </translation>
    </message>
</context>
<context>
    <name>UserSendUrlEvent</name>
    <message>
        <source>No URL specified</source>
        <translation>Keine URL angegeben</translation>
    </message>
    <message>
        <source> - URL</source>
        <translation> - URL</translation>
    </message>
    <message>
        <source>URL : </source>
        <translation>URL : </translation>
    </message>
</context>
<context>
    <name>UserViewEvent</name>
    <message>
        <source>Chat</source>
        <translation>Chat</translation>
    </message>
    <message>
        <source>&amp;Join</source>
        <translation>&amp;Betreten</translation>
    </message>
    <message>
        <source>&amp;Menu</source>
        <translation>&amp;Menü</translation>
    </message>
    <message>
        <source>&amp;View</source>
        <translation>&amp;Anzeigen</translation>
    </message>
    <message>
        <source>Nex&amp;t</source>
        <translation>&amp;Nächste</translation>
    </message>
    <message>
        <source>A&amp;dd %1 Users</source>
        <translation>%1 Benutzer hinzu&amp;fügen</translation>
    </message>
    <message>
        <source>Aut&amp;o Close</source>
        <translation>Automatisch s&amp;chließen</translation>
    </message>
    <message>
        <source>Licq is unable to start your browser and open the URL.
You will need to start the browser and open the URL manually.</source>
        <translation>Licq kann den Browser nicht starten um die URL zu öffnen
Sie müssen den Browser selbst starten und die URL in die Adressleiste kopieren.</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>S&amp;chließen</translation>
    </message>
    <message>
        <source>&amp;Quote</source>
        <translation>&amp;Zitieren</translation>
    </message>
    <message>
        <source>&amp;Reply</source>
        <translation>&amp;Antworten</translation>
    </message>
    <message>
        <source>A&amp;ccept</source>
        <translation>A&amp;kzeptieren</translation>
    </message>
    <message>
        <source>Normal Click - Close Window
&lt;CTRL&gt;+Click - also delete User</source>
        <translation>Normaler Klick - Fenster schließen
Strg + Klick - Benutzer löschen</translation>
    </message>
    <message>
        <source>File Transfer</source>
        <translation>Dateiübertragung</translation>
    </message>
    <message>
        <source>Start Chat</source>
        <translation>Chat starten</translation>
    </message>
    <message>
        <source>&amp;View Email</source>
        <translation>&amp;E-Mail Anzeigen</translation>
    </message>
    <message>
        <source>
--------------------
Request was cancelled.</source>
        <translation> 
--------------------
Anfrage wurde abgebrochen.</translation>
    </message>
    <message>
        <source>A&amp;dd User</source>
        <translation>Benutzer hinzu&amp;fügen</translation>
    </message>
    <message>
        <source>A&amp;uthorize</source>
        <translation>&amp;Autorisieren</translation>
    </message>
    <message>
        <source>&amp;Refuse</source>
        <translation>&amp;Ablehnen</translation>
    </message>
    <message>
        <source>Nex&amp;t (%1)</source>
        <translation>&amp;Nächste (%1)</translation>
    </message>
    <message>
        <source>&amp;Forward</source>
        <translation>&amp;Weiterleiten</translation>
    </message>
    <message>
        <source>Licq is unable to find a browser application due to an internal error.</source>
        <translation>Licq kann aufgrund eines internen Fehlers keine Browseranwendung finden.</translation>
    </message>
    <message>
        <source>&amp;View Info</source>
        <translation>&amp;Informationen</translation>
    </message>
</context>
<context>
    <name>VerifyDlg</name>
    <message>
        <source>&amp;OK</source>
        <translation>&amp;Ok</translation>
    </message>
    <message>
        <source>Retype the letters shown above:</source>
        <translation>Geben Sie die oben zu sehenden Zeichen ein:</translation>
    </message>
    <message>
        <source>Licq - New Account Verification</source>
        <translation>Licq - Prüfung eines neuen Zugangs</translation>
    </message>
</context>
<context>
    <name>WharfIcon</name>
    <message>
        <source>Left click - Show main window
Middle click - Show next message
Right click - System menu</source>
        <translation>Linke Maustaste - Hauptfenster zeigen
Mittlere Maustaste - Nächste Nachricht zeigen
Rechte Maustaste - System-Menü</translation>
    </message>
</context>
</TS>
