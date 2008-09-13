<!DOCTYPE TS><TS>
<context>
    <name>@default</name>
    <message>
        <source>KDE default</source>
        <translation>KDE por defecto</translation>
    </message>
</context>
<context>
    <name>AddUserDlg</name>
    <message>
        <source>Licq - Add User</source>
        <translation>Licq - Añadir Usuario</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Cancelar</translation>
    </message>
    <message>
        <source>Protocol:</source>
        <translation>Protocolo</translation>
    </message>
    <message>
        <source>New User ID:</source>
        <translation>ID del nuevo usuario</translation>
    </message>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;Ok</translation>
    </message>
</context>
<context>
    <name>AuthUserDlg</name>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;Ok</translation>
    </message>
    <message>
        <source>Refuse authorization to %1</source>
        <translation>Rechazar autorización a %1</translation>
    </message>
    <message>
        <source>Authorize which user (UIN):</source>
        <translation type="obsolete">Autorizar usuario (UIN):</translation>
    </message>
    <message>
        <source>Grant authorization to %1</source>
        <translation>Conceder autorización a %1</translation>
    </message>
    <message>
        <source>Licq - Grant Authorisation</source>
        <translation>Licq - Conceder Autorización</translation>
    </message>
    <message>
        <source>Licq - Refuse Authorisation</source>
        <translation>Licq - Rechazar Autorización</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Cancelar</translation>
    </message>
    <message>
        <source>Response</source>
        <translation>Respuesta</translation>
    </message>
    <message>
        <source>Authorize which user (Id):</source>
        <translation>Autorizar usuario (Id):</translation>
    </message>
