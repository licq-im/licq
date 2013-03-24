/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1998-2013 Licq developers <licq-dev@googlegroups.com>
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
#include "config.h"

#include "fifo.h"

#include <boost/foreach.hpp>
#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cctype>
#include <list>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <licq/contactlist/owner.h>
#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>
#include <licq/daemon.h>
#include <licq/icq/icq.h>
#include <licq/logging/log.h>
#include <licq/logging/logservice.h>
#include <licq/logging/logutils.h>
#include <licq/plugin/pluginmanager.h>
#include <licq/plugin/protocolplugin.h>
#include <licq/pluginsignal.h>
#include <licq/protocolmanager.h>
#include <licq/translator.h>
#include <licq/userid.h>

#include "gettext.h"
#include "licq.h"

using std::string;
using Licq::PluginSignal;
using Licq::UserId;
using Licq::gDaemon;
using Licq::gLog;
using Licq::gLogService;
using Licq::gPluginManager;
using Licq::gProtocolManager;
using Licq::gTranslator;
using Licq::gUserManager;
using namespace LicqDaemon;

#define ReportMissingParams(cmdname) \
    (gLog.info("%s `%s': %s `help %s'", \
    L_FIFOxSTR, cmdname, tr("missing arguments. try"), cmdname))

#define ReportBadBuddy(cmdname,szUin) \
    (gLog.info("%s `%s': %s `%s'", L_FIFOxSTR, cmdname, \
    tr("bad buddy string"), szUin))

static const char L_FIFOxSTR[] = "[FIF] ";

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
static const char* const HELP_SETPICTURE = tr(
    "\tsetpicture <filename> [<protocol>]\n"
    "\t\tChanges picture for an account, or all accounts if no protocol entered.\n"
    "\t\tAn empty filename will remove the current picture.\n");
static const char* const HELP_EXIT = tr(
        "\texit\n"
        "\t\tCauses the Licq session to shutdown.\n");
static const char* const HELP_UIVIEWEVENT = tr(
        "\tui_viewevent [<buddy>]\n"
        "\t\tShows the oldest pending event.\n");
static const char* const HELP_UIMESSAGE = tr(
        "\tui_message <buddy>\n"
        "\t\tOpen the plugin message composer to <buddy>\n");
static const char* const HELP_UISHOWUSERLIST = tr(
    "\tui_showuserlist\n"
    "\t\tShow and raise the contact list window\n");
static const char* const HELP_UIHIDEUSERLIST = tr(
    "\tui_hideuserlist\n"
    "\t\tHide the contact list window\n");
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

const unsigned short MAX_UIN_DIGITS  = 13;

enum 
{
  CL_UNKNOWN=-1,
  CL_NONE=-2
};

struct command_t
{
  const char *const szName;
  int (*fnc)(int, const char* const*);
  const char *const szHelp;
};

static int process_tok(const command_t *table,const char *tok);

static std::string buffer_get_ids(const std::string& buffer, unsigned long* protocolId)
{
  std::string name(buffer);
  size_t pos = 0;
  *protocolId = 0;

  while (1)
  {
    pos = name.find('@', pos);

    if (pos == string::npos)
      // Single @ not found, return unmodified buffer
      return buffer;

    if (name[pos+1] == '@')
    {
      // @@ -> @
      name.erase(pos, 1);
      pos++;
      continue;
    }

    break;
  }

  std::string protocolName = name.substr(pos+1);
  name.erase(pos);

  *protocolId = Licq::protocolId_fromString(protocolName);
  if (*protocolId != 0)
    return name;

  // Protocol not recognized, return unmodified buffer
  return buffer;
}

/*! \brief Given an ascii string gets the szId and the nPPID
 *
 *  -# If all chars are digits then:
 *    -# assume it is a uin, and if bList flag is on, then check if it 
 *       is in the list
 *  -# If that fail try with alias Params.
 *
 * @param buff string to convert
 * @return Valid user id on success
 */
