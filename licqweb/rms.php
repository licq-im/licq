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
function readSocket() {
	global $sock;
	return socket_read($sock, 512, PHP_NORMAL_READ);
}

function sendData($cmd) {
	global $sock;
	socket_write($sock, $cmd, strlen($cmd));
}

function rmsLogin($uin, $passwd) {
	global $errmsg, $sock, $server, $port, $loggedIn, $h, $a;

	if ($loggedIn == true) {
		return;
	}
	$loggedIn = true;

	// Connect to the RMS server
	socket_connect($sock, $server, $port);
    
	// Get the version prompt
	$packet = socket_read($sock, 512, PHP_NORMAL_READ);

	// Get the UIN prompt
	$packet = socket_read($sock, 512, PHP_NORMAL_READ);

	if (!preg_match("/^300\s{1}.*/", $packet)) {
		socket_close($sock);
		$sock = 0;
		$loggedIn = false;
		return false;
	}

	//sendData($uin);
	socket_write($sock, $uin, strlen($uin));

	$packet = socket_read($sock, 512, PHP_NORMAL_READ);

	if (!preg_match("/^301\s{1}.*/", $packet)) {
		socket_close($sock);
		$sock = 0;
		$loggedIn = false;
		return false;
	}

	sendData($passwd);

	$packet = socket_read($sock, 512, PHP_NORMAL_READ);

	if (!preg_match("/^200\s{1}.*/", $packet)) {
		$errmsg = $packet;
		socket_close($sock);
		$sock = 0;
		$loggedIn = false;
		return false;
	}
	list($nick, $tmp) = sscanf($packet, "200 Hello %s %s");

	return $nick;
}

function rmsGetlist($type = 'online') {
	global $sock;
	$packet = " ";
	$cmd = "LIST $type\r\n";

	if ($sock == 0) return;

	socket_write($sock, $cmd, strlen($cmd));
	$packet = socket_read($sock, 512, PHP_NORMAL_READ);

	$users = array();
	while (!preg_match("/^206$/", $packet)) {
		if (preg_match("/^204\s{1}.*/", $packet)) {
			$user = getUserStuff($packet);
			array_push($users, $user);
			$packet = socket_read($sock, 512, PHP_NORMAL_READ);
		}
	}
	return $users;
}

function rmsViewEvent($id, $pp) {
	global $sock;
	$cmd = "VIEW " . $id . "." . $pp . "\r\n";
	sendData($cmd);

	$packet = socket_read($sock, 1024, PHP_NORMAL_READ);
	if (substr($packet, 0, 3) == 502) {
		return false;
	}

	$packet = socket_read($sock, 1024, PHP_NORMAL_READ);
	$snttime = substr($packet, 12);

	$packet = socket_read($sock, 1024, PHP_NORMAL_READ);
	$packet = socket_read($sock, 1024, PHP_BINARY_READ);
	$msg = str_replace("\r\n", "<br/>", $packet);
	$msg = str_replace("\n", '<br/>', $msg);
	preg_match("/^(.*)<br\/>223 Message Complete<br\/>$/", $msg, $matches);
	$msg = $matches[1];	
	// kses input filtering
	$allowed = array('b' => array(),
					 'i' => array(),
					 'a' => array('href' => 1, 'title' => 1),
					 'p' => array('align' => 1),
					 'br' => array(),
					 'font' => array('size' => 1, 'color' => 1, 'face' => 1)
					);
	
	if (get_magic_quotes_gpc()) {
		$msg = stripslashes($msg);
	}
	return array('msg' => kses($msg, $allowed), 'time' => trim($snttime));
}

function rmsSendMessage($id, $pp, $msg) {
	global $sock, $h, $a;

	$cmd = "MESSAGE " . $id . "." . $pp . "\r\n";
	sendData($cmd);

	$packet = socket_read($sock, 1024, PHP_NORMAL_READ);
	if (!preg_match("/^302\s{1}.*/", $packet)) {
		return false;
	}

	$msgcontent = $msg . "\r\n.\r\n";
	sendData($msgcontent);
	$packet = socket_read($sock, 1024, PHP_NORMAL_READ); //e.g. 102 [1254] Sending message to foo@hotmail.com.
	preg_match("/\[(\d+)\]/", $packet, $matches);
	$packet = socket_read($sock, 1024, PHP_NORMAL_READ); //e.g. 203 [1254] Event done
	preg_match("/\[(\d+)\] Event (.*)/", $packet, $matches2);
	while ($matches[1] != $matches2[1]) {
		//Event tag doesn't match the Event Done tag, keep trying
		$packet = socket_read($sock, 1024, PHP_NORMAL_READ);
		preg_match("/\[(\d+)\]/", $packet, $matches2);
	}
	return $matches2[2];
}

