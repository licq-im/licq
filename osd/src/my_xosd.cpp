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

#include "my_xosd.h"

#include <iostream>
#include <libintl.h>
#include <sstream>
#include <string>
#include <unistd.h>
#define _(String) gettext (String)

using namespace std;

extern void log(int mode, const char *message);

xosd *osd=0;
unsigned long Lines=0, Linelen=0;
unsigned long Timeout, DelayPerCharacter;
bool Wait=true;

std::string toString(int number)
{
	std::ostringstream os;
	os << number;
    return os.str();
}

// one time setup of the xosd display
// warning: xosd lib functions give 0 on success !!!
int my_xosd_init(string font=FONT,
		 string colour=COLOUR,
		 unsigned long hoffset=HORIZONTAL_OFFSET,
		 unsigned long voffset=VERTICAL_OFFSET,
		 string vpos=VPOS,
                 string hpos=HPOS,
		 unsigned long timeout=DISPLAYTIMEOUT,
                 unsigned long delaypercharacter=DELAYPERCHARACTER,
		 unsigned long lines=LINES,
		 unsigned long linelen=LINELEN,
		 bool wait=WAIT,
		 unsigned long shadowoffset=SHADOW_OFFSET,
		 unsigned long outlineoffset=OUTLINE_OFFSET,
		 string shadowcolour=SHADOW_COLOUR,
		 string outlinecolour=OUTLINE_COLOUR
		)
{
    // two global variables storing these values
    Lines=lines;
    Linelen=linelen;
    Wait=wait;
    xosd_pos osd_vpos=XOSD_bottom;
    xosd_align osd_hpos=XOSD_left;

    // osd is the global osd structure
    osd=xosd_create(lines);
    if (!osd)
	{
		string msg="Unable to create xosd object: ";
		msg+=toString(lines);
		msg+=" lines: ";
        msg+=xosd_error;
        log(1, msg.c_str());
		return 0;
    }
    if (xosd_set_timeout(osd, 1))
	{
        log(1, string("Unable to set timeout ").append(xosd_error).c_str());
		return 0;
    }
    Timeout=timeout;
    DelayPerCharacter=delaypercharacter;


    if (xosd_set_shadow_offset(osd, shadowoffset))
    {
        log(1, string("Unable to set shadow offset ").append(xosd_error).c_str());
	return 0;
    }

    if (xosd_set_outline_offset(osd, outlineoffset))
    {
        log(1, string("Unable to set outline offset ").append(xosd_error).c_str());
	return 0;
    }

    if (xosd_set_shadow_colour(osd, shadowcolour.c_str()))
    {
        log(1, string("Unable to set shadow colour ").append(xosd_error).c_str());
	return 0;
    }

    if (xosd_set_outline_colour(osd, outlinecolour.c_str()))
    {
        log(1, string("Unable to set outline colour ").append(xosd_error).c_str());
	return 0;
    }
    if (xosd_set_font(osd, font.c_str()))
    {
	log(1, string("Unable to set configured font ").append(xosd_error).c_str());
	if (xosd_set_font(osd, FONT))
	{
	    log(1, string("Unable to set default font ").append(xosd_error).c_str());
	    return 0;
	}
    }

    if (vpos=="top")
        osd_vpos=XOSD_top;
    else if (vpos=="bottom")
	osd_vpos=XOSD_bottom;
    else if (vpos=="middle")
	osd_vpos=XOSD_middle;
    else
    {
	log(0, "invalid vertical position");
        osd_vpos=XOSD_bottom;
    }
    if (xosd_set_pos(osd, osd_vpos))
    {
        log (1, string("unable to set vertical position").append(xosd_error).c_str());
	return 0;
    }

    if (hpos=="left")
        osd_hpos=XOSD_left;
    else if (hpos=="right")
	osd_hpos=XOSD_right;
    else if (hpos=="center")
	osd_hpos=XOSD_center;
    else
    {
	log(0, "invalid horizontal position");
        osd_hpos=XOSD_left;
    }
    if (xosd_set_align(osd, osd_hpos))
    {
	log (1, string("Unable to set specified alignment").append(xosd_error).c_str());
	return 0;
    }

    if (xosd_set_vertical_offset(osd, voffset))
    {
        log (1, string("Unable to set vertical offset ").append(xosd_error).c_str());
	return 0;
    }
    if (xosd_set_horizontal_offset(osd, hoffset))
    {
        log (1, string("Unable to set horizontal offset ").append(xosd_error).c_str());
	return 0;
    }
    if (xosd_set_colour(osd, colour.c_str()))
    {
        log (1, string("Unable to set colour ").append(colour).append(xosd_error).c_str());
	return 0;
    }
#ifdef    NEWXOSD
    xosd_set_linetimeout(osd, 1);
    xosd_set_autoscroll(osd, 1);
#endif // NEWXOSD

    // necessary because otherwise the xosd_wait_until_no_display
    // will cause a lock of timeout seconds in the my_xosd_display call
//    if (xosd_hide(osd))
//    {
//	log (1, "Unable to hide osd display");
//	return 0;
//    }
    my_xosd_display(_("System"), _("XOSD plugin initialized"));
    return 1;
}

