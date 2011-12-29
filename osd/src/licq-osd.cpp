/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2003-2005 Martin Maurer (martinmaurer@gmx.at)
 * Copyright (C) 2007-2011 Licq developers
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

#include "licq-osd.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iconv.h>
#include <iostream>
#include <langinfo.h>
#include <libintl.h>
#include <locale.h>
#include <unistd.h>

#include <licq/logging/log.h>
#include <licq/contactlist/owner.h>
#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>
#include <licq/daemon.h>
#include <licq/event.h>
#include <licq/inifile.h>
#include <licq/pluginsignal.h>
#include <licq/userevents.h>

#include "my_xosd.h"
#include "licq_osd.conf.h"
#include "pluginversion.h"

using namespace std;
using Licq::User;
using Licq::UserId;
using Licq::gLog;
using Licq::gUserManager;

//#if CVSLICQ==1
//#warning compiling for licq>=1.2.8
//#else
//#undef CVSLICQ
//#warning compiling for licq<1.2.8
//#endif

#define _(String) gettext (String)

// if you don't want to use codepage translation, comment out this
#define CP_TRANSLATE

struct Config {
    unsigned long Showmessages; //=SHOWLOGON;
    unsigned long Showlogon; //=SHOWLOGON;
    unsigned long ShowStatusChange; //=SHOWSTATUSCHANGE;
    unsigned long ShowAutoResponseCheck; //=SHOWAUTORESPONSECHECK;
    unsigned long quiettimeout; //=QUIETTIMEOUT;
    string pluginfont;
  unsigned showInModes;
  unsigned showMsgsInModes;
    string colour;
    string controlcolour;
    bool osd_wait;
    unsigned long timeout;
    unsigned long hoffset;
    unsigned long voffset;
    unsigned long linelen, lines;
    unsigned long shadowoffset;
    unsigned long outlineoffset;
    unsigned long DelayPerCharacter;
    string vpos;
    string hpos;
    string shadowcolour;
    string outlinecolour;
    string localencoding;
    bool marksecuremessages;
} config;


// some forward declarations
void ProcessSignal(Licq::PluginSignal* s);
void ProcessEvent(Licq::Event* e);
#ifdef CP_TRANSLATE
    const char *get_iconv_encoding_name(const char *licq_encoding_name);
string my_translate(const UserId& userId, const string& msg, const char* userenc);
#endif

// some variables representing the internal state
time_t disabletimer=0;
bool Online;
bool Enabled;
bool Configured=false; // is the xosd display initialized?

using namespace std;

OsdPlugin::OsdPlugin(Licq::GeneralPlugin::Params& p)
  : Licq::GeneralPlugin(p)
{
  // Empty
}

// when licq --help is called
string OsdPlugin::usage() const
{
    static const char name[] = "no options for this plugin. Configure via configfile";
    return name;
}

// plugin name as seen in the licq load plugins menupoint
string OsdPlugin::name() const
{
    static const char name[] = "OSD";
    return name;
}

// config file for this plugin
// used when you select configure in the licq plugin selector
string OsdPlugin::configFile() const
{
    static const char name[] = "licq_osd.conf";
    return name;
}

// displayed in plugin selector
string OsdPlugin::version() const
{
    static const char version[] = PLUGIN_VERSION_STRING;
    return version;
}

// status of plugin - so they can be deactivated
// not implemented for this plugin
bool OsdPlugin::isEnabled() const
{
  return Enabled;
}

// displayed in plugin selector
string OsdPlugin::description() const
{
    static const char desc[] = "OSD-text on new messages";
    return desc;
}

// a wrapper so we can log from my_xosd.cpp to standard licq log
void log(int mode, const char *message)
{
    if (mode==0) // warn/info
      gLog.warning("%s", message);
    if (mode==1) // error
      gLog.error("%s", message);
}

