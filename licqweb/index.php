<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
    <head>
        <title>licq</title>
		<link rel="StyleSheet" type="text/css" href="licqweb.css" />
		<link rel="shortcut icon" href="images/favicon.ico">
    </head>
    <body onload="checkSession()">
	<div id="tooltip"></div>
	<script type="text/javascript" src="licqweb.js" />
    <h1>licq<i><font color="#5555ff">web</font></i></h1>
	<div id="login" class="loginwin">
		<div class="bar">licqweb login</div>
		<div class="centre">
			UIN <input id="uin" type="text"/><p>
			Password <input id="password" type="password"/><p>
			<input type="submit" onclick="login(); return false;" value="Login" />
		</div>
	</div>
	<div id="contactList" class="box" style="left:10px;top:150px;display:none;">
		<div class="bar" onmousedown="init_drag(event, 'contactList')" id="nick">licqweb</div>
		<div id="contacts"></div>
		<div id="ownerStatus"></div>
	</div>
	<div id="statusMenu"></div>
	<div id="windows"></div>
	<div class="logWindow" id="log"></div>
	<div id="debug"></div>
<?
	$fd = fopen("http://www.licq.org/licqweb-news.txt", "r");
	if (!$fd)
		$fd = fopen("http://www.thejon.org/licqweb-news.txt", "r");
	if ($fd) {
		$str = "";
		while(!feof($fd))
			$str .= fread($fd, 4096);
		fclose($fd);
	}
	echo "<div id=\"newsid\" class=\"news\"><div class=\"bar\">News</div>".$str."</div>\n";
?>
    </body>
</html>