int my_xosd_settimeout(unsigned long timeout)
{
	if (timeout>120)
        timeout=120;
	return !xosd_set_timeout(osd, timeout);
}

// free ressources
int my_xosd_exit()
{
    if (osd)
	return !xosd_destroy(osd);
    return 0;
}

#ifndef    NEWXOSD
string getWord(string message, unsigned int &pos, unsigned int maxlen)
{
    string word;
    word="";
    while ((pos<message.length()) && (((unsigned char)message.at(pos))>' ')) // as long as we are on the same word
    {
	word+=message.at(pos++); // add character to word
    }
    if ((pos<message.length()) && (word.length()==0) && (message.at(pos)=='\n')) // newline detected
    {
	word=" ";
	pos++;
    }
    else if ((pos<message.length()) && (message.at(pos)!='\n')) // we are at word boundary
	pos++;
    if (word.length()>maxlen) // if word is too long
    {
        pos-=(word.length()-maxlen);
	word=word.substr(0, maxlen);
    }
    return word;
}
#endif // !NEWXOSD

int my_xosd_display(string username, string message, string colour)
{
#ifndef    NEWXOSD
    string *text;
    string word;
    unsigned int i;
    unsigned int line=1;
#endif // !NEWXOSD

    if (!osd)
	return 0;

    if (Lines==0)
        return 0;

    // hard limit of 50 - more doesnt seems to make sense
    // change this if you need more.
    if (Lines>50)
    {
        log (1, "More than 50 lines not allowed - see my_xosd.cpp");
	return 0;
    }

    if (Linelen==0)
        return 0;

    // hard limit of 500 - more doesnt seems to make sense
    // change this if you need more.
    if (Linelen>500)
    {
        log (1, "More than 500 characters per line not allowed - see my_xosd.cpp");
	return 0;
    }

    if (Linelen<=username.length()+2) // Linelen is too short
	return 0;

    // let the old message time out
    if (Wait && xosd_wait_until_no_display(osd))
	return 0;

    if (colour.length()) {
	if (xosd_set_colour(osd, colour.c_str()))
	{
	    log (1, "Unable to set colour ");
	    return 0;
	}
    }

#ifdef   NEWXOSD
    username += ": ";
    xosd_set_message_indention_string(osd, (char *) username.c_str());
    xosd_display(osd, xosd_get_number_lines(osd), XOSD_message,
                 message.c_str());

#else // !NEWXOSD
    // scroll the current lines out of osd display
    // otherwise we get artefacts when called second time
    xosd_scroll(osd, Lines);

    text=new string[Lines];

    try {
	if ((username!="") && (username!="autoresponse"))
	{
	    // create output lines
	    text[0]=username;
	    text[0]+=": ";
	    for (i=1;i<Lines;i++) // indent second and following lines
		for(unsigned int j=0;j<username.length()+2;j++)
		    text[i]+=" ";
	    i=0;
	    line=0;
	    while ((line<Lines) && (i<message.length()))
	    {
		word=getWord(message, i, Linelen-username.length()-2);
		if (word==" ") // " " is code for newline (see getWord)
		{
		    line++;
		}
		else if (text[line].length()+word.length()<Linelen) // valid word and line long enough
		{
		    text[line]+=word;
		    text[line]+=' ';
		}
		else // line too short --> start new line
		{
		    line++;
		    if (line<Lines) // further lines available?
		    {
			text[line]+=word;
			text[line]+=' ';
		    }
		}
	    }
	    unsigned long timeout_msg=0;
	    for (i=0;i<Lines;i++)
		timeout_msg+=text[i].length();
//	    if (DelayPerCharacter!=0)
		my_xosd_settimeout(Timeout+(DelayPerCharacter*timeout_msg)/1000);
//	    else
//		my_xosd_settimeout(Timeout);

	}
	else
	{
	    text[0]=message;
//	    if (username=="autoresponse")
		my_xosd_settimeout(Timeout+(DelayPerCharacter*text[0].length())/1000);
//		else
//			my_xosd_settimeout(Timeout+(DelayPerCharacter*timeout_msg)/1000);
	}
    }
    catch (...) // some error occurred in message parsing - skip message
    {
        return 0;
    }
    // display the output lines
    for (i=0;i<Lines;i++)
	    xosd_display(osd, i, XOSD_string, text[i].c_str());
    delete[] text;
#endif // NEWXOSD
    return 1;
}
