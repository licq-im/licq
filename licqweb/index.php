<?php
echo '<?xml version="1.0" encoding="iso-8859-1"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
';
?>
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
  <head>
    <title>licq</title>
    <link rel="StyleSheet" type="text/css" href="licqweb.css" />
    <link rel="shortcut icon" href="images/favicon.ico" />
  </head>
  <body onload="checkSession()">
    <div id="tooltip"></div>
    <script type="text/javascript" src="licqweb.js"></script>
    <h1>licq<i style="color: #5555ff">web</i></h1>
      <div id="login" class="loginwin">
      <div class="bar">licqweb login</div>
      <div class="centre">
        <p>UIN <input id="uin" type="text"/></p>
        <p>Password <input id="password" type="password"/></p>
        <p><input type="submit" onclick="login(); return false;" value="Login" /></p>
      </div>
    </div>
    <div id="contactList" class="box" style="left:10px;top:150px;display:none;">
      <div class="bar" onmousedown="init_drag(event, 'contactList')" id="nick">licqweb</div>
      <div id="contacts"></div>
      <div style="height:18px"> 
        <div id="ownerStatus"></div>
        <div class="resizer" onmousedown="init_resize(event, 'contacts', 100, 100)">
          <img class="resizerImage" src="images/resizer.png" />
        </div>
      </div>
    </div>
    <div id="statusMenu"></div>
    <div id="windows"></div>
    <div class="logWindow" id="log"></div>
    <div id="debug"></div>
  </body>
</html>
