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

//option vars
var showOffline = false;
var maxLogLines = 20;
var loadHistory = 3;

var xmlhttp = new XMLHttpRequest(); //for serverpush connection
xmlhttp.multipart = true;
var xmlhttp2; //for viewing events/sending messages

var owners = new Array();
var contacts = new Array();
var totalMessages = 0;
var nick;
var ackMessages = new Array();
var logLines = 0;
var baseurl;

//object for holding owner info
function Owner(id, pp, status) {
	this.id = id;
	this.pp = pp;
	this.status = status;
}

//object for holding a contact
function Contact(id, pp, nick, status, numMsgs) {
	this.id = id;
	this.pp = pp;
	this.nick = nick;
	this.status = status;
	this.nummsgs = numMsgs;
}

function Message(id, pp, uid, message) {
	this.id = id;
	this.pp = pp;
	this.uid = uid;
	this.message = message;
}

//callback for server push reponses
function acceptResponse() {
  if (xmlhttp.readyState == 4 && xmlhttp.responseXML != null)
  {
		if (xmlhttp.status == 200) {
			response = xmlhttp.responseXML.documentElement;
			method = response.getElementsByTagName('method')[0].firstChild.data;
			eval(method + '(response)');
		} else {
			alert("There was a problem retrieving the XML data:\n" + xmlhttp.statusText);
		}
	}
}

//callback for single events
function acceptResponse2() {
	if (xmlhttp2.readyState == 4) {
		if (xmlhttp2.status == 200) {
			response = xmlhttp2.responseXML.documentElement;
			method = response.getElementsByTagName('method')[0].firstChild.data;
			eval(method + '(response)');
		} else {
			alert("There was a problem retrieving the XML data:\n" + xmlhttp2.statusText);
		}
	}
}

//builds contact list from xml
function userList(response) {
	delete contacts;
	contacts = new Array();
	var users = response.getElementsByTagName('user');
	totalMessages = 0;
	var windowhtml = "";
	for (var i = 0; i < users.length; i++) {
		var u = users[i];
		var id = u.getElementsByTagName('id')[0].firstChild.data;
		var pp = u.getElementsByTagName('pp')[0].firstChild.data;
		var nick = u.getElementsByTagName('nick')[0].firstChild.data;
		var status = u.getElementsByTagName('status')[0].firstChild.data;
		var messages = u.getElementsByTagName('newmsgs')[0].firstChild.data;
		if (typeof(contacts[pp]) == 'undefined') {
			contacts[pp] = new Array();
		}
		contacts[pp][id] = new Contact(id, pp, nick, status, messages);
		windowhtml += getWindowHtml(id, pp, nick);
		totalMessages += parseInt(messages);
	}
	document.getElementById('windows').innerHTML = windowhtml;
	window.parent.document.title = 'licq - ' + totalMessages + " messages";
	sortContacts();
}

function viewEvent(response) {
	var id = response.getElementsByTagName('id')[0].firstChild.data;
	var pp = response.getElementsByTagName('pp')[0].firstChild.data;
	var txtdiv = document.getElementById(id + '-' + pp + '-txt');
	var messages = response.getElementsByTagName('message');
	var times = response.getElementsByTagName('time');
  for (var i = 0; i < messages.length; ++i)
    txtdiv.innerHTML += '(<span class="msgDate">' + times[i].firstChild.data
        + '</span>) <span class="msgNick">' + contacts[pp][id].nick
        + '</span>: <span class="msgText">' + messages[i].firstChild.data
        + "</span><br/>";
	txtdiv.scrollTop = txtdiv.scrollHeight;
}

function requestViewEvent(id, pp) {
	xmlhttp2 = new XMLHttpRequest();
	xmlhttp2.onreadystatechange = acceptResponse2;
	xmlhttp2.open("GET", baseurl + "/viewEvent.php?id=" + id + "&pp=" + pp, true);
	xmlhttp2.send(null);
}