static Licq::UserId atoid(const char* buff, bool missingok = true)
{
  if (buff == NULL)
    return Licq::UserId();

  unsigned long protocolId;
  string name = buffer_get_ids(buff, &protocolId);

  Licq::UserListGuard userList(protocolId);
  BOOST_FOREACH(const Licq::User* user, **userList)
  {
    Licq::UserReadGuard u(user);
    if (u->accountId() == name || u->getAlias() == name)
      return u->id();
  }

  if (missingok && protocolId != 0)
  {
    // Use first owner with matching protocol id
    Licq::OwnerListGuard ownerList(protocolId);
    if (!ownerList->empty())
      return Licq::UserId((*ownerList->begin())->id(), name);
  }

  return Licq::UserId();
}

// status 
static int fifo_status(int argc, const char* const* argv)
{
  const char *szStatus = argv[1];

  if( argc == 1 )
  {
    ReportMissingParams(argv[0]);
    return -1;
  }

  // Determine the status to go to
  unsigned status;
  if (!Licq::User::stringToStatus(szStatus, status))
  {
    gLog.warning(tr("%s %s: command with invalid status \"%s\""),
              L_FIFOxSTR,argv[0],szStatus);
    return -1;
  }

  string statusMsg = Licq::ProtocolManager::KeepAutoResponse;
  if ( argc > 2 )
    statusMsg = gTranslator.toUtf8(argv[2]);

  // Get a list of owners first since we can't call setStatus with list locked
  std::list<Licq::UserId> owners;
  {
    Licq::OwnerListGuard ownerList;
    BOOST_FOREACH(const Licq::Owner* o, **ownerList)
      owners.push_back(o->id());
  }

  BOOST_FOREACH(const Licq::UserId& ownerId, owners)
    gProtocolManager.setStatus(ownerId, status, statusMsg);

  return 0;
}


// auto_response <auto response>
static int fifo_auto_response(int argc, const char* const* argv)
{
  if( argc == 1 )
  {
    ReportMissingParams(argv[0]);
    return -1;
  }

  Licq::OwnerListGuard ownerList;
  BOOST_FOREACH(Licq::Owner* owner, **ownerList)
  {
    Licq::OwnerWriteGuard o(owner);
    if ((o->protocolCapabilities() & Licq::ProtocolPlugin::CanHoldStatusMsg) == 0)
      continue;

    o->setAutoResponse(gTranslator.toUtf8(argv[1]));
    o->save(Licq::Owner::SaveOwnerInfo);
  }

  return 0;
}

// message <buddy> <message>
static int fifo_message(int argc, const char* const* argv)
{
  if( argc < 3 )
  {
    ReportMissingParams(argv[0]);
    return -1;
  }

  Licq::UserId userId(atoid(argv[1]));
  if (userId.isValid())
    gProtocolManager.sendMessage(userId, gTranslator.toUtf8(argv[2]));
  else
    ReportBadBuddy(argv[0], argv[1]);

  return 0;
}

// url <buddy> <url> [<description>]
static int fifo_url(int argc, const char* const* argv)
{
  if( argc < 3 )
  {
    ReportMissingParams(argv[0]);
    return -1;
  }

  Licq::UserId userId(atoid(argv[1]));
  if (userId.isValid())
  {
    const char* szDescr = (argc > 3) ? argv[3] : "" ;
    gProtocolManager.sendUrl(userId, gTranslator.toUtf8(argv[2]),
        gTranslator.toUtf8(szDescr));
  }
  else
    ReportBadBuddy(argv[0],argv[1]);

  return 0;
}