// issue a few warnings for wrong config values.
void verifyconfig(string pluginfont, unsigned long /* timeout */,
    unsigned long hoffset, unsigned long voffset, string vpos, string hpos,
    unsigned long lines, unsigned long linelen, unsigned long quiettimeout,
    string colour, bool /* wait */, unsigned long showmessages,
    unsigned long showlogon, unsigned long shadowoffset,
    unsigned long outlineoffset, string shadowcolour, string outlinecolour,
    string localencoding)
{
    try {
	if ((pluginfont=="") || (pluginfont.at(0)=='"') || (pluginfont.at(0)=='\''))
	    gLog.error("CONFIG: Invalid pluginfont %s. This will fail", pluginfont.c_str());
	if (hoffset>10000)
	    gLog.warning("CONFIG: Very high horizontal offset %lu. This might fail", hoffset);
	if (voffset>10000)
	    gLog.warning("CONFIG: Very high vertical offset %lu. This might fail", voffset);
	if ((vpos!="top") && (vpos!="bottom") && (vpos!="middle"))
            gLog.error("CONFIG: Invalid vertical position %s. Should be \"top\" or \"bottom\" or \"middle\". This will fail.", vpos.c_str());
	if ((hpos!="left") && (hpos!="right") && (hpos!="center"))
	    gLog.error("CONFIG: Invalid horizontal position %s. Should be \"left\" or \"right\" or \"center\". This will fail.", hpos.c_str());
	if (lines>50)
	    gLog.error("CONFIG: More than 50 lines not allowed. You used %lu", lines);
	if (linelen>500)
	    gLog.error("CONFIG: More than 500 characters per line not allowed. You used %lu", linelen);
	if (quiettimeout>500)
	    gLog.warning("CONFIG: Your quiettimeout %lu is higher than 500. Do you really want this?", quiettimeout);
	if (colour=="")
	    gLog.error("CONFIG: Invalid colour %s. For possible values look at rgb.txt from your Xfree86 distribution", colour.c_str());
	if (showmessages>4)
            gLog.error("CONFIG: Invalid value for showmessages %lu", showmessages);
	if (showlogon>2)
            gLog.error("CONFIG: Invalid value for showlogon %lu", showlogon);
	if (shadowoffset>200)
            gLog.warning("CONFIG: Very high Shadowoffset value %lu", shadowoffset);
	if (outlineoffset>200)
            gLog.warning("CONFIG: Very high Outlineoffset value %lu", outlineoffset);
	if (shadowcolour=="")
	    gLog.error("CONFIG: Invalid shadow colour %s. For possible values look at rgb.txt from your Xfree86 distribution", shadowcolour.c_str());
	if (outlinecolour=="")
	    gLog.error("CONFIG: Invalid outline colour %s. For possible values look at rgb.txt from your Xfree86 distribution", outlinecolour.c_str());
	if (localencoding=="")
	    gLog.warning("Localencoding could not be determined from your locale");
    }
    catch (...)
    {
	gLog.error("CONFIG: Exception while verifying config values");
    }
}

unsigned parseShowInModesStr(const char* ShowInModesStr)
{
  unsigned showInModes = 0;
    if (strstr(ShowInModesStr, "Online"))
    showInModes |= User::OnlineStatus;
    if (strstr(ShowInModesStr, "FreeForChat"))
    showInModes |= User::FreeForChatStatus;
    if (strstr(ShowInModesStr, "Away"))
    showInModes |= User::AwayStatus;
    if (strstr(ShowInModesStr, "NA"))
    showInModes |= User::NotAvailableStatus;
    if (strstr(ShowInModesStr, "Occupied"))
    showInModes |= User::OccupiedStatus;
    if (strstr(ShowInModesStr, "DND"))
    showInModes |= User::DoNotDisturbStatus;
    if (strstr(ShowInModesStr, "Invisible"))
    showInModes |= User::InvisibleStatus;
  return showInModes;
}

