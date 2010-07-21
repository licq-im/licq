// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq developers
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

#include <licq/userevents.h>

#include <cstdio>
#include <cstdlib>
#include <sstream>

#include <licq/gpghelper.h>
#include <licq/icq.h>
#include <licq/icqdefines.h>
#include <licq/translator.h>

#include "contactlist/user.h"
#include "gettext.h"

#ifdef USE_HEBREW
extern "C" {
extern char *hebrev (char* pszStr);
}
#endif

using namespace std;
using Licq::User;
using Licq::UserEvent;
using Licq::UserId;

int UserEvent::s_nId = 1;

#define EVENT_HEADER_SIZE  80

//----CUserEvent::constructor---------------------------------------------------
UserEvent::UserEvent(unsigned short nSubCommand, unsigned short nCommand,
                       unsigned short nSequence, time_t tTime,
                       unsigned long nFlags, unsigned long nConvoId)
{
   // Assigned stuff
   m_nSubCommand = nSubCommand;
   m_nCommand = nCommand;
   m_nSequence = nSequence;
   m_tTime = (tTime == TimeNow ? time(NULL) : tTime);
   m_nFlags = nFlags;
   m_nConvoId = nConvoId;

   // Initialized stuff
  myIsReceiver = true;
   m_bPending = true;
   m_nId = s_nId++;
}


void UserEvent::CopyBase(const UserEvent* e)
{
  myIsReceiver = e->isReceiver();
  m_bPending = e->Pending();
  m_nId = e->Id(); // this is new and possibly will cause problems...
  myColor.set(e->color());
}


const string& UserEvent::text() const
{
  if (myText.empty())
  {
    CreateDescription();
#ifdef USE_HEBREW
    char* p = hebrev(myText.c_str());
    myText = p;
    free(p);
#endif
  }

  return myText;
}



//-----CUserEvent::LicqVersionToString------------------------------------------
const string UserEvent::licqVersionToString(unsigned long v)
{
  static char s_szVersion[8];
  if(v % 10)
    sprintf(s_szVersion, "v%lu.%lu.%lu", v / 1000, (v / 10) % 100, v % 10);
  else
    sprintf(s_szVersion, "v%lu.%lu", v / 1000, (v / 10) % 100);
  return (s_szVersion);
}

//-----CUserEvent::destructor---------------------------------------------------
UserEvent::~UserEvent()
{
  // Empty
}