//sms <buddy> <message>
static int fifo_sms(int argc, const char *const *argv)
{
  if (argc < 3)
  {
    ReportMissingParams(argv[0]);
    return -1;
  }

  Licq::UserId userId(atoid(argv[1]));
  if (!userId.isValid())
  {
    ReportBadBuddy(argv[0], argv[1]);
    return 0;
  }

  if (userId.protocolId() != ICQ_PPID)
  {
    gLog.info(tr("%s `%s': bad protocol. ICQ only allowed"), L_FIFOxSTR, argv[0]);
    return 0;
  }

  Licq::IcqProtocol::Ptr icq = plugin_internal_cast<Licq::IcqProtocol>(
      Licq::gPluginManager.getProtocolInstance(userId.ownerId()));
  if (!icq)
    return -1;

  string number;
  {
    Licq::UserReadGuard u(userId);
    {
      if (u.isLocked())
        number = u->getCellularNumber();
    }
  }
  if (!number.empty())
    icq->icqSendSms(userId, number, gTranslator.toUtf8(argv[2]));
  else
    gLog.error(tr("Unable to send SMS to %s, no SMS number found"), userId.accountId().c_str());

  return 0;
}

// sms-number <number> <message>
static int fifo_sms_number(int argc, const char *const *argv)
{
  if (argc < 3)
  {
    ReportMissingParams(argv[0]);
    return -1;
  }

  Licq::UserId ownerId;
  {
    Licq::OwnerListGuard ownerList(ICQ_PPID);
    if (ownerList->empty())
      return -1;
    ownerId = (*ownerList->begin())->id();
  }

  Licq::IcqProtocol::Ptr icq = plugin_internal_cast<Licq::IcqProtocol>(
      Licq::gPluginManager.getProtocolInstance(ownerId));
  if (!icq)
    return -1;

  icq->icqSendSms(ownerId, argv[1], gTranslator.toUtf8(argv[2]));
  return 0;
}

// redirect <file>
static int fifo_redirect(int argc, const char* const* argv)
{
  if( argc == 1 )
  {
    ReportMissingParams(argv[0]);
    return -1;
  }

  int fd = open(argv[1], O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
  if (fd == -1 || dup2(fd, STDERR_FILENO) == -1)
  {
    gLog.warning(tr("%s: redirection to \"%s\" failed: %s"),
        argv[0], argv[1],strerror(errno));
  }
  else
    gLog.info(tr("%s: output redirected to \"%s\""), argv[0],
             argv[1]);

  return 0;
}

// debuglvl <level>
static int fifo_debuglvl(int argc, const char* const* argv)
{
  int nRet = 0; 
 
  if( (nRet = (argc == 1)) )
    ReportMissingParams(argv[0]);
  else
  {
    unsigned int mask = Licq::LogUtils::convertOldBitmaskToNew(::atoi(argv[1]));
    gLogService.getDefaultLogSink()->setLogLevelsFromBitmask(mask);
  }

  return -nRet;
}

// adduser <buddy>
static int fifo_adduser(int argc, const char* const* argv)
{
  if( argc  == 1 )
  {
    ReportMissingParams(argv[0]);
    return -1;
  }

  Licq::UserId userId(atoid(argv[1]));
  if (userId.isValid())
    gUserManager.addUser(userId);
  else
    ReportBadBuddy(argv[0],argv[1]);

  return 0;
}

// userinfo <buddy>
static int fifo_userinfo(int argc, const char* const* argv)
{
  if ( argc == 1 )
  {
    ReportMissingParams(argv[0]);
    return -1;
  }

  Licq::UserId userId(atoid(argv[1], false));
  if (!userId.isValid())
  {
    ReportBadBuddy(argv[0],argv[1]);
    return -1;
  }

  if (!gUserManager.userExists(userId))
  {
    gLog.warning(tr("%s: user %s not on contact list, not retrieving info"), argv[0], userId.accountId().c_str());
    return -1;
  }

  gProtocolManager.requestUserInfo(userId);
  return 0;
}

static int fifo_setpicture(int argc, const char* const* argv)
{
  if (argc < 2)
  {
    ReportMissingParams(argv[0]);
    return -1;
  }

  unsigned long protocolId = 0;
  if (argc > 2)
  {
    // Just one plugin, find which one

    protocolId = Licq::protocolId_fromString(argv[2]);
    if (protocolId == 0)
    {
      gLog.info(tr("Couldn't find plugin '%s'"), argv[2]);
      return -1;
    }
  }

  Licq::OwnerListGuard ownerList(protocolId);
  BOOST_FOREACH(Licq::Owner* owner, **ownerList)
  {
    {
      Licq::OwnerWriteGuard o(owner);
      if (strlen(argv[1]) == 0)
        o->SetPicture(NULL);
      else
        o->SetPicture(argv[1]);
      o->save(Licq::Owner::SavePictureInfo);
    }
    Licq::gUserManager.notifyUserUpdated(owner->id(), Licq::PluginSignal::UserPicture);

    if (owner->id().protocolId() == ICQ_PPID)
    {
      Licq::IcqProtocol::Ptr icq = plugin_internal_cast<Licq::IcqProtocol>(
          Licq::gPluginManager.getProtocolInstance(owner->id()));

      icq->icqUpdateInfoTimestamp(
          owner->id(), Licq::IcqProtocol::PluginPicture);
    }
  }

  return 0;
}

// exit
static int fifo_exit(int /* argc */, const char* const* /* argv */)
{
  gDaemon.Shutdown();
  return 0;
}

// ui_viewevent [<buddy>]
static int fifo_ui_viewevent(int argc, const char* const* argv)
{
  Licq::UserId userId;

  if (argc > 1)
  {
    userId = atoid(argv[1], false);
    if (!userId.isValid())
    {
      ReportBadBuddy(argv[0],argv[1]);
      return 0;
    }
  }

  gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalPluginEvent,
      PluginSignal::PluginViewEvent, userId));

  return 0;
}

