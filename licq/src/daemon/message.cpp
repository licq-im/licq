#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "message.h"
#include "user.h"

//----CUserEvent::constructor---------------------------------------------------
CUserEvent::CUserEvent(unsigned short _nSubCommand, unsigned short _nCommand,
                       unsigned long _nSequence, time_t _tTime,
                       unsigned long _nFlags)
{
   m_nSubCommand = _nSubCommand;
   m_nCommand = _nCommand;
   m_nSequence = _nSequence;
   m_tTime = (_tTime == 0 ? time(NULL) : _tTime);
   m_nFlags = _nFlags;
}


//-----CUserEvent::Time---------------------------------------------------------
const char *CUserEvent::Time(void)
{
  static char s_szTime[32];

  // set the time received: "Mon Apr 27 20:49:08 1998\n"
  strcpy(s_szTime, ctime(&m_tTime));
  s_szTime[16] = '\0';

  return s_szTime;
}


//-----CUserEvent::LicqVersionStr-----------------------------------------------
const char *CUserEvent::LicqVersionStr(void)
{
   static char s_szVersion[8];
   sprintf(s_szVersion, "v0.%d", LicqVersion());
   return (s_szVersion);
}


//-----CUserEvent::destructor---------------------------------------------------
CUserEvent::~CUserEvent(void)
{
   delete[] m_szText;
}


//-----NToNS--------------------------------------------------------------------
char *NToNS(const char *_szOldStr)
{
  //if (_szOldStr == NULL) return NULL;
  char *szNewStr = new char[(strlen(_szOldStr) << 1) + 2];
  unsigned long j = 0, i = 0;
  szNewStr[j++] = ':';
  while (_szOldStr[i] != '\0')
  {
    szNewStr[j++] = _szOldStr[i];
    if (_szOldStr[i] == '\n') szNewStr[j++] = ':';
    i++;
  }
  if (szNewStr[j - 1] == ':') j--;
  szNewStr[j] = '\0';
  return (szNewStr);
}


//-----CUserEvent::AddToHistory-------------------------------------------------
void CUserEvent::AddToHistory(ICQUser *u, direction _nDir)
{
  char *szText = NToNS(Text());
  char szOut[strlen(szText) + 128];
  sprintf(szOut, "[ %c | %04d | %04d | %04d | %ld ]\n"
                 "%s\n"
                 "\n",
          _nDir == D_RECEIVER ? 'R' : 'S',
          m_nSubCommand, m_nCommand, (unsigned short)(m_nFlags >> 16), m_tTime, szText);
  if (u != NULL)
    u->WriteToHistory(szOut);
  else
  {
    ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
    o->WriteToHistory(szOut);
    gUserManager.DropOwner();
  }
  delete [] szText;
}




//=====CEventMsg================================================================

CEventMsg::CEventMsg(const char *_szMessage, unsigned short _nCommand,
                     time_t _tTime, unsigned long _nFlags)
   : CUserEvent(ICQ_CMDxSUB_MSG, _nCommand, 0, _tTime, _nFlags)
{
   m_szMessage = strdup(_szMessage == NULL ? "" : _szMessage);

   m_szText = new char[strlen(m_szMessage) + 1];
   sprintf(m_szText, "%s", m_szMessage);

}


CEventMsg::~CEventMsg(void)
{
   free (m_szMessage);
}


//=====CEventFile===============================================================

CEventFile::CEventFile(const char *_szFilename, const char *_szFileDescription,
                       unsigned long _nFileSize, unsigned long _nSequence,
                       time_t _tTime, unsigned long _nFlags)
   : CUserEvent(ICQ_CMDxSUB_FILE, ICQ_CMDxTCP_START, _nSequence, _tTime, _nFlags)
{
   m_szFilename = strdup(_szFilename);
   m_szFileDescription = strdup(_szFileDescription == NULL ? "" : _szFileDescription);
   m_nFileSize = _nFileSize;

   m_szText = new char[strlen(m_szFilename) + strlen(m_szFileDescription) + 64];
   sprintf(m_szText, "File: %s (%ld bytes)\nDescription: %s", m_szFilename,
           m_nFileSize, m_szFileDescription);

}


CEventFile::~CEventFile(void)
{
   free (m_szFilename);
   free (m_szFileDescription);
}


//=====CEventFileCancel=========================================================

CEventFileCancel::CEventFileCancel(unsigned long _nSequence, time_t _tTime,
                                   unsigned long _nFlags)
   : CUserEvent(ICQ_CMDxSUB_FILE, ICQ_CMDxTCP_CANCEL, _nSequence, _tTime, _nFlags)
{
   m_szText = new char[32];
   sprintf(m_szText, "Transfer cancelled.");

}