function rmsGetStatus() {
	global $sock;
	$cmd = "STATUS\r\n";
	sendData($cmd);
	$packet = socket_read($sock, 512, PHP_NORMAL_READ);
	$ownerStatus = array();
	while (!preg_match("/^212/", $packet)) {
		list($tmp, $id, $pp, $status) = explode(' ', $packet, 4);
		$ownerStatus[$pp] = array('id' => $id, 'status' => $status);
		$packet = socket_read($sock, 512, PHP_NORMAL_READ);
	}
	return $ownerStatus;
}

function rmsChangeStatus($pp, $status) {
	global $sock;
	$cmd = "STATUS $status.$pp\r\n";
	sendData($cmd);
	$packet = socket_read($sock, 512, PHP_NORMAL_READ);
	while (!preg_match("/^212/", $packet)) {
		$packet = socket_read($sock, 512, PHP_NORMAL_READ);
	}
	return true;
}

function getUserStuff($packet) {
	$id = " ";
	$status = " ";
	$nick = " ";
	$newmsgs = " ";
	$pp = " ";

	// We have a user now, time to manipulate the string!
	$i = 3;
	while ($packet{$i} == ' ') {
		$i++;
	}

	// Grab the id
	while ($packet{$i} != ' ') {
		$id .= $packet{$i};
		$i++;
	}
	$id = trim($id);

	while ($packet{$i} == ' ') {
		$i++;
	}

	// Grab the Protocol
	while ($packet{$i} != ' ') {
		$pp .= $packet{$i};
		$i++;
	}
	$stop = $i;
	$pp = trim($pp);

	// Grab the status
	$i = strlen($packet) - 1;
	while ($packet{$i} == ' ') {
		$i--;
	}
	while (!($packet{$i} >= '0' && $packet{$i} <= '9')) {
		$status .= $packet{$i};
		$i--;
	}
	$status = strrev($status);
	$status = trim($status);

	// Grab the number of new messages
	while ($packet{$i} == ' ') {
		$i--;
	}
	while ($packet{$i} != ' ') {
		$newmsgs .= $packet{$i};
		$i--;
	}
	$newmsgs = strrev($newmsgs);
	$newmsgs = trim($newmsgs);

	// Grab the nick
	while ($packet{$i} == ' ') {
		$i--;
	}
	while ($i > $stop) {
		$nick .= $packet{$i};
		$i--;
	}
	$nick = strrev($nick);
	$nick = kses(trim($nick), array());

	// Now we have a user, lets build a list struct then display the list
	$user = array('id' => $id, 'nick' => $nick, 'newmsgs' => $newmsgs, 'status' => $status, 'pp' => $pp);
	return $user;
}

function xmlentities ($string) {
   return str_replace (array('&', '"', "'", '<', '>'), array ('&amp;' , '&quot;', '&apos;' , '&lt;' , '&gt;'), $string);
}

function rmsViewHistory($id, $pp, $lenght = 10, $offset = 0)
{
  global $sock;
  $result = array();

  $cmd = "HISTORY " . $id . "." . $pp . " " . $lenght . " " . $offset . "\r\n";
  sendData($cmd);

  $packet = socket_read($sock, 1024, PHP_NORMAL_READ);
  if (substr($packet, 0, 3) >= 400) {
    return false;
  }

  // kses input filtering
  $allowed = array('b' => array(),
      'i' => array(),
      'a' => array('href' => 1, 'title' => 1),
      'p' => array('align' => 1),
      'br' => array(),
      'font' => array('size' => 1, 'color' => 1, 'face' => 1)
      );

  while (!preg_match("/^231 /", $packet))
  {
    $msg = "";
    preg_match("/from (.*)/", $packet, $header);
    $from = $header[1];
    $packet = socket_read($sock, 1024, PHP_NORMAL_READ);
    $snttime = substr($packet, 12);

    $packet = socket_read($sock, 1024, PHP_NORMAL_READ);
    $packet = socket_read($sock, 1024, PHP_NORMAL_READ);

    while (!preg_match("/^223 /", $packet))
    {
      $msg .= (($msg!="")?"<br/>":"").$packet;
      $packet = socket_read($sock, 1024, PHP_NORMAL_READ);
    }

    if (get_magic_quotes_gpc())
      $msg = stripslashes($msg);

    $result[] = array('msg' => kses($msg, $allowed), 'time' => trim($snttime), 'from' => trim($from));

    $packet = socket_read($sock, 1024, PHP_NORMAL_READ);
  }
  return $result;
}
