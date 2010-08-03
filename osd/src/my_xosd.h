/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2003-2005 Martin Maurer (martinmaurer@gmx.at)
 * Copyright (C) 2007-2010 Licq developers
 *
 * Licq is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Licq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Licq; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef my_xosd_h
#define my_xosd_h

#include <string>

#include <xosd.h>

int my_xosd_init(std::string font, // font which should be used (in xfontsel form)
    std::string colour, // colour of OSD (as in Xfree86's rgb.txt)
		 unsigned long hoffset, // offset from left/right of screen
		 unsigned long voffset, // offset from top/bottom of screen
    std::string vpos, // vertical position = top, bottom
    std::string hpos, // horizontal position = left, right
		 unsigned long timeout, // how long to display a message
                 unsigned long delaypercharacter,
		 unsigned long lines, // how many lines to display
		 unsigned long linelen, // how long can a line be
		 bool wait, // when a message arrives before timeout has passed - should we wait ?
         unsigned long shadowoffset,
         unsigned long outlineoffset,
    std::string shadowcolour,
    std::string outlinecolour
		);
int my_xosd_exit();

int my_xosd_settimeout(unsigned long timeout);
// if username is "" then the one line(!!!) message is displayed
// unformatted
// otherwise username is prepended on first line, and further lines are
// intended - that only works correctly if the chosen font is
// fixed width
int my_xosd_display(std::string username, std::string message, std::string colour = "");

// Maximum length of each line in OSD
#define LINELEN 60
// how long a message sholud be displayed
#define DISPLAYTIMEOUT 5
// offset from top/bottom of screen
#define VERTICAL_OFFSET 90
// offset from left/right of screen
#define HORIZONTAL_OFFSET 0
// colour of the OSD messages as in XFree86's rgb.txt
#define COLOUR "yellow"
// colour of the OSD control messages as in XFree86's rgb.txt
#define CONTROLCOLOUR "grey"
// font as in xfontsel
#define FONT "-*-*-*-*-*-*-24-*-*-*-*-*-*-*"
// position of OSD
#define VPOS "bottom"
#define HPOS "left"
// how many lines may the OSD have (maximum)
#define LINES 20
// if a second message arrives before timeout has passed - should we wait ?
// if false then the previous message will be erased and the new will be
// displayed instead for timeout seconds
// if true then the next message will be displayed after the first has timeouted.
#define WAIT true
// show messages of users ?
// 0 = no
// 1 = all users
// 2 = only for online notify users
// 3 = information only about received message for all users
// 4 = information only about received message for online notify users
#define SHOWMESSAGES 1
// show logon/logoff of users ?
// 0 = no
// 1 = all users
// 2 = only for online notify users
#define SHOWLOGON 0
// how long after our logon or logoff should no message be displayed
// this is some sort of a hack:
// when we logon we get all the users who are online at the moment
// as an logon event. Usually you don't want to show all of them in a row
// (especially when wait = true), but only the ones who log on / off while
// you are online.
#define QUIETTIMEOUT 10
// offset for shadow, 0 equals no shadow
#define SHADOW_OFFSET 0
// offset for outline, 0 equals no outline
#define OUTLINE_OFFSET 2
// colour for shadow
#define SHADOW_COLOUR "black"
// offset for outline
#define OUTLINE_COLOUR "black"
// show status change of users?
// 0 = no
// 1 = all users
// 2 = only for online notify users
#define SHOWSTATUSCHANGE 0
// this is an extra delay per character of the message.
// by using this you can achieve, that long messages are displayed
// longer than short ones
// The delay is specified in milliseconds, though only full seconds
// will take effect
// example: if DELAYPERCHARACTER is 200, then a message with 12 characters
// will be displayed for DISPLAYTIMEOUT+2 seconds
// (-->200*12 / 1000(1second) = 2.4, 0.4 ignored)
#define DELAYPERCHARACTER 100

// show an osd message whenever a user checks your auto response?
// 0 = no
// 1 = all users
// 2 = only for online notify users
#define SHOWAUTORESPONSECHECK 0

// If this option is enabled, then the osd messages will only be shown
// if _you_ are in one of those stati.
#define SHOWINMODESSTR "FreeForChat,Online,Away,NA,Occupied,DND,Invisible"
// But msgs will be shown in those stati, even if ^ says not to (statchanges won't)
#define SHOWMSGSINMODESSTR ""

// mark messages coming from a seucre channel with (S)
#define MARKSECUREMESSAGES 0

#endif
