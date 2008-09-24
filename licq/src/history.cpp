// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998 - 2003 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cstdio>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string>

// Localization
#include "gettext.h"

#ifdef HAVE_ERRNO_H
#include <errno.h>
#else
extern int errno;
#endif

#include "licq_history.h"
#include "licq_log.h"
#include "licq_constants.h"
#include "licq_file.h"
#include "licq_message.h"
#include "licq_icq.h"
#include "licq_user.h"

//A sleazy hack
extern char *PPIDSTRING(unsigned long);

#define MAX_HISTORY_MSG_SIZE 8192

using std::string;

CUserHistory::CUserHistory()
{
  m_szFileName = m_szDescription = NULL;
}

CUserHistory::~CUserHistory()
{
  if (m_szFileName != NULL) free(m_szFileName);
  if (m_szDescription != NULL) free(m_szDescription);
}

//---SetFile-------------------------------------------------------------------
/*! \brief Sets the name of the history file
 *
 * Obsolete, see CUserHistory::SetFile(const char *, const char *, unsigned long)
 */
void CUserHistory::SetFile(const char *_sz, unsigned long _nUin)
{
  char szUin[24];
  sprintf(szUin, "%lu", _nUin);
  SetFile(_sz, szUin, LICQ_PPID);
}

//---SetFile-------------------------------------------------------------------
/*! \brief Sets the name of the history file
 *
 * Sets the name of the history file. A value "default" for _sz means to
 * use default history name which is <Id>.<Protocol Name>.history. A Value "none" for _sz means
 * no history file. All other values mean to use _sz as the history file name.
 */
void CUserHistory::SetFile(const char *_sz, const char *_szId,
                           unsigned long _nPPID)
{
  if (m_szFileName != NULL) free(m_szFileName);
  if (m_szDescription != NULL) free(m_szDescription);

  // default history filename, compare only first 7 chars in case of spaces
  if (strncmp(_sz, "default", 7) == 0)
  {
    char temp[MAX_FILENAME_LEN];
    char *p = PPIDSTRING(_nPPID);
    snprintf(temp, MAX_FILENAME_LEN, "%s/%s/%s.%s.%s", BASE_DIR,
             HISTORY_DIR, _szId, p, HISTORY_EXT);
    delete [] p;
    temp[sizeof(temp) - 1] = '\0';
    m_szFileName = strdup(temp);
    m_szDescription = strdup("default");
  }
  // no history file
  else if (strncmp(_sz, "none", 4) == 0)  // no history file
  {
    m_szFileName = NULL;
    m_szDescription = strdup("none");
  }
  // use given name
  else
  {
    m_szFileName = strdup(_sz);
    m_szDescription = strdup(_sz);
  }
}



/* szResult[0] != ':' doubles to check if strlen(szResult) < 1 */
#define GET_VALID_LINE_OR_BREAK \
  { \
    if ((szResult = fgets(sz, MAX_LINE_LEN, f)) == NULL || szResult[0] != ':') \
      break; \
    szResult[strlen(szResult) - 1] = '\0'; \
  }

#define GET_VALID_LINES \
  { \
    unsigned short nPos = 0; \
    while ((szResult = fgets(sz, MAX_LINE_LEN, f)) != NULL && sz[0] == ':') \
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
    while ((szResult = fgets(sz, MAX_LINE_LEN, f)) != NULL && sz[0] == ':') ; \
  }

#define SKIP_LINE \
  { \
	  szResult = fgets(sz, MAX_LINE_LEN, f); \
  }

