// -*- c-basic-offset: 2 -*-
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
#include <licq/icqdefines.h>
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
#define GET_VALID_LINE_OR_BREAK \
  { \
    if ((szResult = fgets(sz, sizeof(sz), f)) == NULL || szResult[0] != ':') \
      break; \
    szResult[strlen(szResult) - 1] = '\0'; \
  }

#define GET_VALID_LINES \
  { \
    unsigned short nPos = 0; \
    while ((szResult = fgets(sz, sizeof(sz), f)) != NULL && sz[0] == ':') \
    { \
      if (nPos < MAX_HISTORY_MSG_SIZE) \
      { \
        int len = strlen(sz+1); \
        if (len+1 > MAX_HISTORY_MSG_SIZE - nPos) \
          len = MAX_HISTORY_MSG_SIZE - nPos; \
        strncpy(&szMsg[nPos], sz+1, len); \
        nPos += len; \
        szMsg[nPos] = '\0'; \
        /* snprintf(&szMsg[nPos], MAX_HISTORY_MSG_SIZE - nPos,
         *                  "%s", &sz[1]);
         * szMsg[MAX_HISTORY_MSG_SIZE - 1] = '\0';
         * nPos += strlen(szMsg + nPos); */ \
      } \
    } \
    if (nPos > 0 && szMsg[nPos - 1] == '\n') \
      szMsg[nPos - 1] = '\0'; \
  }

#define SKIP_VALID_LINES \
  { \
    while ((szResult = fgets(sz, sizeof(sz), f)) != NULL && sz[0] == ':') ; \
  }

