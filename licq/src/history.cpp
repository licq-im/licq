#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

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

#define MAX_HISTORY_MSG_SIZE 8192

CUserHistory::CUserHistory()
{
  m_szFileName = m_szDescription = NULL;
}

CUserHistory::~CUserHistory()
{
  if (m_szFileName != NULL) free(m_szFileName);
  if (m_szDescription != NULL) free(m_szDescription);
}

void CUserHistory::SetFile(const char *_sz, unsigned long _nUin)
{
  if (m_szFileName != NULL) free(m_szFileName);
  if (m_szDescription != NULL) free(m_szDescription);

  // default history filename, compare only first 7 chars in case of spaces
  if (strncmp(_sz, "default", 7) == 0)
  {
    char temp[MAX_FILENAME_LEN];
    snprintf(temp, MAX_FILENAME_LEN, "%s/%s/%ld.%s", BASE_DIR, HISTORY_DIR, _nUin, HISTORY_EXT);
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

#define GET_VALID_LINE_OR_BREAK \
  { \
    if ( (szResult = fgets(sz, MAX_LINE_LEN, f)) == NULL || szResult[0] != ':') break; \
    szResult[strlen(szResult) - 1] = '\0'; \
  }

#define GET_VALID_LINES \
  { \
    unsigned short nPos = 0; \
    while ( (szResult = fgets(sz, MAX_LINE_LEN, f)) != NULL && sz[0] == ':') \
    { \
      if (nPos < MAX_HISTORY_MSG_SIZE) \
      { \
        int len = strlen(sz+1); \
        if(len+1 > MAX_HISTORY_MSG_SIZE - nPos)  len = MAX_HISTORY_MSG_SIZE - nPos; \
        strncpy(&szMsg[nPos], sz+1, len); \
        nPos += len; \
        szMsg[nPos-1] = '\0'; \
        /* nPos += snprintf(&szMsg[nPos], MAX_HISTORY_MSG_SIZE - nPos, "%s", &sz[1]); */ \
      } \
    } \
  }

bool CUserHistory::Load(HistoryList &lHistory)
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
      gLog.Warn("%sUnable to open history file (%s):\n%s%s.\n", L_WARNxSTR,
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
    switch(nSubCommand)
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
        while ( (szResult = fgets(sz, MAX_LINE_LEN, f)) != NULL && sz[0] == ':');
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
        e = new CEventFile(szFile, szMsg, nSize, 0, tTime, nFlags);
        free(szFile);
      }
      else
      {
        while ( (szResult = fgets(sz, MAX_LINE_LEN, f)) != NULL && sz[0] == ':');
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
      unsigned long nUin = atoi(&szResult[1]);
      GET_VALID_LINE_OR_BREAK;
      char *szAlias = strdup(&szResult[1]);
      GET_VALID_LINE_OR_BREAK;
      char *szFName = strdup(&szResult[1]);
      GET_VALID_LINE_OR_BREAK;
      char *szLName = strdup(&szResult[1]);
      GET_VALID_LINE_OR_BREAK;
      char *szEmail = strdup(&szResult[1]);
      GET_VALID_LINES;
      e = new CEventAuthRequest(nUin, szAlias, szFName, szLName, szEmail, szMsg,
                            nCommand, tTime, nFlags);
      free(szAlias);
      free(szFName);
      free(szLName);
      free(szEmail);
      break;
    }
    case ICQ_CMDxSUB_AUTHxGRANTED:
    {
      GET_VALID_LINE_OR_BREAK;
      unsigned long nUin = atoi(&szResult[1]);
      GET_VALID_LINES;
      e = new CEventAuthGranted(nUin, szMsg, nCommand, tTime, nFlags);
      break;
    }
    case ICQ_CMDxSUB_AUTHxREFUSED:
    {
      GET_VALID_LINE_OR_BREAK;
      unsigned long nUin = atoi(&szResult[1]);
      GET_VALID_LINES;
      e = new CEventAuthRefused(nUin, szMsg, nCommand, tTime, nFlags);
      break;
    }
    case ICQ_CMDxSUB_ADDEDxTOxLIST:
    {
      GET_VALID_LINE_OR_BREAK;
      unsigned long nUin = atoi(&szResult[1]);
      GET_VALID_LINE_OR_BREAK;
      char *szAlias = strdup(&szResult[1]);
      GET_VALID_LINE_OR_BREAK;
      char *szFName = strdup(&szResult[1]);
      GET_VALID_LINE_OR_BREAK;
      char *szLName = strdup(&szResult[1]);
      GET_VALID_LINE_OR_BREAK;
      char *szEmail = strdup(&szResult[1]);
      e = new CEventAdded(nUin, szAlias, szFName, szLName, szEmail,
                            nCommand, tTime, nFlags);
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
      unsigned long nUin = 0;
      while (true)
      {
        GET_VALID_LINE_OR_BREAK;
        if (b)
          nUin = atoi(&szResult[1]);
        else if (nUin != 0)
          vc.push_back(new CContact(nUin, &szResult[1]));
        b = !b;
      }
      e = new CEventContactList(vc, false, nCommand, tTime, nFlags);
      break;
    }
    case ICQ_CMDxSUB_USERxINFO:
      break;
    default:
      gLog.Warn("%sCorrupt history file (%s): Unknown sub-command 0x%04X.\n",
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


void CUserHistory::Save(const char *buf)
{
  if (m_szFileName == NULL || buf == NULL) return;
  int fd = open(m_szFileName, O_WRONLY | O_CREAT | O_TRUNC, 00664);
  if (fd == -1)
  {
    gLog.Error("%sUnable to open history file (%s):\n%s%s.\n", L_ERRORxSTR,
               m_szFileName, L_BLANKxSTR, strerror(errno));
    return;
  }
  write(fd, buf, strlen(buf));
  close(fd);
}


void CUserHistory::Clear(HistoryList &hist)
{
  HistoryListIter it = hist.begin();
  while (it != hist.end())
  {
    delete *it;
    it++;
  }
  hist.erase(hist.begin(), hist.end());
}


void CUserHistory::Append(const char *_sz)
{
  if (m_szFileName == NULL || _sz == NULL) return;
  int fd = open(m_szFileName, O_WRONLY | O_CREAT | O_APPEND, 00664);
  if (fd == -1)
  {
    gLog.Error("%sUnable to open history file (%s):\n%s%s.\n", L_ERRORxSTR,
               m_szFileName, L_BLANKxSTR, strerror(errno));
    return;
  }
  write(fd, _sz, strlen(_sz));
  write(fd, "\n", 1);
  close(fd);
}


