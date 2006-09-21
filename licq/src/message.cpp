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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Localization
#include "gettext.h"

#include "licq_message.h"
#include "licq_user.h"
#include "licq_translate.h"
#include "licq_icqd.h"
#include "licq_gpg.h"
#include "support.h"

#ifdef USE_HEBREW
extern "C" {
extern char *hebrev (char* pszStr);
}
#endif

// Hack
extern char *PPIDSTRING(unsigned long);

int CUserEvent::s_nId = 1;

//----CUserEvent::constructor---------------------------------------------------
CUserEvent::CUserEvent(unsigned short nSubCommand, unsigned short nCommand,
                       unsigned short nSequence, time_t tTime,
                       unsigned long nFlags, unsigned long nConvoId)
{
   // Assigned stuff
   m_nSubCommand = nSubCommand;
   m_nCommand = nCommand;
   m_nSequence = nSequence;
   m_tTime = (tTime == TIME_NOW ? time(NULL) : tTime);
   m_nFlags = nFlags;
   m_nConvoId = nConvoId;
   
   // Initialized stuff
   m_eDir = D_RECEIVER;
   m_szText = NULL;
   m_bPending = true;
   m_nId = s_nId++;
}


void CUserEvent::CopyBase(CUserEvent *e)
{
  m_eDir = e->Direction();
  m_bPending = e->Pending();
  m_nId = e->Id(); // this is new and possibly will cause problems...
  m_sColor.Set(e->Color());
}


const char *CUserEvent::Text()
{
  if (m_szText == NULL)
  {
    CreateDescription();
#ifdef USE_HEBREW
    char *p = hebrev(m_szText);
    strcpy(m_szText, p);
    free(p);
#endif
  }

  return m_szText;
}



//-----CUserEvent::LicqVersionToString------------------------------------------
const char *CUserEvent::LicqVersionToString(unsigned long v)
{
  static char s_szVersion[8];
  if(v % 10)
    sprintf(s_szVersion, "v%lu.%lu.%lu", v / 1000, (v / 10) % 100, v % 10);
  else
    sprintf(s_szVersion, "v%lu.%lu", v / 1000, (v / 10) % 100);
  return (s_szVersion);
}


//-----CUserEvent::LicqVersionStr-----------------------------------------------
const char *CUserEvent::LicqVersionStr()
{
  return (LicqVersionToString(LicqVersion()));
}


//-----CUserEvent::destructor---------------------------------------------------
CUserEvent::~CUserEvent()
{
  if (m_szText)
    delete[] m_szText;
}


//-----NToNS--------------------------------------------------------------------
int AddStrWithColons(char *_szNewStr, const char *_szOldStr)
{
  unsigned long j = 0, i = 0;
  _szNewStr[j++] = ':';
  while (_szOldStr[i] != '\0')
  {
    _szNewStr[j++] = _szOldStr[i];
    if (_szOldStr[i] == '\n') _szNewStr[j++] = ':';
    i++;
  }
  if (j > 1 && _szNewStr[j - 2] == '\n') j--;
  _szNewStr[j] = '\0';
  return (j);
}


//-----CUserEvent::AddToHistory-------------------------------------------------
int CUserEvent::AddToHistory_Header(direction _nDir, char *szOut)
{
  return sprintf(szOut, "[ %c | %04d | %04d | %04d | %lu ]\n",
          _nDir == D_RECEIVER ? 'R' : 'S',
          m_nSubCommand, m_nCommand, (unsigned short)(m_nFlags >> 16), m_tTime);
}


void CUserEvent::AddToHistory_Flush(ICQUser *u, unsigned long nPPID, char *szOut)
{
  if (u != NULL)
    u->WriteToHistory(szOut);
  else
  {
    ICQOwner *o = gUserManager.FetchOwner(nPPID, LOCK_W);
    o->WriteToHistory(szOut);
    gUserManager.DropOwner(nPPID);
  }
}




//=====CEventMsg================================================================

CEventMsg::CEventMsg(const char *_szMessage, unsigned short _nCommand,
                     time_t _tTime, unsigned long _nFlags, unsigned long _nConvoId)
   : CUserEvent(ICQ_CMDxSUB_MSG, _nCommand, 0, _tTime, _nFlags, _nConvoId)
{
  m_szMessage = strdup(_szMessage == NULL ? "" : _szMessage);
  
  if (strstr(m_szMessage, CGPGHelper::pgpSig) == m_szMessage)
    if (char *plaintext = gGPGHelper.Decrypt(m_szMessage)) {
      m_nFlags |= E_ENCRYPTED;
	  free(m_szMessage);
	  m_szMessage = plaintext;
	}
}


void CEventMsg::CreateDescription()
{
  if (m_szText) delete [] m_szText;
  m_szText = new char[strlen(m_szMessage) + 1];
  strcpy(m_szText, m_szMessage);
}


CEventMsg::~CEventMsg()
{
  free (m_szMessage);
}