function sortContacts() {
	//sort by status/messages
	var sortedContacts = new Array();
	var statuss = {'Online': 0, 'Occupied': 1, 'DoNotDisturb': 2, 'Away': 3, 'NotAvailable': 4};
	for (var status in statuss) {
		sortedContacts[status] = new Array();
	}
	if (showOffline) {
		statuss['Offline'] = 5;
		sortedContacts['Offline'] = new Array();
	}
	for (var pp in contacts) {
		for (var id in contacts[pp]) {
			for (var status in statuss) {
				if (contacts[pp][id].status.indexOf(status) != -1) {
					if (contacts[pp][id].nummsgs > 0) {
						sortedContacts[status].unshift(contacts[pp][id]);
					} else {
						sortedContacts[status].push(contacts[pp][id]);
					}
				}
			}
		}
	}
	contacthtml = "";
	var aimLower = 'abcdefghijklmnopqrstuvwxyz';
	var aimUpper = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ';
	for (var status in statuss) {
		j = sortedContacts[status].length;
		for (var i = 0; i < j; ++i) {
			var contact = sortedContacts[status][i];
			var uclass = contact.status;
			var pp = "";
			if (contact.pp.toLowerCase() == "licq" &&
				(aimLower.indexOf(contact.id.charAt(0), 0) != -1 ||
				aimUpper.indexOf(contact.id.charAt(0), 0) != -1)) {
				pp = "aim";
			} else {
				pp = contact.pp.toLowerCase();
			}
      var imgsrc = "images/" + pp + "." + contact.status.toLowerCase().replace(/ /g,"") + ".png";
			if (parseInt(contact.nummsgs) > 0) {
				uclass = "newmessage";
				imgsrc = "images/msg.png";
			}
			if (contact.status.indexOf('(') != -1) {
				imgsrc = "images/licq.invisible.png";
				uclass = "invisible";
			}
			key = contact.id + '-' + contact.pp;
			contacthtml += "<div id=\"" + key + "\"><div id=\"" + key + "-s\" class=\"" + uclass + "\" onMouseover=\"showToolTip('" + contact.pp + "', '" + contact.id + "')\" onMouseout=\"hideToolTip()\" onMouseDown=\"showContactWindow('" + contact.id + "', '" + contact.pp + "'); \"><img style=\"vertical-align: middle\" id=\"" + key + "-i\" src=\"" + imgsrc + "\" /> " + contact.nick + '</div></div>';
		}
	}
	document.getElementById('contacts').innerHTML = contacthtml;
}

function _updateUser(id, pp, status, messages, nick) {
	if (typeof(contacts[pp]) == 'undefined') {
		contacts[pp] = new Array();
	}
	if (typeof(contacts[pp][id]) != 'undefined') {
		contacts[pp][id].status = status;
		contacts[pp][id].nummsgs = messages;
		contacts[pp][id].nick = nick;
    contacts[pp][id].historyShowed = false;
  }
  else
  {
		contacts[pp][id] = new Contact(id, pp, nick, status, messages);
		var newcontact = document.createElement('div');
		newcontact.innerHTML = getWindowHtml(id, pp, nick);
		document.getElementById('windows').appendChild(newcontact);
		//document.getElementById('windows').innerHTML += getWindowHtml(id, pp, nick);
		var txtdiv = document.getElementById(id + '-' + pp + '-txt');
		txtdiv.scrollTop = txtdiv.scrollHeight;
	}
}