CEventFileCancel::~CEventFileCancel(void)
{
   // Do nothing
}



//=====CEventUrl================================================================

CEventUrl::CEventUrl(const char *_szUrl, const char *_szUrlDescription,
                     unsigned short _nCommand, time_t _tTime,
                     unsigned long _nFlags)
   : CUserEvent(ICQ_CMDxSUB_URL, _nCommand, 0, _tTime, _nFlags)
{
   m_szUrl = strdup(_szUrl == NULL ? "" : _szUrl);
   m_szUrlDescription = strdup(_szUrlDescription == NULL ? "" : _szUrlDescription);

   m_szText = new char[strlen(m_szUrl) + strlen(m_szUrlDescription) + 64];
   sprintf(m_szText, "URL: %s\nDescription: %s", m_szUrl, m_szUrlDescription);
}


CEventUrl::~CEventUrl(void)
{
   free (m_szUrl);
   free (m_szUrlDescription);
}


//=====CEventChat===============================================================

CEventChat::CEventChat(const char *_szReason, unsigned long _nSequence,
                       time_t _tTime, unsigned long _nFlags)
   : CUserEvent(ICQ_CMDxSUB_CHAT, ICQ_CMDxTCP_START, _nSequence, _tTime, _nFlags)
{
  m_szReason = strdup(_szReason ==  NULL ? "" : _szReason);

  m_szText = new char[strlen(m_szReason) + 16];
  sprintf(m_szText, "Reason: %s", m_szReason);

}


CEventChat::~CEventChat(void)
{
   free (m_szReason);
}



//=====CEventChatCancel=========================================================

CEventChatCancel::CEventChatCancel(unsigned long _nSequence, time_t _tTime,
                                   unsigned long _nFlags)
   : CUserEvent(ICQ_CMDxSUB_CHAT, ICQ_CMDxTCP_CANCEL, _nSequence, _tTime,
                _nFlags)
{
   m_szText = new char[32];
   sprintf(m_szText, "Request cancelled.");
}


CEventChatCancel::~CEventChatCancel(void)
{
   // Do nothing
}


//=====CEventAdded==============================================================
CEventAdded::CEventAdded(unsigned long _nUin, const char *_szAlias,
                         const char *_szFirstName,const char *_szLastName,
                         const char *_szEmail, unsigned short _nCommand,
                         time_t _tTime, unsigned long _nFlags)
   : CUserEvent(ICQ_CMDxSUB_ADDEDxTOxLIST, _nCommand, 0, _tTime, _nFlags)
{
   m_szAlias = strdup(_szAlias);
   m_szFirstName = strdup(_szFirstName);
   m_szLastName = strdup(_szLastName);
   m_szEmail = strdup(_szEmail);
   m_nUin = _nUin;

   m_szText = new char[strlen(_szAlias) + strlen(_szFirstName) +
                       strlen(_szLastName) + strlen(_szEmail) + 128];
   sprintf(m_szText, "%s (%s %s, %s), uin %ld, added you to their contact list.",
           _szAlias, _szFirstName, _szLastName, _szEmail, _nUin);
}


CEventAdded::~CEventAdded(void)
{
   free (m_szAlias);
   free (m_szFirstName);
   free (m_szLastName);
   free (m_szEmail);
}


//=====CEventAuthReq===============================================================
CEventAuthReq::CEventAuthReq(unsigned long _nUin, const char *_szAlias,
                       const char *_szFirstName,const char *_szLastName,
                       const char *_szEmail, const char *_szReason,
                       unsigned short _nCommand, time_t _tTime,
                       unsigned long _nFlags)
   : CUserEvent(ICQ_CMDxSUB_REQxAUTH, _nCommand, 0, _tTime, _nFlags)
{
   m_szAlias = strdup(_szAlias);
   m_szFirstName = strdup(_szFirstName);
   m_szLastName = strdup(_szLastName);
   m_szEmail = strdup(_szEmail);
   m_szReason = strdup(_szReason);
   m_nUin = _nUin;

   m_szText = new char[strlen(_szAlias) + strlen(_szFirstName)
                       + strlen(_szLastName) + strlen(_szEmail)
                       + strlen(_szReason) + 128];
   sprintf(m_szText, "%s (%s %s, %s), uin %ld, requests authorization to add you to their contact list:\n%s",
           _szAlias, _szFirstName, _szLastName, _szEmail, _nUin, _szReason);
}


CEventAuthReq::~CEventAuthReq(void)
{
   free (m_szAlias);
   free (m_szFirstName);
   free (m_szLastName);
   free (m_szEmail);
   free (m_szReason);
}


