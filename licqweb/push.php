<?
/*
 * licqweb. Copyright 2005, Philip Nelson
 *
 * This file is part of licqweb.
 *
 * licqweb is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * licqweb is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with licqweb; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
require_once('config.php');
require_once('serverpush.php');
require_once('rms.php');

define('CODE_NOTIFYxSTATUS', '600');
define('CODE_NOTIFYxMESSAGE', '601');
define('CODE_LOG', '103');
session_set_cookie_params(0, substr($_SERVER['REQUEST_URI'], 0, strrchr($_SERVER['REQUEST_URI'], '/')));
session_start();
if ($_GET['uin'] || $_GET['password']) {
	$_SESSION['uin'] = $uin;
	$_SESSION['password'] = $password;
	$uin = $_GET['uin'];
	$password = $_GET['password'];
} else {
	$uin = $_SESSION['uin'];
	$password = $_SESSION['password'];
}
session_write_close();
if (!$uin || !$password) {
	header('Content-Type: text/xml');
	echo "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>";
	echo "<response><method>loginFailed</method><result>Couldn't log in to rms plugin!</result></response>";
	exit;
}

$nick = rmsLogin("$uin\r\n", "$password\r\n");
if (!$nick) {
	header('Content-Type: text/xml');
	echo "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>";
	echo "<response><method>loginFailed</method><result>Couldn't log in to rms plugin!</result></response>";
	exit;
}

$nick = substr($nick, 0, -1);
$cmd = "NOTIFY\r\n";
socket_write($sock, $cmd, strlen($cmd)); 
socket_read($sock, 512, PHP_NORMAL_READ);

/*
$cmd = "LOG 15\r\n";
socket_write($sock, $cmd, strlen($cmd)); 
socket_read($sock, 512, PHP_NORMAL_READ);
*/

http_push_start();

$ownerInfo = rmsGetStatus();
$ownerxml = "";
foreach ($ownerInfo as $pp => $info) {
	$ownerxml .= "<owner><id>" . $info['id'] . "</id><pp>$pp</pp><status>" . $info['status'] . "</status></owner>";
}
$message = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<response><method>setOwnerInfo</method><result>$ownerxml</result></response>";
push($message);

$message = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<response><method>setOwnerNick</method><result>" . xmlentities($nick) . "</result></response>";
push($message);

$users = rmsGetlist($_GET['listtype']);
if (count($users) > 0) {
	$userstxt = "<users>";
	foreach ($users as $user) {
		$userstxt .= "<user><id>" . xmlentities($user['id']) . "</id><nick>" . str_replace("\n", '', xmlentities($user['nick'])) . "</nick><newmsgs>" . trim(xmlentities($user['newmsgs'])) . "</newmsgs><status>" . str_replace(' ', '', xmlentities($user['status'])) . "</status><pp>" . xmlentities($user['pp']) . "</pp></user>";
	}
	$userstxt .= "</users>";
	$res = "
		<response>
			<method>userList</method>
			<result>$userstxt</result>
		</response>";
	$message = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n" . $res;
	push($message);
}

while (true) {
	$packet = socket_read($sock, 512, PHP_NORMAL_READ);
	if (!$packet) break;
	$code = substr($packet, 0, 3);
	switch ($code) {
		case CODE_NOTIFYxSTATUS:
			$user = getUserStuff($packet);
			$method = "updateStatus";
			$txt = "<newstatus><id>" . xmlentities($user['id']) . "</id><pp>" . xmlentities($user['pp']) . "</pp><nick>" . str_replace("\n", '', xmlentities(kses($user['nick']))) . "</nick><nummsgs>" . trim(xmlentities($user['newmsgs'])) . "</nummsgs><status>" . str_replace(' ', '', xmlentities($user['status'])) . "</status></newstatus>";
			break;
		case CODE_NOTIFYxMESSAGE:
			list($id, $pp, $numMessages) = split(' ', substr($packet, 4), 3);
			$method = "newMessage";
			$txt = "<newmessage><id>$id</id><pp>$pp</pp><nummsgs>$numMessages</nummsgs></newmessage>";
			break;
		case CODE_LOG:
			$method = "log";
			$txt = substr($packet, 4);
			break;
		default:
			continue;
	}
	$message = "
		<response>
		  <method>$method</method>
		  <result>$txt</result>
		</response>
	";
	$message = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n" . $message;
	push($message);
}
?>