function getWindowHtml(id, pp, nick) {
	var key = id + '-' + pp;
	return "<div onClick=\"set_focus(event, '" + key + "')\" class=\"window\" style=\"left:300px;top:150px\" id=\"" + key + "-w\">" +
				"<div onmousedown=\"init_drag(event, '" + key + "-w')\" class=\"bar\">" +
					"<div class=\"wintitle\">" + nick + " (" + pp + ")</div>" +
					"<div onclick=\"showContactWindow('" + id + "', '" + pp + "')\" class=\"close\">[close]</div>" +
				"</div>" +
				"<div class=\"convo\" id=\"" + key + "-txt\"></div>" + 
				"<div class=\"msginput\"><textarea id=\"" + key + "-input\" onKeyPress=\"textarea_keypress(event, '" + id + "', '" + pp + "')\"></textarea><br>" +
					"<input type=\"submit\" class=\"button\" value=\"Send\" onclick=\"sendMessage('" + id + "', '" + pp + "'); return false\" />" +
          "<div class=\"resizer\" onmousedown=\"init_resize(event, '" + key + "-txt', 150, 50)\"><img class=\"resizerImage\" src=\"images/resizerMsg.png\" /></div>" +
				"</div>" +
			"</div>";
}

function textarea_keypress(event, id, pp) {
	//check for ctrl+enter on message textarea
	if (event.ctrlKey && event.keyCode == 13) {
		sendMessage(id, pp);
	}
}

function sendMessage(id, pp) {
	var uid = Math.random() * 1000;
	while (typeof(ackMessages[uid]) != 'undefined') {
		uid = Math.random() * 1000;
	}
	var message = document.getElementById(id + '-' + pp + '-input').value;
	ackMessages[uid] = new Message(id, pp, uid, message);
	xmlhttp2 = new XMLHttpRequest();
	xmlhttp2.onreadystatechange = acceptResponse2;
	xmlhttp2.open("POST", baseurl + "/sendMessage.php", true);
	xmlhttp2.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
	xmlhttp2.send('uid=' + uid + '&id=' + id + '&pp=' + pp + '&msg=' + escape(message));
	document.getElementById(id + '-' + pp + '-input').disabled = true;
	document.getElementById(id + '-' + pp + '-input').value = '';
}

function ackChangeStatus(response) {
}

function ackSendMessage(response) {
	var uid = response.getElementsByTagName('uid')[0].firstChild.data;
	var res = response.getElementsByTagName('result')[0].firstChild.data;
	var ts = response.getElementsByTagName('datetime')[0].firstChild.data;
	var message = ackMessages[uid];
	var txt = document.getElementById(message.id + '-' + message.pp + '-txt');
  if (res == "done.")
    txt.innerHTML += "(<span class=\"msgDate\">" + ts + "</span>) <span class=\"msgNick\">" + nick + "</span>: <span class=\"msgText\">" + message.message.replace(/[\r\n]+/g, "<br/>") + "</span><br/>";
  else
		txt.innerHTML += "--- Message failed!<br/>";
	txt.scrollTop = txt.scrollHeight;
	document.getElementById(message.id + '-' + message.pp + '-input').disabled = false;
	delete ackMessages[uid];
}

function showContactWindow(id, pp) {
	var win;
	if (contacts[pp][id].nummsgs > 0) {
		requestViewEvent(id, pp);
	}
  if (loadHistory > 0 && contacts[pp][id].historyShowed != true)
  { 
    requestViewHistory(id, pp);
    contacts[pp][id].historyShowed = true;
  }

	win = document.getElementById(id + '-' + pp + '-w');
	if (win.style.display == 'block') {
		win.style.display = 'none';
	} else {
		win.style.display = 'block';
	}
	var txtdiv = document.getElementById(id + '-' + pp + '-txt');
	txtdiv.scrollTop = txtdiv.scrollHeight;
	//win.style.zIndex = ++dragwin.win.style.zIndex;
	//dragwin.win = win;
}

