// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998 - 2009 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

/*
 * FIFO commands
 *
 * TODO things
 *  o file transfers
 * THOUGHTS 
 *  o a flag so message, url, etc can been forced to fail if buddy is not
 *  | in the list ?
 *
 * If anyone changes anything here try to update the README.FIFO
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "time-fix.h"

#ifdef HAVE_ERRNO_H
#include <errno.h>
#else
extern int errno;
#endif

// Localization
#include "gettext.h"

#include "licq_icq.h"
#include "licq_user.h"
#include "licq_constants.h"
#include "licq_log.h"
#include "licq_translate.h"
#include "licq_packets.h"
#include "licq_plugind.h"
#include "licq.h"
#include "support.h"

#include "licq_icqd.h"

using std::string;

#define ReportMissingParams(cmdname) \
  (gLog.Info("%s `%s': missing arguments. try `help %s'\n",  \
  L_FIFOxSTR,cmdname,cmdname))

#define ReportBadBuddy(cmdname,szUin) \
  (gLog.Info("%s `%s': bad buddy string `%s'\n",L_FIFOxSTR,cmdname,szUin))

static const char* const HELP_STATUS = tr(
        "\tstatus <[*]<status>> <auto response>\n"
        "\t\tstatus: online, offline, na, away, occupied, dnd, ffc\n\n"
        "\t\tSets the status of the current Licq session\n"
        "\t\tto that given (precede the status by a\n"
        "\t\t\"*\" for invisible mode)\n");
static const char* const HELP_AUTO = tr(
        "\tauto_response <auto response>\n"
        "\t\tSets the auto response message without\n"
        "\t\tchanging the current status.\n");
static const char* const HELP_MSG = tr(
        "\tmessage <buddy> <message>\n"
        "\t\tSend a message to the given buddy.\n");
static const char* const HELP_URL = tr(
        "\turl <buddy> <url> [<description>]\n"
        "\t\tSend a url to the given buddy.\n");
static const char* const HELP_SMS = tr(
        "\tsms <buddy> <message>\n"
        "\tSend a SMS to the given buddy.\n");
static const char* const HELP_SMS_NUMBER = tr(
        "\tsms <number> <message>\n"
        "\tSend a SMS to the given cellular number.\n");
static const char* const HELP_REDIRECT = tr(
        "\tredirect <file>\n"
        "\t\tRedirects stderr for\n"
        "\t\tLicq to the given file.\n");
static const char* const HELP_DEBUGLVL = tr(
        "\tdebuglvl <level>\n"
        "\t\tSet what information is logged.\n"
        "\t\tSee <level> in licq -h\n");
static const char* const HELP_ADDUSER = tr(
        "\tadduser <uin>\n"
        "\t\tAdd user with <uin> to your contact list.\n");
static const char* const HELP_USERINFO = tr(
        "\tuserinfo <buddy>\n"
        "\t\tUpdates a buddy's user information.\n");
static const char* const HELP_EXIT = tr(
        "\texit\n"
        "\t\tCauses the Licq session to shutdown.\n");
static const char* const HELP_UIVIEWEVENT = tr(
        "\tui_viewevent [<buddy>]\n"
        "\t\tShows the oldest pending event.\n");
static const char* const HELP_UIMESSAGE = tr(
        "\tui_message <buddy>\n"
        "\t\tOpen the plugin message composer to <buddy>\n");
static const char* const HELP_PLUGINLIST = tr(
		"\tlist_plugins\n"
		"\t\tLists the loaded UI plugins\n");
static const char* const HELP_PLUGINLOAD = tr(
		"\tload_plugin <plugin>\n"
		"\t\tLoads the UI plugin called <plugin>\n");
static const char* const HELP_PLUGINUNLOAD = tr(
		"\tunload_plugin <plugin>\n"
		"\t\tUnloads the UI plugin called <plugin>.\n"
		"\t\tUse list_plugins to see currently loaded UI plugins.\n");
static const char* const HELP_PROTOPLUGINLIST = tr(
		"\tlist_proto_plugins\n"
		"\t\tLists the loaded protocol plugins\n");
static const char* const HELP_PROTOPLUGINLOAD = tr(
		"\tload_proto_plugin <protoplugin>\n"
		"\t\tLoads the protocol plugin called <protoplugin>\n");
static const char* const HELP_PROTOPLUGINUNLOAD = tr(
		"\tunload_proto_plugin <protoplugin>\n"
		"\t\tUnloads the protocol plugin called <protoplugin>.\n"
		"\t\tUse list_proto_plugins to see currently loaded protocol plugins.\n");
static const char* const HELP_HELP = tr(
        "\thelp <<command> | all>\n" 
        "\t\tPrint help information for <command> or for all commands.\n");
#define MAX_ARGV 64

enum 
{
  CL_UNKNOWN=-1,
  CL_NONE=-2
};

struct command_t
{
  const char *const szName;
  int (*fnc)(int, const char *const*, void *);
  const char *const szHelp;
  int bHelp;
};

static int process_tok(const command_t *table,const char *tok);

unsigned long StringToStatus(const char* _szStatus)
{
  const ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
  unsigned long nStatus = 0;
  int i =0;
  static struct 
  {
    const char *const name;
    const unsigned long nStatus;
  } table[]=
  {
    { "online",   ICQ_STATUS_ONLINE      },
    { "away",     ICQ_STATUS_AWAY        },
    { "na",       ICQ_STATUS_NA          },
    { "occupied", ICQ_STATUS_OCCUPIED    },
    { "dnd",      ICQ_STATUS_DND         },
    { "ffc",      ICQ_STATUS_FREEFORCHAT },
    { "offline",  ICQ_STATUS_OFFLINE     },
    { NULL,       0                      }
  };
  gUserManager.DropOwner(o);
  if (_szStatus[0] == '*')
  {
    _szStatus++;
    nStatus |= ICQ_STATUS_FxPRIVATE;
  }
  for( i=0; table[i].name && strcasecmp(table[i].name,_szStatus)  ; i++)
    ;

  return table[i].name ? nStatus|table[i].nStatus : INT_MAX ;
}


static bool buffer_is_uin(const char *buffer)
{
  unsigned len = 0;
  
  for( ; buffer && isdigit(*buffer) ; buffer++ , len++ )
    ;

  return (len>0 && len <= MAX_UIN_DIGITS) && *buffer=='\0';
}

static bool buffer_get_ids(CICQDaemon *d, char *buffer, 
                           char **szId, unsigned long *nPPID, 
                           bool *missing_protocol)
{
  bool found = false;
  char *write = buffer;

  *missing_protocol = true;
  *szId = write;
  for( ; *buffer && !found ; buffer++ )
  {
    if( *buffer == '@' ) 
    {
      if( *(buffer+1)== '@' ) // @@ -> @
      {
        buffer++;
        *write = '@';
        write++;
      }
      else
        found = true;
    }
    else
    {
      *write = *buffer;
      write++;
    }
  }
  *write = 0;
  
  if( found )
  { 
    ProtoPluginsList l;
    ProtoPluginsListIter it;

    found = false;
    *missing_protocol = false;
    
    d->ProtoPluginList(l);
    for( it = l.begin() ; !found && it != l.end() ; ++it)
    {
      if( !strcmp( (*it)->Name(), buffer) )
      {
        *nPPID = (*it)->PPID();
        found = true;
      }
    }
  }

  return found;
}

/*! \brief Given an ascii string gets the szId and the nPPID
 *
 *  -# If all chars are digits then:
 *    -# assume it is a uin, and if bList flag is on, then check if it 
 *       is in the list
 *  -# If that fail try with alias Params.
 * 
 *  \param  buff     string to convert
 *  \param  bOnList  fail if #buff is not in the list
 *  \param  szId     address where the szId is returned
 *  \param  nPPID    address where the nPPID is returned
 *
 * \returns true on success
 */