// ui_message <buddy>
static int fifo_ui_message(int argc, const char* const* argv)
{
  if ( argc == 1 )
  {
    ReportMissingParams(argv[0]);
    return -1;
  }

  Licq::UserId userId(atoid(argv[1], false));
  if (!userId.isValid())
  {
    ReportBadBuddy(argv[0],argv[1]);
    return -1;
  }

  gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalPluginEvent,
      PluginSignal::PluginStartMessage, userId));

  return 0;
}

static int fifo_ui_showuserlist(int /* argc */, const char* const* /* argv */)
{
  gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalPluginEvent,
      PluginSignal::PluginShowUserList));
  return 0;
}

static int fifo_ui_hideuserlist(int /* argc */, const char* const* /* argv */)
{
  gPluginManager.pushPluginSignal(new PluginSignal(PluginSignal::SignalPluginEvent,
      PluginSignal::PluginHideUserList));
  return 0;
}

static int fifo_plugin_list(int /* argc */, const char* const* /* argv */)
{
  Licq::GeneralPluginsList plugins;
  gPluginManager.getGeneralPluginsList(plugins);

  BOOST_FOREACH(Licq::GeneralPlugin::Ptr plugin, plugins)
  {
    gLog.info("%s\n", plugin->name().c_str());
  }
  return 0;
}

static int fifo_plugin_load(int argc, const char* const* argv)
{
  if (argc == 1)
  {
    ReportMissingParams(argv[0]);
    return -1;
  }

  if (gPluginManager.startGeneralPlugin(argv[1], 0, NULL))
    return 0;

  gLog.info(tr("Couldn't load plugin '%s'"), argv[1]);
  return -1;
}

static int fifo_plugin_unload(int argc, const char* const* argv)
{
  if( argc == 1 )
  {
    ReportMissingParams(argv[0]);
    return -1;
  }

  Licq::GeneralPluginsList plugins;
  gPluginManager.getGeneralPluginsList(plugins);

  BOOST_FOREACH(Licq::GeneralPlugin::Ptr plugin, plugins)
  {
    if (plugin->name() == argv[1])
    {
      gPluginManager.unloadGeneralPlugin(plugin);
      return 0;
    }
  }
  gLog.info(tr("Couldn't find plugin '%s'"), argv[1]);
  return -1;
}

