<?
session_start();
/* clear any post/get input of identification */
$username = "";
$password = "";

/* if your server uses load balancing or anything that
 * causes session variables to be mixed up you can provide
 * a fixed location (physical directory) here. must have
 * full execute/read/write access! */
//session_save_path( "/home/allusers/myself/htdocs/tmp" );
session_set_cookie_params(60*60*5); // five hours

if ( isset( $_SESSION["username"] ) )
{
  $username = $_SESSION["username"];
  $password = $_SESSION["password"];
}
?>
