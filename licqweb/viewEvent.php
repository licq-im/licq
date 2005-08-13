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
require_once('rms.php');

session_start();
$uin = $_SESSION['uin'];
$password = $_SESSION['password'];
session_write_close();

if (!rmsLogin("$uin\r\n", "$password\r\n")) {
	header('Content-Type: text/xml');
	echo "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>";
	echo "<response><method>loginFailed</method><result>Couldn't log in to rms plugin!</result></response>";
	exit;
}

$messages = "";
while (is_array($message = rmsViewEvent($_GET['id'], $_GET['pp']))) {
	$messages .= "<message>" . xmlentities($message['msg']) . "</message><time>" . $message['time'] . "</time>";
}

header('Content-Type: text/xml');
echo "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>";
echo "
<response>
  <method>viewEvent</method>
  <result><id>" . $_GET['id'] . "</id><pp>" . $_GET['pp'] . "</pp><messages>$messages</messages></result>
</response>
";
?>