//=====CEventAuth===============================================================
CEventAuth::CEventAuth(unsigned long _nUin, const char *_szMessage,
                       unsigned short _nCommand, time_t _tTime,
                       unsigned long _nFlags)
   : CUserEvent(ICQ_CMDxSUB_REQxAUTH, _nCommand, 0, _tTime, _nFlags)
{
  if (_szMessage == NULL)
    m_szMessage = strdup("");
  else
    m_szMessage = strdup(_szMessage);
  m_nUin = _nUin;

  m_szText = new char[strlen(m_szMessage) + 128];
  sprintf(m_szText, "Uin %ld has authorized you:\n%s\n",
          _nUin, _szMessage);
}


CEventAuth::~CEventAuth(void)
{
  free (m_szMessage);
}



//====CEventHtmlPanel===========================================================
CEventWebPanel::CEventWebPanel(const char *_szName, char *_szEmail,
                               const char *_szMessage, unsigned short _nCommand,
                               time_t _tTime, unsigned long _nFlags)
   : CUserEvent(ICQ_CMDxSUB_WEBxPANEL, _nCommand, 0, _tTime, _nFlags)
{
  m_szName = strdup(_szName);
  m_szEmail = strdup(_szEmail);
  m_szMessage = strdup(_szMessage);

  m_szText = new char[strlen(_szName) + strlen(_szEmail) + strlen(_szMessage) + 64];
  sprintf(m_szText, "Message from %s (%s) through web panel:\n%s",
          _szName, _szEmail, _szMessage);
}


CEventWebPanel::~CEventWebPanel(void)
{
  free (m_szName);
  free (m_szEmail);
  free (m_szMessage);
}


//====CEventEmailPager==========================================================
CEventEmailPager::CEventEmailPager(const char *_szName, char *_szEmail,
                                   const char *_szMessage, unsigned short _nCommand,
                                   time_t _tTime, unsigned long _nFlags)
   : CUserEvent(ICQ_CMDxSUB_EMAILxPAGER, _nCommand, 0, _tTime, _nFlags)
{
  m_szName = strdup(_szName);
  m_szEmail = strdup(_szEmail);
  m_szMessage = strdup(_szMessage);

  m_szText = new char[strlen(_szName) + strlen(_szEmail) + strlen(_szMessage) + 64];
  sprintf(m_szText, "Message from %s (%s) through email pager:\n%s",
          _szName, _szEmail, _szMessage);
}


CEventEmailPager::~CEventEmailPager(void)
{
  free (m_szName);
  free (m_szEmail);
  free (m_szMessage);
}


//====CEventContactList========================================================
CEventContactList::CEventContactList(vector <char *> &_vszFields,
                                     unsigned short _nCommand,
                                     time_t _tTime, unsigned long _nFlags)
  : CUserEvent(ICQ_CMDxSUB_CONTACTxLIST, _nCommand, 0, _tTime, _nFlags)
{
  for (unsigned short i = 0; i < _vszFields.size(); i++)
    m_vszFields.push_back(strdup(_vszFields[i]));

  m_szText = new char [m_vszFields.size() * 16 + 128];
  sprintf(m_szText, "Contact list (%d contacts):\n", m_vszFields.size() / 2);
  char *szEnd;
  for (unsigned short i = 0; i < m_vszFields.size(); i += 2)
  {
    szEnd = &m_szText[strlen(m_szText)];
    sprintf(szEnd, "%s (%s)\n", m_vszFields[i + 1], m_vszFields[i]);
  }

}


CEventContactList::~CEventContactList(void)
{
  for (unsigned short i = 0; i < m_vszFields.size(); i++)
    free(m_vszFields[i]);
}


//=====CEventSaved==============================================================
CEventSaved::CEventSaved(unsigned short _nNumEvents)
   : CUserEvent(0, 0, 0, 0, 0)
{
   m_nNumEvents = _nNumEvents;
   m_szText = new char[64];
   sprintf(m_szText, "[%d unviewed messages saved in history]", m_nNumEvents);
}


CEventUnknownSysMsg::CEventUnknownSysMsg(unsigned short _nSubCommand,
                             unsigned short _nCommand, unsigned long _nUin,
                             const char *_szMsg, unsigned long _nSequence,
                             time_t _tTime, unsigned long _nFlags)
   : CUserEvent(_nSubCommand, _nCommand, _nSequence, _tTime, _nFlags)
{
  if (_szMsg == NULL)
    m_szMsg = strdup("");
  else
    m_szMsg = strdup(_szMsg);
  m_nUin = _nUin;

  m_szText = new char [strlen(m_szMsg) + 128];
  sprintf(m_szText, "Unknown system message (0x%04X) from %ld:\n%s\n",
          m_nSubCommand, m_nUin, m_szMsg);
}


CEventUnknownSysMsg::~CEventUnknownSysMsg(void)
{
  free(m_szMsg);
}