function updateStatus(response) {
	var newstatus = response.getElementsByTagName('newstatus');
	var id = newstatus[0].getElementsByTagName('id')[0].firstChild.data;
	var pp = newstatus[0].getElementsByTagName('pp')[0].firstChild.data;
	var status = newstatus[0].getElementsByTagName('status')[0].firstChild.data;
	var nick = newstatus[0].getElementsByTagName('nick')[0].firstChild.data;
	var nummsgs = newstatus[0].getElementsByTagName('nummsgs')[0].firstChild.data;
	if (owners[pp].id == id) {
		owners[pp].status = status;
		_updateOwners();
		return;
	}
	_updateUser(id, pp, status, nummsgs, nick);
	sortContacts();
}

function newMessage(response) {
	var newmsg = response.getElementsByTagName('newmessage');
	var id = newmsg[0].getElementsByTagName('id')[0].firstChild.data;
	var pp = newmsg[0].getElementsByTagName('pp')[0].firstChild.data;
	var nummsgs = newmsg[0].getElementsByTagName('nummsgs')[0].firstChild.data;
	var key = id + '-' + pp;
	var oldnummsgs = parseInt(contacts[pp][id].nummsgs);
	if (parseInt(nummsgs) > oldnummsgs) {
		totalMessages += 1;
		if (document.getElementById(key + '-w').style.display == 'block') {
			//window is open, display the message
			requestViewEvent(id, pp);
		}
	} else if (parseInt(nummsgs) < oldnummsgs) {
		totalMessages -= 1;
	}
	contacts[pp][id].nummsgs = nummsgs;
	if (document.getElementById(key + '-s')) {
		if (parseInt(nummsgs) > 0) {
			document.getElementById(key + '-s').className = "newmessage";
			document.getElementById(key + '-i').src = "images/msg.png";
		} else {
			document.getElementById(key + '-s').className = contacts[pp][id].status;
      document.getElementById(key + '-i').src = "images/" + pp.toLowerCase() + "." + contacts[pp][id].status.toLowerCase().replace(/ /g,"") + '.png';
    }
  }
	window.parent.document.title = 'licq - ' + totalMessages + " messages";
	sortContacts();
}

function setOwnerInfo(response) {
	delete owners;
	owners = new Array();
	var ownerList = response.getElementsByTagName('owner');
	for (var i = 0; i < ownerList.length; i++) {
		var o = ownerList[i];
		var id = o.getElementsByTagName('id')[0].firstChild.data;
		var pp = o.getElementsByTagName('pp')[0].firstChild.data;
		var status = o.getElementsByTagName('status')[0].firstChild.data;
		owners[pp] = new Owner(id, pp, status);
	}
	_updateOwners();
}

function _updateOwners() {
	var statushtml = "";
  for (var pp in owners)
    statushtml += "<img onclick=\"showSelectStatus(event, '" + owners[pp].id + "', '" + pp + "'); return false\" src=\"images/" + pp.toLowerCase() + "." + owners[pp].status.toLowerCase().replace(/ /g,"") + ".png\"/> ";
	document.getElementById('ownerStatus').innerHTML = statushtml;
}

function showSelectStatus(e, id, pp) {
	var statusMenu = document.getElementById('statusMenu');
	var statuss = new Array();
  statuss["Licq"] = new Array('Online', 'Away', 'Occupied', 'Do Not Disturb', 'Not Available', 'Offline');
	statuss["MSN"] = new Array('Online', 'Away', 'Occupied', 'Offline');
	var statushtml = "";
	for (var i = 0; i < statuss[pp].length; ++i) {
    statushtml += "<div onclick=\"changeStatus('" + pp + "', '" + statuss[pp][i] + "')\"><img src=\"images/" + pp.toLowerCase() + "." + statuss[pp][i].toLowerCase().replace(/ /g,"") + ".png\">" + statuss[pp][i] + "</div>";
	}
	statusMenu.innerHTML = statushtml;
  statusMenu.style.left = e.pageX + 'px';
  statusMenu.style.top = e.pageY + 'px';
	statusMenu.style.display = 'block';
  if (dragwin.win != null)
	statusMenu.style.zIndex = ++dragwin.win.style.zIndex; //fix this
}

