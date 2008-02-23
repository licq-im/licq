<!DOCTYPE TS>
<TS>
  <context>
    <name>@default</name>
    <message>
      <source>KDE default</source>
      <translation>KDE por defecto</translation>
    </message>
  </context>
  <context>
    <name>Groups</name>
    <message>
      <source>All Users</source>
      <translation>Todos los Usuarios</translation>
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
      <translation>Usuarios Nuevos</translation>
    </message>
    <message>
      <source>Unknown</source>
      <translation>Desconocido</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::AboutDlg</name>
    <message>
      <source>Licq - About</source>
      <translation>Licq-Acerca de</translation>
    </message>
    <message>
      <source>Version</source>
      <translation>Versión</translation>
    </message>
    <message>
      <source>Compiled on</source>
      <translation>Compilado el</translation>
    </message>
    <message>
      <source>Credits</source>
      <translation>Créditos</translation>
    </message>
    <message>
      <source>Maintainer</source>
      <translation>Mantenedor</translation>
    </message>
    <message>
      <source>Contributions</source>
      <translation>Contribuciones</translation>
    </message>
    <message>
      <source>Original author</source>
      <translation>Autor original</translation>
    </message>
    <message>
      <source>Contact us</source>
      <translation>Contáctanos</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::AddUserDlg</name>
    <message>
      <source>&amp;Protocol:</source>
      <translation>&amp;Protocolo:</translation>
    </message>
    <message>
      <source>&amp;New User ID:</source>
      <translation>&amp;Nuevo ID de Usuario:</translation>
    </message>
    <message>
      <source>Licq - Add user</source>
      <translation>Licq - Agregar usuario</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::AuthUserDlg</name>
    <message>
      <source>Response</source>
      <translation>Respuesta</translation>
    </message>
    <message>
      <source>Licq - %1 Authorization</source>
      <translation>Licq - %1 Autorización</translation>
    </message>
    <message>
      <source>Grant</source>
      <translation>Permitir</translation>
    </message>
    <message>
      <source>Refuse</source>
      <translation>Rechazar</translation>
    </message>
    <message>
      <source>User Id:</source>
      <translation>Id de usuario:</translation>
    </message>
    <message>
      <source>%1 authorization to %2</source>
      <translation>%1 autorización a %2</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::AwayMsgDlg</name>
    <message>
      <source>&amp;Edit Items</source>
      <translation>&amp;Editar Elementos</translation>
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
      <source>&amp;Hints</source>
      <translation>Sugerencias</translation>
    </message>
    <message>
      <source>I'm currently %1, %a.
You can leave me a message.
(%m messages pending from you).</source>
      <translation>En estos momentos estoy %1, %a. 