static bool atoid( const char *buff, bool bOnList, 
                   char **szId, unsigned long *nPPID,
                   CICQDaemon *daemon)
{ 
  char *_szId = 0;
  unsigned long _nPPID = 0;
  bool ret = false, missing_protocol=true;
  char *s = 0;
  
  if (buff == NULL) 
    ret = false; 
  else if( (s=strdup(buff)) == 0 )
    ret  = false;
  else if ( buffer_is_uin(buff) && 
           ((bOnList && gUserManager.IsOnList(buff, LICQ_PPID)) || !bOnList  ))
  {
    _nPPID = LICQ_PPID;
    _szId = s;
    ret = true;
  }
  else if( buffer_get_ids(daemon, s, &_szId, &_nPPID, &missing_protocol) )
  {
    ret = false;

    FOR_EACH_PROTO_USER_START(_nPPID, LOCK_R)
    {
      if( strcasecmp(_szId, pUser->GetAlias()) == 0)
      {
        _szId = strdup(pUser->IdString());
        ret = true;
        FOR_EACH_PROTO_USER_BREAK
      }
    }
    FOR_EACH_PROTO_USER_END
  }
  else if( missing_protocol )
  {  
     /* assume ICQ */
    _nPPID = LICQ_PPID;
    
    FOR_EACH_PROTO_USER_START(_nPPID, LOCK_R)
    {
        if( strcasecmp(s, pUser->GetAlias()) == 0)
        {
        _szId = strdup(pUser->IdString());
        ret = true;
          FOR_EACH_PROTO_USER_BREAK
        }
    }
    FOR_EACH_PROTO_USER_END
    free(s);
    s = 0;
  }
  else
  {
    free(s);
    s = 0;
    ret = false;
  }

  if( ret )
  {
    if( szId )
      *szId = _szId;
    if( nPPID )
      *nPPID = _nPPID;
  }
  
  return ret;
}

