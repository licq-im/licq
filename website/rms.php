<?php

function printHeader()
{
  echo "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\"\n" .
    "\"http://www.w3.org/TR/REC-html40/loose.dtd\"><HTML>\n" . 
    "<HEAD>\n" .
    "<meta name=\"description\" content=\"\" >\n" .
    "<meta name=\"author\" content=\"Jon Keating\">\n" .
    "<meta name=\"keywords\" content=\"\" >\n" .
    "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\n" .
    "<TITLE>licqrms</TITLE>\n" .
    "</HEAD>\n". 
    "<BODY>\n";
}

function printFooter()
{
  echo "\n</BODY></HTML>";
}

$packet = "";
$buf = "";

function readSocket($desc)
{
  while (socket_recv($desc, $buf, 128) > 0)
  {
    echo $buf;
    $packet = $packet + $buf;
  }
}

function rmsLogin($uin, $passwd)
{
  // Connect to the RMS server
  $fp = fsockopen("127.0.0.1", 22);
  if (!$fp)
    echo "AFDS\n";    
  else 
    echo "OK\n";

    
 while (!feof($fp)) {
       echo fgets($fp, 18);
   }
  // Get the UIN prompt
  //readSocket($socket);
  
  //echo $packet;
  
  close($fp);
}

?>

<?
  printHeader();

  if (isset($_POST))
  {
    $login_uin = $_POST["login_uin"];
    $login_password = $_POST["login_password"];

    echo "Loggin in\n";
      
    rmsLogin($login_uin, $login_password);
  }
?>

<center>
<BR><BR><BR><BR><BR><BR>
<TABLE border=0><TR><TD align="center">
<FORM method="POST">
<DIV align="right">
UIN: &nbsp;&nbsp;&nbsp;&nbsp;
<INPUT type="text" size=20 name="login_uin" value=""><BR><BR>
Password: &nbsp;&nbsp;&nbsp;&nbsp;
<INPUT type="password" size=20 name="login_password" value="">
</DIV>
<BR><INPUT type="submit" name="submit" value="Login">
</FORM>
</TD></TR></TABLE>
</CENTER>

<?
  printFooter()
?>
