/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1998-2011 Licq developers
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

#include "config.h"

#include "userhistory.h"

#include <boost/foreach.hpp>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <licq/logging/log.h>
#include <licq/userevents.h>
#include <licq/userid.h>

#include "../gettext.h"

#define MAX_HISTORY_MSG_SIZE 8192

using namespace std;
using Licq::UserId;
using Licq::gLog;
using LicqDaemon::UserHistory;

UserHistory::UserHistory(unsigned long ppid)
  : myPpid(ppid)
{
}

UserHistory::~UserHistory()
{
}

void UserHistory::setFile(const string& filename, const string& description)
{
  myFilename = filename;
  myDescription = description;
}


/* szResult[0] != ':' doubles to check if strlen(szResult) < 1 */
#define GET_VALID_LINE_OR_BREAK(dest) \
  { \
    if ((szResult = fgets(sz, sizeof(sz), f)) == NULL || szResult[0] != ':') \
      break; \
    dest.assign(sz+1, strlen(sz+1)-1); \
  }

#define GET_VALID_LINES(dest) \
  { \
    while ((szResult = fgets(sz, sizeof(sz), f)) != NULL && sz[0] == ':') \
      dest.append(sz+1); \
  }

#define SKIP_VALID_LINES \
  { \
    while ((szResult = fgets(sz, sizeof(sz), f)) != NULL && sz[0] == ':') ; \
  }