// status 
static int fifo_status( int argc, const char *const *argv, void *data)
{
  CICQDaemon *d= (CICQDaemon *) data;
  const char *szStatus = argv[1];
  unsigned long nStatus;

  if( argc == 1 )
  {
    ReportMissingParams(argv[0]);
    return -1;
  }

  // Determine the status to go to
  nStatus = StringToStatus(const_cast<char *>(szStatus));

  if (nStatus == INT_MAX)
  {
    gLog.Warn(tr("%s%s %s: command with invalid status \"%s\".\n"),
              L_WARNxSTR,L_FIFOxSTR,argv[0],szStatus);
    return -1;
  }

  d->protoSetStatus(gUserManager.ownerUserId(LICQ_PPID), nStatus);

  // Now set the auto response
  if( argc > 2 )
  {
    ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
    o->SetAutoResponse(argv[2]);
    gUserManager.DropOwner(o);
  }

  return 0;
}


// auto_response <auto response>
static int fifo_auto_response( int argc, const char *const *argv, void* /* data */)
{
  if( argc == 1 )
  {
    ReportMissingParams(argv[0]);
    return -1;
  }

  ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
  o->SetAutoResponse(argv[1]);
  gUserManager.DropOwner(o);

  return 0;
}

// message <buddy> <message>
static int fifo_message ( int argc, const char *const *argv, void *data)
{
  CICQDaemon *d = (CICQDaemon *) data;;
  unsigned long nPPID;
  char *szId = 0;

  if( argc < 3 )
  {
    ReportMissingParams(argv[0]);
    return -1;
  }

  if( atoid(argv[1], false, &szId, &nPPID, d) )
    d->sendMessage(LicqUser::makeUserId(szId, nPPID), argv[2], true, 0);

  else
    ReportBadBuddy(argv[0], argv[1]);

  free(szId);

  return 0;
}