//-----NToNS--------------------------------------------------------------------
int addStrWithColons(char *_szNewStr, const string& oldStr)
{
  const char* _szOldStr = oldStr.c_str();

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
int UserEvent::AddToHistory_Header(bool isReceiver, char* szOut) const
{
  return sprintf(szOut, "[ %c | %04d | %04d | %04d | %lu ]\n",
      isReceiver ? 'R' : 'S', m_nSubCommand, m_nCommand,
      (unsigned short)(m_nFlags >> 16), (unsigned long)m_tTime);
}


void UserEvent::AddToHistory_Flush(User* u, const string& text) const
{
  if (u == NULL)
    return;

  dynamic_cast<LicqDaemon::User*>(u)->writeToHistory(text);
}


//=====CEventMsg================================================================

Licq::EventMsg::EventMsg(const string& message, unsigned short _nCommand,
                     time_t _tTime, unsigned long _nFlags, unsigned long _nConvoId)
  : UserEvent(ICQ_CMDxSUB_MSG, _nCommand, 0, _tTime, _nFlags, _nConvoId),
    myMessage(message)
{
  if (myMessage.find(Licq::GpgHelper::pgpSig) == 0)
    if (char* plaintext = Licq::gGpgHelper.Decrypt(myMessage.c_str()))
    {
      m_nFlags |= FlagEncrypted;
      myMessage = plaintext;
      free(plaintext);
    }
}


void Licq::EventMsg::CreateDescription() const
{
  myText = myMessage;
}

Licq::EventMsg* Licq::EventMsg::Copy() const
{
  EventMsg* e = new EventMsg(myMessage, m_nCommand, m_tTime, m_nFlags);
  e->CopyBase(this);
  return e;
}

void Licq::EventMsg::AddToHistory(User* u, bool isReceiver) const
{
  char *szOut = new char[ (myMessage.size() << 1) + EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(isReceiver, szOut);
  addStrWithColons(&szOut[nPos], myMessage);
  AddToHistory_Flush(u, szOut);
  delete [] szOut;
}


//=====CEventFile===============================================================

Licq::EventFile::EventFile(const string& filename, const string& fileDescription,
    unsigned long _nFileSize, const list<string>& _lFileList,
                       unsigned short _nSequence, time_t _tTime,
                       unsigned long _nFlags, unsigned long _nConvoId,
                       unsigned long _nMsgID1, unsigned long _nMsgID2)
   : UserEvent(ICQ_CMDxSUB_FILE, ICQ_CMDxTCP_START, _nSequence, _tTime, _nFlags, _nConvoId),
    myFilename(filename),
    myFileDescription(fileDescription),
     m_lFileList(_lFileList.begin(), _lFileList.end())
{
  m_nFileSize = _nFileSize;
  m_nMsgID[0] = _nMsgID1;
  m_nMsgID[1] = _nMsgID2;
}


void Licq::EventFile::CreateDescription() const
{
  char* text = new char[myFilename.size() + myFileDescription.size() + 64];
  sprintf(text, tr("File: %s (%lu bytes)\nDescription:\n%s\n"),
      myFilename.c_str(), m_nFileSize, myFileDescription.c_str());
  myText = text;
  delete[] text;
}

Licq::EventFile* Licq::EventFile::Copy() const
{
  EventFile* e = new EventFile(myFilename, myFileDescription,
        m_nFileSize, m_lFileList, m_nSequence, m_tTime, m_nFlags, m_nConvoId, m_nMsgID[0],
        m_nMsgID[1]);
  e->CopyBase(this);
  return e;
}

void Licq::EventFile::AddToHistory(User* u, bool isReceiver) const
{
  char* szOut = new char[(myFilename.size() + myFileDescription.size()) * 2 + 16 + EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(isReceiver, szOut);
  nPos += sprintf(&szOut[nPos], ":%s\n", myFilename.c_str());
  nPos += sprintf(&szOut[nPos], ":%lu\n", m_nFileSize);
  addStrWithColons(&szOut[nPos], myFileDescription);
  AddToHistory_Flush(u, szOut);
  delete [] szOut;
}



//=====CEventUrl================================================================

Licq::EventUrl::EventUrl(const string& url, const string& urlDescription,
                     unsigned short _nCommand, time_t _tTime,
                     unsigned long _nFlags, unsigned long _nConvoId)
  : UserEvent(ICQ_CMDxSUB_URL, _nCommand, 0, _tTime, _nFlags, _nConvoId),
    myUrl(url),
    myUrlDescription(urlDescription)
{
  // Empty
}


void Licq::EventUrl::CreateDescription() const
{
  char* text = new char[myUrl.size() + myUrlDescription.size() + 64];
  sprintf(text, tr("Url: %s\nDescription:\n%s\n"), myUrl.c_str(), myUrlDescription.c_str());
  myText = text;
  delete[] text;
}

Licq::EventUrl* Licq::EventUrl::Copy() const
{
  EventUrl* e = new EventUrl(myUrl, myUrlDescription, m_nCommand,
      m_tTime, m_nFlags);
  e->CopyBase(this);
  return e;
}

void Licq::EventUrl::AddToHistory(User* u, bool isReceiver) const
{
  char* szOut = new char[(myUrlDescription.size() << 1) + (myUrl.size() << 1) + EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(isReceiver, szOut);
  nPos += sprintf(&szOut[nPos], ":%s\n", myUrl.c_str());
  addStrWithColons(&szOut[nPos], myUrlDescription);
  AddToHistory_Flush(u, szOut);
  delete [] szOut;
}


Licq::EventUrl* Licq::EventUrl::Parse(char *sz, unsigned short nCmd, time_t nTime,
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
  Licq::gTranslator.ServerToClient(szUrl[0]);
  EventUrl* e = new EventUrl(szUrl[1], szUrl[0], nCmd, nTime, nFlags, nConvoId);
  delete []szUrl;

  return e;
}

//=====CEventChat===============================================================

Licq::EventChat::EventChat(const string& reason, unsigned short nSequence,
                       time_t tTime, unsigned long nFlags,
                       unsigned long nConvoId, unsigned long nMsgID1, unsigned long nMsgID2)
  : UserEvent(ICQ_CMDxSUB_CHAT, ICQ_CMDxTCP_START, nSequence, tTime, nFlags, nConvoId),
    myReason(reason)
{
  m_nPort = 0;
  m_nMsgID[0] = nMsgID1;
  m_nMsgID[1] = nMsgID2;
}

Licq::EventChat::EventChat(const string& reason, const string& clients,
   unsigned short nPort, unsigned short nSequence,
   time_t tTime, unsigned long nFlags, unsigned long nConvoId, unsigned long nMsgID1,
   unsigned long nMsgID2)
  : UserEvent(ICQ_CMDxSUB_CHAT, ICQ_CMDxTCP_START, nSequence, tTime, nFlags, nConvoId),
    myReason(reason),
    myClients(clients)
{
  m_nPort = nPort;
  m_nMsgID[0] = nMsgID1;
  m_nMsgID[1] = nMsgID2;
}

void Licq::EventChat::CreateDescription() const
{
  if (myClients.empty())
    myText = myReason;
  else
    myText = myReason + "\n--------------------\n" + tr("Multiparty:\n") + myClients;
}

Licq::EventChat* Licq::EventChat::Copy() const
{
  EventChat* e = new EventChat(myText, myClients, m_nPort,
      m_nSequence, m_tTime, m_nFlags, m_nConvoId, m_nMsgID[0], m_nMsgID[1]);
  e->CopyBase(this);
  return e;
}

void Licq::EventChat::AddToHistory(User* u, bool isReceiver) const
{
  char *szOut = new char[(text().size() << 1) + EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(isReceiver, szOut);
  addStrWithColons(&szOut[nPos], text());
  AddToHistory_Flush(u, szOut);
  delete [] szOut;
}


//=====CEventAdded==============================================================
Licq::EventAdded::EventAdded(const UserId& userId, const string& alias,
    const string& firstName, const string& lastName, const string& email,
    unsigned short _nCommand, time_t _tTime, unsigned long _nFlags)
  : UserEvent(ICQ_CMDxSUB_ADDEDxTOxLIST, _nCommand, 0, _tTime, _nFlags),
    myUserId(userId),
    myAlias(alias),
    myFirstName(firstName),
    myLastName(lastName),
    myEmail(email)
{
  // Empty
}

void Licq::EventAdded::CreateDescription() const
{
  string accountId = myUserId.accountId();
  char* text = new char[myAlias.size() + myFirstName.size() +
      myLastName.size() + myEmail.size() + accountId.size() + 512];
  sprintf(text, tr("Alias: %s\nUser: %s\nName: %s %s\nEmail: %s\n"),
      myAlias.c_str(), accountId.c_str(), myFirstName.c_str(), myLastName.c_str(), myEmail.c_str());
  myText = text;
  delete[] text;
}

Licq::EventAdded* Licq::EventAdded::Copy() const
{
  EventAdded* e = new EventAdded(myUserId, myAlias, myFirstName,
      myLastName, myEmail, m_nCommand, m_tTime, m_nFlags);
  e->CopyBase(this);
  return e;
}

void Licq::EventAdded::AddToHistory(User* u, bool isReceiver) const
{
  string accountId = myUserId.accountId();
  char *szOut = new char[(myAlias.size() + myFirstName.size() +
      myLastName.size() + myEmail.size() + accountId.size()) * 2 + 20 + EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(isReceiver, szOut);
  nPos += sprintf(&szOut[nPos], ":%s\n:%s\n:%s\n:%s\n:%s\n",
      accountId.c_str(), myAlias.c_str(), myFirstName.c_str(), myLastName.c_str(), myEmail.c_str());
  AddToHistory_Flush(u, szOut);
  delete [] szOut;
}



//=====CEventAuthReq===============================================================
Licq::EventAuthRequest::EventAuthRequest(const UserId& userId, const string& alias,
    const string& firstName, const string& lastName, const string& email,
    const string& reason, unsigned short _nCommand, time_t _tTime, unsigned long _nFlags)
  : UserEvent(ICQ_CMDxSUB_AUTHxREQUEST, _nCommand, 0, _tTime, _nFlags),
    myUserId(userId),
    myAlias(alias),
    myFirstName(firstName),
    myLastName(lastName),
    myEmail(email),
    myReason(reason)
{
  // Empty
}

void Licq::EventAuthRequest::CreateDescription() const
{
  string userIdStr = myUserId.toString();
  char* text = new char[myAlias.size() + myFirstName.size() + myLastName.size() +
      myEmail.size() + myReason.size() + userIdStr.size() + 256];
  //sprintf(m_szText, "%s (%s %s, %s), uin %s, requests authorization to add you to their contact list:\n%s\n",
  //        m_szAlias, m_szFirstName, m_szLastName, m_szEmail, m_szId, m_szReason);
  int pos = sprintf(text, tr("Alias: %s\nUser: %s\nName: %s %s\nEmail: %s\n"),
      myAlias.c_str(), userIdStr.c_str(), myFirstName.c_str(), myLastName.c_str(), myEmail.c_str());

  if (!myReason.empty())
    sprintf(&text[pos], tr("Authorization Request:\n%s\n"), myReason.c_str());

  myText = text;
  delete[] text;
}

Licq::EventAuthRequest* Licq::EventAuthRequest::Copy() const
{
  EventAuthRequest* e = new EventAuthRequest(myUserId, myAlias, myFirstName,
      myLastName, myEmail, myReason, m_nCommand, m_tTime, m_nFlags);
  e->CopyBase(this);
  return e;
}

void Licq::EventAuthRequest::AddToHistory(User* u, bool isReceiver) const
{
  string accountId = myUserId.accountId();
  char *szOut = new char[(myAlias.size() + myFirstName.size() +
      myLastName.size() + myEmail.size() + myReason.size() +
      accountId.size()) * 2 + 16 + EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(isReceiver, szOut);
  nPos += sprintf(&szOut[nPos], ":%s\n:%s\n:%s\n:%s\n:%s\n",
      accountId.c_str(), myAlias.c_str(), myFirstName.c_str(), myLastName.c_str(), myEmail.c_str());

  addStrWithColons(&szOut[nPos], myReason);
  AddToHistory_Flush(u, szOut);
  delete [] szOut;
}



//=====CEventAuthGranted========================================================
Licq::EventAuthGranted::EventAuthGranted(const UserId& userId, const string& message,
    unsigned short _nCommand, time_t _tTime, unsigned long _nFlags)
  : UserEvent(ICQ_CMDxSUB_AUTHxGRANTED, _nCommand, 0, _tTime, _nFlags),
    myUserId(userId),
    myMessage(message)
{
  // Empty
}

void Licq::EventAuthGranted::CreateDescription() const
{
  string userIdStr = myUserId.toString();
  char* text = new char[userIdStr.size() + myMessage.size() + 128];
  int pos = sprintf(text, tr("User %s authorized you"), userIdStr.c_str());

  if (!myMessage.empty())
    sprintf(&text[pos], ":\n%s\n", myMessage.c_str());
  else
    sprintf(&text[pos], ".\n");
  myText = text;
  delete[] text;
}

Licq::EventAuthGranted* Licq::EventAuthGranted::Copy() const
{
  EventAuthGranted* e = new EventAuthGranted(myUserId, myMessage,
      m_nCommand, m_tTime, m_nFlags);
  e->CopyBase(this);
  return e;
}

void Licq::EventAuthGranted::AddToHistory(User* u, bool isReceiver) const
{
  string accountId = myUserId.accountId();
  char *szOut = new char[(accountId.size() + myMessage.size()) * 2 + 16 + EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(isReceiver, szOut);
  nPos += sprintf(&szOut[nPos], ":%s\n", accountId.c_str());

  addStrWithColons(&szOut[nPos], myMessage);
  AddToHistory_Flush(u, szOut);
  delete [] szOut;
}



//=====CEventAuthRefused==========================================================
Licq::EventAuthRefused::EventAuthRefused(const UserId& userId, const string& message,
    unsigned short _nCommand, time_t _tTime, unsigned long _nFlags)
  : UserEvent(ICQ_CMDxSUB_AUTHxREFUSED, _nCommand, 0, _tTime, _nFlags),
    myUserId(userId),
    myMessage(message)
{
  // Empty
}

void Licq::EventAuthRefused::CreateDescription() const
{
  string userIdStr = myUserId.toString();
  char* text = new char[userIdStr.size() + myMessage.size() + 128];
  int pos = sprintf(text, tr("User %s refused to authorize you"), userIdStr.c_str());

  if (!myMessage.empty())
    sprintf(&text[pos], ":\n%s\n", myMessage.c_str());
  else
    sprintf(&text[pos], ".\n");
  myText = text;
  delete[] text;
}

Licq::EventAuthRefused* Licq::EventAuthRefused::Copy() const
{
  EventAuthRefused* e = new EventAuthRefused(myUserId, myMessage, m_nCommand, m_tTime, m_nFlags);
  e->CopyBase(this);
  return e;
}

void Licq::EventAuthRefused::AddToHistory(User* u, bool isReceiver) const
{
  string accountId = myUserId.accountId();
  char *szOut = new char[(accountId.size() + myMessage.size()) * 2 + 16 + EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(isReceiver, szOut);
  nPos += sprintf(&szOut[nPos], ":%s\n", accountId.c_str());

  addStrWithColons(&szOut[nPos], myMessage);
  AddToHistory_Flush(u, szOut);
  delete [] szOut;
}



//====CEventWebPanel===========================================================
Licq::EventWebPanel::EventWebPanel(const string& name, const string& email,
    const string& message, unsigned short _nCommand, time_t _tTime, unsigned long _nFlags)
  : UserEvent(ICQ_CMDxSUB_WEBxPANEL, _nCommand, 0, _tTime, _nFlags),
    myName(name),
    myEmail(email),
    myMessage(message)
{
  // Empty
}

void Licq::EventWebPanel::CreateDescription() const
{
  char* text = new char[myName.size() + myEmail.size() + myMessage.size() + 64];
  sprintf(text, tr("Message from %s (%s) through web panel:\n%s\n"),
      myName.c_str(), myEmail.c_str(), myMessage.c_str());

  myText = text;
  delete[] text;
}

Licq::EventWebPanel* Licq::EventWebPanel::Copy() const
{
  EventWebPanel* e = new EventWebPanel(myName, myEmail, myMessage,
      m_nCommand, m_tTime, m_nFlags);
  e->CopyBase(this);
  return e;
}

void Licq::EventWebPanel::AddToHistory(User* u, bool isReceiver) const
{
  char *szOut = new char[(myName.size() + myEmail.size() + myMessage.size()) * 2 + EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(isReceiver, szOut);
  nPos += sprintf(&szOut[nPos], ":%s\n:%s\n", myName.c_str(), myEmail.c_str());
  addStrWithColons(&szOut[nPos], myMessage);
  AddToHistory_Flush(u, szOut);
  delete [] szOut;
}


//====CEventEmailPager==========================================================
Licq::EventEmailPager::EventEmailPager(const string& name, const string& email,
    const string& message, unsigned short _nCommand, time_t _tTime, unsigned long _nFlags)
  : UserEvent(ICQ_CMDxSUB_EMAILxPAGER, _nCommand, 0, _tTime, _nFlags),
    myName(name),
    myEmail(email),
    myMessage(message)
{
  // Empty
}

void Licq::EventEmailPager::CreateDescription() const
{
  char* text = new char[myName.size() + myEmail.size() + myMessage.size() + 64];
  sprintf(text, tr("Message from %s (%s) through email pager:\n%s\n"),
      myName.c_str(), myEmail.c_str(), myMessage.c_str());
  myText = text;
  delete[] text;
}

Licq::EventEmailPager* Licq::EventEmailPager::Copy() const
{
  EventEmailPager* e = new EventEmailPager(myName, myEmail, myMessage,
      m_nCommand, m_tTime, m_nFlags);
  e->CopyBase(this);
  return e;
}

void Licq::EventEmailPager::AddToHistory(User* u, bool isReceiver) const
{
  char *szOut = new char[(myName.size() + myEmail.size() + myMessage.size()) * 2 + EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(isReceiver, szOut);
  nPos += sprintf(&szOut[nPos], ":%s\n:%s\n", myName.c_str(), myEmail.c_str());
  addStrWithColons(&szOut[nPos], myMessage);
  AddToHistory_Flush(u, szOut);
  delete [] szOut;
}

Licq::EventContactList::Contact::Contact(const UserId& userId, const string& alias)
  : myUserId(userId),
    myAlias(alias)
{
  // Empty
}

//====CEventContactList========================================================
Licq::EventContactList::EventContactList(const ContactList& cl, bool bDeep,
   unsigned short nCommand, time_t tTime, unsigned long nFlags)
  : UserEvent(ICQ_CMDxSUB_CONTACTxLIST, nCommand, 0, tTime, nFlags)
{
  if (bDeep)
    for(ContactList::const_iterator it = cl.begin(); it != cl.end(); ++it)
      m_vszFields.push_back(new Contact((*it)->userId(), (*it)->alias()));
  else
    m_vszFields = cl;
}


void Licq::EventContactList::CreateDescription() const
{
  char* text = new char [m_vszFields.size() * 32 + 128];
  char *szEnd = text;
  szEnd += sprintf(text, tr("Contact list (%d contacts):\n"), int(m_vszFields.size()));
  ContactList::const_iterator iter;
  for (iter = m_vszFields.begin(); iter != m_vszFields.end(); ++iter)
  {
    szEnd += sprintf(szEnd, "%s (%s)\n", (*iter)->alias().c_str(), (*iter)->userId().toString().c_str());
  }
  myText = text;
  delete[] text;
}


Licq::EventContactList::~EventContactList()
{
  ContactList::iterator iter;
  for (iter = m_vszFields.begin(); iter != m_vszFields.end(); ++iter)
    delete *iter;
}

Licq::EventContactList* Licq::EventContactList::Copy() const
{
  EventContactList* e = new EventContactList(m_vszFields, true,
      m_nCommand, m_tTime, m_nFlags);
  e->CopyBase(this);
  return e;
}

void Licq::EventContactList::AddToHistory(User* u, bool isReceiver) const
{
  char *szOut = new char[m_vszFields.size() * 32 + EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(isReceiver, szOut);
  ContactList::const_iterator iter;
  for (iter = m_vszFields.begin(); iter != m_vszFields.end(); ++iter)
  {
    nPos += sprintf(&szOut[nPos], ":%s\n:%s\n",
        (*iter)->userId().accountId().c_str(), (*iter)->alias().c_str());
  }
  AddToHistory_Flush(u, szOut);
  delete [] szOut;
}


Licq::EventContactList* Licq::EventContactList::Parse(char *sz, unsigned short nCmd, time_t nTime, unsigned long nFlags)
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
    Licq::gTranslator.ServerToClient(szFields[i + 1]);
    UserId userId(szFields[i], LICQ_PPID);
    vc.push_back(new Contact(userId, szFields[i + 1]));
  }
  delete[] szFields;

  return new EventContactList(vc, false, nCmd, nTime, nFlags);
}

//=====CEventSms===============================================================
Licq::EventSms::EventSms(const string& number, const string& message,
                     unsigned short _nCommand, time_t _tTime, unsigned long _nFlags)
  : UserEvent(ICQ_CMDxSUB_SMS, _nCommand, 0, _tTime, _nFlags),
    myNumber(number),
    myMessage(message)
{
  // Empty
}

void Licq::EventSms::CreateDescription() const
{
  char* text = new char[myNumber.size() + myMessage.size() + 32];
  sprintf(text, tr("Phone: %s\n%s\n"), myNumber.c_str(), myMessage.c_str());
  myText = text;
  delete[] text;
}

Licq::EventSms* Licq::EventSms::Copy() const
{
  EventSms* e = new EventSms(myNumber, myMessage, m_nCommand, m_tTime, m_nFlags);
  e->CopyBase(this);
  return e;
}

void Licq::EventSms::AddToHistory(User* u, bool isReceiver) const
{
  char *szOut = new char[ (myNumber.size() << 1) + (myMessage.size() << 1) + + EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(isReceiver, szOut);
  nPos += sprintf(&szOut[nPos], ":%s\n", myNumber.c_str());
  addStrWithColons(&szOut[nPos], myMessage);
  AddToHistory_Flush(u, szOut);
  delete [] szOut;
}

Licq::EventSms* Licq::EventSms::Parse(const std::string& s, unsigned short nCmd, time_t nTime, unsigned long nFlags)
{
  string xmlSms = CICQDaemon::getXmlTag(s, "sms_message");
  if (xmlSms.empty())
    return NULL;

  string number = CICQDaemon::getXmlTag(xmlSms, "sender");
  string msg = CICQDaemon::getXmlTag(xmlSms, "text");

  return new EventSms(number, msg, nCmd, nTime, nFlags);
}


//=====CEventServerMessage=====================================================
Licq::EventServerMessage::EventServerMessage(const string& name,
    const string& email, const string& message, time_t _tTime)
  : UserEvent(ICQ_CMDxSUB_MSGxSERVER, 0, 0, _tTime, 0),
    myName(name),
    myEmail(email),
    myMessage(message)
{
  // Empty
}

void Licq::EventServerMessage::CreateDescription() const
{
  char* text = new char[myName.size() + myEmail.size() + myMessage.size() + 64];
  sprintf(text, tr("System Server Message from %s (%s):\n%s\n"),
      myName.c_str(), myEmail.c_str(), myMessage.c_str());

  myText = text;
  delete[] text;
}

Licq::EventServerMessage* Licq::EventServerMessage::Copy() const
{
  EventServerMessage* e = new EventServerMessage(myName, myEmail, myMessage, m_tTime);
  e->CopyBase(this);
  return e;
}

void Licq::EventServerMessage::AddToHistory(User* u, bool isReceiver) const
{
  char *szOut = new char[(myName.size() + myEmail.size() + (myMessage.size() * 2) + EVENT_HEADER_SIZE)];
  int nPos = AddToHistory_Header(isReceiver, szOut);
  nPos += sprintf(&szOut[nPos], ":%s\n%s\n", myName.c_str(), myEmail.c_str());
  addStrWithColons(&szOut[nPos], myMessage);
  AddToHistory_Flush(u, szOut);
  delete [] szOut;
}


Licq::EventServerMessage* Licq::EventServerMessage::Parse(char *sz, unsigned short /* nCmd */,
    time_t nTime, unsigned long /* nFlags */)
{
  char **szMsg = new char*[6]; // name, email, msg
  if (!ParseFE(sz, &szMsg, 6))
  {
    delete [] szMsg;
    return NULL;
  }

  EventServerMessage* e = new EventServerMessage(szMsg[0], szMsg[3], szMsg[5], nTime);
  delete [] szMsg;
  return e;
}


//=====CEventEmailAlert=====================================================
Licq::EventEmailAlert::EventEmailAlert(const string& name, const string& to,
    const string& email, const string& subject, time_t _tTime, const string& mspAuth,
    const string& sid, const string& kv, const string& id, const string& postUrl,
    const string& msgUrl, const string& creds, unsigned long sessionLength)
  : UserEvent(ICQ_CMDxSUB_EMAILxALERT, ICQ_CMDxTCP_START, 0, _tTime, 0),
    myName(name),
    myTo(to),
    myEmail(email),
    mySubject(subject),
    myMspAuth(mspAuth),
    mySid(sid),
    myKv(kv),
    myId(id),
    myPostUrl(postUrl),
    myMsgUrl(msgUrl),
    myCreds(creds),
    mySessionLength(sessionLength)
{
  // Empty
}

void Licq::EventEmailAlert::CreateDescription() const
{
  char* text = new char[myName.size() + myEmail.size() + mySubject.size() + 64];
  sprintf(text, tr("New Email from %s (%s):\nSubject: %s\n"),
      myName.c_str(), myEmail.c_str(), mySubject.c_str());
  myText = text;
  delete[] text;
}

Licq::EventEmailAlert* Licq::EventEmailAlert::Copy() const
{
  EventEmailAlert* e = new EventEmailAlert(myName, myTo, myEmail,
      mySubject, m_tTime, myMspAuth, mySid, myKv, myId,
      myPostUrl, myMsgUrl, myCreds, mySessionLength);
  e->CopyBase(this);
  return e;
}

void Licq::EventEmailAlert::AddToHistory(User* u, bool isReceiver) const
{
  char *szOut = new char[myName.size() + myEmail.size() +
      mySubject.size() * 2 + EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(isReceiver, szOut);
  nPos += sprintf(&szOut[nPos], ":%s\n:%s\n", myName.c_str(), myEmail.c_str());
  addStrWithColons(&szOut[nPos], mySubject);
  AddToHistory_Flush(u, szOut);
  delete [] szOut;
}

//=====CEventUnknownSysMsg=====================================================
Licq::EventUnknownSysMsg::EventUnknownSysMsg(unsigned short _nSubCommand,
    unsigned short _nCommand, const UserId& userId, const string& message,
                             time_t _tTime, unsigned long _nFlags)
  : UserEvent(_nSubCommand, _nCommand, 0, _tTime, _nFlags | FlagUnknown),
    myUserId(userId),
    myMessage(message)
{
  // Empty
}

void Licq::EventUnknownSysMsg::CreateDescription() const
{
  char* text = new char [myMessage.size() + 128];
  sprintf(text, "Unknown system message (0x%04X) from %s:\n%s\n",
      m_nSubCommand, myUserId.toString().c_str(), myMessage.c_str());
  myText = text;
  delete[] text;
}

Licq::EventUnknownSysMsg* Licq::EventUnknownSysMsg::Copy() const
{
  EventUnknownSysMsg* e = new EventUnknownSysMsg(m_nSubCommand,
      m_nCommand, myUserId, myMessage, m_tTime, m_nFlags);
  e->CopyBase(this);
  return e;
}

void Licq::EventUnknownSysMsg::AddToHistory(User* /* u */, bool /* isReceiver */) const
{
/*  char *szOut = new char[strlen(m_szMsg) * 2 + 16 + EVENT_HEADER_SIZE];
  int nPos = sprintf(szOut, "[ %c | 0000 | %04d | %04d | %lu ]\n",
      isReceiver ? 'R' : 'S', m_nCommand, (unsigned short)(m_nFlags >> 16), m_tTime);
  nPos += sprintf(&szOut[nPos], ":%s\n", m_szId);
  addStrWithColons(&szOut[nPos], myMessage);
  AddToHistory_Flush(u, szOut);
  delete [] szOut;*/
}


//=====EventDescriptions=====================================================

static const int MAX_EVENT = 26;

static const char *const eventDescriptions[27] =
{ tr("Plugin Event"),
  tr("Message"),
  tr("Chat Request"),
  tr("File Transfer"),
  tr("URL"),
  "",
  tr("Authorization Request"),
  tr("Authorization Refused"),
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

string UserEvent::description() const
{
  if (SubCommand() > MAX_EVENT || eventDescriptions[SubCommand()][0] == '\0')
    return tr("Unknown Event");

  string desc = eventDescriptions[SubCommand()];
  if (IsCancelled())
    desc += tr(" (cancelled)");
  return desc;
}