</context>
<context>
    <name>AwayMsgDlg</name>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;Ok</translation>
    </message>
    <message>
        <source>&amp;Edit Items</source>
        <translation>&amp;Editar Elementos</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Cancelar</translation>
    </message>
    <message>
        <source>&amp;Select</source>
        <translation>&amp;Seleccionar</translation>
    </message>
    <message>
        <source>Set %1 Response for %2</source>
        <translation>Configurar respuesta %1 para %2</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Hints for Setting&lt;br&gt;your Auto-Response&lt;/h2&gt;&lt;br&gt;&lt;hr&gt;&lt;br&gt;&lt;ul&gt;&lt;li&gt;You can include any of the % expansions (described in the main hints page).&lt;/li&gt;&lt;li&gt;Any line beginning with a pipe (|) will be treated as a command to be run.  The line will be replaced by the output of the command. The command is parsed by /bin/sh so any shell commands or meta-characters are allowed.  For security reasons, any % expansions are automatically passed to the command surrounded by single quotes to prevent shell parsing of any meta-characters included in an alias.&lt;br&gt;Examples of popular uses include:&lt;ul&gt;&lt;li&gt;&lt;tt&gt;|date&lt;/tt&gt;: Will replace that line by the current date&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|fortune&lt;/tt&gt;: Show a fortune, as a tagline for example&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|myscript.sh %u %a&lt;/tt&gt;: Run a script, passing the uin and alias&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|myscript.sh %u %a &gt; /dev/null&lt;/tt&gt;: Run the same script but ignore the output (for tracking auto response checks or something)&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|if [ %u -lt 100000 ]; then echo &quot;You are special&quot;; fi&lt;/tt&gt;: Useless, but shows how you can use shell script.&lt;/li&gt;&lt;/ul&gt;Of course, multiple &quot;|&quot; can appear in the auto response, and commands and regular text can be mixed line by line.&lt;/li&gt;&lt;hr&gt;&lt;p&gt; For more information, see the Licq webpage (&lt;tt&gt;http://www.licq.org&lt;/tt&gt;).&lt;/p&gt;</source>
        <translation>&lt;h2&gt;Sugerencias para configurar&lt;br&gt;su Respuesta Automática&lt;/h2&gt;&lt;br&gt;&lt;hr&gt;&lt;br&gt;&lt;ul&gt;&lt;li&gt;Puede incluir cualquier de las expansiones % (descriptas en la página principal de sugerencias).&lt;/li&gt;&lt;li&gt;Cualquier línea que comience con un pipe (|) será tratada como un comando a ejecutar. Dicha línea será reemplazada con el resultado del comando. El comando es ejecutado por /bin/sh de forma tal que cualquier comando de shell o meta-caracteres están permitidos. Por cuestiones de seguridad cualquiera de las expansiones % son pasadas automáticamente al comando encerradas en comillas simples para prevenir que el shell intente procesar cualquier meta-caracter contenido en un alias&lt;br&gt;Algunos ejemplos de uso popular son:&lt;ul&gt;&lt;li&gt;&lt;tt&gt;|date&lt;/tt&gt;: Reemplaza la línea por la fecha actual&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|fortune&lt;/tt&gt;:Muestra la fortuna, como un tag por ejemplo &lt;/li&gt;&lt;li&gt;&lt;tt&gt;|myscript.sh %u %a&lt;/tt&gt;: Ejecuta un script que recibe el uin y el alias como parámetros&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|myscript.sh %u %a &gt; /dev/null&lt;/tt&gt;: Ejecuta el mismo script, pero ignora la salida (para llevar la cuenta de la veces que se ha verificado la Respuesta Automaticamente, por ejemplo)&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|if [ %u -lt 100000 ]; then echo &quot;You are special&quot;; fi&lt;/tt&gt;: No tiene aplicación práctica, pero demuestra el uso de un shell script.&lt;/li&gt;&lt;/ul&gt;Por supuesto, multiples &quot;|&quot; pueden aparecer en la Respuesta Automática, y comandos y texto regular pueden ser mezclados línea por línea.&lt;/li&gt;&lt;hr&gt;&lt;p&gt; Para más información, vea la página de Licq: (&lt;tt&gt;http://www.licq.org&lt;/tt&gt;).&lt;/p&gt;</translation>
    </message>
    <message>
        <source>&amp;Hints</source>
        <translation>Sugerencias</translation>
    </message>
    <message>
        <source>I&apos;m currently %1, %a.
You can leave me a message.
(%m messages pending from you).</source>
        <translation>En estos momentos estoy %1, %a. 
Puedes dejarme un mensaje (tengo %m mensajes pendientes de ti).</translation>
    </message>
    <message>
        <source>(Closing in %1)</source>
        <translation>(Cerrando en %1)</translation>
    </message>
</context>
<context>
    <name>CEditFileListDlg</name>
    <message>
        <source>Licq - Files to send</source>
        <translation>Licq - Archivos a enviar</translation>
    </message>
    <message>
        <source>&amp;Done</source>
        <translation>Hecho</translation>
    </message>
    <message>
        <source>&amp;Up</source>
        <translation>Arriba</translation>
    </message>
    <message>
        <source>&amp;Down</source>
        <translation>Abajo</translation>
    </message>
    <message>
        <source>&amp;Delete</source>
        <translation>Borrar</translation>
    </message>
</context>
<context>
    <name>CEmoticons</name>
    <message>
        <source>Default</source>
        <translation>Por defecto</translation>
    </message>
    <message>
        <source>None</source>
        <translation>Ninguno</translation>
    </message>
</context>
<context>
    <name>CFileDlg</name>
    <message>
        <source>KB</source>
        <translation>Kb</translation>
    </message>
    <message>
        <source>MB</source>
        <translation>Mb</translation>
    </message>
    <message>
        <source>Byte</source>
        <translation>Byte</translation>
    </message>
    <message>
        <source>ETA:</source>
        <translation>T.Est.:</translation>
    </message>
    <message>
        <source>%1/%2</source>
        <translation>%1/%2</translation>
    </message>
    <message>
        <source>Connecting to remote...</source>
        <translation>Conectando...</translation>
    </message>
    <message>
        <source>Bytes</source>
        <translation>Bytes</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
    <message>
        <source>File:</source>
        <translation>Fichero:</translation>
    </message>
    <message>
        <source>Retry</source>
        <translation>Reintentar</translation>
    </message>
    <message>
        <source>Time:</source>
        <translation>Tiempo:</translation>
    </message>
    <message>
        <source>Licq - File Transfer (%1)</source>
        <translation>Licq - Transferencia de Ficheros (%1)</translation>
    </message>
    <message>
        <source>Waiting for connection...</source>
        <translation>Esperando conexión...</translation>
    </message>
    <message>
        <source>File already exists and appears incomplete.</source>
        <translation>El fichero ya existe y parece incompleto.</translation>
    </message>
    <message>
        <source>Batch:</source>
        <translation>Lote:</translation>
    </message>
    <message>
        <source>File already exists and is at least as big as the incoming file.</source>
        <translation>El fichero ya existe y es al menos tan grande como el fichero que se transmitió</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <source>Resume</source>
        <translation>Continuar</translation>
    </message>
    <message>
        <source>File name:</source>
        <translation>Fichero:</translation>
    </message>
    <message>
        <source>Sending file...</source>
        <translation>Enviar fichero...</translation>
    </message>
    <message>
        <source>Receiving file...</source>
        <translation>Recibiendo fichero...</translation>
    </message>
    <message>
        <source>Current:</source>
        <translation>Actual:</translation>
    </message>
    <message>
        <source>Open error - unable to open file for writing.</source>
        <translation>Error - no se pudo escribir en el fichero.</translation>
    </message>
    <message>
        <source>Overwrite</source>
        <translation>Sobreescribir</translation>
    </message>
    <message>
        <source>&amp;Cancel Transfer</source>
        <translation>&amp;Cancelar Transferencia</translation>
    </message>
    <message>
        <source>File transfer cancelled.</source>
        <translation>Transferencia cancelada</translation>
    </message>
    <message>
        <source>Received %1 from %2 successfully.</source>
        <translation>Se recibió %1 de %2 con éxito</translation>
    </message>
    <message>
        <source>Sent %1 to %2 successfully.</source>
        <translation>Se envió %1 a %2 con éxito</translation>
    </message>
    <message>
        <source>File transfer complete.</source>
        <translation>Transferencia completada</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Remote side disconnected.</source>
        <translation>Se desconectó el lado remoto</translation>
    </message>
    <message>
        <source>File I/O error: %1.</source>
        <translation>Error de E/S: %1</translation>
    </message>
    <message>
        <source>File I/O Error:
%1

See Network Window for details.</source>
        <translation>Error de E/S: 
%1 

Vea la Ventana de Red para más detalles.</translation>
    </message>
    <message>
        <source>Handshaking error.
</source>
        <translation>Error de handshaking</translation>
    </message>
    <message>
        <source>Handshake Error.
See Network Window for details.</source>
        <translation>Error de handshaking. 
Vea la Ventana de Red para más detalles.</translation>
    </message>
    <message>
        <source>Connection error.
</source>
        <translation>Error de conexión.</translation>
    </message>
    <message>
        <source>Unable to reach remote host.
See Network Window for details.</source>
        <translation>No se puede alcanzar el host remoto. 
Vea la Ventana de Red para más detalles.</translation>
    </message>
    <message>
        <source>Bind error.
</source>
        <translation>Error de bind.</translation>
    </message>
    <message>
        <source>Unable to bind to a port.
See Network Window for details.</source>
        <translation>No se puede conectar al puerto (bind). 
Vea la Ventana de Red para más detalles.</translation>
    </message>
    <message>
        <source>Not enough resources.
</source>
        <translation>Recursos insuficientes.</translation>
    </message>
    <message>
        <source>Unable to create a thread.
See Network Window for details.</source>
        <translation>No se puede crear el hilo de ejecución. Vea la Ventana de Red para más detalles.</translation>
    </message>
    <message>
        <source>&amp;Open</source>
        <translation>Abrir</translation>
    </message>
    <message>
        <source>O&amp;pen Dir</source>
        <translation>Abrir directorio.</translation>
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
        <translation>Mensaje</translation>
    </message>
    <message>
        <source>Forward %1 To User</source>
        <translation>Reenviar %1 Al Usuario</translation>
    </message>
    <message>
        <source>Drag the user to forward to here:</source>
        <translation>Arrastre aquí el usuario que desea reenviar:</translation>
    </message>
    <message>
        <source>Unable to forward this message type (%d).</source>
        <translation>No se pudo reenviar este tipo de mensaje (%d).</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Cancelar</translation>
    </message>
    <message>
        <source>Forwarded message:
</source>
        <translation>Mensaje reenviado:
</translation>
    </message>
    <message>
        <source>&amp;Forward</source>
        <translation>Reenviar</translation>
    </message>
    <message>
        <source>Forwarded URL:
</source>
        <translation>URL reenviada:
</translation>
    </message>
</context>
<context>
    <name>CInfoField</name>
    <message>
        <source>Unknown</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>CJoinChatDlg</name>
    <message>
        <source>Select chat to join:</source>
        <translation>Seleccionar charla a la que unirse:</translation>
    </message>
    <message>
        <source>Invite to Join Chat</source>
        <translation>Invitar a Unirse a la Charla</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Cancelar</translation>
    </message>
    <message>
        <source>Select chat to invite:</source>
        <translation>Seleccionar charla a la que invitar:</translation>
    </message>
    <message>
        <source>Join Multiparty Chat</source>
        <translation>Unirse a la Charla Múltiple</translation>
    </message>
    <message>
        <source>&amp;Invite</source>
        <translation>%Invitar</translation>
    </message>
    <message>
        <source>&amp;Join</source>
        <translation>Unirse</translation>
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
        <translation>Siguiente</translation>
    </message>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;Ok</translation>
    </message>
    <message>
        <source>&amp;Clear All</source>
        <translation>Limpiar Todo</translation>
    </message>
    <message>
        <source>&amp;Next (%1)</source>
        <translation>Siguiente (%1)</translation>
    </message>
    <message>
        <source>Licq Information</source>
        <translation>Informatión de Licq</translation>
    </message>
    <message>
        <source>Licq Warning</source>
        <translation>Advertencia de Licq</translation>
    </message>
    <message>
        <source>Licq Critical</source>
        <translation>Mensaje Crítico de Licq</translation>
    </message>
</context>
<context>
    <name>CMMSendDlg</name>
    <message>
        <source>Sending mass message to %1...</source>
        <translation>Enviando mensaje masivo a %1...</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Cerrar</translation>
    </message>
    <message>
        <source>Sending mass URL to %1...</source>
        <translation>Enviando URL masiva a %1...</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>falló</translation>
    </message>
    <message>
        <source>Multiple Recipient URL</source>
        <translation>URL a Múltiples Destinatarios</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Cancelar</translation>
    </message>
    <message>
        <source>Multiple Recipient Message</source>
        <translation>Mensaje a Múltiples Destinatarios</translation>
    </message>
    <message>
        <source>Multiple Recipient Contact List</source>
        <translation>Lista de Contactos a Múltiples Destinatarios.</translation>
    </message>
    <message>
        <source>Sending mass list to %1...</source>
        <translation>Enviando lista masiva a %1</translation>
    </message>
</context>
<context>
    <name>CMMUserView</name>
    <message>
        <source>Crop</source>
        <translation>Recortar</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation>Limpiar</translation>
    </message>
    <message>
        <source>Remove</source>
        <translation>Eliminar</translation>
    </message>
    <message>
        <source>Add Group</source>
        <translation>Añadir Grupo</translation>
    </message>
    <message>
        <source>Add All</source>
        <translation>Añadir Todos</translation>
    </message>
    <message>
        <source>Drag&apos;n&apos;Drop didn&apos;t work</source>
        <translation>No funcionó arrastrar y soltar</translation>
    </message>
</context>
<context>
    <name>CMainWindow</name>
    <message>
        <source>s</source>
        <translation>s</translation>
    </message>
    <message>
        <source>Ok</source>
        <translation>Aceptar</translation>
    </message>
    <message>
        <source>&amp;No</source>
        <translation>&amp;No</translation>
    </message>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;Ok</translation>
    </message>
    <message>
        <source>&amp;Yes</source>
        <translation>Si</translation>
    </message>
    <message>
        <source>Send</source>
        <translation>Enviar</translation>
    </message>
    <message>
        <source>Online to User</source>
        <translation>Conectado/a para este Usuario</translation>
    </message>
    <message>
        <source>&amp;Away</source>
        <translation>&amp;Ausente</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>Ayuda</translation>
    </message>
    <message>
        <source>&amp;Info</source>
        <translation>&amp;Información</translation>
    </message>
    <message>
        <source>U&amp;tilities</source>
        <translation>U&amp;tilidades</translation>
    </message>
    <message>
        <source>E&amp;xit</source>
        <translation>Salir</translation>
    </message>
    <message>
        <source>&amp;Options...</source>
        <translation>&amp;Opciones...</translation>
    </message>
    <message>
        <source>R&amp;andom Chat</source>
        <translation>Charla &amp;Aleatoria</translation>
    </message>
    <message>
        <source>&amp;Random Chat Group</source>
        <translation>Grupo de Charla Aleato&amp;rio</translation>
    </message>
    <message>
        <source>Up since %1
</source>
        <translation>Conectado/a desde %1
</translation>
    </message>
    <message>
        <source>Check %1 Response</source>
        <translation>Comprobar Respuesta %1</translation>
    </message>
    <message>
        <source>Unable to open icons file
%1.</source>
        <translation>No se pudo abrir el fichero de íconos
%1.</translation>
    </message>
    <message>
        <source>Send &amp;Authorization</source>
        <translation>Enviar &amp;Autorización</translation>
    </message>
    <message>
        <source>&amp;Not Available</source>
        <translation>&amp;No Disponible</translation>
    </message>
    <message>
        <source>&amp;About</source>
        <translation>&amp;Acerca de</translation>
    </message>
    <message>
        <source>&amp;Group</source>
        <translation>&amp;Grupos</translation>
    </message>
    <message>
        <source>&amp;Hints</source>
        <translation>Sugerencias</translation>
    </message>
    <message>
        <source>&amp;Reset</source>
        <translation>&amp;Reiniciar</translation>
    </message>
    <message>
        <source>Custom Auto Response...</source>
        <translation>Respuesta Automática Personalizada...</translation>
    </message>
    <message>
        <source>Set &amp;Auto Response...</source>
        <translation>Configurar Respuesta &amp;Automática...</translation>
    </message>
    <message>
        <source>Remove From List</source>
        <translation>Eliminar de la Lista</translation>
    </message>
    <message>
        <source>%1 msg%2</source>
        <translation>%1 msg%2</translation>
    </message>
    <message>
        <source>&amp;Popup All Messages</source>
        <translation>Elevar Todos los Mensajes</translation>
    </message>
    <message>
        <source>Accept in Not Available</source>
        <translation>Aceptar en No Disponible</translation>
    </message>
    <message>
        <source>View &amp;History</source>
        <translation>Ver &amp;Historial</translation>
    </message>
    <message>
        <source>Other Users</source>
        <translation>Otros Usuarios</translation>
    </message>
    <message>
        <source>Edit User Group</source>
        <translation>Editar Pertenencia a Grupos</translation>
    </message>
    <message>
        <source>Unknown Packets</source>
        <translation>Paquetes Desconocidos</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <source>&amp;Mini Mode</source>
        <translation>Modo &amp;Miniatura</translation>
    </message>
    <message>
        <source>Errors</source>
        <translation>Errores</translation>
    </message>
    <message>
        <source>Close &amp;Secure Channel</source>
        <translation>Cerrar Canal &amp;Seguro</translation>
    </message>
    <message>
        <source>No msgs</source>
        <translation>No hay mensajes</translation>
    </message>
    <message>
        <source>Up since %1

</source>
        <translation>Conectado desde %1

</translation>
    </message>
    <message>
        <source>Daemon Statistics

</source>
        <translation>Estadísticas del Demonio

</translation>
    </message>
    <message>
        <source>Daemon Statistics
(Today/Total)
</source>
        <translation>Estadísticas del Demonio
(Hoy/Total)
</translation>
    </message>
    <message>
        <source>SysMsg</source>
        <translation>Mensaje del Sistema</translation>
    </message>
    <message>
        <source>System</source>
        <translation>Sistema</translation>
    </message>
    <message>
        <source>Last reset %1

</source>
        <translation>Última reinicialización %1

</translation>
    </message>
    <message>
        <source>Are you sure you want to remove
%1 (%2)
from the &apos;%3&apos; group?</source>
        <translation>¿Está seguro de que quiere eliminar a
%1 (%2)
del grupo &apos;%3&apos;?</translation>
    </message>
    <message>
        <source>Do you really want to add
%1 (%2)
to your ignore list?</source>
        <translation>¿Realmente quiere añadir a
%1 (%2)
a su lista de ignorados?</translation>
    </message>
    <message>
        <source>Packets</source>
        <translation>Paquetes</translation>
    </message>
    <message>
        <source>&amp;Save All Users</source>
        <translation>Guardar Todo&amp;s los Usuarios</translation>
    </message>
    <message>
        <source>%1: %2 / %3
</source>
        <translation>%1: %2 / %3
</translation>
    </message>
    <message>
        <source>Do Not Disturb to User</source>
        <translation>No Molestar para este Usuario</translation>
    </message>
    <message>
        <source>Update All Users</source>
        <translation>Actualizar Todos los Usuarios</translation>
    </message>
    <message>
        <source>&amp;Redraw User Window</source>
        <translation>&amp;Redibujar Ventana de Usuario</translation>
    </message>
    <message>
        <source>(with KDE support)
</source>
        <translation>(con soporte KDE)
</translation>
    </message>
    <message>
        <source>Occupied to User</source>
        <translation>Ocupado/a para este Usuario</translation>
    </message>
    <message>
        <source>Logon failed.
See network window for details.</source>
        <translation>Falló la Identificación.
Para más detalles, mire en la Ventana de Red.</translation>
    </message>
    <message>
        <source>Accept in Away</source>
        <translation>Aceptar en Ausente</translation>
    </message>
    <message>
        <source>Send &amp;Chat Request</source>
        <translation>Petición de &amp;Charla</translation>
    </message>
    <message>
        <source>Show Offline &amp;Users</source>
        <translation>Mostrar &amp;Usuarios Desconectados/as</translation>
    </message>
    <message>
        <source>%1: %2
</source>
        <translation>%1: %2
</translation>
    </message>
    <message>
        <source>Warnings</source>
        <translation>Advertencias</translation>
    </message>
    <message>
        <source>Debug Level</source>
        <translation>Nivel de Depuración</translation>
    </message>
    <message>
        <source>&amp;Network Window</source>
        <translation>Venta&amp;na de Red</translation>
    </message>
    <message>
        <source>&amp;View Event</source>
        <translation>&amp;Ver Evento(s)</translation>
    </message>
    <message>
        <source>&amp;Plugin Manager...</source>
        <translation>Gestor de A&amp;pliques...</translation>
    </message>
    <message>
        <source>O&amp;ccupied</source>
        <translation>O&amp;cupado</translation>
    </message>
    <message>
        <source>Set All</source>
        <translation>Activar Todos</translation>
    </message>
    <message>
        <source>&amp;View System Messages</source>
        <translation>&amp;Ver Mensajes del Sistema</translation>
    </message>
    <message>
        <source>&amp;Thread Group View</source>
        <translation>Vista de Grupos Jerárquica</translation>
    </message>
    <message>
        <source>Send &amp;Message</source>
        <translation>Enviar &amp;Mensaje</translation>
    </message>
    <message>
        <source>Update Current Group</source>
        <translation>Actualizar Grupo Actual</translation>
    </message>
    <message>
        <source>Licq (%1)</source>
        <translation>Licq (%1)</translation>
    </message>
    <message>
        <source>Right click - Status menu
Double click - Set auto response</source>
        <translation>Botón Derecho - Menú de Estado
Doble Pulsación - Configurar respuesta automática</translation>
    </message>
    <message>
        <source>&amp;Add User</source>
        <translation>&amp;Añadir Usuario</translation>
    </message>
    <message>
        <source>Accept in Do Not Disturb</source>
        <translation>Aceptar en No Molestar</translation>
    </message>
    <message>
        <source>&amp;Statistics</source>
        <translation>E&amp;stadísticas</translation>
    </message>
    <message>
        <source>&amp;Online</source>
        <translation>C&amp;onectado/a</translation>
    </message>
    <message>
        <source>Clear All</source>
        <translation>Limpiar Todos</translation>
    </message>
    <message>
        <source>&amp;Status</source>
        <translation>E&amp;stado</translation>
    </message>
    <message>
        <source>&amp;System</source>
        <translation>&amp;Sistema</translation>
    </message>
    <message>
        <source>Accept in Occupied</source>
        <translation>Aceptar en Ocupado/a</translation>
    </message>
    <message>
        <source>Send Contact &amp;List</source>
        <translation>Enviar &amp;Lista de Contactos</translation>
    </message>
    <message>
        <source>Status Info</source>
        <translation>Información de Estado</translation>
    </message>
    <message>
        <source>&amp;Do Not Disturb</source>
        <translation>No Molestar</translation>
    </message>
    <message>
        <source>Are you sure you want to remove
%1 (%2)
from your contact list?</source>
        <translation>¿Está seguro de que quiere eliminar a
%1 (%2)
de su lista de contactos?</translation>
    </message>
    <message>
        <source>O&amp;ffline</source>
        <translation>Desconectado/a</translation>
    </message>
    <message>
        <source>A&amp;uthorize User</source>
        <translation>A&amp;utorizar Usuario</translation>
    </message>
    <message>
        <source>Free for C&amp;hat</source>
        <translation>Disponible para C&amp;harlar</translation>
    </message>
    <message>
        <source>Right click - User groups
Double click - Show next message</source>
        <translation>Botón Derecho - Grupos de usuarios
Doble Pulsación - Mostrar mensaje siguiente</translation>
    </message>
    <message>
        <source>&amp;Invisible</source>
        <translation>&amp;Invisible</translation>
    </message>
    <message>
        <source>Send &amp;File Transfer</source>
        <translation>Transferencia de &amp;Ficheros</translation>
    </message>
    <message>
        <source>User Functions</source>
        <translation>Funciones de Usuario</translation>
    </message>
    <message>
        <source>Check Auto Response</source>
        <translation>Comprobar Respuesta Automática</translation>
    </message>
    <message>
        <source>System Functions</source>
        <translation>Funciones del Sistema</translation>
    </message>
    <message>
        <source>Edit &amp;Groups</source>
        <translation>Editar &amp;Grupos</translation>
    </message>
    <message>
        <source>S&amp;kin Browser...</source>
        <translation>Gestor de Temas...</translation>
    </message>
    <message>
        <source>Request &amp;Secure Channel</source>
        <translation>Pedir Canal &amp;Seguro</translation>
    </message>
    <message>
        <source>S&amp;earch for User</source>
        <translation>Buscar Usuario</translation>
    </message>
    <message>
        <source>Not Available to User</source>
        <translation>No Disponible para este Usuario</translation>
    </message>
    <message>
        <source>Toggle &amp;Floaty</source>
        <translation>Des/Activar &amp;Flotante</translation>
    </message>
    <message>
        <source>Sa&amp;ve Settings</source>
        <translation>Guardar Configuración</translation>
    </message>
    <message>
        <source>Away to User</source>
        <translation>Ausente para el Usuario</translation>
    </message>
    <message>
        <source>System Message</source>
        <translation>Mensaje del Sistema</translation>
    </message>
    <message>
        <source>%1 message%2</source>
        <translation>%1 mensaje%2</translation>
    </message>
    <message>
        <source>No messages</source>
        <translation>Sin mensajes</translation>
    </message>
    <message>
        <source>Send &amp;URL</source>
        <translation>Enviar &amp;URL</translation>
    </message>
    <message>
        <source>Send Authorization Re&amp;quest</source>
        <translation>Enviar Pedido de Autorización</translation>
    </message>
    <message>
        <source>Send &amp;SMS</source>
        <translation>Enviar &amp;SMS</translation>
    </message>
    <message>
        <source>&amp;Security/Password Options</source>
        <translation>Opciones de &amp;Seguridad/Contraseña</translation>
    </message>
    <message>
        <source>Re&amp;quest Authorization</source>
        <translation>Pedir Autorización</translation>
    </message>
    <message>
        <source>Auto Accept Files</source>
        <translation>Aceptar ficheros automáticamente</translation>
    </message>
    <message>
        <source>Auto Accept Chats</source>
        <translation>Aceptar conversaciones automáticamente</translation>
    </message>
    <message>
        <source>Auto Request Secure</source>
        <translation>Pedir Canal Seguro Automáticamente</translation>
    </message>
    <message>
        <source>Use Real Ip (LAN)</source>
        <translation>Usar IP Real (LAN)</translation>
    </message>
    <message>
        <source>Misc Modes</source>
        <translation>Modos Mezclados</translation>
    </message>
    <message>
        <source>&lt;hr&gt;&lt;p&gt; For more information, see the Licq webpage (&lt;tt&gt;http://www.licq.org&lt;/tt&gt;).&lt;/p&gt;</source>
        <translation>&lt;hr&gt;&lt;p&gt; Para más información, visite la página web de Licq (&lt;tt&gt;http://www.licq.org&lt;/tt&gt;).&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Error! No owner set</source>
        <translation>Error! Dueño no asignado</translation>
    </message>
    <message>
        <source>&lt;b&gt;%1&lt;/b&gt; is online</source>
        <translation>&lt;b&gt;%1&lt;/b&gt; está conectado/a</translation>
    </message>
    <message>
        <source>Server Group</source>
        <translation>Grupo en el Servidor</translation>
    </message>
    <message>
        <source>(Error! No owner set)</source>
        <translation>(Error! Dueño no asignado)</translation>
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
        <translation>Versión de Licq %1%8.
Versión de añadido Qt GUI %2.
Compilado el: %7
%6
Mantenedor: Jon Keating
Contribuciones de Dirk A. Mueller
Autor Original: Graham Roff

http://www.licq.org
#licq en irc.freenode.net

%3 (%4)
%5 contactos.</translation>
    </message>
    <message>
        <source>Unable to open extended icons file
%1.</source>
        <translation>No se puede abrir el fichero de íconos extendidos %1</translation>
    </message>
    <message>
        <source>&amp;GPG Key Manager...</source>
        <translation>Manejador de Claves &amp;GPG</translation>
    </message>
    <message>
        <source>Don&apos;t Show</source>
        <translation>No Mostrar</translation>
    </message>
    <message>
        <source>Available</source>
        <translation>Disponible</translation>
    </message>
    <message>
        <source>Busy</source>
        <translation>Ocupado/a</translation>
    </message>
    <message>
        <source>Phone &quot;Follow Me&quot;</source>
        <translation>Teléfono &quot;Sígame&quot;</translation>
    </message>
    <message>
        <source>&amp;Owner Manager</source>
        <translation>Manejador de Dueños</translation>
    </message>
    <message>
        <source>Use GPG Encryption</source>
        <translation>Usar Encriptación GPG</translation>
    </message>
    <message>
        <source>Update Info Plugin List</source>
        <translation>Actualizar Información de la Lista de Añadidos</translation>
    </message>
    <message>
        <source>Update Status Plugin List</source>
        <translation>Actualizar Lista de Estados de Añadidos</translation>
    </message>
    <message>
        <source>Update Phone &quot;Follow Me&quot; Status</source>
        <translation>Actualizar Estado de Teléfono &quot;Sígame&quot;</translation>
    </message>
    <message>
        <source>Update ICQphone Status</source>
        <translation>Actualizar Estado de ICQPhone</translation>
    </message>
    <message>
        <source>Update File Server Status</source>
        <translation>Actualizar Estado del Servidor de Ficheros</translation>
    </message>
    <message>
        <source>Set GPG key</source>
        <translation>Asginar Clave GPG</translation>
    </message>
    <message>
        <source>Do you really want to reset your stats?</source>
        <translation>Realmente desea volver a cero sus estadísticas?</translation>
    </message>
    <message>
        <source>Licq is unable to find a browser application due to an internal error.</source>
        <translation>Licq no ha podido encontrar un navegador debido a un error interno.</translation>
    </message>
    <message>
        <source>Licq is unable to start your browser and open the URL.
You will need to start the browser and open the URL manually.</source>
        <translation>Licq no ha podido iniciar su navegador y abrir la URL. 
Deberá iniciar el navegador y abrir la URL de forma manual.</translation>
    </message>
    <message>
        <source></source>
        <translation> </translation>
    </message>
    <message>
        <source>&lt;ul&gt;&lt;li&gt;&lt;tt&gt;%a - &lt;/tt&gt;user alias&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%e - &lt;/tt&gt;email&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%f - &lt;/tt&gt;first name&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%h - &lt;/tt&gt;phone number&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%i - &lt;/tt&gt;user ip&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%l - &lt;/tt&gt;last name&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%L - &lt;/tt&gt;local time&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%m - &lt;/tt&gt;# pending messages&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%M - &lt;/tt&gt;# pending messages (if any)&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%n - &lt;/tt&gt;full name&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%o - &lt;/tt&gt;last seen online&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%O - &lt;/tt&gt;online since&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%p - &lt;/tt&gt;user port&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%s - &lt;/tt&gt;full status&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%S - &lt;/tt&gt;abbreviated status&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%u - &lt;/tt&gt;uin&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%w - &lt;/tt&gt;webpage&lt;/li&gt;&lt;/ul&gt;</source>
        <translation>&lt;ul&gt;&lt;li&gt;&lt;tt&gt;%a - &lt;/tt&gt;alias&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%e - &lt;/tt&gt;email&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%f - &lt;/tt&gt;nombre&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%h - &lt;/tt&gt;teléfono&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%i - &lt;/tt&gt;ip&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%l - &lt;/tt&gt;apellido&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%L - &lt;/tt&gt;hora local&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%m - &lt;/tt&gt;# mensajes pendientes&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%M - &lt;/tt&gt;# mensajes pendientes (si hay alguno)&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%n - &lt;/tt&gt;nombre completo&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%o - &lt;/tt&gt;conectado por última vez&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%O - &lt;/tt&gt;conectado desde&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%p - &lt;/tt&gt;puerto del usuario&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%s - &lt;/tt&gt;estado&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%S - &lt;/tt&gt;estado abreviado&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%u - &lt;/tt&gt;uin&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%w - &lt;/tt&gt;paǵina web&lt;/li&gt;&lt;/ul&gt;</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Hints for Using&lt;br&gt;the Licq Qt-GUI Plugin&lt;/h2&gt;&lt;br&gt;&lt;hr&gt;&lt;br&gt;&lt;ul&gt;&lt;li&gt;Change your status by right clicking on the status label.&lt;/li&gt;&lt;li&gt;Change your auto response by double-clicking on the status label.&lt;/li&gt;&lt;li&gt;View system messages by double clicking on the message label.&lt;/li&gt;&lt;li&gt;Change groups by right clicking on the message label.&lt;/li&gt;&lt;li&gt;Use the following shortcuts from the contact list:&lt;ul&gt;&lt;li&gt;&lt;tt&gt;Ctrl-M : &lt;/tt&gt;Toggle mini-mode&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-O : &lt;/tt&gt;Toggle show offline users&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-X : &lt;/tt&gt;Exit&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-H : &lt;/tt&gt;Hide&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-I : &lt;/tt&gt;View the next message&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-V : &lt;/tt&gt;View message&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-S : &lt;/tt&gt;Send message&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-U : &lt;/tt&gt;Send Url&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-C : &lt;/tt&gt;Send chat request&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-F : &lt;/tt&gt;Send File&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-A : &lt;/tt&gt;Check Auto response&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-P : &lt;/tt&gt;Popup all messages&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-L : &lt;/tt&gt;Redraw user window&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Delete : &lt;/tt&gt;Delete user from current group&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-Delete : &lt;/tt&gt;Delete user from contact list&lt;/li&gt;&lt;/ul&gt;&lt;li&gt;Hold control while clicking on close in the function window to remove   the user from your contact list.&lt;/li&gt;&lt;li&gt;Hit Ctrl-Enter from most text entry fields to select &quot;Ok&quot; or &quot;Accept&quot;.   For example in the send tab of the user function window.&lt;/li&gt;&lt;li&gt;Here is the complete list of user % options, which can be used in &lt;b&gt;OnEvent&lt;/b&gt;   parameters, &lt;b&gt;auto responses&lt;/b&gt;, and &lt;b&gt;utilities&lt;/b&gt;:</source>
        <translation>&lt;h2&gt;Sugerencias para usar&lt;br&gt;el Añadido de Licq Qt-GUI &lt;/h2&gt;&lt;br&gt;&lt;hr&gt;&lt;br&gt;&lt;ul&gt;&lt;li&gt;Cambie su estado haciendo click on el botón derecho sobre la etiqueta de estado.&lt;/li&gt;&lt;li&gt;Cambie su respuesta automática haciendo doble click sobre la etiqueta de estado.&lt;/li&gt;&lt;li&gt;Vea los mensajes del sistema haciendo doble click en la etiqueta de mensajes.&lt;/li&gt;&lt;li&gt;Modifique los grupos haciendo click con el botón derecho sobre la etiqueta de mensajes..&lt;/li&gt;&lt;li&gt;Use los siguientes atajos de teclado desde la lista de contactos:&lt;ul&gt;&lt;li&gt;&lt;tt&gt;Ctrl-M : &lt;/tt&gt;Cambiar a modo miniatura&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-O : &lt;/tt&gt;Mostrar usuarios desconectados&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-X : &lt;/tt&gt;Salir&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-H : &lt;/tt&gt;Ocultar&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-I : &lt;/tt&gt;Mostrar el siguiente mensaje&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-V : &lt;/tt&gt;Ver mensaje&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-S : &lt;/tt&gt;Enviar mensaje&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-U : &lt;/tt&gt;Enviar Url&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-C : &lt;/tt&gt;Enviar pedido de charla&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-F : &lt;/tt&gt;Enviar fichero&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-A : &lt;/tt&gt;Ver respuesta automática&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-P : &lt;/tt&gt;Mostrar todos los mensajes en ventana emergente&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-L : &lt;/tt&gt;Redibujar la ventana de usuario&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Suprimir : &lt;/tt&gt;Eliminar Usuario del grupo actual&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-Suprimir : &lt;/tt&gt;Eliminar Usuario de la lista de contactos&lt;/li&gt;&lt;/ul&gt;&lt;li&gt;Mantener presionada la tecla control mientras clickea cerrar en la ventana de funciones para eliminar un contacto de la lista.&lt;/li&gt;&lt;li&gt;Presione Ctrl-Enter en los campos de texto para seleccionar &quot;Ok&quot; o &quot;Aceptar&quot;.   Por ejemplo, en el tab enviar de la ventana de fuciones de usuarios.&lt;/li&gt;&lt;li&gt;Aquí hay un listado completo de las opciones de usuario %, que pueden ser utilizadas en&lt;b&gt;Evento&lt;/b&gt;   parametros, &lt;b&gt;respuestas automáticas&lt;/b&gt;, y &lt;b&gt;utilidades&lt;/b&gt;:</translation>
    </message>
    <message>
        <source> </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>CMessageViewWidget</name>
    <message>
        <source>%1 from %2</source>
        <translation>%1 de %2</translation>
    </message>
    <message>
        <source>%1 to %2</source>
        <translation>%1 a %2</translation>
    </message>
    <message>
        <source>Default</source>
        <translation>Por defecto</translation>
    </message>
    <message>
        <source>Compact</source>
        <translation>Compacto</translation>
    </message>
    <message>
        <source>Tiny</source>
        <translation>Pequeño</translation>
    </message>
    <message>
        <source>Table</source>
        <translation>Tabla</translation>
    </message>
    <message>
        <source>Long</source>
        <translation>Largo</translation>
    </message>
    <message>
        <source>Wide</source>
        <translation>Ancho</translation>
    </message>
</context>
<context>
    <name>CQtLogWindow</name>
    <message>
        <source>&amp;Save</source>
        <translation>Guardar</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Cerrar</translation>
    </message>
    <message>
        <source>Failed to open file:
%1</source>
        <translation>No se pudo abrir el fichero:
%1</translation>
    </message>
    <message>
        <source>C&amp;lear</source>
        <translation>&amp;Limpiar</translation>
    </message>
    <message>
        <source>Licq Network Log</source>
        <translation>Bitácora de Red de Licq</translation>
    </message>
</context>
<context>
    <name>CRandomChatDlg</name>
    <message>
        <source>Games</source>
        <translation>Juegos</translation>
    </message>
    <message>
        <source>20 Something</source>
        <translation>Veintialgo</translation>
    </message>
    <message>
        <source>30 Something</source>
        <translation>Treinta y algo</translation>
    </message>
    <message>
        <source>40 Something</source>
        <translation>Cuarenta y algo</translation>
    </message>
    <message>
        <source>Random chat search timed out.</source>
        <translation>Tiempo de espera excedido durante la búsqueda de charla aleatoria.</translation>
    </message>
    <message>
        <source>Random chat search had an error.</source>
        <translation>Error durante la búsqueda de charla aleatoria.</translation>
    </message>
    <message>
        <source>No random chat user found in that group.</source>
        <translation>No se encontraron usuarios para charlas libres en ese grupo.</translation>
    </message>
    <message>
        <source>50 Plus</source>
        <translation>Más de cincuenta</translation>
    </message>
    <message>
        <source>Romance</source>
        <translation>Romance</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Cancelar</translation>
    </message>
    <message>
        <source>Searching for Random Chat Partner...</source>
        <translation>Buscando Compañero/a para Charla Aleatoria...</translation>
    </message>
    <message>
        <source>&amp;Search</source>
        <translation>Bu&amp;scar</translation>
    </message>
    <message>
        <source>Students</source>
        <translation>Estudiantes</translation>
    </message>
    <message>
        <source>General</source>
        <translation>General</translation>
    </message>
    <message>
        <source>Random Chat Search</source>
        <translation>Búsqueda de Charla Aleatoria</translation>
    </message>
    <message>
        <source>Seeking Women</source>
        <translation>Buscando Mujer</translation>
    </message>
    <message>
        <source>Seeking Men</source>
        <translation>Buscando Hombre</translation>
    </message>
</context>
<context>
    <name>CRefuseDlg</name>
    <message>
        <source>Licq %1 Refusal</source>
        <translation>Rechazo Licq %1</translation>
    </message>
    <message>
        <source>Refusal message for %1 with </source>
        <translation>Mensaje para %1 rechazado con </translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Canclear</translation>
    </message>
    <message>
        <source>Refuse</source>
        <translation>Rechazar</translation>
    </message>
</context>
<context>
    <name>CSetRandomChatGroupDlg</name>
    <message>
        <source>&amp;Set</source>
        <translation>Configurar</translation>
    </message>
    <message>
        <source>done</source>
        <translation>hecho</translation>
    </message>
    <message>
        <source>Games</source>
        <translation>Juegos</translation>
    </message>
    <message>
        <source>error</source>
        <translation>error</translation>
    </message>
    <message>
        <source>20 Something</source>
        <translation>Veintialgo</translation>
    </message>
    <message>
        <source>30 Something</source>
        <translation>Treinta y algo</translation>
    </message>
    <message>
        <source>40 Something</source>
        <translation>Cuarenta y algo</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Cerrar</translation>
    </message>
    <message>
        <source>(none)</source>
        <translation>(ninguno)</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>tiempo máximo excedido</translation>
    </message>
    <message>
        <source>Set Random Chat Group</source>
        <translation>Elegir Grupo de Charla Aleatoria</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>fallo</translation>
    </message>
    <message>
        <source>50 Plus</source>
        <translation>Más de cincuenta</translation>
    </message>
    <message>
        <source>Romance</source>
        <translation>Romance</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Cancelar</translation>
    </message>
    <message>
        <source>Students</source>
        <translation>Estudiantes</translation>
    </message>
    <message>
        <source>General</source>
        <translation>General</translation>
    </message>
    <message>
        <source>Setting Random Chat Group...</source>
        <translation>Configurando Grupo de Charla Aleatoria</translation>
    </message>
    <message>
        <source>Seeking Women</source>
        <translation>Buscando Mujer</translation>
    </message>
    <message>
        <source>Seeking Men</source>
        <translation>Buscando Hombre</translation>
    </message>
</context>
<context>
    <name>CTimeZoneField</name>
    <message>
        <source>Unknown</source>
        <translation>Desconocido</translation>
    </message>
</context>
<context>
    <name>CUserView</name>
    <message>
        <source>S</source>
        <translation>E</translation>
    </message>
    <message>
        <source>Online</source>
        <translation>Conectados/as</translation>
    </message>
    <message>
        <source>Offline</source>
        <translation>Desconectados/as</translation>
    </message>
    <message>
        <source>%1 Floaty (%2)</source>
        <translation>%1 Flotante (%2)</translation>
    </message>
    <message>
        <source>Not In List</source>
        <translation>Fuera de la  Lista</translation>
    </message>
    <message>
        <source> weeks</source>
        <translation> semanas</translation>
    </message>
    <message>
        <source> week</source>
        <translation> semana</translation>
    </message>
    <message>
        <source> days</source>
        <translation> días</translation>
    </message>
    <message>
        <source> day</source>
        <translation> día</translation>
    </message>
    <message>
        <source> hours</source>
        <translation> horas</translation>
    </message>
    <message>
        <source> hour</source>
        <translation> hora</translation>
    </message>
    <message>
        <source> minutes</source>
        <translation> minutos</translation>
    </message>
    <message>
        <source> minute</source>
        <translation> minuto</translation>
    </message>
    <message>
        <source>0 minutes</source>
        <translation>0 minutos</translation>
    </message>
    <message>
        <source>Birthday Today!</source>
        <translation>Cumple años hoy!</translation>
    </message>
    <message>
        <source>Typing a message</source>
        <translation>Escribiendo un mensaje</translation>
    </message>
    <message>
        <source>Phone &amp;quot;Follow Me&amp;quot;: Available</source>
        <translation>Phone &amp;quot;Follow Me&amp;quot;: Disponible</translation>
    </message>
    <message>
        <source>Phone &amp;quot;Follow Me&amp;quot;: Busy</source>
        <translation>Phone &amp;quot;Follow Me&amp;quot;: Ocupado</translation>
    </message>
    <message>
        <source>ICQphone: Available</source>
        <translation>ICQphone: Disponible</translation>
    </message>
    <message>
        <source>ICQphone: Busy</source>
        <translation>ICQphone: Ocupado</translation>
    </message>
    <message>
        <source>File Server: Enabled</source>
        <translation>Servidor de Ficheros: Activo</translation>
    </message>
    <message>
        <source>Secure connection</source>
        <translation>Conexión segura</translation>
    </message>
    <message>
        <source>Custom Auto Response</source>
        <translation>Respuesta Automática Personalizada</translation>
    </message>
    <message>
        <source>Auto Response:</source>
        <translation>Respuesta Automática:</translation>
    </message>
    <message>
        <source>E: </source>
        <translation>E:</translation>
    </message>
    <message>
        <source>P: </source>
        <translation>P:</translation>
    </message>
    <message>
        <source>C: </source>
        <translation>C:</translation>
    </message>
    <message>
        <source>F: </source>
        <translation>F:</translation>
    </message>
    <message>
        <source>Ip: </source>
        <translation>Ip:</translation>
    </message>
    <message>
        <source>O: </source>
        <translation>O:</translation>
    </message>
    <message>
        <source>Logged In: </source>
        <translation>Conectado:</translation>
    </message>
    <message>
        <source>Idle: </source>
        <translation>Sin Actividad:</translation>
    </message>
    <message>
        <source>Local time: </source>
        <translation>Hora local:</translation>
    </message>
    <message>
        <source>ID: </source>
        <translation>ID:</translation>
    </message>
    <message>
        <source>Awaiting authorization</source>
        <translation>Esperando Autorización</translation>
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
        <translation>Ejecuta&amp;r</translation>
    </message>
    <message>
        <source>Done</source>
        <translation>Listo</translation>
    </message>
    <message>
        <source>Done:</source>
        <translation>Hecho:</translation>
    </message>
    <message>
        <source>Edit:</source>
        <translation>Editar:</translation>
    </message>
    <message>
        <source>Edit final command</source>
        <translation>Editar comando final</translation>
    </message>
    <message>
        <source>Command Window</source>
        <translation>Ventana de Comandos</translation>
    </message>
    <message>
        <source>Licq Utility: %1</source>
        <translation>Utilidad de Licq: %1</translation>
    </message>
    <message>
        <source>C&amp;lose</source>
        <translation>Cerrar</translation>
    </message>
    <message>
        <source>User Fields</source>
        <translation>Campos de Usuario</translation>
    </message>
    <message>
        <source>Description:</source>
        <translation>Descripción:</translation>
    </message>
    <message>
        <source>Internal</source>
        <translation>Interno</translation>
    </message>
    <message>
        <source>Command:</source>
        <translation>Comando:</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Cancelar</translation>
    </message>
    <message>
        <source>Running:</source>
        <translation>Ejecutando:</translation>
    </message>
    <message>
        <source>Failed:</source>
        <translation>Falló:</translation>
    </message>
    <message>
        <source>Terminal</source>
        <translation>Terminal</translation>
    </message>
    <message>
        <source>Window:</source>
        <translation>Ventana:</translation>
    </message>
</context>
<context>
    <name>ChatDlg</name>
    <message>
        <source>No</source>
        <translation>No</translation>
    </message>
    <message>
        <source>Yes</source>
        <translation>Si</translation>
    </message>
    <message>
        <source>Beep</source>
        <translation>Pitido</translation>
    </message>
    <message>
        <source>Bold</source>
        <translation>Negrita</translation>
    </message>
    <message>
        <source>Chat</source>
        <translation>Charla</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation>Modo</translation>
    </message>
    <message>
        <source>Toggles Bold font</source>
        <translation>Activa/Desactiva las fuentes en Negrita</translation>
    </message>
    <message>
        <source>Toggles Italic font</source>
        <translation>Activa/Desactiva las fuentes en Itálica</translation>
    </message>
    <message>
        <source>Remote - Waiting for joiners...</source>
        <translation>Remoto - Esperando participantes...</translation>
    </message>
    <message>
        <source>Licq - Chat</source>
        <translation>Licq - Charla</translation>
    </message>
    <message>
        <source>&amp;Audio</source>
        <translation>&amp;Audio</translation>
    </message>
    <message>
        <source>Sends a Beep to all recipients</source>
        <translation>Enviar un Pitido a todos los destinatarios</translation>
    </message>
    <message>
        <source>&amp;Pane Mode</source>
        <translation>Modo &amp;Paneles</translation>
    </message>
    <message>
        <source>Italic</source>
        <translation>Itálica</translation>
    </message>
    <message>
        <source>Do you want to save the chat session?</source>
        <translation>¿Quiere guardar la sesión de charla?</translation>
    </message>
    <message>
        <source>Licq - Chat %1</source>
        <translation>Licq - Charla %1</translation>
    </message>
    <message>
        <source>&amp;IRC Mode</source>
        <translation>Modo &amp;IRC</translation>
    </message>
    <message>
        <source>Changes the foreground color</source>
        <translation>Cambiar el color de primer plano</translation>
    </message>
    <message>
        <source>Background color</source>
        <translation>Color de Fondo</translation>
    </message>
    <message>
        <source>Local - %1</source>
        <translation>Local - %1</translation>
    </message>
    <message>
        <source>Remote - Not connected</source>
        <translation>Remoto - No conectado</translation>
    </message>
    <message>
        <source>&amp;Close Chat</source>
        <translation>&amp;Cerrar Charla</translation>
    </message>
    <message>
        <source>Underline</source>
        <translation>Subrayado</translation>
    </message>
    <message>
        <source>%1 closed connection.</source>
        <translation>%1 cerró la conexión.</translation>
    </message>
    <message>
        <source>Remote - Connecting...</source>
        <translation>Remoto - Conectando...</translation>
    </message>
    <message>
        <source>&amp;Save Chat</source>
        <translation>Guardar Charla</translation>
    </message>
    <message>
        <source>Changes the background color</source>
        <translation>Cambiar el color de fondo</translation>
    </message>
    <message>
        <source>Foreground color</source>
        <translation>Color de Primer Plano</translation>
    </message>
    <message>
        <source>Ignore user settings</source>
        <translation>Ignorar configuración de usuario</translation>
    </message>
    <message>
        <source>Ignores user color settings</source>
        <translation>Ignorar la configuración de color del usuario</translation>
    </message>
    <message>
        <source>Unable to bind to a port.
See Network Window for details.</source>
        <translation>No se puede conectar al puerto. 
Vea la Ventana de Red para más detalles.</translation>
    </message>
    <message>
        <source>Unable to connect to the remote chat.
See Network Window for details.</source>
        <translation>No se puede conectar a la charla remota. 
Vea la Ventana de Red para más detalles.</translation>
    </message>
    <message>
        <source>Unable to create new thread.
See Network Window for details.</source>
        <translation>No se puede crear un nuevo hilo de ejecución. Vea la Ventana de Red para más detalles.</translation>
    </message>
    <message>
        <source>Failed to open file:
%1</source>
        <translation>No se pudo abrir el fichero: 
%1</translation>
    </message>
    <message>
        <source>Unable to load encoding &lt;b&gt;%1&lt;/b&gt;. Message contents may appear garbled.</source>
        <translation>No se pudo cargar la codificación &lt;b&gt;%1&lt;/b&gt;. El contenido del mensaje puede ser ilegible.</translation>
    </message>
    <message>
        <source>Toggles Underline font</source>
        <translation>Des/Activar fuente de Subrayado</translation>
    </message>
    <message>
        <source>StrikeOut</source>
        <translation>Tachado</translation>
    </message>
    <message>
        <source>Toggles StrikeOut font</source>
        <translation>Des/Activar fuente de Tachado</translation>
    </message>
    <message>
        <source>Set Encoding</source>
        <translation>Configurar Codificación</translation>
    </message>
    <message>
        <source>
&lt;--BEEP--&gt;
</source>
        <translation>
&lt;--PITIDO--&gt;</translation>
    </message>
    <message>
        <source>&gt; &lt;--BEEP--&gt;
</source>
        <translation>&gt; &lt;--PITIDO--&gt;</translation>
    </message>
    <message>
        <source>/%1.chat</source>
        <translation>/%1.charla</translation>
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
        <translation>L&amp;impiar</translation>
    </message>
    <message>
        <source>Set Custom Auto Response for %1</source>
        <translation>Configurar Respuesta Automática para %1</translation>
    </message>
    <message>
        <source>I am currently %1.
You can leave me a message.</source>
        <translation>Ahora estoy %1.
Puede dejarme un mensaje.</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Cancelar</translation>
    </message>
    <message>
        <source>&amp;Hints</source>
        <translation>Sugerencias</translation>
    </message>
</context>
<context>
    <name>EditCategoryDlg</name>
    <message>
        <source>Unspecified</source>
        <translation>No especificado</translation>
    </message>
    <message>
        <source>&amp;OK</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Cancelar</translation>
    </message>
</context>
<context>
    <name>EditFileDlg</name>
    <message>
        <source>&amp;Save</source>
        <translation>Guardar</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Cerrar</translation>
    </message>
    <message>
        <source>Failed to open file:
%1</source>
        <translation>Error abriendo fichero:
%1</translation>
    </message>
    <message>
        <source>Licq File Editor - %1</source>
        <translation>Editor de Ficheros Licq - %1</translation>
    </message>
    <message>
        <source>[ Read-Only ]</source>
        <translation>[ Sólo-Lectura ]</translation>
    </message>
</context>
<context>
    <name>EditGrpDlg</name>
    <message>
        <source>Ok</source>
        <translation>Aceptar</translation>
    </message>
    <message>
        <source>Add</source>
        <translation>Añadir</translation>
    </message>
    <message>
        <source>&amp;Done</source>
        <translation>Hecho</translation>
    </message>
    <message>
        <source>Edit Name</source>
        <translation>Editar Nombre</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <source>Groups</source>
        <translation>Grupos</translation>
    </message>
    <message>
        <source>Are you sure you want to remove
the group &apos;%1&apos;?</source>
        <translation>¿Está seguro de que quiere eliminar
el grupo &apos;%1&apos;?</translation>
    </message>
    <message>
        <source>Remove</source>
        <translation>Eliminar</translation>
    </message>
    <message>
        <source>noname</source>
        <translation>sin_nombre</translation>
    </message>
    <message>
        <source>Set Default</source>
        <translation type="obsolete">Valores Por Defecto</translation>
    </message>
    <message>
        <source>Default:</source>
        <translation type="obsolete">Por Defecto:</translation>
    </message>
    <message>
        <source>The default group to start up in.</source>
        <translation type="obsolete">El grupo en el que se empieza por defecto.</translation>
    </message>
    <message>
        <source>Edit group name (hit enter to save).</source>
        <translation>Editar nombre del grupo (pulse ENTER para guardarlo).</translation>
    </message>
    <message>
        <source>Shift Down</source>
        <translation>Mover Abajo</translation>
    </message>
    <message>
        <source>Licq - Edit Groups</source>
        <translation>Licq - Editar Grupos</translation>
    </message>
    <message>
        <source>Shift Up</source>
        <translation>Mover Arriba</translation>
    </message>
    <message>
        <source>Set New Users</source>
        <translation type="obsolete">Configurar Nuevos Usuarios</translation>
    </message>
    <message>
        <source>The group to which new users will be automatically added.  All new users will be in the local system group New Users but for server side storage will also be stored in the specified group.</source>
        <translation type="obsolete">El grupo al cual serán añadidos automáticamente los nuevos usuarios.  Todos los nuevos usuarios pertenecen al grupo del sistema Nuevos Usuarios, pero en el servidor también serán almacenados en el grupo especificado.</translation>
    </message>
    <message>
        <source>New User:</source>
        <translation type="obsolete">Nuevo Usuario:</translation>
    </message>
    <message>
        <source>&amp;Save</source>
        <translation>Guardar</translation>
    </message>
    <message>
        <source>Save the name of a group being modified.</source>
        <translation>Guardar el nombre de un grupo modificado.</translation>
    </message>
</context>
<context>
    <name>EditPhoneDlg</name>
    <message>
        <source>Description:</source>
        <translation>Descripción:</translation>
    </message>
    <message>
        <source>Home Phone</source>
        <translation>Teléfono de Casa.</translation>
    </message>
    <message>
        <source>Work Phone</source>
        <translation>Teléfono del Trabajo</translation>
    </message>
    <message>
        <source>Private Cellular</source>
        <translation>Celular Privado</translation>
    </message>
    <message>
        <source>Work Cellular</source>
        <translation>Celular del Trabajo.</translation>
    </message>
    <message>
        <source>Home Fax</source>
        <translation>Fax de Casa</translation>
    </message>
    <message>
        <source>Work Fax</source>
        <translation>Fax del Trabajo</translation>
    </message>
    <message>
        <source>Wireless Pager</source>
        <translation>Pager Wireless</translation>
    </message>
    <message>
        <source>Type:</source>
        <translation>Tipo:</translation>
    </message>
    <message>
        <source>Phone</source>
        <translation>Teléfono</translation>
    </message>
    <message>
        <source>Cellular</source>
        <translation>Celular</translation>
    </message>
    <message>
        <source>Cellular SMS</source>
        <translation>Celular SMS</translation>
    </message>
    <message>
        <source>Fax</source>
        <translation>Fax</translation>
    </message>
    <message>
        <source>Pager</source>
        <translation>Pager</translation>
    </message>
    <message>
        <source>Country:</source>
        <translation>País:</translation>
    </message>
    <message>
        <source>Network #/Area code:</source>
        <translation>Red #/Código de Área:</translation>
    </message>
    <message>
        <source>Number:</source>
        <translation>Número:</translation>
    </message>
    <message>
        <source>Extension:</source>
        <translation>Extensión:</translation>
    </message>
    <message>
        <source>Provider:</source>
        <translation>Proveedor:</translation>
    </message>
    <message>
        <source>Custom</source>
        <translation>Personalizado</translation>
    </message>
    <message>
        <source>E-mail Gateway:</source>
        <translation>Puerta de Salida de Correo Electrónico:</translation>
    </message>
    <message>
        <source>Remove leading 0s from Area Code/Network #</source>
        <translation>Quitar los 0s antepuestos al Código de Área/Red #</translation>
    </message>
    <message>
        <source>&amp;OK</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Cancelar</translation>
    </message>
    <message>
        <source>@</source>
        <translation>@</translation>
    </message>
    <message>
        <source>Please enter a phone number</source>
        <translation>Por favor ingrese un número telefónico</translation>
    </message>
</context>
<context>
    <name>Event</name>
    <message>
        <source>Unknown Event</source>
        <translation>Evento Desconocido</translation>
    </message>
    <message>
        <source>Plugin Event</source>
        <translation>Evento de Añadidos</translation>
    </message>
    <message>
        <source>Message</source>
        <translation>Mensaje</translation>
    </message>
    <message>
        <source>Chat Request</source>
        <translation>Petición de Charla</translation>
    </message>
    <message>
        <source>File Transfer</source>
        <translation>Transferencia de Ficheros</translation>
    </message>
    <message>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <source>Authorization Request</source>
        <translation>Petición de Autorización</translation>
    </message>
    <message>
        <source>Authorization Refused</source>
        <translation>Autorización Rechazada</translation>
    </message>
    <message>
        <source>Authorization Granted</source>
        <translation>Autorización Concedida</translation>
    </message>
    <message>
        <source>Added to Contact List</source>
        <translation>Añadido a la Lista de Contactos</translation>
    </message>
    <message>
        <source>Web Panel</source>
        <translation>Panel Web</translation>
    </message>
    <message>
        <source>Email Pager</source>
        <translation>Email Pager</translation>
    </message>
    <message>
        <source>Contact List</source>
        <translation>Lista de Contactos</translation>
    </message>
    <message>
        <source>SMS</source>
        <translation>SMS</translation>
    </message>
    <message>
        <source>(cancelled)</source>
        <translation>(cancelado)</translation>
    </message>
    <message>
        <source>System Server Message</source>
        <translation>Mensaje del Servidor del Sistema</translation>
    </message>
    <message>
        <source></source>
        <translation> </translation>
    </message>
</context>
<context>
    <name>GPGKeyManager</name>
    <message>
        <source>Licq GPG Key Manager</source>
        <translation>Manejador de Claves GPG</translation>
    </message>
    <message>
        <source>GPG Passphrase</source>
        <translation>Frase GPG</translation>
    </message>
    <message>
        <source>No passphrase set</source>
        <translation>Configurar sin frase</translation>
    </message>
    <message>
        <source>&amp;Set GPG Passphrase</source>
        <translation>Configurar Fra&amp;se GPG</translation>
    </message>
    <message>
        <source>User Keys</source>
        <translation>Teclas del Usuario</translation>
    </message>
    <message>
        <source>&amp;Add</source>
        <translation>&amp;Agregar</translation>
    </message>
    <message>
        <source>&amp;Edit</source>
        <translation>&amp;Editar</translation>
    </message>
    <message>
        <source>&amp;Remove</source>
        <translation>Quita&amp;r</translation>
    </message>
    <message>
        <source>&lt;qt&gt;Drag&amp;Drop user to add to list.&lt;/qt&gt;</source>
        <translation>&lt;qt&gt;Arrastre y suelte el usuario para aña&amp;dirlo a la lista.&lt;/qt&gt;</translation>
    </message>
    <message>
        <source>User</source>
        <translation>Usuario</translation>
    </message>
    <message>
        <source>Active</source>
        <translation>Activo</translation>
    </message>
    <message>
        <source>Key ID</source>
        <translation>Clave de ID</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Cerrar</translation>
    </message>
    <message>
        <source>Set Passphrase</source>
        <translation>Configurar Frase</translation>
    </message>
    <message>
        <source>Not yet implemented. Use licq_gpg.conf.</source>
        <translation>Todavía no implementado. Use licq_gpg.conf.</translation>
    </message>
    <message>
        <source>Remove GPG key</source>
        <translation>Borrar Clave GPG</translation>
    </message>
    <message>
        <source>Do you want to remove the GPG key? The key isn&apos;t deleted from your keyring.</source>
        <translation>Desea borrar la clave GPG? La clave no es borrada de su anillo de claves.</translation>
    </message>
</context>
<context>
    <name>GPGKeySelect</name>
    <message>
        <source>Select GPG Key for user %1</source>
        <translation>Seleccione Clave GPG para el usuario %1</translation>
    </message>
    <message>
        <source>Select a GPG key for user %1.</source>
        <translation>Seleccione una Clave GPG para el usuario %1</translation>
    </message>
    <message>
        <source>Current key: No key selected</source>
        <translation>Clave actual: Clave no seleccionada</translation>
    </message>
    <message>
        <source>Current key: %1</source>
        <translation>Clave actual: %1</translation>
    </message>
    <message>
        <source>Use GPG Encryption</source>
        <translation>Usar Encriptación GPG</translation>
    </message>
    <message>
        <source>Filter:</source>
        <translation>Filtro:</translation>
    </message>
    <message>
        <source>&amp;OK</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>&amp;No Key</source>
        <translation>Si&amp;n Clave</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Cancelar</translation>
    </message>
</context>
<context>
    <name>Groups</name>
    <message>
        <source>All Users</source>
        <translation>Todos los usuarios</translation>
    </message>
    <message>
        <source>Online Notify</source>
        <translation>Aviso de Conexión</translation>
    </message>
    <message>
        <source>Visible List</source>
        <translation>Lista Visible</translation>
    </message>
    <message>
        <source>Invisible List</source>
        <translation>Lista Invisible</translation>
    </message>
    <message>
        <source>Ignore List</source>
        <translation>Lista de Ignorados</translation>
    </message>
    <message>
        <source>New Users</source>
        <translation>Nuevos Usuarios</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation>Desconocido</translation>
    </message>
</context>
<context>
    <name>HintsDlg</name>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Cerrar</translation>
    </message>
    <message>
        <source>Licq - Hints</source>
        <translation>Licq - Sugerencias</translation>
    </message>
</context>
<context>
    <name>IconManager_KDEStyle</name>
    <message>
        <source>&lt;br&gt;&lt;b&gt;%1 system messages&lt;/b&gt;</source>
        <translation>&lt;br&gt;&lt;b&gt;%1 mensajes del sistema&lt;/b&gt;</translation>
    </message>
    <message>
        <source>&lt;br&gt;%1 msgs</source>
        <translation>&lt;br&gt;%1 mensajes</translation>
    </message>
    <message>
        <source>&lt;br&gt;1 msg</source>
        <translation>&lt;br&gt;1 mensaje</translation>
    </message>
    <message>
        <source>&lt;br&gt;Left click - Show main window&lt;br&gt;Middle click - Show next message&lt;br&gt;Right click - System menu</source>
        <translation>&lt;br&gt;Click Izquierdo- Mostrar Ventana Principal&lt;br&gt;Click Medio- Mostrar Mensaje Siguiente&lt;br&gt;Click Derecho- Menú del Sistema</translation>
    </message>
</context>
<context>
    <name>IconManager_Themed</name>
    <message>
        <source>Unable to load dock theme image
%1</source>
        <translation>No se pudo cargar la imagen para el ícono
%1</translation>
    </message>
    <message>
        <source>Unable to load dock theme file
(%1)
:%2</source>
        <translation>No se pudo cargar el fichero de tema para el ícono
(%1)
:%2</translation>
    </message>
</context>
<context>
    <name>KeyListItem</name>
    <message>
        <source>Yes</source>
        <translation>Si</translation>
    </message>
    <message>
        <source>No</source>
        <translation>No</translation>
    </message>
</context>
<context>
    <name>KeyRequestDlg</name>
    <message>
        <source>&amp;Send</source>
        <translation>Enviar</translation>
    </message>
    <message>
        <source>Closing secure channel...</source>
        <translation>Cerrando canal seguro...</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Cerrar</translation>
    </message>
    <message>
        <source>Client does not support OpenSSL.
Rebuild Licq with OpenSSL support.</source>
        <translation>El cliente no soporta OpenSSL.
Vuelva a compilar Licq con soporte para OpenSSL.</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;blue&quot;&gt;Secure channel closed.&lt;/font&gt;
</source>
        <translation>&lt;font color=&quot;blue&quot;&gt;Canal seguro cerrado.&lt;/font&gt;
</translation>
    </message>
    <message>
        <source>Licq - Secure Channel with %1</source>
        <translation>Licq - Canal Seguro con %1</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;Could not connect to remote client.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;No se pudo conectar con el cliente remoto.&lt;/font&gt;</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;yellow&quot;&gt;Secure channel already established.&lt;/font&gt;
</source>
        <translation>&lt;font color=&quot;yellow&quot;&gt;Ya hay establecido un canal seguro.&lt;/font&gt;
</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;yellow&quot;&gt;Secure channel not established.&lt;/font&gt;
</source>
        <translation>&lt;font color=&quot;yellow&quot;&gt;Canal seguro no establecido.&lt;/font&gt;
</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;Remote client does not support OpenSSL.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;El cliente remoto no soporta OpenSSL.&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Requesting secure channel...</source>
        <translation>Pidiendo canal seguro...</translation>
    </message>
    <message>
        <source>Secure channel is established using SSL
with Diffie-Hellman key exchange and
the TLS version 1 protocol.

</source>
        <translation>El canal seguro está establecido usando SSL con intercambio de claves Diffie-Hellman y protocolo TLS versión 1.</translation>
    </message>
    <message>
        <source>The remote uses Licq %1/SSL.</source>
        <translation>El lado remoto usa Licq %1/SSL</translation>
    </message>
    <message>
        <source>The remote uses Licq %1, however it
has no secure channel support compiled in.
This probably won&apos;t work.</source>
        <translation>El lado remoto usa Licq %1, sin embargo no tiene habilitado el soporte para canales seguros. 
Esto probablemente no funcione.</translation>
    </message>
    <message>
        <source>This only works with other Licq clients &gt;= v0.85
The remote doesn&apos;t seem to use such a client.
This might not work.</source>
        <translation>Esto sólo funciona con otros clientes Licq &lt;= v0.85 
El lado remoto no parece estar utilizando dicho cliente. 
Podría no funcionar.</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;ForestGreen&quot;&gt;Secure channel established.&lt;/font&gt;
</source>
        <translation>&lt;font color=&quot;ForestGreen&quot;&gt;Canal Seguro Establecido&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Ready to close channel</source>
        <translation>Preparado para cerrar canal.</translation>
    </message>
    <message>
        <source>Ready to request channel</source>
        <translation>Preparado para pedir canal</translation>
    </message>
</context>
<context>
    <name>KeyView</name>
    <message>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
</context>
<context>
    <name>MLEditWrap</name>
    <message>
        <source>Allow Tabulations</source>
        <translation>Permitir Tabulaciones</translation>
    </message>
</context>
<context>
    <name>MLView</name>
    <message>
        <source>Copy URL</source>
        <translation>Copiar URL</translation>
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
        <translation>Hora</translation>
    </message>
    <message>
        <source>Event Type</source>
        <translation>Tipo de Evento</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>Opciones</translation>
    </message>
    <message>
        <source>Direct</source>
        <translation>Directo</translation>
    </message>
    <message>
        <source>Server</source>
        <translation>Servidor</translation>
    </message>
    <message>
        <source>Urgent</source>
        <translation>Urgente</translation>
    </message>
    <message>
        <source>Multiple Recipients</source>
        <translation>Múltiples Destinatarios</translation>
    </message>
    <message>
        <source>Cancelled Event</source>
        <translation>Evento Cancelado</translation>
    </message>
</context>
<context>
    <name>OptionsDlg</name>
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
        <translation>Ausente</translation>
    </message>
    <message>
        <source>Font</source>
        <translation>Fuente</translation>
    </message>
    <message>
        <source>Left</source>
        <translation>Izquierda</translation>
    </message>
    <message>
        <source>Save</source>
        <translation>Guardar</translation>
    </message>
    <message>
        <source>URL:</source>
        <translation>URL:</translation>
    </message>
    <message>
        <source>none</source>
        <translation>Ninguno</translation>
    </message>
    <message>
        <source>Show Extended Icons</source>
        <translation>Mostrar Íconos Extendidos</translation>
    </message>
    <message>
        <source>Font:</source>
        <translation>Fuente:</translation>
    </message>
    <message>
        <source>Never</source>
        <translation>Nunca</translation>
    </message>
    <message>
        <source>Right</source>
        <translation>Derecha</translation>
    </message>
    <message>
        <source>Text:</source>
        <translation>Texto:</translation>
    </message>
    <message>
        <source>Title</source>
        <translation>Título</translation>
    </message>
    <message>
        <source>Width</source>
        <translation>Ancho</translation>
    </message>
    <message>
        <source>Parameter for received file transfers</source>
        <translation>Parámetros para transferencias de ficheros recibidas</translation>
    </message>
    <message>
        <source>Firewall</source>
        <translation>Cortafuegos</translation>
    </message>
    <message>
        <source>Show Grid Lines</source>
        <translation>Muestra Líneas de Rejilla</translation>
    </message>
    <message>
        <source>Override the skin setting for the frame style of the user window:
   0 (No frame), 1 (Box), 2 (Panel), 3 (WinPanel)
 + 16 (Plain), 32 (Raised), 48 (Sunken)
 + 240 (Shadow)</source>
        <translation>Utilizar este estilo para el marco de la ventana de usuario en lugar del predeterminado por el tema:
   0 (Sin marco), 1 (Caja), 2 (Panel), 3 (WinPanel)
 + 16 (Liso), 32 (Elevado), 48 (Hundido)
 + 240 (Sombreado)</translation>
    </message>
    <message>
        <source>Parameter for received chat requests</source>
        <translation>Parámetros para charlas recibidas</translation>
    </message>
    <message>
        <source>Turns on or off the display of headers above each column in the user list</source>
        <translation>Muestra u oculta las cabeceras de las columnas en la lista de contactos</translation>
    </message>
    <message>
        <source>Auto Away:</source>
        <translation>Modo &quot;Ausente&quot; Automático en (minutos):</translation>
    </message>
    <message>
        <source>Number of minutes of inactivity after which to automatically be marked &quot;not available&quot;.  Set to &quot;0&quot; to disable.</source>
        <translation>Número de minutos de inactividad después de los cuales se le marcará automáticamente como &quot;No Disponible&quot;.  Póngalo a &quot;0&quot; para desactivar esta función.</translation>
    </message>
    <message>
        <source>Ignore Mass Messages</source>
        <translation>Ignorar Mensajes Masivos</translation>
    </message>
    <message>
        <source>Show the &quot;--online--&quot; and &quot;--offline--&quot; bars in the contact list</source>
        <translation>Muestra en la lista de contactos las barras &quot;--conectado--&quot; y &quot;--desconectado--&quot;</translation>
    </message>
    <message>
        <source>Parameter for online notification</source>
        <translation>Parámetros para notificaciones de conexión</translation>
    </message>
    <message>
        <source>TCP port range for incoming connections.</source>
        <translation>Rango de puertos TCP para aceptar conexiones.</translation>
    </message>
    <message>
        <source>Accept Modes</source>
        <translation>Tratamiento según estado</translation>
    </message>
    <message>
        <source>Online Notify:</source>
        <translation>Aviso de Conexión:</translation>
    </message>
    <message>
        <source>Not Available</source>
        <translation>No Disponible</translation>
    </message>
    <message>
        <source>&amp;Apply</source>
        <translation>&amp;Aplicar</translation>
    </message>
    <message>
        <source>Determines if new users are automatically added to your list or must first request authorization.</source>
        <translation>Determina si los nuevos usuarios se añaden automáticamente a la lista o si se debe pedir autorización primero.</translation>
    </message>
    <message>
        <source>Show the name of the current group in the messages label when there are no new messages</source>
        <translation>Mostrar el nombre del grupo actual en la etiqueta de mensajes cuando no hayan mensajes</translation>
    </message>
    <message>
        <source>Ignore Web Panel</source>
        <translation>Ignorar Panel Web</translation>
    </message>
    <message>
        <source>Use Font Styles</source>
        <translation>Usar Estilos de Fuente</translation>
    </message>
    <message>
        <source>Preset slot:</source>
        <translation>Preconfigurar casilla:</translation>
    </message>
    <message>
        <source>Licq Options</source>
        <translation>Opciones Licq</translation>
    </message>
    <message>
        <source>Show online notify users who are offline even when offline users are hidden.</source>
        <translation>Mostrar los usuarios con aviso de conexión que estén desconectados incluso cuando no se muestren los usuarios desconectados.</translation>
    </message>
    <message>
        <source>Show Column Headers</source>
        <translation>Muestra Cabeceras de las Columnas</translation>
    </message>
    <message>
        <source>Bold Message Label on Incoming Msg</source>
        <translation>Etiqueta en negrita cuando lleguen mensajes</translation>
    </message>
    <message>
        <source>Column %1</source>
        <translation>Columna %1</translation>
    </message>
    <message>
        <source>Selects between the standard 64x64 icon used in the WindowMaker/Afterstep wharf and a shorter 64x48 icon for use in the Gnome/KDE panel.</source>
        <translation>Elige entre el icono 64x64 estandard usado en los paneles de WindowMaker/Afterstep u otro 64x48 más pequeño para Gnome/KDE.</translation>
    </message>
    <message>
        <source>Perform OnEvent command in occupied mode</source>
        <translation>Activar Tratamiento de Eventos en modo Ocupado</translation>
    </message>
    <message>
        <source>Auto Offline:</source>
        <translation>Auto Desconexión:</translation>
    </message>
    <message>
        <source>OnEvent in DND</source>
        <translation>Eventos en No Molestar</translation>
    </message>
    <message>
        <source>OnEvent in N/A</source>
        <translation>Eventos en No Disponible</translation>
    </message>
    <message>
        <source>Parameter for received messages</source>
        <translation>Parámetros para mensajes recibidos</translation>
    </message>
    <message>
        <source>Center</source>
        <translation>Centro</translation>
    </message>
    <message>
        <source>Allow scroll bar</source>
        <translation>Permitir barra de desplazamiento</translation>
    </message>
    <message>
        <source>OnEvent in Away</source>
        <translation>Eventos en Ausente</translation>
    </message>
    <message>
        <source>Network</source>
        <translation>Red</translation>
    </message>
    <message>
        <source>Format</source>
        <translation>Formato</translation>
    </message>
    <message>
        <source>The fonts used</source>
        <translation>Las fuentes utilizadas</translation>
    </message>
    <message>
        <source>Number of minutes of inactivity after which to automatically go offline.  Set to &quot;0&quot; to disable.</source>
        <translation>Número de minutos de inactividad después de los cuales se le pondrá como &quot;desconectado&quot; automáticamente. Póngalo a &quot;0&quot; para inhabilitarlo.</translation>
    </message>
    <message>
        <source>Use Dock Icon</source>
        <translation>Usar Ícono</translation>
    </message>
    <message>
        <source>Chat Request:</source>
        <translation>Petición de Charla:</translation>
    </message>
    <message>
        <source>Online</source>
        <translation>Conectado/a</translation>
    </message>
    <message>
        <source>Server settings</source>
        <translation>Configuración del Servidor</translation>
    </message>
    <message>
        <source>Auto Logon:</source>
        <translation>Estado Conexión Inicial:</translation>
    </message>
    <message>
        <source>Main Window</source>
        <translation>Ventana Principal</translation>
    </message>
    <message>
        <source>Status</source>
        <translation>Estado</translation>
    </message>
    <message>
        <source>Theme:</source>
        <translation>Tema:</translation>
    </message>
    <message>
        <source>Always show online notify users</source>
        <translation>Mostrar siempre notificación de conexión</translation>
    </message>
    <message>
        <source>default (%1)</source>
        <translation>por defecto (%1)</translation>
    </message>
    <message>
        <source>Offline</source>
        <translation>Desconectado/a</translation>
    </message>
    <message>
        <source>Port Range:</source>
        <translation>Rango de Puertos:</translation>
    </message>
    <message>
        <source>OnEvent</source>
        <translation>Eventos</translation>
    </message>
    <message>
        <source>Command:</source>
        <translation>Comando:</translation>
    </message>
    <message>
        <source>Edit Font:</source>
        <translation>Fuente Edición:</translation>
    </message>
    <message>
        <source>I can receive direct connections</source>
        <translation>Usted puede recibir conexiones directas</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>Opciones</translation>
    </message>
    <message>
        <source>Miscellaneous</source>
        <translation>Miscelánea</translation>
    </message>
    <message>
        <source>Perform OnEvent command in do not disturb mode</source>
        <translation>Activar Tratamiento de Eventos en modo No Molestar</translation>
    </message>
    <message>
        <source>Determines if mass messages are ignored or not.</source>
        <translation>Determina si se ignoran o no los mensajes de masas.</translation>
    </message>
    <message>
        <source>Frame Style: </source>
        <translation>Estilo del Marco: </translation>
    </message>
    <message>
        <source>The alignment of the column</source>
        <translation>Alineamiento de las columnas</translation>
    </message>
    <message>
        <source>Make the user window transparent when there is no scroll bar</source>
        <translation>Hacer transparente la ventana de usuario cuando no haya barra de desplazamiento</translation>
    </message>
    <message>
        <source>Default Icon</source>
        <translation>Icono Por Defecto</translation>
    </message>
    <message>
        <source>Parameters</source>
        <translation>Parámetros</translation>
    </message>
    <message>
        <source>Show birthday, invisible, and custom auto response icons to the right of users in the list</source>
        <translation>Mostrar íconos para la fecha de nacimiento, invisible y respuesta automática a la derecha de los usuarios en la lista</translation>
    </message>
    <message>
        <source>Transparent when possible</source>
        <translation>Transparente cuando sea posible</translation>
    </message>
    <message>
        <source>Default Auto Response Messages</source>
        <translation>Mensajes por Defecto para Respuesta Automática</translation>
    </message>
    <message>
        <source>File Transfer:</source>
        <translation>Transferencia de Ficheros:</translation>
    </message>
    <message>
        <source>OnEvents Enabled</source>
        <translation>Activar Tratamiento de Eventos</translation>
    </message>
    <message>
        <source>Paranoia</source>
        <translation>Paranoia</translation>
    </message>
    <message>
        <source>Auto-Raise on Incoming Msg</source>
        <translation>Auto-Elevar Mensajes Entrantes</translation>
    </message>
    <message>
        <source>Terminal:</source>
        <translation>Terminal:</translation>
    </message>
    <message>
        <source>Automatically log on when first starting up.</source>
        <translation>Estado en el que se le marcará nada más conectar.</translation>
    </message>
    <message>
        <source>Number of minutes of inactivity after which to automatically be marked &quot;away&quot;.  Set to &quot;0&quot; to disable.</source>
        <translation>Número de minutos de inactividad después de los cuales se le marcará automáticamente como &quot;Ausente&quot;.  Póngalo a &quot;0&quot; para desactivar esta función.</translation>
    </message>
    <message>
        <source>&lt;p&gt;Command to execute when an event is received.&lt;br&gt;It will be passed the relevant parameters from below.&lt;br&gt;Parameters can contain the following expressions &lt;br&gt; which will be replaced with the relevant information:&lt;/p&gt;</source>
        <translation>&lt;p&gt;Comando a ejecutar cuando se recibe un evento.&lt;br&gt;Se le pasarán los parámetros indicados de la lista de abajo.&lt;br&gt;Los parámetros pueden contener las siguientes expresiones&lt;br&gt;que se reemplazarán por la información correspondiente:&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Determines if email pager messages are ignored or not.</source>
        <translation>Determina si se ignoran o no los mensajes del EMail Pager.</translation>
    </message>
    <message>
        <source>Perform OnEvent command in not available mode</source>
        <translation>Activar Tratamiento de Eventos en modo No Disponible</translation>
    </message>
    <message>
        <source>Occupied</source>
        <translation>Ocupado/a</translation>
    </message>
    <message>
        <source>Show User Dividers</source>
        <translation>Muestra División de Usuarios</translation>
    </message>
    <message>
        <source>OnEvent in Occupied</source>
        <translation>Eventos en Ocupado</translation>
    </message>
    <message>
        <source>Parameter for received system messages</source>
        <translation>Parámetros para mensajes del sistema recibidos</translation>
    </message>
    <message>
        <source>Manual &quot;New User&quot; group handling</source>
        <translation>Gestión Manual del Grupo &quot;Usuarios Nuevos&quot;</translation>
    </message>
    <message>
        <source>Select Font</source>
        <translation>Seleccionar</translation>
    </message>
    <message>
        <source>Auto-Popup Incoming Msg</source>
        <translation>Auto-Abrir Mensajes Entrantes</translation>
    </message>
    <message>
        <source>64 x 48 Dock Icon</source>
        <translation>Icono 64 x 64</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Cancelar</translation>
    </message>
    <message>
        <source>Startup</source>
        <translation>Inicialización</translation>
    </message>
    <message>
        <source>Status:</source>
        <translation>Estado:</translation>
    </message>
    <message>
        <source>default</source>
        <translation>por defecto</translation>
    </message>
    <message>
        <source>Use italics and bold in the user list to indicate special characteristics such as online notify and visible list</source>
        <translation>Usa fuentes en itálica y en negrita en la lista de contactos para indicar características tales como si está activado el aviso de conexión para un usuario o si pertenece a la lista de visibles</translation>
    </message>
    <message>
        <source>Auto N/A:</source>
        <translation>Modo &quot;No Molestar&quot; Automático en (minutos):</translation>
    </message>
    <message>
        <source>Themed Icon</source>
        <translation>Icono con Temas</translation>
    </message>
    <message>
        <source>Auto Close Function Window</source>
        <translation>Cerrar Ventana de Funciones Automáticamente</translation>
    </message>
    <message>
        <source>Perform OnEvent command in away mode</source>
        <translation>Activar Tratamiento de Eventos en modo Ausente</translation>
    </message>
    <message>
        <source>Docking</source>
        <translation>Ícono de Panel</translation>
    </message>
    <message>
        <source>Determines if web panel messages are ignored or not.</source>
        <translation>Determina si se ignoran o no los mensajes del panel web.</translation>
    </message>
    <message>
        <source>Enable running of &quot;Command&quot; when the relevant event occurs.</source>
        <translation>Activar la ejecución de &quot;Comando&quot; cuando ocurran los eventos programados.</translation>
    </message>
    <message>
        <source>System Msg:</source>
        <translation>Mensaje del Sistema:</translation>
    </message>
    <message>
        <source>Show group name if no messages</source>
        <translation>Mostrar nombre del grupo si no hay mensajes</translation>
    </message>
    <message>
        <source>Free for Chat</source>
        <translation>Disponible Para Charlar </translation>
    </message>
    <message>
        <source>Free For Chat</source>
        <translation>Disponible Para Charlar</translation>
    </message>
    <message>
        <source>Online Notify when Logging On</source>
        <translation>Aviso de Conexión</translation>
    </message>
    <message>
        <source>Extensions</source>
        <translation>Extensiones</translation>
    </message>
    <message>
        <source>Do Not Disturb</source>
        <translation>No Molestar</translation>
    </message>
    <message>
        <source>Url Viewer:</source>
        <translation>Visor URLs:</translation>
    </message>
    <message>
        <source>Message:</source>
        <translation>Mensaje:</translation>
    </message>
    <message>
        <source>Draw boxes around each square in the user list</source>
        <translation>Dibujar líneas alrededor de cada recuadro de la lista de usuarios</translation>
    </message>
    <message>
        <source>Column Configuration</source>
        <translation>Columnas</translation>
    </message>
    <message>
        <source>Invisible</source>
        <translation>Invisible</translation>
    </message>
    <message>
        <source>General</source>
        <translation>General</translation>
    </message>
    <message>
        <source>Ignore Email Pager</source>
        <translation>Ignorar EMail Pager</translation>
    </message>
    <message>
        <source>Controls whether or not the dockable icon should be displayed.</source>
        <translation>Controla si se muestra o no el ícono acoplable a los paneles.</translation>
    </message>
    <message>
        <source>Contact List</source>
        <translation>Lista de Contactos</translation>
    </message>
    <message>
        <source>The width of the column</source>
        <translation>Ancho de la columna</translation>
    </message>
    <message>
        <source>Select a font from the system list</source>
        <translation>Seleccione una fuente de la lista del sistema</translation>
    </message>
    <message>
        <source>The string which will appear in the list box column header</source>
        <translation>La cadena que aparecerá en las cabeceras de las columnas</translation>
    </message>
    <message>
        <source>Parameter for received URLs</source>
        <translation>Parámetros para URLs recibidas</translation>
    </message>
    <message>
        <source>The command to run to view a URL.  Will be passed the URL as a parameter.</source>
        <translation>El comando a ejecutar para ver una URL.  Se le pasará la URL como parámetro.</translation>
    </message>
    <message>
        <source>Allow the vertical scroll bar in the user list</source>
        <translation>Permitir la aparición de la barra de desplazamiento vertical en la lista de usuarios</translation>
    </message>
    <message>
        <source>The command to run to start your terminal program.</source>
        <translation>Comando a ejecutar para lanzar su programa de terminal.</translation>
    </message>
    <message>
        <source>Font used in message editor etc.</source>
        <translation>Fuente utilizada en el editor de mensajes, etc.</translation>
    </message>
    <message>
        <source>Ignore New Users</source>
        <translation>Ignorar Nuevos Usuarios</translation>
    </message>
    <message>
        <source>Alignment</source>
        <translation>Alineamiento</translation>
    </message>
    <message>
        <source>Auto close the user function window after a succesful event</source>
        <translation>Cerrar automáticamente la ventana de funciones de usuario luego de un evento exitoso.</translation>
    </message>
    <message>
        <source>Open all incoming messages automatically when received if we are online (or free for chat)</source>
        <translation>Abrir todos los mensajes automáticamente cuando se reciben si estoy conectado/a (O libre para charlar)</translation>
    </message>
    <message>
        <source>Raise the main window on incoming messages</source>
        <translation>Levantar la ventana principal cuando se recibe un nuevo mensaje</translation>
    </message>
    <message>
        <source>Start Hidden</source>
        <translation>Comenzar Oculto</translation>
    </message>
    <message>
        <source>Start main window hidden. Only the dock icon will be visible.</source>
        <translation>Iniciar la ventana principal oculta. Sólo el ícono dock será visible.</translation>
    </message>
    <message>
        <source>Show the message info label in bold font if there are incoming messages</source>
        <translation>Mostrar la etiqueta de información de mensaje en negrita si no hay mensajes entrantes.</translation>
    </message>
    <message>
        <source>If not checked, a user will be automatically removed from &quot;New User&quot; group when you first send an event to them</source>
        <translation>Si no está seleccionado, los usuarios serán automáticamente quitados del grupo &quot;Nuevos Usuarios&quot; cuando le envíes un evento por primera vez</translation>
    </message>
    <message>
        <source>Check Clipboard For Urls/Files</source>
        <translation>Verificar el Protapapeles en busca de Urls/Ficheros</translation>
    </message>
    <message>
        <source>When double-clicking on a user to send a message check for urls/files in the clipboard</source>
        <translation>Cuando se hace doble-click sobre un usuario para enviarle un mensaje, buscar urls/ficheros en el portapapeles</translation>
    </message>
    <message>
        <source>Auto Position the Reply Window</source>
        <translation>Posicionar Automáticamente la Ventana de Respuesta</translation>
    </message>
    <message>
        <source>Position a new reply window just underneath the message view window</source>
        <translation>Posicionar la ventana de respuesta justo debajo de la ventana de vista de mensaje</translation>
    </message>
    <message>
        <source>Auto send through server</source>
        <translation>Enviar automáticamente a través del servidor</translation>
    </message>
    <message>
        <source>Automatically send messages through the server if direct connection fails</source>
        <translation>Enviar mensajes automáticamente a través del servidor si falla la conexión directa.</translation>
    </message>
    <message>
        <source>Allow dragging main window</source>
        <translation>Permitir que se arraste la ventana principal</translation>
    </message>
    <message>
        <source>Lets you drag around the main window with your mouse</source>
        <translation>Le permite arrastrar la ventana principal con el ratón</translation>
    </message>
    <message>
        <source>Chatmode Messageview</source>
        <translation>Vista de mensaje en modo Charla</translation>
    </message>
    <message>
        <source>Show the current chat history in Send Window</source>
        <translation>Mostrar el historial de la conversación actual en la Ventana de Envío</translation>
    </message>
    <message>
        <source>Localization</source>
        <translation>Localización</translation>
    </message>
    <message>
        <source>Default Encoding:</source>
        <translation>Codificación por defecto:</translation>
    </message>
    <message>
        <source>Sets which default encoding should be used for newly added contacts.</source>
        <translation>Configura la codificación por defecto que debe usarse para los nuevos contactos.</translation>
    </message>
    <message>
        <source>System default (%1)</source>
        <translation>Por defecto del sistema (%1)</translation>
    </message>
    <message>
        <source>Show all encodings</source>
        <translation>Mostrar todas las codificaciones</translation>
    </message>
    <message>
        <source>Show all available encodings in the User Encoding selection menu. Normally, this menu shows only commonly used encodings.</source>
        <translation>Mostrar todas las codificaciones disponibles en el menú de selección de Codificaciones del Usuario. Normalmente este menú sólo muestra las codificaciones comunmente usadas.</translation>
    </message>
    <message>
        <source>Message Sent:</source>
        <translation>Mensaje Enviado:</translation>
    </message>
    <message>
        <source>Parameter for sent messages</source>
        <translation>Parámetro para los mensajes enviados</translation>
    </message>
    <message>
        <source>Perform the online notify OnEvent when logging on (this is different from how the Mirabilis client works)</source>
        <translation>Lleva a cabo la notificación de conexión OnEvent cuando se conecta (funciona de forma diferente al cliente de Mirabilis)</translation>
    </message>
    <message>
        <source>ICQ Server:</source>
        <translation>Servidor ICQ:</translation>
    </message>
    <message>
        <source>ICQ Server Port:</source>
        <translation>Puerto del Servidor ICQ:</translation>
    </message>
    <message>
        <source>Proxy</source>
        <translation>Proxy</translation>
    </message>
    <message>
        <source>Use proxy server</source>
        <translation>Usar Servidor Proxy</translation>
    </message>
    <message>
        <source>Proxy Type:</source>
        <translation>Tipo de Proxy</translation>
    </message>
    <message>
        <source>HTTPS</source>
        <translation>HTTPS</translation>
    </message>
    <message>
        <source>Proxy Server:</source>
        <translation>Servidor Proxy:</translation>
    </message>
    <message>
        <source>Proxy Server Port:</source>
        <translation>Puerto del Servidor Proxy:</translation>
    </message>
    <message>
        <source>Use authorization</source>
        <translation>Usar autorización</translation>
    </message>
    <message>
        <source>Username:</source>
        <translation>Usuario:</translation>
    </message>
    <message>
        <source>Password:</source>
        <translation>Contraseña:</translation>
    </message>
    <message>
        <source>Use server side contact list</source>
        <translation>Lista de contactos del lado del servidor</translation>
    </message>
    <message>
        <source>Store your contacts on the server so they are accessible from different locations and/or programs</source>
        <translation>Almacena sus contactos en el servidor para que sean accesible desde diferentes lugares y/o programas</translation>
    </message>
    <message>
        <source>Blink All Events</source>
        <translation>Parpadear Todos los Eventos</translation>
    </message>
    <message>
        <source>All incoming events will blink</source>
        <translation>Todos los Eventos Entrantes van a Parpadear</translation>
    </message>
    <message>
        <source>Blink Urgent Events</source>
        <translation>Parpadear Eventos Urgentes</translation>
    </message>
    <message>
        <source>Only urgent events will blink</source>
        <translation>Sólo los eventos urgentes van a parpadear</translation>
    </message>
    <message>
        <source>Use System Background Color</source>
        <translation>Usar Color de Fondo del Sistema</translation>
    </message>
    <message>
        <source>Hot key: </source>
        <translation>Tecla abreviada:</translation>
    </message>
    <message>
        <source>Hotkey pops up the next pending message
Enter the hotkey literally, like &quot;shift+f10&quot;, &quot;none&quot; for disabling
changes here require a Restart to take effect!
</source>
        <translation>La tecla abreviada muestra el próximo mensaje pendiente. 
Ingrse la tecla abreviada literalmente, como &quot;shift+f10&quot;, &quot;none&quot; para deshabilitar. 
Se requiere que se reinicie la aplicación para que estos cambios tengan efecto!</translation>
    </message>
    <message>
        <source>Popup info</source>
        <translation>Mostrar Información </translation>
    </message>
    <message>
        <source>Email</source>
        <translation>Correo-E</translation>
    </message>
    <message>
        <source>Phone</source>
        <translation>Teléfono</translation>
    </message>
    <message>
        <source>Fax</source>
        <translation>Fax</translation>
    </message>
    <message>
        <source>Cellular</source>
        <translation>Celular</translation>
    </message>
    <message>
        <source>IP</source>
        <translation>IP</translation>
    </message>
    <message>
        <source>Last online</source>
        <translation>Conectado/a por última vez</translation>
    </message>
    <message>
        <source>Online Time</source>
        <translation>Tiempo Conectado/a</translation>
    </message>
    <message>
        <source>Idle Time</source>
        <translation>Tiempo de inactividad</translation>
    </message>
    <message>
        <source>Auto Away Messages</source>
        <translation>Mensajes Automáticos de Ausente</translation>
    </message>
    <message>
        <source>Away:</source>
        <translation>Ausente:</translation>
    </message>
    <message>
        <source>N/A:</source>
        <translation>N/D:</translation>
    </message>
    <message>
        <source>Previous Message</source>
        <translation>Mensaje Anterior</translation>
    </message>
    <message>
        <source>Message Display</source>
        <translation>Muestra del Mensaje</translation>
    </message>
    <message>
        <source>Flash Taskbar on Incoming Msg</source>
        <translation>Hacer parpadear la Barra de Tareas cuando se recibe un Mensaje Entrante</translation>
    </message>
    <message>
        <source>Flash the Taskbar on incoming messages</source>
        <translation>Hacer parpadear la Barra de Tareas cuando se recibe un Mensaje Entrante</translation>
    </message>
    <message>
        <source>Tabbed Chatting</source>
        <translation>Ventana de conversaciones con tabs</translation>
    </message>
    <message>
        <source>Use tabs in Send Window</source>
        <translation>Usar Tabs en la Ventana de Envíos</translation>
    </message>
    <message>
        <source>Show recent messages</source>
        <translation>Mostrar mensajes recientes</translation>
    </message>
    <message>
        <source>Show the last 5 messages when a Send Window is opened</source>
        <translation>Mostrar los últimos 5 mensajes cuando se abre una Ventana de Envíos</translation>
    </message>
    <message>
        <source>Send typing notifications</source>
        <translation>Enviar notificación de tipeo</translation>
    </message>
    <message>
        <source>Send a notification to the user so they can see when you are typing a message to them</source>
        <translation>Envia una notificación al usuario para que sepa que usted a tipeando un mensaje para él/ella</translation>
    </message>
    <message>
        <source>Sticky Main Window</source>
        <translation>Ventana Principal Pegajosa</translation>
    </message>
    <message>
        <source>Makes the Main window visible on all desktops</source>
        <translation>Hace que la ventana principal sea visible en todos los escritorios</translation>
    </message>
    <message>
        <source>Sticky Message Window</source>
        <translation>Ventana de Mensajes Pegajosa</translation>
    </message>
    <message>
        <source>Makes the Message window visible on all desktops</source>
        <translation>Hace que la ventana de mensajes sea visible en todos los escritorios</translation>
    </message>
    <message>
        <source>Small Icon</source>
        <translation>Ícono pequeño</translation>
    </message>
    <message>
        <source>Uses the freedesktop.org standard to dock a small icon into the window manager.  Works with many different window managers.</source>
        <translation>Usa el estándar de freedesktop.org para posicionar un ícono pequeño dentro del manejador de ventanas. Funciona con diferentes manejadores de ventanas.</translation>
    </message>
    <message>
        <source>I am behind a firewall</source>
        <translation>Estoy detrás de un cortafuegos</translation>
    </message>
    <message>
        <source>Connection</source>
        <translation>Conexión</translation>
    </message>
    <message>
        <source>Reconnect after Uin clash</source>
        <translation>Reconectar luego de una colisión de Uin</translation>
    </message>
    <message>
        <source>Licq can reconnect you when you got disconnected because your Uin was used from another location. Check this if you want Licq to reconnect automatically.</source>
        <translation>Licq puede reconectarse cuando fue desconectado debido a que su Uin fue usado desde otro lugar. Seleccionelo si desea que Licq se reconecte automáticamente.</translation>
    </message>
    <message>
        <source>Additional &amp;sorting:</source>
        <translation>Ordenamiento Adicional:</translation>
    </message>
    <message>
        <source>status</source>
        <translation>estado</translation>
    </message>
    <message>
        <source>status + last event</source>
        <translation>estado + último evento</translation>
    </message>
    <message>
        <source>status + new messages</source>
        <translation>estado + nuevos mensajes</translation>
    </message>
    <message>
        <source>&lt;b&gt;none:&lt;/b&gt; - Don&apos;t sort online users by Status&lt;br&gt;
&lt;b&gt;status&lt;/b&gt; - Sort online users by status&lt;br&gt;
&lt;b&gt;status + last event&lt;/b&gt; - Sort online users by status and by last event&lt;br&gt;
&lt;b&gt;status + new messages&lt;/b&gt; - Sort online users by status and number of new messages</source>
        <translation>&lt;b&gt;ninguno:&lt;/b&gt; - No ordena los usuarios conectados por Estado&lt;br&gt;
&lt;b&gt;estado&lt;/b&gt; - Ordena los usuarios conectados por Estado&lt;br&gt;
&lt;b&gt;estado + último evento&lt;/b&gt; - Ordena los usuarios conectados por Estado y por último evento&lt;br&gt;
&lt;b&gt;estado + nuevos mensajes&lt;/b&gt; - Ordena los usuarios conectados por Estado y por número de mensajes nuevos.</translation>
    </message>
    <message>
        <source>Show user display picture</source>
        <translation>Mostrar la foto del usuario</translation>
    </message>
    <message>
        <source>Show the user&apos;s display picture instead of a status icon, if the user is online and has a display picture</source>
        <translation>Muestra la foto del usuario en lugar de su ícono de estado, si el usuario está conectado y tiene una foto.</translation>
    </message>
    <message>
        <source>Protocol ID</source>
        <translation>ID del protocolo</translation>
    </message>
    <message>
        <source>Auto Update</source>
        <translation>Actualizar Automáticamente</translation>
    </message>
    <message>
        <source>Auto Update Info</source>
        <translation>Actualizar Automáticamente la Información</translation>
    </message>
    <message>
        <source>Automatically update users&apos; server stored information.</source>
        <translation>Actualizar Automáticamente la información de los usuarios almacenada en el servidor</translation>
    </message>
    <message>
        <source>Auto Update Info Plugins</source>
        <translation>Actualizar Automáticamente la información de los añadidos</translation>
    </message>
    <message>
        <source>Automatically update users&apos; Phone Book and Picture.</source>
        <translation>Actualizar Automáticamente la agenda telefónica de los usuarios y sus fotos.</translation>
    </message>
    <message>
        <source>Auto Update Status Plugins</source>
        <translation>Actualizar Automáticamente el Estado de los Añadidos</translation>
    </message>
    <message>
        <source>Automatically update users&apos; Phone &quot;Follow Me&quot;, File Server and ICQphone status.</source>
        <translation>Actualizar Automáticamente los estados de Teléfono &quot;Sígame&quot;, del Servidor de Archivos y del ICQPhone</translation>
    </message>
    <message>
        <source>Style:</source>
        <translation>Estilo:</translation>
    </message>
    <message>
        <source>Insert Horizontal Line</source>
        <translation>Insertar Línea Horizontal</translation>
    </message>
    <message>
        <source>Colors</source>
        <translation>Colores</translation>
    </message>
    <message>
        <source>Message Received:</source>
        <translation>Mensaje Recibido:</translation>
    </message>
    <message>
        <source>History Received:</source>
        <translation>Historial de Recibidos:</translation>
    </message>
    <message>
        <source>History Sent:</source>
        <translation>Historial de Enviados:</translation>
    </message>
    <message>
        <source>Typing Notification Color:</source>
        <translation>Color de la Notificación de Tipeo:</translation>
    </message>
    <message>
        <source>Background Color:</source>
        <translation>Color de Fondo:</translation>
    </message>
    <message>
        <source>Single line chat mode</source>
        <translation>Modo de Charla de una sola línea</translation>
    </message>
    <message>
        <source>In single line chat mode you send messages with Enter and insert new lines with Ctrl+Enter, opposite of the normal mode</source>
        <translation>En modo de charla de una sola línea se envían los mensajes pulsando Enter y se inserta una nueva línea con Ctrl+Enter, contrariamente al modo normal</translation>
    </message>
    <message>
        <source>Use double return</source>
        <translation>Usar doble retorno</translation>
    </message>
    <message>
        <source>Hitting Return twice will be used instead of Ctrl+Return to send messages and close input dialogs. Multiple new lines can be inserted with Ctrl+Return.</source>
        <translation>Pulsar dos veces la tecla de retorno se utilizará en reemplazo de Ctrl+Return para enviar mensajes y cerrar los diálogos de entrada. Múltiples líneas pueden insertarse con Ctrl+Return.</translation>
    </message>
    <message>
        <source>Show Join/Left Notices</source>
        <translation>Mostrar notificación de ingreso y salida del chat</translation>
    </message>
    <message>
        <source>Show a notice in the chat window when a user joins or leaves the conversation.</source>
        <translation>Mostrar una notificación en la ventana de charla cuando un usuario se une o deja una conversación.</translation>
    </message>
    <message>
        <source>Alias</source>
        <translation>Alias</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <source>Date Format:</source>
        <translation>Formato de Fecha:</translation>
    </message>
    <message>
        <source>Picture</source>
        <translation>Foto</translation>
    </message>
    <message>
        <source>Local time</source>
        <translation>Hora local</translation>
    </message>
    <message>
        <source>Chat Options</source>
        <translation>Opciones de charla</translation>
    </message>
    <message>
        <source>Insert Vertical Spacing</source>
        <translation>Inserte espaciado vertical</translation>
    </message>
    <message>
        <source>Insert extra space between messages.</source>
        <translation>Inserte espacio extra entre los mensajes</translation>
    </message>
    <message>
        <source>Insert a line between each message.</source>
        <translation>Inserte una línea entre cada mensaje</translation>
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
        <translation>&lt;p&gt;Variables disponibles para personalizar fecha y hora.&lt;/p&gt;
&lt;table&gt;
&lt;tr&gt;&lt;th&gt;Expresión&lt;/th&gt;&lt;th&gt;Salida&lt;/th&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;d&lt;/td&gt;&lt;td&gt;el día como número sin cero al principio (1-31)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;dd&lt;/td&gt;&lt;td&gt;el día como número con cero al principio (01-31)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;ddd&lt;/td&gt;&lt;td&gt;el nombre del día abreviado (p.ej. &apos;Lun&apos;..&apos;Dom&apos;)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;dddd&lt;/td&gt;&lt;td&gt;el nombre del día (p.ej. &apos;Lunes&apos;..&apos;Domingo&apos;)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;M&lt;/td&gt;&lt;td&gt;el mes como número sin cero al principio (1-12)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;MM&lt;/td&gt;&lt;td&gt;el mes como número con cero al principio (01-12)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;MMM&lt;/td&gt;&lt;td&gt;el nombre del mes abreviado (p.ej. &apos;Ene&apos;..&apos;Dic&apos;)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;MMMM&lt;/td&gt;&lt;td&gt;el nombre del mes (p.ej. &apos;Enero&apos;..&apos;Diciembre&apos;)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;yy&lt;/td&gt;&lt;td&gt;el año en dos dígitos (00-99)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;yyyy&lt;/td&gt;&lt;td&gt;el año en cuatro dígitos (1752-8000)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td colspan=2&gt;&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;h&lt;/td&gt;&lt;td&gt;la hora sin cero al principio (0..23 o 1..12 si se muestra en formato AM/PM)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;hh&lt;/td&gt;&lt;td&gt;la hora con cero al principio (00..23 o 01..12 si se muestra en formato AM/PM)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;m&lt;/td&gt;&lt;td&gt;los minutos sin cero al principio (0..59)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;mm&lt;/td&gt;&lt;td&gt;los minutos sin cero al principio (00..59)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;s&lt;/td&gt;&lt;td&gt;los segundos sin cero al principio (0..59)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;ss&lt;/td&gt;&lt;td&gt;los segundos con cero al principio (00..59)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;z&lt;/td&gt;&lt;td&gt;los milisegundos sin cero al principio (0..999)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;zzz&lt;/td&gt;&lt;td&gt;los milisegundos sin cero al principio (000..999)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;AP&lt;/td&gt;&lt;td&gt;mostrar en formato AM/PM. AP será reemplazado por &apos;AM&apos; o &apos;PM&apos;&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;ap&lt;/td&gt;&lt;td&gt;mostrar en formato am/pm. ap será reemplazado por &apos;am&apos; o &apos;pm&apos;&lt;/td&gt;&lt;/tr&gt;
&lt;/table&gt;</translation>
    </message>
    <message>
        <source>History Options</source>
        <translation>Opciones de historial</translation>
    </message>
    <message>
        <source>Notice:</source>
        <translation>Aviso:</translation>
    </message>
    <message>
        <source>History</source>
        <translation>Historial</translation>
    </message>
    <message>
        <source>This is a received message</source>
        <translation>Este es un mensaje recibido</translation>
    </message>
    <message>
        <source>This is a sent message</source>
        <translation>Este es un mensaje enviado</translation>
    </message>
    <message>
        <source>Have you gone to the Licq IRC Channel?</source>
        <translation>Pasaste por el canal de IRC de Licq?</translation>
    </message>
    <message>
        <source>No, where is it?</source>
        <translation>No, ¿adónde está?</translation>
    </message>
    <message>
        <source>#Licq on irc.freenode.net</source>
        <translation>#Licq on irc.freenode.net</translation>
    </message>
    <message>
        <source>Cool, I&apos;ll see you there :)</source>
        <translation>Genial, nos vemos ahí :)</translation>
    </message>
    <message>
        <source>We&apos;ll be waiting!</source>
        <translation>¡Te estaremos esperando!</translation>
    </message>
    <message>
        <source>Marge has left the conversation.</source>
        <translation>Marge dejó la conversación.</translation>
    </message>
    <message>
        <source>to</source>
        <translation type="obsolete">a</translation>
    </message>
    <message>
        <source>The format string used to define what will appear in each column.
The following parameters can be used:</source>
        <translation>El formato de la cadena utilizado para definir qué aparecerá en cada columna. 
Los siguientes parámetros pueden ser utilizados:</translation>
    </message>
    <message>
        <source><byte value="x9"/>to</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>OwnerEditDlg</name>
    <message>
        <source>Edit Account</source>
        <translation>Editar Cuenta</translation>
    </message>
    <message>
        <source>User ID:</source>
        <translation>ID de Usuario:</translation>
    </message>
    <message>
        <source>Password:</source>
        <translation>Contraseña:</translation>
    </message>
    <message>
        <source>Protocol:</source>
        <translation>Protocolo:</translation>
    </message>
    <message>
        <source>Currently only one account per protocol is supported.</source>
        <translation>Sólo una cuenta por protoclo está soportada.</translation>
    </message>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;Ok</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Cancelar</translation>
    </message>
</context>
<context>
    <name>OwnerManagerDlg</name>
    <message>
        <source>Licq - Account Manager</source>
        <translation>Licq- Manejador de Cuentas</translation>
    </message>
    <message>
        <source>&amp;Add</source>
        <translation>&amp;Agregar</translation>
    </message>
    <message>
        <source>&amp;Register</source>
        <translation>&amp;Registrar</translation>
    </message>
    <message>
        <source>&amp;Modify</source>
        <translation>&amp;Modificar</translation>
    </message>
    <message>
        <source>D&amp;elete</source>
        <translation>Borrar</translation>
    </message>
    <message>
        <source>&amp;Done</source>
        <translation>&amp;Hecho</translation>
    </message>
    <message>
        <source>From the Account Manager dialog you are able to add and register your accounts.
Currently, only one account per protocol is supported, but this will be changed in future versions.</source>
        <translation>Desde el diálogo del Manejador de Cuentas usted puede agregar y registrar sus cuentas. 
Actualmente sólo una cuenta por protocolo está soportada, pero eso será cambiado en futuras versiones.</translation>
    </message>
    <message>
        <source>You are currently registered as
UIN (User ID): %1
Base Directory: %2
Rerun licq with the -b option to select a new
base directory and then register a new user.</source>
        <translation>Actualmente estás registrado como
UIN (ID de Usuario): %1
Directorio Base: %2
Ejecute licq nuevamente con la opción -b para seleccionar un nuevo directorio base y luego registre un nuevo usuario.</translation>
    </message>
    <message>
        <source>Successfully registered, your user identification
number (UIN) is %1.
Now set your personal information.</source>
        <translation>Registro finalizado, su número identificador
de usuario (UIN) es el %1.
Ahora introduzca sus datos personales.</translation>
    </message>
    <message>
        <source>Registration failed.  See network window for details.</source>
        <translation>Falló el Registro. Para más detalles, mire en la Ventana de Red.</translation>
    </message>
</context>
<context>
    <name>OwnerView</name>
    <message>
        <source>User ID</source>
        <translation>ID de Usuario</translation>
    </message>
    <message>
        <source>Protocol</source>
        <translation>Protocolo</translation>
    </message>
    <message>
        <source>(Invalid ID)</source>
        <translation>(ID Inválido)</translation>
    </message>
    <message>
        <source>Invalid Protocol</source>
        <translation>Protocolo Inválido</translation>
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
        <translation>Aceptar</translation>
    </message>
    <message>
        <source>Load</source>
        <translation>Cargar</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <source>Enable</source>
        <translation>Activar</translation>
    </message>
    <message>
        <source>Licq Plugin %1 %2
</source>
        <translation>Añadido Licq %1 %2</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Refrescar</translation>
    </message>
    <message>
        <source>Description</source>
        <translation>Descripción</translation>
    </message>
    <message>
        <source>Version</source>
        <translation>Versión</translation>
    </message>
    <message>
        <source>Plugin %1 has no configuration file</source>
        <translation>El añadido %1 ni tiene fichero de configuración</translation>
    </message>
    <message>
        <source>Standard Plugins</source>
        <translation>Añadidos Estándar</translation>
    </message>
    <message>
        <source>Protocol Plugins</source>
        <translation>Añadidos de Protocolo</translation>
    </message>
    <message>
        <source>(Unloaded)</source>
        <translation>(No cargado)</translation>
    </message>
</context>
<context>
    <name>QMessageBox</name>
    <message>
        <source>Licq Question</source>
        <translation>Pregunta de Licq</translation>
    </message>
    <message>
        <source>Yes</source>
        <translation>Si</translation>
    </message>
    <message>
        <source>No</source>
        <translation>No</translation>
    </message>
</context>
<context>
    <name>RegisterUserDlg</name>
    <message>
        <source>Please enter your password in both input fields.</source>
        <translation>Por favor introduzca su contraseña en ambos campos de texto.</translation>
    </message>
    <message>
        <source>Now please click &apos;Finish&apos; to start the registration process.</source>
        <translation>Por favor, pulse &apos;Finalizar&apos; para comenzar el proceso de registro.</translation>
    </message>
    <message>
        <source>Now please press the &apos;Back&apos; button and try again.</source>
        <translation>Por favo, pulse el botón &apos;Atrás&apos; para intentarlo otra vez.</translation>
    </message>
    <message>
        <source>The passwords don&apos;t seem to match.</source>
        <translation>Las contraseñas no coinciden.</translation>
    </message>
    <message>
        <source>Password:</source>
        <translation>Contraseña:</translation>
    </message>
    <message>
        <source>Verify:</source>
        <translation>Verificación:</translation>
    </message>
    <message>
        <source>Welcome to the Registration Wizard.

You can register a new user here.

Press &quot;Next&quot; to proceed.</source>
        <translation>Bienvenido al Asistente de Registración. 

Aquí puede registrar un nuevo usuario. 

Presione &quot;Siguiente&quot; para continuar.</translation>
    </message>
    <message>
        <source>Account Registration</source>
        <translation>Registro de Cuenta</translation>
    </message>
    <message>
        <source>Enter a password to protect your account.</source>
        <translation>Ingrese una contraseña para proteger su cuenta</translation>
    </message>
    <message>
        <source>&amp;Remember Password</source>
        <translation>&amp;Recordar Contraseña</translation>
    </message>
    <message>
        <source>Account Registration - Step 2</source>
        <translation>Registro de Cuenta - Paso 2</translation>
    </message>
    <message>
        <source>Account Registration - Step 3</source>
        <translation>Registro de Cuenta - Paso 3</translation>
    </message>
    <message>
        <source>Licq Account Registration</source>
        <translation>Registro de Cuenta de Licq</translation>
    </message>
    <message>
        <source>Invalid password, must be between 1 and 8 characters.</source>
        <translation>Contraseña no válida. Debe contener entre 1 y 8 caracteres.</translation>
    </message>
    <message>
        <source>Account Registration in Progress...</source>
        <translation>Registro de Cuenta en Progreso...</translation>
    </message>
    <message>
        <source>Account registration has been successfuly completed.
Your new user id is %1.
You are now being automatically logged on.
Click OK to edit your personal details.
After you are online, you can send your personal details to the server.</source>
        <translation>El registro de su cuenta se ha completado con éxito. 
Su nuevo ID de usuario es %1. 
Usted será conectado/a automáticamente. 
Presione OK para editar sus detalles personales. 
Luego de conectarse podrá enviar sus detalles personales al servidor.</translation>
    </message>
</context>
<context>
    <name>ReqAuthDlg</name>
    <message>
        <source>Licq - Request Authorization</source>
        <translation>Licq - Pedir Autorización</translation>
    </message>
    <message>
        <source>Request authorization from (UIN):</source>
        <translation>Pedir autorización de (UIN):</translation>
    </message>
    <message>
        <source>Request</source>
        <translation>Pedido</translation>
    </message>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;Ok</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Cancelar</translation>
    </message>
</context>
<context>
    <name>SearchUserDlg</name>
    <message>
        <source>Reset Search</source>
        <translation>Reiniciar Búsqueda</translation>
    </message>
    <message>
        <source>&amp;Done</source>
        <translation>Aceptar</translation>
    </message>
    <message>
        <source>UIN#:</source>
        <translation>UIN:</translation>
    </message>
    <message>
        <source>Enter search parameters and select &apos;Search&apos;</source>
        <translation>Introduzca los parámetros de la búsqueda y pulse &apos;Buscar&apos;</translation>
    </message>
    <message>
        <source>Searching (this can take awhile)...</source>
        <translation>Buscando (puede llevar un rato)...</translation>
    </message>
    <message>
        <source>More users found. Narrow search.</source>
        <translation>Se encontraron más usuarios. Refine la búsqueda.</translation>
    </message>
    <message>
        <source>Alias:</source>
        <translation>Alias:</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <source>First Name:</source>
        <translation>Nombre:</translation>
    </message>
    <message>
        <source>Last Name:</source>
        <translation>Apellidos:</translation>
    </message>
    <message>
        <source>Search failed.</source>
        <translation>La búsqueda fracasó.</translation>
    </message>
    <message>
        <source>A&amp;lert User</source>
        <translation>Avisar Usuario</translation>
    </message>
    <message>
        <source>&amp;Add %1 Users</source>
        <translation>&amp;Añadidos %1 Usuarios</translation>
    </message>
    <message>
        <source>Email Address:</source>
        <translation>Dirección de Correo-E:</translation>
    </message>
    <message>
        <source>&amp;Add User</source>
        <translation>&amp;Añadir Usuario</translation>
    </message>
    <message>
        <source>&amp;Search</source>
        <translation>Bu&amp;scar</translation>
    </message>
    <message>
        <source>Licq - User Search</source>
        <translation>Licq - Búsqueda de Usuarios</translation>
    </message>
    <message>
        <source>Age Range:</source>
        <translation>Rango de Edades:</translation>
    </message>
    <message>
        <source>Unspecified</source>
        <translation>No especificado</translation>
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
        <translation>Sexo:</translation>
    </message>
    <message>
        <source>Female</source>
        <translation>Mujer</translation>
    </message>
    <message>
        <source>Male</source>
        <translation>Hombre</translation>
    </message>
    <message>
        <source>Language:</source>
        <translation>Idioma:</translation>
    </message>
    <message>
        <source>City:</source>
        <translation>Ciudad:</translation>
    </message>
    <message>
        <source>State:</source>
        <translation>Provincia:</translation>
    </message>
    <message>
        <source>Country:</source>
        <translation>País:</translation>
    </message>
    <message>
        <source>Company Name:</source>
        <translation>Nombre de la Compañia:</translation>
    </message>
    <message>
        <source>Company Department:</source>
        <translation>Departamento de la Compañia:</translation>
    </message>
    <message>
        <source>Company Position:</source>
        <translation>Posición en la Compañia:</translation>
    </message>
    <message>
        <source>Keyword:</source>
        <translation>Palabra Clave:</translation>
    </message>
    <message>
        <source>Return Online Users Only</source>
        <translation>Retornar Sólo Usuarios Conectados</translation>
    </message>
    <message>
        <source>&amp;Whitepages</source>
        <translation>Páginas Blancas de ICQ</translation>
    </message>
    <message>
        <source>%1 more users found. Narrow search.</source>
        <translation>%1 usuarios más encontrados. Limite su búsqueda.</translation>
    </message>
    <message>
        <source>&amp;UIN#</source>
        <translation>&amp;UIN#</translation>
    </message>
    <message>
        <source>View &amp;Info</source>
        <translation>Ver &amp;información</translation>
    </message>
    <message>
        <source>Search complete.</source>
        <translation>Búsqueda completada</translation>
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
        <translation>Nombre</translation>
    </message>
    <message>
        <source>Alias</source>
        <translation>Alias</translation>
    </message>
    <message>
        <source>Email</source>
        <translation>Correo-E</translation>
    </message>
    <message>
        <source>Status</source>
        <translation>Estado</translation>
    </message>
    <message>
        <source>Sex &amp; Age</source>
        <translation>Sexo y Edad</translation>
    </message>
    <message>
        <source>Authorize</source>
        <translation>Autorizar</translation>
    </message>
    <message>
        <source>Offline</source>
        <translation>Desconectado/a</translation>
    </message>
    <message>
        <source>Online</source>
        <translation>Conectado/a</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation>Deconocido</translation>
    </message>
    <message>
        <source>F</source>
        <translation>F</translation>
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
        <translation>No</translation>
    </message>
    <message>
        <source>Yes</source>
        <translation>Si</translation>
    </message>
</context>
<context>
    <name>SecurityDlg</name>
    <message>
        <source>error</source>
        <translation>error</translation>
    </message>
    <message>
        <source>ICQ Security Options [Setting...]</source>
        <translation>Opciones de Seguridad de ICQ [Configurando...]</translation>
    </message>
    <message>
        <source>ICQ Security Options</source>
        <translation>Opciones de Seguridad de ICQ</translation>
    </message>
    <message>
        <source>You need to be connected to the
ICQ Network to change the settings.</source>
        <translation>Necesita estar conectado/a a la Red ICQ
para cambiar la configuración.</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>tiempo excedido</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>Opciones</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>fallo</translation>
    </message>
    <message>
        <source>Authorization Required</source>
        <translation>Necesita Autorización</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Cancelar</translation>
    </message>
    <message>
        <source>&amp;Update</source>
        <translation>Act&amp;ualizar</translation>
    </message>
    <message>
        <source>Web Presence</source>
        <translation>Presencia Web</translation>
    </message>
    <message>
        <source>ICQ Security Options [Setting...</source>
        <translation>Opciones de Seguridad de ICQ [Configurando...</translation>
    </message>
    <message>
        <source>Determines whether regular ICQ clients require your authorization to add you to their contact list.</source>
        <translation>Determina si los clientes normales de ICQ necesitan su autorización para poder añadirle a sus listas de contactos.</translation>
    </message>
    <message>
        <source>Hide IP</source>
        <translation>Ocultar IP</translation>
    </message>
    <message>
        <source>Password/UIN settings</source>
        <translation>Configuración de Contraseña/UIN</translation>
    </message>
    <message>
        <source>&amp;Uin:</source>
        <translation>&amp;Uin:</translation>
    </message>
    <message>
        <source>&amp;Password:</source>
        <translation>Contraseña:</translation>
    </message>
    <message>
        <source>Enter your ICQ password here.</source>
        <translation>Introduzca su contraseña de ICQ aquí.</translation>
    </message>
    <message>
        <source>Verify your ICQ password here.</source>
        <translation>Verifique su contraseña de ICQ aquí.</translation>
    </message>
    <message>
        <source>&amp;Local changes only</source>
        <translation>Sólo cambios &amp;locales</translation>
    </message>
    <message>
        <source>If checked, password/UIN changes will apply only on your local computer.  Useful if your password is incorrectly saved in Licq.</source>
        <translation>Si está seleccionado, los cambios a la contraseña/UIN se aplicarán sólo localmente. Es útil si la contraseña guardada en Licq es incorrecta.</translation>
    </message>
    <message>
        <source>Web Presence allows users to see if you are online through your web indicator.</source>
        <translation>Presencia Web le permite a otros usuarios ver si usted está conectado/a a través de su indicador web.</translation>
    </message>
    <message>
        <source>Hide IP stops users from seeing your IP address. It doesn&apos;t guarantee it will be hidden though.</source>
        <translation>Ocultar IP no permite que otros usuarios vean su dirección IP. A pesar de todo, esto no garantiza que esté oculta.</translation>
    </message>
    <message>
        <source>Invalid password, must be between 1 and 8 characters.</source>
        <translation>Contraseña no válida, debe tener entre 1 y 8 caracteres.</translation>
    </message>
    <message>
        <source>Passwords do not match, try again.</source>
        <translation>Las contraseñas no coinciden, intente nuevamente.</translation>
    </message>
    <message>
        <source>Setting security options failed.</source>
        <translation>Fallo al guardar las opciones de seguridad.</translation>
    </message>
    <message>
        <source>Changing password failed.</source>
        <translation>Falló el cambio de contraseña.</translation>
    </message>
    <message>
        <source>Timeout while setting security options.</source>
        <translation>Se venció el tiempo de espera mientras se guardaban las opciones de configuración.</translation>
    </message>
    <message>
        <source>Timeout while changing password.</source>
        <translation>Se venció el tiempo de espera mientras se cambiaba la contraseña.</translation>
    </message>
    <message>
        <source>Internal error while setting security options.</source>
        <translation>Error interno mientras se guardaban las opciones de seguridad.</translation>
    </message>
    <message>
        <source>Internal error while changing password.</source>
        <translation>Error interno mientras se cambiaba la contraseña.</translation>
    </message>
    <message>
        <source>Enter the UIN which you want to use.  Only available if &quot;Local changes only&quot; is checked.</source>
        <translation>Ingrese el UIN que desea utilizar. Sólo disponible si &quot;Sólo cambios locales&quot; está activado.</translation>
    </message>
    <message>
        <source>&amp;Verify:</source>
        <translation>&amp;Verificar:</translation>
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
        <translation>error</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>tiempo excedido</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>fallo</translation>
    </message>
    <message>
        <source>&amp;Show Again</source>
        <translation>Mo&amp;strar Otra Vez</translation>
    </message>
    <message>
        <source>%1 Response for %2</source>
        <translation>%1 Respuesta para %2</translation>
    </message>
    <message>
        <source>refused</source>
        <translation>rechazado</translation>
    </message>
</context>
<context>
    <name>SkinBrowserDlg</name>
    <message>
        <source>Error</source>
        <translation>Error</translation>
    </message>
    <message>
        <source>&amp;Apply</source>
        <translation>&amp;Aplicar</translation>
    </message>
    <message>
        <source>Licq Skin Browser</source>
        <translation>Selector de Temas Licq</translation>
    </message>
    <message>
        <source>Skin selection</source>
        <translation>Selección de pieles</translation>
    </message>
    <message>
        <source>Preview</source>
        <translation>Vista Previa</translation>
    </message>
    <message>
        <source>S&amp;kins:</source>
        <translation>Pieles:</translation>
    </message>
    <message>
        <source>Use this combo box to select one of the available skins</source>
        <translation>Use las opciones para seleccionar una de las pieles disponibles.</translation>
    </message>
    <message>
        <source>&amp;Icons:</source>
        <translation>&amp;Íconos:</translation>
    </message>
    <message>
        <source>Use this combo box to select one of the available icon sets</source>
        <translation>Use las opciones para seleccionar uno de los paquetes de íconos disponibles.</translation>
    </message>
    <message>
        <source>E&amp;xtended Icons:</source>
        <translation>Íconos E&amp;xtendidos:</translation>
    </message>
    <message>
        <source>Use this combo box to select one of the available extended icon sets</source>
        <translation>Use las opciones para seleccionar uno de los paquetes de íconos disponibles.</translation>
    </message>
    <message>
        <source>Use this combo box to select one of the available emoticon icon sets</source>
        <translation>Use las opciones para seleccionar uno de los paquetes de íconos disponibles.</translation>
    </message>
    <message>
        <source>Skin:</source>
        <translation>Piel:</translation>
    </message>
    <message>
        <source>Icons:</source>
        <translation>Íconos:</translation>
    </message>
    <message>
        <source>Extended Icons:</source>
        <translation>Íconos Extendidos:</translation>
    </message>
    <message>
        <source>Emoticons:</source>
        <translation>Emoticones:</translation>
    </message>
    <message>
        <source>&amp;Edit Skin</source>
        <translation>&amp;Editar Piel</translation>
    </message>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;Ok</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Cancelar</translation>
    </message>
    <message>
        <source>Unable to open icons file
%1
Iconset &apos;%2&apos; has been disabled.</source>
        <translation>No se puede abrir el fichero de íconos 
%1 
El paquete de íconos %2 ha sido deshabilitado.</translation>
    </message>
    <message>
        <source>Unable to open extended icons file
%1
Extended Iconset &apos;%2&apos; has been disabled.</source>
        <translation>No se puede abrir el fichero de íconos extendidos 
%1 
El paquete de íconos extendidos %2 ha sido deshabilitado.</translation>
    </message>
    <message>
        <source>Unable to open icons file
%1</source>
        <translation>No se puede abrir el fichero de íconos 
%1</translation>
    </message>
    <message>
        <source>Unable to open extended icons file
%1</source>
        <translation>No se puede abrir el fichero de íconos extendidos
%1</translation>
    </message>
    <message>
        <source>E&amp;moticons:</source>
        <translation>E&amp;moticones:</translation>
    </message>
</context>
<context>
    <name>Status</name>
    <message>
        <source>Offline</source>
        <translation>Desconcetado/a</translation>
    </message>
    <message>
        <source>Online</source>
        <translation>Conectado/a</translation>
    </message>
    <message>
        <source>Away</source>
        <translation>Ausente</translation>
    </message>
    <message>
        <source>Do Not Disturb</source>
        <translation>No Molestar</translation>
    </message>
    <message>
        <source>Not Available</source>
        <translation>No Disponible</translation>
    </message>
    <message>
        <source>Occupied</source>
        <translation>Ocupado/a</translation>
    </message>
    <message>
        <source>Free for Chat</source>
        <translation>Disponible Para Charlar </translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation>Desconocido</translation>
    </message>
    <message>
        <source>Off</source>
        <translation>Desc</translation>
    </message>
    <message>
        <source>On</source>
        <translation>Conec</translation>
    </message>
    <message>
        <source>DND</source>
        <translation>NoMolestar</translation>
    </message>
    <message>
        <source>N/A</source>
        <translation>NoDisp</translation>
    </message>
    <message>
        <source>Occ</source>
        <translation>Ocu</translation>
    </message>
    <message>
        <source>FFC</source>
        <translation>Libre</translation>
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
        <translation>Arábigo</translation>
    </message>
    <message>
        <source>Baltic</source>
        <translation>Báltico</translation>
    </message>
    <message>
        <source>Central European</source>
        <translation>Centroeuropeo</translation>
    </message>
    <message>
        <source>Chinese</source>
        <translation>Chino</translation>
    </message>
    <message>
        <source>Chinese Traditional</source>
        <translation>Chino Tradicional</translation>
    </message>
    <message>
        <source>Cyrillic</source>
        <translation>Cirílico</translation>
    </message>
    <message>
        <source>Esperanto</source>
        <translation>Esperanto</translation>
    </message>
    <message>
        <source>Greek</source>
        <translation>Griego</translation>
    </message>
    <message>
        <source>Hebrew</source>
        <translation>Hebreo</translation>
    </message>
    <message>
        <source>Japanese</source>
        <translation>Japonés</translation>
    </message>
    <message>
        <source>Korean</source>
        <translation>Coreano</translation>
    </message>
    <message>
        <source>Western European</source>
        <translation>Europeo Occidental</translation>
    </message>
    <message>
        <source>Tamil</source>
        <translation>Tamil</translation>
    </message>
    <message>
        <source>Thai</source>
        <translation>Tailandés</translation>
    </message>
    <message>
        <source>Turkish</source>
        <translation>Turco</translation>
    </message>
    <message>
        <source>Ukrainian</source>
        <translation>Ucraniano</translation>
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
        <translation>Hora:</translation>
    </message>
    <message>
        <source>Show User Info</source>
        <translation>Mostrar Información del Usuario</translation>
    </message>
    <message>
        <source>Status:</source>
        <translation>Estado:</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation>Desconocido/a</translation>
    </message>
    <message>
        <source>Show User History</source>
        <translation>Mostrar Historial del Usuario</translation>
    </message>
    <message>
        <source>Open / Close secure channel</source>
        <translation>Abrir / Cerrar canal seguro</translation>
    </message>
    <message>
        <source>Change user text encoding</source>
        <translation>Cambiar codificación de texto del usuario</translation>
    </message>
    <message>
        <source>This button selects the text encoding used when communicating with this user. You might need to change the encoding to communicate in a different language.</source>
        <translation>Este botón selecciona la codificación de texto utilizada para comunicarse con el usuario. Es posible que sea necesario cambiar la codificación para comunicarse en otros idiomas</translation>
    </message>
    <message>
        <source>Unable to load encoding &lt;b&gt;%1&lt;/b&gt;. Message contents may appear garbled.</source>
        <translation>No se pudo cargar la codificación &lt;b&gt;%1&lt;/b&gt;. El contenido del mensaje puede ser ilegible.</translation>
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
        <translation>Ahora</translation>
    </message>
    <message>
        <source>Age:</source>
        <translation>Edad:</translation>
    </message>
    <message>
        <source>Fax:</source>
        <translation>Fax:</translation>
    </message>
    <message>
        <source>Male</source>
        <translation>Hombre</translation>
    </message>
    <message>
        <source>Zip:</source>
        <translation>C.P.:</translation>
    </message>
    <message>
        <source>done</source>
        <translation>hecho</translation>
    </message>
    <message>
        <source> Day:</source>
        <translation> Día:</translation>
    </message>
    <message>
        <source>&amp;Menu</source>
        <translation>&amp;Menu</translation>
    </message>
    <message>
        <source>&amp;More</source>
        <translation>&amp;Más</translation>
    </message>
    <message>
        <source>&amp;Save</source>
        <translation>Guardar</translation>
    </message>
    <message>
        <source>&amp;Work</source>
        <translation>Trabajo</translation>
    </message>
    <message>
        <source>&amp;Filter: </source>
        <translation>&amp;Filtrar: </translation>
    </message>
    <message>
        <source>City:</source>
        <translation>Ciudad:</translation>
    </message>
    <message>
        <source>P&amp;rev</source>
        <translation>Ante&amp;rior</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation>Nombre:</translation>
    </message>
    <message>
        <source>Nex&amp;t</source>
        <translation>Siguien&amp;te</translation>
    </message>
    <message>
        <source>error</source>
        <translation>error</translation>
    </message>
    <message>
        <source>Updating server...</source>
        <translation>Actualizando en servidor...</translation>
    </message>
    <message>
        <source> Year:</source>
        <translation> Año:</translation>
    </message>
    <message>
        <source>&amp;About</source>
        <translation>&amp;Acerca de</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Cerrar</translation>
    </message>
    <message>
        <source>Cellular:</source>
        <translation>Tlf.Móvil:</translation>
    </message>
    <message>
        <source>EMail 1:</source>
        <translation>Correo-E 1:</translation>
    </message>
    <message>
        <source>EMail 2:</source>
        <translation>Correo-E 2:</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>tiempo consumido</translation>
    </message>
    <message>
        <source>Homepage:</source>
        <translation>Página Web:</translation>
    </message>
    <message>
        <source>Timezone:</source>
        <translation>Zona Horaria:</translation>
    </message>
    <message>
        <source>About:</source>
        <translation>Acerca de:</translation>
    </message>
    <message>
        <source>Alias:</source>
        <translation>Alias:</translation>
    </message>
    <message>
        <source>Female</source>
        <translation>Mujer</translation>
    </message>
    <message>
        <source> Month:</source>
        <translation> Més:</translation>
    </message>
    <message>
        <source>Phone:</source>
        <translation>Teléfono:</translation>
    </message>
    <message>
        <source>State:</source>
        <translation>Provincia:</translation>
    </message>
    <message>
        <source>Authorization Not Required</source>
        <translation>No Necesita Autorización</translation>
    </message>
    <message>
        <source>[&lt;font color=&quot;%1&quot;&gt;Received&lt;/font&gt;] [&lt;font color=&quot;%2&quot;&gt;Sent&lt;/font&gt;] %3 out of %4 matches</source>
        <translation>[&lt;font color=&quot;%1&quot;&gt;Recibido&lt;/font&gt;] [&lt;font color=&quot;%2&quot;&gt;Enviado&lt;/font&gt;] %3 de %4 coincidencias</translation>
    </message>
    <message>
        <source>[&lt;font color=&quot;%1&quot;&gt;Received&lt;/font&gt;] [&lt;font color=&quot;%2&quot;&gt;Sent&lt;/font&gt;] %3 to %4 of %5</source>
        <translation>[&lt;font color=&quot;%1&quot;&gt;Recibido&lt;/font&gt;] [&lt;font color=&quot;%2&quot;&gt;Enviado&lt;/font&gt;] %3 a %4 de %5</translation>
    </message>
    <message>
        <source>Country:</source>
        <translation>País:</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>fallo</translation>
    </message>
    <message>
        <source>server</source>
        <translation>servidor</translation>
    </message>
    <message>
        <source>Authorization Required</source>
        <translation>Necesita Autorización</translation>
    </message>
    <message>
        <source>Birthday:</source>
        <translation>Fecha Nac.:</translation>
    </message>
    <message>
        <source>Unknown (%1)</source>
        <translation>Desconocido (%1)</translation>
    </message>
    <message>
        <source>Department:</source>
        <translation>Departamento:</translation>
    </message>
    <message>
        <source>Position:</source>
        <translation>Posición:</translation>
    </message>
    <message>
        <source>Sorry, history is disabled for this person.</source>
        <translation>Lo siento, el historial de esta persona está inhabilitado.</translation>
    </message>
    <message>
        <source>Licq - Info </source>
        <translation>Licq - Información </translation>
    </message>
    <message>
        <source>Status:</source>
        <translation>Estado:</translation>
    </message>
    <message>
        <source>Address:</source>
        <translation>Dirección:</translation>
    </message>
    <message>
        <source>Updating...</source>
        <translation>Actualizando...</translation>
    </message>
    <message>
        <source>Language 3:</source>
        <translation>Idioma 3:</translation>
    </message>
    <message>
        <source>Language 1:</source>
        <translation>Idioma 1:</translation>
    </message>
    <message>
        <source>Language 2:</source>
        <translation>Idioma 2:</translation>
    </message>
    <message>
        <source>Unspecified</source>
        <translation>No especificado</translation>
    </message>
    <message>
        <source>&amp;Update</source>
        <translation>Act&amp;ualizar</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation>Desconocido</translation>
    </message>
    <message>
        <source>Rever&amp;se</source>
        <translation>Rever&amp;so</translation>
    </message>
    <message>
        <source>Gender:</source>
        <translation>Sexo:</translation>
    </message>
    <message>
        <source>&amp;General</source>
        <translation>&amp;General</translation>
    </message>
    <message>
        <source>&amp;History</source>
        <translation>&amp;Historial</translation>
    </message>
    <message>
        <source>Error loading history file: %1
Description: %2</source>
        <translation>Error cargando fichero de historial: %1
Descripción: %2</translation>
    </message>
    <message>
        <source>Retrieve</source>
        <translation>Recuperar</translation>
    </message>
    <message>
        <source>INVALID USER</source>
        <translation>USUARIO INVÁLIDO</translation>
    </message>
    <message>
        <source>Old Email:</source>
        <translation>Correo-E Viejo:</translation>
    </message>
    <message>
        <source>&amp;Last</source>
        <translation>Ú&amp;ltimo</translation>
    </message>
    <message>
        <source>Last Online:</source>
        <translation>Última vez conectado/a:</translation>
    </message>
    <message>
        <source>Last Sent Event:</source>
        <translation>Último evento enviado:</translation>
    </message>
    <message>
        <source>Last Received Event:</source>
        <translation>Último evento recibido:</translation>
    </message>
    <message>
        <source>Last Checked Auto Response:</source>
        <translation>Última vez que verificó la Respuesta Automática:</translation>
    </message>
    <message>
        <source>Online Since:</source>
        <translation>Conectado/a Desde:</translation>
    </message>
    <message>
        <source>Offline</source>
        <translation>Desconectado/a</translation>
    </message>
    <message>
        <source>You need to be connected to the
ICQ Network to retrieve your settings.</source>
        <translation>Debe estar conectado a la Red ICQ para recuperar su configuración.</translation>
    </message>
    <message>
        <source>You need to be connected to the
ICQ Network to change your settings.</source>
        <translation>Debe estar conectado a la Red ICQ para modificar su configuración.</translation>
    </message>
    <message>
        <source>Keep Alias on Update</source>
        <translation>Mantener el Alias al Actualizar</translation>
    </message>
    <message>
        <source>Normally Licq overwrites the Alias when updating user details.
Check this if you want to keep your changes to the Alias.</source>
        <translation>Normalmente Licq sobreescribe el Alias cuando se actualizan los detalles del usuario. 
Seleccione esta opción si desea mantener el Alias actual luego de los cambios.</translation>
    </message>
    <message>
        <source>ID:</source>
        <translation>ID:</translation>
    </message>
    <message>
        <source>Category:</source>
        <translation>Categoría:</translation>
    </message>
    <message>
        <source>Description:</source>
        <translation>Descripción:</translation>
    </message>
    <message>
        <source>User has an ICQ Homepage </source>
        <translation>El Usuario tiene una página en ICQ:</translation>
    </message>
    <message>
        <source>User has no ICQ Homepage</source>
        <translation>El Usuario no tiene una página en ICQ:</translation>
    </message>
    <message>
        <source>M&amp;ore II</source>
        <translation>Más II</translation>
    </message>
    <message>
        <source>(none)</source>
        <translation>(ninguno)</translation>
    </message>
    <message>
        <source>Occupation:</source>
        <translation>Ocupación:</translation>
    </message>
    <message>
        <source>&amp;Phone</source>
        <translation>Teléfono</translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <source>Number/Gateway</source>
        <translation>Número/Enlace</translation>
    </message>
    <message>
        <source>Country/Provider</source>
        <translation>País/Proveedor</translation>
    </message>
    <message>
        <source>Currently at:</source>
        <translation>Actualmente en:</translation>
    </message>
    <message>
        <source>(</source>
        <translation>(</translation>
    </message>
    <message>
        <source>) </source>
        <translation>)</translation>
    </message>
    <message>
        <source>-</source>
        <translation>-</translation>
    </message>
    <message>
        <source>P&amp;icture</source>
        <translation>Foto</translation>
    </message>
    <message>
        <source>Not Available</source>
        <translation>No Disponible</translation>
    </message>
    <message>
        <source>Failed to Load</source>
        <translation>Fallo al Cargar</translation>
    </message>
    <message>
        <source>&amp;KDE Addressbook</source>
        <translation>Libreta de Direcciones de &amp;KDE</translation>
    </message>
    <message>
        <source>Email:</source>
        <translation>Correo Electrónico:</translation>
    </message>
    <message>
        <source>&amp;Retrieve</source>
        <translation>&amp;Recuperar</translation>
    </message>
    <message>
        <source>S&amp;end</source>
        <translation>&amp;Enviar</translation>
    </message>
    <message>
        <source>&amp;Add</source>
        <translation>&amp;Agregar</translation>
    </message>
    <message>
        <source>&amp;Clear</source>
        <translation>Limpiar</translation>
    </message>
    <message>
        <source>&amp;Browse</source>
        <translation>Seleccionar</translation>
    </message>
    <message>
        <source>Select your picture</source>
        <translation>Seleccione su Foto</translation>
    </message>
    <message>
        <source> is over %1 bytes.
Select another picture?</source>
        <translation>tiene más de %1 bytes. 
Desea seleccionar otro foto?</translation>
    </message>
    <message>
        <source>Yes</source>
        <translation>Si</translation>
    </message>
    <message>
        <source>No</source>
        <translation>No</translation>
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
        <translation>&amp;Usuario:</translation>
    </message>
    <message>
        <source>&amp;Password:</source>
        <translation>Contraseña:</translation>
    </message>
    <message>
        <source>&amp;Save Password</source>
        <translation>Guardar Contra&amp;seña</translation>
    </message>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;Ok</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Cancelar</translation>
    </message>
    <message>
        <source>Licq User Select</source>
        <translation>Seleccionar Usuario Licq</translation>
    </message>
</context>
<context>
    <name>UserSendChatEvent</name>
    <message>
        <source>Clear</source>
        <translation>Limpiar</translation>
    </message>
    <message>
        <source>Multiparty: </source>
        <translation>Multiples Usuarios: </translation>
    </message>
    <message>
        <source>Invite</source>
        <translation>Invitar</translation>
    </message>
    <message>
        <source>Chat with %2 refused:
%3</source>
        <translation>Charla con %2 rechazada:
%3</translation>
    </message>
    <message>
        <source> - Chat Request</source>
        <translation> - Petición de Charla</translation>
    </message>
    <message>
        <source>No reason provided</source>
        <translation>No se dió ninguna razón</translation>
    </message>
</context>
<context>
    <name>UserSendCommon</name>
    <message>
        <source>No</source>
        <translation>No</translation>
    </message>
    <message>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <source>Yes</source>
        <translation>Si</translation>
    </message>
    <message>
        <source>done</source>
        <translation>hecho</translation>
    </message>
    <message>
        <source>&amp;Menu</source>
        <translation>&amp;Menú</translation>
    </message>
    <message>
        <source>&amp;Send</source>
        <translation>Enviar</translation>
    </message>
    <message>
        <source>error</source>
        <translation>error</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Cerrar</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>tiempo consumido</translation>
    </message>
    <message>
        <source>Message</source>
        <translation>Mensaje</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <source>File Transfer</source>
        <translation>Transferencia de Ficheros</translation>
    </message>
    <message>
        <source>Drag Users Here
Right Click for Options</source>
        <translation>Arrastre los Usuarios Aquí
Pulse con el Botón Derecho para Opciones</translation>
    </message>
    <message>
        <source>direct</source>
        <translation>directo</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>fallo</translation>
    </message>
    <message>
        <source>via server</source>
        <translation>por servidor</translation>
    </message>
    <message>
        <source>U&amp;rgent</source>
        <translation>U&amp;rgente</translation>
    </message>
    <message>
        <source>Direct send failed,
send through server?</source>
        <translation>Falló el envío directo,
¿enviar a través del servidor?</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Cancelar</translation>
    </message>
    <message>
        <source>Sending </source>
        <translation>Enviando </translation>
    </message>
    <message>
        <source>Chat Request</source>
        <translation>Petición de Charla</translation>
    </message>
    <message>
        <source>Contact List</source>
        <translation>Lista de Contactos</translation>
    </message>
    <message>
        <source>M&amp;ultiple recipients</source>
        <translation>M&amp;ultiples Destinatarios</translation>
    </message>
    <message>
        <source>Se&amp;nd through server</source>
        <translation>E&amp;nviar por el servidor</translation>
    </message>
    <message>
        <source>SMS</source>
        <translation>SMS</translation>
    </message>
    <message>
        <source>%1 is in %2 mode:
%3
Send...</source>
        <translation>%1 está en modo %2: 
%3 
Enviar...</translation>
    </message>
    <message>
        <source>Urgent</source>
        <translation>Urgente</translation>
    </message>
    <message>
        <source> to Contact List</source>
        <translation> a la Lista de Contactos</translation>
    </message>
    <message>
        <source>Warning: Message can&apos;t be sent securely
through the server!</source>
        <translation>Atención: El mensaje no se puede enviar de forma segura a través del servidor!</translation>
    </message>
    <message>
        <source>Send anyway</source>
        <translation>Enviar de todas formas.</translation>
    </message>
    <message>
        <source>Error! no owner set</source>
        <translation>Error! no se seleccionó dueño</translation>
    </message>
    <message>
        <source>cancelled</source>
        <translation>cancelado</translation>
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
        <translation>Arraster Usuarios Aquí - Pulse con el Botón Derecho para Opciones</translation>
    </message>
    <message>
        <source> - Contact List</source>
        <translation> - Lista de Contactos</translation>
    </message>
</context>
<context>
    <name>UserSendFileEvent</name>
    <message>
        <source>You must specify a file to transfer!</source>
        <translation>¡Debe indicar qué ficheros enviar!</translation>
    </message>
    <message>
        <source>File(s): </source>
        <translation>Fichero(s): </translation>
    </message>
    <message>
        <source>Browse</source>
        <translation>Explorar</translation>
    </message>
    <message>
        <source> - File Transfer</source>
        <translation> - Transferencia de Ficheros</translation>
    </message>
    <message>
        <source>File transfer with %2 refused:
%3</source>
        <translation>Rechazada Transferencia de Ficheros con %2:
%3</translation>
    </message>
    <message>
        <source>Select files to send</source>
        <translation>Seleccione los archivos a enviar</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Editar</translation>
    </message>
    <message>
        <source>No reason provided</source>
        <translation>No se dió ninguna razón</translation>
    </message>
</context>
<context>
    <name>UserSendMsgEvent</name>
    <message>
        <source>&amp;No</source>
        <translation>&amp;No</translation>
    </message>
    <message>
        <source>&amp;Yes</source>
        <translation>Si</translation>
    </message>
    <message>
        <source> - Message</source>
        <translation> - Mensaje</translation>
    </message>
    <message>
        <source>You didn&apos;t edit the message.
Do you really want to send it?</source>
        <translation>No editó el mensaje.
¿Realmente quiere enviarlo?</translation>
    </message>
</context>
<context>
    <name>UserSendSmsEvent</name>
    <message>
        <source>Phone : </source>
        <translation>Teléfono : </translation>
    </message>
    <message>
        <source>Chars left : </source>
        <translation>Caracteres disponibles : </translation>
    </message>
    <message>
        <source> - SMS</source>
        <translation> - SMS</translation>
    </message>
    <message>
        <source>You didn&apos;t edit the SMS.
Do you really want to send it?</source>
        <translation>No editó el SMS. 
¿Realmente desea enviarlo?</translation>
    </message>
    <message>
        <source>&amp;Yes</source>
        <translation>Si</translation>
    </message>
    <message>
        <source>&amp;No</source>
        <translation>&amp;No</translation>
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
        <translation>No se especificó URL</translation>
    </message>
</context>
<context>
    <name>UserViewEvent</name>
    <message>
        <source>Chat</source>
        <translation>Charla</translation>
    </message>
    <message>
        <source>&amp;Join</source>
        <translation>Unirse</translation>
    </message>
    <message>
        <source>&amp;Menu</source>
        <translation>&amp;Menú</translation>
    </message>
    <message>
        <source>&amp;View</source>
        <translation>&amp;Ver</translation>
    </message>
    <message>
        <source>Nex&amp;t</source>
        <translation>Siguien&amp;te</translation>
    </message>
    <message>
        <source>A&amp;dd %1 Users</source>
        <translation>Aña&amp;dir %1 Usuarios</translation>
    </message>
    <message>
        <source>Aut&amp;o Close</source>
        <translation>Cerrar Aut&amp;omáticamente</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Cerrar</translation>
    </message>
    <message>
        <source>&amp;Quote</source>
        <translation>Citar</translation>
    </message>
    <message>
        <source>&amp;Reply</source>
        <translation>&amp;Responder</translation>
    </message>
    <message>
        <source>A&amp;ccept</source>
        <translation>A&amp;ceptar</translation>
    </message>
    <message>
        <source>Normal Click - Close Window
&lt;CTRL&gt;+Click - also delete User</source>
        <translation>Pulsación Normal - Cerrar Ventana
&lt;CTRL&gt;+Pulsación - también borrar el Usuario</translation>
    </message>
    <message>
        <source>File Transfer</source>
        <translation>Transferencia de Ficheros</translation>
    </message>
    <message>
        <source>Start Chat</source>
        <translation>Iniciar Charla</translation>
    </message>
    <message>
        <source>
--------------------
Request was cancelled.</source>
        <translation>
--------------------
Petición cancelada.</translation>
    </message>
    <message>
        <source>A&amp;dd User</source>
        <translation>Aña&amp;dir Usuario</translation>
    </message>
    <message>
        <source>A&amp;uthorize</source>
        <translation>A&amp;utorizar</translation>
    </message>
    <message>
        <source>&amp;Refuse</source>
        <translation>&amp;Rechazar</translation>
    </message>
    <message>
        <source>Nex&amp;t (%1)</source>
        <translation>Siguien&amp;te (%1)</translation>
    </message>
    <message>
        <source>&amp;Forward</source>
        <translation>Reenviar</translation>
    </message>
    <message>
        <source>&amp;View Email</source>
        <translation>&amp;Ver Correo Electrónico</translation>
    </message>
    <message>
        <source>Licq is unable to find a browser application due to an internal error.</source>
        <translation>Licq no ha podido encontrar un navegador debido a un error interno.</translation>
    </message>
    <message>
        <source>Licq is unable to start your browser and open the URL.
You will need to start the browser and open the URL manually.</source>
        <translation>Licq no ha podido iniciar su navegador y abrir la URL. 
Deberá iniciar el navegador y abrir la URL de forma manual.</translation>
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
        <translation>Escriba las letras mostradas más abajo:</translation>
    </message>
    <message>
        <source>&amp;OK</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>Licq - New Account Verification</source>
        <translation>Licq - Verificación de Nueva Cuenta</translation>
    </message>
</context>
<context>
    <name>WharfIcon</name>
    <message>
        <source>Left click - Show main window
Middle click - Show next message
Right click - System menu</source>
        <translation>Botón Izquierdo - Mostrar ventana principal
Botón Central - Mostrar siguiente mensaje
Botón Derecho - Menú del sistema</translation>
    </message>
</context>
</TS>