// url <buddy> <url> [<description>]
static int fifo_url ( int argc, const char *const *argv, void *data)
{
  const char *szDescr;
  CICQDaemon *d = (CICQDaemon *) data;
  unsigned long nPPID;
  char *szId = 0;

  if( argc < 3 )
  {
    ReportMissingParams(argv[0]);
    return -1;
  }

  if( atoid(argv[1], false, &szId, &nPPID, d) )
  {
    szDescr = (argc > 3) ? argv[3] : "" ;
    d->sendUrl(LicqUser::makeUserId(szId, nPPID), argv[2], szDescr, true, false);
  }
  else
    ReportBadBuddy(argv[0],argv[1]);

  free(szId);

  return 0;
}

//sms <buddy> <message>
static int fifo_sms(int argc, const char *const *argv, void *data)
{
  CICQDaemon *d = (CICQDaemon *) data;
  unsigned long nPPID;
  char *szId = 0;

  if (argc < 3)
  {
    ReportMissingParams(argv[0]);
    return -1;
  }

  if (atoid(argv[1], false, &szId, &nPPID, d) )
  { 
    if( nPPID == LICQ_PPID )
    {
      const ICQUser* u = gUserManager.FetchUser(szId, nPPID, LOCK_R);
      if (u != NULL)
      {
        string number = u->getCellularNumber();
        gUserManager.DropUser(u);
        if (!number.empty())
          d->icqSendSms(szId, nPPID, number.c_str(), argv[2]);
        else
          gLog.Error("%sUnable to send SMS to %s, no SMS number found.\n",
                     L_ERRORxSTR, szId);
      }
    }
    else
      gLog.Info(tr("%s `%s': bad protol. ICQ only alowed\n"), L_FIFOxSTR, argv[0]);
  }
  else
    ReportBadBuddy(argv[0], argv[1]);

  free(szId);

  return 0;
}

// sms-number <number> <message>
static int fifo_sms_number(int argc, const char *const *argv, void *data)
{
  CICQDaemon *d = (CICQDaemon *) data;

  if (argc < 3)
  {
    ReportMissingParams(argv[0]);
    return -1;
  }

  string id = gUserManager.OwnerId(LICQ_PPID);
  d->icqSendSms(id.c_str(), LICQ_PPID, argv[1], argv[2]);
  return 0;
}

// redirect <file>
static int fifo_redirect ( int argc, const char *const *argv, void* /* data */)
{
  if( argc == 1 )
  {
    ReportMissingParams(argv[0]);
    return -1;
  }

  if ( !Redirect(argv[1]) )
  {
    gLog.Warn(tr("%s %s: redirection to \"%s\" failed: %s.\n"),
              L_WARNxSTR,argv[0], argv[1],strerror(errno));
  }
  else
    gLog.Info(tr("%s %s: output redirected to \"%s\".\n"), L_INITxSTR, argv[0],
             argv[1]);

  return 0;
}

// debuglvl <level>
static int fifo_debuglvl ( int argc, const char *const *argv, void* /* data */)
{
  int nRet = 0; 
 
  if( (nRet = (argc == 1)) )
    ReportMissingParams(argv[0]);
  else
    gLog.ModifyService( S_STDERR, atoi(argv[1]));

  return -nRet;
}

// adduser <buddy>
static int fifo_adduser ( int argc, const char *const *argv, void *data)
{ 
  CICQDaemon *d = (CICQDaemon *) data;
  unsigned long nPPID;
  char *szId = 0;

  if( argc  == 1 )
  {
    ReportMissingParams(argv[0]);
    return -1;
  }

  if( atoid(argv[1], false, &szId, &nPPID, d) )
  {
    UserId userId = LicqUser::makeUserId(szId, nPPID);
    gUserManager.addUser(userId);
  }
  else
    ReportBadBuddy(argv[0],argv[1]);
  free(szId);

  return 0;
}

