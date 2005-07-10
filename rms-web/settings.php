<?php
include "config.php";

global $server, $port, $showonlyonline, $listrefresh;

$fc = array();

function setVariable($strKey, $strVal, $isString)
{
    $fc = file("config.php");
    $f = fopen("config.php", "w");
    
    foreach($fc as $line)
    {
      if (strncmp($line, "$" . $strKey, strlen($strKey) + 1) == 0)
      {
        $output = "$" . $strKey . "=";
        if ($isString == true)
          $output = $output . "\"";
        $output = $output . $strVal;
        if ($isString == true)
          $output = $output . "\"";
        $output= $output . ";\n";
      
        fputs($f, $output);
      }
      else
      {
        fputs($f, $line);
      }
    }
    
    fclose($f);
    $fc = array();
}

  $newserver = $_POST["server"];
  $newport = $_POST["port"];
  $newshowonlyonline = $_POST["showoffline"];
  $newlistrefresh = $_POST["listrefresh"];
  
  // Did we receive any input?
  if ($newserver)
  {
    $server = $newserver;
    if ($newport)
      $port = $newport;
    if ($newlistrefresh)
      $listrefresh = $newlistrefresh;
    if (!$newshowonlyonline)
      $showonlyonline = 1;
    else
      $showonlyonline = 0;
    
    // Save in the config file now
    setVariable("server", $server, true);
    setVariable("port", $port, false);
    setVariable("listrefresh", $listrefresh, false);
    setVariable("showonlyonline", $showonlyonline, false);
    
    echo "<FONT color=\"red\">Settings Saved</FONT><BR><BR>";
  }
?>

<HTML>
<TITLE>Licq RMS Web Settings</TITLE>
<BODY>
<CENTER>

<FORM action="settings.php" method="POST">
<TABLE cellpadding="2" cellspacing="0" border="0">
<TR>
<TD>Server</TD>
<TD><INPUT type="text" name="server" size="20" value="<? echo $server ?>"></TD></TR>
<TR><TD>Port</TD>
<TD><INPUT type="text" name="port" size="6" value="<? echo $port ?>"></TD></TR>
<TR><TD>List Refresh</TD>
<TD><INPUT type="text" name="listrefresh" size="6" value="<? echo $listrefresh ?>"></TD></TR>
<TR><TD>Show Offline Users</TD>
<TD><INPUT type="checkbox" value=1 name="showoffline"
 <? if (!$showonlyonline) echo "CHECKED"; ?>></TD></TR>
<TR><TD><INPUT type="submit" name="Save" value="Save">
</TABLE>
</FORM>

</CENTER>
</BODY>
</HTML>