bool UserHistory::load(Licq::HistoryList& lHistory) const
{
  if (myFilename.empty())
    return false;

  FILE* f = fopen(myFilename.c_str(), "r");
  if (f == NULL)
  {
    if (errno == ENOENT)
    {
      return true;
    }
    else
    {
      gLog.warning(tr("Unable to open history file (%s): %s."),
          myFilename.c_str(), strerror(errno));
      return false;
    }
  }

  // Now read in a line at a time
  char sz[4096], *szResult;
  unsigned long nFlags;
  unsigned short nCommand, nSubCommand;
  time_t tTime;
  char cDir;
  Licq::UserEvent* e;
  szResult = fgets(sz, sizeof(sz), f);
  while(true)
  {
    while (szResult != NULL && sz[0] != '[')
      szResult = fgets(sz, sizeof(sz), f);
    if (szResult == NULL) break;
    // Zero unused part of sz to avoid interpreting garbage if sz is too
    // short
    memset(sz + strlen(sz) + 1, '\0', sizeof(sz) - strlen(sz) - 1);
    //"[ C | 0000 | 0000 | 0000 | 000... ]"
    cDir = sz[2];
    // Stick some \0's in to terminate strings
    sz[0] = sz[10] = sz[17] = sz[24] = '\0';
    // Read out the relevant values
    nSubCommand = atoi(&sz[6]);
    nCommand = atoi(&sz[13]);
    nFlags = atoi(&sz[20]) << 16;
    tTime = (time_t)atoi(&sz[27]);

    // nCommand == Licq::UserEvent::CommandDirect => FlagDirect (already present in flags)
    // nCommand == Licq::UserEvent::CommandSent => FlagSender (present in cDir)
    // nCommand == Licq::UserEvent::CommandRcvOnline => No additional flags set
    if (nCommand == Licq::UserEvent::CommandRcvOffline)
      nFlags |= Licq::UserEvent::FlagOffline;

    if (cDir != 'R')
      nFlags |= Licq::UserEvent::FlagSender;

    // Now read in the message
    e = NULL;
    switch (nSubCommand)
    {
      case Licq::UserEvent::TypeMessage:
      {
        string message;
        GET_VALID_LINES(message);
        e = new Licq::EventMsg(message, tTime, nFlags);
        break;
      }
      case Licq::UserEvent::TypeChat:
      {
        if (nCommand != Licq::UserEvent::CommandCancelled)
        {
          string message;
          GET_VALID_LINES(message);
          e = new Licq::EventChat(message, 0, tTime, nFlags);
        }
        else
        {
          SKIP_VALID_LINES;
          //e = new Licq::EventChatCancel(0, tTime, nFlags);
        }
        break;
      }
      case Licq::UserEvent::TypeFile:
      {
        if (nCommand != Licq::UserEvent::CommandCancelled)
        {
          string file, sizeStr, message;
          GET_VALID_LINE_OR_BREAK(file);
          GET_VALID_LINE_OR_BREAK(sizeStr);
          unsigned long nSize = atoi(sizeStr.c_str());
          GET_VALID_LINES(message);
          list<string> filelist;
          filelist.push_back(file);
          e = new Licq::EventFile(file, message, nSize, filelist, 0, tTime, nFlags);
        }
        else
        {
          SKIP_VALID_LINES;
          //e = new Licq::EventFileCancel(0, tTime, nFlags);
        }
        break;
      }
      case Licq::UserEvent::TypeUrl:
      {
        string url, message;
        GET_VALID_LINE_OR_BREAK(url);
        GET_VALID_LINES(message);
        e = new Licq::EventUrl(url, message, tTime, nFlags);
        break;
      }
      case Licq::UserEvent::TypeAuthRequest:
      {
        string accountId, alias, firstName, lastName, email, message;
        GET_VALID_LINE_OR_BREAK(accountId);
        GET_VALID_LINE_OR_BREAK(alias);
        GET_VALID_LINE_OR_BREAK(firstName);
        GET_VALID_LINE_OR_BREAK(lastName);
        GET_VALID_LINE_OR_BREAK(email);
        GET_VALID_LINES(message);
        e = new Licq::EventAuthRequest(UserId(accountId, myPpid), alias,
            firstName, lastName, email, message, tTime, nFlags);
        break;
      }
      case Licq::UserEvent::TypeAuthGranted:
      {
        string accountId, message;
        GET_VALID_LINE_OR_BREAK(accountId);
        GET_VALID_LINES(message);
        e = new Licq::EventAuthGranted(UserId(accountId, myPpid), message,
            tTime, nFlags);
        break;
      }
      case Licq::UserEvent::TypeAuthRefused:
      {
        string accountId, message;
        GET_VALID_LINE_OR_BREAK(accountId);
        GET_VALID_LINES(message);
        e = new Licq::EventAuthRefused(UserId(accountId, myPpid), message,
            tTime, nFlags);
        break;
      }
      case Licq::UserEvent::TypeAdded:
      {
        string accountId, alias, firstName, lastName, email;
        GET_VALID_LINE_OR_BREAK(accountId);
        GET_VALID_LINE_OR_BREAK(alias);
        GET_VALID_LINE_OR_BREAK(firstName);
        GET_VALID_LINE_OR_BREAK(lastName);
        GET_VALID_LINE_OR_BREAK(email);
        e = new Licq::EventAdded(UserId(accountId, myPpid), alias, firstName,
            lastName, email, tTime, nFlags);
        break;
      }
      case Licq::UserEvent::TypeWebPanel:
      {
        string name, email, message;
        GET_VALID_LINE_OR_BREAK(name);
        GET_VALID_LINE_OR_BREAK(email);
        GET_VALID_LINES(message);
        e = new Licq::EventWebPanel(name, email, message, tTime, nFlags);
        break;
      }
      case Licq::UserEvent::TypeEmailPager:
      {
        string name, email, message;
        GET_VALID_LINE_OR_BREAK(name);
        GET_VALID_LINE_OR_BREAK(email);
        GET_VALID_LINES(message);
        e = new Licq::EventEmailPager(name, email, message, tTime, nFlags);
        break;
      }
      case Licq::UserEvent::TypeContactList:
      {
        Licq::EventContactList::ContactList vc;
        while (true)
          {
          string accountId, alias;
          GET_VALID_LINE_OR_BREAK(accountId);
          GET_VALID_LINE_OR_BREAK(alias);
          vc.push_back(new Licq::EventContactList::Contact(UserId(accountId, myPpid), alias));
        }
        e = new Licq::EventContactList(vc, false, tTime, nFlags);
        break;
      }
      case Licq::UserEvent::TypeSms:
      {
        string number, message;
        GET_VALID_LINE_OR_BREAK(number);
        GET_VALID_LINES(message);
        e = new Licq::EventSms(number, message, tTime, nFlags);
        break;
      }
      case Licq::UserEvent::TypeMsgServer:
      {
        string name, email, message;
        GET_VALID_LINE_OR_BREAK(name);
        GET_VALID_LINE_OR_BREAK(email);
        GET_VALID_LINES(message);
        e = new Licq::EventServerMessage(name, email, message, tTime);
        break;
      }
      case Licq::UserEvent::TypeEmailAlert:
      {
        string name, email, message;
        GET_VALID_LINE_OR_BREAK(name);
        GET_VALID_LINE_OR_BREAK(email);
        GET_VALID_LINES(message);
        e = new Licq::EventEmailAlert(name, 0, email, message, tTime);
        break;
      }
      default:
        gLog.warning(tr("Corrupt history file (%s): Unknown sub-command 0x%04X."),
            myFilename.c_str(), nSubCommand);
        break;
    }
    if (e != NULL)
    {
      e->SetPending(false);
      lHistory.push_back(e);
    }
    if (szResult == NULL) break;
  }

  // Close the file
  fclose(f);
  return true;
}

void UserHistory::write(const string& buf, bool append)
{
  if (myFilename.empty() || buf.empty())
    return;

  // Make sure history dir exists before trying to write a file in it
  size_t slashPos = myFilename.rfind('/');
  if (slashPos != string::npos)
  {
    string historydir = myFilename.substr(0, slashPos);
    if (mkdir(historydir.c_str(), 0700) == -1 && errno != EEXIST)
    {
      fprintf(stderr, "Couldn't mkdir %s: %s\n", historydir.c_str(), strerror(errno));
      return;
    }
  }

  int fd = open(myFilename.c_str(), O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC), 00600);
  if (fd == -1)
  {
    gLog.error(tr("Unable to open history file (%s): %s."),
        myFilename.c_str(), strerror(errno));
    return;
  }
  ::write(fd, buf.c_str(), buf.size());
  if (append)
    ::write(fd, "\n", 1);
  close(fd);
}

void UserHistory::clear(Licq::HistoryList& hist)
{
  BOOST_FOREACH(Licq::UserEvent* event, hist)
    delete event;

  hist.clear();
}