Puedes dejarme un mensaje (tengo %m mensajes pendientes de ti).</translation>
    </message>
    <message>
      <source>(Closing in %1)</source>
      <translation>(Cerrando en %1)</translation>
    </message>
    <message>
      <source>&lt;h2>Hints for Setting&lt;br>your Auto-Response&lt;/h2>&lt;hr>&lt;ul>&lt;li>You can include any of the % expansions (described in the main hints page).&lt;/li>&lt;li>Any line beginning with a pipe (|) will be treated as a command to be run.  The line will be replaced by the output of the command. The command is parsed by /bin/sh so any shell commands or meta-characters are allowed.  For security reasons, any % expansions are automatically passed to the command surrounded by single quotes to prevent shell parsing of any meta-characters included in an alias.&lt;br>Examples of popular uses include:&lt;ul>&lt;li>&lt;tt>|date&lt;/tt>: Will replace that line by the current date&lt;/li>&lt;li>&lt;tt>|fortune&lt;/tt>: Show a fortune, as a tagline for example&lt;/li>&lt;li>&lt;tt>|myscript.sh %u %a&lt;/tt>: Run a script, passing the uin and alias&lt;/li>&lt;li>&lt;tt>|myscript.sh %u %a > /dev/null&lt;/tt>: Run the same script but ignore the output (for tracking auto response checks or something)&lt;/li>&lt;li>&lt;tt>|if [ %u -lt 100000 ]; then echo &quot;You are special&quot;; fi&lt;/tt>: Useless, but shows how you can use shell script.&lt;/li>&lt;/ul>Of course, multiple &quot;|&quot; can appear in the auto response, and commands and regular text can be mixed line by line.&lt;/li>&lt;hr>&lt;p> For more information, see the Licq webpage (&lt;tt>http://www.licq.org&lt;/tt>).&lt;/p></source>
      <translation>&lt;h2>Sugerencias para configurar&lt;br>su Respuesta Automática&lt;/h2>&lt;br>&lt;hr>&lt;br>&lt;ul>&lt;li>Puede incluir cualquier de las expansiones % (descriptas en la página principal de sugerencias).&lt;/li>&lt;li>Cualquier línea que comience con un pipe (|) será tratada como un comando a ejecutar. Dicha línea será reemplazada con el resultado del comando. El comando es ejecutado por /bin/sh de forma tal que cualquier comando de shell o meta-caracteres están permitidos. Por cuestiones de seguridad cualquiera de las expansiones % son pasadas automáticamente al comando encerradas en comillas simples para prevenir que el shell intente procesar cualquier meta-caracter contenido en un alias&lt;br>Algunos ejemplos de uso popular son:&lt;ul>&lt;li>&lt;tt>|date&lt;/tt>: Reemplaza la línea por la fecha actual&lt;/li>&lt;li>&lt;tt>|fortune&lt;/tt>:Muestra la fortuna, como un tag por ejemplo &lt;/li>&lt;li>&lt;tt>|myscript.sh %u %a&lt;/tt>: Ejecuta un script que recibe el uin y el alias como parámetros&lt;/li>&lt;li>&lt;tt>|myscript.sh %u %a > /dev/null&lt;/tt>: Ejecuta el mismo script, pero ignora la salida (para llevar la cuenta de la veces que se ha verificado la Respuesta Automáticamente, por ejemplo)&lt;/li>&lt;li>&lt;tt>|if [ %u -lt 100000 ]; then echo &quot;You are special&quot;; fi&lt;/tt>: No tiene aplicación práctica, pero demuestra el uso de un shell script.&lt;/li>&lt;/ul>Por supuesto, múltiples &quot;|&quot; pueden aparecer en la Respuesta Automática, y comandos y texto regular pueden ser mezclados línea por línea.&lt;/li>&lt;hr>&lt;p> Para más información, vea la página de Licq: (&lt;tt>http://www.licq.org&lt;/tt>).&lt;/p></translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::ChatDlg</name>
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
      <source>Unable to load encoding &lt;b>%1&lt;/b>. Message contents may appear garbled.</source>
      <translation>No se pudo cargar la codificación &lt;b>%1&lt;/b>. El contenido del mensaje puede ser ilegible.</translation>
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
&lt;--BEEP-->
</source>
      <translation>
&lt;--PITIDO--></translation>
    </message>
    <message>
      <source>> &lt;--BEEP-->
</source>
      <translation>> &lt;--PITIDO--></translation>
    </message>
    <message>
      <source>/%1.chat</source>
      <translation>/%1.charla</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::ContactBar</name>
    <message>
      <source>Online</source>
      <translation>En línea</translation>
    </message>
    <message>
      <source>Offline</source>
      <translation>Fuera de línea</translation>
    </message>
    <message>
      <source>Not In List</source>
      <translation>Fuera de la Lista</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::ContactListModel</name>
    <message>
      <source>Other Users</source>
      <translation>Otros Usuarios</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::ContactUserData</name>
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
      <translation>Teléfono &amp;quot;Sígame&amp;quot;: Disponible</translation>
    </message>
    <message>
      <source>Phone &amp;quot;Follow Me&amp;quot;: Busy</source>
      <translation>Teléfono &amp;quot;Sígame&amp;quot;: Ocupado</translation>
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
      <translation>Servidor de ficheros: Habilitado</translation>
    </message>
    <message>
      <source>Secure connection</source>
      <translation>Conexión segura</translation>
    </message>
    <message>
      <source>Custom Auto Response</source>
      <translation>Respuesta automática personalizada</translation>
    </message>
    <message>
      <source>Auto Response:</source>
      <translation>Respuesta automática</translation>
    </message>
    <message>
      <source>E: </source>
      <translation>E:</translation>
    </message>
    <message>
      <source>P: </source>
      <translation>T:</translation>
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
      <translation>En línea::</translation>
    </message>
    <message>
      <source>Idle: </source>
      <translation>Inactivo:</translation>
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
      <translation>Esperando autorización</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::CustomAutoRespDlg</name>
    <message>
      <source>Clear</source>
      <translation>Limpiar</translation>
    </message>
    <message>
      <source>Hints</source>
      <translation>Sugerencias</translation>
    </message>
    <message>
      <source>Set Custom Auto Response for %1</source>
      <translation>Configurar Respuesta Automática para %1</translation>
    </message>
    <message>
      <source>I am currently %1.
You can leave me a message.</source>
      <translation>Ahora estoy %1.
Puedes dejarme un mensaje.</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::DockIcon</name>
    <message>
      <source>&lt;br>Left click - Show main window&lt;br>Middle click - Show next message&lt;br>Right click - System menu</source>
      <translation>&lt;br>Click Izquierdo- Mostrar Ventana Principal&lt;br>Click Medio- Mostrar Mensaje Siguiente&lt;br>Click Derecho- Menú del Sistema</translation>
    </message>
    <message>
      <source>%1 system messages</source>
      <translation>%1 mensajes del sistema</translation>
    </message>
    <message>
      <source>%1 msgs</source>
      <translation>%1 msjs</translation>
    </message>
    <message>
      <source>1 msg</source>
      <translation>1 msj</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::EditCategoryDlg</name>
    <message>
      <source>Unspecified</source>
      <translation>No especificado</translation>
    </message>
    <message>
      <source>Personal Interests</source>
      <translation>Intereses personales</translation>
    </message>
    <message>
      <source>Organization, Affiliation, Group</source>
      <translation>Organización, Afiliación, Grupo</translation>
    </message>
    <message>
      <source>Past Background</source>
      <translation>Antecedentes</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::EditFileDlg</name>
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
    <message>
      <source>Revert</source>
      <translation>Revertir</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::EditFileListDlg</name>
    <message>
      <source>Licq - Files to send</source>
      <translation>Licq - Ficheros a enviar</translation>
    </message>
    <message>
      <source>D&amp;one</source>
      <translation>Hech&amp;o</translation>
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
      <source>D&amp;elete</source>
      <translation>&amp;Eliminar</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::EditGrpDlg</name>
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
the group '%1'?</source>
      <translation>¿Está seguro de que quiere eliminar 
el grupo '%1'?</translation>
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
      <translation>Valores Por Defecto</translation>
    </message>
    <message>
      <source>Default:</source>
      <translation>Por Defecto:</translation>
    </message>
    <message>
      <source>The default group to start up in.</source>
      <translation>El grupo en el que se empieza por defecto.</translation>
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
      <translation>Configurar Nuevos Usuarios</translation>
    </message>
    <message>
      <source>The group to which new users will be automatically added.  All new users will be in the local system group New Users but for server side storage will also be stored in the specified group.</source>
      <translation>El grupo al cual serán añadidos automáticamente los nuevos usuarios.  Todos los nuevos usuarios pertenecen al grupo del sistema Nuevos Usuarios, pero en el servidor también serán almacenados en el grupo especificado.</translation>
    </message>
    <message>
      <source>New User:</source>
      <translation>Nuevo Usuario:</translation>
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
    <name>LicqQtGui::EditPhoneDlg</name>
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
    <name>LicqQtGui::Emoticons</name>
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
    <name>LicqQtGui::EventDescription</name>
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
      <source>New Email Alert</source>
      <translation>Alerta de nuevo e-mail</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::FileDlg</name>
    <message>
      <source>Licq - File Transfer (%1)</source>
      <translation>Licq - Transferencia de Ficheros (%1)</translation>
    </message>
    <message>
      <source>Current:</source>
      <translation>Actual:</translation>
    </message>
    <message>
      <source>File name:</source>
      <translation>Fichero:</translation>
    </message>
    <message>
      <source>File:</source>
      <translation>Fichero:</translation>
    </message>
    <message>
      <source>Batch:</source>
      <translation>Lote:</translation>
    </message>
    <message>
      <source>Time:</source>
      <translation>Tiempo:</translation>
    </message>
    <message>
      <source>ETA:</source>
      <translation>T.Est.:</translation>
    </message>
    <message>
      <source>&amp;Cancel Transfer</source>
      <translation>&amp;Cancelar Transferencia</translation>
    </message>
    <message>
      <source>&amp;Open</source>
      <translation>Abrir</translation>
    </message>
    <message>
      <source>O&amp;pen Dir</source>
      <translation>Abrir directorio.</translation>
    </message>
    <message>
      <source>File transfer cancelled.</source>
      <translation>Transferencia cancelada</translation>
    </message>
    <message>
      <source>Close</source>
      <translation>Cerrar</translation>
    </message>
    <message>
      <source>%1/%2</source>
      <translation>%1/%2</translation>
    </message>
    <message>
      <source>Waiting for connection...</source>
      <translation>Esperando conexión...</translation>
    </message>
    <message>
      <source>Receiving file...</source>
      <translation>Recibiendo fichero...</translation>
    </message>
    <message>
      <source>Sending file...</source>
      <translation>Enviando fichero...</translation>
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
      <translation>No se puede abrir el puerto. 
Vea la Ventana de Red para más detalles</translation>
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
      <source>Connecting to remote...</source>
      <translation>Conectando...</translation>
    </message>
    <message>
      <source>MB</source>
      <translation>Mb</translation>
    </message>
    <message>
      <source>KB</source>
      <translation>Kb</translation>
    </message>
    <message>
      <source>Bytes</source>
      <translation>Bytes</translation>
    </message>
    <message>
      <source>Byte</source>
      <translation>Byte</translation>
    </message>
    <message>
      <source>File already exists and is at least as big as the incoming file.</source>
      <translation>El fichero ya existe y es al menos tan grande como el fichero que se transmitió</translation>
    </message>
    <message>
      <source>Overwrite</source>
      <translation>Sobreescribir</translation>
    </message>
    <message>
      <source>Cancel</source>
      <translation>Cancelar</translation>
    </message>
    <message>
      <source>File already exists and appears incomplete.</source>
      <translation>El fichero ya existe y parece incompleto.</translation>
    </message>
    <message>
      <source>Resume</source>
      <translation>Continuar</translation>
    </message>
    <message>
      <source>Open error - unable to open file for writing.</source>
      <translation>Error - no se pudo escribir en el fichero.</translation>
    </message>
    <message>
      <source>Retry</source>
      <translation>Reintentar</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::FloatyView</name>
    <message>
      <source>%1 Floaty (%2)</source>
      <translation>%1 Flotante (%2)</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::ForwardDlg</name>
    <message>
      <source>Message</source>
      <translation>Mensaje</translation>
    </message>
    <message>
      <source>URL</source>
      <translation>URL</translation>
    </message>
    <message>
      <source>Unable to forward this message type (%d).</source>
      <translation>No se pudo reenviar este tipo de mensaje (%d).</translation>
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
      <source>&amp;Forward</source>
      <translation>Reenviar</translation>
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
      <source>Forwarded URL:
</source>
      <translation>URL reenviada:
</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::GPGKeyManager</name>
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
      <source>Do you want to remove the GPG key? The key isn't deleted from your keyring.</source>
      <translation>Desea borrar la clave GPG? La clave no es borrada de su anillo de claves.</translation>
    </message>
    <message>
      <source>Drag&amp;Drop user to add to list.</source>
      <translation>Arrastre y suelte el usuario para añadirlo a la lista</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::GPGKeySelect</name>
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
      <source>&amp;No Key</source>
      <translation>Si&amp;n Clave</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::HintsDlg</name>
    <message>
      <source>Licq - Hints</source>
      <translation>Licq - Sugerencias</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::HistoryDlg</name>
    <message>
      <source>&amp;Previous day</source>
      <translation>Día anterior</translation>
    </message>
    <message>
      <source>&amp;Next day</source>
      <translation>Día siguie&amp;nte</translation>
    </message>
    <message>
      <source>Search</source>
      <translation>Buscar</translation>
    </message>
    <message>
      <source>Find:</source>
      <translation>Buscar:</translation>
    </message>
    <message>
      <source>Match &amp;case</source>
      <translation>Coincidir mayúsculas y minúsculas</translation>
    </message>
    <message>
      <source>&amp;Regular expression</source>
      <translation>Expresión &amp;regular</translation>
    </message>
    <message>
      <source>F&amp;ind previous</source>
      <translation>Buscar anterior</translation>
    </message>
    <message>
      <source>&amp;Find next</source>
      <translation>Buscar siguiente</translation>
    </message>
    <message>
      <source>&amp;Menu</source>
      <translation>&amp;Menú</translation>
    </message>
    <message>
      <source>INVALID USER</source>
      <translation>USUARIO NO VALIDO</translation>
    </message>
    <message>
      <source>Licq - History </source>
      <translation>Licq - Historial </translation>
    </message>
    <message>
      <source>Invalid user requested</source>
      <translation>Pedido de usuario no válido</translation>
    </message>
    <message>
      <source>Error loading history file: %1
Description: %2</source>
      <translation>Error cargando fichero de historial: %1
Descripción: %2</translation>
    </message>
    <message>
      <source>Sorry, history is disabled for this person</source>
      <translation>Disculpe, el historial está deshabilitado para este usuario</translation>
    </message>
    <message>
      <source>History is empty</source>
      <translation>El historial está vacío</translation>
    </message>
    <message>
      <source>server</source>
      <translation>servidor</translation>
    </message>
    <message>
      <source>Search returned no matches</source>
      <translation>La búsqueda retornó sin coincidencias</translation>
    </message>
    <message>
      <source>Search wrapped around</source>
      <translation>Búsqueda circular</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::HistoryView</name>
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
    <message>
      <source>from</source>
      <translation>de</translation>
    </message>
    <message>
      <source>to</source>
      <translation>a</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::InfoField</name>
    <message>
      <source>Unknown</source>
      <translation>Desconocido</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::JoinChatDlg</name>
    <message>
      <source>Select chat to invite:</source>
      <translation>Seleccionar charla para invitar:</translation>
    </message>
    <message>
      <source>Invite to Join Chat</source>
      <translation>Invitar a Unirse a la Charla</translation>
    </message>
    <message>
      <source>&amp;Invite</source>
      <translation>&amp;Invitar</translation>
    </message>
    <message>
      <source>&amp;Cancel</source>
      <translation>&amp;Cancelar</translation>
    </message>
    <message>
      <source>Select chat to join:</source>
      <translation>Seleccionar charla para unirse:</translation>
    </message>
    <message>
      <source>Join Multiparty Chat</source>
      <translation>Unirse a la Charla Múltiple</translation>
    </message>
    <message>
      <source>&amp;Join</source>
      <translation>Unirse</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::KeyListItem</name>
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
    <name>LicqQtGui::KeyRequestDlg</name>
    <message>
      <source>&amp;Send</source>
      <translation>Enviar</translation>
    </message>
    <message>
      <source>Closing secure channel...</source>
      <translation>Cerrando canal seguro...</translation>
    </message>
    <message>
      <source>Client does not support OpenSSL.
Rebuild Licq with OpenSSL support.</source>
      <translation>El cliente no soporta OpenSSL.
Vuelva a compilar Licq con soporte para OpenSSL.</translation>
    </message>
    <message>
      <source>Licq - Secure Channel with %1</source>
      <translation>Licq - Canal Seguro con %1</translation>
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
This probably won't work.</source>
      <translation>El lado remoto usa Licq %1, sin embargo no tiene habilitado el soporte para canales seguros. 
Esto probablemente no funcione.</translation>
    </message>
    <message>
      <source>This only works with other Licq clients >= v0.85
The remote doesn't seem to use such a client.
This might not work.</source>
      <translation>Esto sólo funciona con otros clientes Licq &lt;= v0.85 
El lado remoto no parece estar utilizando dicho cliente. 
Podría no funcionar.</translation>
    </message>
    <message>
      <source>Ready to close channel</source>
      <translation>Preparado para cerrar canal.</translation>
    </message>
    <message>
      <source>Ready to request channel</source>
      <translation>Preparado para pedir canal</translation>
    </message>
    <message>
      <source>Secure channel already established.</source>
      <translation>Canal seguro establecido previamente</translation>
    </message>
    <message>
      <source>Secure channel not established.</source>
      <translation>Canal seguro no establecido</translation>
    </message>
    <message>
      <source>Remote client does not support OpenSSL.</source>
      <translation>El cliente remoto no soporta OpenSSL</translation>
    </message>
    <message>
      <source>Could not connect to remote client.</source>
      <translation>NO se puede conectar con el cliente remoto</translation>
    </message>
    <message>
      <source>Secure channel established.</source>
      <translation>Canal seguro establecido</translation>
    </message>
    <message>
      <source>Secure channel closed.</source>
      <translation>Canal seguro cerrado</translation>
    </message>
    <message>
      <source>Unknown state.</source>
      <translation>Estado desconocido</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::KeyView</name>
    <message>
      <source>Name</source>
      <translation>Nombre</translation>
    </message>
    <message>
      <source>EMail</source>
      <translation>EMail</translation>
    </message>
    <message>
      <source>ID</source>
      <translation>ID</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::LicqGui</name>
    <message>
      <source>Are you sure you want to remove
%1 (%2)
from your contact list?</source>
      <translation>¿Está seguro de que quiere eliminar a
%1 (%2)
de su lista de contactos?</translation>
    </message>
    <message>
      <source>Are you sure you want to remove
%1 (%2)
from the '%3' group?</source>
      <translation>¿Está seguro de que quiere eliminar a
%1 (%2)
del grupo '%3'?</translation>
    </message>
    <message>
      <source>Licq is unable to start your browser and open the URL.
You will need to start the browser and open the URL manually.</source>
      <translation>Licq no ha podido iniciar su navegador y abrir la URL. 
Deberá iniciar el navegador y abrir la URL de forma manual.</translation>
    </message>
    <message>
      <source>There was an error loading the default configuration file.
Would you like to try loading the old one?</source>
      <translation>Hubo un error al cargar el fichero de configuración por defecto. 
Desea cargar el anterior?</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::LogWindow</name>
    <message>
      <source>Licq Network Log</source>
      <translation>Bitácora de Red de Licq</translation>
    </message>
    <message>
      <source>Failed to open file:
%1</source>
      <translation>Fallo al abrir archivo: 
%1</translation>
    </message>
    <message>
      <source>Clear</source>
      <translation>Limpiar</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::MLEdit</name>
    <message>
      <source>Allow Tabulations</source>
      <translation>Permitir Tabulaciones</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::MLView</name>
    <message>
      <source>Copy URL</source>
      <translation>Copiar URL</translation>
    </message>
    <message>
      <source>Quote</source>
      <translation>Cita</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::MMSendDlg</name>
    <message>
      <source>Multiple Recipient Message</source>
      <translation>Mensaje a Múltiples Destinatarios</translation>
    </message>
    <message>
      <source>Multiple Recipient URL</source>
      <translation>URL a Múltiples Destinatarios</translation>
    </message>
    <message>
      <source>Multiple Recipient Contact List</source>
      <translation>Lista de Contactos a Múltiples Destinatarios.</translation>
    </message>
    <message>
      <source>failed</source>
      <translation>falló</translation>
    </message>
    <message>
      <source>&amp;Close</source>
      <translation>&amp;Cerrar</translation>
    </message>
    <message>
      <source>Sending mass message to %1...</source>
      <translation>Enviando mensaje masivo a %1...</translation>
    </message>
    <message>
      <source>Sending mass URL to %1...</source>
      <translation>Enviando URL masiva a %1...</translation>
    </message>
    <message>
      <source>Sending mass list to %1...</source>
      <translation>Enviando lista masiva a %1</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::MMUserView</name>
    <message>
      <source>Remove</source>
      <translation>Eliminar</translation>
    </message>
    <message>
      <source>Crop</source>
      <translation>Recortar</translation>
    </message>
    <message>
      <source>Clear</source>
      <translation>Limpiar</translation>
    </message>
    <message>
      <source>Add Group</source>
      <translation>Añadir Grupo</translation>
    </message>
    <message>
      <source>Add All</source>
      <translation>Añadir Todos</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::MainWindow</name>
    <message>
      <source>&lt;ul>&lt;li>&lt;tt>%a - &lt;/tt>user alias&lt;/li>&lt;li>&lt;tt>%e - &lt;/tt>email&lt;/li>&lt;li>&lt;tt>%f - &lt;/tt>first name&lt;/li>&lt;li>&lt;tt>%h - &lt;/tt>phone number&lt;/li>&lt;li>&lt;tt>%i - &lt;/tt>user ip&lt;/li>&lt;li>&lt;tt>%l - &lt;/tt>last name&lt;/li>&lt;li>&lt;tt>%L - &lt;/tt>local time&lt;/li>&lt;li>&lt;tt>%m - &lt;/tt># pending messages&lt;/li>&lt;li>&lt;tt>%M - &lt;/tt># pending messages (if any)&lt;/li>&lt;li>&lt;tt>%n - &lt;/tt>full name&lt;/li>&lt;li>&lt;tt>%o - &lt;/tt>last seen online&lt;/li>&lt;li>&lt;tt>%O - &lt;/tt>online since&lt;/li>&lt;li>&lt;tt>%p - &lt;/tt>user port&lt;/li>&lt;li>&lt;tt>%s - &lt;/tt>full status&lt;/li>&lt;li>&lt;tt>%S - &lt;/tt>abbreviated status&lt;/li>&lt;li>&lt;tt>%u - &lt;/tt>uin&lt;/li>&lt;li>&lt;tt>%w - &lt;/tt>webpage&lt;/li>&lt;/ul></source>
      <translation>&lt;ul>&lt;li>&lt;tt>%a - &lt;/tt>Apodo de usuario&lt;/li>&lt;li>&lt;tt>%e - &lt;/tt>email&lt;/li>&lt;li>&lt;tt>%f - &lt;/tt>Nombre&lt;/li>&lt;li>&lt;tt>%h - &lt;/tt>Número de teléfono&lt;/li>&lt;li>&lt;tt>%i - &lt;/tt>ip del usuario&lt;/li>&lt;li>&lt;tt>%l - &lt;/tt>Apellido&lt;/li>&lt;li>&lt;tt>%L - &lt;/tt>hora local&lt;/li>&lt;li>&lt;tt>%m - &lt;/tt># mensajes pendientes&lt;/li>&lt;li>&lt;tt>%n - &lt;/tt>Nombre completo&lt;/li>&lt;li>&lt;tt>%o - &lt;/tt>última vez visto en línea&lt;/li>&lt;li>&lt;tt>%O - &lt;/tt>en línea desde&lt;/li>&lt;li>&lt;tt>%p - &lt;/tt>puerto del usuario&lt;/li>&lt;li>&lt;tt>%s - &lt;/tt>estado completo&lt;/li>&lt;li>&lt;tt>%S - &lt;/tt>estado abreviado&lt;/li>&lt;li>&lt;tt>%u - &lt;/tt>uin&lt;/li>&lt;li>&lt;tt>%w - &lt;/tt>sitio web&lt;/li>&lt;/ul></translation>
    </message>
    <message>
      <source>System</source>
      <translation>Sistema</translation>
    </message>
    <message>
      <source>&amp;System</source>
      <translation>&amp;Sistema</translation>
    </message>
    <message>
      <source>Right click - User groups
Double click - Show next message</source>
      <translation>Botón Derecho - Grupos de usuarios
Doble Pulsación - Mostrar mensaje siguiente</translation>
    </message>
    <message>
      <source>Right click - Status menu
Double click - Set auto response</source>
      <translation>Botón Derecho - Menú de Estado
Doble Pulsación - Configurar respuesta automática</translation>
    </message>
    <message>
      <source>Error! No owner set</source>
      <translation>Error! Dueño no asignado</translation>
    </message>
    <message>
      <source>SysMsg</source>
      <translation>Mensaje del Sistema</translation>
    </message>
    <message>
      <source>System Message</source>
      <translation>Mensaje del Sistema</translation>
    </message>
    <message>
      <source>%1 msg%2</source>
      <translation>%1 msj%2</translation>
    </message>
    <message>
      <source></source>
      <translation> </translation>
    </message>
    <message>
      <source>s</source>
      <translation>s</translation>
    </message>
    <message>
      <source>%1 message%2</source>
      <translation>%1 mensaje%2</translation>
    </message>
    <message>
      <source>No msgs</source>
      <translation>Sin mensajes</translation>
    </message>
    <message>
      <source>No messages</source>
      <translation>Sin mensajes</translation>
    </message>
    <message>
      <source>Logon failed.
See network window for details.</source>
      <translation>Falló la Identificación.
Para más detalles, mire en la Ventana de Red.</translation>
    </message>
    <message>
      <source>&lt;h2>Hints for Using&lt;br>the Licq Qt-GUI Plugin&lt;/h2>&lt;hr>&lt;ul>&lt;li>Change your status by right clicking on the status label.&lt;/li>&lt;li>Change your auto response by double-clicking on the status label.&lt;/li>&lt;li>View system messages by double clicking on the message label.&lt;/li>&lt;li>Change groups by right clicking on the message label.&lt;/li>&lt;li>Use the following shortcuts from the contact list:&lt;ul>&lt;li>&lt;tt>Ctrl-M : &lt;/tt>Toggle mini-mode&lt;/li>&lt;li>&lt;tt>Ctrl-O : &lt;/tt>Toggle show offline users&lt;/li>&lt;li>&lt;tt>Ctrl-X : &lt;/tt>Exit&lt;/li>&lt;li>&lt;tt>Ctrl-H : &lt;/tt>Hide&lt;/li>&lt;li>&lt;tt>Ctrl-I : &lt;/tt>View the next message&lt;/li>&lt;li>&lt;tt>Ctrl-V : &lt;/tt>View message&lt;/li>&lt;li>&lt;tt>Ctrl-S : &lt;/tt>Send message&lt;/li>&lt;li>&lt;tt>Ctrl-U : &lt;/tt>Send Url&lt;/li>&lt;li>&lt;tt>Ctrl-C : &lt;/tt>Send chat request&lt;/li>&lt;li>&lt;tt>Ctrl-F : &lt;/tt>Send File&lt;/li>&lt;li>&lt;tt>Ctrl-A : &lt;/tt>Check Auto response&lt;/li>&lt;li>&lt;tt>Ctrl-P : &lt;/tt>Popup all messages&lt;/li>&lt;li>&lt;tt>Ctrl-L : &lt;/tt>Redraw user window&lt;/li>&lt;li>&lt;tt>Delete : &lt;/tt>Delete user from current group&lt;/li>&lt;li>&lt;tt>Ctrl-Delete : &lt;/tt>Delete user from contact list&lt;/li>&lt;/ul>&lt;li>Hold control while clicking on close in the function window to remove   the user from your contact list.&lt;/li>&lt;li>Hit Ctrl-Enter from most text entry fields to select &quot;Ok&quot; or &quot;Accept&quot;.   For example in the send tab of the user function window.&lt;/li>&lt;li>Here is the complete list of user % options, which can be used in &lt;b>OnEvent&lt;/b>   parameters, &lt;b>auto responses&lt;/b>, and &lt;b>utilities&lt;/b>:</source>
      <translation>&lt;h2>Sugerencias para Usar&lt;br>el Añadido Licq Qt-GUI&lt;/h2>&lt;br>&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;u estado pulsando con el botón derecho en la etiqueta de estado.&lt;/li>&lt;li>Cambie su respuesta automática haciendo doble click en la etiqueta de estado.&lt;/li>&lt;li>Vea los mensajes del sistema haciendo doble click en la etiqueta de mensajes.&lt;/li>&lt;li>Cambie de grupo pulsando con el botón derecho en la etiqueta de mensajes.&lt;/li>&lt;li>Los siguientes atajos de teclado están disponibles desde la lista de contactos:&lt;ul>&lt;li>&lt;tt>Ctrl-M : &lt;/tt>(Des)Activar modo en miniatura&lt;/li>&lt;li>&lt;tt>Ctrl-O : &lt;&lt;Mostrar (o no) los usuarios desconectados&lt;/li>&lt;li>&lt;tt>Ctrl-X : &lt;/tt>Salir&lt;/li>&lt;li>&lt;tt>Ctrl-H : &lt;/tt>Ocultar&lt;/li>&lt;li>&lt;tt>Ctrl-I : &lt;/tt>Ver el siguiente mensaje&lt;/li>&lt;li>&lt;tt>Ctrl-V : &lt;/tt>Ver mensaje&lt;/li>&lt;li>&lt;tt>Ctrl-S : &lt;&lt;Enviar mensaje&lt;/li>&lt;li>&lt;tt>Ctrl-U : &lt;/tt>Enviar Url&lt;/li>&lt;li>&lt;tt>Ctrl-C : &lt;/tt>Enviar petición de charla&lt;/li>&lt;li>&lt;tt>Ctrl-F : &lt;/tt>Enviar fichero&lt;/li>&lt;li>&lt;tt>Ctrl-A : &lt;/tt>Ver respuesta automática&lt;/li>&lt;li>&lt;tt>Ctrl-P : &lt;/tt>Elevar todos los mensajes&lt;/li>&lt;li>&lt;tt>Ctrl-L : &lt;/tt>Redibujar ventana de usuario&lt;/li>&lt;li>&lt;tt>Supr : &lt;/tt>Borrar al usuario del grupo actual&lt;/li>&lt;li>&lt;tt>Ctrl-Supr : &lt;/tt>Borrar al usuario de la lista de contactos&lt;/li>&lt;/ul>&lt;li>Mantenga pulsada la tecla control al cerrar la ventana de funciones para   eliminar al usuario de su lista de contactos.&lt;/li>&lt;li>Pulse Ctrl-Enter en la mayoría de campos de texto para elegir &amp;quot;Si&amp;quot; o &amp;quot;Aceptar&amp;quot;.   Por ejemplo en el apartado de envío de la ventana de funciones.&lt;/li>&lt;li>Esta es la lista completa de opciones % de usuario, que puede usar para los parámetros   de &lt;b>Eventos&lt;/b>, &lt;b>respuestas automáticas&lt;/b> y &lt;b>utilidades&lt;/b>:
</translation>
    </message>
    <message>
      <source>&lt;hr>&lt;p> For more information, see the Licq webpage (&lt;tt>http://www.licq.org&lt;/tt>).&lt;/p></source>
      <translation>&lt;hr>&lt;p> Para más información, visite la página web de Licq (&lt;tt>http://www.licq.org&lt;/tt>).&lt;/p></translation>
    </message>
    <message>
      <source>is online</source>
      <translation>está en línea</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::MessageBox</name>
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
      <translation>Siguie&amp;nte</translation>
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
      <translation>Siguie&amp;nte (%1)</translation>
    </message>
    <message>
      <source>Licq Information</source>
      <translation>Información de Licq</translation>
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
    <name>LicqQtGui::MessageList</name>
    <message>
      <source>D</source>
      <translation>D</translation>
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
      <source>Time</source>
      <translation>Hora</translation>
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
    <name>LicqQtGui::OptionsDlg</name>
    <message>
      <source>&lt;p>Available custom date format variables.&lt;/p>&lt;table>&lt;tr>&lt;th>Expression&lt;/th>&lt;th>Output&lt;/th>&lt;/tr>&lt;tr>&lt;td>d&lt;/td>&lt;td>the day as number without a leading zero (1-31)&lt;/td>&lt;/tr>&lt;tr>&lt;td>dd&lt;/td>&lt;td>the day as number with a leading zero (01-31)&lt;/td>&lt;/tr>&lt;tr>&lt;td>ddd&lt;/td>&lt;td>the abbreviated localized day name (e.g. 'Mon'..'Sun')&lt;/td>&lt;/tr>&lt;tr>&lt;td>dddd&lt;/td>&lt;td>the long localized day name (e.g. 'Monday'..'Sunday')&lt;/td>&lt;/tr>&lt;tr>&lt;td>M&lt;/td>&lt;td>the month as number without a leading zero (1-12)&lt;/td>&lt;/tr>&lt;tr>&lt;td>MM&lt;/td>&lt;td>the month as number with a leading zero (01-12)&lt;/td>&lt;/tr>&lt;tr>&lt;td>MMM&lt;/td>&lt;td>the abbreviated localized month name (e.g. 'Jan'..'Dec')&lt;/td>&lt;/tr>&lt;tr>&lt;td>MMMM&lt;/td>&lt;td>the long localized month name (e.g. 'January'..'December')&lt;/td>&lt;/tr>&lt;tr>&lt;td>yy&lt;/td>&lt;td>the year as two digit number (00-99)&lt;/td>&lt;/tr>&lt;tr>&lt;td>yyyy&lt;/td>&lt;td>the year as four digit number (1752-8000)&lt;/td>&lt;/tr>&lt;tr>&lt;td colspan=2>&lt;/td>&lt;/tr>&lt;tr>&lt;td>h&lt;/td>&lt;td>the hour without a leading zero (0..23 or 1..12 if AM/PM display)&lt;/td>&lt;/tr>&lt;tr>&lt;td>hh&lt;/td>&lt;td>the hour with a leading zero (00..23 or 01..12 if AM/PM display)&lt;/td>&lt;/tr>&lt;tr>&lt;td>m&lt;/td>&lt;td>the minute without a leading zero (0..59)&lt;/td>&lt;/tr>&lt;tr>&lt;td>mm&lt;/td>&lt;td>the minute with a leading zero (00..59)&lt;/td>&lt;/tr>&lt;tr>&lt;td>s&lt;/td>&lt;td>the second without a leading zero (0..59)&lt;/td>&lt;/tr>&lt;tr>&lt;td>ss&lt;/td>&lt;td>the second with a leading zero (00..59)&lt;/td>&lt;/tr>&lt;tr>&lt;td>z&lt;/td>&lt;td>the millisecond without leading zero (0..999)&lt;/td>&lt;/tr>&lt;tr>&lt;td>zzz&lt;/td>&lt;td>the millisecond with leading zero (000..999)&lt;/td>&lt;/tr>&lt;tr>&lt;td>AP&lt;/td>&lt;td>use AM/PM display. AP will be replaced by either 'AM' or 'PM'&lt;/td>&lt;/tr>&lt;tr>&lt;td>ap&lt;/td>&lt;td>use am/pm display. ap will be replaced by either 'am' or 'pm'&lt;/td>&lt;/tr>&lt;/table></source>
      <translation>&lt;p>Variables disponibles para personalizar fecha y hora.&lt;/p>
&lt;table>
&lt;tr>&lt;th>Expresión&lt;/th>&lt;th>Salida&lt;/th>&lt;/tr>
&lt;tr>&lt;td>d&lt;/td>&lt;td>el día como número sin cero al principio (1-31)&lt;/td>&lt;/tr>
&lt;tr>&lt;td>dd&lt;/td>&lt;td>el día como número con cero al principio (01-31)&lt;/td>&lt;/tr>
&lt;tr>&lt;td>ddd&lt;/td>&lt;td>el nombre del día abreviado (p.ej. 'Lun'..'Dom')&lt;/td>&lt;/tr>
&lt;tr>&lt;td>dddd&lt;/td>&lt;td>el nombre del día (p.ej. 'Lunes'..'Domingo')&lt;/td>&lt;/tr>
&lt;tr>&lt;td>M&lt;/td>&lt;td>el mes como número sin cero al principio (1-12)&lt;/td>&lt;/tr>
&lt;tr>&lt;td>MM&lt;/td>&lt;td>el mes como número con cero al principio (01-12)&lt;/td>&lt;/tr>
&lt;tr>&lt;td>MMM&lt;/td>&lt;td>el nombre del mes abreviado (p.ej. 'Ene'..'Dic')&lt;/td>&lt;/tr>
&lt;tr>&lt;td>MMMM&lt;/td>&lt;td>el nombre del mes (p.ej. 'Enero'..'Diciembre')&lt;/td>&lt;/tr>
&lt;tr>&lt;td>yy&lt;/td>&lt;td>el año en dos dígitos (00-99)&lt;/td>&lt;/tr>
&lt;tr>&lt;td>yyyy&lt;/td>&lt;td>el año en cuatro dígitos (1752-8000)&lt;/td>&lt;/tr>
&lt;tr>&lt;td colspan=2>&lt;/td>&lt;/tr>
&lt;tr>&lt;td>h&lt;/td>&lt;td>la hora sin cero al principio (0..23 o 1..12 si se muestra en formato AM/PM)&lt;/td>&lt;/tr>
&lt;tr>&lt;td>hh&lt;/td>&lt;td>la hora con cero al principio (00..23 o 01..12 si se muestra en formato AM/PM)&lt;/td>&lt;/tr>
&lt;tr>&lt;td>m&lt;/td>&lt;td>los minutos sin cero al principio (0..59)&lt;/td>&lt;/tr>
&lt;tr>&lt;td>mm&lt;/td>&lt;td>los minutos sin cero al principio (00..59)&lt;/td>&lt;/tr>
&lt;tr>&lt;td>s&lt;/td>&lt;td>los segundos sin cero al principio (0..59)&lt;/td>&lt;/tr>
&lt;tr>&lt;td>ss&lt;/td>&lt;td>los segundos con cero al principio (00..59)&lt;/td>&lt;/tr>
&lt;tr>&lt;td>z&lt;/td>&lt;td>los milisegundos sin cero al principio (0..999)&lt;/td>&lt;/tr>
&lt;tr>&lt;td>zzz&lt;/td>&lt;td>los milisegundos sin cero al principio (000..999)&lt;/td>&lt;/tr>
&lt;tr>&lt;td>AP&lt;/td>&lt;td>mostrar en formato AM/PM. AP será reemplazado por 'AM' o 'PM'&lt;/td>&lt;/tr>
&lt;tr>&lt;td>ap&lt;/td>&lt;td>mostrar en formato am/pm. ap será reemplazado por 'am' o 'pm'&lt;/td>&lt;/tr>
&lt;/table></translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::OwnerEditDlg</name>
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
      <translation>Sólo una cuenta por protocolo está soportada.</translation>
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
      <source>Save Password</source>
      <translation>Guardar contraseña</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::OwnerManagerDlg</name>
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
      <source>Protocol</source>
      <translation>Protocolo</translation>
    </message>
    <message>
      <source>User ID</source>
      <translation>ID de Usuario</translation>
    </message>
    <message>
      <source>(Invalid Protocol)</source>
      <translation>(Protocolo No Válido)</translation>
    </message>
    <message>
      <source>(Invalid ID)</source>
      <translation>(ID Inválido)</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::PluginDlg</name>
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
    <message>
      <source>Plugin Manager</source>
      <translation>Manejador de añadidos</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::RandomChatDlg</name>
    <message>
      <source>Random Chat Search</source>
      <translation>Búsqueda de Charla Aleatoria</translation>
    </message>
    <message>
      <source>&amp;Search</source>
      <translation>Bu&amp;scar</translation>
    </message>
    <message>
      <source>General</source>
      <translation>General</translation>
    </message>
    <message>
      <source>Romance</source>
      <translation>Romance</translation>
    </message>
    <message>
      <source>Games</source>
      <translation>Juegos</translation>
    </message>
    <message>
      <source>Students</source>
      <translation>Estudiantes</translation>
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
      <source>50 Plus</source>
      <translation>Más de cincuenta</translation>
    </message>
    <message>
      <source>Seeking Women</source>
      <translation>Buscando Mujeres</translation>
    </message>
    <message>
      <source>Seeking Men</source>
      <translation>Buscando Hombres</translation>
    </message>
    <message>
      <source>Searching for Random Chat Partner...</source>
      <translation>Buscando Compañero/a para Charla Aleatoria...</translation>
    </message>
    <message>
      <source>No random chat user found in that group.</source>
      <translation>No se encontraron usuarios para charlas libres en ese grupo.</translation>
    </message>
    <message>
      <source>Random chat search timed out.</source>
      <translation>Tiempo de espera excedido durante la búsqueda de charla aleatoria.</translation>
    </message>
    <message>
      <source>Random chat search had an error.</source>
      <translation>Error durante la búsqueda de charla aleatoria.</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::RefuseDlg</name>
    <message>
      <source>Refusal message for %1 with </source>
      <translation>Mensaje para %1 rechazado con </translation>
    </message>
    <message>
      <source>Refuse</source>
      <translation>Rechazar</translation>
    </message>
    <message>
      <source>Cancel</source>
      <translation>Cancelar</translation>
    </message>
    <message>
      <source>Licq %1 Refusal</source>
      <translation>Rechazo Licq %1</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::RegisterUserDlg</name>
    <message>
      <source>&amp;Remember Password</source>
      <translation>&amp;Recordar Contraseña</translation>
    </message>
    <message>
      <source>Register Account</source>
      <translation>Registrar cuenta</translation>
    </message>
    <message>
      <source>Introduction</source>
      <translation>Introducción</translation>
    </message>
    <message>
      <source>Welcome to the Registration Wizard.

You can register a new ICQ account here.

Press &quot;Next&quot; to proceed.</source>
      <translation>Bienvenido al Asistente de Registro. 

Aquí puedes registrar tu nueva cuenta de ICQ. 

Presiona \&quot;siguiente\&quot; para continuar.</translation>
    </message>
    <message>
      <source>Select password</source>
      <translation>Seleccione la contraseña</translation>
    </message>
    <message>
      <source>Specify a password for your account.
Length must be 1 to 8 characters.</source>
      <translation>Especifique la contraseña para su cuenta. 
El tamaño debe ser de 1 a 8 caracteres.</translation>
    </message>
    <message>
      <source>&amp;Password:</source>
      <translation>Contraseña:</translation>
    </message>
    <message>
      <source>&amp;Verify:</source>
      <translation>&amp;Verificar:</translation>
    </message>
    <message>
      <source>Account Verification</source>
      <translation>Verificación de la cuenta</translation>
    </message>
    <message>
      <source>Retype the letters shown in the image.</source>
      <translation>Escriba nuevamente las letras mostradas en la imagen</translation>
    </message>
    <message>
      <source>&amp;Verification:</source>
      <translation>&amp;Verificación:</translation>
    </message>
    <message>
      <source>Registration Completed</source>
      <translation>Registro finalizado</translation>
    </message>
    <message>
      <source>Account registration has been successfuly completed.</source>
      <translation>Se ha completado el registro con éxito.</translation>
    </message>
    <message>
      <source>Your new user Id:</source>
      <translation>Su nuevo Id de usuario:</translation>
    </message>
    <message>
      <source>You are now being automatically logged on.
Click Finish to edit your personal details.
After you are online, you can send your personal details to the server.</source>
      <translation>Usted está siendo conectado automáticamente. 
Presione Finalizar para editar sus datos personales. 
Una vez que esté en línea, podrá enviar sus detalles personales al servidor.</translation>
    </message>
    <message>
      <source>Passwords don't match.</source>
      <translation>Las contraseñas no coinciden.</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::ReqAuthDlg</name>
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
  </context>
  <context>
    <name>LicqQtGui::SearchUserDlg</name>
    <message>
      <source>Reset Search</source>
      <translation>Reiniciar Búsqueda</translation>
    </message>
    <message>
      <source>Enter search parameters and select 'Search'</source>
      <translation>Introduzca los parámetros de la búsqueda y pulse 'Buscar'</translation>
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
      <translation>Nombre de la Compañía:</translation>
    </message>
    <message>
      <source>Company Department:</source>
      <translation>Departamento de la Compañía:</translation>
    </message>
    <message>
      <source>Company Position:</source>
      <translation>Posición en la Compañía:</translation>
    </message>
    <message>
      <source>Keyword:</source>
      <translation>Palabra Clave:</translation>
    </message>
    <message>
      <source>Return Online Users Only</source>
      <translation>Retornar Sólo Usuarios En línea</translation>
    </message>
    <message>
      <source>%1 more users found. Narrow search.</source>
      <translation>%1 usuarios más encontrados. Limite su búsqueda.</translation>
    </message>
    <message>
      <source>View &amp;Info</source>
      <translation>Ver &amp;información</translation>
    </message>
    <message>
      <source>Search complete.</source>
      <translation>Búsqueda completada</translation>
    </message>
    <message>
      <source>Alias</source>
      <translation>Alias</translation>
    </message>
    <message>
      <source>UIN</source>
      <translation>UIN</translation>
    </message>
    <message>
      <source>Name</source>
      <translation>Nombre</translation>
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
      <source>Offline</source>
      <translation>Fuera de línea</translation>
    </message>
    <message>
      <source>Online</source>
      <translation>En Línea</translation>
    </message>
    <message>
      <source>Unknown</source>
      <translation>Desconocido</translation>
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
    <message>
      <source>Search Criteria</source>
      <translation>Criterio de búsqueda</translation>
    </message>
    <message>
      <source>UIN:</source>
      <translation>UIN:</translation>
    </message>
    <message>
      <source>Result</source>
      <translation>Resultado</translation>
    </message>
    <message>
      <source>A/G</source>
      <translation>A/G</translation>
    </message>
    <message>
      <source>Auth</source>
      <translation>Autoriz</translation>
    </message>
    <message>
      <source>Close</source>
      <translation>Cerrar</translation>
    </message>
    <message>
      <source>New Search</source>
      <translation>Nueva búsqueda</translation>
    </message>
    <message>
      <source>Search interrupted</source>
      <translation>Busqueda interrumpida</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::SecurityDlg</name>
    <message>
      <source>error</source>
      <translation>error</translation>
    </message>
    <message>
      <source>ICQ Security Options</source>
      <translation>Opciones de Seguridad de ICQ</translation>
    </message>
    <message>
      <source>You need to be connected to the
ICQ Network to change the settings.</source>
      <translation>Necesita estar conectado a la Red ICQ
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
      <source>&amp;Update</source>
      <translation>Act&amp;ualizar</translation>
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
      <source>&amp;Verify:</source>
      <translation>&amp;Verificar:</translation>
    </message>
    <message>
      <source>&amp;Authorization Required</source>
      <translation>Requiere &amp;Autorización</translation>
    </message>
    <message>
      <source>&amp;Web Presence</source>
      <translation>Presencia &amp;Web</translation>
    </message>
    <message>
      <source>&amp;Hide IP</source>
      <translation>Ocultar IP</translation>
    </message>
    <message>
      <source>Determines whether regular ICQ clients require
your authorization to add you to their contact list.</source>
      <translation>Indica si otros usuarios requieren de tu autorización 
para agregarte a su lista de contacto.</translation>
    </message>
    <message>
      <source>Web Presence allows users to see
if you are online through your web indicator.</source>
      <translation>La Presencia Web permite ver si estás conectado 
a través de un indicador web</translation>
    </message>
    <message>
      <source>Hide IP stops users from seeing your IP address.
It doesn't guarantee it will be hidden though.</source>
      <translation>Ocultar IP evita que otros usuarios vean tu dirección IP. 
Esto no garantiza que la IP no será visible.</translation>
    </message>
    <message>
      <source>Enter the UIN which you want to use.
Only available if &quot;Local changes only&quot; is checked.</source>
      <translation>Ingrese el UIN que desea utilizar. 
Esta opción sólo está disponible si la opción\&quot;sólo cambios locales\&quot; está seleccionada</translation>
    </message>
    <message>
      <source>If checked, password/UIN changes will apply only on your local computer.
Useful if your password is incorrectly saved in Licq.</source>
      <translation>Si está seleccionada, los cambios de contraseña/UIN sólo serán aplicados en su ordenador local. 
Útil si su contraseña está guardada incorrectamente en Licq.</translation>
    </message>
    <message>
      <source>Failed to change the settings.</source>
      <translation>Fallo al cambiar la configuración.</translation>
    </message>
    <message>
      <source>Setting...</source>
      <translation>Configuración...</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::SetRandomChatGroupDlg</name>
    <message>
      <source>Set Random Chat Group</source>
      <translation>Elegir Grupo de Charla Aleatoria</translation>
    </message>
    <message>
      <source>&amp;Set</source>
      <translation>Configurar</translation>
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
      <source>General</source>
      <translation>General</translation>
    </message>
    <message>
      <source>Romance</source>
      <translation>Romance</translation>
    </message>
    <message>
      <source>Games</source>
      <translation>Juegos</translation>
    </message>
    <message>
      <source>Students</source>
      <translation>Estudiantes</translation>
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
      <source>50 Plus</source>
      <translation>Más de cincuenta</translation>
    </message>
    <message>
      <source>Seeking Women</source>
      <translation>Buscando Mujeres</translation>
    </message>
    <message>
      <source>Seeking Men</source>
      <translation>Buscando Hombres</translation>
    </message>
    <message>
      <source>&amp;Cancel</source>
      <translation>&amp;Cancelar</translation>
    </message>
    <message>
      <source>Setting Random Chat Group...</source>
      <translation>Configurando Grupo de Charla Aleatoria</translation>
    </message>
    <message>
      <source>failed</source>
      <translation>falló</translation>
    </message>
    <message>
      <source>timed out</source>
      <translation>expiró el tiempo</translation>
    </message>
    <message>
      <source>error</source>
      <translation>error</translation>
    </message>
    <message>
      <source>done</source>
      <translation>hecho</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::Settings::Chat</name>
    <message>
      <source>Chat</source>
      <translation>Charla</translation>
    </message>
    <message>
      <source>Chat Display</source>
      <translation>Muestra de la charla</translation>
    </message>
    <message>
      <source>History Display</source>
      <translation>Muestra del historial</translation>
    </message>
    <message>
      <source>General Chat Options</source>
      <translation>Opciones Generales de Charla</translation>
    </message>
    <message>
      <source>Chatmode messageview</source>
      <translation>Vista de mensaje en modo Charla</translation>
    </message>
    <message>
      <source>Show the current chat history in Send Window</source>
      <translation>Mostrar el historial de la conversación actual en la Ventana de Envío</translation>
    </message>
    <message>
      <source>Tabbed chatting</source>
      <translation>Charla con Tabs</translation>
    </message>
    <message>
      <source>Use tabs in Send Window</source>
      <translation>Usar Tabs en la Ventana de Envíos</translation>
    </message>
    <message>
      <source>Single line chat mode</source>
      <translation>Modo de Charla de una sola línea</translation>
    </message>
    <message>
      <source>Send messages with Enter and insert new lines with Ctrl+Enter, opposite of the normal mode</source>
      <translation>Enviar mensajes al presionar enter e insertar nuevas líneas con Ctrl+Enter, contrariamente al modo normal</translation>
    </message>
    <message>
      <source>Use double return</source>
      <translation>Usar doble retorno</translation>
    </message>
    <message>
      <source>Hitting Return twice will be used instead of Ctrl+Return
to send messages and close input dialogs.
Multiple new lines can be inserted with Ctrl+Return.</source>
      <translation>Presione Enter dos veces en lugar de Ctrl+Enter 
para enviar mensajes y cerrar los diálogos. 
Para insertar varias líneas presione Ctrl+Enter.</translation>
    </message>
    <message>
      <source>Show Send/Close buttons</source>
      <translation>Mostrar los botones Enviar/Cerrar</translation>
    </message>
    <message>
      <source>Show Send and Close buttons in the chat dialog.</source>
      <translation>Muestra los botones Enviar y Cerrar en la ventana de conversación.</translation>
    </message>
    <message>
      <source>Sticky message window(s)</source>
      <translation>Ventana(s) de Mensajes Pegajosa</translation>
    </message>
    <message>
      <source>Makes the message window(s) visible on all desktops</source>
      <translation>Hace la(s) ventana(s) visible en todos los escritorios</translation>
    </message>
    <message>
      <source>Auto close function window</source>
      <translation>Cerrar automáticamente la ventana de funciones</translation>
    </message>
    <message>
      <source>Auto close the user function window after a successful event</source>
      <translation>Cerrar automáticamente la ventana de funciones de usuario luego de un evento exitoso.</translation>
    </message>
    <message>
      <source>Check clipboard For URIs/files</source>
      <translation>Buscar URIs/Ficheros en el portapapeles</translation>
    </message>
    <message>
      <source>When double-clicking on a user to send a message check for urls/files in the clipboard</source>
      <translation>Cuando se hace doble-click sobre un usuario para enviarle un mensaje, buscar urls/ficheros en el portapapeles</translation>
    </message>
    <message>
      <source>Auto position the reply window</source>
      <translation>Autoposicionar la ventana de respuesta</translation>
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
      <source>Send typing notifications</source>
      <translation>Enviar notificación de tipeo</translation>
    </message>
    <message>
      <source>Send a notification to the user so they can see when you are typing a message to them</source>
      <translation>Envía una notificación al usuario para que sepa que usted a tipeando un mensaje</translation>
    </message>
    <message>
      <source>Localization</source>
      <translation>Localización</translation>
    </message>
    <message>
      <source>Default encoding:</source>
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
      <source>Show all available encodings in the User Encoding selection menu.
Normally, this menu shows only commonly used encodings.</source>
      <translation>Mostrar todas las codificaciones disponibles en el menú de selección de Codificación de Usuario. 
Normalmente sólo se mostrarán las codificación usadas comúnmente.</translation>
    </message>
    <message>
      <source>Extensions</source>
      <translation>Extensiones</translation>
    </message>
    <message>
      <source>URI viewer:</source>
      <translation>Visor de URIs:</translation>
    </message>
    <message>
      <source>The command to run to view a URL.  Will be passed the URL as a parameter.</source>
      <translation>El comando a ejecutar para ver una URL.  Se le pasará la URL como parámetro.</translation>
    </message>
    <message>
      <source>KDE default</source>
      <translation>KDE por defecto</translation>
    </message>
    <message>
      <source>Terminal:</source>
      <translation>Terminal:</translation>
    </message>
    <message>
      <source>The command to run to start your terminal program.</source>
      <translation>Comando a ejecutar para lanzar su programa de terminal.</translation>
    </message>
    <message>
      <source>Style:</source>
      <translation>Estilo:</translation>
    </message>
    <message>
      <source>Date format:</source>
      <translation>Formato de fecha:</translation>
    </message>
    <message>
      <source>Insert vertical spacing</source>
      <translation>Insertar espacio vertical</translation>
    </message>
    <message>
      <source>Insert extra space between messages.</source>
      <translation>Inserte espacio extra entre los mensajes</translation>
    </message>
    <message>
      <source>Insert horizontal line</source>
      <translation>Insertar línea horizontal</translation>
    </message>
    <message>
      <source>Insert a line between each message.</source>
      <translation>Inserte una línea entre cada mensaje</translation>
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
      <source>Show join/left notices</source>
      <translation>Mostrar notificación de ingreso y salida de la charla</translation>
    </message>
    <message>
      <source>Show a notice in the chat window when a user joins or leaves the conversation.</source>
      <translation>Mostrar una notificación en la ventana de charla cuando un usuario se une o deja una conversación.</translation>
    </message>
    <message>
      <source>Colors</source>
      <translation>Colores</translation>
    </message>
    <message>
      <source>Message received:</source>
      <translation>Mensaje recibido:</translation>
    </message>
    <message>
      <source>Message sent:</source>
      <translation>Mensaje enviado:</translation>
    </message>
    <message>
      <source>History received:</source>
      <translation>Historial recibido:</translation>
    </message>
    <message>
      <source>History sent:</source>
      <translation>Historial enviado:</translation>
    </message>
    <message>
      <source>Notice:</source>
      <translation>Aviso:</translation>
    </message>
    <message>
      <source>Typing notification color:</source>
      <translation>Color de la notificación de tipeo:</translation>
    </message>
    <message>
      <source>Background color:</source>
      <translation>Color de fondo:</translation>
    </message>
    <message>
      <source>Reverse history</source>
      <translation>Historial inverso</translation>
    </message>
    <message>
      <source>Put recent messages on top.</source>
      <translation>Colocar los mensajes recientes arriba</translation>
    </message>
    <message>
      <source>Preview</source>
      <translation>Vista Previa</translation>
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
      <translation>No, ¿adonde está?</translation>
    </message>
    <message>
      <source>#Licq on irc.freenode.net</source>
      <translation>#Licq en irc.freenode.net</translation>
    </message>
    <message>
      <source>Cool, I'll see you there :)</source>
      <translation>Genial, nos vemos ahí :)</translation>
    </message>
    <message>
      <source>We'll be waiting!</source>
      <translation>¡Te estaremos esperando!</translation>
    </message>
    <message>
      <source>Marge has left the conversation.</source>
      <translation>Marge dejó la conversación.</translation>
    </message>
    <message>
      <source>none</source>
      <translation>Ninguno</translation>
    </message>
    <message>
      <source>Show user picture</source>
      <translation>Mostrar foto del usuario</translation>
    </message>
    <message>
      <source>Show user picture next to the input area</source>
      <translation>Muestra la foto del usuario al lado de la entrada de texto</translation>
    </message>
    <message>
      <source>Minimize user picture</source>
      <translation>Minimizar la foto del usuario</translation>
    </message>
    <message>
      <source>Hide user picture upon opening</source>
      <translation>Ocultar la foto del usuario al abrir</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::Settings::ContactList</name>
    <message>
      <source>Contact List</source>
      <translation>Lista de Contactos</translation>
    </message>
    <message>
      <source>Columns</source>
      <translation>Columnas</translation>
    </message>
    <message>
      <source>Contact Info</source>
      <translation>Información de contacto</translation>
    </message>
    <message>
      <source>Show grid lines</source>
      <translation>Mostrar las líneas de la grilla</translation>
    </message>
    <message>
      <source>Draw boxes around each square in the user list</source>
      <translation>Dibujar líneas alrededor de cada recuadro de la lista de usuarios</translation>
    </message>
    <message>
      <source>Show column headers</source>
      <translation>Mostrar las cabeceras de las columnas</translation>
    </message>
    <message>
      <source>Turns on or off the display of headers above each column in the user list</source>
      <translation>Muestra u oculta las cabeceras de las columnas en la lista de contactos</translation>
    </message>
    <message>
      <source>Show user dividers</source>
      <translation>Mostrar los divisores de usuarios</translation>
    </message>
    <message>
      <source>Show the &quot;--online--&quot; and &quot;--offline--&quot; bars in the contact list</source>
      <translation>Muestra en la lista de contactos las barras &quot;--en línea--&quot; y &quot;--fuera de línea--&quot;</translation>
    </message>
    <message>
      <source>Use font styles</source>
      <translation>Usar fuentes de estilo</translation>
    </message>
    <message>
      <source>Use italics and bold in the user list to indicate special characteristics such as online notify and visible list</source>
      <translation>Usa fuentes en itálica y en negrita en la lista de contactos para indicar características tales como si está activado el aviso de conexión para un usuario o si pertenece a la lista de visibles</translation>
    </message>
    <message>
      <source>Show extended icons</source>
      <translation>Mostrar íconos extendidos</translation>
    </message>
    <message>
      <source>Show birthday, invisible, and custom auto response icons to the right of users in the list</source>
      <translation>Mostrar íconos para la fecha de nacimiento, invisible y respuesta automática a la derecha de los usuarios en la lista</translation>
    </message>
    <message>
      <source>Show user display picture</source>
      <translation>Mostrar la foto del usuario</translation>
    </message>
    <message>
      <source>Show the user's display picture instead of a status icon, if the user is online and has a display picture</source>
      <translation>Muestra la foto del usuario en lugar de su ícono de estado, si el usuario está en línea y tiene una foto.</translation>
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
      <source>Always show online notify users</source>
      <translation>Mostrar siempre notificación de conexión</translation>
    </message>
    <message>
      <source>Show online notify users who are offline even when offline users are hidden.</source>
      <translation>Mostrar los usuarios con aviso de conexión que estén desconectados incluso cuando no se muestren los usuarios desconectados.</translation>
    </message>
    <message>
      <source>Manual &quot;New User&quot; group handling</source>
      <translation>Gestión Manual del Grupo &quot;Usuarios Nuevos&quot;</translation>
    </message>
    <message>
      <source>If not checked, a user will be automatically removed from &quot;New User&quot; group when you first send an event to them.</source>
      <translation>Si esta opción no está tildada, el usuario será quitado automáticamente del grupo de \&quot;Nuevos usuarios\&quot; cuando se le envía un evento por primera vez</translation>
    </message>
    <message>
      <source>Allow dragging main window</source>
      <translation>Permitir que se arrastre la ventana principal</translation>
    </message>
    <message>
      <source>Lets you drag around the main window with your mouse</source>
      <translation>Le permite arrastrar la ventana principal con el ratón</translation>
    </message>
    <message>
      <source>Sticky main window</source>
      <translation>Ventana principal pegajosa</translation>
    </message>
    <message>
      <source>Makes the Main window visible on all desktops</source>
      <translation>Hace que la ventana principal sea visible en todos los escritorios</translation>
    </message>
    <message>
      <source>Transparent when possible</source>
      <translation>Transparente cuando sea posible</translation>
    </message>
    <message>
      <source>Make the user window transparent when there is no scroll bar</source>
      <translation>Hacer transparente la ventana de usuario cuando no haya barra de desplazamiento</translation>
    </message>
    <message>
      <source>Show group name if no messages</source>
      <translation>Mostrar nombre del grupo si no hay mensajes</translation>
    </message>
    <message>
      <source>Show the name of the current group in the messages label when there are no new messages</source>
      <translation>Mostrar el nombre del grupo actual en la etiqueta de mensajes cuando no haya mensajes</translation>
    </message>
    <message>
      <source>Use system background color</source>
      <translation>Usar color del fondo del sistema</translation>
    </message>
    <message>
      <source>Allow scroll bar</source>
      <translation>Permitir barra de desplazamiento</translation>
    </message>
    <message>
      <source>Allow the vertical scroll bar in the user list</source>
      <translation>Permitir la aparición de la barra de desplazamiento vertical en la lista de usuarios</translation>
    </message>
    <message>
      <source>Frame style:</source>
      <translation>Estilo del marco:</translation>
    </message>
    <message>
      <source>Override the skin setting for the frame style of the user window:
   0 (No frame), 1 (Box), 2 (Panel), 3 (WinPanel)
 + 16 (Plain), 32 (Raised), 48 (Sunken), 240 (Shadow)</source>
      <translation>Invalida la configuración de estilo del marco de la ventana de usuario: 
   0 (Sin Marco), 1 (Caja), 2 (Panel), 3 (WinPanel)
 + 16 (Plano), 32 (Elevada), 48 (Hundida), 240 (Sombra)</translation>
    </message>
    <message>
      <source>Additional sorting:</source>
      <translation>Ordenamiento adicional:</translation>
    </message>
    <message>
      <source>&lt;b>none:&lt;/b> - Don't sort online users by Status&lt;br>
&lt;b>status&lt;/b> - Sort online users by status&lt;br>
&lt;b>status + last event&lt;/b> - Sort online users by status and by last event&lt;br>
&lt;b>status + new messages&lt;/b> - Sort online users by status and number of new messages</source>
      <translation>&lt;b>ninguno:&lt;/b> - No ordena los usuarios conectados por Estado&lt;br>
&lt;b>estado&lt;/b> - Ordena los usuarios conectados por Estado&lt;br>
&lt;b>estado + último evento&lt;/b> - Ordena los usuarios conectados por Estado y por último evento&lt;br>
&lt;b>estado + nuevos mensajes&lt;/b> - Ordena los usuarios conectados por Estado y por número de mensajes nuevos.</translation>
    </message>
    <message>
      <source>none</source>
      <translation>Ninguno</translation>
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
      <source>Column Configuration</source>
      <translation>Configuración de Columnas</translation>
    </message>
    <message>
      <source>Title</source>
      <translation>Título</translation>
    </message>
    <message>
      <source>The string which will appear in the list box column header</source>
      <translation>La cadena que aparecerá en las cabeceras de las columnas</translation>
    </message>
    <message>
      <source>Format</source>
      <translation>Formato</translation>
    </message>
    <message>
      <source>The format string used to define what will appear in each column.&lt;br>The following parameters can be used:</source>
      <translation>La cadena de formato usada para definir qué aparecerá en cada columna.&lt;br>Se pueden utilizar los siguientes parámetros:</translation>
    </message>
    <message>
      <source>Width</source>
      <translation>Ancho</translation>
    </message>
    <message>
      <source>The width of the column</source>
      <translation>Ancho de la columna</translation>
    </message>
    <message>
      <source>Alignment</source>
      <translation>Alineamiento</translation>
    </message>
    <message>
      <source>The alignment of the column</source>
      <translation>Alineamiento de las columnas</translation>
    </message>
    <message>
      <source>Number of columns</source>
      <translation>Número de columnas</translation>
    </message>
    <message>
      <source>Left</source>
      <translation>Izquierda</translation>
    </message>
    <message>
      <source>Right</source>
      <translation>Derecha</translation>
    </message>
    <message>
      <source>Center</source>
      <translation>Centro</translation>
    </message>
    <message>
      <source>Popup info</source>
      <translation>Mostrar Información </translation>
    </message>
    <message>
      <source>Picture</source>
      <translation>Foto</translation>
    </message>
    <message>
      <source>Alias</source>
      <translation>Alias</translation>
    </message>
    <message>
      <source>Full name</source>
      <translation>Nombre completo</translation>
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
      <source>IP address</source>
      <translation>Dirección IP</translation>
    </message>
    <message>
      <source>Last online</source>
      <translation>En línea por última vez</translation>
    </message>
    <message>
      <source>Online time</source>
      <translation>Tiempo de conexión</translation>
    </message>
    <message>
      <source>Idle time</source>
      <translation>Tiempo sin actividad</translation>
    </message>
    <message>
      <source>Local time</source>
      <translation>Hora local</translation>
    </message>
    <message>
      <source>Protocol ID</source>
      <translation>ID del protocolo</translation>
    </message>
    <message>
      <source>Authorization status</source>
      <translation>Estado de autorización</translation>
    </message>
    <message>
      <source>Automatic Update</source>
      <translation>Actualización Automática</translation>
    </message>
    <message>
      <source>Contact information</source>
      <translation>Información del contacto</translation>
    </message>
    <message>
      <source>Automatically update users' server stored information.</source>
      <translation>Actualizar Automáticamente la información de los usuarios almacenada en el servidor</translation>
    </message>
    <message>
      <source>Info plugins</source>
      <translation>Información de los añadidos</translation>
    </message>
    <message>
      <source>Automatically update users' Phone Book and Picture.</source>
      <translation>Actualizar Automáticamente la agenda telefónica de los usuarios y sus fotos.</translation>
    </message>
    <message>
      <source>Status plugins</source>
      <translation>Estado de los añadidos</translation>
    </message>
    <message>
      <source>Automatically update users' Phone &quot;Follow Me&quot;, File Server and ICQphone status.</source>
      <translation>Actualizar Automáticamente los estados de Teléfono &quot;Sígame&quot;, del Servidor de Archivos y del ICQPhone</translation>
    </message>
    <message>
      <source>Contact List Appearance</source>
      <translation>Apariencia de la Lista de Contactos</translation>
    </message>
    <message>
      <source>Contact List Behaviour</source>
      <translation>Comportamiento de la Lista de Contactos</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::Settings::Events</name>
    <message>
      <source>Events</source>
      <translation>Eventos</translation>
    </message>
    <message>
      <source>Sounds</source>
      <translation>Sonidos</translation>
    </message>
    <message>
      <source>Actions On Incoming Messages</source>
      <translation>Acciones al recibir mensajes</translation>
    </message>
    <message>
      <source>Bold message label</source>
      <translation>Resaltar etiqueta del mensaje</translation>
    </message>
    <message>
      <source>Show the message info label in bold font if there are incoming messages</source>
      <translation>Mostrar la etiqueta de información de mensaje en negrita si no hay mensajes entrantes.</translation>
    </message>
    <message>
      <source>Auto-popup message</source>
      <translation>Maximizar automáticamente los mensajes</translation>
    </message>
    <message>
      <source>Open all incoming messages automatically when received if we are online (or free for chat)</source>
      <translation>Abrir todos los mensajes automáticamente cuando se reciben si estoy en línea (O libre para charlar)</translation>
    </message>
    <message>
      <source>Auto-focus message</source>
      <translation>Auto-enfocar mensaje</translation>
    </message>
    <message>
      <source>Automatically focus opened message windows.</source>
      <translation>Pone el foco automáticamente en la ventana de mensajes abierta.</translation>
    </message>
    <message>
      <source>Auto-raise main window</source>
      <translation>Mostrar automáticamente la ventana principal</translation>
    </message>
    <message>
      <source>Raise the main window on incoming messages</source>
      <translation>Levantar la ventana principal cuando se recibe un nuevo mensaje</translation>
    </message>
    <message>
      <source>Flash taskbar</source>
      <translation>Parpadear en la barra de tareas</translation>
    </message>
    <message>
      <source>Flash the taskbar on incoming messages</source>
      <translation>Parpadear en la barra de tareas cuando hay mensajes entrantes</translation>
    </message>
    <message>
      <source>Blink all events</source>
      <translation>Parpadear en todos los eventos</translation>
    </message>
    <message>
      <source>All incoming events will blink</source>
      <translation>Todos los Eventos Entrantes van a Parpadear</translation>
    </message>
    <message>
      <source>Blink urgent events</source>
      <translation>Parpadear en los eventos urgente</translation>
    </message>
    <message>
      <source>Only urgent events will blink</source>
      <translation>Sólo los eventos urgentes van a parpadear</translation>
    </message>
    <message>
      <source>Hot key:</source>
      <translation>Tecla abreviada:</translation>
    </message>
    <message>
      <source>Hotkey to pop up the next pending message.
Enter the hotkey literally, like &quot;shift+f10&quot;, or &quot;none&quot; for disabling.</source>
      <translation>Combinación de Teclas para mostrar el próximo mensaje pendiente. 
Ingrese la combinación de teclas literalmente, como \&quot;shift+f10\&quot;, o  \&quot;none\&quot; para deshabilitarla.</translation>
    </message>
    <message>
      <source>Paranoia</source>
      <translation>Paranoia</translation>
    </message>
    <message>
      <source>Ignore new users</source>
      <translation>Ignorar nuevos usuarios</translation>
    </message>
    <message>
      <source>Determines if new users are automatically added to your list or must first request authorization.</source>
      <translation>Determina si los nuevos usuarios se añaden automáticamente a la lista o si se debe pedir autorización primero.</translation>
    </message>
    <message>
      <source>Ignore mass messages</source>
      <translation>Ignorar mensajes masivos</translation>
    </message>
    <message>
      <source>Determines if mass messages are ignored or not.</source>
      <translation>Determina si se ignoran o no los mensajes de masas.</translation>
    </message>
    <message>
      <source>Ignore web panel</source>
      <translation>Ignorar panel web</translation>
    </message>
    <message>
      <source>Determines if web panel messages are ignored or not.</source>
      <translation>Determina si se ignoran o no los mensajes del panel web.</translation>
    </message>
    <message>
      <source>Ignore email pager</source>
      <translation>Ignorar el email pager</translation>
    </message>
    <message>
      <source>Determines if email pager messages are ignored or not.</source>
      <translation>Determina si se ignoran o no los mensajes del EMail Pager.</translation>
    </message>
    <message>
      <source>Sounds enabled</source>
      <translation>Habilitar los sonidos</translation>
    </message>
    <message>
      <source>Enable running of &quot;Command&quot; when the relevant event occurs.</source>
      <translation>Activar la ejecución de &quot;Comando&quot; cuando ocurran los eventos programados.</translation>
    </message>
    <message>
      <source>Command:</source>
      <translation>Comando:</translation>
    </message>
    <message>
      <source>Parameters</source>
      <translation>Parámetros</translation>
    </message>
    <message>
      <source>Message:</source>
      <translation>Mensaje:</translation>
    </message>
    <message>
      <source>Parameter for received messages</source>
      <translation>Parámetro para mensajes recibidos</translation>
    </message>
    <message>
      <source>URL:</source>
      <translation>URL:</translation>
    </message>
    <message>
      <source>Parameter for received URLs</source>
      <translation>Parámetro para URLs recibidas</translation>
    </message>
    <message>
      <source>Chat request:</source>
      <translation>Petición de Charla:</translation>
    </message>
    <message>
      <source>Parameter for received chat requests</source>
      <translation>Parámetro para charlas recibidas</translation>
    </message>
    <message>
      <source>File transfer:</source>
      <translation>Transferencia de fichero:</translation>
    </message>
    <message>
      <source>Parameter for received file transfers</source>
      <translation>Parámetro para transferencias de ficheros recibidas</translation>
    </message>
    <message>
      <source>Online notify:</source>
      <translation>Aviso de conexión:</translation>
    </message>
    <message>
      <source>Parameter for online notification</source>
      <translation>Parámetro para notificaciones de conexión</translation>
    </message>
    <message>
      <source>System msg:</source>
      <translation>Mensaje del sistema:</translation>
    </message>
    <message>
      <source>Parameter for received system messages</source>
      <translation>Parámetro para mensajes del sistema recibidos</translation>
    </message>
    <message>
      <source>Message sent:</source>
      <translation>Mensaje enviado:</translation>
    </message>
    <message>
      <source>Parameter for sent messages</source>
      <translation>Parámetro para los mensajes enviados</translation>
    </message>
    <message>
      <source>Enable Events</source>
      <translation>Habilitar Eventos</translation>
    </message>
    <message>
      <source>Sounds when Away</source>
      <translation>Sonidos cuando estoy Ausente</translation>
    </message>
    <message>
      <source>Perform OnEvent command in away mode</source>
      <translation>Activar Tratamiento de Eventos en modo Ausente</translation>
    </message>
    <message>
      <source>Sounds when N/A</source>
      <translation>Sonidos cuando estoy N/D</translation>
    </message>
    <message>
      <source>Perform OnEvent command in not available mode</source>
      <translation>Activar Tratamiento de Eventos en modo No Disponible</translation>
    </message>
    <message>
      <source>Sounds when Occupied</source>
      <translation>Sonidos cuando estoy Ocupado</translation>
    </message>
    <message>
      <source>Perform OnEvent command in occupied mode</source>
      <translation>Activar Tratamiento de Eventos en modo Ocupado</translation>
    </message>
    <message>
      <source>Sounds when DND</source>
      <translation>Sonidos cuando estoy No Molestar</translation>
    </message>
    <message>
      <source>Perform OnEvent command in do not disturb mode</source>
      <translation>Activar Tratamiento de Eventos en modo No Molestar</translation>
    </message>
    <message>
      <source>Online notify when logging on</source>
      <translation>Aviso de conexión al conectarse</translation>
    </message>
    <message>
      <source>Perform the online notify OnEvent when logging on (this is different from how the Mirabilis client works)</source>
      <translation>Lleva a cabo la notificación de conexión OnEvent cuando se conecta (funciona de forma diferente al cliente de Mirabilis)</translation>
    </message>
    <message>
      <source>Command to execute when an event is received.&lt;br>It will be passed the relevant parameters from below.&lt;br>Parameters can contain the following expressions &lt;br> which will be replaced with the relevant information:</source>
      <translation>Comando a ejecutar cuando se recibe un evento.&lt;br>Se le pasarán los parámetros relevantes de abajo.&lt;br>Los parámetros pueden contener las siguientes expresiones &lt;br> que serán reemplazadas por información relevante:</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::Settings::General</name>
    <message>
      <source>Docking</source>
      <translation>Ícono de Panel</translation>
    </message>
    <message>
      <source>Fonts</source>
      <translation>Fuentes</translation>
    </message>
    <message>
      <source>Use dock icon</source>
      <translation>Usar ícono de panel</translation>
    </message>
    <message>
      <source>Controls whether or not the dockable icon should be displayed.</source>
      <translation>Controla si se muestra o no el ícono acoplable a los paneles.</translation>
    </message>
    <message>
      <source>Start hidden</source>
      <translation>Comenzar Oculto</translation>
    </message>
    <message>
      <source>Start main window hidden. Only the dock icon will be visible.</source>
      <translation>Iniciar la ventana principal oculta. Sólo el ícono de panel será visible.</translation>
    </message>
    <message>
      <source>Default icon</source>
      <translation>Ícono por defecto</translation>
    </message>
    <message>
      <source>64 x 48 dock icon</source>
      <translation>ícono de panel de 64 x 48</translation>
    </message>
    <message>
      <source>Selects between the standard 64x64 icon used in the WindowMaker/Afterstep wharf
and a shorter 64x48 icon for use in the Gnome/KDE panel.</source>
      <translation>Selecciona entre los íconos estándar de 64x64 utilizados en WindowMaker/Afterstep y uno más pequeño de 64x48 utilizados por los paneles de Gnome/KDE</translation>
    </message>
    <message>
      <source>Themed icon</source>
      <translation>Icono de tema</translation>
    </message>
    <message>
      <source>Tray icon</source>
      <translation>Ícono de panel</translation>
    </message>
    <message>
      <source>Uses the freedesktop.org standard to dock a small icon into the system tray.
Works with many different window managers.</source>
      <translation>Usa el estándar de freedesktop.org para colocar el ícono dentro de la barra del sistema. Funciona con muchos gestores de ventanas.</translation>
    </message>
    <message>
      <source>Blink on events</source>
      <translation>Parpadear al recibir un evento</translation>
    </message>
    <message>
      <source>Make tray icon blink on unread incoming events.</source>
      <translation>Hacer parpadear el ícono del panel cuando hay eventos no leídos</translation>
    </message>
    <message>
      <source>General:</source>
      <translation>General:</translation>
    </message>
    <message>
      <source>Used for normal text.</source>
      <translation>Usado para texto normal</translation>
    </message>
    <message>
      <source>Select Font</source>
      <translation>Seleccionar Fuente</translation>
    </message>
    <message>
      <source>Select a font from the system list.</source>
      <translation>Seleccione una fuente de la lista del sistema</translation>
    </message>
    <message>
      <source>Editing:</source>
      <translation>Editando:</translation>
    </message>
    <message>
      <source>Used in message editor etc.</source>
      <translation>Usado para editar mensajes etc</translation>
    </message>
    <message>
      <source>default (%1)</source>
      <translation>por defecto (%1)</translation>
    </message>
    <message>
      <source>default</source>
      <translation>por defecto</translation>
    </message>
    <message>
      <source>Network</source>
      <translation>Red</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::Settings::Network</name>
    <message>
      <source>ICQ</source>
      <translation>ICQ</translation>
    </message>
    <message>
      <source>Firewall</source>
      <translation>Cortafuegos</translation>
    </message>
    <message>
      <source>I am behind a firewall</source>
      <translation>Estoy detrás de un cortafuegos</translation>
    </message>
    <message>
      <source>I can receive direct connections</source>
      <translation>Usted puede recibir conexiones directas</translation>
    </message>
    <message>
      <source>Port range:</source>
      <translation>Rango de puertos:</translation>
    </message>
    <message>
      <source>TCP port range for incoming connections.</source>
      <translation>Rango de puertos TCP para aceptar conexiones.</translation>
    </message>
    <message>
      <source>Auto</source>
      <translation>Auto</translation>
    </message>
    <message>
      <source>to</source>
      <translation>a</translation>
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
      <source>Proxy type:</source>
      <translation>Tipo de proxy</translation>
    </message>
    <message>
      <source>HTTPS</source>
      <translation>HTTPS</translation>
    </message>
    <message>
      <source>Proxy server:</source>
      <translation>Servidor proxy</translation>
    </message>
    <message>
      <source>Proxy server port:</source>
      <translation>Puerto del servidor proxy</translation>
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
      <source>Server Settings</source>
      <translation>Configuración del servidor</translation>
    </message>
    <message>
      <source>ICQ server:</source>
      <translation>Servidor ICQ:</translation>
    </message>
    <message>
      <source>ICQ server port:</source>
      <translation>Puerto del servidor ICQ:</translation>
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
      <translation>Licq puede reconectarse cuando fue desconectado debido a que su Uin fue usado desde otro lugar. Selecciónelo si desea que Licq se reconecte automáticamente.</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::Settings::Status</name>
    <message>
      <source>Status</source>
      <translation>Estado</translation>
    </message>
    <message>
      <source>Auto Response</source>
      <translation>Respuesta Automática</translation>
    </message>
    <message>
      <source>Startup</source>
      <translation>Inicialización</translation>
    </message>
    <message>
      <source>Offline</source>
      <translation>Fuera de Línea</translation>
    </message>
    <message>
      <source>Online</source>
      <translation>En Línea</translation>
    </message>
    <message>
      <source>Away</source>
      <translation>Ausente</translation>
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
      <source>Do Not Disturb</source>
      <translation>No Molestar</translation>
    </message>
    <message>
      <source>Free for Chat</source>
      <translation>Disponible Para Charlar </translation>
    </message>
    <message>
      <source>Automatically log on when first starting up.</source>
      <translation>Estado en el que se le marcará nada más conectar.</translation>
    </message>
    <message>
      <source>Invisible</source>
      <translation>Invisible</translation>
    </message>
    <message>
      <source>Auto Change Status</source>
      <translation>Cambio de estado automático</translation>
    </message>
    <message>
      <source>Auto Away:</source>
      <translation>Modo &quot;Ausente&quot; Automático en (minutos):</translation>
    </message>
    <message>
      <source>Number of minutes of inactivity after which to automatically be marked &quot;away&quot;.  Set to &quot;0&quot; to disable.</source>
      <translation>Número de minutos de inactividad después de los cuales se le marcará automáticamente como &quot;Ausente&quot;.  Póngalo a &quot;0&quot; para desactivar esta función.</translation>
    </message>
    <message>
      <source>Never</source>
      <translation>Nunca</translation>
    </message>
    <message>
      <source>Auto N/A:</source>
      <translation>Modo &quot;No Molestar&quot; Automático en (minutos):</translation>
    </message>
    <message>
      <source>Number of minutes of inactivity after which to automatically be marked &quot;not available&quot;.  Set to &quot;0&quot; to disable.</source>
      <translation>Número de minutos de inactividad después de los cuales se le marcará automáticamente como &quot;No Disponible&quot;.  Póngalo a &quot;0&quot; para desactivar esta función.</translation>
    </message>
    <message>
      <source>Auto Offline:</source>
      <translation>Auto Desconexión:</translation>
    </message>
    <message>
      <source>Number of minutes of inactivity after which to automatically go offline.  Set to &quot;0&quot; to disable.</source>
      <translation>Número de minutos de inactividad después de los cuales se le pondrá como &quot;desconectado&quot; automáticamente. Póngalo a &quot;0&quot; para inhabilitarlo.</translation>
    </message>
    <message>
      <source>Default Auto Response Messages</source>
      <translation>Mensajes por Defecto para Respuesta Automática</translation>
    </message>
    <message>
      <source>Status:</source>
      <translation>Estado:</translation>
    </message>
    <message>
      <source>Free For Chat</source>
      <translation>Disponible Para Charlar</translation>
    </message>
    <message>
      <source>Preset slot:</source>
      <translation>Preconfigurar casilla:</translation>
    </message>
    <message>
      <source>Hints</source>
      <translation>Sugerencias</translation>
    </message>
    <message>
      <source>Save</source>
      <translation>Guardar</translation>
    </message>
    <message>
      <source>Previous Message</source>
      <translation>Mensaje Anterior</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::SettingsDlg</name>
    <message>
      <source>Licq Settings</source>
      <translation>Configuración de Licq</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::ShowAwayMsgDlg</name>
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
    <name>LicqQtGui::SkinBrowserDlg</name>
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
Iconset '%2' has been disabled.</source>
      <translation>No se puede abrir el fichero de íconos 
%1 
El paquete de íconos %2 ha sido deshabilitado.</translation>
    </message>
    <message>
      <source>Unable to open extended icons file
%1
Extended Iconset '%2' has been disabled.</source>
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
    <message>
      <source>System</source>
      <translation>Sistema</translation>
    </message>
    <message>
      <source>Unable to load icons 
%1.</source>
      <translation>No se pueden cargar los iconos 
%1.</translation>
    </message>
    <message>
      <source>Unable to load extended icons 
%1.</source>
      <translation>No se pueden cargar los íconos extendidos 
%1.</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::StatsDlg</name>
    <message>
      <source>Licq - Statistics</source>
      <translation>Licq - Estadísticas</translation>
    </message>
    <message>
      <source>Daemon Statistics</source>
      <translation>Estadísticas del Daemon</translation>
    </message>
    <message>
      <source>Up since</source>
      <translation>Ejecutándose desde</translation>
    </message>
    <message>
      <source>Last reset</source>
      <translation>Último reinicio</translation>
    </message>
    <message>
      <source>Number of users</source>
      <translation>Número de usuarios</translation>
    </message>
    <message>
      <source>Event Statistics</source>
      <translation>Estadísticas de eventos</translation>
    </message>
    <message>
      <source>Today</source>
      <translation>Hoy</translation>
    </message>
    <message>
      <source>Total</source>
      <translation>Total</translation>
    </message>
    <message>
      <source>Do you really want to
reset your statistics?</source>
      <translation>¿Realmente desea poner sus estadísticas en cero?</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::SystemMenu</name>
    <message>
      <source>Debug Level</source>
      <translation>Nivel de Depuración</translation>
    </message>
    <message>
      <source>Status Info</source>
      <translation>Información de Estado</translation>
    </message>
    <message>
      <source>Unknown Packets</source>
      <translation>Paquetes Desconocidos</translation>
    </message>
    <message>
      <source>Errors</source>
      <translation>Errores</translation>
    </message>
    <message>
      <source>Warnings</source>
      <translation>Advertencias</translation>
    </message>
    <message>
      <source>Packets</source>
      <translation>Paquetes</translation>
    </message>
    <message>
      <source>Set All</source>
      <translation>Activar Todos</translation>
    </message>
    <message>
      <source>Clear All</source>
      <translation>Limpiar Todos</translation>
    </message>
    <message>
      <source>&amp;View System Messages...</source>
      <translation>&amp;Ver Mensajes del Sistema...</translation>
    </message>
    <message>
      <source>&amp;Owner Manager...</source>
      <translation>Manejador de Cuentas...</translation>
    </message>
    <message>
      <source>&amp;Security/Password Options...</source>
      <translation>Opciones de &amp;Seguridad/Contraseñas</translation>
    </message>
    <message>
      <source>&amp;Random Chat Group...</source>
      <translation>Grupo de Cha&amp;rla Aleatorio</translation>
    </message>
    <message>
      <source>&amp;Add User...</source>
      <translation>&amp;Agregar Usuario...</translation>
    </message>
    <message>
      <source>S&amp;earch for User...</source>
      <translation>Buscar Usuario...</translation>
    </message>
    <message>
      <source>A&amp;uthorize User...</source>
      <translation>A&amp;utorizar Usuario...</translation>
    </message>
    <message>
      <source>Re&amp;quest Authorization...</source>
      <translation>P&amp;edir Autorización...</translation>
    </message>
    <message>
      <source>R&amp;andom Chat...</source>
      <translation>Charla &amp;Aleatoria...</translation>
    </message>
    <message>
      <source>&amp;Popup All Messages...</source>
      <translation>Mostrar mensajes en ventanas emergentes...</translation>
    </message>
    <message>
      <source>Edit &amp;Groups...</source>
      <translation>Editar &amp;Grupos...</translation>
    </message>
    <message>
      <source>Update All Users</source>
      <translation>Actualizar Todos los Usuarios</translation>
    </message>
    <message>
      <source>Update Current Group</source>
      <translation>Actualizar Grupo Actual</translation>
    </message>
    <message>
      <source>&amp;Redraw User Window</source>
      <translation>&amp;Redibujar Ventana de Usuario</translation>
    </message>
    <message>
      <source>&amp;Save All Users</source>
      <translation>Guardar Todo&amp;s los Usuarios</translation>
    </message>
    <message>
      <source>Phone &quot;Follow Me&quot;</source>
      <translation>Teléfono &quot;Sígame&quot;</translation>
    </message>
    <message>
      <source>Don't Show</source>
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
      <source>&amp;Status</source>
      <translation>E&amp;stado</translation>
    </message>
    <message>
      <source>&amp;Group</source>
      <translation>&amp;Grupo</translation>
    </message>
    <message>
      <source>&amp;Help</source>
      <translation>Ayuda</translation>
    </message>
    <message>
      <source>&amp;Hints...</source>
      <translation>Sugerencias...</translation>
    </message>
    <message>
      <source>&amp;About...</source>
      <translation>&amp;Acerca De...</translation>
    </message>
    <message>
      <source>&amp;Statistics...</source>
      <translation>E&amp;stadísticas</translation>
    </message>
    <message>
      <source>Set &amp;Auto Response...</source>
      <translation>Configurar Respuesta &amp;Automática...</translation>
    </message>
    <message>
      <source>&amp;Network Window...</source>
      <translation>Ve&amp;ntana de Red...</translation>
    </message>
    <message>
      <source>&amp;Mini Mode</source>
      <translation>Modo &amp;Miniatura</translation>
    </message>
    <message>
      <source>Show Offline &amp;Users</source>
      <translation>Mostrar &amp;Usuarios Fuera de línea</translation>
    </message>
    <message>
      <source>&amp;Thread Group View</source>
      <translation>Vista de Grupos Jerárquica</translation>
    </message>
    <message>
      <source>&amp;Plugin Manager...</source>
      <translation>Gestor de A&amp;pliques...</translation>
    </message>
    <message>
      <source>Sa&amp;ve Settings</source>
      <translation>Guardar Configuración</translation>
    </message>
    <message>
      <source>E&amp;xit</source>
      <translation>Salir</translation>
    </message>
    <message>
      <source>S&amp;ystem Functions</source>
      <translation>Funciones del Sistema</translation>
    </message>
    <message>
      <source>User &amp;Functions</source>
      <translation>&amp;Funciones de Usuario</translation>
    </message>
    <message>
      <source>Sh&amp;ow Empty Groups</source>
      <translation>M&amp;ostrar Grupos Vacíos</translation>
    </message>
    <message>
      <source>S&amp;ettings...</source>
      <translation>Configuración...</translation>
    </message>
    <message>
      <source>Sk&amp;in Browser...</source>
      <translation>Pieles...</translation>
    </message>
    <message>
      <source>GPG &amp;Key Manager...</source>
      <translation>Manejador de claves GPG...</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::SystemMenuPrivate::OwnerData</name>
    <message>
      <source>&amp;Info...</source>
      <translation>&amp;Info...</translation>
    </message>
    <message>
      <source>View &amp;History...</source>
      <translation>Ver &amp;Historial...</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::ThemedDockIcon</name>
    <message>
      <source>Unable to load dock theme file
(%1)
:%2</source>
      <translation>No se pudo cargar el fichero de tema para el ícono del panel
(%1)
:%2</translation>
    </message>
    <message>
      <source>Unable to load dock theme image
%1</source>
      <translation>No se pudo cargar la imagen para el ícono del panel
%1</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::TimeZoneEdit</name>
    <message>
      <source>Unknown</source>
      <translation>Desconocido</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::TreePager</name>
    <message>
      <source>Categories</source>
      <translation>Categorías</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::UserCodec</name>
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
    <name>LicqQtGui::UserEventCommon</name>
    <message>
      <source>Unknown</source>
      <translation>Desconocido/a</translation>
    </message>
    <message>
      <source>Open / Close secure channel</source>
      <translation>Abrir / Cerrar canal seguro</translation>
    </message>
    <message>
      <source>Open user menu</source>
      <translation>Abrir menú de usuario</translation>
    </message>
    <message>
      <source>Show user history</source>
      <translation>Mostrar historial del usuario</translation>
    </message>
    <message>
      <source>Show user information</source>
      <translation>Mostrar información del usuario</translation>
    </message>
    <message>
      <source>User's current local time</source>
      <translation>Hora actual del usuario</translation>
    </message>
    <message>
      <source>Menu</source>
      <translation>Menú</translation>
    </message>
    <message>
      <source>History...</source>
      <translation>Historial...</translation>
    </message>
    <message>
      <source>User Info...</source>
      <translation>Info del Usuario...</translation>
    </message>
    <message>
      <source>Encoding</source>
      <translation>Codificación</translation>
    </message>
    <message>
      <source>Select the text encoding used for outgoing messages.</source>
      <translation>Seleccione la codificación de los mensajes a enviar.</translation>
    </message>
    <message>
      <source>Secure Channel</source>
      <translation>Canal Seguro</translation>
    </message>
    <message>
      <source>Unable to load encoding &lt;b>%1&lt;/b>.&lt;br>Message contents may appear garbled.</source>
      <translation>No se pudo cargar la codificación &lt;b>%1&lt;/b>.&lt;br>Los contenidos del mensaje pueden mostrarse ilegibles.</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::UserEventTabDlg</name>
    <message>
      <source>[UNKNOWN_USER]</source>
      <translation>[USUARIO_DESCONOCIDO]</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::UserInfoDlg</name>
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
      <translation>&amp;Menú</translation>
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
      <source>City:</source>
      <translation>Ciudad:</translation>
    </message>
    <message>
      <source>Name:</source>
      <translation>Nombre:</translation>
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
      <translation> Mes:</translation>
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
      <source>Country:</source>
      <translation>País:</translation>
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
      <source>Gender:</source>
      <translation>Sexo:</translation>
    </message>
    <message>
      <source>&amp;General</source>
      <translation>&amp;General</translation>
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
      <translation>Última vez en línea:</translation>
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
      <translation>En línea desde:</translation>
    </message>
    <message>
      <source>Offline</source>
      <translation>Fuera de línea</translation>
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
      <source>Registration Date:</source>
      <translation>Fecha de Registro:</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::UserMenu</name>
    <message>
      <source>Send</source>
      <translation>Enviar</translation>
    </message>
    <message>
      <source>Send &amp;Message</source>
      <translation>Enviar &amp;Mensaje</translation>
    </message>
    <message>
      <source>Send &amp;URL</source>
      <translation>Enviar &amp;URL</translation>
    </message>
    <message>
      <source>Send &amp;Chat Request</source>
      <translation>Petición de &amp;Charla</translation>
    </message>
    <message>
      <source>Send &amp;File Transfer</source>
      <translation>Transferencia de &amp;Ficheros</translation>
    </message>
    <message>
      <source>Send Contact &amp;List</source>
      <translation>Enviar &amp;Lista de Contactos</translation>
    </message>
    <message>
      <source>Send &amp;Authorization</source>
      <translation>Enviar &amp;Autorización</translation>
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
      <source>Misc Modes</source>
      <translation>Modos Mezclados</translation>
    </message>
    <message>
      <source>Accept in Away</source>
      <translation>Aceptar en Ausente</translation>
    </message>
    <message>
      <source>Accept in Not Available</source>
      <translation>Aceptar en No Disponible</translation>
    </message>
    <message>
      <source>Accept in Occupied</source>
      <translation>Aceptar en Ocupado/a</translation>
    </message>
    <message>
      <source>Accept in Do Not Disturb</source>
      <translation>Aceptar en No Molestar</translation>
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
      <source>Use GPG Encryption</source>
      <translation>Usar Encriptación GPG</translation>
    </message>
    <message>
      <source>Use Real Ip (LAN)</source>
      <translation>Usar IP Real (LAN)</translation>
    </message>
    <message>
      <source>Online to User</source>
      <translation>En línea para este Usuario</translation>
    </message>
    <message>
      <source>Away to User</source>
      <translation>Ausente para el Usuario</translation>
    </message>
    <message>
      <source>Not Available to User</source>
      <translation>No Disponible para este Usuario</translation>
    </message>
    <message>
      <source>Occupied to User</source>
      <translation>Ocupado/a para este Usuario</translation>
    </message>
    <message>
      <source>Do Not Disturb to User</source>
      <translation>No Molestar para este Usuario</translation>
    </message>
    <message>
      <source>U&amp;tilities</source>
      <translation>U&amp;tilidades</translation>
    </message>
    <message>
      <source>Edit User Group</source>
      <translation>Editar Pertenencia a Grupos</translation>
    </message>
    <message>
      <source>Server Group</source>
      <translation>Grupo en el Servidor</translation>
    </message>
    <message>
      <source>&amp;View Event</source>
      <translation>&amp;Ver Evento(s)</translation>
    </message>
    <message>
      <source>Custom Auto Response...</source>
      <translation>Respuesta Automática Personalizada...</translation>
    </message>
    <message>
      <source>Toggle &amp;Floaty</source>
      <translation>Des/Activar &amp;Flotante</translation>
    </message>
    <message>
      <source>Remove From List</source>
      <translation>Eliminar de la Lista</translation>
    </message>
    <message>
      <source>Set GPG key</source>
      <translation>Asignar Clave GPG</translation>
    </message>
    <message>
      <source>View &amp;History</source>
      <translation>Ver &amp;Historial</translation>
    </message>
    <message>
      <source>&amp;Info</source>
      <translation>&amp;Información</translation>
    </message>
    <message>
      <source>Check Auto Response</source>
      <translation>Comprobar Respuesta Automática</translation>
    </message>
    <message>
      <source>Check %1 Response</source>
      <translation>Comprobar Respuesta %1</translation>
    </message>
    <message>
      <source>Close &amp;Secure Channel</source>
      <translation>Cerrar Canal &amp;Seguro</translation>
    </message>
    <message>
      <source>Request &amp;Secure Channel</source>
      <translation>Pedir Canal &amp;Seguro</translation>
    </message>
    <message>
      <source>Do you really want to add
%1 (%2)
to your ignore list?</source>
      <translation>¿Realmente quiere añadir a
%1 (%2)
a su lista de ignorados?</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::UserSelectDlg</name>
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
    <name>LicqQtGui::UserSendChatEvent</name>
    <message>
      <source>Clear</source>
      <translation>Limpiar</translation>
    </message>
    <message>
      <source>Multiparty: </source>
      <translation>Múltiples Usuarios: </translation>
    </message>
    <message>
      <source>Invite</source>
      <translation>Invitar</translation>
    </message>
    <message>
      <source> - Chat Request</source>
      <translation> - Petición de Charla</translation>
    </message>
    <message>
      <source>No reason provided</source>
      <translation>No se dio ninguna razón</translation>
    </message>
    <message>
      <source>Chat with %1 refused:
%2</source>
      <translation>Charla con %1 rechazada:
%2</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::UserSendCommon</name>
    <message>
      <source>URL</source>
      <translation>URL</translation>
    </message>
    <message>
      <source>done</source>
      <translation>hecho</translation>
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
      <source>Warning: Message can't be sent securely
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
      <source>Message type</source>
      <translation>Tipo de mensaje</translation>
    </message>
    <message>
      <source>Send through server</source>
      <translation>Enviar a través del servidor</translation>
    </message>
    <message>
      <source>Multiple Recipients</source>
      <translation>Múltiples Destinatarios</translation>
    </message>
    <message>
      <source>Multiple recipients</source>
      <translation>Múltiples destinatarios</translation>
    </message>
    <message>
      <source>Smileys</source>
      <translation>Emoticones</translation>
    </message>
    <message>
      <source>Insert smileys</source>
      <translation>Agregar Emoticones</translation>
    </message>
    <message>
      <source>Change text color</source>
      <translation>Cambiar el color del texto</translation>
    </message>
    <message>
      <source>Change background color</source>
      <translation>Cambiar el color de fondo</translation>
    </message>
    <message>
      <source>Select type of message to send</source>
      <translation>Seleccione el tipo de mensaje a enviar</translation>
    </message>
    <message>
      <source>Text Color...</source>
      <translation>Color del Texto...</translation>
    </message>
    <message>
      <source>Background Color...</source>
      <translation>Color de Fondo...</translation>
    </message>
    <message>
      <source>%1 has joined the conversation.</source>
      <translation>%1 se unió a la charla.</translation>
    </message>
    <message>
      <source>%1 has left the conversation.</source>
      <translation>%1 dejó la charla.</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::UserSendContactEvent</name>
    <message>
      <source>Drag Users Here - Right Click for Options</source>
      <translation>Arrastre Usuarios Aquí - Pulse con el Botón Derecho para Opciones</translation>
    </message>
    <message>
      <source> - Contact List</source>
      <translation> - Lista de Contactos</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::UserSendFileEvent</name>
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
      <source>Select files to send</source>
      <translation>Seleccione los archivos a enviar</translation>
    </message>
    <message>
      <source>Edit</source>
      <translation>Editar</translation>
    </message>
    <message>
      <source>No reason provided</source>
      <translation>No se dio ninguna razón</translation>
    </message>
    <message>
      <source>%1 Files</source>
      <translation>%1 Ficheros</translation>
    </message>
    <message>
      <source>File transfer with %1 refused:
%2</source>
      <translation>Rechazada Transferencia de Ficheros con %1:
%2</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::UserSendMsgEvent</name>
    <message>
      <source> - Message</source>
      <translation> - Mensaje</translation>
    </message>
    <message>
      <source>You didn't edit the message.
Do you really want to send it?</source>
      <translation>No editó el mensaje.
¿Realmente quiere enviarlo?</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::UserSendSmsEvent</name>
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
      <source>You didn't edit the SMS.
Do you really want to send it?</source>
      <translation>No editó el SMS. 
¿Realmente desea enviarlo?</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::UserSendUrlEvent</name>
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
    <name>LicqQtGui::UserViewEvent</name>
    <message>
      <source>Chat</source>
      <translation>Charla</translation>
    </message>
    <message>
      <source>&amp;Join</source>
      <translation>Unirse</translation>
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
&lt;CTRL>+Click - also delete User</source>
      <translation>Pulsación Normal - Cerrar Ventana
&lt;CTRL>+Pulsación - también borrar el Usuario</translation>
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
      <source>Licq is unable to start your browser and open the URL.
You will need to start the browser and open the URL manually.</source>
      <translation>Licq no ha podido iniciar su navegador y abrir la URL. 
Deberá iniciar el navegador y abrir la URL de forma manual.</translation>
    </message>
  </context>
  <context>
    <name>LicqQtGui::UtilityDlg</name>
    <message>
      <source>Licq Utility: %1</source>
      <translation>Utilidad de Licq: %1</translation>
    </message>
    <message>
      <source>Command:</source>
      <translation>Comando:</translation>
    </message>
    <message>
      <source>Window:</source>
      <translation>Ventana:</translation>
    </message>
    <message>
      <source>GUI</source>
      <translation>GUI</translation>
    </message>
    <message>
      <source>Terminal</source>
      <translation>Terminal</translation>
    </message>
    <message>
      <source>Internal</source>
      <translation>Interno</translation>
    </message>
    <message>
      <source>Description:</source>
      <translation>Descripción:</translation>
    </message>
    <message>
      <source>Edit final command</source>
      <translation>Editar comando final</translation>
    </message>
    <message>
      <source>User Fields</source>
      <translation>Campos de Usuario</translation>
    </message>
    <message>
      <source>&amp;Run</source>
      <translation>Ejecuta&amp;r</translation>
    </message>
    <message>
      <source>&amp;Cancel</source>
      <translation>&amp;Cancelar</translation>
    </message>
    <message>
      <source>Edit:</source>
      <translation>Editar:</translation>
    </message>
    <message>
      <source>Running:</source>
      <translation>Ejecutando:</translation>
    </message>
    <message>
      <source>Command Window</source>
      <translation>Ventana de Comandos</translation>
    </message>
    <message>
      <source>Failed:</source>
      <translation>Falló:</translation>
    </message>
    <message>
      <source>Done</source>
      <translation>Hecho</translation>
    </message>
    <message>
      <source>Done:</source>
      <translation>Hecho:</translation>
    </message>
    <message>
      <source>C&amp;lose</source>
      <translation>Cerrar</translation>
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
    <name>Status</name>
    <message>
      <source>Offline</source>
      <translation>Fuera de línea</translation>
    </message>
    <message>
      <source>Do Not Disturb</source>
      <translation>No Molestar</translation>
    </message>
    <message>
      <source>Occupied</source>
      <translation>Ocupado/a</translation>
    </message>
    <message>
      <source>Not Available</source>
      <translation>No Disponible</translation>
    </message>
    <message>
      <source>Away</source>
      <translation>Ausente</translation>
    </message>
    <message>
      <source>Free for Chat</source>
      <translation>Disponible Para Charlar </translation>
    </message>
    <message>
      <source>Online</source>
      <translation>En línea</translation>
    </message>
    <message>
      <source>Unknown</source>
      <translation>Desconocido</translation>
    </message>
    <message>
      <source>Off</source>
      <translation>Desc.</translation>
    </message>
    <message>
      <source>DND</source>
      <translation>NoMol.</translation>
    </message>
    <message>
      <source>Occ</source>
      <translation>Occ</translation>
    </message>
    <message>
      <source>N/A</source>
      <translation>NoDisp.</translation>
    </message>
    <message>
      <source>FFC</source>
      <translation>LPC</translation>
    </message>
    <message>
      <source>On</source>
      <translation>Con.</translation>
    </message>
    <message>
      <source>???</source>
      <translation>???</translation>
    </message>
    <message>
      <source>Invisible</source>
      <translation>Invisible</translation>
    </message>
    <message>
      <source>Inv</source>
      <translation>Inv.</translation>
    </message>
  </context>
</TS>
