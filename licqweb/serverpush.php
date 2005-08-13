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
$boundary = md5(uniqid(time()));

function http_push_end() {
	global $boundary;
    echo "--$boundary--\r\n";
}

function http_push_start($time_limit = 0) {
	global $boundary;
	$contentType = "multipart/x-mixed-replace; boundary=$boundary";
	header("Content-type: $contentType");
	ob_implicit_flush();
	set_time_limit($time_limit);
	register_shutdown_function('http_push_end');
}
    
function push(&$content, $contentType = 'text/xml') {
	global $boundary;
	static $is_first = true;
	if ($is_first) {
		echo "--$boundary\r\n";
	}
	echo "Content-type: $contentType\r\n\r\n";
	echo "$content\r\n";
	echo "--$boundary\r\n";
	$is_first = false;
}
?>