bool CUserHistory::Load(HistoryList &lHistory) const
{
  if (m_szFileName == NULL)
  {
    return false;
  }

  FILE *f = fopen(m_szFileName, "r");
  if (f == NULL)
  {
    if (errno == ENOENT)
    {
      return true;
    }
    else
    {
      gLog.Warn(tr("%sUnable to open history file (%s):\n%s%s.\n"), L_WARNxSTR,
              m_szFileName, L_BLANKxSTR, strerror(errno));
      return false;
    }
  }

  // Now read in a line at a time
  char sz[MAX_LINE_LEN], *szResult, szMsg[MAX_HISTORY_MSG_SIZE + 1];
  unsigned long nFlags;
  unsigned short nCommand, nSubCommand;
  time_t tTime;
  char cDir;
  CUserEvent *e;
  szResult = fgets(sz, MAX_LINE_LEN, f);
  while(true)
  {
    while (szResult != NULL && sz[0] != '[')
      szResult = fgets(sz, MAX_LINE_LEN, f);
    if (szResult == NULL) break;
    // Zero unused part of sz to avoid interpreting garbage if sz is too
    // short
    memset(sz + strlen(sz) + 1, '\0', MAX_LINE_LEN - strlen(sz) - 1);
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
        ConstFileList filelist;
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
      GET_VALID_LINE_OR_BREAK;
      char *szAlias = strdup(&szResult[1]);
      GET_VALID_LINE_OR_BREAK;
      char *szFName = strdup(&szResult[1]);
      GET_VALID_LINE_OR_BREAK;
      char *szLName = strdup(&szResult[1]);
      GET_VALID_LINE_OR_BREAK;
      char *szEmail = strdup(&szResult[1]);
      GET_VALID_LINES;
        e = new CEventAuthRequest(id, LICQ_PPID, szAlias, szFName, szLName,
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
      GET_VALID_LINES;
        e = new CEventAuthGranted(id, LICQ_PPID, szMsg, nCommand, tTime, nFlags);
        free(id);
      break;
    }
    case ICQ_CMDxSUB_AUTHxREFUSED:
    {
      GET_VALID_LINE_OR_BREAK;
        char* id = strdup(&szResult[1]);
      GET_VALID_LINES;
        e = new CEventAuthRefused(id, LICQ_PPID, szMsg, nCommand, tTime, nFlags);
        free(id);
      break;
    }
    case ICQ_CMDxSUB_ADDEDxTOxLIST:
    {
      GET_VALID_LINE_OR_BREAK;
        char* id = strdup(&szResult[1]);
      GET_VALID_LINE_OR_BREAK;
      char *szAlias = strdup(&szResult[1]);
      GET_VALID_LINE_OR_BREAK;
      char *szFName = strdup(&szResult[1]);
      GET_VALID_LINE_OR_BREAK;
      char *szLName = strdup(&szResult[1]);
      GET_VALID_LINE_OR_BREAK;
      char *szEmail = strdup(&szResult[1]);
        e = new CEventAdded(id, LICQ_PPID, szAlias, szFName, szLName, szEmail,
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
          vc.push_back(new CContact(id.c_str(), LICQ_PPID, &szResult[1]));
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
                L_WARNxSTR, m_szFileName, nSubCommand);
      break;
    }
    if (e != NULL)
    {
      e->SetDirection(cDir == 'R' ? D_RECEIVER : D_SENDER);
      e->SetPending(false);
      lHistory.push_back(e);
    }
    if (szResult == NULL) break;
  }

  // Close the file
  fclose(f);
  return true;
}

void CUserHistory::Write(const char* buf, bool append)
{
  if (m_szFileName == NULL || buf == NULL) return;

  // Make sure history dir exists before trying to write a file in it
  char historydir[MAX_FILENAME_LEN];
  snprintf(historydir, sizeof(historydir) - 1, "%s/%s", BASE_DIR, HISTORY_DIR);
  if (mkdir(historydir, 0700) == -1 && errno != EEXIST)
  {
    fprintf(stderr, "Couldn't mkdir %s: %s\n", historydir, strerror(errno));
    return;
  }

  int fd = open(m_szFileName, O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC), 00600);
  if (fd == -1)
  {
    gLog.Error("%sUnable to open history file (%s):\n%s%s.\n", L_ERRORxSTR,
               m_szFileName, L_BLANKxSTR, strerror(errno));
    return;
  }
  write(fd, buf, strlen(buf));
  if (append)
    write(fd, "\n", 1);
  close(fd);
}


//---Clear---------------------------------------------------------------------
/*! \brief Clears the history */
void CUserHistory::Clear(HistoryList &hist)
{
  HistoryListIter it = hist.begin();
  while (it != hist.end())
  {
    delete *it;
    ++it;
  }
  hist.erase(hist.begin(), hist.end());
}
