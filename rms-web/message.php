 
<?php

include "config.php";
include "check_session.php";
include "rms.php";

function rmsHandleCommand($cmd, $id = "", $pp = "")
{
  switch ($cmd)
  {
    case "MSG":
      rmsInputMessage($id, $pp);
      break;
    case "VIEW":
      rmsViewMessage($id, $pp);
      break;
    case "frmMsg":
      rmsSendMessage();
  }
}

function rmsInputMessage($id, $pp)
{
?>
  <SCRIPT type="text/javascript">
  function clearMsg()
  {
    document.location = "message.php"
  }
  
  //-->
  </SCRIPT>
  
  <CENTER><BR><BR>
  <FORM method="POST">
    <TABLE border=1>
    <TR>
      <TD>
        To <INPUT type="text" size="40" name="send_id" value="<?php echo $id ?>">
      </TD>
    </TR>
    <TR>
      <TD>
        <TEXTAREA name="message" cols="48" rows="4"></TEXTAREA>
      </TD>
    </TR>
    <TR>
      <TD>
        <INPUT type="submit" name="submit" value="Send">
        <INPUT type="button" onClick="clearMsg()" value="Cancel">
      </TD>
    </TR>
    <INPUT type="hidden" name="cmd" value="frmMsg">
    <INPUT type="hidden" name="pp" value="<?php echo $pp ?>">
    </TABLE>
  </FORM>

<?php
}

function rmsViewMessage($id, $pp)
{
  global $username, $password, $sock;
  
  rmsLogin($username, $password);
  
  $cmd = "VIEW " . $id . "." . $pp . "\r\n";
  sendData($cmd);
  
  $packet = socket_read($sock, 1024, PHP_NORMAL_READ);
  $event = substr($packet, 4);
  
  $packet = socket_read($sock, 1024, PHP_NORMAL_READ);
  $snttime = substr($packet, 12);
  
  $packet = socket_read($sock, 1024, PHP_NORMAL_READ);
  $packet = socket_read($sock, 1024, PHP_BINARY_READ);
  $msg = substr($packet, 0, strpos($packet, '223'));
  
  $msg = str_replace("\r\n", "<BR>", $msg);
  $msg = str_replace("\n", "<BR>", $msg);
  
  $location = "message.php?command=MSG&id=" . $id . "&pp=" . $pp;
?>

  <SCRIPT type="text/javascript">
  <!--
  function reply()
  {
    document.location = "<?php echo $location ?>"
  }
  
  function clearMsg()
  {
    document.location = "message.php"
  }
  
  //-->
  </SCRIPT>

  <CENTER>
  <BR><BR>
  <TABLE border=1>
    <TR>
      <TD>
        <?php echo $event; ?>
        at <?php echo $snttime; ?>
      </TD>
    </TR>
    <TR>
      <TD>
        <?php echo $msg; ?>
      </TD>
    </TR>
    <TR>
      <TD>
        <FORM>
          <INPUT type="button" onClick="reply()" value="Reply">
          <INPUT type="button" onClick="clearMsg()" value="Clear">
        </FORM>
      </TD>
    </TR>
  </TABLE>
  </CENTER>
  
<?php

}

function rmsSendMessage()
{
  global $username, $password, $sock;
  $id = $_POST["send_id"];
  $msg = $_POST["message"];
  $pp = $_POST["pp"];
  
  rmsLogin($username, $password);
  
  $cmd = "MESSAGE " . $id . "." . $pp . "\r\n";
  sendData($cmd);
  
  $packet = socket_read($sock, 1024, PHP_NORMAL_READ);
  if (!strstr($packet, "302"))
  {
    echo "<BR><B>Invalid response (" . $packet . ")</B><BR>\n";
    return;
  }
  
  $msgcontent = $msg . "\r\n.\r\n";
  sendData($msgcontent);

  header('Location: message.php');
}

//----------------------------------------
// Here is where we start the HTML coding
//----------------------------------------

  $cmd = $_POST["cmd"];
  $id = $_GET["id"];
  $pp = $_GET["pp"];
  $command = $_GET["command"];

  if (!empty($cmd))
  {
    rmsHandleCommand($cmd);
  }
  else if (!empty($command))
  {
    rmsHandleCommand($command, $id, $pp);
  }

?>
