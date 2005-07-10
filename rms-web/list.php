<?php

include "config.php";
include "check_session.php";
include "rms.php";

function printUserList()
{
  global $allusers, $ownerStatus, $title, $showonlyonline;
  $totalmsgs = 0;
  
  if ($allusers == 0) return;
  
  echo "\n<TABLE border=1>\n";
  
  foreach ($allusers as $user)
  {
    if ($showonlyonline == 1 && $user[2] == 0 && strcmp($user[3], "Offline") == 0)
       continue;
       
    $file = "offline.png";
    switch ($user[3])
    {
      case "Online":
        $file = "online.png";
        break;
      case "Free for Chat":
        $file = "ffc.png";
        break;
      case "Away":
        $file = "away.png";
        break;
      case "Not Available":
        $file = "na.png";
        break;
      case "Do Not Distrub":
        $file = "dnd.png";
        break;
      case "Occupied":
        $file = "occ.png";
        break;
    }
    
    if (strcmp($user[4], "Licq") == 0 &&
       !($user[0]{0} >= '0' && $user[0]{0} <= '9'))
      $file = "aim" . $file;
    else if (strcmp($user[4], "MSN") == 0)
      $file = "msn" . $file;
    
    if ($user[2] != "0")
    {
      $file = "msg.png";
      $totalmsgs += $user[2];
    }
    
    echo "<TR>\n";
    if (strcmp($file, "msg.png") == 0)
      echo "<TD><a href=\"message.php?command=VIEW&id=" . $user[0] . "&pp=" . $user[4] . "\" target=\"work\"><img src=\"" . $file . "\"></TD></a>\n";
    else
      echo "<TD><img src=\"" . $file . "\"></TD>\n";
    echo "<TD><a href=\"message.php?command=MSG&id=" . $user[0] . "&pp=" . $user[4] . "\" target=\"work\">" . $user[1] . "</a></TD></TR>\n";
  }
    
  echo "<TR><TD></TD><TD><FORM action=\"server.php\" method=\"POST\">\n" .
       "<INPUT type=\"hidden\" name=\"cmd\" value=\"chgStatus\">\n" .
       "<SELECT name=\"status\">\n" .
       "<OPTION value=\"online\" onClick=\"javascript:document.forms[0].submit()\"";
  if (strcasecmp($ownerStatus, "Online") == 0)
    echo " SELECTED";
  echo ">Online</OPTION>\n" .
       "<OPTION value=\"ffc\" onClick=\"javascript:document.forms[0].submit()\"";
  if (strcasecmp($ownerStatus, "Free for Chat") == 0)
    echo " SELECTED";
  echo ">Free for Chat</OPTION>\n" .
       "<OPTION value=\"away\" onClick=\"javascript:document.forms[0].submit()\"";
  if (strcasecmp($ownerStatus, "Away") == 0)
    echo " SELECTED";
  echo ">Away</OPTION>\n" .
       "<OPTION value=\"na\" onClick=\"javascript:document.forms[0].submit()\"";
  if (strcasecmp($ownerStatus, "Not Available") == 0)
    echo " SELECTED";
  echo ">Not Available</OPTION>\n" .
       "<OPTION value=\"occ\" onClick=\"javascript:document.forms[0].submit()\"";
  if (strcasecmp($ownerStatus, "Occupied") == 0)
    echo " SELECTED";
  echo ">Occupied</OPTION>\n" .
       "<OPTION value=\"dnd\" onClick=\"javascript:document.forms[0].submit()\"";
  if (strcasecmp($ownerStatus, "Do Not Distrub") == 0)
    echo " SELECTED";
  echo ">Do Not Disturb</OPTION>\n" .
       "<OPTION value=\"offline\" onClick=\"javascript:document.forms[0].submit()\"";
  if (strcasecmp($ownerStatus, "Offline") == 0)
    echo " SELECTED";
  echo ">Offline</OPTION>\n" .
       "</SELECT></FORM></TD></TR>\n";
  echo "</TABLE>\n";
}


rmsLogin($username, $password);
rmsGetList();
rmsGetOwnerStatus();

?>

<html>
<head>
<meta HTTP-EQUIV="REFRESH" CONTENT="<?php echo $listrefresh; ?>; URL=list.php">
<body>
<center>
<br>

<?php
printUserList();
?>