static int fifo_proto_plugin_list(int /* argc */, const char* const* /* argv */)
{
  Licq::ProtocolPluginsList plugins;
  gPluginManager.getProtocolPluginsList(plugins);

  BOOST_FOREACH(Licq::ProtocolPlugin::Ptr plugin, plugins)
  {
    gLog.info("%s\n", plugin->name().c_str());
  }
  return 0;
}

static int fifo_proto_plugin_load(int argc, const char* const* argv)
{
  if (argc == 1)
  {
    ReportMissingParams(argv[0]);
    return -1;
  }

  if (gPluginManager.startProtocolPlugin(argv[1]))
    return 0;

  gLog.info(tr("Couldn't load protocol plugin '%s'"), argv[1]);
  return -1;
}

static int fifo_proto_plugin_unload(int argc, const char* const* argv)
{
  if (argc == 1)
  {
    ReportMissingParams(argv[0]);
    return -1;
  }

  Licq::ProtocolPluginsList plugins;
  gPluginManager.getProtocolPluginsList(plugins);

  BOOST_FOREACH(Licq::ProtocolPlugin::Ptr plugin, plugins)
  {
    if (plugin->name() == argv[1])
    {
      gPluginManager.unloadProtocolPlugin(plugin);
      return 0;
    }
  }
  gLog.info(tr("Couldn't find protocol plugin '%s'"), argv[1]);
  return -1;
}

static int fifo_help(int argc, const char *const *argv);

static struct command_t fifocmd_table[]=
{
  {"status",              fifo_status,              HELP_STATUS},
  {"auto_response",       fifo_auto_response,       HELP_AUTO},
  {"message",             fifo_message,             HELP_MSG},
  {"url",                 fifo_url,                 HELP_URL},
  {"sms",                 fifo_sms,                 HELP_SMS},
  {"sms-number",          fifo_sms_number,          HELP_SMS_NUMBER},
  {"redirect",            fifo_redirect,            HELP_REDIRECT},
  {"debuglvl",            fifo_debuglvl,            HELP_DEBUGLVL},
  {"adduser",             fifo_adduser,             HELP_ADDUSER},
  {"userinfo",            fifo_userinfo,            HELP_USERINFO},
  {"setpicture",          fifo_setpicture,          HELP_SETPICTURE},
  {"exit",                fifo_exit,                HELP_EXIT},
  {"ui_viewevent",        fifo_ui_viewevent,        HELP_UIVIEWEVENT},
  {"ui_message",          fifo_ui_message,          HELP_UIMESSAGE},
  {"ui_showuserlist",     fifo_ui_showuserlist,     HELP_UISHOWUSERLIST},
  {"ui_hideuserlist",     fifo_ui_hideuserlist,     HELP_UIHIDEUSERLIST},
  {"list_plugins",        fifo_plugin_list,         HELP_PLUGINLIST},
  {"load_plugin",         fifo_plugin_load,         HELP_PLUGINLOAD},
  {"unload_plugin",       fifo_plugin_unload,       HELP_PLUGINUNLOAD},
  {"list_proto_plugins",  fifo_proto_plugin_list,   HELP_PROTOPLUGINLIST},
  {"load_proto_plugin",   fifo_proto_plugin_load,   HELP_PROTOPLUGINLOAD},
  {"unload_proto_plugin", fifo_proto_plugin_unload, HELP_PROTOPLUGINUNLOAD},
  {"help",                fifo_help,                HELP_HELP},
  {NULL,                  NULL,                     NULL}
};

