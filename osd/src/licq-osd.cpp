// This file is released under the GPL
// author: Martin Maurer (martinmaurer@gmx.at)
// first version 7 Apr 2003

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

#include <licq_plugin.h>
#include <licq_icqd.h>
#include <licq_events.h>
#include <licq_file.h>
#include <licq_log.h>

#include "config.h"
#include "my_xosd.h"
#include "licq_osd.conf.h"

using namespace std;

//#if CVSLICQ==1
//#warning compiling for licq>=1.2.8
//#else
//#undef CVSLICQ
//#warning compiling for licq<1.2.8
//#endif

#define _(String) gettext (String)

// if you don't want to use codepage translation, comment out this
#define CP_TRANSLATE

// this string is prepended to all the OSD debug  messages
#define L_OSD_STR "[OSD] "

// licq does not define an invisible status constant
// so I chose one which won't be taken most likely
// by licq sources: include/licq_icq.h
#define OSD_STATUS_INVISIBLE 0x8000
// licq defines ICQ_STATUS_ONLINE as 0, that's not andable so we use our own
#define OSD_STATUS_ONLINE 0x4000

struct Config {
    unsigned long Showmessages; //=SHOWLOGON;
    unsigned long Showlogon; //=SHOWLOGON;
    unsigned long ShowStatusChange; //=SHOWSTATUSCHANGE;
    unsigned long ShowAutoResponseCheck; //=SHOWAUTORESPONSECHECK;
    unsigned long quiettimeout; //=QUIETTIMEOUT;
    string pluginfont;
    unsigned long ShowInModes;
    unsigned long ShowMsgsInModes;
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
void ProcessSignal(LicqSignal* s);
void ProcessEvent(ICQEvent *e);
#ifdef CP_TRANSLATE
    const char *get_iconv_encoding_name(const char *licq_encoding_name);
char* my_translate(const UserId& userId, const char* msg, const char* userenc);
#endif

// some variables representing the internal state
time_t disabletimer=0;
bool Online;
bool Enabled;
bool Configured=false; // is the xosd display initialized?

using namespace std;

const char *LP_Usage(void) // when licq --help is called
{
    static const char name[] = "no options for this plugin. Configure via configfile";
    return name;
}

const char *LP_Name(void) // plugin name as seen in the licq load plugins menupoint
{
    static const char name[] = "OSD Plugin";
    return name;
}

// config file for this plugin
// used when you select configure in the licq plugin selector
const char *LP_ConfigFile(void)
{
    static const char name[] = "licq_osd.conf";
    return name;
}

// displayed in plugin selector
const char *LP_Version(void)
{
    static const char version[] = VERSION;
    return version;
}

// status of plugin - so they can be deactivated
// not implemented for this plugin
const char *LP_Status(void)
{
    static const char status_running[] = "running";
    static const char status_disabled[] = "disabled";
    if (Enabled)
	return status_running;
    return status_disabled;
}

// displayed in plugin selector
const char *LP_Description(void)
{
    static const char desc[] = "OSD-text on new messages";
    return desc;
}

// a wrapper so we can log from my_xosd.cpp to standard licq log
void log(int mode, const char *message)
{
    if (mode==0) // warn/info
	gLog.Warn("%s%s\n", L_OSD_STR, message);
    if (mode==1) // error
	gLog.Error("%s%s\n", L_ERRORxSTR, message);
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
	    gLog.Error("%sCONFIG: Invalid pluginfont %s. This will fail\n", L_ERRORxSTR, pluginfont.c_str());
	if (hoffset>10000)
	    gLog.Warn("%sCONFIG: Very high horizontal offset %lu. This might fail\n", L_OSD_STR, hoffset);
	if (voffset>10000)
	    gLog.Warn("%sCONFIG: Very high vertical offset %lu. This might fail\n", L_OSD_STR, voffset);
	if ((vpos!="top") && (vpos!="bottom") && (vpos!="middle"))
            gLog.Error("%sCONFIG: Invalid vertical position %s. Should be \"top\" or \"bottom\" or \"middle\". This will fail.\n", L_ERRORxSTR, vpos.c_str());
	if ((hpos!="left") && (hpos!="right") && (hpos!="center"))
	    gLog.Error("%sCONFIG: Invalid horizontal position %s. Should be \"left\" or \"right\" or \"center\". This will fail.\n", L_ERRORxSTR, hpos.c_str());
	if (lines>50)
	    gLog.Error("%sCONFIG: More than 50 lines not allowed. You used %lu\n", L_ERRORxSTR, lines);
	if (linelen>500)
	    gLog.Error("%sCONFIG: More than 500 characters per line not allowed. You used %lu\n", L_ERRORxSTR, linelen);
	if (quiettimeout>500)
	    gLog.Warn("%sCONFIG: Your quiettimeout %lu is higher than 500. Do you really want this ?\n", L_OSD_STR, quiettimeout);
	if (colour=="")
	    gLog.Error("%sCONFIG: Invalid colour %s. For possible values look at rgb.txt from your Xfree86 distribution\n", L_ERRORxSTR, colour.c_str());
	if (showmessages>4)
            gLog.Error("%sCONFIG: Invalid value for showmessages %lu\n", L_ERRORxSTR, showmessages);
	if (showlogon>2)
            gLog.Error("%sCONFIG: Invalid value for showlogon %lu\n", L_ERRORxSTR, showlogon);
	if (shadowoffset>200)
            gLog.Warn("%sCONFIG: Very high Shadowoffset value %lu\n", L_OSD_STR, shadowoffset);
	if (outlineoffset>200)
            gLog.Warn("%sCONFIG: Very high Outlineoffset value %lu\n", L_OSD_STR, outlineoffset);
	if (shadowcolour=="")
	    gLog.Error("%sCONFIG: Invalid shadow colour %s. For possible values look at rgb.txt from your Xfree86 distribution\n", L_ERRORxSTR, shadowcolour.c_str());
	if (outlinecolour=="")
	    gLog.Error("%sCONFIG: Invalid outline colour %s. For possible values look at rgb.txt from your Xfree86 distribution\n", L_ERRORxSTR, outlinecolour.c_str());
	if (localencoding=="")
	    gLog.Warn("%sLocalencoding could not be determined from your locale\n", L_OSD_STR);
    }
    catch (...)
    {
	gLog.Error("%sCONFIG: Exception while verifying config values", L_OSD_STR);
    }
}