function changeStatus(pp, status) {
	xmlhttp2 = new XMLHttpRequest();
	xmlhttp2.onreadystatechange = acceptResponse2;
	xmlhttp2.open("POST", baseurl + "/changeStatus.php", true);
	xmlhttp2.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
  xmlhttp2.send('pp=' + pp + '&status=' + escape(status.replace(/ /g,"")));
	document.getElementById('statusMenu').style.display = 'none';
}

function setOwnerNick(response) {
	nick = response.getElementsByTagName('result')[0].firstChild.data;
	document.getElementById('nick').innerHTML = 'licqweb - ' + nick;
}

function log(response) {
	result = response.getElementsByTagName('result')[0].firstChild.data;
	var log = document.getElementById('log');
	var txt = document.getElementById('log').innerHTML;
	++logLines;
	if (logLines >= maxLogLines) {
		txt = txt.substring(txt.indexOf('<br') + 4);
		--logLines;
	}
	document.getElementById('log').innerHTML = txt + result + '<br/>';
	document.getElementById('log').scrollTop = document.getElementById('log').scrollHeight;
}

function doLogin(uin, password) {
	var url = document.location.toString();
	baseurl = url.substring(0, url.lastIndexOf('/'));
	var listtype = 'online';
	if (showOffline) {
		listtype = 'all';
	}
	var login = "";
	if (typeof(uin) != "undefined" && typeof(password) != "undefined") {
		login = "&uin=" + uin + "&password=" + password;
	}
	xmlhttp.open("GET", baseurl + "/push.php?listtype=" + listtype + login, true);
	xmlhttp.onreadystatechange = acceptResponse;
	xmlhttp.send(null);
	document.getElementById('login').style.display = 'none';
	document.getElementById('contactList').style.display = 'block';
}

function login() {
	doLogin(document.getElementById('uin').value, document.getElementById('password').value);
}

function checkSession() {
	if (document.cookie.indexOf('PHPSESSID') != -1) {
		doLogin();
	}
}

function loginFailed(response) {
	result = response.getElementsByTagName('result')[0].firstChild.data;
	document.getElementById('debug').innerHTML = result;
}

function stop() {
	xmlhttp.abort()
}

/* Tool tip stuff */
var enabletip = false;
var tooltip = document.getElementById('tooltip');

function showToolTip(pp, id){
	tooltip.innerHTML = contacts[pp][id].id + ' (' + contacts[pp][id].pp + ')<br>' + contacts[pp][id].status + '<br>Messages: ' + contacts[pp][id].nummsgs;
    enabletip = true;
    return false;
}

function positionToolTip(e){
    if (enabletip) {
        tooltip.style.left = e.pageX - 10  + "px";
        tooltip.style.top = e.pageY + 20  + "px";
        tooltip.style.visibility="visible";
    }
}

function hideToolTip() {
    enabletip = false;
    tooltip.style.visibility = "hidden";
    tooltip.style.left = "-1000px";
    tooltip.style.backgroundColor = '';
    tooltip.style.width = '';
}

document.onmousemove = positionToolTip


/* Window dragging stuff */
var dragwin = new Object();
dragwin.zIndex = 0;

function init_drag(event, id)
{
	dragwin.win = document.getElementById(id);

	dragwin.startX = event.clientX + window.scrollX;
	dragwin.startY = event.clientY + window.scrollY;
	dragwin.startLeft = parseInt(dragwin.win.style.left, 10);
	dragwin.startTop = parseInt(dragwin.win.style.top,  10);

	if (isNaN(dragwin.startLeft)) dragwin.startLeft = 0;
	if (isNaN(dragwin.startTop)) dragwin.startTop = 0;

	dragwin.win.style.zIndex = ++dragwin.zIndex;

	document.addEventListener("mousemove", start_drag, true);
	document.addEventListener("mouseup", stop_drag, true);
	event.preventDefault();
}