// userinfo <buddy>
static int fifo_userinfo ( int argc, const char *const *argv, void *data)
{
  CICQDaemon *d = (CICQDaemon *) data;
  unsigned long nPPID;
  char *szId = 0; 
  int ret = -1; 

  if ( argc == 1 )
    ReportMissingParams(argv[0]);
  else if( !atoid(argv[1], true, &szId, &nPPID, d) )
    ReportBadBuddy(argv[0],argv[1]);
  else if( nPPID != LICQ_PPID )
     gLog.Info(tr("%s `%s': bad protol. ICQ only alowed\n"), L_FIFOxSTR, argv[0]);
  else
  {
    const ICQUser* u = gUserManager.FetchUser(szId, nPPID, LOCK_R);
    if (u == NULL)
      gLog.Warn(tr("%s %s: user %s not on contact list, not retrieving "
                "info.\n"), L_WARNxSTR, argv[0], szId);
    else
    {
      UserId userId = u->id();
      gUserManager.DropUser(u);
      d->requestUserInfo(userId);
      ret = 0;
    }
  }
  
  free( szId );
  
  return ret;
}

// exit
static int fifo_exit(int /* argc */, const char* const* /* argv */, void* data)
{
  CICQDaemon *d = (CICQDaemon *) data;
  d->Shutdown();
  return 0;
}

// ui_viewevent [<buddy>]
static int fifo_ui_viewevent ( int argc, const char *const *argv, void *data)
{
  CICQDaemon *d = (CICQDaemon *) data; 
  unsigned long nPPID;
  char *szId = 0; 
  
  if( argc ==1 )
  {
    szId = strdup("0");
    nPPID = 0;
  }
  else if( !atoid(argv[1], true, &szId, &nPPID, d) )
  {  
    ReportBadBuddy(argv[0],argv[1]);
    free(szId);

    return 0;
  }
  
  d->pluginUIViewEvent(LicqUser::makeUserId(szId, nPPID));

  if (szId != NULL)
    free(szId);

  return 0;
}

// ui_message <buddy>
static int fifo_ui_message ( int argc, const char *const *argv, void *data)
{
  CICQDaemon *d = (CICQDaemon *) data;
  unsigned long nPPID = 0;
  char *szId = 0;
  int nRet = 0;

  if ( argc == 1 )
  {
    ReportMissingParams(argv[0]);
    nRet = -1;
  }
  else if( atoid(argv[1], true, &szId, &nPPID, d) )
    d->pluginUIMessage(LicqUser::makeUserId(szId, nPPID));
  else
  {
    ReportBadBuddy(argv[0],argv[1]);
    return -1;
  }
  free(szId);

  return nRet;
}

static int fifo_plugin_list(int /* argc */, const char* const* /* argv */, void *data)
{
  CICQDaemon *d = (CICQDaemon *) data;
  PluginsList l;
  PluginsListIter it;

  d->PluginList(l);
  for (it = l.begin(); it != l.end(); ++it)
  {
    gLog.Info("[%3d] %s\n", (*it)->Id(), (*it)->Name());
  }
  return 0;
}

static int fifo_plugin_load(int argc, const char *const *argv, void *data)
{
  CICQDaemon *d = (CICQDaemon *) data;
  PluginsList l;
  PluginsListIter it;

  if (argc == 1)
  {
    ReportMissingParams(argv[0]);
    return -1;
  }
  
  const char *sz[] = { "licq", NULL };
  if (d->PluginLoad(argv[1], 1, const_cast<char**>(sz)))
    return 0;
  
  gLog.Info("Couldn't load plugin '%s'\n", argv[1]);
  return -1;
}

static int fifo_plugin_unload(int argc, const char *const *argv, void *data)
{
  CICQDaemon *d = (CICQDaemon *) data;
  PluginsList l;
  PluginsListIter it;

  if( argc == 1 )
  {
    ReportMissingParams(argv[0]);
    return -1;
  }
  
  d->PluginList(l);
  for (it = l.begin(); it != l.end(); ++it)
  {
    if (strcmp((*it)->Name(), argv[1]) == 0)
    {
      d->PluginShutdown((*it)->Id());
	  return 0;
    }
  }
  gLog.Info("Couldn't find plugin '%s'\n", argv[1]);
  return -1;
}

