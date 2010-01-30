<!DOCTYPE TS><TS>
<context>
    <name>@default</name>
    <message>
        <source>KDE default</source>
        <translation>KDE standard</translation>
    </message>
</context>
<context>
    <name>AddUserDlg</name>
    <message>
        <source>Licq - Add User</source>
        <translation>Licq - Lägg till användare</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Avbryt</translation>
    </message>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>Protocol:</source>
        <translation>Protokoll:</translation>
    </message>
    <message>
        <source>New User ID:</source>
        <translation>Nytt AnvändarID:</translation>
    </message>
</context>
<context>
    <name>AuthUserDlg</name>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>Refuse authorization to %1</source>
        <translation>Godkänn inte %1</translation>
    </message>
    <message>
        <source>Grant authorization to %1</source>
        <translation>Godkänn %1</translation>
    </message>
    <message>
        <source>Licq - Grant Authorisation</source>
        <translation>Licq - godkänn</translation>
    </message>
    <message>
        <source>Licq - Refuse Authorisation</source>
        <translation>Licq - godkänn ej</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Avbryt</translation>
    </message>
    <message>
        <source>Response</source>
        <translation>Svar</translation>
    </message>
    <message>
        <source>Authorize which user (Id):</source>
        <translation>Godkänn vilken användare (ID):</translation>
    </message>
</context>
<context>
    <name>AwayMsgDlg</name>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>I&apos;m currently %1, %a.
You can leave me a message.
(%m messages pending from you).</source>
        <translation>Jag är förnärvarande %1, %a.
