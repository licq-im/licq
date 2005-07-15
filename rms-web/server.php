<?php
include "check_session.php";
include "rms.php";
include "config.php";

$cmd = $_POST["cmd"];

switch($cmd)
{
  case "chgStatus":
    $newStatus = $_POST["status"];
    rmsLogin($username, $password);
    rmsChangeStatus($newStatus);
    
    header("Location: list.php");
    break;
}

?>
