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

#include "history.h"
#include "log.h"
#include "constants.h"
#include "file.h"
#include "message.h"
#include "icq-defines.h"

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
    sprintf(temp, "%s/%s/%ld.history", BASE_DIR, HISTORY_DIR, _nUin);
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
    while ( (szResult = fgets(sz, MAX_LINE_LEN, f)) != NULL && sz[0] == ':') \
      strcat(szMsg, &sz[1]); \
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
  char sz[MAX_LINE_LEN], *szResult, szMsg[8192];
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
    sz[10] = sz[17] = sz[24] = '\0';
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
      if (nCommand == ICQ_CMDxTCP_CANCEL)
      {
        while ( (szResult = fgets(sz, MAX_LINE_LEN, f)) != NULL && sz[0] == ':');
        e = new CEventChatCancel(0, tTime, nFlags);
      }
      else
      {
        GET_VALID_LINES;
        e = new CEventChat(szMsg, 0, tTime, nFlags);
      }
      break;
    }
    case ICQ_CMDxSUB_FILE:
    {
      if (nCommand == ICQ_CMDxTCP_CANCEL)
      {
        while ( (szResult = fgets(sz, MAX_LINE_LEN, f)) != NULL && sz[0] == ':');
        e = new CEventFileCancel(0, tTime, nFlags);
      }
      else
      {
        GET_VALID_LINE_OR_BREAK;
        char *szFile = strdup(&szResult[1]);
        GET_VALID_LINE_OR_BREAK;
        unsigned long nSize = atoi(&szResult[1]);
        GET_VALID_LINES;
        e = new CEventFile(szFile, szMsg, nSize, 0, tTime, nFlags);
        free(szFile);
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
    case ICQ_CMDxSUB_REQxAUTH:
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
      e = new CEventAuthReq(nUin, szAlias, szFName, szLName, szEmail, szMsg,
                            nCommand, tTime, nFlags);
      free(szAlias);
      free(szFName);
      free(szLName);
      free(szEmail);
      break;
    }
    case ICQ_CMDxSUB_AUTHORIZED:
    {
      GET_VALID_LINE_OR_BREAK;
      unsigned long nUin = atoi(&szResult[1]);
      GET_VALID_LINES;
      e = new CEventAuth(nUin, szMsg, nCommand, tTime, nFlags);
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
      vector<char *> vsz;
      while (true)
      {
        GET_VALID_LINE_OR_BREAK;
        vsz.push_back(strdup(&szResult[1]));
      }
      e = new CEventContactList(vsz, nCommand, tTime, nFlags);
      for (unsigned short i = 0; i < vsz.size(); i++)
        free(vsz[i]);
      break;
    }
    case ICQ_CMDxSUB_USERxINFO:
      break;
    default:
      gLog.Warn("%sCorrupt history file (%s): Unknown sub-command 0x%04X.\n",
                m_szFileName, nSubCommand);
      break;
    }
    if (e != NULL)
    {
      e->SetDirection(cDir == 'R' ? D_RECEIVER : D_SENDER);
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