unsigned long parseShowInModesStr(char *ShowInModesStr)
{
    unsigned long ShowInModes=0;
    if (strstr(ShowInModesStr, "Online"))
	ShowInModes|=OSD_STATUS_ONLINE;
    if (strstr(ShowInModesStr, "FreeForChat"))
	ShowInModes|=ICQ_STATUS_FREEFORCHAT;
    if (strstr(ShowInModesStr, "Away"))
	ShowInModes|=ICQ_STATUS_AWAY;
    if (strstr(ShowInModesStr, "NA"))
	ShowInModes|=ICQ_STATUS_NA;
    if (strstr(ShowInModesStr, "Occupied"))
	ShowInModes|=ICQ_STATUS_OCCUPIED;
    if (strstr(ShowInModesStr, "DND"))
	ShowInModes|=ICQ_STATUS_DND;
    if (strstr(ShowInModesStr, "Invisible"))
	ShowInModes|=OSD_STATUS_INVISIBLE;
    return ShowInModes;
}

// called once on Load of the plugin
bool LP_Init(int /* argc */, char** /* argv */)
{
    char ShowInModesStr[MAX_LINE_LEN+1];
    char ShowMsgsInModesStr[MAX_LINE_LEN+1];
    char temp[MAX_LINE_LEN+1];
    string filename;
    try {
	Configured=false;
	gLog.Info("%sOSD Plugin initializing\n", L_OSD_STR);

	filename=BASE_DIR;
	filename+="/licq_osd.conf";
	CIniFile conf;
	if (!conf.LoadFile(filename.c_str())) // no configfile found
	{
	    FILE *f = fopen(filename.c_str(), "w");
	    if (f) // create config file
	    {
		fprintf(f, "%s", OSD_CONF);
		fclose(f);
	    }
	    else // configfile cannot be created
	    {
		gLog.Error("%sConfigfile can not be created. Check the permissions on %s\n", L_ERRORxSTR, filename.c_str());
		return 0;
	    }
	    if (!conf.LoadFile(filename.c_str())) // configfile cannot be read after creating it - this should not happen
	    {
		gLog.Error("%sConfigfile created but cannot be loaded. This should not happen.\n", L_ERRORxSTR);
		return 0;
	    }
	}
	conf.SetSection("Main");
	conf.ReadBool("Wait", config.osd_wait, WAIT);

	conf.ReadStr("Font", temp, FONT);
	temp[MAX_LINE_LEN]=0;
        config.pluginfont=temp;

	conf.ReadNum("Timeout", config.timeout, DISPLAYTIMEOUT);
	conf.ReadNum("HOffset", config.hoffset, HORIZONTAL_OFFSET);
	conf.ReadNum("VOffset", config.voffset, VERTICAL_OFFSET);

	conf.ReadStr("VPos", temp, VPOS);
	temp[MAX_LINE_LEN]=0;
        config.vpos=temp;

	conf.ReadStr("HPos", temp, HPOS);
	temp[MAX_LINE_LEN]=0;
        config.hpos=temp;

	conf.ReadNum("Lines", config.lines, LINES);
	conf.ReadNum("Linelen", config.linelen, LINELEN);
	conf.ReadNum("Quiettimeout", config.quiettimeout, QUIETTIMEOUT);

	conf.ReadStr("Colour", temp, COLOUR);
	temp[MAX_LINE_LEN]=0;
        config.colour=temp;
	conf.ReadStr("ControlColour", temp, CONTROLCOLOUR);
	temp[MAX_LINE_LEN]=0;
        config.controlcolour=temp;

	conf.ReadNum("Showmessages", config.Showmessages, SHOWMESSAGES);
	conf.ReadNum("ShowAutoResponseCheck", config.ShowAutoResponseCheck, SHOWAUTORESPONSECHECK);
	conf.ReadNum("Showlogon", config.Showlogon, SHOWLOGON);
	conf.ReadNum("DelayPerCharacter", config.DelayPerCharacter, DELAYPERCHARACTER);
	conf.ReadNum("ShowStatusChange", config.ShowStatusChange, SHOWSTATUSCHANGE);
	conf.ReadNum("ShadowOffset", config.shadowoffset, SHADOW_OFFSET);
	conf.ReadNum("OutlineOffset", config.outlineoffset, OUTLINE_OFFSET);

	conf.ReadBool("MarkSecureMessages", config.marksecuremessages, MARKSECUREMESSAGES);

	conf.ReadStr("ShadowColour", temp, SHADOW_COLOUR);
	temp[MAX_LINE_LEN]=0;
        config.shadowcolour=temp;

	conf.ReadStr("OutlineColour", temp, OUTLINE_COLOUR);
	temp[MAX_LINE_LEN]=0;
        config.outlinecolour=temp;

	conf.ReadStr("ShowInModes", ShowInModesStr, SHOWINMODESSTR);
        ShowInModesStr[MAX_LINE_LEN]=0;
	conf.ReadStr("ShowMsgsInModes", ShowMsgsInModesStr, SHOWMSGSINMODESSTR);
        ShowMsgsInModesStr[MAX_LINE_LEN]=0;

	conf.CloseFile();

	config.ShowInModes=parseShowInModesStr(ShowInModesStr);
	config.ShowMsgsInModes=parseShowInModesStr(ShowMsgsInModesStr);

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
int LP_Main(CICQDaemon *_licqDaemon)
{
    // register plugin at the licq daemon
    int nPipe = _licqDaemon->RegisterPlugin(SIGNAL_UPDATExUSER | SIGNAL_LOGON| SIGNAL_LOGOFF);
    bool Exit=false; // exit plugin?
    char buf[16];

    if (nPipe==-1)
    {
	gLog.Warn("%sInvalid Pipe received\n", L_ERRORxSTR);
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
	case 'S':  // A signal is pending
	    {
		// read the actual signal from the daemon
        LicqSignal* s = _licqDaemon->popPluginSignal();
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
	case 'E':
	    {
		gLog.Warn("%sEvent received - should not happen in this plugin\n", L_WARNxSTR);
		ICQEvent *e = _licqDaemon->PopPluginEvent();
		if (e)
		{
		    delete e;
		    e=0;
		}
		break;
	    }
	    // shutdown command from daemon
	    // every plugin has to implement this command
	case 'X':  // Shutdown = plugin unloaded or licq shutting down
	    {
		Exit = true;
		gLog.Info("%sOSD Plugin shutting down\n", L_OSD_STR);
		break;
	    }

	case '0': // disable plugin (see plugin selector window)
	    Enabled=false;
	    gLog.Info("%sOSD Plugin disabled\n", L_OSD_STR);
	    break;
	case '1': // enable plugin (see plugin selector window)
	    Enabled=true;
	    gLog.Info("%sOSD Plugin enabled\n", L_OSD_STR);
	    break;
	default:
	    gLog.Warn("%sUnknown message type %d\n", L_WARNxSTR, buf[0]);
	    //            cout << "Unknown message type !!! " << endl;
	}
    }

    if (Configured)
    {
	my_xosd_exit();
        Configured=false;
    }
    // unregister the plugin
    _licqDaemon->UnregisterPlugin();

    return 0;
}


void ProcessSignal(LicqSignal* s)
{
    string username;
    bool notify=false;
    bool ignore=false;
    bool invisible=false;
  bool want_osd = true; // if we are in DND,... we maybe don't want OSD
    bool want_osd_msgs_only=false; // though we don't want OSD we want msgs
    bool secure=false;
    unsigned long status=0;
  const char* userencoding = NULL;
  const CUserEvent* e = NULL;

    switch (s->Signal()) // signaltype
    {
    case SIGNAL_UPDATExUSER:
	{

	    ICQUser *u;
	    ICQOwner *o;

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
        o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
        if (o != NULL)
        {
		    status=o->Status();
		    //want_osd=true;

		    if ((status&ICQ_STATUS_DND) && (!(config.ShowInModes&ICQ_STATUS_DND)))
			want_osd=false;
		    else if ((status&ICQ_STATUS_OCCUPIED) && (!(config.ShowInModes&ICQ_STATUS_OCCUPIED)))
			want_osd=false;
		    else if ((status&ICQ_STATUS_NA) && (!(config.ShowInModes&ICQ_STATUS_NA)))
			want_osd=false;
		    else if ((status&ICQ_STATUS_AWAY) && (!(config.ShowInModes&ICQ_STATUS_AWAY)))
			want_osd=false;
		    else if ((status==0) && (!(config.ShowInModes&OSD_STATUS_ONLINE)))
			want_osd=false;
		    else if ((status&ICQ_STATUS_FREEFORCHAT) && (!(config.ShowInModes&ICQ_STATUS_FREEFORCHAT)))
			want_osd=false;
		    else if ((o->StatusInvisible()) && (!(config.ShowInModes&OSD_STATUS_INVISIBLE))) // is reached when a user gets invisible
			want_osd=false;

		    if (!want_osd) {
			if ((status&ICQ_STATUS_DND) && (config.ShowMsgsInModes&ICQ_STATUS_DND))
			    want_osd_msgs_only=true;
			else if ((status&ICQ_STATUS_OCCUPIED) && (config.ShowMsgsInModes&ICQ_STATUS_OCCUPIED))
			    want_osd_msgs_only=true;
			else if ((status&ICQ_STATUS_NA) && (config.ShowMsgsInModes&ICQ_STATUS_NA))
			    want_osd_msgs_only=true;
			else if ((status&ICQ_STATUS_AWAY) && (config.ShowMsgsInModes&ICQ_STATUS_AWAY))
			    want_osd_msgs_only=true;
			else if ((status==0) && (config.ShowMsgsInModes&OSD_STATUS_ONLINE))
			    want_osd_msgs_only=true;
			else if ((status&ICQ_STATUS_FREEFORCHAT) && (config.ShowMsgsInModes&ICQ_STATUS_FREEFORCHAT))
			    want_osd_msgs_only=true;
			else if ((o->StatusInvisible()) && (config.ShowMsgsInModes&OSD_STATUS_INVISIBLE)) // is reached when a user gets invisible
			    want_osd_msgs_only=true;

			if (want_osd_msgs_only) {
			    want_osd=true;
			}
		    }

          gUserManager.DropOwner(o);
        }
	    }

	    if (want_osd)
	    {
        u = gUserManager.fetchUser(s->userId(), LOCK_R);
		if (u)
		{
		    // user alias as displayed in licq -
		    // (if no alias known, ICQ number as string returned)
		    username=u->GetAlias();
		    notify=u->OnlineNotify();
		    ignore=u->InvisibleList() || u->IgnoreList();
		    status=u->Status();
		    invisible=u->StatusInvisible();
			userencoding=u->UserEncoding(); // needed in translate function
            secure=u->Secure();

          char* username_translated_temp = my_translate(s->userId(), username.c_str(), "UTF-8");
		    username = username_translated_temp;
		    free(username_translated_temp);

                    if (
			(s->SubSignal() == USER_EVENTS) &&
			(s->Argument() > 0) // message
		       )
		    {
			// get the user event whose ID we got as the signal argument
			e = u->EventPeekId(s->Argument());

			if (e == NULL) // event not found
			{
              gLog.Warn("%sEvent for user %s not found\n", L_WARNxSTR, USERID_TOSTR(s->userId()));
                            want_osd=false;
			}
		    }

		    // free user object as we no longer need it
		    gUserManager.DropUser(u);
		}
		else
		{
          gLog.Warn("%sUser %s not found\n", L_WARNxSTR, USERID_TOSTR(s->userId()));
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
		(s->SubSignal() == USER_EVENTS) &&
		(s->Argument() == 0) // auto response
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
		(s->SubSignal() == USER_EVENTS) &&
		(s->Argument() > 0) // message
	       )
	    {
		if (
		    (config.Showmessages==1) ||  // display messages on screen if configured in config file
		    ((config.Showmessages==2)&&(notify))
		   )
		{
          char* translated = my_translate(s->userId(), e->Text(), userencoding);
		    string msg="";
		    if (secure && config.marksecuremessages)
			msg="(S) ";
		    msg+=translated;
		    my_xosd_display(username.c_str(), msg.c_str(), config.colour);
		    free(translated);
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
	    if (want_osd && (!want_osd_msgs_only) && (s->SubSignal() == USER_STATUS)) // logon/logoff or status change
	    {
		string msg = username;
		if (s->Argument()!=0) // logon/logoff
		{
                    if (
			(config.Showlogon==1) ||
			((notify)&&(config.Showlogon==2))
		       )
		    {
			if (s->Argument()>0)
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
			if (status&ICQ_STATUS_DND)
			    msg+=_("do not disturb");
			else if (status&ICQ_STATUS_OCCUPIED)
			    msg+=_("occupied");
			else if (status&ICQ_STATUS_NA)
			    msg+=_("not available");
			else if (status&ICQ_STATUS_AWAY)
			    msg+=_("away");
			else if (status==ICQ_STATUS_ONLINE)
			    msg+=_("online");
			else if (status&ICQ_STATUS_FREEFORCHAT)
			    msg+=_("free for chat");
			else if (invisible)
			    msg+=_("invisible");
			else if (status==ICQ_STATUS_OFFLINE)
			    msg+=_("offline");
			else // shouldnt be reached
			    msg+=_("unknown");
//			my_xosd_settimeout(config.timeout);
			my_xosd_display("", msg, config.controlcolour);
		    }
		}

	    }
	}
	break;
    case SIGNAL_LOGOFF:
	gLog.Info("%sOSD Plugin received logoff\n", L_OSD_STR);
	disabletimer=time(0);
	Online=false;
	break;
    case SIGNAL_LOGON:
	gLog.Info("%sOSD Plugin received logon\n", L_OSD_STR);
	disabletimer=time(0);
	Online=true;
	break;
	// we are not interested in those
    case SIGNAL_ADDxSERVERxLIST:
    case SIGNAL_UI_MESSAGE:
	break;
    case SIGNAL_UPDATExLIST:
    default: // shouldnt happen
	gLog.Warn("%sUnknown signal %ld\n", L_WARNxSTR, s->Signal());
	//        cout << "Unknown signal" << s->Signal() << endl;
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
// the other user. (change it for example via the licq-qt-gui message window)
// LicqSignal is needed to get the User for this message -
// some day i will do this more elegant
char* my_translate(const UserId& /* userId */, const char* msg, const char* userenc)
{
    // will be deleted outside of this function
    char *result = (char*)malloc(strlen(msg) + 1);

    iconv_t conv;
    size_t fromsize, tosize, ressize;
    const char *msgptr;
    char *resptr;

    if (config.localencoding == "") {
		gLog.Warn("%sDidn't get our local encoding\n", L_OSD_STR);
	strcpy(result,msg);
	return result;
    }

    if ((userenc == 0) || (*userenc == 0))
	{
		strcpy(result, msg);
		gLog.Info("%sNo translation needs to be done\n", L_OSD_STR);
        return result;
    }
    conv=iconv_open(config.localencoding.c_str(), get_iconv_encoding_name(userenc));

    // no translation possible?
    if (conv==(iconv_t)-1)
    {
	gLog.Warn("%sError initializing iconv\n", L_OSD_STR);
	strcpy(result, msg); // return original string
	return result;
    }

    fromsize=strlen(msg);
    tosize=fromsize;
    ressize=tosize;
    msgptr=msg;
    resptr=result;

    while ((fromsize>0) && (tosize>0))
    {
	if ((int)iconv(conv, (char **)&msgptr, &fromsize, &resptr, &tosize)==-1)
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
      gLog.Warn("%sError in my_translate - stopping translation, error on %ld. char\n",
          L_OSD_STR, (long int)(msgptr-msg+1));
	    strcpy(result, msg); // return original string
	    return result;
	}
    }
    *resptr = 0;
    iconv_close(conv);
    return result;
}
#else
// a dummy function which helps me to remove the #ifdef CP_TRANSLATEs in a lot of my code
char* my_translate(const UserId& /* userId */, const char* msg, LicqSignal* s)
{
    return strdup(msg);
}

#endif