// called once on Load of the plugin
bool OsdPlugin::init(int /* argc */, char** /* argv */)
{
  string showInModes;
  string showMsgsInModes;
    try {
	Configured=false;
	gLog.info("OSD Plugin initializing\n");

    string filename = "licq_osd.conf";
    Licq::IniFile conf(filename);
    if (!conf.loadFile()) // no configfile found
    {
      filename = Licq::gDaemon.baseDir() + filename;
	    FILE *f = fopen(filename.c_str(), "w");
	    if (f) // create config file
	    {
		fprintf(f, "%s", OSD_CONF);
		fclose(f);
	    }
	    else // configfile cannot be created
	    {
		gLog.error("Configfile can not be created. Check the permissions on %s", filename.c_str());
		return 0;
	    }
      if (!conf.loadFile()) // configfile cannot be read after creating it - this should not happen
      {
		gLog.error("Configfile created but cannot be loaded. This should not happen");
		return 0;
      }
    }
    conf.setSection("Main");
    conf.get("Wait", config.osd_wait, WAIT);
    conf.get("Font", config.pluginfont, FONT);
    conf.get("Timeout", config.timeout, DISPLAYTIMEOUT);
    conf.get("HOffset", config.hoffset, HORIZONTAL_OFFSET);
    conf.get("VOffset", config.voffset, VERTICAL_OFFSET);
    conf.get("VPos", config.vpos, VPOS);
    conf.get("HPos", config.hpos, HPOS);
    conf.get("Lines", config.lines, LINES);
    conf.get("Linelen", config.linelen, LINELEN);
    conf.get("Quiettimeout", config.quiettimeout, QUIETTIMEOUT);
    conf.get("Colour", config.colour, COLOUR);
    conf.get("ControlColour", config.controlcolour, CONTROLCOLOUR);
    conf.get("Showmessages", config.Showmessages, SHOWMESSAGES);
    conf.get("ShowAutoResponseCheck", config.ShowAutoResponseCheck, SHOWAUTORESPONSECHECK);
    conf.get("Showlogon", config.Showlogon, SHOWLOGON);
    conf.get("DelayPerCharacter", config.DelayPerCharacter, DELAYPERCHARACTER);
    conf.get("ShowStatusChange", config.ShowStatusChange, SHOWSTATUSCHANGE);
    conf.get("ShadowOffset", config.shadowoffset, SHADOW_OFFSET);
    conf.get("OutlineOffset", config.outlineoffset, OUTLINE_OFFSET);
    conf.get("MarkSecureMessages", config.marksecuremessages, MARKSECUREMESSAGES);
    conf.get("ShadowColour", config.shadowcolour, SHADOW_COLOUR);
    conf.get("OutlineColour", config.outlinecolour, OUTLINE_COLOUR);
    conf.get("ShowInModes", showInModes, SHOWINMODESSTR);
    conf.get("ShowMsgsInModes", showMsgsInModes, SHOWMSGSINMODESSTR);

    config.showInModes = parseShowInModesStr(showInModes.c_str());
    config.showMsgsInModes = parseShowInModesStr(showMsgsInModes.c_str());

	setlocale(LC_ALL, "");

	config.localencoding=nl_langinfo(CODESET);
	bindtextdomain (PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(PACKAGE, config.localencoding.c_str());
	textdomain (PACKAGE);
	// check config values for validity
	verifyconfig(config.pluginfont, config.timeout, config.hoffset, config.voffset, config.vpos, config.hpos, config.lines, config.linelen, config.quiettimeout,
		     config.colour, config.osd_wait, config.Showmessages, config.Showlogon, config.shadowoffset, config.outlineoffset, config.shadowcolour,
		     config.outlinecolour, config.localencoding);

	return 1;
    }
    catch (...)
    {
	return 0;
    }
}


// run method of plugin
int OsdPlugin::run()
{
    // register plugin at the licq daemon
  int nPipe = getReadPipe();
  setSignalMask(Licq::PluginSignal::SignalUser |
      Licq::PluginSignal::SignalLogon | Licq::PluginSignal::SignalLogoff);
    bool Exit=false; // exit plugin?
    char buf[16];

    if (nPipe==-1)
    {
	gLog.warning("Invalid Pipe received");
	return 1;
    }

    disabletimer=time(0);
    Enabled=true;
    Online=false;
    // as long as no shutdown command from licq daemon received
    while (!Exit)
    {
	read(nPipe, buf, 1); // Information about a new signal is sent through pipe
	if (!Configured)
	{
	    if (!my_xosd_init(config.pluginfont, config.colour, config.hoffset, config.voffset, config.vpos, config.hpos, config.timeout, config.DelayPerCharacter, config.lines, config.linelen, config.osd_wait, config.shadowoffset, config.outlineoffset, config.shadowcolour, config.outlinecolour))
		return 0;
	    Configured=true;
	}

	switch (buf[0])
	{
      case Licq::GeneralPlugin::PipeSignal:
      {
		// read the actual signal from the daemon
        Licq::PluginSignal* s = popSignal();
		if (s)
		{
		    ProcessSignal(s);
		    delete s;
		    s=0;
		}
		break;
	    }

	    // An event is pending - skip it - shouldnt happen
	    // events are responses to some requests to the licq daemon
	    // like send a message - we never do such a thing
      case Licq::GeneralPlugin::PipeEvent:
      {
        gLog.warning("Event received - should not happen in this plugin");
        Licq::Event* e = popEvent();
        delete e;
        break;
      }
	    // shutdown command from daemon
	    // every plugin has to implement this command
      case Licq::GeneralPlugin::PipeShutdown:
      {
		Exit = true;
		gLog.info("OSD Plugin shutting down");
		break;
	    }

      case Licq::GeneralPlugin::PipeDisable:
	    Enabled=false;
	    gLog.info("OSD Plugin disabled");
	    break;
      case Licq::GeneralPlugin::PipeEnable:
	    Enabled=true;
	    gLog.info("OSD Plugin enabled");
	    break;
	default:
	    gLog.warning("Unknown message type %d", buf[0]);
	}
    }

    if (Configured)
    {
	my_xosd_exit();
        Configured=false;
    }

    return 0;
}

void OsdPlugin::destructor()
{
  delete this;
}

void ProcessSignal(Licq::PluginSignal* s)
{
    string username;
    bool notify=false;
    bool ignore=false;
  bool want_osd = true; // if we are in DND,... we maybe don't want OSD
    bool want_osd_msgs_only=false; // though we don't want OSD we want msgs
    bool secure=false;
  unsigned status = User::OnlineStatus;
  const Licq::UserEvent* e = NULL;

  switch (s->signal()) // signaltype
  {
    case Licq::PluginSignal::SignalUser:
    {
	    // FIX: we seem to get others logged on messages before
	    // our own one - so start quiettimeout in this case too
	    if (!Online)
	    {
		Online=true;
		disabletimer=time(0);
	    }
	    if (disabletimer) // no time() calls when timeout has been done
	    {
		if (time(0)-disabletimer>=(time_t)config.quiettimeout)
		    disabletimer=0;
		else
		    want_osd=false;
	    }

            if (want_osd)
	    {
        Licq::OwnerReadGuard o(LICQ_PPID);
        if (o.isLocked())
        {
          status = o->status();
		    //want_osd=true;

          if ((status & User::DoNotDisturbStatus) && (!(config.showInModes & User::DoNotDisturbStatus)))
			want_osd=false;
          else if ((status & User::OccupiedStatus) && (!(config.showInModes & User::OccupiedStatus)))
			want_osd=false;
          else if ((status & User::NotAvailableStatus) && (!(config.showInModes & User::NotAvailableStatus)))
			want_osd=false;
          else if ((status & User::AwayStatus) && (!(config.showInModes & User::AwayStatus)))
			want_osd=false;
          else if ((status & User::FreeForChatStatus) && (!(config.showInModes & User::FreeForChatStatus)))
			want_osd=false;
          else if ((status & User::InvisibleStatus) && (!(config.showInModes & User::InvisibleStatus)))
			want_osd=false;
          else if (!(config.showInModes & User::OnlineStatus))
            want_osd = false;

		    if (!want_osd) {
            if ((status & User::DoNotDisturbStatus) && (config.showMsgsInModes & User::DoNotDisturbStatus))
			    want_osd_msgs_only=true;
            else if ((status & User::OccupiedStatus) && (config.showMsgsInModes & User::OccupiedStatus))
			    want_osd_msgs_only=true;
            else if ((status & User::NotAvailableStatus) && (config.showMsgsInModes & User::NotAvailableStatus))
			    want_osd_msgs_only=true;
            else if ((status & User::AwayStatus) && (config.showMsgsInModes & User::AwayStatus))
			    want_osd_msgs_only=true;
            else if ((status & User::FreeForChatStatus) && (config.showMsgsInModes & User::FreeForChatStatus))
			    want_osd_msgs_only=true;
            else if ((status & User::InvisibleStatus) && (config.showMsgsInModes & User::InvisibleStatus))
			    want_osd_msgs_only=true;
            else if (config.showMsgsInModes & User::OnlineStatus)
              want_osd_msgs_only = true;

			if (want_osd_msgs_only) {
			    want_osd=true;
			}
		    }
        }
	    }

	    if (want_osd)
	    {
        Licq::UserReadGuard u(s->userId());
        if (u.isLocked())
        {
		    // user alias as displayed in licq -
		    // (if no alias known, ICQ number as string returned)
          username = u->getAlias();
		    notify=u->OnlineNotify();
		    ignore=u->InvisibleList() || u->IgnoreList();
          status = u->status();
            secure=u->Secure();

          username = my_translate(s->userId(), username, "UTF-8");

          if (s->subSignal() == Licq::PluginSignal::UserEvents && s->argument() > 0) // message
          {
			// get the user event whose ID we got as the signal argument
            e = u->EventPeekId(s->argument());

			if (e == NULL) // event not found
			{
              gLog.warning("Event for user %s not found", s->userId().toString().c_str());
                            want_osd=false;
			}
		    }
		}
		else
		{
          gLog.warning("User %s not found", s->userId().toString().c_str());
		    want_osd=false;
		}
	    }

	    if (!Enabled)
		want_osd=false;
	    if (ignore)
		want_osd=false;
      if (gUserManager.isOwner(s->userId())) // no messages for our own actions
                want_osd=false;

	    // user checked our auto-response
      // this is some evil functionality - most users don't know
	    // that you realize when they check your auto-response
	    // i implemented this just for fun :)
	    if ((want_osd) && (!want_osd_msgs_only) && // not ignored or disabled
          (s->subSignal() == Licq::PluginSignal::UserEvents) &&
          (s->argument() == 0) // auto response
          )
      {
		if (
		     (config.ShowAutoResponseCheck==1) ||  // display auto_reponse checks
		     ((config.ShowAutoResponseCheck==2)&&(notify))
		   )
		{
		    string msg(username);
		    msg+=_(" checked your auto-response");
		    my_xosd_display("autoresponse", msg, config.controlcolour);
		}
	    }

	    // messages that are not sent by myself
	    if ((want_osd) && // not ignored or disabled
          (s->subSignal() == Licq::PluginSignal::UserEvents) &&
          (s->argument() > 0) // message
	       )
	    {
		if (
		    (config.Showmessages==1) ||  // display messages on screen if configured in config file
		    ((config.Showmessages==2)&&(notify))
		   )
		{
		    string msg="";
		    if (secure && config.marksecuremessages)
			msg="(S) ";
          msg += my_translate(s->userId(), e->text(), "UTF-8");
		    my_xosd_display(username.c_str(), msg.c_str(), config.colour);
		}
		if (
		    (config.Showmessages==3) ||   // only display information about message on screen
		    ((config.Showmessages==4)&&(notify))
		   )
		{
		    string msg=_("Message from ");
		    msg += username;
		    my_xosd_display("", msg, config.colour);
		}
	    }
      if (want_osd && (!want_osd_msgs_only) && (s->subSignal() == Licq::PluginSignal::UserStatus)) // logon/logoff or status change
	    {
		string msg = username;
        if (s->argument()!=0) // logon/logoff
        {
                    if (
			(config.Showlogon==1) ||
			((notify)&&(config.Showlogon==2))
		       )
          {
            if (s->argument() > 0)
			    msg+=_(" logged on");
			else
			    msg+=_(" logged off");
//			my_xosd_settimeout(config.timeout);
			my_xosd_display("", msg, config.controlcolour);
		    }
		}
		else // status change
		{
                    if (
			(config.ShowStatusChange==1) ||
			((notify)&&(config.ShowStatusChange==2))
		       )
		    {
			string msg = username;
			msg+=_(" changed status to: ");
            if (status == User::OfflineStatus)
              msg += _("offline");
            else if (status & User::DoNotDisturbStatus)
			    msg+=_("do not disturb");
            else if (status & User::OccupiedStatus)
			    msg+=_("occupied");
            else if (status & User::NotAvailableStatus)
			    msg+=_("not available");
            else if (status & User::AwayStatus)
			    msg+=_("away");
            else if (status & User::FreeForChatStatus)
			    msg+=_("free for chat");
            else if (status & User::InvisibleStatus)
			    msg+=_("invisible");
            else if (status & User::IdleStatus)
              msg += _("idle");
            else
              msg += _("online");
//			my_xosd_settimeout(config.timeout);
			my_xosd_display("", msg, config.controlcolour);
		    }
		}

	    }
	}
	break;
    case Licq::PluginSignal::SignalLogoff:
	gLog.info("OSD Plugin received logoff");
	disabletimer=time(0);
	Online=false;
	break;
    case Licq::PluginSignal::SignalLogon:
	gLog.info("OSD Plugin received logon");
	disabletimer=time(0);
	Online=true;
	break;
	// we are not interested in those
    case Licq::PluginSignal::SignalAddedToServer:
    case Licq::PluginSignal::SignalPluginEvent:
      break;
    default: // shouldnt happen
      gLog.warning("Unknown signal %d", s->signal());
      break;
    }
}

#ifdef CP_TRANSLATE
// this function maps the licq character encoding names to the iconv ones.
// this is a mess and should be replaced
// FIXME !!!
const char *get_iconv_encoding_name(const char *licq_encoding_name)
{
    static const char iso8859_1[] = "ISO-8859-1";
    static const char iso8859_2[] = "ISO-8859-2";
    static const char iso8859_3[] = "ISO-8859-3";
    static const char iso8859_5[] = "ISO-8859-5";
    static const char iso8859_6[] = "ISO-8859-6";
    static const char iso8859_7[] = "ISO-8859-7";
    static const char iso8859_8_i[] = "ISO-8859-8";
    static const char iso8859_9[] = "ISO-8859-9";
    static const char iso8859_15[] = "ISO-8859-15";

    static const char cp_1250[] = "CP1250";
    static const char cp_1251[] = "CP1251";
    static const char cp_1252[] = "CP1252";
    static const char cp_1253[] = "CP1253";
    static const char cp_1254[] = "CP1254";
    static const char cp_1255[] = "CP1255";
    static const char cp_1256[] = "CP1256";
    static const char cp_1257[] = "CP1257";

    static const char gbk[] = "GBK";
    static const char big5[] = "BIG-5";
    static const char koi8_r[] = "KOI8R";
    static const char shift_jis[] = "SHIFT-JIS";
    static const char jis7[] = ""; // no idea
    static const char eucjp[] = "EUCJP";
    static const char euckr[] = "EUCKR";
    static const char tscii[] = "TSCII";
    static const char tis_620[] = "TIS620";
    static const char koi8_u[] = "KOI8U";
    static const char utf_8[] = "UTF-8";

    if (strcasecmp(licq_encoding_name, "ISO 8859-1")==0)
	return iso8859_1;
    if (strcasecmp(licq_encoding_name, "ISO 8859-2")==0)
	return iso8859_2;
    if (strcasecmp(licq_encoding_name, "ISO 8859-3")==0)
	return iso8859_3;
    if (strcasecmp(licq_encoding_name, "ISO 8859-5")==0)
	return iso8859_5;
    if (strcasecmp(licq_encoding_name, "ISO 8859-6")==0)
	return iso8859_6;
    if (strcasecmp(licq_encoding_name, "ISO 8859-7")==0)
	return iso8859_7;
    if (strcasecmp(licq_encoding_name, "ISO 8859-8-I")==0)
	return iso8859_8_i;
    if (strcasecmp(licq_encoding_name, "ISO 8859-9")==0)
	return iso8859_9;
    if (strcasecmp(licq_encoding_name, "ISO 8859-15")==0)
	return iso8859_15;

    if (strcasecmp(licq_encoding_name, "CP 1250")==0)
	return cp_1250;
    if (strcasecmp(licq_encoding_name, "CP 1251")==0)
	return cp_1251;
    if (strcasecmp(licq_encoding_name, "CP 1252")==0)
	return cp_1252;
    if (strcasecmp(licq_encoding_name, "CP 1253")==0)
	return cp_1253;
    if (strcasecmp(licq_encoding_name, "CP 1254")==0)
	return cp_1254;
    if (strcasecmp(licq_encoding_name, "CP 1255")==0)
	return cp_1255;
    if (strcasecmp(licq_encoding_name, "CP 1256")==0)
	return cp_1256;
    if (strcasecmp(licq_encoding_name, "CP 1257")==0)
	return cp_1257;

    if (strcasecmp(licq_encoding_name, "GBK")==0)
	return gbk;
    if (strcasecmp(licq_encoding_name, "BIG5")==0)
	return big5;
    if (strcasecmp(licq_encoding_name, "KOI8-R")==0)
	return koi8_r;
    if (strcasecmp(licq_encoding_name, "Shift-JIS")==0)
	return shift_jis;
    if (strcasecmp(licq_encoding_name, "JIS7")==0)
	return jis7;
    if (strcasecmp(licq_encoding_name, "eucJP")==0)
	return eucjp;
    if (strcasecmp(licq_encoding_name, "eucKR")==0)
	return euckr;
    if (strcasecmp(licq_encoding_name, "TSCII")==0)
	return tscii;
    if (strcasecmp(licq_encoding_name, "TIS-620")==0)
	return tis_620;
    if (strcasecmp(licq_encoding_name, "KOI8-U")==0)
	return koi8_u;
    if (strcasecmp(licq_encoding_name, "UTF-8")==0)
	return utf_8;
    return licq_encoding_name;
}


//translates incoming messages or user names to our codepage.
// this works only if it is convertable by iconv
// the codepage of the other user is determined by the UserEncoding property of
// the other user. (change it for example via the qt4-gui message window)
// Licq:PluginSignal is needed to get the User for this message -
// some day i will do this more elegant
string my_translate(const UserId& /* userId */, const string& msg, const char* userenc)
{
    iconv_t conv;
    size_t fromsize, tosize, ressize;
    const char *msgptr;
    char *resptr;

    if (config.localencoding == "") {
      gLog.warning("Didn't get our local encoding");
    return msg;
  }

    if ((userenc == 0) || (*userenc == 0))
	{
          gLog.info("No translation needs to be done");
    return msg;
  }
  conv = iconv_open((config.localencoding + "//IGNORE").c_str(), get_iconv_encoding_name(userenc));

    // no translation possible?
    if (conv==(iconv_t)-1)
    {
      gLog.warning("Error initializing iconv");
    return msg;
  }

  fromsize = msg.size();
    tosize=fromsize;
    ressize=tosize;
  msgptr = msg.c_str();

  char* result = (char*)malloc(msg.size() + 1);
    resptr=result;

    while ((fromsize>0) && (tosize>0))
    {
	if ((int)iconv(conv, (ICONV_CONST char **)&msgptr, &fromsize, &resptr, &tosize)==-1)
	{
	    // array is not enough
	    if (errno == E2BIG)
	    {
		// add fromsize + 4 more characters to array
		result = (char*) realloc(result,ressize + fromsize + 4);
		resptr = result + ressize;
		ressize += fromsize + 4;
		tosize += fromsize + 4;
		continue;
	    }
	    gLog.warning("Error in my_translate - stopping translation, error on %ld. char",
                         (long int)(msgptr - msg.c_str() + 1));
      free(result);
      return msg;
    }
  }
    *resptr = 0;
    iconv_close(conv);

  string ret(result);
  free(result);
  return ret;
}
#else
// a dummy function which helps me to remove the #ifdef CP_TRANSLATEs in a lot of my code
string my_translate(const UserId& /* userId */, const string& msg, Licq::PluginSignal* /* s */)
{
    return strdup(msg.c_str());
}

#endif