static int fifo_help(int argc, const char* const* argv)
{
  int i,j;

  if( argc == 1 )
  {
    gLog.info(tr("%sFifo commands:\n"), L_FIFOxSTR);
    for (i = 0; fifocmd_table[i].fnc; i++)
      gLog.info("                %s", fifocmd_table[i].szName);
    gLog.info(tr("%s: Type `help command'\n"), L_FIFOxSTR);
  }
  else 
  {
    for (i = 0 ; i < argc ; i++)
    {
      if (strcmp(argv[i], "all") == 0)
      {
        // show help for all commands 
        j = 0;
        while (fifocmd_table[j].szName)
        {
          gLog.info(tr("%s %s: help for `%s'\n%s\n"),
              L_FIFOxSTR, argv[0], fifocmd_table[j].szName, fifocmd_table[j].szHelp);
          j++;
	}
      }
      else
      {
        // show help for a specific command
        j = process_tok(fifocmd_table, argv[i]);
        if (j >= 0)
          gLog.info(tr("%s %s: help for `%s'\n%s\n"),
              L_FIFOxSTR, argv[0], argv[i], fifocmd_table[j].szHelp);
        else
          gLog.info(tr("%s %s: unknown command `%s'\n"),
                    L_FIFOxSTR, argv[0], argv[i]);
      }
    }
  }
  return 0;
}


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
static bool line2argv( char *p, const char **argv, int *argc, int size )
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

// Declare global Fifo (internal for daemon)
LicqDaemon::Fifo LicqDaemon::gFifo;

Fifo::Fifo()
{
  // Empty
}

Fifo::~Fifo()
{
  // Empty
}

void Fifo::initialize(Licq::MainLoop& mainLoop)
{
  string filename = gDaemon.baseDir() + "licq_fifo";

  // Open the fifo
  gLog.info(tr("Opening fifo"));
  fifo_fd = open(filename.c_str(), O_RDWR);
  if (fifo_fd == -1)
  {
    if (mkfifo(filename.c_str(), 00600) == -1)
      gLog.warning(tr("Unable to create fifo: %s"), strerror(errno));
    else
    {
      fifo_fd = open(filename.c_str(), O_RDWR);
      if (fifo_fd == -1)
        gLog.warning(tr("Unable to open fifo: %s"), strerror(errno));
    }
  }
  if (fifo_fd != -1)
  {
    struct stat buf;
    fstat(fifo_fd, &buf);
    if (!S_ISFIFO(buf.st_mode))
    {
      gLog.warning(tr("%s is not a FIFO, disabling fifo support"),
          filename.c_str());
      close(fifo_fd);
      fifo_fd = -1;
    }
  }

  // Register callback from main loop when there is data to read
  if (fifo_fd != -1)
    mainLoop.addRawFile(fifo_fd, this);
}

void Fifo::shutdown()
{
  if (fifo_fd != -1)
    close(fifo_fd);
}

void Fifo::rawFileEvent(int /*fd*/, int /*revents*/)
{
  char szBuf[1024];
  ssize_t readret = read(fifo_fd, szBuf, sizeof(szBuf));
  if (readret <= 0)
    return;
  myInputBuffer.append(szBuf, readret);

  // Handle each line in the buffer (and don't assume it's only one at a time)
  size_t nlpos;
  while ((nlpos = myInputBuffer.find('\n')) != string::npos)
  {
    // Extract first line from buffer
    size_t copylen = (nlpos < sizeof(szBuf) ? nlpos+1 : sizeof(szBuf));
    myInputBuffer.copy(szBuf, 0, copylen);
    szBuf[copylen] = '\0';
    myInputBuffer.erase(0, nlpos+1);

    int argc, index;
    const char* argv[MAX_ARGV];

    gLog.info(tr("%sReceived string: %s"), L_FIFOxSTR, szBuf);
    line2argv(szBuf, argv, &argc, sizeof(argv) / sizeof(argv[0]) );
    index = process_tok(fifocmd_table,argv[0]);

    switch (index)
    {
      case CL_UNKNOWN:
        gLog.info(tr("%s: '%s' Unknown fifo command. Try 'help'\n"),
            L_FIFOxSTR,argv[0]);
        break;
      case CL_NONE:
        break;
      default:
        argv[0] = fifocmd_table[index].szName;
        if (fifocmd_table[index].fnc)
          fifocmd_table[index].fnc(argc, argv);
        break;
    }
  }
}