static int fifo_proto_plugin_list(int /* argc */, const char* const* /* argv */, void *data)
{
  CICQDaemon *d = (CICQDaemon *) data;
  ProtoPluginsList l;
  ProtoPluginsListIter it;

  d->ProtoPluginList(l);
  for (it = l.begin(); it != l.end(); ++it)
  {
    gLog.Info("[%3d] %s\n", (*it)->Id(), (*it)->Name());
  }
  return 0;
}

static int fifo_proto_plugin_load(int argc, const char *const *argv, void *data)
{
  CICQDaemon *d = (CICQDaemon *) data;

  if (argc == 1)
  {
    ReportMissingParams(argv[0]);
    return -1;
  }
  
  if (d->ProtoPluginLoad(argv[1]))
    return 0;
  
  gLog.Info("Couldn't load protocol plugin '%s'\n", argv[1]);
  return -1;
}

static int fifo_proto_plugin_unload(int argc, const char *const *argv, void *data)
{
  CICQDaemon *d = (CICQDaemon *) data;
  ProtoPluginsList l;
  ProtoPluginsListIter it;

  if (argc == 1)
  {
    ReportMissingParams(argv[0]);
    return -1;
  }

  d->ProtoPluginList(l);
  for (it = l.begin(); it != l.end(); ++it)
  {
    if (strcmp((*it)->Name(), argv[1]) == 0)
    {
      d->ProtoPluginShutdown((*it)->Id());
      return 0;
    }
  }
  gLog.Info("Couldn't find protocol plugin '%s'\n", argv[1]);
  return -1;
}

static int fifo_help ( int argc, const char *const *argv, void *data)
{
  struct command_t *table = (struct command_t *)data;
  int i,j;

  if( argc == 1 )
  {
    gLog.Info(tr("%sFifo commands:\n"), L_FIFOxSTR);
    for( i=0; table[i].fnc ; i++ )
      gLog.Info("%s%s\n",L_BLANKxSTR,table[i].szName);
    gLog.Info(tr("%s: Type `help command'\n"), L_FIFOxSTR);
  }
  else 
  {
    for (i = 0 ; i < argc ; i++)
    {
      if (strcmp(argv[i], "all") == 0)
      {
        // show help for all commands 
        j = 0;
        while (table[j].szName)
        {
          gLog.Info(tr("%s %s: help for `%s'\n%s\n"),
                    L_FIFOxSTR, argv[0], table[j].szName, table[j].szHelp);
          j++;
	}
      }
      else
      {
        // show help for a specific command
        j = process_tok(table, argv[i]);
        if (j >= 0)
          gLog.Info(tr("%s %s: help for `%s'\n%s\n"),
                    L_FIFOxSTR, argv[0], argv[i], table[j].szHelp);
        else
          gLog.Info(tr("%s %s: unknown command `%s'\n"),
                    L_FIFOxSTR, argv[0], argv[i]);
      }
    }
  }
  return 0;
}


static struct command_t fifocmd_table[]=
{
  {"status",              fifo_status,              HELP_STATUS,            0},
  {"auto_response",       fifo_auto_response,       HELP_AUTO,              0},
  {"message",             fifo_message,             HELP_MSG,               0},
  {"url",                 fifo_url,                 HELP_URL,               0},
  {"sms",                 fifo_sms,                 HELP_SMS,               0},
  {"sms-number",          fifo_sms_number,          HELP_SMS_NUMBER,        0},
  {"redirect",            fifo_redirect,            HELP_REDIRECT,          0},
  {"debuglvl",            fifo_debuglvl,            HELP_DEBUGLVL,          0},
  {"adduser",             fifo_adduser,             HELP_ADDUSER,           0},
  {"userinfo",            fifo_userinfo,            HELP_USERINFO,          0},
  {"exit",                fifo_exit,                HELP_EXIT,              0},
  {"ui_viewevent",        fifo_ui_viewevent,        HELP_UIVIEWEVENT,       0},
  {"ui_message",          fifo_ui_message,          HELP_UIMESSAGE,         0},
  {"list_plugins",        fifo_plugin_list,         HELP_PLUGINLIST,        0},
  {"load_plugin",         fifo_plugin_load,         HELP_PLUGINLOAD,        0},
  {"unload_plugin",       fifo_plugin_unload,       HELP_PLUGINUNLOAD,      0},
  {"list_proto_plugins",  fifo_proto_plugin_list,   HELP_PROTOPLUGINLIST,   0},
  {"load_proto_plugin",   fifo_proto_plugin_load,   HELP_PROTOPLUGINLOAD,   0},
  {"unload_proto_plugin", fifo_proto_plugin_unload, HELP_PROTOPLUGINUNLOAD, 0},
  {"help",                fifo_help,                HELP_HELP,              1},
  {NULL,                  NULL,                     NULL,                   0}
};

