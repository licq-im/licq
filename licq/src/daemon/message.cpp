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
   m_tTime = _tTime;
   m_nFlags = _nFlags;
}


//-----CUserEvent::Time---------------------------------------------------------
const char *CUserEvent::Time(void)
{
  static char s_szTime[32];

  // set the time received: "Mon Apr 27 20:49:08 1998\n"
  if (m_tTime == 0) m_tTime = time(NULL);
  strcpy(s_szTime, ctime(&m_tTime));
  // Remove the trailing newline
  //s_szTime[24] = '\0';
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
   free (m_szDescription);
}


//-----CUserEvent::AddToHistory-------------------------------------------------
void CUserEvent::AddToHistory(ICQUser *u, direction _nDir)
{
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  char szOut[strlen(Text()) + 512];
  switch (_nDir)
  {
  case D_RECEIVER:
    /*sprintf(szOut, "From %s  %s\n"
                   "From: %s <%s>\n"
                   "To: %s <%s>\n"
                   "Date: %s\n"
                   "Subject: %s\n"
                   "Content-Length: %d\n\n%s\n\n",
            u == NULL ? "Server" : u->getEmail(), Time(),
            u->getAlias(), u->getEmail(),
            o->getAlias(), o->getEmail(),
            Time(), Description(), strlen(Text()), Text());*/
    sprintf(szOut, "%s -> %s (%s): %s\n%s\n--------------------\n",
            u == NULL ? "Server" : u->getAlias(), o->getAlias(),
            Time(), Description(), Text());
    if (u != NULL)
      u->WriteToHistory(szOut);
    else
    {
      gUserManager.DropOwner();
      o = gUserManager.FetchOwner(LOCK_W);
      o->WriteToHistory(szOut);
    }
    break;

  case D_SENDER:
    /*sprintf(szOut, "From %s  %s\n"
                   "From: %s <%s>\n"
                   "To: %s <%s>\n"
                   "Date: %s\n"
                   "Subject: %s\n"
                   "Content-Length: %d\n\n%s\n\n",
              u == NULL ? "Server" : o->getEmail(), Time(),
              o->getAlias(), o->getEmail(),
              u->getAlias(), u->getEmail(),
              Time(), Description(), strlen(Text()), Text());*/
    sprintf(szOut, "%s -> %s (%s): %s\n%s\n--------------------\n",
            o->getAlias(), u == NULL ? "Server" : u->getAlias(),
            Time(), Description(), Text());
    if (u != NULL)
      u->WriteToHistory(szOut);
    else
    {
      gUserManager.DropOwner();
      o = gUserManager.FetchOwner(LOCK_W);
      o->WriteToHistory(szOut);
    }
    break;
  }
  gUserManager.DropOwner();
}




//=====CEventMsg================================================================

CEventMsg::CEventMsg(const char *_szMessage, unsigned short _nCommand,
                     time_t _tTime, unsigned long _nFlags)
   : CUserEvent(ICQ_CMDxSUB_MSG, _nCommand, 0, _tTime, _nFlags)
{
   m_szMessage = strdup(_szMessage == NULL ? "" : _szMessage);

   m_szText = new char[strlen(m_szMessage) + 1];
   sprintf(m_szText, "%s", m_szMessage);
   m_szDescription = strdup("Message");

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
   m_szDescription = strdup("File Transfer");

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
   m_szDescription = strdup("File Transfer Cancel");

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
   m_szDescription = strdup("URL");
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
  m_szDescription = strdup("Chat Request");

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
   m_szDescription = strdup("Chat Request Cancel");
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
   m_szDescription = strdup("Added to Contact List");
}


CEventAdded::~CEventAdded(void)
{
   free (m_szAlias);
   free (m_szFirstName);
   free (m_szLastName);
   free (m_szEmail);
}


//=====CEventAuth===============================================================
CEventAuth::CEventAuth(unsigned long _nUin, const char *_szAlias, 
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
   m_szDescription = strdup("Authorization Request");
}


CEventAuth::~CEventAuth(void)
{
   free (m_szAlias);
   free (m_szFirstName);
   free (m_szLastName);
   free (m_szEmail);
   free (m_szReason);
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
  m_szDescription = strdup("Web Panel");
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
  m_szDescription = strdup("Email Pager");
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
  m_szDescription = strdup("Contact List");

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
   m_szDescription = strdup("Saved Messages");
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
  m_szDescription = strdup("Unknown System Message");
}


CEventUnknownSysMsg::~CEventUnknownSysMsg(void)
{
  free(m_szMsg);
}