#define SKIP_LINE \
  { \
    szResult = fgets(sz, sizeof(sz), f); \
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
      gLog.warning(tr("%sUnable to open history file (%s):\n%s%s.\n"), L_WARNxSTR,
          myFilename.c_str(), L_BLANKxSTR, strerror(errno));
      return false;
    }
  }

  // Now read in a line at a time
  char sz[4096], *szResult, szMsg[MAX_HISTORY_MSG_SIZE + 1];
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
    // Now read in the message
    szMsg[0] = '\0';
    e = NULL;
    switch (nSubCommand)
    {
    case ICQ_CMDxSUB_MSG:
    {
      GET_VALID_LINES;
        e = new Licq::EventMsg(szMsg, nCommand, tTime, nFlags);
        break;
    }
    case ICQ_CMDxSUB_CHAT:
    {
      if (nCommand != ICQ_CMDxTCP_CANCEL)
      {
        GET_VALID_LINES;
          e = new Licq::EventChat(szMsg, 0, tTime, nFlags);
        }
      else
      {
        SKIP_VALID_LINES;
          //e = new Licq::EventChatCancel(0, tTime, nFlags);
        }
        break;
    }
    case ICQ_CMDxSUB_FILE:
    {
      if (nCommand != ICQ_CMDxTCP_CANCEL)
      {
        GET_VALID_LINE_OR_BREAK;
          string file  = &szResult[1];
        GET_VALID_LINE_OR_BREAK;
        unsigned long nSize = atoi(&szResult[1]);
        GET_VALID_LINES;
        list<string> filelist;
        filelist.push_back(file);
          e = new Licq::EventFile(file, szMsg, nSize, filelist, 0, tTime, nFlags);
      }
      else
      {
        SKIP_VALID_LINES;
          //e = new Licq::EventFileCancel(0, tTime, nFlags);
        }
      break;
    }
    case ICQ_CMDxSUB_URL:
    {
      GET_VALID_LINE_OR_BREAK;
        string url = &szResult[1];
      GET_VALID_LINES;
        e = new Licq::EventUrl(url, szMsg, nCommand, tTime, nFlags);
      break;
    }
    case ICQ_CMDxSUB_AUTHxREQUEST:
    {
      GET_VALID_LINE_OR_BREAK;
        UserId userId(&szResult[1], myPpid);
      GET_VALID_LINE_OR_BREAK;
        string alias = &szResult[1];
      GET_VALID_LINE_OR_BREAK;
        string firstName = &szResult[1];
      GET_VALID_LINE_OR_BREAK;
        string lastName = &szResult[1];
      GET_VALID_LINE_OR_BREAK;
        string email = &szResult[1];
      GET_VALID_LINES;
        e = new Licq::EventAuthRequest(userId, alias, firstName, lastName,
            email, szMsg, nCommand, tTime, nFlags);
      break;
    }
    case ICQ_CMDxSUB_AUTHxGRANTED:
    {
      GET_VALID_LINE_OR_BREAK;
        UserId userId(&szResult[1], myPpid);
      GET_VALID_LINES;
        e = new Licq::EventAuthGranted(userId, szMsg, nCommand, tTime, nFlags);
      break;
    }
    case ICQ_CMDxSUB_AUTHxREFUSED:
    {
      GET_VALID_LINE_OR_BREAK;
        UserId userId(&szResult[1], myPpid);
      GET_VALID_LINES;
        e = new Licq::EventAuthRefused(userId, szMsg, nCommand, tTime, nFlags);
      break;
    }
    case ICQ_CMDxSUB_ADDEDxTOxLIST:
    {
      GET_VALID_LINE_OR_BREAK;
        UserId userId(&szResult[1], myPpid);
      GET_VALID_LINE_OR_BREAK;
        string alias = &szResult[1];
      GET_VALID_LINE_OR_BREAK;
        string firstName = &szResult[1];
      GET_VALID_LINE_OR_BREAK;
        string lastName = &szResult[1];
      GET_VALID_LINE_OR_BREAK;
        string email = &szResult[1];
        e = new Licq::EventAdded(userId, alias, firstName, lastName, email,
                            nCommand, tTime, nFlags);
      break;
    }
    case ICQ_CMDxSUB_WEBxPANEL:
    {
      GET_VALID_LINE_OR_BREAK;
        string name = &szResult[1];
      GET_VALID_LINE_OR_BREAK;
        string email = &szResult[1];
      GET_VALID_LINES;
        e = new Licq::EventWebPanel(name, email, szMsg,
                             nCommand, tTime, nFlags);
      break;
    }
    case ICQ_CMDxSUB_EMAILxPAGER:
    {
      GET_VALID_LINE_OR_BREAK;
        string name = &szResult[1];
      GET_VALID_LINE_OR_BREAK;
        string email = &szResult[1];
      GET_VALID_LINES;
        e = new Licq::EventEmailPager(name, email, szMsg,
                             nCommand, tTime, nFlags);
      break;
    }
    case ICQ_CMDxSUB_CONTACTxLIST:
    {
      Licq::EventContactList::ContactList vc;
      bool b = true;
      string id;
      while (true)
      {
        GET_VALID_LINE_OR_BREAK;
        if (b)
          id = &szResult[1];
        else if (!id.empty())
          {
            UserId userId(id, myPpid);
            vc.push_back(new Licq::EventContactList::Contact(userId, &szResult[1]));
          }
        b = !b;
      }
        e = new Licq::EventContactList(vc, false, nCommand, tTime, nFlags);
        break;
    }
    case ICQ_CMDxSUB_SMS:
    {
      GET_VALID_LINE_OR_BREAK;
        string number = &szResult[1];
      GET_VALID_LINES;
        e = new Licq::EventSms(number, szMsg, nCommand, tTime, nFlags);
      break;
    }
    case ICQ_CMDxSUB_MSGxSERVER:
    {
      GET_VALID_LINE_OR_BREAK;
        string name = &szResult[1];
      SKIP_LINE;
      GET_VALID_LINES;
        e = new Licq::EventServerMessage(name, "", szMsg, tTime);
      break;
    }
    case ICQ_CMDxSUB_EMAILxALERT:
    {
      GET_VALID_LINE_OR_BREAK;
        string name = &szResult[1];
      GET_VALID_LINE_OR_BREAK;
        string email = &szResult[1];
      GET_VALID_LINES;
        e = new Licq::EventEmailAlert(name, 0, email, szMsg, tTime);
      break;
    }
    default:
      gLog.warning(tr("%sCorrupt history file (%s): Unknown sub-command 0x%04X.\n"),
          L_WARNxSTR, myFilename.c_str(), nSubCommand);
      break;
    }
    if (e != NULL)
    {
      e->setIsReceiver(cDir == 'R');
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
    gLog.error("%sUnable to open history file (%s):\n%s%s.\n", L_ERRORxSTR,
        myFilename.c_str(), L_BLANKxSTR, strerror(errno));
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
