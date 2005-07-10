<?
// Globals
$errmsg = "";
$packet = "";
$sock = socket_create(AF_INET, SOCK_STREAM, 0);
$allusers = 0;
$ownerStatus = "";
$userArray = array();
$loggedIn = false;
$title = "";
$version = "0.20";

// Variables
$listrefresh=31;
$server="192.168.0.100";
$port=33333;
$showonlyonline=1;
?>