//-----ProcessFifo--------------------------------------------------------------
static char getQuotedChar( char c )
{
  char ret;

  switch( c )
  {
    case 'n': ret = '\n'; break;
    case 't': ret = '\t'; break;
    case 'v': ret = '\v'; break;
    case 'b': ret = '\b'; break;
    case 'r': ret = '\r'; break;
    case 'f': ret = '\f'; break;
    case 'a': ret = '\a'; break;
    default: ret = c;    break;
  }
  return ret;
}

/*! 
 * given a command line string p, fills argv and argc. 
 * p is modified.
 */
static bool line2argv( char *p, char **argv, int *argc, int size )
{
  char *q;
  bool bQuote;

  /* trim */
  for( ; isspace(*p) ; p ++ )
    ;
  for( q=p+strlen(p)-1; isspace(*q) ; q-- )
    ;
  q[1] = '\0'; 

  /* split: the ugly thing  */
  bQuote=false,*argc=0,argv[(*argc)++]=p,size--;
  for( q = p ; *argc < size && *p ; p++)
  {
    if(!bQuote && isspace(*p))
    {
      for( *(q++)='\0'; isspace(*(p+1)) ; p++ )
        ;
      if( *(p+1) )
        argv[(*argc)++] = q; 
    }
    else if( *p == '"' )
      bQuote = !bQuote;
    else if( bQuote && *p == '\\' )
      *(q++) = getQuotedChar( *(++p) );
    else
      *(q++) = *p;
  }
  *q = '\0';
 
  // TODO:  ADD argv[argv] = NULL?
  return !bQuote;
}

/*!
 * \returns the index of @tok in @table or CL_UNKNOWN if @tok doesn't exist 
 */
static int process_tok(const command_t *table,const char *tok)
{
  int i;
  bool bFound;

  /* empty line */
  if(tok[0]==0)
    return CL_NONE;
 
  for( i=0, bFound=false ; !bFound && table[i].szName != NULL ; i++ )
    if( ! strcasecmp(table[i].szName,tok) )
      bFound =  true;
 
  return  bFound ? i -1 : CL_UNKNOWN;
}

void CICQDaemon::ProcessFifo(const char* _szBuf)
{
#ifdef USE_FIFO
  int argc, index;
  char * argv[MAX_ARGV];
  char *szBuf = strdup(_szBuf);

  if( szBuf == NULL )
    return ;

  gLog.Info(tr("%sReceived string: %s"), L_FIFOxSTR, szBuf);
  line2argv(szBuf, argv, &argc, sizeof(argv) / sizeof(argv[0]) );
  index = process_tok(fifocmd_table,argv[0]);

  switch( index )
  {
    case CL_UNKNOWN:
      gLog.Info(tr("%s: '%s' Unknown fifo command. Try 'help'\n"),
                L_FIFOxSTR,argv[0]);
      break;
    case CL_NONE:
      break;
    default:
      argv[0] = (char *)fifocmd_table[index].szName;
      if( fifocmd_table[index].fnc )
      {
        if(fifocmd_table[index].bHelp)
          fifocmd_table[index].fnc(argc,argv,(void *)fifocmd_table);
        else
          fifocmd_table[index].fnc(argc,argv,this);
      }
      break;
  }
  
  free( szBuf );

#endif //USE_FIFO
}
