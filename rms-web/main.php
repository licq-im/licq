<?php include "config.php" ?>

<html>
  <head>
    <title>Licq RMS Interface v<?php echo $version; if ($title) echo " " . $title; ?></title>
  </head>
  <frameset cols="187,*" border=0 frameborder="2">
    <frame name="list" src="list.php" marginwidth="0" topmargin="0" leftmargin="0" marginheight="0">
  <frameset rows="71,*" border=0 frameborder="2">
    <frame name="header" src="header.php" marginwidth="0" topmargin="0" leftmargin="0" marginheight="0">
    <frame name="work" src="message.php" marginwidth="0" topmargin="0" leftmargin="0" marginheight="0">
  </frameset>
  
  <body>
  
  
  
  </body>
  </frameset>
</html>