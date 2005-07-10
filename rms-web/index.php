<?php

include "config.php";
include "check_session.php";
include "rms.php";

function printHeader()
{
  echo "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\"\n" .
    "\"http://www.w3.org/TR/REC-html40/loose.dtd\"><HTML>\n" . 
    "<HEAD>\n" .
    "<meta name=\"description\" content=\"\" >\n" .
    "<meta name=\"author\" content=\"Jon Keating\">\n" .
    "<meta name=\"keywords\" content=\"\" >\n" .
    "<TITLE>Licq PHP Interface - Login</TITLE>\n" .
    "</HEAD>\n". 
    "<BODY>\n";
}

function printFooter()
{
  echo "\n</BODY></HTML>";
}

//----------------------------------------
// Here is where we start the HTML coding
//----------------------------------------
  
  printHeader();

  // Logging in
  $login_uin = $_POST["login_uin"];
  $login_password = $_POST["login_password"];
  
  if (!empty($login_uin) && !empty($login_password))
  {
    $login_uin .= "\r\n";
    $login_password .= "\r\n";
      
    if (!rmsLogin($login_uin, $login_password))
    {
      if (!empty($errmsg))
      {
        echo "<B><FONT color=\"red\">" . $errmsg . "</FONT></B><BR><BR>";
      }

      echo "UIN: &nbsp;&nbsp;&nbsp;&nbsp;\n";
      echo "<INPUT type=\"text\" size=20 name=\"login_uin\" value=\"\"><BR><BR>\n";
      echo "Password: &nbsp;&nbsp;&nbsp;&nbsp;\n";
      echo "<INPUT type=\"password\" size=20 name=\"login_password\" value=\"\">\n";
      echo "</DIV>\n";
      echo "<BR><INPUT type=\"submit\" name=\"submit\" value=\"Login\">\n";
      echo "</FORM>\n";
      echo "</TD></TR></TABLE>\n";
      echo "</CENTER>\n";
      
      printFooter();
      
      return;
    }
    
    header('Location: main.php');
  }
  
  //if ((!empty($errmsg)) || empty($login_uin) || empty($login_password))
  {
    // Login failed or just loading this page
    echo "<center>\n";
    echo "<BR><BR><BR><BR><BR><BR>\n";
    echo "<TABLE border=0><TR><TD align=\"center\">\n";
    echo "<FORM method=\"POST\">\n";
    echo "<DIV align=\"right\">\n";
    
    if (!empty($errmsg))
    {
      echo "<B><FONT color=\"red\">" . $errmsg . "</FONT></B><BR><BR>";
    }

    echo "UIN: &nbsp;&nbsp;&nbsp;&nbsp;\n";
    echo "<INPUT type=\"text\" size=20 name=\"login_uin\" value=\"\"><BR><BR>\n";
    echo "Password: &nbsp;&nbsp;&nbsp;&nbsp;\n";
    echo "<INPUT type=\"password\" size=20 name=\"login_password\" value=\"\">\n";
    echo "</DIV>\n";
    echo "<BR><INPUT type=\"submit\" name=\"submit\" value=\"Login\">\n";
    echo "</FORM>\n";
    echo "</TD></TR></TABLE>\n";
    echo "</CENTER>\n";
  }
  
  printFooter()
?>