Du kan lämna ett meddelande.
(%m vilande meddelanden från dig).</translation>
    </message>
    <message>
        <source>&amp;Edit Items</source>
        <translation>&amp;Redigera föremål</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Avbryt</translation>
    </message>
    <message>
        <source>&amp;Select</source>
        <translation>&amp;Välj</translation>
    </message>
    <message>
        <source>Set %1 Response for %2</source>
        <translation>Ange %1 som svar till %2</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Hints for Setting&lt;br&gt;your Auto-Response&lt;/h2&gt;&lt;br&gt;&lt;hr&gt;&lt;br&gt;&lt;ul&gt;&lt;li&gt;You can include any of the % expansions (described in the main hints page).&lt;/li&gt;&lt;li&gt;Any line beginning with a pipe (|) will be treated as a command to be run.  The line will be replaced by the output of the command. The command is parsed by /bin/sh so any shell commands or meta-characters are allowed.  For security reasons, any % expansions are automatically passed to the command surrounded by single quotes to prevent shell parsing of any meta-characters included in an alias.&lt;br&gt;Examples of popular uses include:&lt;ul&gt;&lt;li&gt;&lt;tt&gt;|date&lt;/tt&gt;: Will replace that line by the current date&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|fortune&lt;/tt&gt;: Show a fortune, as a tagline for example&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|myscript.sh %u %a&lt;/tt&gt;: Run a script, passing the uin and alias&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|myscript.sh %u %a &gt; /dev/null&lt;/tt&gt;: Run the same script but ignore the output (for tracking auto response checks or something)&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|if [ %u -lt 100000 ]; then echo &quot;You are special&quot;; fi&lt;/tt&gt;: Useless, but shows how you can use shell script.&lt;/li&gt;&lt;/ul&gt;Of course, multiple &quot;|&quot; can appear in the auto response, and commands and regular text can be mixed line by line.&lt;/li&gt;&lt;hr&gt;&lt;p&gt; For more information, see the Licq webpage (&lt;tt&gt;http://www.licq.org&lt;/tt&gt;).&lt;/p&gt;</source>
        <translation>&lt;h2&gt;Tips för användning av&lt;br&gt;Auto-svar&lt;/h2&gt;&lt;br&gt;&lt;hr&gt;&lt;br&gt;&lt;ul&gt;&lt;li&gt;You can include any of the % expansions (described in the main hints page).&lt;/li&gt;&lt;li&gt;Varje rad som börjar med en pipa (|) kommer att hanteras som ett kommando som ska köras. Raden kommer att ersättas med utmatningen från kommandot. Kommandot tolkas av /bin/sh så alla shell-kommandon och meta-tecken är tillåtna. Av säkerhetsskäl skickas alla %-uttryck till kommandot omgivna av enkla citattecken för att undvika att ev. meta-tecken i ett smeknamn tolkas av shellet.&lt;br&gt;Några exempel på populära användningsområden:&lt;ul&gt;&lt;li&gt;&lt;tt&gt;|date&lt;/tt&gt;: Kommer att ersätta den raden med dagens datum&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|fortune&lt;/tt&gt;: Visa en &quot;fortune&quot; som t.ex en &quot;tagline&quot;&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|myscript.sh %u %a&lt;/tt&gt;: Kör ett skript och ger det variablerna uin och alias&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|myscript.sh %u %a &gt; /dev/null&lt;/tt&gt;: Kör samma skript men strunta i utmatningen (t.ex för att räkna auto-svars kontroller eller liknande) &lt;/li&gt;&lt;li&gt;&lt;tt&gt;|if [ %u -lt 100000 ]; then echo &quot;Du är speciell&quot;; fi&lt;/tt&gt;: Oanvändbart men visar hur du kan använda shell-skript.&lt;/li&gt;&lt;/ul&gt;Det kan naturligtvis förekomma flera &quot;|&quot; i ett autosvar och kommandon och vanlig text kan blandas rad för rad.&lt;/li&gt;&lt;hr&gt;&lt;p&gt; Mer information hittar du på Licqs webbplats (&lt;tt&gt;http://www.licq.org&lt;/tt&gt;).&lt;/p&gt;</translation>
    </message>
    <message>
        <source>&amp;Hints</source>
        <translation>&amp;Tips</translation>
    </message>
    <message>
        <source>(Closing in %1)</source>
        <translation>(Stänger om %1)</translation>
    </message>
</context>
<context>
    <name>CEditFileListDlg</name>
    <message>
        <source>Licq - Files to send</source>
        <translation>Licq - Filer att skicka</translation>
    </message>
    <message>
        <source>&amp;Done</source>
        <translation>&amp;Klar</translation>
    </message>
    <message>
        <source>&amp;Up</source>
        <translation>&amp;Upp</translation>
    </message>
    <message>
        <source>&amp;Down</source>
        <translation>&amp;Ner</translation>
    </message>
    <message>
        <source>&amp;Delete</source>
        <translation>&amp;Ta Bort</translation>
    </message>
</context>
<context>
    <name>CEmoticons</name>
    <message>
        <source>Default</source>
        <translation>Standard</translation>
    </message>
    <message>
        <source>None</source>
        <translation>Inga</translation>
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
        <source>Byte</source>
        <translation>Byte</translation>
    </message>
    <message>
        <source>ETA:</source>
        <translation>ETA:</translation>
    </message>
    <message>
        <source>%1/%2</source>
        <translation>%1/%2</translation>
    </message>
    <message>
        <source>Bytes</source>
        <translation>Bytes</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Stäng</translation>
    </message>
    <message>
        <source>File:</source>
        <translation>Fil:</translation>
    </message>
    <message>
        <source>Retry</source>
        <translation>Försök igen</translation>
    </message>
    <message>
        <source>Time:</source>
        <translation>Tid:</translation>
    </message>
    <message>
        <source>Licq - File Transfer (%1)</source>
        <translation>Filöverföring (%1)</translation>
    </message>
    <message>
        <source>File already exists and appears incomplete.</source>
        <translation>Filen finns redan, men ser ut att vara ofullständig.</translation>
    </message>
    <message>
        <source>Batch:</source>
        <translation>Bunt:</translation>
    </message>
    <message>
        <source>File already exists and is at least as big as the incoming file.</source>
        <translation>Filen finns redan och är åtminstone lika stor som den inkommande filen.</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Avbryt</translation>
    </message>
    <message>
        <source>Resume</source>
        <translation>Återuppta</translation>
    </message>
    <message>
        <source>File name:</source>
        <translation>Filnamn:</translation>
    </message>
    <message>
        <source>Current:</source>
        <translation>Nuvarande</translation>
    </message>
    <message>
        <source>Open error - unable to open file for writing.</source>
        <translation>Öppnafel - kunde inte öppna fil för skrivning.</translation>
    </message>
    <message>
        <source>Overwrite</source>
        <translation>Skriv över</translation>
    </message>
    <message>
        <source>&amp;Cancel Transfer</source>
        <translation>&amp;Avbryt Överföring</translation>
    </message>
    <message>
        <source>File transfer cancelled.</source>
        <translation>Filöverföring avbruten.</translation>
    </message>
    <message>
        <source>Waiting for connection...</source>
        <translation>Väntar på anslutning...</translation>
    </message>
    <message>
        <source>Receiving file...</source>
        <translation>Tar emot fil...</translation>
    </message>
    <message>
        <source>Sending file...</source>
        <translation>Skickar fil...</translation>
    </message>
    <message>
        <source>Received %1 from %2 successfully.</source>
        <translation>Emottog %1 från %2 utan problem.</translation>
    </message>
    <message>
        <source>Sent %1 to %2 successfully.</source>
        <translation>%1 skickad till %2 utan problem.</translation>
    </message>
    <message>
        <source>File transfer complete.</source>
        <translation>Filöverföring färdig.</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Remote side disconnected.</source>
        <translation>Mottagaren kopplade ifrån.</translation>
    </message>
    <message>
        <source>File I/O error: %1.</source>
        <translation>I/O fel på fil: %1.</translation>
    </message>
    <message>
        <source>File I/O Error:
%1

See Network Window for details.</source>
        <translation>I/O fel på fil:
%1

Se nätverksfönstret för mer information.</translation>
    </message>
    <message>
        <source>Handshaking error.
</source>
        <translation>Fel vid upprättandet av förbindelsen.</translation>
    </message>
    <message>
        <source>Handshake Error.
See Network Window for details.</source>
        <translation>Fel vid upprättandet av förbindelsen. Se nätverksfönstret för mer information.</translation>
    </message>
    <message>
        <source>Connection error.
</source>
        <translation>Anslutningsfel.</translation>
    </message>
    <message>
        <source>Unable to reach remote host.
See Network Window for details.</source>
        <translation>Kan ej nå mottagar datorn.
Se nätverksfönstret för mer information.</translation>
    </message>
    <message>
        <source>Bind error.
</source>
        <translation>Bindningsfel.
</translation>
    </message>
    <message>
        <source>Unable to bind to a port.
See Network Window for details.</source>
        <translation>Kunde inte binda till port.
Se nätverksfönstret för mer information.</translation>
    </message>
    <message>
        <source>Not enough resources.
</source>
        <translation>Ej tillräckligt med resurser.</translation>
    </message>
    <message>
        <source>Unable to create a thread.
See Network Window for details.</source>
        <translation>Kunde inte skapa en tråd.
se nätverksfönstret för mer information.</translation>
    </message>
    <message>
        <source>Connecting to remote...</source>
        <translation>Ansluter till mottagare...</translation>
    </message>
    <message>
        <source>&amp;Open</source>
        <translation>&amp;Öppna</translation>
    </message>
    <message>
        <source>O&amp;pen Dir</source>
        <translation>Öppna &amp;katalog</translation>
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
        <translation>Meddelande</translation>
    </message>
    <message>
        <source>Forward %1 To User</source>
        <translation>Vidarebefodra %1 till användare</translation>
    </message>
    <message>
        <source>Drag the user to forward to here:</source>
        <translation>Dra användaren, som du vill vidarebfodra till, hit:</translation>
    </message>
    <message>
        <source>Unable to forward this message type (%d).</source>
        <translation>Kan inte vidarebefodra denna typ av meddelande (%d).</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Avbryt</translation>
    </message>
    <message>
        <source>Forwarded message:
</source>
        <translation>Vidarebefodrat meddelande:
</translation>
    </message>
    <message>
        <source>&amp;Forward</source>
        <translation>&amp;Vidarebefodra</translation>
    </message>
    <message>
        <source>Forwarded URL:
</source>
        <translation>Vidarebefodra URL:
</translation>
    </message>
</context>
<context>
    <name>CInfoField</name>
    <message>
        <source>Unknown</source>
        <translation type="unfinished">Okänd</translation>
    </message>
</context>
<context>
    <name>CJoinChatDlg</name>
    <message>
        <source>Select chat to join:</source>
        <translation>Välj chatt att delta i:</translation>
    </message>
    <message>
        <source>Invite to Join Chat</source>
        <translation>Bjud in till att delta i chatt</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Avbryt</translation>
    </message>
    <message>
        <source>Select chat to invite:</source>
        <translation>Välj chatt att inbjuda:</translation>
    </message>
    <message>
        <source>Join Multiparty Chat</source>
        <translation>Delta i gruppchatt</translation>
    </message>
    <message>
        <source>&amp;Invite</source>
        <translation>&amp;Bjud in</translation>
    </message>
    <message>
        <source>&amp;Join</source>
        <translation>&amp;Gå med</translation>
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
        <translation>&amp;Lista</translation>
    </message>
    <message>
        <source>&amp;Next</source>
        <translation>&amp;Nästa</translation>
    </message>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>&amp;Clear All</source>
        <translation>&amp;Rensa alla</translation>
    </message>
    <message>
        <source>&amp;Next (%1)</source>
        <translation>&amp;Nästa (%1)</translation>
    </message>
    <message>
        <source>Licq Information</source>
        <translation>Licq Information</translation>
    </message>
    <message>
        <source>Licq Warning</source>
        <translation>Licq Varning</translation>
    </message>
    <message>
        <source>Licq Critical</source>
        <translation>Licq Kritisk</translation>
    </message>
</context>
<context>
    <name>CMMSendDlg</name>
    <message>
        <source>Sending mass message to %1...</source>
        <translation>Skickar massmeddelande till %1...</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>S&amp;täng</translation>
    </message>
    <message>
        <source>Sending mass URL to %1...</source>
        <translation>Skickar mass-URL till %1...</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>misslyckades</translation>
    </message>
    <message>
        <source>Multiple Recipient Contact List</source>
        <translation>Kontaktlista för flera användare</translation>
    </message>
    <message>
        <source>Multiple Recipient URL</source>
        <translation>URL till flera mottagare</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Avbryt</translation>
    </message>
    <message>
        <source>Sending mass list to %1...</source>
        <translation>Skickar masslista till %1...</translation>
    </message>
    <message>
        <source>Multiple Recipient Message</source>
        <translation>Meddelande till flera mottagare</translation>
    </message>
</context>
<context>
    <name>CMMUserView</name>
    <message>
        <source>Crop</source>
        <translation>Beskär</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation>Rensa</translation>
    </message>
    <message>
        <source>Remove</source>
        <translation>Ta bort</translation>
    </message>
    <message>
        <source>Add Group</source>
        <translation>Lägg till grupp</translation>
    </message>
    <message>
        <source>Add All</source>
        <translation>Lägg till alla</translation>
    </message>
    <message>
        <source>Drag&apos;n&apos;Drop didn&apos;t work</source>
        <translation>Dra och släpp fungerade inte</translation>
    </message>
</context>
<context>
    <name>CMainWindow</name>
    <message>
        <source>s</source>
        <translation></translation>
    </message>
    <message>
        <source>Ok</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>&amp;No</source>
        <translation>&amp;Nej</translation>
    </message>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>&amp;Yes</source>
        <translation>&amp;Ja</translation>
    </message>
    <message>
        <source>Send</source>
        <translation>Skicka</translation>
    </message>
    <message>
        <source>Online to User</source>
        <translation>Ansluten för användare</translation>
    </message>
    <message>
        <source>&amp;Away</source>
        <translation>&amp;Borta</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>&amp;Hjälp</translation>
    </message>
    <message>
        <source>&amp;Info</source>
        <translation>&amp;Info</translation>
    </message>
    <message>
        <source>U&amp;tilities</source>
        <translation>&amp;Verktyg</translation>
    </message>
    <message>
        <source>E&amp;xit</source>
        <translation>A&amp;vsluta</translation>
    </message>
    <message>
        <source>&amp;Options...</source>
        <translation>&amp;Alternativ...</translation>
    </message>
    <message>
        <source>R&amp;andom Chat</source>
        <translation>Slump&amp;chatt</translation>
    </message>
    <message>
        <source>&amp;Random Chat Group</source>
        <translation>&amp;Slumpmässig chattgrupp</translation>
    </message>
    <message>
        <source>Up since %1
</source>
        <translation>Igång sedan %1
</translation>
    </message>
    <message>
        <source>Check %1 Response</source>
        <translation>Kontrollera %1 svar</translation>
    </message>
    <message>
        <source>Unable to open icons file
%1.</source>
        <translation>Kunde inte öppna ikonfil
%1.</translation>
    </message>
    <message>
        <source>Send &amp;Authorization</source>
        <translation>Skicka &amp;godkännande</translation>
    </message>
    <message>
        <source>&amp;Not Available</source>
        <translation>&amp;Ej tillgänglig</translation>
    </message>
    <message>
        <source>&amp;About</source>
        <translation>&amp;Om</translation>
    </message>
    <message>
        <source>&amp;Group</source>
        <translation>&amp;Grupp</translation>
    </message>
    <message>
        <source>&amp;Hints</source>
        <translation>&amp;Tips</translation>
    </message>
    <message>
        <source>&amp;Reset</source>
        <translation>&amp;Återställ</translation>
    </message>
    <message>
        <source>Custom Auto Response...</source>
        <translation>Anpassat autosvar...</translation>
    </message>
    <message>
        <source>Set &amp;Auto Response...</source>
        <translation>Ange &amp;autosvar...</translation>
    </message>
    <message>
        <source>Remove From List</source>
        <translation>Ta bort från listan</translation>
    </message>
    <message>
        <source>System Message</source>
        <translation>Systemmeddelande</translation>
    </message>
    <message>
        <source>%1 msg%2</source>
        <translation>%1 med.%2</translation>
    </message>
    <message>
        <source>&amp;Popup All Messages</source>
        <translation>&amp;Visa automatiskt alla meddelanden</translation>
    </message>
    <message>
        <source>Accept in Not Available</source>
        <translation>Tillåt i Ej tillgänglig</translation>
    </message>
    <message>
        <source>View &amp;History</source>
        <translation>Visa &amp;Historik</translation>
    </message>
    <message>
        <source>Other Users</source>
        <translation>Andra användare</translation>
    </message>
    <message>
        <source>Edit User Group</source>
        <translation>Redigera användargrupper</translation>
    </message>
    <message>
        <source>Unknown Packets</source>
        <translation>Okända paket</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Avbryt</translation>
    </message>
    <message>
        <source>&amp;Mini Mode</source>
        <translation>&amp;Mini-läge</translation>
    </message>
    <message>
        <source>Errors</source>
        <translation>Fel</translation>
    </message>
    <message>
        <source>Close &amp;Secure Channel</source>
        <translation>Stäng &amp;säker kanal</translation>
    </message>
    <message>
        <source>No msgs</source>
        <translation>Inga med.</translation>
    </message>
    <message>
        <source>Up since %1

</source>
        <translation>Igång sedan %1

</translation>
    </message>
    <message>
        <source>Daemon Statistics

</source>
        <translation>Statistik för bakgrundsprocess

</translation>
    </message>
    <message>
        <source>Daemon Statistics
(Today/Total)
</source>
        <translation>Statistik för bakgrundsprocess
(Idag/Totalt)
</translation>
    </message>
    <message>
        <source>SysMsg</source>
        <translation>SysMed</translation>
    </message>
    <message>
        <source>System</source>
        <translation>System</translation>
    </message>
    <message>
        <source>Last reset %1

</source>
        <translation>Senaste återställning %1

</translation>
    </message>
    <message>
        <source>Are you sure you want to remove
%1 (%2)
from the &apos;%3&apos; group?</source>
        <translation>Är du säker på att du vill ta bort
%1 (%2)
från gruppen &apos;%3&apos;?</translation>
    </message>
    <message>
        <source>Do you really want to add
%1 (%2)
to your ignore list?</source>
        <translation>Vill du verkligen lägga till
%1 (%2)
till din ignoreringslista?</translation>
    </message>
    <message>
        <source>Packets</source>
        <translation>Paket</translation>
    </message>
    <message>
        <source>&amp;Save All Users</source>
        <translation>&amp;Spara alla användare</translation>
    </message>
    <message>
        <source>%1: %2 / %3
</source>
        <translation>%1: %2 / %3
</translation>
    </message>
    <message>
        <source>Do Not Disturb to User</source>
        <translation>Stör ej för användare</translation>
    </message>
    <message>
        <source>Update All Users</source>
        <translation>Uppdatera alla användare</translation>
    </message>
    <message>
        <source>&amp;Redraw User Window</source>
        <translation>&amp;Rita om användarfönstret</translation>
    </message>
    <message>
        <source>(with KDE support)
</source>
        <translation>(med KDE-stöd)
</translation>
    </message>
    <message>
        <source>Occupied to User</source>
        <translation>Upptagen för användare</translation>
    </message>
    <message>
        <source>Logon failed.
See network window for details.</source>
        <translation>Anslutning misslyckades.
Se nätverksfönstret för mer information.</translation>
    </message>
    <message>
        <source>Accept in Away</source>
        <translation>Tillåt i borta</translation>
    </message>
    <message>
        <source>Send &amp;Chat Request</source>
        <translation>Skicka &amp;chattbegäran</translation>
    </message>
    <message>
        <source>Show Offline &amp;Users</source>
        <translation>Visa frånkopplade &amp;användare</translation>
    </message>
    <message>
        <source>%1 message%2</source>
        <translation>%1 meddelande%2</translation>
    </message>
    <message>
        <source>%1: %2
</source>
        <translation>%1: %2
</translation>
    </message>
    <message>
        <source>Warnings</source>
        <translation>Varningar</translation>
    </message>
    <message>
        <source>Debug Level</source>
        <translation>Felsökningsnivå</translation>
    </message>
    <message>
        <source>&amp;Network Window</source>
        <translation>&amp;Nätverksfönstret</translation>
    </message>
    <message>
        <source>&amp;View Event</source>
        <translation>&amp;Visa Händelse</translation>
    </message>
    <message>
        <source>&amp;Plugin Manager...</source>
        <translation>&amp;Instickshanterare...</translation>
    </message>
    <message>
        <source>O&amp;ccupied</source>
        <translation>&amp;Upptagen</translation>
    </message>
    <message>
        <source>Set All</source>
        <translation>Ange alla</translation>
    </message>
    <message>
        <source>&amp;View System Messages</source>
        <translation>&amp;Visa systemmeddelanden</translation>
    </message>
    <message>
        <source>&amp;Thread Group View</source>
        <translation>&amp;Trådad gruppvy</translation>
    </message>
    <message>
        <source>Send &amp;Message</source>
        <translation>Skicka &amp;meddelande</translation>
    </message>
    <message>
        <source>Update Current Group</source>
        <translation>Uppdatera aktuell grupp</translation>
    </message>
    <message>
        <source>Licq (%1)</source>
        <translation>Licq (%1)</translation>
    </message>
    <message>
        <source>Right click - Status menu
Double click - Set auto response</source>
        <translation>Högerklicka - Statusmeny
Dubbelklicka - Sätt autosvar</translation>
    </message>
    <message>
        <source>&amp;Add User</source>
        <translation>&amp;Lägg till användare</translation>
    </message>
    <message>
        <source>Accept in Do Not Disturb</source>
        <translation>Tillåt i Stör ej</translation>
    </message>
    <message>
        <source>&amp;Statistics</source>
        <translation>&amp;Statistik</translation>
    </message>
    <message>
        <source>&amp;Online</source>
        <translation>&amp;Ansluten</translation>
    </message>
    <message>
        <source>Clear All</source>
        <translation>Rensa alla</translation>
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
        <translation>Tillåt i Upptagen</translation>
    </message>
    <message>
        <source>Send Contact &amp;List</source>
        <translation>Skicka kontakt&amp;lista</translation>
    </message>
    <message>
        <source>Status Info</source>
        <translation>Statusinfo</translation>
    </message>
    <message>
        <source>&amp;Do Not Disturb</source>
        <translation>&amp;Stör ej</translation>
    </message>
    <message>
        <source>Are you sure you want to remove
%1 (%2)
from your contact list?</source>
        <translation>Är du säker på att du vill ta bort
%1 (%2)
från din kontaktlista?</translation>
    </message>
    <message>
        <source>O&amp;ffline</source>
        <translation>&amp;Frånkopplad</translation>
    </message>
    <message>
        <source>A&amp;uthorize User</source>
        <translation>&amp;Godkänn användare</translation>
    </message>
    <message>
        <source>Free for C&amp;hat</source>
        <translation>Chatt&amp;ledig</translation>
    </message>
    <message>
        <source>Right click - User groups
Double click - Show next message</source>
        <translation>Högerklicka - Användargrupper
Dubbelklicka - Visa nästa meddelande</translation>
    </message>
    <message>
        <source>No messages</source>
        <translation>Inga meddelanden</translation>
    </message>
    <message>
        <source>&amp;Invisible</source>
        <translation>&amp;Osynlig</translation>
    </message>
    <message>
        <source>Send &amp;File Transfer</source>
        <translation>Skicka &amp;fil</translation>
    </message>
    <message>
        <source>User Functions</source>
        <translation>Användarfunktioner</translation>
    </message>
    <message>
        <source>Check Auto Response</source>
        <translation>Kontrollera autosvar</translation>
    </message>
    <message>
        <source>System Functions</source>
        <translation>Systemfunktioner</translation>
    </message>
    <message>
        <source>Edit &amp;Groups</source>
        <translation>Redigera &amp;grupper</translation>
    </message>
    <message>
        <source>S&amp;kin Browser...</source>
        <translation>&amp;Temaväljare...</translation>
    </message>
    <message>
        <source>Request &amp;Secure Channel</source>
        <translation>Begär &amp;säker kanal</translation>
    </message>
    <message>
        <source>S&amp;earch for User</source>
        <translation>S&amp;ök efter användare</translation>
    </message>
    <message>
        <source>Not Available to User</source>
        <translation>Ej tillgänglig för användare</translation>
    </message>
    <message>
        <source>Toggle &amp;Floaty</source>
        <translation>Växla &amp;flytande</translation>
    </message>
    <message>
        <source>Sa&amp;ve Settings</source>
        <translation>S&amp;para inställningar</translation>
    </message>
    <message>
        <source>Away to User</source>
        <translation>Borta för användare</translation>
    </message>
    <message>
        <source>Send &amp;URL</source>
        <translation>Skicka &amp;länk</translation>
    </message>
    <message>
        <source>Send Authorization Re&amp;quest</source>
        <translation>Skicka begäran om &amp;godkännande</translation>
    </message>
    <message>
        <source>Send &amp;SMS</source>
        <translation>Skicka &amp;SMS</translation>
    </message>
    <message>
        <source>&amp;Security/Password Options</source>
        <translation>&amp;Säkerhet/Lösenords inställningar</translation>
    </message>
    <message>
        <source>Re&amp;quest Authorization</source>
        <translation>&amp;Begär godkännande</translation>
    </message>
    <message>
        <source>Auto Accept Files</source>
        <translation>Acceptera filer automatiskt</translation>
    </message>
    <message>
        <source>Auto Accept Chats</source>
        <translation>Acceptera chat automatiskt</translation>
    </message>
    <message>
        <source>Auto Request Secure</source>
        <translation>Begär säker kanal automatiskt</translation>
    </message>
    <message>
        <source>Use Real Ip (LAN)</source>
        <translation>Använd sant IP (LAN)</translation>
    </message>
    <message>
        <source>Misc Modes</source>
        <translation>Diverse inställningar</translation>
    </message>
    <message>
        <source>&lt;hr&gt;&lt;p&gt; For more information, see the Licq webpage (&lt;tt&gt;http://www.licq.org&lt;/tt&gt;).&lt;/p&gt;</source>
        <translation>&lt;hr&gt;&lt;p&gt;För mer information, se Licq:s webbplats (&lt;tt&gt;http://www.licq.org&lt;/tt&gt;).&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Server Group</source>
        <translation>Server grupp</translation>
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
        <translation>Licq version %1%8.
Qt GUI plugin version %2.
Kompilerad %7
%6
Projektansvarig: Jon Keating
Bidrag från Dirk A. Mueller
Ursprunglig skapare : Graham Roff
Svensk översättning : Samuel Blomqvist

http://www.licq.org
#licq på irc.freenode.net

%3 (%4)
%5 kontakter.
</translation>
    </message>
    <message>
        <source>Unable to open extended icons file
%1.</source>
        <translation>Kunde inte öppna fil %1 med utökade ikoner.</translation>
    </message>
    <message>
        <source>Licq is unable to find a browser application due to an internal error.</source>
        <translation>Licq kan inte hitta en webläsare pga. ett internt fel.</translation>
    </message>
    <message>
        <source>Licq is unable to start your browser and open the URL.
You will need to start the browser and open the URL manually.</source>
        <translation>Licq kan inte starta din webbläsare och öppna länken.
Du måste starta din webbläsare och öppna länken manuellt.</translation>
    </message>
    <message>
        <source>Error! No owner set</source>
        <translation>Fel! Ingen användare angiven</translation>
    </message>
    <message>
        <source>&lt;b&gt;%1&lt;/b&gt; is online</source>
        <translation>&lt;b&gt;%1&lt;/b&gt; är ansluten</translation>
    </message>
    <message>
        <source></source>
        <translation></translation>
    </message>
    <message>
        <source>(Error! No owner set)</source>
        <translation>Fel! Ingen användare angiven</translation>
    </message>
    <message>
        <source>&amp;GPG Key Manager...</source>
        <translation>&amp;GPG Nyckelhanterare...</translation>
    </message>
    <message>
        <source>Don&apos;t Show</source>
        <translation>Visa Inte</translation>
    </message>
    <message>
        <source>Available</source>
        <translation>Tillgänglig</translation>
    </message>
    <message>
        <source>Busy</source>
        <translation>Upptagen</translation>
    </message>
    <message>
        <source>Phone &quot;Follow Me&quot;</source>
        <translation>Telefon &quot;Följ Mig&quot;</translation>
    </message>
    <message>
        <source>&amp;Owner Manager</source>
        <translation>&amp;Ägarhanterare</translation>
    </message>
    <message>
        <source>Use GPG Encryption</source>
        <translation>Använd GPG-kryptering</translation>
    </message>
    <message>
        <source>Update Info Plugin List</source>
        <translation>Uppdatera &quot;Info Plugin&quot; listan</translation>
    </message>
    <message>
        <source>Update Status Plugin List</source>
        <translation>Uppdatera &quot;Status Plugin&quot; listan</translation>
    </message>
    <message>
        <source>Update Phone &quot;Follow Me&quot; Status</source>
        <translation>Uppdatera Telefon &quot;Följ Mig&quot; läge</translation>
    </message>
    <message>
        <source>Update ICQphone Status</source>
        <translation>Updatera ICQphone läge</translation>
    </message>
    <message>
        <source>Update File Server Status</source>
        <translation>Uppdatera filserver status</translation>
    </message>
    <message>
        <source>Set GPG key</source>
        <translation>Ange GPGnyckel</translation>
    </message>
    <message>
        <source>Do you really want to reset your stats?</source>
        <translation>Vill du verkligen nollställa din statistik?</translation>
    </message>
    <message>
        <source> </source>
        <translation> </translation>
    </message>
    <message>
        <source>&lt;ul&gt;&lt;li&gt;&lt;tt&gt;%a - &lt;/tt&gt;user alias&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%e - &lt;/tt&gt;email&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%f - &lt;/tt&gt;first name&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%h - &lt;/tt&gt;phone number&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%i - &lt;/tt&gt;user ip&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%l - &lt;/tt&gt;last name&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%L - &lt;/tt&gt;local time&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%m - &lt;/tt&gt;# pending messages&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%M - &lt;/tt&gt;# pending messages (if any)&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%n - &lt;/tt&gt;full name&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%o - &lt;/tt&gt;last seen online&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%O - &lt;/tt&gt;online since&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%p - &lt;/tt&gt;user port&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%s - &lt;/tt&gt;full status&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%S - &lt;/tt&gt;abbreviated status&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%u - &lt;/tt&gt;uin&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%w - &lt;/tt&gt;webpage&lt;/li&gt;&lt;/ul&gt;</source>
        <translation>&lt;ul&gt;&lt;li&gt;&lt;tt&gt;%a - &lt;/tt&gt;kontaktalias&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%e - &lt;/tt&gt;epost&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%f - &lt;/tt&gt;förnamn&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%h - &lt;/tt&gt;telefonnummer&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%i - &lt;/tt&gt;kontaktip&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%l - &lt;/tt&gt;efternamn&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%L - &lt;/tt&gt;lokal tid&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%m - &lt;/tt&gt;# olästa meddelanden&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%M - &lt;/tt&gt;# olästa meddelanden (om några)&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%n - &lt;/tt&gt;fullständigt namn&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%o - &lt;/tt&gt;senast ansluten&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%O - &lt;/tt&gt;ansluten sedan&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%p - &lt;/tt&gt;kontaktport&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%s - &lt;/tt&gt;full status&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%S - &lt;/tt&gt;förkortad status&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%u - &lt;/tt&gt;uin&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%w - &lt;/tt&gt;hemsida&lt;/li&gt;&lt;/ul&gt;</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Hints for Using&lt;br&gt;the Licq Qt-GUI Plugin&lt;/h2&gt;&lt;br&gt;&lt;hr&gt;&lt;br&gt;&lt;ul&gt;&lt;li&gt;Change your status by right clicking on the status label.&lt;/li&gt;&lt;li&gt;Change your auto response by double-clicking on the status label.&lt;/li&gt;&lt;li&gt;View system messages by double clicking on the message label.&lt;/li&gt;&lt;li&gt;Change groups by right clicking on the message label.&lt;/li&gt;&lt;li&gt;Use the following shortcuts from the contact list:&lt;ul&gt;&lt;li&gt;&lt;tt&gt;Ctrl-M : &lt;/tt&gt;Toggle mini-mode&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-O : &lt;/tt&gt;Toggle show offline users&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-X : &lt;/tt&gt;Exit&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-H : &lt;/tt&gt;Hide&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-I : &lt;/tt&gt;View the next message&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-V : &lt;/tt&gt;View message&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-S : &lt;/tt&gt;Send message&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-U : &lt;/tt&gt;Send Url&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-C : &lt;/tt&gt;Send chat request&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-F : &lt;/tt&gt;Send File&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-A : &lt;/tt&gt;Check Auto response&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-P : &lt;/tt&gt;Popup all messages&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-L : &lt;/tt&gt;Redraw user window&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Delete : &lt;/tt&gt;Delete user from current group&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-Delete : &lt;/tt&gt;Delete user from contact list&lt;/li&gt;&lt;/ul&gt;&lt;li&gt;Hold control while clicking on close in the function window to remove   the user from your contact list.&lt;/li&gt;&lt;li&gt;Hit Ctrl-Enter from most text entry fields to select &quot;Ok&quot; or &quot;Accept&quot;.   For example in the send tab of the user function window.&lt;/li&gt;&lt;li&gt;Here is the complete list of user % options, which can be used in &lt;b&gt;OnEvent&lt;/b&gt;   parameters, &lt;b&gt;auto responses&lt;/b&gt;, and &lt;b&gt;utilities&lt;/b&gt;:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>CMessageViewWidget</name>
    <message>
        <source>%1 from %2</source>
        <translation>%1 från %2</translation>
    </message>
    <message>
        <source>%1 to %2</source>
        <translation>%1 till %2</translation>
    </message>
    <message>
        <source>Default</source>
        <translation>Standard</translation>
    </message>
    <message>
        <source>Compact</source>
        <translation>Kompakt</translation>
    </message>
    <message>
        <source>Tiny</source>
        <translation>Minimal</translation>
    </message>
    <message>
        <source>Table</source>
        <translation>Tabell</translation>
    </message>
    <message>
        <source>Long</source>
        <translation>Lång</translation>
    </message>
    <message>
        <source>Wide</source>
        <translation>Bred</translation>
    </message>
</context>
<context>
    <name>CQtLogWindow</name>
    <message>
        <source>&amp;Save</source>
        <translation>&amp;Spara</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>S&amp;täng</translation>
    </message>
    <message>
        <source>Failed to open file:
%1</source>
        <translation>Kunde inte öppna fil:
%1</translation>
    </message>
    <message>
        <source>C&amp;lear</source>
        <translation>&amp;Rensa</translation>
    </message>
    <message>
        <source>Licq Network Log</source>
        <translation>Licq nätverkslogg</translation>
    </message>
</context>
<context>
    <name>CRandomChatDlg</name>
    <message>
        <source>Games</source>
        <translation>Spel</translation>
    </message>
    <message>
        <source>20 Something</source>
        <translation>Omkring 20</translation>
    </message>
    <message>
        <source>30 Something</source>
        <translation>Omkring 30</translation>
    </message>
    <message>
        <source>40 Something</source>
        <translation>Omkring 40</translation>
    </message>
    <message>
        <source>Random chat search timed out.</source>
        <translation>Slumpchattsökningen dog ut.</translation>
    </message>
    <message>
        <source>Random chat search had an error.</source>
        <translation>Slumpchattökningen felade.</translation>
    </message>
    <message>
        <source>No random chat user found in that group.</source>
        <translation>Ingen slumpchattanvändare funnen i den gruppen.</translation>
    </message>
    <message>
        <source>50 Plus</source>
        <translation>Över 50</translation>
    </message>
    <message>
        <source>Romance</source>
        <translation>Romans</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Avbryt</translation>
    </message>
    <message>
        <source>Searching for Random Chat Partner...</source>
        <translation>Söker efter Slumpchattpartner</translation>
    </message>
    <message>
        <source>&amp;Search</source>
        <translation>&amp;Sök</translation>
    </message>
    <message>
        <source>Students</source>
        <translation>Studenter</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Allmänt</translation>
    </message>
    <message>
        <source>Random Chat Search</source>
        <translation>Slumpchattsökning</translation>
    </message>
    <message>
        <source>Seeking Women</source>
        <translation>Söker kvinnor</translation>
    </message>
    <message>
        <source>Seeking Men</source>
        <translation>Söker män</translation>
    </message>
</context>
<context>
    <name>CRefuseDlg</name>
    <message>
        <source>Licq %1 Refusal</source>
        <translation>Licq %1 vägran</translation>
    </message>
    <message>
        <source>Refusal message for %1 with </source>
        <translation>Vägringsmeddelande för %1 med </translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Avbryt</translation>
    </message>
    <message>
        <source>Refuse</source>
        <translation>Vägra</translation>
    </message>
</context>
<context>
    <name>CSetRandomChatGroupDlg</name>
    <message>
        <source>&amp;Set</source>
        <translation>&amp;Ange</translation>
    </message>
    <message>
        <source>done</source>
        <translation>klar</translation>
    </message>
    <message>
        <source>Games</source>
        <translation>Spel</translation>
    </message>
    <message>
        <source>error</source>
        <translation>fel</translation>
    </message>
    <message>
        <source>20 Something</source>
        <translation>Omkring 20</translation>
    </message>
    <message>
        <source>30 Something</source>
        <translation>Omkring 30</translation>
    </message>
    <message>
        <source>40 Something</source>
        <translation>Omkring 40</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Stäng</translation>
    </message>
    <message>
        <source>(none)</source>
        <translation>(ingen)</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>dog ut</translation>
    </message>
    <message>
        <source>Set Random Chat Group</source>
        <translation>Sätt slumpchattgrupp</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>misslyckades</translation>
    </message>
    <message>
        <source>50 Plus</source>
        <translation>Över 50</translation>
    </message>
    <message>
        <source>Romance</source>
        <translation>Romans</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Avbryt</translation>
    </message>
    <message>
        <source>Students</source>
        <translation>Studenter</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Allmänt</translation>
    </message>
    <message>
        <source>Setting Random Chat Group...</source>
        <translation>Sätter slumpchattgrupp...</translation>
    </message>
    <message>
        <source>Seeking Women</source>
        <translation>Söker kvinnor</translation>
    </message>
    <message>
        <source>Seeking Men</source>
        <translation>Söker män</translation>
    </message>
</context>
<context>
    <name>CTimeZoneField</name>
    <message>
        <source>Unknown</source>
        <translation>Okänd</translation>
    </message>
</context>
<context>
    <name>CUserView</name>
    <message>
        <source>S</source>
        <translation>S</translation>
    </message>
    <message>
        <source>Online</source>
        <translation>Anslutna</translation>
    </message>
    <message>
        <source>Offline</source>
        <translation>Frånkopplade</translation>
    </message>
    <message>
        <source>%1 Floaty (%2)</source>
        <translation>%1 flytande (%2)</translation>
    </message>
    <message>
        <source> weeks</source>
        <translation> veckor</translation>
    </message>
    <message>
        <source> week</source>
        <translation> vecka</translation>
    </message>
    <message>
        <source> days</source>
        <translation> dagar</translation>
    </message>
    <message>
        <source> day</source>
        <translation> dag</translation>
    </message>
    <message>
        <source> hours</source>
        <translation> timmar</translation>
    </message>
    <message>
        <source> hour</source>
        <translation> timme</translation>
    </message>
    <message>
        <source> minutes</source>
        <translation> minuter</translation>
    </message>
    <message>
        <source> minute</source>
        <translation> minut</translation>
    </message>
    <message>
        <source>0 minutes</source>
        <translation>0 minuter</translation>
    </message>
    <message>
        <source>Not In List</source>
        <translation>Ej i listan</translation>
    </message>
    <message>
        <source>Birthday Today!</source>
        <translation>Födelsedag idag!</translation>
    </message>
    <message>
        <source>Typing a message</source>
        <translation>Skriver ett meddelande</translation>
    </message>
    <message>
        <source>Phone &amp;quot;Follow Me&amp;quot;: Available</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Phone &amp;quot;Follow Me&amp;quot;: Busy</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>ICQphone: Available</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>ICQphone: Busy</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>File Server: Enabled</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Secure connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Custom Auto Response</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Auto Response:</source>
        <translation type="unfinished"></translation>
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
        <translation>Ip: </translation>
    </message>
    <message>
        <source>O: </source>
        <translation>O: </translation>
    </message>
    <message>
        <source>Logged In: </source>
        <translation>Inloggad: </translation>
    </message>
    <message>
        <source>Idle: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Local time: </source>
        <translation>Lokal tid: </translation>
    </message>
    <message>
        <source>ID: </source>
        <translation>ID: </translation>
    </message>
    <message>
        <source>Awaiting authorization</source>
        <translation>Väntar på godkännande</translation>
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
        <translation>&amp;Kör</translation>
    </message>
    <message>
        <source>Done</source>
        <translation>Klar</translation>
    </message>
    <message>
        <source>Done:</source>
        <translation>Klar:</translation>
    </message>
    <message>
        <source>Edit:</source>
        <translation>Redigera:</translation>
    </message>
    <message>
        <source>Edit final command</source>
        <translation>Redigera slutgiltigt kommando</translation>
    </message>
    <message>
        <source>Command Window</source>
        <translation>Kommandofönster</translation>
    </message>
    <message>
        <source>Licq Utility: %1</source>
        <translation>Licq verktyg: %1</translation>
    </message>
    <message>
        <source>C&amp;lose</source>
        <translation>&amp;Stäng</translation>
    </message>
    <message>
        <source>User Fields</source>
        <translation>Användarfält</translation>
    </message>
    <message>
        <source>Description:</source>
        <translation>Beskrivning:</translation>
    </message>
    <message>
        <source>Internal</source>
        <translation>Internt</translation>
    </message>
    <message>
        <source>Command:</source>
        <translation>Kommando:</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Avbryt</translation>
    </message>
    <message>
        <source>Running:</source>
        <translation>Igång:</translation>
    </message>
    <message>
        <source>Failed:</source>
        <translation>Misslyckades:</translation>
    </message>
    <message>
        <source>Terminal</source>
        <translation>Terminal</translation>
    </message>
    <message>
        <source>Window:</source>
        <translation>Fönster:</translation>
    </message>
</context>
<context>
    <name>ChatDlg</name>
    <message>
        <source>No</source>
        <translation>Nej</translation>
    </message>
    <message>
        <source>Yes</source>
        <translation>Ja</translation>
    </message>
    <message>
        <source>Beep</source>
        <translation>Ljudsignal</translation>
    </message>
    <message>
        <source>Bold</source>
        <translation>Fetstil</translation>
    </message>
    <message>
        <source>Chat</source>
        <translation>Snack</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation>Läge</translation>
    </message>
    <message>
        <source>Toggles Bold font</source>
        <translation>Växlar fet stil</translation>
    </message>
    <message>
        <source>Toggles Italic font</source>
        <translation>Växlar kursiv stil</translation>
    </message>
    <message>
        <source>Remote - Waiting for joiners...</source>
        <translation>Ej lokal - väntar på deltagare...</translation>
    </message>
    <message>
        <source>Licq - Chat</source>
        <translation>Licq - chatt</translation>
    </message>
    <message>
        <source>&amp;Audio</source>
        <translation>&amp;Ljud</translation>
    </message>
    <message>
        <source>Sends a Beep to all recipients</source>
        <translation>Skickar en ljudsignal till alla mottagare</translation>
    </message>
    <message>
        <source>&amp;Pane Mode</source>
        <translation>&amp;Fönsterläge</translation>
    </message>
    <message>
        <source>Italic</source>
        <translation>Kursivstil</translation>
    </message>
    <message>
        <source>Do you want to save the chat session?</source>
        <translation>Vill du spara chattsessionen?</translation>
    </message>
    <message>
        <source>Licq - Chat %1</source>
        <translation>Licq - chatt %1</translation>
    </message>
    <message>
        <source>&amp;IRC Mode</source>
        <translation>&amp;IRC-Läge</translation>
    </message>
    <message>
        <source>Changes the foreground color</source>
        <translation>Ändrar förgrundsfärgen</translation>
    </message>
    <message>
        <source>Background color</source>
        <translation>Bakgrundsfärg</translation>
    </message>
    <message>
        <source>Local - %1</source>
        <translation>Lokal - %1</translation>
    </message>
    <message>
        <source>Remote - Not connected</source>
        <translation>Ej lokal - Ej ansluten</translation>
    </message>
    <message>
        <source>&amp;Close Chat</source>
        <translation>&amp;Stäng Snack</translation>
    </message>
    <message>
        <source>Underline</source>
        <translation>Understruket</translation>
    </message>
    <message>
        <source>%1 closed connection.</source>
        <translation>%1 avbröt anslutningen.</translation>
    </message>
    <message>
        <source>Remote - Connecting...</source>
        <translation>Ej lokal - ansluter...</translation>
    </message>
    <message>
        <source>&amp;Save Chat</source>
        <translation>&amp;Spara chatt</translation>
    </message>
    <message>
        <source>Changes the background color</source>
        <translation>Ändrar bakgrundsfärgen</translation>
    </message>
    <message>
        <source>Foreground color</source>
        <translation>Förgrundsfärg</translation>
    </message>
    <message>
        <source>Ignore user settings</source>
        <translation>Ignorera Användarinställningar</translation>
    </message>
    <message>
        <source>Ignores user color settings</source>
        <translation>Ignorera användarens färginställningar</translation>
    </message>
    <message>
        <source>Unable to bind to a port.
See Network Window for details.</source>
        <translation>Kunde inte binda till port.
Se nätverksfönstret för mer information.</translation>
    </message>
    <message>
        <source>Unable to connect to the remote chat.
See Network Window for details.</source>
        <translation>Kunde inte ansluta till chatsessionen.
Se nätverksfönstret för mer information.</translation>
    </message>
    <message>
        <source>Unable to create new thread.
See Network Window for details.</source>
        <translation>Kunde inte skapa ny tråd.
Se nätverksfönstret för mer information.</translation>
    </message>
    <message>
        <source>Failed to open file:
%1</source>
        <translation>Kunde inte öppna fil:
%1</translation>
    </message>
    <message>
        <source>Unable to load encoding &lt;b&gt;%1&lt;/b&gt;. Message contents may appear garbled.</source>
        <translation>Kunde inte ladda fontsystem &lt;b&gt;%1&lt;/b&gt;. Innehållet i meddelandet kan verka skadat.</translation>
    </message>
    <message>
        <source>StrikeOut</source>
        <translation>Överstruken</translation>
    </message>
    <message>
        <source>Toggles StrikeOut font</source>
        <translation>Slår av/på överstruket teckensnitt</translation>
    </message>
    <message>
        <source>Set Encoding</source>
        <translation>Ange fontsystem</translation>
    </message>
    <message>
        <source>
&lt;--BEEP--&gt;
</source>
        <translation>
&lt;--PIIP--&gt;
</translation>
    </message>
    <message>
        <source>&gt; &lt;--BEEP--&gt;
</source>
        <translation>&gt; &lt;--PIIP--&gt;
</translation>
    </message>
    <message>
        <source>/%1.chat</source>
        <translation>/%1.chat</translation>
    </message>
    <message>
        <source>Toggles Underline font</source>
        <translation>Växlar understryken font</translation>
    </message>
</context>
<context>
    <name>CustomAwayMsgDlg</name>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>&amp;Clear</source>
        <translation>&amp;Rensa</translation>
    </message>
    <message>
        <source>Set Custom Auto Response for %1</source>
        <translation>Ange anpassat autosvar för %1</translation>
    </message>
    <message>
        <source>I am currently %1.
You can leave me a message.</source>
        <translation>Jag är förnärvarande %1.
Du kan lämna ett meddelande.</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Avbryt</translation>
    </message>
    <message>
        <source>&amp;Hints</source>
        <translation>&amp;Tips</translation>
    </message>
</context>
<context>
    <name>EditCategoryDlg</name>
    <message>
        <source>Unspecified</source>
        <translation>Ospecificerat</translation>
    </message>
    <message>
        <source>&amp;OK</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Avbryt</translation>
    </message>
</context>
<context>
    <name>EditFileDlg</name>
    <message>
        <source>&amp;Save</source>
        <translation>&amp;Spara</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>S&amp;täng</translation>
    </message>
    <message>
        <source>Failed to open file:
%1</source>
        <translation>Kunde inte öppna fil:
%1</translation>
    </message>
    <message>
        <source>Licq File Editor - %1</source>
        <translation>Licq filredigerare - %1</translation>
    </message>
    <message>
        <source>[ Read-Only ]</source>
        <translation>[ Skrivskyddad ]</translation>
    </message>
</context>
<context>
    <name>EditGrpDlg</name>
    <message>
        <source>Ok</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Add</source>
        <translation>Lägg till</translation>
    </message>
    <message>
        <source>&amp;Done</source>
        <translation>&amp;Klar</translation>
    </message>
    <message>
        <source>Edit Name</source>
        <translation>Redigera namn</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Avbryt</translation>
    </message>
    <message>
        <source>Groups</source>
        <translation>Grupper</translation>
    </message>
    <message>
        <source>Are you sure you want to remove
the group &apos;%1&apos;?</source>
        <translation>Är du säker på att du vill ta bort
gruppen &apos;%1&apos;?</translation>
    </message>
    <message>
        <source>Remove</source>
        <translation>Ta bort</translation>
    </message>
    <message>
        <source>noname</source>
        <translation>namnlös</translation>
    </message>
    <message>
        <source>Edit group name (hit enter to save).</source>
        <translation>Redigera gruppnamn (slå retur för att spara).</translation>
    </message>
    <message>
        <source>Shift Down</source>
        <translation>Flytta ned</translation>
    </message>
    <message>
        <source>Licq - Edit Groups</source>
        <translation>Licq - redigera grupper</translation>
    </message>
    <message>
        <source>Shift Up</source>
        <translation>Flytta upp</translation>
    </message>
    <message>
        <source>&amp;Save</source>
        <translation>&amp;Spara</translation>
    </message>
    <message>
        <source>Save the name of a group being modified.</source>
        <translation>Spara namnet på den grupp som ändras.</translation>
    </message>
</context>
<context>
    <name>EditPhoneDlg</name>
    <message>
        <source>Description:</source>
        <translation>Beskrivning:</translation>
    </message>
    <message>
        <source>Home Phone</source>
        <translation>Hemtelefon</translation>
    </message>
    <message>
        <source>Work Phone</source>
        <translation>Arbetstelefon</translation>
    </message>
    <message>
        <source>Private Cellular</source>
        <translation>Mobil (privat)</translation>
    </message>
    <message>
        <source>Work Cellular</source>
        <translation>Mobil (arbete)</translation>
    </message>
    <message>
        <source>Home Fax</source>
        <translation>Fax (privat)</translation>
    </message>
    <message>
        <source>Work Fax</source>
        <translation>Fax (arbete)</translation>
    </message>
    <message>
        <source>Wireless Pager</source>
        <translation>Personsökare</translation>
    </message>
    <message>
        <source>Type:</source>
        <translation>Typ:</translation>
    </message>
    <message>
        <source>Phone</source>
        <translation>Telefon</translation>
    </message>
    <message>
        <source>Cellular</source>
        <translation>Mobiltelefon</translation>
    </message>
    <message>
        <source>Cellular SMS</source>
        <translation>SMS</translation>
    </message>
    <message>
        <source>Fax</source>
        <translation>Fax</translation>
    </message>
    <message>
        <source>Pager</source>
        <translation>Personsökare</translation>
    </message>
    <message>
        <source>Country:</source>
        <translation>Land:</translation>
    </message>
    <message>
        <source>Network #/Area code:</source>
        <translation>Riktnummer:</translation>
    </message>
    <message>
        <source>Number:</source>
        <translation>Nummer:</translation>
    </message>
    <message>
        <source>Extension:</source>
        <translation>Anknytning:</translation>
    </message>
    <message>
        <source>Provider:</source>
        <translation>Operatör:</translation>
    </message>
    <message>
        <source>Custom</source>
        <translation>Egendefinerad</translation>
    </message>
    <message>
        <source>E-mail Gateway:</source>
        <translation>E-post server:</translation>
    </message>
    <message>
        <source>Remove leading 0s from Area Code/Network #</source>
        <translation>Ange ej första 0:an i riktnummer</translation>
    </message>
    <message>
        <source>&amp;OK</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Avbryt</translation>
    </message>
    <message>
        <source>@</source>
        <translation>@</translation>
    </message>
    <message>
        <source>Please enter a phone number</source>
        <translation>Var snäll och skriv ett telefonnummer</translation>
    </message>
</context>
<context>
    <name>Event</name>
    <message>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <source>Message</source>
        <translation>Meddelande</translation>
    </message>
    <message>
        <source>Added to Contact List</source>
        <translation>Tillade till kontaktlistan</translation>
    </message>
    <message>
        <source>Web Panel</source>
        <translation>Webbpanel</translation>
    </message>
    <message>
        <source>Plugin Event</source>
        <translation>Tillägghändelse</translation>
    </message>
    <message>
        <source>File Transfer</source>
        <translation>Filöverföring</translation>
    </message>
    <message>
        <source>Authorization Request</source>
        <translation>Begäran om auktorisering</translation>
    </message>
    <message>
        <source>Authorization Refused</source>
        <translation>Auktorisering vägrades</translation>
    </message>
    <message>
        <source>Unknown Event</source>
        <translation>Okänd händelse</translation>
    </message>
    <message>
        <source>Chat Request</source>
        <translation>Chattbegäran</translation>
    </message>
    <message>
        <source>Authorization Granted</source>
        <translation>Auktorisering godkändes</translation>
    </message>
    <message>
        <source>Email Pager</source>
        <translation>E-postläsare</translation>
    </message>
    <message>
        <source>Contact List</source>
        <translation>Kontaktlista</translation>
    </message>
    <message>
        <source>SMS</source>
        <translation>SMS</translation>
    </message>
    <message>
        <source>(cancelled)</source>
        <translation>(avbruten)</translation>
    </message>
    <message>
        <source>System Server Message</source>
        <translation>Systemmeddelande från Servern</translation>
    </message>
    <message>
        <source></source>
        <translation></translation>
    </message>
</context>
<context>
    <name>GPGKeyManager</name>
    <message>
        <source>Licq GPG Key Manager</source>
        <translation>Licq GPG nyckelhanterare</translation>
    </message>
    <message>
        <source>GPG Passphrase</source>
        <translation>GPG-lösenordsfras</translation>
    </message>
    <message>
        <source>No passphrase set</source>
        <translation>Ingen lösenordsfras angiven</translation>
    </message>
    <message>
        <source>&amp;Set GPG Passphrase</source>
        <translation>&amp;Ange GPG-lösenordsfras</translation>
    </message>
    <message>
        <source>User Keys</source>
        <translation>Kontaktnycklar</translation>
    </message>
    <message>
        <source>&amp;Add</source>
        <translation>&amp;Lägg till</translation>
    </message>
    <message>
        <source>&amp;Edit</source>
        <translation>&amp;Redigera</translation>
    </message>
    <message>
        <source>&amp;Remove</source>
        <translation>&amp;Ta bort</translation>
    </message>
    <message>
        <source>&lt;qt&gt;Drag&amp;Drop user to add to list.&lt;/qt&gt;</source>
        <translation>&lt;qt&gt;Dra&amp;Släpp kontakter för att lägga till i listan.&lt;/qt&gt;</translation>
    </message>
    <message>
        <source>User</source>
        <translation>Kontakt</translation>
    </message>
    <message>
        <source>Active</source>
        <translation>Aktiv</translation>
    </message>
    <message>
        <source>Key ID</source>
        <translation>Nyckelid</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Stäng</translation>
    </message>
    <message>
        <source>Set Passphrase</source>
        <translation>Ange lösenordsfras</translation>
    </message>
    <message>
        <source>Not yet implemented. Use licq_gpg.conf.</source>
        <translation>Inte implementerat ännu. Använd licq_gpg.conf.</translation>
    </message>
    <message>
        <source>Remove GPG key</source>
        <translation>Ta bort GPG-nyckel</translation>
    </message>
    <message>
        <source>Do you want to remove the GPG key? The key isn&apos;t deleted from your keyring.</source>
        <translation>Vill du ta bort GPG-nykeln? Nyckeln tas inte bort från din nyckelring.</translation>
    </message>
</context>
<context>
    <name>GPGKeySelect</name>
    <message>
        <source>Select GPG Key for user %1</source>
        <translation>Välj GPGnyckel för användare %1</translation>
    </message>
    <message>
        <source>Select a GPG key for user %1.</source>
        <translation>Välj en GPGnyckel för användare %1.</translation>
    </message>
    <message>
        <source>Current key: No key selected</source>
        <translation>Nuvarande nyckel: Ingen nyckel vald</translation>
    </message>
    <message>
        <source>Current key: %1</source>
        <translation>Nuvarande nyckel: %1</translation>
    </message>
    <message>
        <source>Use GPG Encryption</source>
        <translation>Använd GPG-kryptering</translation>
    </message>
    <message>
        <source>Filter:</source>
        <translation>Filter:</translation>
    </message>
    <message>
        <source>&amp;OK</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>&amp;No Key</source>
        <translation>&amp;Ingen nyckel</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Avbryt</translation>
    </message>
</context>
<context>
    <name>Groups</name>
    <message>
        <source>All Users</source>
        <translation>Alla användare</translation>
    </message>
    <message>
        <source>Online Notify</source>
        <translation>Anslutningsavisering</translation>
    </message>
    <message>
        <source>Visible List</source>
        <translation>Synlighetslista</translation>
    </message>
    <message>
        <source>Invisible List</source>
        <translation>Osynlighetslista</translation>
    </message>
    <message>
        <source>Ignore List</source>
        <translation>Ignoreringslista</translation>
    </message>
    <message>
        <source>New Users</source>
        <translation>Nya användare</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation>Okänd</translation>
    </message>
</context>
<context>
    <name>HintsDlg</name>
    <message>
        <source>&amp;Close</source>
        <translation>S&amp;täng</translation>
    </message>
    <message>
        <source>Licq - Hints</source>
        <translation>Licq - tips</translation>
    </message>
</context>
<context>
    <name>IconManager_KDEStyle</name>
    <message>
        <source>&lt;br&gt;&lt;b&gt;%1 system messages&lt;/b&gt;</source>
        <translation>&lt;br&gt;&lt;b&gt;%1 systemmeddelande&lt;/b&gt;</translation>
    </message>
    <message>
        <source>&lt;br&gt;%1 msgs</source>
        <translation>&lt;br&gt;%1 med.</translation>
    </message>
    <message>
        <source>&lt;br&gt;1 msg</source>
        <translation>&lt;br&gt;1 med.</translation>
    </message>
    <message>
        <source>&lt;br&gt;Left click - Show main window&lt;br&gt;Middle click - Show next message&lt;br&gt;Right click - System menu</source>
        <translation>&lt;br&gt;Vänsterklicka - Visa huvudfönstret&lt;br&gt;Mittenklicka - Visa nästa meddelande&lt;br&gt;Högerklicka - System meny</translation>
    </message>
</context>
<context>
    <name>IconManager_Themed</name>
    <message>
        <source>Unable to load dock theme image
%1</source>
        <translation>Kunde inte ladda temabilden för docken
%1</translation>
    </message>
    <message>
        <source>Unable to load dock theme file
(%1)
:%2</source>
        <translation>Kunde inte ladda temafilen för docken
(%1)
:%2</translation>
    </message>
</context>
<context>
    <name>KeyListItem</name>
    <message>
        <source>Yes</source>
        <translation>Ja</translation>
    </message>
    <message>
        <source>No</source>
        <translation>Nej</translation>
    </message>
</context>
<context>
    <name>KeyRequestDlg</name>
    <message>
        <source>&amp;Send</source>
        <translation>&amp;Skicka</translation>
    </message>
    <message>
        <source>Closing secure channel...</source>
        <translation>Stänger säker kanal...</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>S&amp;täng</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;ForestGreen&quot;&gt;Secure channel established.&lt;/font&gt;
</source>
        <translation>&lt;font color=&quot;ForestGreen&quot;&gt;Säker kanal upprättad.&lt;/font&gt;
</translation>
    </message>
    <message>
        <source>Client does not support OpenSSL.
Rebuild Licq with OpenSSL support.</source>
        <translation>Klienten stöder inte OpenSSL.
Återbygg Licq med OpenSSL-stöd.</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;blue&quot;&gt;Secure channel closed.&lt;/font&gt;
</source>
        <translation>&lt;font color=&quot;blue&quot;&gt;Säker kanal stängd.&lt;/font&gt;
</translation>
    </message>
    <message>
        <source>Licq - Secure Channel with %1</source>
        <translation>Licq - Säker kanal med %1</translation>
    </message>
    <message>
        <source>Secure channel is established using SSL
with Diffie-Hellman key exchange and
the TLS version 1 protocol.

</source>
        <translation>Säker kanal är upprättad med SSL
med Diffie-Hellman nyckelväxling och
TLS version 1 protokollet.

</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;Could not connect to remote client.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;Kunde inte ansluta till fjärrklient.&lt;/font&gt;</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;yellow&quot;&gt;Secure channel already established.&lt;/font&gt;
</source>
        <translation>&lt;font color=&quot;yellow&quot;&gt;Säker kanal redan upprättad.&lt;/font&gt;
</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;yellow&quot;&gt;Secure channel not established.&lt;/font&gt;
</source>
        <translation>&lt;font color=&quot;yellow&quot;&gt;Säker kanal ej upprättad.&lt;/font&gt;
</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;Remote client does not support OpenSSL.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;Fjärrklient stöder inte OpenSSL.&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Requesting secure channel...</source>
        <translation>Begär säker kanal...</translation>
    </message>
    <message>
        <source>This only works with other Licq clients &gt;= v0.85
The remote doesn&apos;t seem to use such a client.
This might not work.</source>
        <translation>Detta fungerar endast med andra Licq klienter &gt;= v0.85
Fjärrklienten ser inte ut att vara en sådan klient.
Detta kommer kanske inte att fungera.</translation>
    </message>
    <message>
        <source>The remote uses Licq %1/SSL.</source>
        <translation>Mottagaren använder Licq %1/SSL.</translation>
    </message>
    <message>
        <source>The remote uses Licq %1, however it
has no secure channel support compiled in.
This probably won&apos;t work.</source>
        <translation>Mottagaren använder Licq %1, men klienten har inte
stöd för &quot;säker kanal&quot; inkompilerat.
Det här kommer troligen inte att fungera.</translation>
    </message>
    <message>
        <source>Ready to close channel</source>
        <translation>Klar att stänga kanal</translation>
    </message>
    <message>
        <source>Ready to request channel</source>
        <translation>Klar att begära kanal</translation>
    </message>
</context>
<context>
    <name>KeyView</name>
    <message>
        <source>Name</source>
        <translation>Namn</translation>
    </message>
</context>
<context>
    <name>MLEditWrap</name>
    <message>
        <source>Allow Tabulations</source>
        <translation>Tillåt tabbuleringar</translation>
    </message>
</context>
<context>
    <name>MLView</name>
    <message>
        <source>Copy URL</source>
        <translation>Kopiera URL</translation>
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
        <translation>Tid</translation>
    </message>
    <message>
        <source>Cancelled Event</source>
        <translation>Avbruten händelse</translation>
    </message>
    <message>
        <source>Direct</source>
        <translation>Direkt</translation>
    </message>
    <message>
        <source>Event Type</source>
        <translation>Händelsetyp</translation>
    </message>
    <message>
        <source>Server</source>
        <translation>Server</translation>
    </message>
    <message>
        <source>Urgent</source>
        <translation>Brådskande</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>Alternativ</translation>
    </message>
    <message>
        <source>Multiple Recipients</source>
        <translation>Flera mottagare</translation>
    </message>
</context>
<context>
    <name>OptionsDlg</name>
    <message>
        <source><byte value="x9"/>to</source>
        <translation>Till:</translation>
    </message>
    <message>
        <source>&amp;OK</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>Auto</source>
        <translation>Auto</translation>
    </message>
    <message>
        <source>Away</source>
        <translation>Borta</translation>
    </message>
    <message>
        <source>Font</source>
        <translation>Teckensnitt</translation>
    </message>
    <message>
        <source>Left</source>
        <translation>Vänster</translation>
    </message>
    <message>
        <source>Save</source>
        <translation>Spara</translation>
    </message>
    <message>
        <source>URL:</source>
        <translation>URL:</translation>
    </message>
    <message>
        <source>none</source>
        <translation>ingen</translation>
    </message>
    <message>
        <source>Show Extended Icons</source>
        <translation>Visa utökade ikoner</translation>
    </message>
    <message>
        <source>Font:</source>
        <translation>Teckensnitt:</translation>
    </message>
    <message>
        <source>Never</source>
        <translation>Aldrig</translation>
    </message>
    <message>
        <source>Right</source>
        <translation>Höger</translation>
    </message>
    <message>
        <source>Text:</source>
        <translation>Text:</translation>
    </message>
    <message>
        <source>Title</source>
        <translation>Titel</translation>
    </message>
    <message>
        <source>Width</source>
        <translation>Bredd</translation>
    </message>
    <message>
        <source>Parameter for received file transfers</source>
        <translation>Parameter för mottagna filöverföringar</translation>
    </message>
    <message>
        <source>Firewall</source>
        <translation>Brandvägg</translation>
    </message>
    <message>
        <source>Show Grid Lines</source>
        <translation>Visa rutnät</translation>
    </message>
    <message>
        <source>Override the skin setting for the frame style of the user window:
   0 (No frame), 1 (Box), 2 (Panel), 3 (WinPanel)
 + 16 (Plain), 32 (Raised), 48 (Sunken)
 + 240 (Shadow)</source>
        <translation>Åsidosätt skinninställningarna för användarfönstrets kantstil:
   0 (Ingen kant), 1 (Box), 2 (Panel), 3 (FönsterPanel)
 + 16 (Enkel), 32 (Upphöjd), 48 (Nedsänkt)
 + 240 (Skugga)</translation>
    </message>
    <message>
        <source>Parameter for received chat requests</source>
        <translation>Parameter för mottagna chattbegäran</translation>
    </message>
    <message>
        <source>Turns on or off the display of headers above each column in the user list</source>
        <translation>Växlar visandet av rubriker ovanför kolumnerna i användarlistan</translation>
    </message>
    <message>
        <source>Auto Away:</source>
        <translation>Sätt automatiskt &quot;Borta&quot; efter:</translation>
    </message>
    <message>
        <source>Number of minutes of inactivity after which to automatically be marked &quot;not available&quot;.  Set to &quot;0&quot; to disable.</source>
        <translation>Antal minuter av inaktivitet innan status sätts till &quot;Ej tillgänglig&quot;. Ange &quot;0&quot; för att inaktivera.</translation>
    </message>
    <message>
        <source>Ignore Mass Messages</source>
        <translation>Ignorera massmeddelanden</translation>
    </message>
    <message>
        <source>Show the &quot;--online--&quot; and &quot;--offline--&quot; bars in the contact list</source>
        <translation>Visa &quot;--ansluten--&quot; och &quot;--frånkopplad--&quot; avgränsarna i kontaktlistan</translation>
    </message>
    <message>
        <source>Parameter for online notification</source>
        <translation>Parameter för anslutningsavisering</translation>
    </message>
    <message>
        <source>TCP port range for incoming connections.</source>
        <translation>TCP portområde för inkommande anslutningar.</translation>
    </message>
    <message>
        <source>Accept Modes</source>
        <translation>Accepteringslägen</translation>
    </message>
    <message>
        <source>Online Notify:</source>
        <translation>Anslutningsavisering:</translation>
    </message>
    <message>
        <source>Not Available</source>
        <translation>Ej tillgänglig</translation>
    </message>
    <message>
        <source>&amp;Apply</source>
        <translation>&amp;Använd</translation>
    </message>
    <message>
        <source>Determines if new users are automatically added to your list or must first request authorization.</source>
        <translation>Bestämmer om nya användare automatiskt skall läggas till din kontaktlista, eller om de först måste auktoriseras.</translation>
    </message>
    <message>
        <source>Show the name of the current group in the messages label when there are no new messages</source>
        <translation>Visa namnet på den aktuella gruppen i meddelande-etiketten när det inte finns några nya meddelanden</translation>
    </message>
    <message>
        <source>Ignore Web Panel</source>
        <translation>Ignorera webbpanelen</translation>
    </message>
    <message>
        <source>Use Font Styles</source>
        <translation>Använd teckensnitsstilar</translation>
    </message>
    <message>
        <source>Preset slot:</source>
        <translation>Förutbestämt fack:</translation>
    </message>
    <message>
        <source>Licq Options</source>
        <translation>Licq-alternativ</translation>
    </message>
    <message>
        <source>Show online notify users who are offline even when offline users are hidden.</source>
        <translation>Visa frånkopplade &quot;online notify&quot; användare även om frånkopplade användare är gömda</translation>
    </message>
    <message>
        <source>Show Column Headers</source>
        <translation>Visa kolumnrubriker</translation>
    </message>
    <message>
        <source>Bold Message Label on Incoming Msg</source>
        <translation>Fet stil på etiketter vid inkommande med.</translation>
    </message>
    <message>
        <source>Column %1</source>
        <translation>Kolumn %1</translation>
    </message>
    <message>
        <source>Selects between the standard 64x64 icon used in the WindowMaker/Afterstep wharf and a shorter 64x48 icon for use in the Gnome/KDE panel.</source>
        <translation>Väljer mellan 64x64 standardikonen använd av Window Maker/AfterStep och den mindre 64x48 ikonen använd av GNOME-/KDE-panelen.</translation>
    </message>
    <message>
        <source>Perform OnEvent command in occupied mode</source>
        <translation>Utför VidHändelse-kommando i Upptagenläget</translation>
    </message>
    <message>
        <source>Auto Offline:</source>
        <translation>Frånkopplad efter: </translation>
    </message>
    <message>
        <source>OnEvent in DND</source>
        <translation>VidHändelse i &quot;Stör ej&quot;</translation>
    </message>
    <message>
        <source>OnEvent in N/A</source>
        <translation>VidHändelse i &quot;Ej tillgänglig&quot;</translation>
    </message>
    <message>
        <source>Parameter for received messages</source>
        <translation>Parameter för mottagna meddelanden</translation>
    </message>
    <message>
        <source>Center</source>
        <translation>Centrera</translation>
    </message>
    <message>
        <source>Allow scroll bar</source>
        <translation>Tillåt rullningslist</translation>
    </message>
    <message>
        <source>OnEvent in Away</source>
        <translation>VidHändelse i &quot;Borta&quot;</translation>
    </message>
    <message>
        <source>Network</source>
        <translation>Nätverk</translation>
    </message>
    <message>
        <source>Format</source>
        <translation>Format</translation>
    </message>
    <message>
        <source>The fonts used</source>
        <translation>De använda typsnitten</translation>
    </message>
    <message>
        <source>Number of minutes of inactivity after which to automatically go offline.  Set to &quot;0&quot; to disable.</source>
        <translation>Antal minuter av inaktivitet innan status sätts till &quot;frånkopplad&quot;. Ange &quot;0&quot; för att inaktivera.</translation>
    </message>
    <message>
        <source>Use Dock Icon</source>
        <translation>Använd dockikon</translation>
    </message>
    <message>
        <source>Chat Request:</source>
        <translation>Chattbegäran:</translation>
    </message>
    <message>
        <source>Online</source>
        <translation>Ansluten</translation>
    </message>
    <message>
        <source>Server settings</source>
        <translation>Serverinställningar</translation>
    </message>
    <message>
        <source>Auto Logon:</source>
        <translation>Anslut automatiskt som:</translation>
    </message>
    <message>
        <source>Main Window</source>
        <translation>Huvudfönstret</translation>
    </message>
    <message>
        <source>Status</source>
        <translation>Status</translation>
    </message>
    <message>
        <source>Theme:</source>
        <translation>Tema</translation>
    </message>
    <message>
        <source>Always show online notify users</source>
        <translation>Visa alltid anslutningsaviserade användare</translation>
    </message>
    <message>
        <source>default (%1)</source>
        <translation>Förvalt värde (%1)</translation>
    </message>
    <message>
        <source>Offline</source>
        <translation>Frånkopplad</translation>
    </message>
    <message>
        <source>Port Range:</source>
        <translation>Portområde:</translation>
    </message>
    <message>
        <source>OnEvent</source>
        <translation>VidHändelse</translation>
    </message>
    <message>
        <source>Command:</source>
        <translation>Kommando:</translation>
    </message>
    <message>
        <source>Edit Font:</source>
        <translation>Redigera teckensnitt:</translation>
    </message>
    <message>
        <source>I can receive direct connections</source>
        <translation>Jag kan mottaga direkta förbindelser</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>Alternativ</translation>
    </message>
    <message>
        <source>Miscellaneous</source>
        <translation>Blandat</translation>
    </message>
    <message>
        <source>Perform OnEvent command in do not disturb mode</source>
        <translation>Utför VidHändelse-kommando i Stör-ej-läget</translation>
    </message>
    <message>
        <source>Determines if mass messages are ignored or not.</source>
        <translation>Bestämmer om massmeddelanden ignoreras eller ej.</translation>
    </message>
    <message>
        <source>Frame Style: </source>
        <translation>Kantstil: </translation>
    </message>
    <message>
        <source>The alignment of the column</source>
        <translation>Justeringen av kolumnen</translation>
    </message>
    <message>
        <source>Make the user window transparent when there is no scroll bar</source>
        <translation>Gör användarfönstret genomskinligt när det inte är någon rullningslist</translation>
    </message>
    <message>
        <source>Default Icon</source>
        <translation>Förvald ikon</translation>
    </message>
    <message>
        <source>Parameters</source>
        <translation>Paramatrar</translation>
    </message>
    <message>
        <source>Show birthday, invisible, and custom auto response icons to the right of users in the list</source>
        <translation>Visa födelsedag, osynlighet, och anpassade autosvar ikoner till höger om användarna i listan</translation>
    </message>
    <message>
        <source>Transparent when possible</source>
        <translation>Genomskinlig om möjligt</translation>
    </message>
    <message>
        <source>Default Auto Response Messages</source>
        <translation>Förvalt autosvar</translation>
    </message>
    <message>
        <source>File Transfer:</source>
        <translation>Filöverföring:</translation>
    </message>
    <message>
        <source>OnEvents Enabled</source>
        <translation>VidHändelser aktiverade</translation>
    </message>
    <message>
        <source>Paranoia</source>
        <translation>Paranoia</translation>
    </message>
    <message>
        <source>Auto-Raise on Incoming Msg</source>
        <translation>Autohöj vid inkommande med.</translation>
    </message>
    <message>
        <source>Terminal:</source>
        <translation>Terminal</translation>
    </message>
    <message>
        <source>Automatically log on when first starting up.</source>
        <translation>Anslut automatiskt vid uppstart.</translation>
    </message>
    <message>
        <source>Number of minutes of inactivity after which to automatically be marked &quot;away&quot;.  Set to &quot;0&quot; to disable.</source>
        <translation>Antal minuter av inaktivitet innan status sätts till &quot;Borta&quot;. Ange &quot;0&quot; för att inaktivera.</translation>
    </message>
    <message>
        <source>&lt;p&gt;Command to execute when an event is received.&lt;br&gt;It will be passed the relevant parameters from below.&lt;br&gt;Parameters can contain the following expressions &lt;br&gt; which will be replaced with the relevant information:&lt;/p&gt;</source>
        <translation>&lt;p&gt;Kommando som skall köras när en händelse mottages.&lt;br&gt;
Det kommer att skickas releventa parametrar från nedan.&lt;br&gt;
Parametrar kan innehålla följande uttryck &lt;br&gt; som ersätts med relevant information:&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Determines if email pager messages are ignored or not.</source>
        <translation>Bestämmer om e-postmeddelanden ignoreras eller ej.</translation>
    </message>
    <message>
        <source>Perform OnEvent command in not available mode</source>
        <translation>Utför VidHändelse-kommando i Ej-tillgänglig-läget</translation>
    </message>
    <message>
        <source>Occupied</source>
        <translation>Upptagen</translation>
    </message>
    <message>
        <source>Show User Dividers</source>
        <translation>Visa användaravgränsare</translation>
    </message>
    <message>
        <source>OnEvent in Occupied</source>
        <translation>VidHändelse i &quot;Upptagen&quot;</translation>
    </message>
    <message>
        <source>Parameter for received system messages</source>
        <translation>Parameter för mottagna systemmeddelanden</translation>
    </message>
    <message>
        <source>Manual &quot;New User&quot; group handling</source>
        <translation>Manuell hantering av gruppen &quot;Nya användare&quot;</translation>
    </message>
    <message>
        <source>Select Font</source>
        <translation>Välj teckensnitt</translation>
    </message>
    <message>
        <source>Auto-Popup Incoming Msg</source>
        <translation>Visa inkommande med. automatiskt.</translation>
    </message>
    <message>
        <source>64 x 48 Dock Icon</source>
        <translation>64 x 48 dockikon</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Avbryt</translation>
    </message>
    <message>
        <source>Startup</source>
        <translation>Uppstart</translation>
    </message>
    <message>
        <source>Status:</source>
        <translation>Status:</translation>
    </message>
    <message>
        <source>default</source>
        <translation>Förvalt värde</translation>
    </message>
    <message>
        <source>Use italics and bold in the user list to indicate special characteristics such as online notify and visible list</source>
        <translation>Använd kursiv- och fetstil i användarlistan för att indikera speciella egenskaper, såsom aviserings- och synlighetslista</translation>
    </message>
    <message>
        <source>Auto N/A:</source>
        <translation>Ej tillgänglig efter:</translation>
    </message>
    <message>
        <source>Themed Icon</source>
        <translation>Ikon i temat</translation>
    </message>
    <message>
        <source>Auto Close Function Window</source>
        <translation>Autostäng funktionsfönstret</translation>
    </message>
    <message>
        <source>Perform OnEvent command in away mode</source>
        <translation>Utför VidHändelse-kommando i frånvaroläget</translation>
    </message>
    <message>
        <source>Docking</source>
        <translation>Dockning</translation>
    </message>
    <message>
        <source>Determines if web panel messages are ignored or not.</source>
        <translation>Besämmer om webbpanelmeddelanden ignoreras eller ej.</translation>
    </message>
    <message>
        <source>Enable running of &quot;Command&quot; when the relevant event occurs.</source>
        <translation>Aktivera körning av &quot;Kommando&quot; när relevant händelse inträffar.</translation>
    </message>
    <message>
        <source>System Msg:</source>
        <translation>Systemmeddelande:</translation>
    </message>
    <message>
        <source>Show group name if no messages</source>
        <translation>Visa gruppnamn om meddelanden saknas</translation>
    </message>
    <message>
        <source>Free for Chat</source>
        <translation>Chattledig</translation>
    </message>
    <message>
        <source>Free For Chat</source>
        <translation>Chattledig</translation>
    </message>
    <message>
        <source>Online Notify when Logging On</source>
        <translation>Anslutningsavisering under anslutning</translation>
    </message>
    <message>
        <source>Extensions</source>
        <translation>Extensioner</translation>
    </message>
    <message>
        <source>Do Not Disturb</source>
        <translation>Stör ej</translation>
    </message>
    <message>
        <source>Url Viewer:</source>
        <translation>URL visare:</translation>
    </message>
    <message>
        <source>Message:</source>
        <translation>Meddelande:</translation>
    </message>
    <message>
        <source>Draw boxes around each square in the user list</source>
        <translation>Rita rutor kring varje kvadrat i användarlistan</translation>
    </message>
    <message>
        <source>Column Configuration</source>
        <translation>Kolumnkonfiguration</translation>
    </message>
    <message>
        <source>Invisible</source>
        <translation>Osynlig</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Allmänt</translation>
    </message>
    <message>
        <source>Ignore Email Pager</source>
        <translation>Ignorera e-postmeddelanden</translation>
    </message>
    <message>
        <source>Controls whether or not the dockable icon should be displayed.</source>
        <translation>Styr huruvida den dockningsbara ikonen skall visas eller ej.</translation>
    </message>
    <message>
        <source>Contact List</source>
        <translation>Kontaktlista</translation>
    </message>
    <message>
        <source>The width of the column</source>
        <translation>Kolumnbredden</translation>
    </message>
    <message>
        <source>Select a font from the system list</source>
        <translation>Välj ett teckensnitt från systemlistan</translation>
    </message>
    <message>
        <source>The string which will appear in the list box column header</source>
        <translation>Texten som skall visas i listrutans kolumnrubrik</translation>
    </message>
    <message>
        <source>Parameter for received URLs</source>
        <translation>Parameter för mottagna URL:er</translation>
    </message>
    <message>
        <source>The command to run to view a URL.  Will be passed the URL as a parameter.</source>
        <translation>Kommandot för att visa en URL. URL:en ges som parameter.</translation>
    </message>
    <message>
        <source>Allow the vertical scroll bar in the user list</source>
        <translation>Tillåt vertikal rullningslist i användarlistan</translation>
    </message>
    <message>
        <source>The command to run to start your terminal program.</source>
        <translation>Kommandot för att starta ditt terminalprogram</translation>
    </message>
    <message>
        <source>Font used in message editor etc.</source>
        <translation>Teckensnitt använt i meddelanderedigerare etc.</translation>
    </message>
    <message>
        <source>Ignore New Users</source>
        <translation>Ignorera nya användare</translation>
    </message>
    <message>
        <source>Alignment</source>
        <translation>Justering</translation>
    </message>
    <message>
        <source>Auto close the user function window after a successful event</source>
        <translation>Stäng automatiskt fönstret för användarfunktioner efter en lyckad händelse</translation>
    </message>
    <message>
        <source>Open all incoming messages automatically when received if we are online (or free for chat)</source>
        <translation>Öppna alla inkommande meddelanden automatiskt, när de kommer, om jag är online (eller chatledig)</translation>
    </message>
    <message>
        <source>Raise the main window on incoming messages</source>
        <translation>Visa huvudfönstret vid inkommande meddelanden</translation>
    </message>
    <message>
        <source>Start Hidden</source>
        <translation>Starta gömd</translation>
    </message>
    <message>
        <source>Start main window hidden. Only the dock icon will be visible.</source>
        <translation>Starta med huvudfönstret gömt. Endast docken kommer synas.</translation>
    </message>
    <message>
        <source>Show the message info label in bold font if there are incoming messages</source>
        <translation>Visa medelande ettiketten i fetstil när det finns inkommande meddelanden</translation>
    </message>
    <message>
        <source>If not checked, a user will be automatically removed from &quot;New User&quot; group when you first send an event to them</source>
        <translation>Om ej ikryssad kommer användare att automatiskt tas bort ur gruppen &quot;Nya användare&quot; när du skickar något till dem.</translation>
    </message>
    <message>
        <source>Check Clipboard For Urls/Files</source>
        <translation>Kontrollera urklippshanteraren om där finns länkar/filer</translation>
    </message>
    <message>
        <source>When double-clicking on a user to send a message check for urls/files in the clipboard</source>
        <translation>Vid dubbelklickning på en användare för att skicka ett meddelande, kontrollera om urklippshanteraren innehåller länkare/filer.</translation>
    </message>
    <message>
        <source>Auto Position the Reply Window</source>
        <translation>Positionera svarsfönstret automatiskt</translation>
    </message>
    <message>
        <source>Position a new reply window just underneath the message view window</source>
        <translation>Positionera ett nytt svarsfönster strax under meddelandefönstret</translation>
    </message>
    <message>
        <source>Auto send through server</source>
        <translation>Skicka alltid via server</translation>
    </message>
    <message>
        <source>Automatically send messages through the server if direct connection fails</source>
        <translation>Skicka meddelanden via servern om en direktanslutning ej kan upprättas</translation>
    </message>
    <message>
        <source>Allow dragging main window</source>
        <translation>Tillåt flytt av huvudfönstret</translation>
    </message>
    <message>
        <source>Lets you drag around the main window with your mouse</source>
        <translation>Låter dig flytta huvudfönstret med muspekaren</translation>
    </message>
    <message>
        <source>Chatmode Messageview</source>
        <translation>Meddelandevisning i chatläge</translation>
    </message>
    <message>
        <source>Show the current chat history in Send Window</source>
        <translation>Visa den aktuella chathistoriken i skickafönstret</translation>
    </message>
    <message>
        <source>Localization</source>
        <translation>Lokalisering</translation>
    </message>
    <message>
        <source>Default Encoding:</source>
        <translation>Standard fontsystem:</translation>
    </message>
    <message>
        <source>Sets which default encoding should be used for newly added contacts.</source>
        <translation>Anger vilket fontsystem som skall anges som standard för nya kontakter.</translation>
    </message>
    <message>
        <source>System default (%1)</source>
        <translation>System standard (%1)</translation>
    </message>
    <message>
        <source>Show all encodings</source>
        <translation>Visa alla fontsystem</translation>
    </message>
    <message>
        <source>Show all available encodings in the User Encoding selection menu. Normally, this menu shows only commonly used encodings.</source>
        <translation>Visa alla tillgängliga fontsystem i menyn för val av fontsystem för användare. Normalt visar menyn de vanligaste fontsystemen.</translation>
    </message>
    <message>
        <source>Message Sent:</source>
        <translation>Meddelande skickat:</translation>
    </message>
    <message>
        <source>Parameter for sent messages</source>
        <translation>Parametrar för skickade meddelanden</translation>
    </message>
    <message>
        <source>Perform the online notify OnEvent when logging on (this is different from how the Mirabilis client works)</source>
        <translation>Utför anslutnings notifiering OnEvent vid inloggning (detta är olikt det sätt Mirabilis klient arbetar)</translation>
    </message>
    <message>
        <source>ICQ Server:</source>
        <translation>ICQ server:</translation>
    </message>
    <message>
        <source>ICQ Server Port:</source>
        <translation>ICQ serverns port:</translation>
    </message>
    <message>
        <source>Proxy</source>
        <translation>Proxy</translation>
    </message>
    <message>
        <source>Use proxy server</source>
        <translation>Använd proxy server</translation>
    </message>
    <message>
        <source>Proxy Type:</source>
        <translation>Typ av proxy:</translation>
    </message>
    <message>
        <source>HTTPS</source>
        <translation>HTTPS</translation>
    </message>
    <message>
        <source>Proxy Server:</source>
        <translation>Proxyserver:</translation>
    </message>
    <message>
        <source>Proxy Server Port:</source>
        <translation>Proxyserverns port:</translation>
    </message>
    <message>
        <source>Use authorization</source>
        <translation>Använd autentsiering</translation>
    </message>
    <message>
        <source>Username:</source>
        <translation>Användarnamn:</translation>
    </message>
    <message>
        <source>Password:</source>
        <translation>Lösenord:</translation>
    </message>
    <message>
        <source>Use server side contact list</source>
        <translation>Använd kontaktlista på servern</translation>
    </message>
    <message>
        <source>Store your contacts on the server so they are accessible from different locations and/or programs</source>
        <translation>Sparar dina kontakter på servern så att de är tillgängliga från flera platser och/eller program</translation>
    </message>
    <message>
        <source>Blink All Events</source>
        <translation>Låt alla händelser blinka</translation>
    </message>
    <message>
        <source>All incoming events will blink</source>
        <translation>Låt alla inkommande händelser blinka</translation>
    </message>
    <message>
        <source>Blink Urgent Events</source>
        <translation>Låt alla brådskande händelser blinka</translation>
    </message>
    <message>
        <source>Only urgent events will blink</source>
        <translation>Låt bara brådskande händelser blinka</translation>
    </message>
    <message>
        <source>Use System Background Color</source>
        <translation>Använd systemets bakgrundsfärg</translation>
    </message>
    <message>
        <source>Hot key: </source>
        <translation>Snabbtangent:</translation>
    </message>
    <message>
        <source>Hotkey pops up the next pending message
Enter the hotkey literally, like &quot;shift+f10&quot;, &quot;none&quot; for disabling
changes here require a Restart to take effect!
</source>
        <translation>Snabbtangenten öppnar nästa väntande meddelande
Skriv snabbtangenten bokstavligt, ex &quot;shift+f10&quot; eller &quot;none&quot; för att stänga av
förändringar här kräver en omstart av programmet innan de får effekt!</translation>
    </message>
    <message>
        <source>Popup info</source>
        <translation>Popup info</translation>
    </message>
    <message>
        <source>Email</source>
        <translation>E-post</translation>
    </message>
    <message>
        <source>Phone</source>
        <translation>Telefon</translation>
    </message>
    <message>
        <source>Fax</source>
        <translation>Fax</translation>
    </message>
    <message>
        <source>Cellular</source>
        <translation>Mobiltelefon</translation>
    </message>
    <message>
        <source>IP</source>
        <translation>IP</translation>
    </message>
    <message>
        <source>Last online</source>
        <translation>Senast ansluten</translation>
    </message>
    <message>
        <source>Online Time</source>
        <translation>Anslöt</translation>
    </message>
    <message>
        <source>Idle Time</source>
        <translation>Tid inaktiv</translation>
    </message>
    <message>
        <source>Auto Away Messages</source>
        <translation>Automatiskt bortameddelande</translation>
    </message>
    <message>
        <source>Away:</source>
        <translation>Borta:</translation>
    </message>
    <message>
        <source>N/A:</source>
        <translation>Ej tillgänglig:</translation>
    </message>
    <message>
        <source>Previous Message</source>
        <translation>Tidigare meddelande</translation>
    </message>
    <message>
        <source>I am behind a firewall</source>
        <translation>Jag är bakom en brandvägg</translation>
    </message>
    <message>
        <source>Tabbed Chatting</source>
        <translation>Chatfönster med flikar</translation>
    </message>
    <message>
        <source>Use tabs in Send Window</source>
        <translation>Använd flikar i skicka fönstret</translation>
    </message>
    <message>
        <source>Additional &amp;sorting:</source>
        <translation>Utökad &amp;sortering:</translation>
    </message>
    <message>
        <source>status</source>
        <translation>status</translation>
    </message>
    <message>
        <source>status + last event</source>
        <translation>status + senaste händelse</translation>
    </message>
    <message>
        <source>status + new messages</source>
        <translation>status + nya meddelanden</translation>
    </message>
    <message>
        <source>&lt;b&gt;none:&lt;/b&gt; - Don&apos;t sort online users by Status&lt;br&gt;
&lt;b&gt;status&lt;/b&gt; - Sort online users by status&lt;br&gt;
&lt;b&gt;status + last event&lt;/b&gt; - Sort online users by status and by last event&lt;br&gt;
&lt;b&gt;status + new messages&lt;/b&gt; - Sort online users by status and number of new messages</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Message Display</source>
        <translation>Meddelanderuta</translation>
    </message>
    <message>
        <source>Flash Taskbar on Incoming Msg</source>
        <translation>Blinka aktivitetsfältet vid inkommande meddelande</translation>
    </message>
    <message>
        <source>Flash the Taskbar on incoming messages</source>
        <translation>Blinka aktivitetsfältet vid inkommande meddelanden</translation>
    </message>
    <message>
        <source>Show recent messages</source>
        <translation>Visa senaste meddelandena</translation>
    </message>
    <message>
        <source>Show the last 5 messages when a Send Window is opened</source>
        <translation>Visa senaster 5 meddelandena när Skicka fönstret öppnas</translation>
    </message>
    <message>
        <source>Small Icon</source>
        <translation>Liten Ikon</translation>
    </message>
    <message>
        <source>Uses the freedesktop.org standard to dock a small icon into the window manager.  Works with many different window managers.</source>
        <translation>Använder freedesktop.org standarden för att docka en liten ikon i fönsterahanteraren.  Fungerar med flera olika fönsterahanterare.</translation>
    </message>
    <message>
        <source>Connection</source>
        <translation>Anslutning</translation>
    </message>
    <message>
        <source>Reconnect after Uin clash</source>
        <translation>Återantlut efter UINkrock</translation>
    </message>
    <message>
        <source>Protocol ID</source>
        <translation>Protokoll ID</translation>
    </message>
    <message>
        <source>Auto Update</source>
        <translation>Uppdatera automatiskt</translation>
    </message>
    <message>
        <source>Auto Update Info</source>
        <translation>Uppdatera automatiskt information</translation>
    </message>
    <message>
        <source>Automatically update users&apos; server stored information.</source>
        <translation>Uppdatera automatiskt användares sparade information på servern.</translation>
    </message>
    <message>
        <source>Auto Update Info Plugins</source>
        <translation>Uppdatera automatiskt Info plugin</translation>
    </message>
    <message>
        <source>Automatically update users&apos; Phone Book and Picture.</source>
        <translation>Uppdatera automatiskt användares telefonbok och bild.</translation>
    </message>
    <message>
        <source>Auto Update Status Plugins</source>
        <translation>Uppdatera automatiskt Status Plugin</translation>
    </message>
    <message>
        <source>Automatically update users&apos; Phone &quot;Follow Me&quot;, File Server and ICQphone status.</source>
        <translation>Uppdatera automatiskt användares Telefon &quot;Följ Mig&quot;, Filserver och ICQphone status.</translation>
    </message>
    <message>
        <source>Style:</source>
        <translation>Stil:</translation>
    </message>
    <message>
        <source>Insert Horizontal Line</source>
        <translation>Infoga horisontell linje</translation>
    </message>
    <message>
        <source>Colors</source>
        <translation>Färger</translation>
    </message>
    <message>
        <source>Message Received:</source>
        <translation>Meddelande mottaget:</translation>
    </message>
    <message>
        <source>History Received:</source>
        <translation>Gamla mottagna meddelanden:</translation>
    </message>
    <message>
        <source>History Sent:</source>
        <translation>Gamla skickade meddelanden:</translation>
    </message>
    <message>
        <source>Typing Notification Color:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Background Color:</source>
        <translation>Bakgrundsfärg:</translation>
    </message>
    <message>
        <source>Send typing notifications</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Send a notification to the user so they can see when you are typing a message to them</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Sticky Main Window</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Makes the Main window visible on all desktops</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Sticky Message Window</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Makes the Message window visible on all desktops</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Licq can reconnect you when you got disconnected because your Uin was used from another location. Check this if you want Licq to reconnect automatically.</source>
        <translation>Licq kan återansluta om du blivit frånkopplad på grund av att ditt UIN används från ett annat ställe. Markera detta om du vill återansluta automatiskt.</translation>
    </message>
    <message>
        <source>Show user display picture</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Show the user&apos;s display picture instead of a status icon, if the user is online and has a display picture</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Single line chat mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>In single line chat mode you send messages with Enter and insert new lines with Ctrl+Enter, opposite of the normal mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Alias</source>
        <translation>Alias</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Namn</translation>
    </message>
    <message>
        <source>Date Format:</source>
        <translation>Datumformat:</translation>
    </message>
    <message>
        <source>Picture</source>
        <translation>Bild</translation>
    </message>
    <message>
        <source>Notice:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This is a sent message</source>
        <translation>Dettar är ett meddelande som skickats</translation>
    </message>
    <message>
        <source>Have you gone to the Licq IRC Channel?</source>
        <translation>Har du besökt Licqs IRC-kanal?</translation>
    </message>
    <message>
        <source>No, where is it?</source>
        <translation>Nej, var finns den?</translation>
    </message>
    <message>
        <source>#Licq on irc.freenode.net</source>
        <translation>#Licq på irc.freenode.net</translation>
    </message>
    <message>
        <source>Cool, I&apos;ll see you there :)</source>
        <translation>Läckert, vi syns där :)</translation>
    </message>
    <message>
        <source>We&apos;ll be waiting!</source>
        <translation>Vi väntar!</translation>
    </message>
    <message>
        <source>Marge has left the conversation.</source>
        <translation>Marge har lämnat konversationen.</translation>
    </message>
    <message>
        <source>This is a received message</source>
        <translation>Detta är ett meddelande som tagits emot</translation>
    </message>
    <message>
        <source>Use double return</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Hitting Return twice will be used instead of Ctrl+Return to send messages and close input dialogs. Multiple new lines can be inserted with Ctrl+Return.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Show Join/Left Notices</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Show a notice in the chat window when a user joins or leaves the conversation.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Local time</source>
        <translation>Lokal tid</translation>
    </message>
    <message>
        <source>Chat Options</source>
        <translation>Chattinställningar</translation>
    </message>
    <message>
        <source>Insert Vertical Spacing</source>
        <translation>Infoga vertikalt mellanrum</translation>
    </message>
    <message>
        <source>Insert extra space between messages.</source>
        <translation>Infoga extra mellanrum mellan meddelandena.</translation>
    </message>
    <message>
        <source>Insert a line between each message.</source>
        <translation>Infoga en linje mellan varje meddelande.</translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>History Options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>History</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The format string used to define what will appear in each column.
The following parameters can be used:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>OwnerEditDlg</name>
    <message>
        <source>Edit Account</source>
        <translation>Redigera konto</translation>
    </message>
    <message>
        <source>User ID:</source>
        <translation>Användar ID:</translation>
    </message>
    <message>
        <source>Password:</source>
        <translation>Lösenord:</translation>
    </message>
    <message>
        <source>Protocol:</source>
        <translation>Protokoll:</translation>
    </message>
    <message>
        <source>Currently only one account per protocol is supported.</source>
        <translation>För närvarande kan endast ett konto per protokoll användas.</translation>
    </message>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Avbryt</translation>
    </message>
</context>
<context>
    <name>OwnerManagerDlg</name>
    <message>
        <source>Licq - Account Manager</source>
        <translation>Licq - Kontohanterare</translation>
    </message>
    <message>
        <source>&amp;Add</source>
        <translation>&amp;Lägg till</translation>
    </message>
    <message>
        <source>&amp;Register</source>
        <translation>&amp;Registrera</translation>
    </message>
    <message>
        <source>&amp;Modify</source>
        <translation>&amp;Modifiera</translation>
    </message>
    <message>
        <source>D&amp;elete</source>
        <translation>&amp;Ta bort</translation>
    </message>
    <message>
        <source>&amp;Done</source>
        <translation>&amp;Klar</translation>
    </message>
    <message>
        <source>From the Account Manager dialog you are able to add and register your accounts.
Currently, only one account per protocol is supported, but this will be changed in future versions.</source>
        <translation>Från Kontohanteraren kan du lägga till och registrera konton.
För närvarande kan endast ett konto per protokoll användas. Detta kommer att ändras i framtida versioner.</translation>
    </message>
    <message>
        <source>You are currently registered as
UIN (User ID): %1
Base Directory: %2
Rerun licq with the -b option to select a new
base directory and then register a new user.</source>
        <translation>Du är för närvarande registrerad som
UIN (AnvändarID): %1
Hemkatalog: %2
Startaom licq med växeln -b för att välja en ny
hemkatalog och registrera en ny användare.</translation>
    </message>
    <message>
        <source>Successfully registered, your user identification
number (UIN) is %1.
Now set your personal information.</source>
        <translation>Lyckad registrering, ditt användaridentifikationnummer
(UIN) är %1.
Fyll i din personliga information nu.</translation>
    </message>
    <message>
        <source>Registration failed.  See network window for details.</source>
        <translation>Registrering misslyckades. Se nätverksfönstret för mer information.</translation>
    </message>
</context>
<context>
    <name>OwnerView</name>
    <message>
        <source>User ID</source>
        <translation>AnvändarID</translation>
    </message>
    <message>
        <source>Protocol</source>
        <translation>Protokoll</translation>
    </message>
    <message>
        <source>(Invalid ID)</source>
        <translation>(Felaktigt ID)</translation>
    </message>
    <message>
        <source>Invalid Protocol</source>
        <translation>Felaktigt protokoll</translation>
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
        <translation>Klar</translation>
    </message>
    <message>
        <source>Load</source>
        <translation>Lägg till</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Namn</translation>
    </message>
    <message>
        <source>Plugin %1 has no configuration file</source>
        <translation>Tillägget %1 saknar konfigurationsfil</translation>
    </message>
    <message>
        <source>Enable</source>
        <translation>Aktivera</translation>
    </message>
    <message>
        <source>Licq Plugin %1 %2
</source>
        <translation>Licq tillägg %1 %2
</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Uppdatera</translation>
    </message>
    <message>
        <source>Description</source>
        <translation>Beskrivning</translation>
    </message>
    <message>
        <source>Version</source>
        <translation>Version</translation>
    </message>
    <message>
        <source>Standard Plugins</source>
        <translation type="unfinished">Standard plugin</translation>
    </message>
    <message>
        <source>Protocol Plugins</source>
        <translation type="unfinished">Protokoll plugin</translation>
    </message>
    <message>
        <source>(Unloaded)</source>
        <translation>(Ej laddad)</translation>
    </message>
</context>
<context>
    <name>QMessageBox</name>
    <message>
        <source>Licq Question</source>
        <translation>Licq Fråga</translation>
    </message>
    <message>
        <source>Yes</source>
        <translation>Ja</translation>
    </message>
    <message>
        <source>No</source>
        <translation>Nej</translation>
    </message>
</context>
<context>
    <name>RegisterUserDlg</name>
    <message>
        <source>Please enter your password in both input fields.</source>
        <translation>Vargod ange ditt lösenord i båda textrutorna.</translation>
    </message>
    <message>
        <source>Now please click &apos;Finish&apos; to start the registration process.</source>
        <translation>Vargod klicka &apos;Sltuför&apos; för att starta registreringsprocessen.</translation>
    </message>
    <message>
        <source>Now please press the &apos;Back&apos; button and try again.</source>
        <translation>Vargod klicka på &apos;Tillbaka&apos;-knappen och försök igen.</translation>
    </message>
    <message>
        <source>The passwords don&apos;t seem to match.</source>
        <translation>Lösenorden verkar inte stämma överräns.</translation>
    </message>
    <message>
        <source>Password:</source>
        <translation>Lösenord:</translation>
    </message>
    <message>
        <source>Verify:</source>
        <translation>Bekräfra:</translation>
    </message>
    <message>
        <source>Invalid password, must be between 1 and 8 characters.</source>
        <translation>Ogiltigt lösenord, måste vara mellan 1 och 8 tecken.</translation>
    </message>
    <message>
        <source>Welcome to the Registration Wizard.

You can register a new user here.

Press &quot;Next&quot; to proceed.</source>
        <translation>Välkommer till Registreringsguiden.

Du kan registrera en ny användare här.

Klicka &quot;Nästa&quot; för att fortsätta.</translation>
    </message>
    <message>
        <source>Account Registration</source>
        <translation>Kontoregistrering</translation>
    </message>
    <message>
        <source>Enter a password to protect your account.</source>
        <translation>Ange ett lösenord för att skydda ditt konto.</translation>
    </message>
    <message>
        <source>&amp;Remember Password</source>
        <translation>&amp;Spara lösenord</translation>
    </message>
    <message>
        <source>Account Registration - Step 2</source>
        <translation>Kontoregistrering - Steg 2</translation>
    </message>
    <message>
        <source>Account Registration - Step 3</source>
        <translation>Kontoregistrering - Steg 3</translation>
    </message>
    <message>
        <source>Licq Account Registration</source>
        <translation>Licq Kontoregistrering</translation>
    </message>
    <message>
        <source>Account Registration in Progress...</source>
        <translation>Kontoregistrering pågår...</translation>
    </message>
    <message>
        <source>Account registration has been successfuly completed.
Your new user id is %1.
You are now being automatically logged on.
Click OK to edit your personal details.
After you are online, you can send your personal details to the server.</source>
        <translation>Kontoregistreringen har avslutats lyckosamt.
Ditt nya användarid är %1.
Du kommer nu att automatiskt loggas in.
Klicka OK för att redigera dina personliga uppgifter.
Efter att du anslutit kan du skicka dina personliga uppgifter till servern.</translation>
    </message>
</context>
<context>
    <name>ReqAuthDlg</name>
    <message>
        <source>Licq - Request Authorization</source>
        <translation>Begär auktorisering</translation>
    </message>
    <message>
        <source>Request authorization from (UIN):</source>
        <translation>Begär auktorisering från (UIN):</translation>
    </message>
    <message>
        <source>Request</source>
        <translation>Begär</translation>
    </message>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Avbryt</translation>
    </message>
</context>
<context>
    <name>SearchUserDlg</name>
    <message>
        <source>Reset Search</source>
        <translation>Återsätll sökning</translation>
    </message>
    <message>
        <source>&amp;Done</source>
        <translation>&amp;Klar</translation>
    </message>
    <message>
        <source>UIN#:</source>
        <translation>UIN#:</translation>
    </message>
    <message>
        <source>Enter search parameters and select &apos;Search&apos;</source>
        <translation>Ange sökparametrar och välj &apos;Sök&apos;</translation>
    </message>
    <message>
        <source>Searching (this can take awhile)...</source>
        <translation>Söker (detta kan dröja en stund)...</translation>
    </message>
    <message>
        <source>More users found. Narrow search.</source>
        <translation>Fler användare funna. Begränsa sökning.</translation>
    </message>
    <message>
        <source>Alias:</source>
        <translation>Alias:</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Avbryt</translation>
    </message>
    <message>
        <source>First Name:</source>
        <translation>Förnamn:</translation>
    </message>
    <message>
        <source>Last Name:</source>
        <translation>Efternamn:</translation>
    </message>
    <message>
        <source>Search failed.</source>
        <translation>Sökning misslyckades.</translation>
    </message>
    <message>
        <source>A&amp;lert User</source>
        <translation>&amp;Varna användare</translation>
    </message>
    <message>
        <source>&amp;Add %1 Users</source>
        <translation>&amp;Lägg till %1 användare</translation>
    </message>
    <message>
        <source>Email Address:</source>
        <translation>E-postaddress:</translation>
    </message>
    <message>
        <source>&amp;Add User</source>
        <translation>&amp;Lägg till Användare</translation>
    </message>
    <message>
        <source>&amp;Search</source>
        <translation>&amp;Sök</translation>
    </message>
    <message>
        <source>Licq - User Search</source>
        <translation>Licq - användarsökning</translation>
    </message>
    <message>
        <source>Age Range:</source>
        <translation>Åldersintervall:</translation>
    </message>
    <message>
        <source>Unspecified</source>
        <translation>Ospecificerat</translation>
    </message>
    <message>
        <source>18 - 22</source>
        <translation>18 - 22</translation>
    </message>
    <message>
        <source>23 - 29</source>
        <translation>23 - 29</translation>
    </message>
    <message>
        <source>30 - 39</source>
        <translation>30 - 39</translation>
    </message>
    <message>
        <source>40 - 49</source>
        <translation>40 - 49</translation>
    </message>
    <message>
        <source>50 - 59</source>
        <translation>50 - 59</translation>
    </message>
    <message>
        <source>60+</source>
        <translation>60+</translation>
    </message>
    <message>
        <source>Gender:</source>
        <translation>Kön:</translation>
    </message>
    <message>
        <source>Female</source>
        <translation>Kvinnlig</translation>
    </message>
    <message>
        <source>Male</source>
        <translation>Manlig</translation>
    </message>
    <message>
        <source>Language:</source>
        <translation>Språk:</translation>
    </message>
    <message>
        <source>City:</source>
        <translation>Stad:</translation>
    </message>
    <message>
        <source>State:</source>
        <translation>Stat:</translation>
    </message>
    <message>
        <source>Country:</source>
        <translation>Land:</translation>
    </message>
    <message>
        <source>Company Name:</source>
        <translation>Företagsnamn:</translation>
    </message>
    <message>
        <source>Company Department:</source>
        <translation>Avdelning:</translation>
    </message>
    <message>
        <source>Company Position:</source>
        <translation>Position i företaget:</translation>
    </message>
    <message>
        <source>Keyword:</source>
        <translation>Nyckelord:</translation>
    </message>
    <message>
        <source>Return Online Users Only</source>
        <translation>Visa endast anslutna användare</translation>
    </message>
    <message>
        <source>&amp;Whitepages</source>
        <translation>&amp;Vitasidorna</translation>
    </message>
    <message>
        <source>%1 more users found. Narrow search.</source>
        <translation>%1 ytterligare användare funna. Begränsa din sökning.</translation>
    </message>
    <message>
        <source>Search complete.</source>
        <translation>Sökning färdig.</translation>
    </message>
    <message>
        <source>View &amp;Info</source>
        <translation>Visa &amp;Information</translation>
    </message>
    <message>
        <source>&amp;UIN#</source>
        <translation>&amp;UIN#</translation>
    </message>
</context>
<context>
    <name>SearchUserView</name>
    <message>
        <source>UIN</source>
        <translation>UIN</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Namn</translation>
    </message>
    <message>
        <source>Alias</source>
        <translation>Alias</translation>
    </message>
    <message>
        <source>Email</source>
        <translation>E-post</translation>
    </message>
    <message>
        <source>Status</source>
        <translation>Status</translation>
    </message>
    <message>
        <source>Sex &amp; Age</source>
        <translation>Kön &amp; Ålder</translation>
    </message>
    <message>
        <source>Authorize</source>
        <translation>Godkänn</translation>
    </message>
    <message>
        <source>Offline</source>
        <translation>Frånkopplad</translation>
    </message>
    <message>
        <source>Online</source>
        <translation>Ansluten</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation>Okänd</translation>
    </message>
    <message>
        <source>F</source>
        <translation>K</translation>
    </message>
    <message>
        <source>M</source>
        <translation>M</translation>
    </message>
    <message>
        <source>?</source>
        <translation>?</translation>
    </message>
    <message>
        <source>No</source>
        <translation>Nej</translation>
    </message>
    <message>
        <source>Yes</source>
        <translation>Ja</translation>
    </message>
</context>
<context>
    <name>SecurityDlg</name>
    <message>
        <source>error</source>
        <translation>fel</translation>
    </message>
    <message>
        <source>ICQ Security Options [Setting...]</source>
        <translation>ICQ säkerhetsalternativ [alternativ...</translation>
    </message>
    <message>
        <source>ICQ Security Options</source>
        <translation>ICQ säkerhetsalternativ</translation>
    </message>
    <message>
        <source>You need to be connected to the
ICQ Network to change the settings.</source>
        <translation>Du måste vara ansluten till ICQ-Nätverket för att ändra inställningarna.</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>dog ut</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>Alternativ</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>misslyckades</translation>
    </message>
    <message>
        <source>Authorization Required</source>
        <translation>Godkännande nödvändigt</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Avbryt</translation>
    </message>
    <message>
        <source>&amp;Update</source>
        <translation>&amp;Uppdatera</translation>
    </message>
    <message>
        <source>Web Presence</source>
        <translation>Webbnärvaro</translation>
    </message>
    <message>
        <source>ICQ Security Options [Setting...</source>
        <translation>ICQ säkerhetsalternativ [alternativ...</translation>
    </message>
    <message>
        <source>Determines whether regular ICQ clients require your authorization to add you to their contact list.</source>
        <translation>Avgör om vanliga ICQ-klienter behöver ditt godkännande för att lägga till dig i deras kontaktlistor</translation>
    </message>
    <message>
        <source>Hide IP</source>
        <translation>Dölj IP</translation>
    </message>
    <message>
        <source>Password/UIN settings</source>
        <translation>Lösenord/UIN inställningar</translation>
    </message>
    <message>
        <source>&amp;Uin:</source>
        <translation>&amp;Uin:</translation>
    </message>
    <message>
        <source>&amp;Password:</source>
        <translation>&amp;Lösenord:</translation>
    </message>
    <message>
        <source>Enter your ICQ password here.</source>
        <translation>Skriv ditt ICQ-lösenord här.</translation>
    </message>
    <message>
        <source>Verify your ICQ password here.</source>
        <translation>Bekräfta ditt ICQ-lösenord här.</translation>
    </message>
    <message>
        <source>&amp;Local changes only</source>
        <translation>Endast &amp;Lokala ändringar</translation>
    </message>
    <message>
        <source>If checked, password/UIN changes will apply only on your local computer.  Useful if your password is incorrectly saved in Licq.</source>
        <translation>Om ikryssad kommer lösenord/UIN ändringar endast att ske på din lokala dator. Användbart om ett felaktigt lösenord sparats av Licq.</translation>
    </message>
    <message>
        <source>Web Presence allows users to see if you are online through your web indicator.</source>
        <translation>Webbnärvaro tillåter användare att genom din webbindikator se om du är ansluten.</translation>
    </message>
    <message>
        <source>Hide IP stops users from seeing your IP address. It doesn&apos;t guarantee it will be hidden though.</source>
        <translation>Dölj IP förhindrar att användare ser din IP adress. Det garanterar dock inte att den är osynlig.</translation>
    </message>
    <message>
        <source>Invalid password, must be between 1 and 8 characters.</source>
        <translation>Ogiltigt lösenord, måste vara mellan 1 och 8 tecken.</translation>
    </message>
    <message>
        <source>Passwords do not match, try again.</source>
        <translation>Lösenorden är olika, försök igen.</translation>
    </message>
    <message>
        <source>Setting security options failed.</source>
        <translation>Misslyckades att spara säkerhetsinställningar.</translation>
    </message>
    <message>
        <source>Changing password failed.</source>
        <translation>Byte av lösenord misslyckades.</translation>
    </message>
    <message>
        <source>Timeout while setting security options.</source>
        <translation>Anslutningen dog ut när säkerhetsinställningarna skulle sparas.</translation>
    </message>
    <message>
        <source>Timeout while changing password.</source>
        <translation>Anslutnignen dog ut när lösenordet skulle bytas.</translation>
    </message>
    <message>
        <source>Internal error while setting security options.</source>
        <translation>Internt fel vid försök att spara säkerhetsinställningar.</translation>
    </message>
    <message>
        <source>Internal error while changing password.</source>
        <translation>Internt fel vid försök att ändra lösenord.</translation>
    </message>
    <message>
        <source>Enter the UIN which you want to use.  Only available if &quot;Local changes only&quot; is checked.</source>
        <translation>Skriv det UIN som du vill använda. Endast tillgänglig om &quot;Endast lokala ändringar&quot; är ikryssad.</translation>
    </message>
    <message>
        <source>&amp;Verify:</source>
        <translation>&amp;Bekräfta:</translation>
    </message>
</context>
<context>
    <name>ShowAwayMsgDlg</name>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>error</source>
        <translation>fel</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>dog ut</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>misslyckades</translation>
    </message>
    <message>
        <source>&amp;Show Again</source>
        <translation>&amp;Visa igen</translation>
    </message>
    <message>
        <source>%1 Response for %2</source>
        <translation>%1 svar till %2</translation>
    </message>
    <message>
        <source>refused</source>
        <translation>vägrad</translation>
    </message>
</context>
<context>
    <name>SkinBrowserDlg</name>
    <message>
        <source>Error</source>
        <translation>Fel</translation>
    </message>
    <message>
        <source>&amp;Apply</source>
        <translation>&amp;Använd</translation>
    </message>
    <message>
        <source>Licq Skin Browser</source>
        <translation>Licq temabläddrare</translation>
    </message>
    <message>
        <source>Skin selection</source>
        <translation>Temaväljare</translation>
    </message>
    <message>
        <source>Preview</source>
        <translation>Förhandsgranska</translation>
    </message>
    <message>
        <source>S&amp;kins:</source>
        <translation>&amp;Teman:</translation>
    </message>
    <message>
        <source>&amp;Icons:</source>
        <translation>&amp;Ikoner:</translation>
    </message>
    <message>
        <source>E&amp;xtended Icons:</source>
        <translation>&amp;Utökade ikoner:</translation>
    </message>
    <message>
        <source>Skin:</source>
        <translation>Tema:</translation>
    </message>
    <message>
        <source>Icons:</source>
        <translation>Ikoner:</translation>
    </message>
    <message>
        <source>Extended Icons:</source>
        <translation>Utökade ikoner:</translation>
    </message>
    <message>
        <source>&amp;Edit Skin</source>
        <translation>&amp;Redigera tema</translation>
    </message>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>Av&amp;bryt</translation>
    </message>
    <message>
        <source>Unable to open icons file
%1
Iconset &apos;%2&apos; has been disabled.</source>
        <translation>Kan inte öppna ikon fil
%1
Ikongrupp &apos;%2&apos; är ej tillgänglig.</translation>
    </message>
    <message>
        <source>Unable to open extended icons file
%1
Extended Iconset &apos;%2&apos; has been disabled.</source>
        <translation>Kan inte öppna fil %1 med utökade ikoner.
Gruppen utökadeikoner &apos;%2&apos; är ej tillgänglig.</translation>
    </message>
    <message>
        <source>Unable to open icons file
%1</source>
        <translation>Kan inte öppna ikonfilen
%1</translation>
    </message>
    <message>
        <source>Unable to open extended icons file
%1</source>
        <translation>Kan inte öppna fil %1 med utökade ikoner</translation>
    </message>
    <message>
        <source>Use this combo box to select one of the available skins</source>
        <translation>Använd denna meny för att välja ett av de tillgängliga temana</translation>
    </message>
    <message>
        <source>Use this combo box to select one of the available icon sets</source>
        <translation>Använd denna meny för att välja ett av de tillgängliga ikonpacketen</translation>
    </message>
    <message>
        <source>Use this combo box to select one of the available extended icon sets</source>
        <translation>Använd denna meny för att välja ett av de tillgängliga packeten med utökade ikoner</translation>
    </message>
    <message>
        <source>Use this combo box to select one of the available emoticon icon sets</source>
        <translation>Använd denna ruta för att välja ett av setten med känsloikoner</translation>
    </message>
    <message>
        <source>Emoticons:</source>
        <translation>Känsloikoner:</translation>
    </message>
    <message>
        <source>E&amp;moticons:</source>
        <translation>K&amp;änsloikoner:</translation>
    </message>
</context>
<context>
    <name>Status</name>
    <message>
        <source>Offline</source>
        <translation>Frånkopplad</translation>
    </message>
    <message>
        <source>Online</source>
        <translation>Ansluten</translation>
    </message>
    <message>
        <source>Away</source>
        <translation>Borta</translation>
    </message>
    <message>
        <source>Do Not Disturb</source>
        <translation>Stör ej</translation>
    </message>
    <message>
        <source>Not Available</source>
        <translation>Ej tillgänglig</translation>
    </message>
    <message>
        <source>Occupied</source>
        <translation>Upptagen</translation>
    </message>
    <message>
        <source>Free for Chat</source>
        <translation>Chattledig</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation>Okänd</translation>
    </message>
    <message>
        <source>Off</source>
        <translation>Från</translation>
    </message>
    <message>
        <source>On</source>
        <translation>Ansl</translation>
    </message>
    <message>
        <source>DND</source>
        <translation>Stör ej</translation>
    </message>
    <message>
        <source>N/A</source>
        <translation>Ej till</translation>
    </message>
    <message>
        <source>Occ</source>
        <translation>Uppt</translation>
    </message>
    <message>
        <source>FFC</source>
        <translation>FFC</translation>
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
        <source>Arabic</source>
        <translation>Arabiska</translation>
    </message>
    <message>
        <source>Baltic</source>
        <translation>Baltiska</translation>
    </message>
    <message>
        <source>Central European</source>
        <translation>Centraleuropeiska</translation>
    </message>
    <message>
        <source>Chinese</source>
        <translation>Kinesiska</translation>
    </message>
    <message>
        <source>Chinese Traditional</source>
        <translation>Traditionell Kinesiska</translation>
    </message>
    <message>
        <source>Cyrillic</source>
        <translation>Kyrilliska</translation>
    </message>
    <message>
        <source>Esperanto</source>
        <translation>Esperanto</translation>
    </message>
    <message>
        <source>Greek</source>
        <translation>Grekiska</translation>
    </message>
    <message>
        <source>Hebrew</source>
        <translation>Hebreiska</translation>
    </message>
    <message>
        <source>Japanese</source>
        <translation>Japanska</translation>
    </message>
    <message>
        <source>Korean</source>
        <translation>Koreanska</translation>
    </message>
    <message>
        <source>Western European</source>
        <translation>Västeuropeiska</translation>
    </message>
    <message>
        <source>Tamil</source>
        <translation>Tamilska</translation>
    </message>
    <message>
        <source>Thai</source>
        <translation>Thailändska</translation>
    </message>
    <message>
        <source>Turkish</source>
        <translation>Turkiska</translation>
    </message>
    <message>
        <source>Ukrainian</source>
        <translation>Ukrainska</translation>
    </message>
    <message>
        <source>Unicode-16</source>
        <translation>Unicode-16</translation>
    </message>
</context>
<context>
    <name>UserEventCommon</name>
    <message>
        <source>Time:</source>
        <translation>Tid:</translation>
    </message>
    <message>
        <source>Show User Info</source>
        <translation>Visa användarinformation</translation>
    </message>
    <message>
        <source>Status:</source>
        <translation>Status:</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation>Okänd</translation>
    </message>
    <message>
        <source>Show User History</source>
        <translation>Visa användarhistorik</translation>
    </message>
    <message>
        <source>Open / Close secure channel</source>
        <translation>Öppna / Stäng säker kanal</translation>
    </message>
    <message>
        <source>Change user text encoding</source>
        <translation>Ändra fontsysteme för användare</translation>
    </message>
    <message>
        <source>This button selects the text encoding used when communicating with this user. You might need to change the encoding to communicate in a different language.</source>
        <translation>Den här knappen väljer vilket fontsystem som ska användas när du kommunicerar med denna användare. Du kan behöva byta fontsystem för att kommunicera på ett annat språk.</translation>
    </message>
    <message>
        <source>Unable to load encoding &lt;b&gt;%1&lt;/b&gt;. Message contents may appear garbled.</source>
        <translation>Kunde inte ladda fontsystem &lt;b&gt;%1&lt;/b&gt;. Meddelandet kan verka skadat.</translation>
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
        <translation>Nu</translation>
    </message>
    <message>
        <source>Age:</source>
        <translation>Ålder:</translation>
    </message>
    <message>
        <source>Fax:</source>
        <translation>Fax:</translation>
    </message>
    <message>
        <source>Male</source>
        <translation>Manlig</translation>
    </message>
    <message>
        <source>Zip:</source>
        <translation>Postnummer:</translation>
    </message>
    <message>
        <source>done</source>
        <translation>klar</translation>
    </message>
    <message>
        <source> Day:</source>
        <translation> Dag:</translation>
    </message>
    <message>
        <source>&amp;Last</source>
        <translation>&amp;Sista</translation>
    </message>
    <message>
        <source>&amp;Menu</source>
        <translation>&amp;Meny</translation>
    </message>
    <message>
        <source>&amp;More</source>
        <translation>&amp;Mer</translation>
    </message>
    <message>
        <source>&amp;Save</source>
        <translation>&amp;Spara</translation>
    </message>
    <message>
        <source>&amp;Work</source>
        <translation>&amp;Arbete</translation>
    </message>
    <message>
        <source>&amp;Filter: </source>
        <translation>&amp;Filter: </translation>
    </message>
    <message>
        <source>City:</source>
        <translation>Stad:</translation>
    </message>
    <message>
        <source>P&amp;rev</source>
        <translation>&amp;Föregående</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation>Namn:</translation>
    </message>
    <message>
        <source>Nex&amp;t</source>
        <translation>&amp;Nästa</translation>
    </message>
    <message>
        <source>error</source>
        <translation>fel</translation>
    </message>
    <message>
        <source>Last Online:</source>
        <translation>Senast ansluten:</translation>
    </message>
    <message>
        <source>Updating server...</source>
        <translation>Uppdaterar server...</translation>
    </message>
    <message>
        <source> Year:</source>
        <translation> År</translation>
    </message>
    <message>
        <source>Last Sent Event:</source>
        <translation>Senast skickade händelse:</translation>
    </message>
    <message>
        <source>Last Checked Auto Response:</source>
        <translation>Senast visade autosvar:</translation>
    </message>
    <message>
        <source>&amp;About</source>
        <translation>&amp;Om</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Stäng</translation>
    </message>
    <message>
        <source>You need to be connected to the
ICQ Network to retrieve your settings.</source>
        <translation>Du måste vara ansluten till
ICQ-nätverket för att hämta dina inställningar.</translation>
    </message>
    <message>
        <source>Cellular:</source>
        <translation>Mobil:</translation>
    </message>
    <message>
        <source>EMail 1:</source>
        <translation>E-post 1:</translation>
    </message>
    <message>
        <source>EMail 2:</source>
        <translation>E-post 2:</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>dog ut</translation>
    </message>
    <message>
        <source>Homepage:</source>
        <translation>Hemsida:</translation>
    </message>
    <message>
        <source>Timezone:</source>
        <translation>Tidszon:</translation>
    </message>
    <message>
        <source>About:</source>
        <translation>Om:</translation>
    </message>
    <message>
        <source>Alias:</source>
        <translation>Alias:</translation>
    </message>
    <message>
        <source>Old Email:</source>
        <translation>Gammal e-post:</translation>
    </message>
    <message>
        <source>Female</source>
        <translation>Kvinnlig</translation>
    </message>
    <message>
        <source> Month:</source>
        <translation> Månad:</translation>
    </message>
    <message>
        <source>Phone:</source>
        <translation>Telefon:</translation>
    </message>
    <message>
        <source>State:</source>
        <translation>Stat:</translation>
    </message>
    <message>
        <source>Last Received Event:</source>
        <translation>Senast mottagna händelse:</translation>
    </message>
    <message>
        <source>Authorization Not Required</source>
        <translation>Godkännande ej nödvändigt</translation>
    </message>
    <message>
        <source>[&lt;font color=&quot;%1&quot;&gt;Received&lt;/font&gt;] [&lt;font color=&quot;%2&quot;&gt;Sent&lt;/font&gt;] %3 out of %4 matches</source>
        <translation>[&lt;font color=&quot;%1&quot;&gt;Mottaget&lt;/font&gt;] [&lt;font color=&quot;%2&quot;&gt;Skickat&lt;/font&gt;] %3 utav %4 överränsstämmelser</translation>
    </message>
    <message>
        <source>[&lt;font color=&quot;%1&quot;&gt;Received&lt;/font&gt;] [&lt;font color=&quot;%2&quot;&gt;Sent&lt;/font&gt;] %3 to %4 of %5</source>
        <translation>[&lt;font color=&quot;%1&quot;&gt;Mottaget&lt;/font&gt;] [&lt;font color=&quot;%2&quot;&gt;Skickat&lt;/font&gt;] %3 till %4 utav %5</translation>
    </message>
    <message>
        <source>Country:</source>
        <translation>Land:</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>misslyckades</translation>
    </message>
    <message>
        <source>server</source>
        <translation>server</translation>
    </message>
    <message>
        <source>Authorization Required</source>
        <translation>Godkännande nödvändigt</translation>
    </message>
    <message>
        <source>Birthday:</source>
        <translation>Födelsedag:</translation>
    </message>
    <message>
        <source>Unknown (%1)</source>
        <translation>Okänd (%1)</translation>
    </message>
    <message>
        <source>Department:</source>
        <translation>Avdelning:</translation>
    </message>
    <message>
        <source>Position:</source>
        <translation>Befattning:</translation>
    </message>
    <message>
        <source>Sorry, history is disabled for this person.</source>
        <translation>Tyvärr, historik är inaktiverad för den här personen.</translation>
    </message>
    <message>
        <source>Licq - Info </source>
        <translation>Licq - information</translation>
    </message>
    <message>
        <source>Status:</source>
        <translation>Status:</translation>
    </message>
    <message>
        <source>Address:</source>
        <translation>Adress:</translation>
    </message>
    <message>
        <source>Updating...</source>
        <translation>Uppdaterar...</translation>
    </message>
    <message>
        <source>You need to be connected to the
ICQ Network to change your settings.</source>
        <translation>Du måste vara ansluten till
ICQ-nätverket för att ändra dina inställningar.</translation>
    </message>
    <message>
        <source>Language 3:</source>
        <translation>Språk 3:</translation>
    </message>
    <message>
        <source>Language 1:</source>
        <translation>Språk 1:</translation>
    </message>
    <message>
        <source>Language 2:</source>
        <translation>Språk 2:</translation>
    </message>
    <message>
        <source>Unspecified</source>
        <translation>Ospecificerat</translation>
    </message>
    <message>
        <source>&amp;Update</source>
        <translation>&amp;Uppdatera</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation>Okänd</translation>
    </message>
    <message>
        <source>Rever&amp;se</source>
        <translation>&amp;Omvänd</translation>
    </message>
    <message>
        <source>Gender:</source>
        <translation>Kön:</translation>
    </message>
    <message>
        <source>&amp;General</source>
        <translation>&amp;Allmänt</translation>
    </message>
    <message>
        <source>&amp;History</source>
        <translation>&amp;Historik</translation>
    </message>
    <message>
        <source>Error loading history file: %1
Description: %2</source>
        <translation>Fel vid inhämtande av historikfil: %1
Beskrivning: %2</translation>
    </message>
    <message>
        <source>INVALID USER</source>
        <translation>OGILTIG ANVÄNDARE</translation>
    </message>
    <message>
        <source>Online Since:</source>
        <translation>Ansluten sedan:</translation>
    </message>
    <message>
        <source>Offline</source>
        <translation>Frånkopplad</translation>
    </message>
    <message>
        <source>&amp;Retrieve</source>
        <translation>&amp;Hämta</translation>
    </message>
    <message>
        <source>S&amp;end</source>
        <translation>&amp;Skicka</translation>
    </message>
    <message>
        <source>Keep Alias on Update</source>
        <translation>Behålla smeknamn vid updatering</translation>
    </message>
    <message>
        <source>Normally Licq overwrites the Alias when updating user details.
Check this if you want to keep your changes to the Alias.</source>
        <translation>Normalt skriver Licq över smeknamnet när man updaterar användarinformationen.
Markera här för att behålla ändringar av smeknamnet.</translation>
    </message>
    <message>
        <source>Retrieve</source>
        <translation>Hämta</translation>
    </message>
    <message>
        <source>ID:</source>
        <translation>ID:</translation>
    </message>
    <message>
        <source>Category:</source>
        <translation>Kategori:</translation>
    </message>
    <message>
        <source>Description:</source>
        <translation>Beskrivning:</translation>
    </message>
    <message>
        <source>User has an ICQ Homepage </source>
        <translation>Användaren har en ICQ-hemsida </translation>
    </message>
    <message>
        <source>User has no ICQ Homepage</source>
        <translation>Användaren har ingen ICQ-hemsida</translation>
    </message>
    <message>
        <source>M&amp;ore II</source>
        <translation>&amp;Mera II</translation>
    </message>
    <message>
        <source>(none)</source>
        <translation>(ingen)</translation>
    </message>
    <message>
        <source>Occupation:</source>
        <translation>Sysselsättning:</translation>
    </message>
    <message>
        <source>&amp;Phone</source>
        <translation>&amp;Telefon</translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Typ</translation>
    </message>
    <message>
        <source>Number/Gateway</source>
        <translation>Nummer/Gateway</translation>
    </message>
    <message>
        <source>Country/Provider</source>
        <translation>Land/Operatör</translation>
    </message>
    <message>
        <source>Currently at:</source>
        <translation>För tillfället:</translation>
    </message>
    <message>
        <source>(</source>
        <translation>(</translation>
    </message>
    <message>
        <source>) </source>
        <translation>) </translation>
    </message>
    <message>
        <source>-</source>
        <translation>-</translation>
    </message>
    <message>
        <source>P&amp;icture</source>
        <translation>&amp;Bild</translation>
    </message>
    <message>
        <source>Not Available</source>
        <translation>Ej tillgänglig</translation>
    </message>
    <message>
        <source>Failed to Load</source>
        <translation>Misslyckades att ladda</translation>
    </message>
    <message>
        <source>Email:</source>
        <translation>E-post:</translation>
    </message>
    <message>
        <source>&amp;Add</source>
        <translation>&amp;Lägg till</translation>
    </message>
    <message>
        <source>&amp;Clear</source>
        <translation>&amp;Rensa</translation>
    </message>
    <message>
        <source>&amp;Browse</source>
        <translation>&amp;Bläddra</translation>
    </message>
    <message>
        <source>Select your picture</source>
        <translation>Välj din bild</translation>
    </message>
    <message>
        <source> is over %1 bytes.
Select another picture?</source>
        <translation> är mer än %1 bytes
Välj en annan bild?</translation>
    </message>
    <message>
        <source>Yes</source>
        <translation>Ja</translation>
    </message>
    <message>
        <source>No</source>
        <translation>Nej</translation>
    </message>
    <message>
        <source>&amp;KDE Addressbook</source>
        <translation>&amp;KDEs adressbok</translation>
    </message>
    <message>
        <source>Registration Date:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>UserSelectDlg</name>
    <message>
        <source>&amp;User:</source>
        <translation>&amp;Användare:</translation>
    </message>
    <message>
        <source>&amp;Password:</source>
        <translation>&amp;Lösenord:</translation>
    </message>
    <message>
        <source>&amp;Save Password</source>
        <translation>&amp;Spara lösenord</translation>
    </message>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Avbryt</translation>
    </message>
    <message>
        <source>Licq User Select</source>
        <translation>Välj Användare för Licq</translation>
    </message>
</context>
<context>
    <name>UserSendChatEvent</name>
    <message>
        <source>Multiparty: </source>
        <translation>Grupp: </translation>
    </message>
    <message>
        <source>Invite</source>
        <translation>Bjud in</translation>
    </message>
    <message>
        <source> - Chat Request</source>
        <translation> - Chattbegäran</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation>Rensa</translation>
    </message>
    <message>
        <source>No reason provided</source>
        <translation>Ingen anledning given</translation>
    </message>
    <message>
        <source>Chat with %2 refused:
%3</source>
        <translation>%2 vägrade chatta med dig:
%3</translation>
    </message>
</context>
<context>
    <name>UserSendCommon</name>
    <message>
        <source>No</source>
        <translation>Nej</translation>
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
        <translation>klar</translation>
    </message>
    <message>
        <source>&amp;Menu</source>
        <translation>&amp;Meny</translation>
    </message>
    <message>
        <source>&amp;Send</source>
        <translation>&amp;Skicka</translation>
    </message>
    <message>
        <source>error</source>
        <translation>fel</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>S&amp;täng</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>dog ut</translation>
    </message>
    <message>
        <source>Message</source>
        <translation>Meddelande</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Avbryt</translation>
    </message>
    <message>
        <source>File Transfer</source>
        <translation>Filöverföring</translation>
    </message>
    <message>
        <source>Drag Users Here
Right Click for Options</source>
        <translation>Dra användare hit
högerklicka för alternativ</translation>
    </message>
    <message>
        <source>direct</source>
        <translation>direkt</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>misslyckades</translation>
    </message>
    <message>
        <source>via server</source>
        <translation>via server</translation>
    </message>
    <message>
        <source>U&amp;rgent</source>
        <translation>&amp;Brådskande</translation>
    </message>
    <message>
        <source>Direct send failed,
send through server?</source>
        <translation>Direktsändning misslyckades,
skicka genom server?</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Avbryt</translation>
    </message>
    <message>
        <source>Sending </source>
        <translation>Skickar </translation>
    </message>
    <message>
        <source>Chat Request</source>
        <translation>Chattbegäran</translation>
    </message>
    <message>
        <source>Contact List</source>
        <translation>Kontaktlista</translation>
    </message>
    <message>
        <source>M&amp;ultiple recipients</source>
        <translation>&amp;Flera mottagare</translation>
    </message>
    <message>
        <source>Se&amp;nd through server</source>
        <translation>S&amp;kicka genom server</translation>
    </message>
    <message>
        <source>SMS</source>
        <translation>SMS</translation>
    </message>
    <message>
        <source>%1 is in %2 mode:
%3
Send...</source>
        <translation>%1 är i %2 läge:
%3
Skicka...</translation>
    </message>
    <message>
        <source>Urgent</source>
        <translation>Brådskande</translation>
    </message>
    <message>
        <source> to Contact List</source>
        <translation>till kontaktlistan</translation>
    </message>
    <message>
        <source>Warning: Message can&apos;t be sent securely
through the server!</source>
        <translation>Varning: Meddelanden kan inte skickas säkert
genom servern!</translation>
    </message>
    <message>
        <source>Send anyway</source>
        <translation>Skicka i alla fall</translation>
    </message>
    <message>
        <source>cancelled</source>
        <translation>avbruten</translation>
    </message>
    <message>
        <source>Error! no owner set</source>
        <translation>Fel! ingen ägare vald</translation>
    </message>
    <message>
        <source>%1 has joined the conversation.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>%1 has left the conversation.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>UserSendContactEvent</name>
    <message>
        <source>Drag Users Here - Right Click for Options</source>
        <translation>Dra användare hit - högerklicka för alternativ</translation>
    </message>
    <message>
        <source> - Contact List</source>
        <translation> - Kontaktlista</translation>
    </message>
</context>
<context>
    <name>UserSendFileEvent</name>
    <message>
        <source>File(s): </source>
        <translation>Fil(er): </translation>
    </message>
    <message>
        <source>Browse</source>
        <translation>Bläddra</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Redigera</translation>
    </message>
    <message>
        <source> - File Transfer</source>
        <translation> - Filöverföring</translation>
    </message>
    <message>
        <source>Select files to send</source>
        <translation>Välj filer att skicka</translation>
    </message>
    <message>
        <source>You must specify a file to transfer!</source>
        <translation>Du måste ange en fil att överföra!</translation>
    </message>
    <message>
        <source>No reason provided</source>
        <translation>Ingen anledning given</translation>
    </message>
    <message>
        <source>File transfer with %2 refused:
%3</source>
        <translation>Filöverföring med %2 vägrad:
%3</translation>
    </message>
</context>
<context>
    <name>UserSendMsgEvent</name>
    <message>
        <source>&amp;No</source>
        <translation>&amp;Nej</translation>
    </message>
    <message>
        <source>&amp;Yes</source>
        <translation>&amp;Ja</translation>
    </message>
    <message>
        <source> - Message</source>
        <translation> - Meddelande</translation>
    </message>
    <message>
        <source>You didn&apos;t edit the message.
Do you really want to send it?</source>
        <translation>Du har inte redigerat meddelandet.
Skicka det i alla fall?</translation>
    </message>
</context>
<context>
    <name>UserSendSmsEvent</name>
    <message>
        <source>Phone : </source>
        <translation>Telefon:</translation>
    </message>
    <message>
        <source>Chars left : </source>
        <translation>Tecken kvar:</translation>
    </message>
    <message>
        <source> - SMS</source>
        <translation>- SMS</translation>
    </message>
    <message>
        <source>You didn&apos;t edit the SMS.
Do you really want to send it?</source>
        <translation>Du har inte redigerat SMS:et.
Vill du skicka det i alla fall?</translation>
    </message>
    <message>
        <source>&amp;Yes</source>
        <translation>&amp;Ja</translation>
    </message>
    <message>
        <source>&amp;No</source>
        <translation>&amp;Nej</translation>
    </message>
</context>
<context>
    <name>UserSendUrlEvent</name>
    <message>
        <source> - URL</source>
        <translation> - URL</translation>
    </message>
    <message>
        <source>URL : </source>
        <translation>URL : </translation>
    </message>
    <message>
        <source>No URL specified</source>
        <translation>Ingen URL angiven</translation>
    </message>
</context>
<context>
    <name>UserViewEvent</name>
    <message>
        <source>Chat</source>
        <translation>Chatt</translation>
    </message>
    <message>
        <source>&amp;Join</source>
        <translation>&amp;Delta</translation>
    </message>
    <message>
        <source>&amp;Menu</source>
        <translation>&amp;Meny</translation>
    </message>
    <message>
        <source>&amp;View</source>
        <translation>&amp;Visa</translation>
    </message>
    <message>
        <source>Nex&amp;t</source>
        <translation>&amp;Nästa</translation>
    </message>
    <message>
        <source>A&amp;dd %1 Users</source>
        <translation>&amp;Lägg till %1 Användare</translation>
    </message>
    <message>
        <source>Aut&amp;o Close</source>
        <translation>&amp;Autostäng</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>S&amp;täng</translation>
    </message>
    <message>
        <source>&amp;Quote</source>
        <translation>&amp;Citera</translation>
    </message>
    <message>
        <source>&amp;Reply</source>
        <translation>&amp;Svara</translation>
    </message>
    <message>
        <source>A&amp;ccept</source>
        <translation>&amp;Acceptera</translation>
    </message>
    <message>
        <source>Normal Click - Close Window
&lt;CTRL&gt;+Click - also delete User</source>
        <translation>Vänsterklicka - stäng fönster
&lt;Ctrl&gt;+Vänsterklicka - ta samtidigt bort användare</translation>
    </message>
    <message>
        <source>File Transfer</source>
        <translation>Filöverföring</translation>
    </message>
    <message>
        <source>Start Chat</source>
        <translation>Påbörja chatt</translation>
    </message>
    <message>
        <source>
--------------------
Request was cancelled.</source>
        <translation> 
-------------------
Begäran avbröts.</translation>
    </message>
    <message>
        <source>A&amp;dd User</source>
        <translation>&amp;Lägg till Användare</translation>
    </message>
    <message>
        <source>A&amp;uthorize</source>
        <translation>&amp;Godkänn</translation>
    </message>
    <message>
        <source>&amp;Refuse</source>
        <translation>&amp;Vägra</translation>
    </message>
    <message>
        <source>Nex&amp;t (%1)</source>
        <translation>&amp;Nästa (%1)</translation>
    </message>
    <message>
        <source>&amp;Forward</source>
        <translation>&amp;Vidarebefodra</translation>
    </message>
    <message>
        <source>&amp;View Email</source>
        <translation>&amp;Visa E-post</translation>
    </message>
    <message>
        <source>Licq is unable to find a browser application due to an internal error.</source>
        <translation>Licq kan inte hitta en webläsare pga. ett internt fel.</translation>
    </message>
    <message>
        <source>Licq is unable to start your browser and open the URL.
You will need to start the browser and open the URL manually.</source>
        <translation>Licq kan inte starta din webbläsare och öppna länken.
Du måste starta din webbläsare och öppna länken manuellt.</translation>
    </message>
    <message>
        <source>&amp;View Info</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>VerifyDlg</name>
    <message>
        <source>Retype the letters shown above:</source>
        <translation>Skriv bokstäverna som visas ovan:</translation>
    </message>
    <message>
        <source>&amp;OK</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>Licq - New Account Verification</source>
        <translation>Licq - Verifiering av nytt konto</translation>
    </message>
</context>
<context>
    <name>WharfIcon</name>
    <message>
        <source>Left click - Show main window
Middle click - Show next message
Right click - System menu</source>
        <translation>Vänsterklicka - Visa huvudfönstret
Mittenklicka - Visa nästa meddelande
Högerklicka - System meny</translation>
    </message>
</context>
</TS>
