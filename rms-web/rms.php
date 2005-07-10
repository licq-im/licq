<?php

include "config.php";

function readSocket()
{
  global $sock, $packet;
  $packet = "";
  $packet = socket_read($sock, 512, PHP_NORMAL_READ);
}

function sendData($cmd)
{
  global $sock;
  socket_write($sock, $cmd, strlen($cmd));
}

function rmsLogin($uin, $passwd)
{
  global $errmsg, $sock, $server, $port, $loggedIn;
  
  if ($loggedIn == true)
    return;
    
  $loggedIn = true;
  
  // Connect to the RMS server
  socket_connect($sock, $server, $port);
    
  // Get the version prompt
  $packet = socket_read($sock, 512, PHP_NORMAL_READ);
  
  // Get the UIN prompt
  $packet = socket_read($sock, 512, PHP_NORMAL_READ);
  
  if (!strstr($packet, "300"))
  {
     echo "Invalid response";
     socket_close($sock);
     $sock = 0;
     $loggedIn = false;
     return false;
  }
    
  //sendData($uin);
  socket_write($sock, $uin, strlen($uin));
    
  $packet = socket_read($sock, 512, PHP_NORMAL_READ);
  
  if (!strstr($packet, "301"))
  {
    echo "Invalid response";
    socket_close($sock);
    $sock = 0;
    $loggedIn = false;
    return false;
  }
    
  sendData($passwd);
  
  $packet = socket_read($sock, 512, PHP_NORMAL_READ);
    
  if (!strstr($packet, "200"))
  {
    $errmsg = $packet;
    socket_close($sock);
    $sock = 0;
    $loggedIn = false;
    return false;
  }
  
  $_SESSION["username"] = $uin;
  $_SESSION["password"] = $passwd;

	return true;
}

function rmsGetOwnerStatus()
{
  global $sock, $ownerStatus;
  $packet = " ";
  $cmd = "STATUS\r\n";
    
  if ($sock == 0) return;
  
  socket_write($sock, $cmd, strlen($cmd));
  $packet = socket_read($sock, 512, PHP_NORMAL_READ);
  
  if (strstr($packet, "202"))
  {
    //TODO Handle all the protocols seperately
    $i = 5;
    while ($packet{$i} == ' ')
    {
      $i++;
    }
    
    $ownerStatus = "";
    while ($i < strlen($packet))
    {
      if ($packet{$i} != "\n")
        $ownerStatus .= $packet{$i};
      $i++;
    }  
  }
  else
  {
    $ownerStatus = "Unknown";
  }
}

function rmsChangeStatus($newStatus)
{
  global $sock, $ownerStatus;
  $packet = " ";
  $cmd = "STATUS " . $newStatus . "\r\n";
  
  if ($sock == 0) return;
  
  socket_write($sock, $cmd, strlen($cmd));
  $packet = socket_read($sock, 512, PHP_NORMAL_READ);
}

function rmsGetlist()
{
  global $sock, $allusers, $showonlyonline, $userArray;
  $packet = " ";
  $cmd = "LIST";
  $cmd .= "\r\n";
  
  if ($sock == 0) return;
  
  socket_write($sock, $cmd, strlen($cmd));
  $packet = socket_read($sock, 512, PHP_NORMAL_READ);
  
  while (!strstr($packet, "206"))
  {
    if (strstr($packet, "204"))
    {
      $id = " ";
      $status = " ";
      $nick = " ";
      $newmsgs = " ";
      $pp = " ";
      
      // We have a user now, time to manipulate the string!
      $i = 3;
      while ($packet{$i} == ' ')
      {
        $i++;
      }
      
      // Grab the id
      while ($packet{$i} != ' ')
      {
        $id .= $packet{$i};
        $i++;
      }
      $id = trim($id);
      
      while ($packet{$i} == ' ')
        $i++;
        
      // Grab the Protocol
      while ($packet{$i} != ' ')
      {
        $pp .= $packet{$i};
        $i++;
      }
      $stop = $i;
      $pp = trim($pp);
      
      // Grab the status
      $i = strlen($packet) - 1;
      while ($packet{$i} == ' ')
        $i--;
      while (!($packet{$i} >= '0' && $packet{$i} <= '9'))
      {
        $status .= $packet{$i};
        $i--;
      }
      $status = strrev($status);
      $status = trim($status);
      
      // Grab the number of new messages
      while ($packet{$i} == ' ')
        $i--;
      while ($packet{$i} != ' ')
      {
        $newmsgs .= $packet{$i};
        $i--;
      }
      $newmsgs = strrev($newmsgs);
      $newmsgs = trim($newmsgs);
      
      // Grab the nick
      while ($packet{$i} == ' ')
        $i--;
      while ($i > $stop)
      {
        $nick .= $packet{$i};
        $i--;
      }
      $nick = strrev($nick);
      $nick = trim($nick);
      
      // Now we have a user, lets build a list struct then display the list
      $user = array($id, $nick, $newmsgs, $status, $pp);
      if ($allusers == 0)
        $allusers = array($user);
      else
        array_push($allusers, $user);
        
      $packet = socket_read($sock, 512, PHP_NORMAL_READ);
    }
  }
}

?>
