// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998 - 2009 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
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

#include "licq_log.h"
#include "licq_constants.h"
#include "licq_message.h"
#include <licq/icqdefines.h>
#include <licq/userid.h>

#include "../gettext.h"

#define MAX_HISTORY_MSG_SIZE 8192

using namespace std;
using Licq::UserId;
using LicqDaemon::UserHistory;

UserHistory::UserHistory()
{
}

UserHistory::~UserHistory()
{
}

void UserHistory::setFile(const string& filename, const UserId& userId)
{
  // default history filename, compare only first 7 chars in case of spaces
  if (filename == "default")
  {
    char p[5];
    Licq::protocolId_toStr(p, userId.protocolId());
    myFilename = BASE_DIR;
    myFilename += HISTORY_DIR;
    myFilename += '/';
    myFilename += userId.accountId().c_str();
    myFilename += '.';
    myFilename += p;
    myFilename += '.';
    myFilename += HISTORY_EXT;
    myDescription = "default";
  }
  // no history file
  else if (filename == "none")
  {
    myFilename = "";
    myDescription = "none";
  }
  // use given name
  else
  {
    myFilename = filename;
    myDescription = filename;
  }
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
      gLog.Warn(tr("%sUnable to open history file (%s):\n%s%s.\n"), L_WARNxSTR,
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
  CUserEvent *e;
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
      e = new CEventMsg(szMsg, nCommand, tTime, nFlags);
      break;
    }
    case ICQ_CMDxSUB_CHAT:
    {
      if (nCommand != ICQ_CMDxTCP_CANCEL)
      {
        GET_VALID_LINES;
        e = new CEventChat(szMsg, 0, tTime, nFlags);
      }
      else
      {
        SKIP_VALID_LINES;
        //e = new CEventChatCancel(0, tTime, nFlags);
      }
      break;
    }
    case ICQ_CMDxSUB_FILE:
    {
      if (nCommand != ICQ_CMDxTCP_CANCEL)
      {
        GET_VALID_LINE_OR_BREAK;
        char *szFile = strdup(&szResult[1]);
        GET_VALID_LINE_OR_BREAK;
        unsigned long nSize = atoi(&szResult[1]);
        GET_VALID_LINES;
        list<string> filelist;
        filelist.push_back(szFile);
        e = new CEventFile(szFile, szMsg, nSize, filelist, 0, tTime, nFlags);
        free(szFile);
      }
      else
      {
        SKIP_VALID_LINES;
        //e = new CEventFileCancel(0, tTime, nFlags);
      }
      break;
    }
    case ICQ_CMDxSUB_URL:
    {
      GET_VALID_LINE_OR_BREAK;
      char *szUrl = strdup(&szResult[1]);
      GET_VALID_LINES;
      e = new CEventUrl(szUrl, szMsg, nCommand, tTime, nFlags);
      free(szUrl);
      break;
    }
    case ICQ_CMDxSUB_AUTHxREQUEST:
    {
      GET_VALID_LINE_OR_BREAK;
        char* id = strdup(&szResult[1]);
        UserId userId(id, LICQ_PPID);
      GET_VALID_LINE_OR_BREAK;
      char *szAlias = strdup(&szResult[1]);
      GET_VALID_LINE_OR_BREAK;
      char *szFName = strdup(&szResult[1]);
      GET_VALID_LINE_OR_BREAK;
      char *szLName = strdup(&szResult[1]);
      GET_VALID_LINE_OR_BREAK;
      char *szEmail = strdup(&szResult[1]);
      GET_VALID_LINES;
        e = new CEventAuthRequest(userId, szAlias, szFName, szLName,
            szEmail, szMsg, nCommand, tTime, nFlags);
        free(id);
      free(szAlias);
      free(szFName);
      free(szLName);
      free(szEmail);
      break;
    }
    case ICQ_CMDxSUB_AUTHxGRANTED:
    {
      GET_VALID_LINE_OR_BREAK;
        char* id = strdup(&szResult[1]);
        UserId userId(id, LICQ_PPID);
      GET_VALID_LINES;
        e = new CEventAuthGranted(userId, szMsg, nCommand, tTime, nFlags);
        free(id);
      break;
    }
    case ICQ_CMDxSUB_AUTHxREFUSED:
    {
      GET_VALID_LINE_OR_BREAK;
        char* id = strdup(&szResult[1]);
        UserId userId(id, LICQ_PPID);
      GET_VALID_LINES;
        e = new CEventAuthRefused(userId, szMsg, nCommand, tTime, nFlags);
        free(id);
      break;
    }
    case ICQ_CMDxSUB_ADDEDxTOxLIST:
    {
      GET_VALID_LINE_OR_BREAK;
        char* id = strdup(&szResult[1]);
        UserId userId(id, LICQ_PPID);
      GET_VALID_LINE_OR_BREAK;
      char *szAlias = strdup(&szResult[1]);
      GET_VALID_LINE_OR_BREAK;
      char *szFName = strdup(&szResult[1]);
      GET_VALID_LINE_OR_BREAK;
      char *szLName = strdup(&szResult[1]);
      GET_VALID_LINE_OR_BREAK;
      char *szEmail = strdup(&szResult[1]);
        e = new CEventAdded(userId, szAlias, szFName, szLName, szEmail,
                            nCommand, tTime, nFlags);
        free(id);
      free(szAlias);
      free(szFName);
      free(szLName);
      free(szEmail);
      break;
    }
    case ICQ_CMDxSUB_WEBxPANEL:
    {
      GET_VALID_LINE_OR_BREAK;
      char *szName = strdup(&szResult[1]);
      GET_VALID_LINE_OR_BREAK;
      char *szEmail = strdup(&szResult[1]);
      GET_VALID_LINES;
      e = new CEventWebPanel(szName, szEmail, szMsg,
                             nCommand, tTime, nFlags);
      free(szName);
      free(szEmail);
      break;
    }
    case ICQ_CMDxSUB_EMAILxPAGER:
    {
      GET_VALID_LINE_OR_BREAK;
      char *szName = strdup(&szResult[1]);
      GET_VALID_LINE_OR_BREAK;
      char *szEmail = strdup(&szResult[1]);
      GET_VALID_LINES;
      e = new CEventWebPanel(szName, szEmail, szMsg,
                             nCommand, tTime, nFlags);
      free(szName);
      free(szEmail);
      break;
    }
    case ICQ_CMDxSUB_CONTACTxLIST:
    {
      ContactList vc;
      bool b = true;
      string id;
      while (true)
      {
        GET_VALID_LINE_OR_BREAK;
        if (b)
          id = &szResult[1];
        else if (!id.empty())
          {
            UserId userId(id, LICQ_PPID);
            vc.push_back(new CContact(userId, &szResult[1]));
          }
        b = !b;
      }
      e = new CEventContactList(vc, false, nCommand, tTime, nFlags);
      break;
    }
    case ICQ_CMDxSUB_SMS:
    {
      GET_VALID_LINE_OR_BREAK;
      char *szNum = strdup(&szResult[1]);
      GET_VALID_LINES;
      e = new CEventSms(szNum, szMsg, nCommand, tTime, nFlags);
      free(szNum);
      break;
    }
    case ICQ_CMDxSUB_MSGxSERVER:
    {
      GET_VALID_LINE_OR_BREAK;
      char *szName = strdup(&szResult[1]);
      SKIP_LINE;
      const char *szEmail = "";
      GET_VALID_LINES;
      e = new CEventServerMessage(szName, szEmail, szMsg, tTime);
      free(szName);
      break;
    }
    case ICQ_CMDxSUB_EMAILxALERT:
    {
      GET_VALID_LINE_OR_BREAK;
      char *szName = strdup(&szResult[1]);
      GET_VALID_LINE_OR_BREAK;
      char *szEmail = strdup(&szResult[1]);
      GET_VALID_LINES;
      e = new CEventEmailAlert(szName, 0, szEmail, szMsg, tTime);
      free(szName);
      free(szEmail);
      break;
    }
    default:
      gLog.Warn(tr("%sCorrupt history file (%s): Unknown sub-command 0x%04X.\n"),
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
  string historydir = BASE_DIR;
  historydir += HISTORY_DIR;
  if (mkdir(historydir.c_str(), 0700) == -1 && errno != EEXIST)
  {
    fprintf(stderr, "Couldn't mkdir %s: %s\n", historydir.c_str(), strerror(errno));
    return;
  }

  int fd = open(myFilename.c_str(), O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC), 00600);
  if (fd == -1)
  {
    gLog.Error("%sUnable to open history file (%s):\n%s%s.\n", L_ERRORxSTR,
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
  BOOST_FOREACH(CUserEvent* event, hist)
    delete event;

  hist.clear();
}