void CEventMsg::AddToHistory(ICQUser *u, unsigned long _nPPID, direction _nDir)
{
  char *szOut = new char[ (strlen(m_szMessage) << 1) + EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(_nDir, szOut);
  AddStrWithColons(&szOut[nPos], m_szMessage);
  AddToHistory_Flush(u, _nPPID, szOut);
  delete [] szOut;
}


CEventMsg *CEventMsg::Parse(char *sz, unsigned short nCmd, time_t nTime,
  unsigned long nFlags, unsigned long nConvoId)
{
  gTranslator.ServerToClient (sz);
  return new CEventMsg(sz, nCmd, nTime, nFlags, nConvoId);
}






//=====CEventFile===============================================================

CEventFile::CEventFile(const char *_szFilename, const char *_szFileDescription,
                       unsigned long _nFileSize, ConstFileList &_lFileList,
                       unsigned short _nSequence, time_t _tTime,
                       unsigned long _nFlags, unsigned long _nConvoId,
                       unsigned long _nMsgID1, unsigned long _nMsgID2)
   : CUserEvent(ICQ_CMDxSUB_FILE, ICQ_CMDxTCP_START, _nSequence, _tTime, _nFlags, _nConvoId),
     m_lFileList(_lFileList.begin(), _lFileList.end())
{
  m_szFilename = strdup(_szFilename == NULL ? "" : _szFilename);
  m_szFileDescription = strdup(_szFileDescription == NULL ? "" : _szFileDescription);
  m_nFileSize = _nFileSize;
  m_nMsgID[0] = _nMsgID1;
  m_nMsgID[1] = _nMsgID2;
}


void CEventFile::CreateDescription()
{
  if (m_szText) delete [] m_szText;
  m_szText = new char[strlen(m_szFilename) + strlen(m_szFileDescription) + 64];
  sprintf(m_szText, tr("File: %s (%lu bytes)\nDescription:\n%s\n"), m_szFilename,
          m_nFileSize, m_szFileDescription);
}


CEventFile::~CEventFile()
{
   free (m_szFilename);
   free (m_szFileDescription);
}

void CEventFile::AddToHistory(ICQUser *u, unsigned long _nPPID, direction _nDir)
{
  char *szOut = new char[(strlen(m_szFilename) + strlen(m_szFileDescription)) * 2 + 16 + EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(_nDir, szOut);
  nPos += sprintf(&szOut[nPos], ":%s\n", m_szFilename);
  nPos += sprintf(&szOut[nPos], ":%lu\n", m_nFileSize);
  AddStrWithColons(&szOut[nPos], m_szFileDescription);
  AddToHistory_Flush(u, _nPPID, szOut);
  delete [] szOut;
}



//=====CEventUrl================================================================

CEventUrl::CEventUrl(const char *_szUrl, const char *_szUrlDescription,
                     unsigned short _nCommand, time_t _tTime,
                     unsigned long _nFlags, unsigned long _nConvoId)
   : CUserEvent(ICQ_CMDxSUB_URL, _nCommand, 0, _tTime, _nFlags, _nConvoId)
{
   m_szUrl = strdup(_szUrl == NULL ? "" : _szUrl);
   m_szUrlDescription = strdup(_szUrlDescription == NULL ? "" : _szUrlDescription);
}


void CEventUrl::CreateDescription()
{
  if (m_szText) delete [] m_szText;
  m_szText = new char[strlen(m_szUrl) + strlen(m_szUrlDescription) + 64];
  sprintf(m_szText, tr("Url: %s\nDescription:\n%s\n"), m_szUrl, m_szUrlDescription);
}


CEventUrl::~CEventUrl()
{
   free (m_szUrl);
   free (m_szUrlDescription);
}

void CEventUrl::AddToHistory(ICQUser *u, unsigned long _nPPID, direction _nDir)
{
  char *szOut = new char[(strlen(m_szUrlDescription) << 1) +
                   (strlen(m_szUrl) << 1) + EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(_nDir, szOut);
  nPos += sprintf(&szOut[nPos], ":%s\n", m_szUrl);
  AddStrWithColons(&szOut[nPos], m_szUrlDescription);
  AddToHistory_Flush(u, _nPPID, szOut);
  delete [] szOut;
}


CEventUrl *CEventUrl::Parse(char *sz, unsigned short nCmd, time_t nTime,
  unsigned long nFlags, unsigned long nConvoId)
{
  // parse the message into url and url description
  char **szUrl = new char*[2]; // desc, url
  if (!ParseFE(sz, &szUrl, 2))
  {
    delete []szUrl;
    return NULL;
  }

  // translating string with Translation Table
  gTranslator.ServerToClient(szUrl[0]);
  CEventUrl *e = new CEventUrl(szUrl[1], szUrl[0], nCmd, nTime,
    nFlags, nConvoId);
  delete []szUrl;

  return e;
}

//=====CEventChat===============================================================

CEventChat::CEventChat(const char *szReason, unsigned short nSequence,
                       time_t tTime, unsigned long nFlags,
                       unsigned long nConvoId, unsigned long nMsgID1, unsigned long nMsgID2)
   : CUserEvent(ICQ_CMDxSUB_CHAT, ICQ_CMDxTCP_START, nSequence, tTime, nFlags, nConvoId)
{
  m_szReason = strdup(szReason ==  NULL ? "" : szReason);
  m_szClients = NULL;
  m_nPort = 0;
  m_nMsgID[0] = nMsgID1;
  m_nMsgID[1] = nMsgID2;
}

CEventChat::CEventChat(const char *szReason, const char *szClients,
   unsigned short nPort, unsigned short nSequence,
   time_t tTime, unsigned long nFlags, unsigned long nConvoId, unsigned long nMsgID1,
   unsigned long nMsgID2)
   : CUserEvent(ICQ_CMDxSUB_CHAT, ICQ_CMDxTCP_START, nSequence, tTime, nFlags, nConvoId)
{
  m_szReason = strdup(szReason ==  NULL ? "" : szReason);
  if (nPort == 0)
    m_szClients = NULL;
  else
    m_szClients = strdup(szClients);
  m_nPort = nPort;
  m_nMsgID[0] = nMsgID1;
  m_nMsgID[1] = nMsgID2;
}

void CEventChat::CreateDescription()
{
  if (m_szText) delete [] m_szText;
  if (m_szClients == NULL) {
    m_szText = new char [strlen(m_szReason) + 1];
    strcpy(m_szText, m_szReason);
  }
  else
  {
    m_szText = new char[strlen(m_szReason) + strlen(m_szClients) + 128];
    sprintf(m_szText, tr("%s\n--------------------\nMultiparty:\n%s"), m_szReason, m_szClients);
  }
}


CEventChat::~CEventChat()
{
  free(m_szReason);
  if (m_szClients != NULL) free(m_szClients);
}


void CEventChat::AddToHistory(ICQUser *u, unsigned long _nPPID, direction _nDir)
{
  char *szOut = new char[(strlen(Text()) << 1) + EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(_nDir, szOut);
  AddStrWithColons(&szOut[nPos], Text());
  AddToHistory_Flush(u, _nPPID, szOut);
  delete [] szOut;
}


//=====CEventAdded==============================================================
CEventAdded::CEventAdded(const char *_szId, unsigned long _nPPID, const char *_szAlias,
                         const char *_szFirstName,const char *_szLastName,
                         const char *_szEmail, unsigned short _nCommand,
                         time_t _tTime, unsigned long _nFlags)
   : CUserEvent(ICQ_CMDxSUB_ADDEDxTOxLIST, _nCommand, 0, _tTime, _nFlags)
{
  m_szId = strdup(_szId);
  m_szAlias = strdup(_szAlias);
  m_szFirstName = strdup(_szFirstName);
  m_szLastName = strdup(_szLastName);
  m_szEmail = strdup(_szEmail);
  m_nPPID =_nPPID;
  if (_nPPID == LICQ_PPID)
    m_nUin = strtoul(_szId, (char **)NULL, 10);
  else
    m_nUin = 0;
}

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

  char szUin[24];
  sprintf(szUin, "%lu", _nUin);
  m_szId = strdup(szUin);
  m_nPPID = LICQ_PPID;
}

void CEventAdded::CreateDescription()
{
  if (m_szText) delete [] m_szText;
  char *p = PPIDSTRING(m_nPPID);
  m_szText = new char[strlen(m_szAlias) + strlen(m_szFirstName) +
                      strlen(m_szLastName) + strlen(m_szEmail) +
                      strlen(m_szId) + strlen(p) + 512];
  sprintf(m_szText, tr("Alias: %s\nUser: %s (%s)\nName: %s %s\nEmail: %s\n"),
    m_szAlias, m_szId, p, m_szFirstName, m_szLastName, m_szEmail);
  delete [] p;
}


CEventAdded::~CEventAdded()
{
  free (m_szAlias);
  free (m_szFirstName);
  free (m_szLastName);
  free (m_szEmail);
  free (m_szId);
}

void CEventAdded::AddToHistory(ICQUser *u, unsigned long _nPPID, direction _nDir)
{
  char *p = PPIDSTRING(m_nPPID);
  char *szOut = new char[(strlen(m_szAlias) + strlen(m_szFirstName) +
                    strlen(m_szLastName) + strlen(m_szEmail) + strlen(m_szId) +
                    strlen(p)) * 2 + 20 + EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(_nDir, szOut);
  nPos += sprintf(&szOut[nPos], ":%s (%s)\n:%s\n:%s\n:%s\n:%s\n", m_szId,
                  p, m_szAlias, m_szFirstName, m_szLastName, m_szEmail);
  delete [] p;
  AddToHistory_Flush(u, m_nPPID, szOut);
  delete [] szOut;
}



//=====CEventAuthReq===============================================================
CEventAuthRequest::CEventAuthRequest(const char *_szId, unsigned long _nPPID,
                       const char *_szAlias, const char *_szFirstName,
                       const char *_szLastName, const char *_szEmail,
                       const char *_szReason, unsigned short _nCommand,
                       time_t _tTime, unsigned long _nFlags)
   : CUserEvent(ICQ_CMDxSUB_AUTHxREQUEST, _nCommand, 0, _tTime, _nFlags)
{
   m_szId = strdup(_szId);
   m_szAlias = strdup(_szAlias);
   m_szFirstName = strdup(_szFirstName);
   m_szLastName = strdup(_szLastName);
   m_szEmail = strdup(_szEmail);
   m_szReason = strdup(_szReason);
   m_nPPID = _nPPID;
   if (_nPPID == LICQ_PPID)
    m_nUin = strtoul(_szId, (char **)NULL, 10);
   else
    m_nUin = 0;
}

CEventAuthRequest::CEventAuthRequest(unsigned long _nUin, const char *_szAlias,
                       const char *_szFirstName,const char *_szLastName,
                       const char *_szEmail, const char *_szReason,
                       unsigned short _nCommand, time_t _tTime,
                       unsigned long _nFlags)
   : CUserEvent(ICQ_CMDxSUB_AUTHxREQUEST, _nCommand, 0, _tTime, _nFlags)
{
   m_szAlias = strdup(_szAlias);
   m_szFirstName = strdup(_szFirstName);
   m_szLastName = strdup(_szLastName);
   m_szEmail = strdup(_szEmail);
   m_szReason = strdup(_szReason);

   char szUin[24];
   sprintf(szUin, "%lu", _nUin);
   m_szId = strdup(szUin);
   m_nPPID = LICQ_PPID;
   m_nUin = _nUin;
}

void CEventAuthRequest::CreateDescription()
{
  if (m_szText) delete [] m_szText;
  char *p = PPIDSTRING(m_nPPID);
  m_szText = new char[strlen(m_szAlias) + strlen(m_szFirstName)
                      + strlen(m_szLastName) + strlen(m_szEmail)
                      + strlen(m_szReason) + strlen(m_szId) + strlen(p) + 256];
  //sprintf(m_szText, "%s (%s %s, %s), uin %lu, requests authorization to add you to their contact list:\n%s\n",
  //        m_szAlias, m_szFirstName, m_szLastName, m_szEmail, m_nUin, m_szReason);
  int pos = sprintf(m_szText, tr("Alias: %s\nUser: %s (%s)\nName: %s %s\nEmail: %s\n"),
     m_szAlias, m_szId, p, m_szFirstName, m_szLastName, m_szEmail);
  delete [] p;

  if (m_szReason[0] != '\0')
    sprintf(&m_szText[pos], tr("Authorization Request:\n%s\n"), m_szReason);
}


CEventAuthRequest::~CEventAuthRequest()
{
  free (m_szAlias);
  free (m_szFirstName);
  free (m_szLastName);
  free (m_szEmail);
  free (m_szReason);
  free (m_szId);
}


void CEventAuthRequest::AddToHistory(ICQUser *u, unsigned long _nPPID, direction _nDir)
{
  char *p = PPIDSTRING(m_nPPID);
  char *szOut = new char[(strlen(m_szAlias) + strlen(m_szFirstName) +
                    strlen(m_szLastName) + strlen(m_szEmail) +
                    strlen(m_szReason) + strlen(m_szId) +
                    strlen(p)) * 2 + 16 + EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(_nDir, szOut);
  nPos += sprintf(&szOut[nPos], ":%s (%s)\n:%s\n:%s\n:%s\n:%s\n", m_szId,
                  p, m_szAlias, m_szFirstName, m_szLastName, m_szEmail);
  delete [] p;

  AddStrWithColons(&szOut[nPos], m_szReason);
  AddToHistory_Flush(u, m_nPPID, szOut);
  delete [] szOut;
}



//=====CEventAuthGranted========================================================
CEventAuthGranted::CEventAuthGranted(const char *_szId, unsigned long _nPPID,
                       const char *_szMessage, unsigned short _nCommand,
                       time_t _tTime, unsigned long _nFlags)
   : CUserEvent(ICQ_CMDxSUB_AUTHxGRANTED, _nCommand, 0, _tTime, _nFlags)
{
  m_szMessage = _szMessage == NULL ? strdup("") : strdup(_szMessage);
  m_szId = strdup(_szId);
  m_nPPID = _nPPID;
  if (_nPPID == LICQ_PPID)
    m_nUin = strtoul(_szId, (char **)NULL, 10);
  else
    m_nUin = 0;
}

CEventAuthGranted::CEventAuthGranted(unsigned long _nUin, const char *_szMessage,
                       unsigned short _nCommand, time_t _tTime,
                       unsigned long _nFlags)
   : CUserEvent(ICQ_CMDxSUB_AUTHxGRANTED, _nCommand, 0, _tTime, _nFlags)
{
  m_szMessage = _szMessage == NULL ? strdup("") : strdup(_szMessage);

  char szUin[24];
  sprintf(szUin, "%lu", _nUin);
  m_szId = strdup(szUin);
  m_nPPID = LICQ_PPID;
  m_nUin = _nUin;
}

void CEventAuthGranted::CreateDescription()
{
  if (m_szText) delete [] m_szText;
  char *p = PPIDSTRING(m_nPPID);
  m_szText = new char[strlen(m_szId) + strlen(p) + strlen(m_szMessage) + 128];
  int pos = sprintf(m_szText, tr("User %s (%s) authorized you"), m_szId, p);
  delete [] p;

  if (m_szMessage[0] != '\0')
    sprintf(&m_szText[pos], ":\n%s\n", m_szMessage);
  else
    sprintf(&m_szText[pos], ".\n");
}


CEventAuthGranted::~CEventAuthGranted()
{
  free (m_szMessage);
  free (m_szId);
}

void CEventAuthGranted::AddToHistory(ICQUser *u, unsigned long _nPPID, direction _nDir)
{
  char *p = PPIDSTRING(m_nPPID);
  char *szOut = new char[(strlen(m_szId) + strlen(p) + strlen(m_szMessage))
                         * 2 + 16 + EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(_nDir, szOut);
  nPos += sprintf(&szOut[nPos], ":%s (%s)\n", m_szId, p);
  delete [] p;

  AddStrWithColons(&szOut[nPos], m_szMessage);
  AddToHistory_Flush(u, m_nPPID, szOut);
  delete [] szOut;
}



//=====CEventAuthRefused==========================================================
CEventAuthRefused::CEventAuthRefused(const char *_szId, unsigned long _nPPID,
                       const char *_szMessage, unsigned short _nCommand,
                       time_t _tTime, unsigned long _nFlags)
   : CUserEvent(ICQ_CMDxSUB_AUTHxREFUSED, _nCommand, 0, _tTime, _nFlags)
{
  m_szMessage = _szMessage == NULL ? strdup("") : strdup(_szMessage);
  m_szId = strdup(_szId);
  m_nPPID = _nPPID;
  if (_nPPID == LICQ_PPID)
    m_nUin = strtoul(_szId, (char **)NULL, 10);
  else
    m_nUin = 0;
}

CEventAuthRefused::CEventAuthRefused(unsigned long _nUin, const char *_szMessage,
                       unsigned short _nCommand, time_t _tTime,
                       unsigned long _nFlags)
   : CUserEvent(ICQ_CMDxSUB_AUTHxREFUSED, _nCommand, 0, _tTime, _nFlags)
{
  m_szMessage = _szMessage == NULL ? strdup("") : strdup(_szMessage);
  m_nUin = _nUin;
  
  char szUin[24];
  sprintf(szUin, "%lu", _nUin);
  m_szId = strdup(szUin);
  m_nPPID = LICQ_PPID;
}

void CEventAuthRefused::CreateDescription()
{
  if (m_szText) delete [] m_szText;
  char *p = PPIDSTRING(m_nPPID);
  m_szText = new char[strlen(m_szId) + strlen(p) + strlen(m_szMessage) + 128];
  int pos = sprintf(m_szText, tr("User %s (%s) refused to authorize you"), m_szId, p);
  delete [] p;

  if (m_szMessage[0] != '\0')
    sprintf(&m_szText[pos], ":\n%s\n", m_szMessage);
  else
    sprintf(&m_szText[pos], ".\n");
}


CEventAuthRefused::~CEventAuthRefused()
{
  free (m_szMessage);
  free (m_szId);
}

void CEventAuthRefused::AddToHistory(ICQUser *u, unsigned long _nPPID, direction _nDir)
{
  char *p = PPIDSTRING(m_nPPID);
  char *szOut = new char[(strlen(m_szId) + strlen(p) + strlen(m_szMessage)) * 2 +
                         16 + EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(_nDir, szOut);
  nPos += sprintf(&szOut[nPos], ":%s (%s)\n", m_szId, p);
  delete [] p;

  AddStrWithColons(&szOut[nPos], m_szMessage);
  AddToHistory_Flush(u, m_nPPID, szOut);
  delete [] szOut;
}



//====CEventWebPanel===========================================================
CEventWebPanel::CEventWebPanel(const char *_szName, char *_szEmail,
                               const char *_szMessage, unsigned short _nCommand,
                               time_t _tTime, unsigned long _nFlags)
   : CUserEvent(ICQ_CMDxSUB_WEBxPANEL, _nCommand, 0, _tTime, _nFlags)
{
  m_szName = strdup(_szName);
  m_szEmail = strdup(_szEmail);
  m_szMessage = strdup(_szMessage);
}

void CEventWebPanel::CreateDescription()
{
  if (m_szText) delete [] m_szText;
  m_szText = new char[strlen(m_szName) + strlen(m_szEmail) + strlen(m_szMessage) + 64];
  sprintf(m_szText, tr("Message from %s (%s) through web panel:\n%s\n"),
          m_szName, m_szEmail, m_szMessage);
}


CEventWebPanel::~CEventWebPanel()
{
  free (m_szName);
  free (m_szEmail);
  free (m_szMessage);
}


void CEventWebPanel::AddToHistory(ICQUser *u, unsigned long _nPPID, direction _nDir)
{
  char *szOut = new char[(strlen(m_szName) + strlen(m_szEmail) +
                    strlen(m_szMessage)) * 2 +
                   EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(_nDir, szOut);
  nPos += sprintf(&szOut[nPos], ":%s\n:%s\n", m_szName, m_szEmail);
  AddStrWithColons(&szOut[nPos], m_szMessage);
  AddToHistory_Flush(u, _nPPID, szOut);
  delete [] szOut;
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
}

void CEventEmailPager::CreateDescription()
{
  if (m_szText) delete [] m_szText;
  m_szText = new char[strlen(m_szName) + strlen(m_szEmail) + strlen(m_szMessage) + 64];
  sprintf(m_szText, tr("Message from %s (%s) through email pager:\n%s\n"),
          m_szName, m_szEmail, m_szMessage);
}


CEventEmailPager::~CEventEmailPager()
{
  free (m_szName);
  free (m_szEmail);
  free (m_szMessage);
}

void CEventEmailPager::AddToHistory(ICQUser *u, unsigned long _nPPID, direction _nDir)
{
  char *szOut = new char[(strlen(m_szName) + strlen(m_szEmail) +
                    strlen(m_szMessage)) * 2 +
                   EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(_nDir, szOut);
  nPos += sprintf(&szOut[nPos], ":%s\n:%s\n", m_szName, m_szEmail);
  AddStrWithColons(&szOut[nPos], m_szMessage);
  AddToHistory_Flush(u, _nPPID, szOut);
  delete [] szOut;
}


//====CEventContactList========================================================
CEventContactList::CEventContactList(ContactList &cl, bool bDeep,
   unsigned short nCommand, time_t tTime, unsigned long nFlags)
  : CUserEvent(ICQ_CMDxSUB_CONTACTxLIST, nCommand, 0, tTime, nFlags)
{
  if (bDeep)
    for(ContactList::const_iterator it = cl.begin(); it != cl.end(); ++it)
      m_vszFields.push_back(new CContact((*it)->IdString(), (*it)->PPID(), (*it)->Alias()));
  else
    m_vszFields = cl;
}


void CEventContactList::CreateDescription()
{
  if (m_szText) delete [] m_szText;
  m_szText = new char [m_vszFields.size() * 32 + 128];
  char *szEnd = m_szText;
  szEnd += sprintf(m_szText, tr("Contact list (%d contacts):\n"), int(m_vszFields.size()));
  ContactList::const_iterator iter;
  for (iter = m_vszFields.begin(); iter != m_vszFields.end(); ++iter)
  {
    szEnd += sprintf(szEnd, "%s (%s)\n", (*iter)->Alias(), (*iter)->IdString());
  }
}


CEventContactList::~CEventContactList()
{
  ContactList::iterator iter;
  for (iter = m_vszFields.begin(); iter != m_vszFields.end(); ++iter)
    delete *iter;
}


void CEventContactList::AddToHistory(ICQUser *u, unsigned long _nPPID, direction _nDir)
{
  unsigned long nPPID = LICQ_PPID;
  char *szOut = new char[m_vszFields.size() * 32 + EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(_nDir, szOut);
  ContactList::const_iterator iter;
  for (iter = m_vszFields.begin(); iter != m_vszFields.end(); ++iter)
  {
    char *p = PPIDSTRING((*iter)->PPID());
    nPPID = (*iter)->PPID();
    nPos += sprintf(&szOut[nPos], ":%s (%s)\n:%s\n", (*iter)->IdString(),
      p, (*iter)->Alias());
    delete [] p;
  }
  AddToHistory_Flush(u, nPPID, szOut);
  delete [] szOut;
}


CEventContactList *CEventContactList::Parse(char *sz, unsigned short nCmd, time_t nTime, unsigned long nFlags)
{
  unsigned short i = 0;
  while (sz[i] != '\0' && (unsigned char)sz[i] != 0xFE) i++;
  sz[i] = '\0';
  int nNumContacts = atoi(sz);
  char **szFields = new char*[nNumContacts * 2 + 1];
  if (!ParseFE(&sz[++i], &szFields, nNumContacts * 2 + 1))
  {
    delete []szFields;
    return NULL;
  }

  // Translate the aliases
  ContactList vc;
  for (i = 0; i < nNumContacts * 2; i += 2)
  {
    gTranslator.ServerToClient(szFields[i + 1]);
    vc.push_back(new CContact(szFields[i], LICQ_PPID, szFields[i + 1]));
  }
  delete[] szFields;

  return new CEventContactList(vc, false, nCmd, nTime, nFlags);
}

//=====CEventSms===============================================================
CEventSms::CEventSms(const char *_szNumber, const char *_szMessage,
		     unsigned short _nCommand, time_t _tTime, unsigned long _nFlags)
   : CUserEvent(ICQ_CMDxSUB_SMS, _nCommand, 0, _tTime, _nFlags)
{
  m_szNumber = strdup(_szNumber == NULL ? "" : _szNumber);
  m_szMessage = strdup(_szMessage == NULL ? "" : _szMessage);
}

void CEventSms::CreateDescription()
{
  if (m_szText) delete [] m_szText;
  m_szText = new char[strlen(m_szNumber) + strlen(m_szMessage) + 32];
  sprintf(m_szText, tr("Phone: %s\n%s\n"), m_szNumber, m_szMessage);
}

CEventSms::~CEventSms()
{
  free(m_szNumber);
  free(m_szMessage);
}

void CEventSms::AddToHistory(ICQUser *u, unsigned long _nPPID, direction _nDir)
{
  char *szOut = new char[ (strlen(m_szNumber) << 1) + 
			  (strlen(m_szMessage) << 1) +
			  + EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(_nDir, szOut);
  nPos += sprintf(&szOut[nPos], ":%s\n", m_szNumber);
  AddStrWithColons(&szOut[nPos], m_szMessage);
  AddToHistory_Flush(u, _nPPID, szOut);
  delete [] szOut;
}

CEventSms *CEventSms::Parse(char *sz, unsigned short nCmd, time_t nTime, unsigned long nFlags)
{
  char *szXmlSms, *szNum, *szTxt;
  
  szXmlSms = GetXmlTag(sz, "sms_message");
  if (szXmlSms == NULL) return NULL;
  
  szNum = GetXmlTag(szXmlSms, "sender");
  szTxt = GetXmlTag(szXmlSms, "text");
  
  CEventSms *e = new CEventSms(szNum, szTxt, nCmd, nTime, nFlags);
  
  if (szNum) free(szNum);
  if (szTxt) free(szTxt);
  free(szXmlSms);
  
  return e;
}


//=====CEventServerMessage=====================================================
CEventServerMessage::CEventServerMessage(const char *_szName,
                                         const char *_szEmail,
					 const char *_szMessage,
					 time_t _tTime)
   : CUserEvent(ICQ_CMDxSUB_MSGxSERVER, 0, 0, _tTime, 0)
{
  m_szName = strdup(_szName == NULL ? "" : _szName);
  m_szEmail = strdup(_szEmail == NULL ? "" : _szEmail);
  m_szMessage = strdup(_szMessage == NULL ? "" : _szMessage);
}

CEventServerMessage::~CEventServerMessage()
{
  free(m_szName);
  free(m_szEmail);
  free(m_szMessage);
}

void CEventServerMessage::CreateDescription()
{
  if (m_szText) delete [] m_szText;
  m_szText = new char[strlen(m_szName) + strlen(m_szEmail) + strlen(m_szMessage) + 64];
  sprintf(m_szText, tr("System Server Message from %s (%s):\n%s\n"), m_szName,
          m_szEmail, m_szMessage);
}

void CEventServerMessage::AddToHistory(ICQUser *u, unsigned long _nPPID, direction _nDir)
{
  char *szOut = new char[(strlen(m_szName) + strlen(m_szEmail) +
                         (strlen(m_szMessage) * 2) + EVENT_HEADER_SIZE)];
  int nPos = AddToHistory_Header(_nDir, szOut);
  nPos += sprintf(&szOut[nPos], ":%s\n%s\n", m_szName, m_szEmail);
  AddStrWithColons(&szOut[nPos], m_szMessage);
  AddToHistory_Flush(u, _nPPID, szOut);
  delete [] szOut;
}


CEventServerMessage *CEventServerMessage::Parse(char *sz, unsigned short nCmd,
                                                time_t nTime,
						unsigned long nFlags)
{
  char **szMsg = new char*[6]; // name, email, msg
  if (!ParseFE(sz, &szMsg, 6))
  { 
    delete [] szMsg;
    return NULL;
  }

  CEventServerMessage *e = new CEventServerMessage(szMsg[0], szMsg[3], szMsg[5],
                                                   nTime);
  delete [] szMsg;
  return e;
}


//=====CEventEmailAlert=====================================================
CEventEmailAlert::CEventEmailAlert(const char *_szName, const char *_szTo,
  const char *_szEmail, const char *_szSubject, time_t _tTime,
  const char *_szMSPAuth, const char *_szSID, const char *_szKV,
  const char *_szId, const char *_szPostURL, const char *_szMsgURL,
  const char *_szCreds, unsigned long _nSessionLength)
   : CUserEvent(ICQ_CMDxSUB_EMAILxALERT, ICQ_CMDxTCP_START, 0, _tTime, 0)
{
  m_szName = strdup(_szName == NULL ? "" : _szName);
  m_szTo = strdup(_szTo == NULL ? "" : _szTo);
  m_szEmail = strdup(_szEmail == NULL ? "" : _szEmail);
  m_szSubject = strdup(_szSubject == NULL ? "" : _szSubject);
  m_szMSPAuth = strdup(_szMSPAuth == NULL ? "" : _szMSPAuth);
  m_szSID = strdup(_szSID == NULL ? "" : _szSID);
  m_szKV = strdup(_szKV == NULL ? "" : _szKV);
  m_szId = strdup(_szId == NULL ? "" : _szId);
  m_szPostURL = strdup(_szPostURL == NULL ? "" : _szPostURL);
  m_szMsgURL = strdup(_szMsgURL == NULL ? "" : _szMsgURL);
  m_szCreds = strdup(_szCreds == NULL ? "" : _szCreds);
  m_nSessionLength = _nSessionLength;
}

CEventEmailAlert::~CEventEmailAlert()
{
  free(m_szName);
  free(m_szTo);
  free(m_szEmail);
  free(m_szSubject);
  free(m_szMSPAuth);
  free(m_szSID);
  free(m_szKV);
  free(m_szId);
  free(m_szPostURL);
  free(m_szMsgURL);
  free(m_szCreds);
}

void CEventEmailAlert::CreateDescription()
{
  if (m_szText) delete [] m_szText;
  m_szText = new char[strlen(m_szName) + strlen(m_szEmail) + strlen(m_szSubject) + 64];
  sprintf(m_szText, tr("New Email from %s (%s):\nSubject: %s\n"), m_szName,
          m_szEmail, m_szSubject);
}

void CEventEmailAlert::AddToHistory(ICQUser *u, unsigned long _nPPID, direction _nDir)
{
  char *szOut = new char[(strlen(m_szName) + strlen(m_szEmail) +
                         (strlen(m_szSubject) * 2) + EVENT_HEADER_SIZE)];
  int nPos = AddToHistory_Header(_nDir, szOut);
  nPos += sprintf(&szOut[nPos], ":%s\n:%s\n", m_szName, m_szEmail);
  AddStrWithColons(&szOut[nPos], m_szSubject);
  AddToHistory_Flush(u, _nPPID, szOut);
  delete [] szOut;
}

//=====EventPlugin=============================================================
CEventPlugin::CEventPlugin(const char *sz, unsigned short nSubCommand,
   time_t tTime, unsigned long nFlags)
   : CUserEvent(nSubCommand, 0, 0, tTime, nFlags)
{
  m_sz = sz == NULL ? strdup("") : strdup(sz);
}

void CEventPlugin::CreateDescription()
{
  if (m_szText) delete [] m_szText;
  m_szText = new char[strlen(m_sz) + 1];
  strcpy(m_szText, m_sz);
}


CEventPlugin::~CEventPlugin()
{
  free(m_sz);
}

void CEventPlugin::AddToHistory(ICQUser *, unsigned long, direction)
{
  // Don't write these to the history file
}


//=====CEventUnknownSysMsg=====================================================
CEventUnknownSysMsg::CEventUnknownSysMsg(unsigned short _nSubCommand,
                             unsigned short _nCommand, unsigned long _nUin,
                             const char *_szMsg,
                             time_t _tTime, unsigned long _nFlags)
   : CUserEvent(_nSubCommand, _nCommand, 0, _tTime, _nFlags | E_UNKNOWN)
{
  m_szMsg = _szMsg == NULL ? strdup("") : strdup(_szMsg);
  m_nUin = _nUin;
}

void CEventUnknownSysMsg::CreateDescription()
{
  if (m_szText) delete [] m_szText;
  m_szText = new char [strlen(m_szMsg) + 128];
  sprintf(m_szText, "Unknown system message (0x%04X) from %lu:\n%s\n",
          m_nSubCommand, m_nUin, m_szMsg);
}


CEventUnknownSysMsg::~CEventUnknownSysMsg()
{
  free(m_szMsg);
}

void CEventUnknownSysMsg::AddToHistory(ICQUser * /*u*/, unsigned long /*_nPPID*/, direction /*_nDir*/)
{
/*  char *szOut = new char[strlen(m_szMsg) * 2 + 16 + EVENT_HEADER_SIZE];
  int nPos = sprintf(szOut, "[ %c | 0000 | %04d | %04d | %lu ]\n",
          _nDir == D_RECEIVER ? 'R' : 'S',
          m_nCommand, (unsigned short)(m_nFlags >> 16), m_tTime);
  nPos += sprintf(&szOut[nPos], ":%lu\n", m_nUin);
  AddStrWithColons(&szOut[nPos], m_szMsg);
  AddToHistory_Flush(u, _nPPID, szOut);
  delete [] szOut;*/
}


//=====EventDescriptions=====================================================

static const int MAX_EVENT = 26;

static const char *szEventTypes[27] =
{ tr("Plugin Event"),
  tr("Message"),
  tr("Chat Request"),
  tr("File Transfer"),
  tr("URL"),
  "",
  tr("Authorization Request"),
  tr("AUthorization Refused"),
  tr("Authorization Granted"),
  tr("Server Message"),
  "",
  "",
  tr("Added to Contact List"),
  tr("Web Panel"),
  tr("Email Pager"),
  "",
  "",
  "",
  "",
  tr("Contact List"),
  "",
  "",
  "",
  "",
  "",
  "",
  tr("SMS")
};


const char *CUserEvent::Description()
{
  // not thread-safe, but I'm lazy
  static char desc[128];

  if (SubCommand() > MAX_EVENT ||
      szEventTypes[SubCommand()][0] == '\0')
    strcpy(desc, tr("Unknown Event"));
  else
  {
    strcpy(desc, szEventTypes[SubCommand()]);
    if (IsCancelled())
      strcat(desc, tr(" (cancelled)"));
  }
  return desc;
}