function start_drag(event) {
	var x, y;
	x = event.clientX + window.scrollX;
	y = event.clientY + window.scrollY;

	dragwin.win.style.left = (dragwin.startLeft + x - dragwin.startX) + "px";
	dragwin.win.style.top = (dragwin.startTop + y - dragwin.startY) + "px";
	event.preventDefault();
}

function stop_drag(event) {
	document.removeEventListener("mousemove", start_drag, true);
	document.removeEventListener("mouseup", stop_drag, true);
}

function set_focus(event, key)
{
  win = document.getElementById(key + "-w");
  win.style.zIndex = ++dragwin.zIndex;
  event.preventDefault();

  input = document.getElementById(key + "-input");
  input.focus();

  //stop_propagate(event);
}

function stop_propagate(event)
{
  if (!event)
    var event = window.event;
  event.cancelBubble = true;
  if (event.stopPropagation)
    event.stopPropagation();
}

/* Window resizing stuff */
var resizewin = new Object();

function init_resize(event, id, minW, minH)
{
  resizewin.win = document.getElementById(id);
  resizewin.minW = minW;
  resizewin.minH = minH;

  resizewin.startX = event.clientX + window.scrollX;
  resizewin.startY = event.clientY + window.scrollY;
  resizewin.startWidth = parseInt(resizewin.win.style.width, 10);
  resizewin.startHeight = parseInt(resizewin.win.style.height, 10);

  if (isNaN(resizewin.startWidth))
    resizewin.startWidth = resizewin.win.offsetWidth;
  if (isNaN(resizewin.startHeight))
    resizewin.startHeight = resizewin.win.offsetHeight;

  //resizewin.win.style.zIndex = ++resizewin.zIndex;

  document.addEventListener("mousemove", do_resize, true);
  document.addEventListener("mouseup", stop_resize, true);
  event.preventDefault();
}

function do_resize(event)
{
  var x, y;
  x = event.clientX + window.scrollX;
  y = event.clientY + window.scrollY;

  if(resizewin.minW < (resizewin.startWidth + x - resizewin.startX))
  {
    resizewin.win.style.width = (resizewin.startWidth + x - resizewin.startX) + "px";
    resizewin.win.parentNode.style.width = (resizewin.startWidth + x - resizewin.startX) + "px";
  }
  if(resizewin.minH < (resizewin.startHeight + y - resizewin.startY))
    resizewin.win.style.height = (resizewin.startHeight + y - resizewin.startY) + "px";

  event.preventDefault();
}

function stop_resize(event)
{
  document.removeEventListener("mousemove", do_resize, true);
  document.removeEventListener("mouseup", stop_resize, true);
}

function requestViewHistory(id, pp)
{
  xmlhttp2 = new XMLHttpRequest();
  xmlhttp2.onreadystatechange = acceptResponse2;
  xmlhttp2.open("GET", baseurl + "/viewHistory.php?id=" + id + "&pp=" + pp + "&lenght=" + loadHistory + "&offset=0", true);
  xmlhttp2.send(null);
}

function viewHistory(response)
{
  var id = response.getElementsByTagName('id')[0].firstChild.data;
  var pp = response.getElementsByTagName('pp')[0].firstChild.data;
  var txtdiv = document.getElementById(id + '-' + pp + '-txt');
  var messages = response.getElementsByTagName('message');
  var times = response.getElementsByTagName('time');
  var froms = response.getElementsByTagName('from');
  for (var i = messages.length-1; i >=0 ; --i)
    txtdiv.innerHTML += '(<span class="msgDate">' + times[i].firstChild.data + '</span>) <span class="msgNick">' + froms[i].firstChild.data + '</span>: <span class="msgText">' + messages[i].firstChild.data + "</span><br/>";

  txtdiv.scrollTop = txtdiv.scrollHeight;
}
