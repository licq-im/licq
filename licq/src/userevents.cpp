/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2012 Licq developers <licq-dev@googlegroups.com>
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

#include <cstdlib>
#include <sstream>

#include <licq/contactlist/user.h>
#include <licq/gpghelper.h>
#include <licq/translator.h>

#include "contactlist/usermanager.h"
#include "gettext.h"

#ifdef USE_HEBREW
extern "C" {
extern char *hebrev (const char* pszStr);
}
#endif

using namespace std;
using Licq::User;
using Licq::UserEvent;
using Licq::UserId;

int UserEvent::s_nId = 1;

string UserEvent::eventName(unsigned eventType)
{
  switch (eventType)
  {
    case TypeMessage:           return tr("Message");
    case TypeFile:              return tr("File Transfer");
    case TypeUrl:               return tr("URL");
    case TypeChat:              return tr("Chat Request");
    case TypeAdded:             return tr("Added to Contact List");
    case TypeAuthRequest:       return tr("Authorization Request");
    case TypeAuthGranted:       return tr("Authorization Granted");
    case TypeAuthRefused:       return tr("Authorization Refused");
    case TypeWebPanel:          return tr("Web Panel");
    case TypeEmailPager:        return tr("Email Pager");
    case TypeContactList:       return tr("Contact List");
    case TypeSms:               return tr("SMS");
    case TypeMsgServer:         return tr("Server Message");
    case TypeEmailAlert:        return tr("Email Alert");
    case TypeUnknownSys:        return tr("Unknown Event");
    default:                    return "";
  }
}

//----CUserEvent::constructor---------------------------------------------------
UserEvent::UserEvent(EventType eventType,
                       unsigned short nSequence, time_t tTime,
                       unsigned long nFlags, unsigned long nConvoId)
  : myEventType(eventType)
{
   // Assigned stuff
   m_nSequence = nSequence;
   m_tTime = (tTime == TimeNow ? time(NULL) : tTime);
   m_nFlags = nFlags;
   m_nConvoId = nConvoId;

   // Initialized stuff
   m_bPending = true;
   m_nId = s_nId++;
}


void UserEvent::CopyBase(const UserEvent* e)
{
  m_nFlags = e->m_nFlags;
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

const string& UserEvent::textLoc() const
{
  if (myTextLoc.empty())
    myTextLoc = gTranslator.fromUtf8(text());
  return myTextLoc;
}

//-----CUserEvent::LicqVersionToString------------------------------------------
const string UserEvent::licqVersionToString(unsigned long v)
{
  stringstream buf;
  buf << 'v' << (v/1000) << '.' << ((v/10)%100) << '.' << (v%10);
  return buf.str();
}

//-----CUserEvent::destructor---------------------------------------------------
UserEvent::~UserEvent()
{
  // Empty
}


//-----NToNS--------------------------------------------------------------------
string addStrWithColons(const string& oldStr)
{
  string str = ":";
  str += oldStr;
  size_t pos = 0;
  while ((pos = str.find('\n', pos)) < str.size())
  {
    str.insert(pos+1, ":");
    pos += 2;
  }
  return str;
}


//-----CUserEvent::AddToHistory-------------------------------------------------
string UserEvent::historyHeader(bool isReceiver) const
{
  int m_nCommand;

  if (myEventType == TypeUnknownSys)
    m_nCommand = (dynamic_cast<const EventUnknownSysMsg*>(this))->Command();
  else if (m_nFlags & FlagDirect)
    m_nCommand = CommandDirect;
  else if (m_nFlags & FlagSender)
    m_nCommand = CommandSent;
  else if (m_nFlags & FlagOffline)
    m_nCommand = CommandRcvOffline;
  else
    m_nCommand = CommandRcvOnline;

  // Format: "[ x | 1234 | 1234 | 1234 | 123456789 ]\n"
  stringstream buf;
  buf << "[ " << (isReceiver ? 'R' : 'S') << " | ";
  buf.fill('0');
  buf.width(4);
  buf << (myEventType != TypeUnknownSys ? myEventType :
      (dynamic_cast<const EventUnknownSysMsg*>(this))->subCommand());
  buf << " | ";
  buf.width(4);
  buf << m_nCommand;
  buf << " | ";
  buf.width(4);
  buf << (((unsigned short)((m_nFlags | FlagUnicode) >> 16)) & 0x809F);
  buf << " | ";
  buf.width(1);
  buf << (unsigned long)m_tTime << " ]\n";

  return buf.str();
}


void UserEvent::writeUserHistory(User* u, const string& text) const
{
  if (u == NULL)
    return;

  LicqDaemon::gUserManager.writeToUserHistory(u, text);
}


//=====CEventMsg================================================================

Licq::EventMsg::EventMsg(const string& message,
                     time_t _tTime, unsigned long _nFlags, unsigned long _nConvoId)
  : UserEvent(TypeMessage, 0, _tTime, _nFlags, _nConvoId),
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
  EventMsg* e = new EventMsg(myMessage, m_tTime, m_nFlags);
  e->CopyBase(this);
  return e;
}

void Licq::EventMsg::AddToHistory(User* u, bool isReceiver) const
{
  string str;
  str = historyHeader(isReceiver);
  str += addStrWithColons(myMessage);
  writeUserHistory(u, str);
}


//=====CEventFile===============================================================

Licq::EventFile::EventFile(const string& filename, const string& fileDescription,
    unsigned long _nFileSize, const list<string>& _lFileList,
                       unsigned short _nSequence, time_t _tTime,
                       unsigned long _nFlags, unsigned long _nConvoId,
                       unsigned long _nMsgID1, unsigned long _nMsgID2)
  : UserEvent(TypeFile, _nSequence, _tTime, _nFlags | FlagDirect, _nConvoId),
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
  stringstream buf;
  buf << gTranslator.toUtf8(tr("File")) << ": " << gTranslator.toUtf8(myFilename);
  buf << " (" << m_nFileSize << ' ' << gTranslator.toUtf8(tr("bytes")) << ')' << '\n';
  buf << gTranslator.toUtf8(tr("Description")) << ":\n" << myFileDescription << '\n';
  myText = buf.str();
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
  stringstream buf;
  buf << historyHeader(isReceiver);
  buf << ':' << myFilename << '\n';
  buf << ':' << m_nFileSize << '\n';
  buf << addStrWithColons(myFileDescription);
  writeUserHistory(u, buf.str());
}



//=====CEventUrl================================================================

Licq::EventUrl::EventUrl(const string& url, const string& urlDescription,
    time_t _tTime, unsigned long _nFlags, unsigned long _nConvoId)
  : UserEvent(TypeUrl, 0, _tTime, _nFlags, _nConvoId),
    myUrl(url),
    myUrlDescription(urlDescription)
{
  // Empty
}


void Licq::EventUrl::CreateDescription() const
{
  stringstream buf;
  buf << gTranslator.toUtf8(tr("Url")) << ": " <<  myUrl << '\n';
  buf << gTranslator.toUtf8(tr("Description")) << ":\n" << myUrlDescription << '\n';
  myText = buf.str();
}

Licq::EventUrl* Licq::EventUrl::Copy() const
{
  EventUrl* e = new EventUrl(myUrl, myUrlDescription, m_tTime, m_nFlags);
  e->CopyBase(this);
  return e;
}

void Licq::EventUrl::AddToHistory(User* u, bool isReceiver) const
{
  string str;
  str = historyHeader(isReceiver);
  str += ':';
  str += myUrl;
  str += '\n';
  str += addStrWithColons(myUrlDescription);
  writeUserHistory(u, str);
}

//=====CEventChat===============================================================

Licq::EventChat::EventChat(const string& reason, unsigned short nSequence,
                       time_t tTime, unsigned long nFlags,
                       unsigned long nConvoId, unsigned long nMsgID1, unsigned long nMsgID2)
  : UserEvent(TypeChat, nSequence, tTime, nFlags | FlagDirect, nConvoId),
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
  : UserEvent(TypeChat, nSequence, tTime, nFlags | FlagDirect, nConvoId),
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
    myText = myReason + "\n--------------------\n" + gTranslator.toUtf8(tr("Multiparty")) + ":\n" + myClients;
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
  string str;
  str = historyHeader(isReceiver);
  str += addStrWithColons(text());
  writeUserHistory(u, str);
}


//=====CEventAdded==============================================================
Licq::EventAdded::EventAdded(const UserId& userId, const string& alias,
    const string& firstName, const string& lastName, const string& email,
    time_t _tTime, unsigned long _nFlags)
  : UserEvent(TypeAdded, 0, _tTime, _nFlags),
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
  stringstream buf;
  buf << gTranslator.toUtf8(tr("Alias")) << ": " << myAlias << '\n';
  buf << gTranslator.toUtf8(tr("User")) << ": " << myUserId.accountId() << '\n';
  buf << gTranslator.toUtf8(tr("Name")) << ": " << myFirstName << ' ' << myLastName << '\n';
  buf << gTranslator.toUtf8(tr("Email")) << ": " << myEmail << '\n';
  myText = buf.str();
}

Licq::EventAdded* Licq::EventAdded::Copy() const
{
  EventAdded* e = new EventAdded(myUserId, myAlias, myFirstName,
      myLastName, myEmail, m_tTime, m_nFlags);
  e->CopyBase(this);
  return e;
}

void Licq::EventAdded::AddToHistory(User* u, bool isReceiver) const
{
  stringstream buf;
  buf << historyHeader(isReceiver);
  buf << ':' << myUserId.accountId() << '\n';
  buf << ':' << myAlias << '\n';
  buf << ':' << myFirstName << '\n';
  buf << ':' << myLastName << '\n';
  buf << ':' << myEmail << '\n';
  writeUserHistory(u, buf.str());
}



//=====CEventAuthReq===============================================================
Licq::EventAuthRequest::EventAuthRequest(const UserId& userId, const string& alias,
    const string& firstName, const string& lastName, const string& email,
    const string& reason, time_t _tTime, unsigned long _nFlags)
  : UserEvent(TypeAuthRequest, 0, _tTime, _nFlags),
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
  stringstream buf;
  buf << gTranslator.toUtf8(tr("Alias")) << ": " << myAlias << '\n';
  buf << gTranslator.toUtf8(tr("User")) << ": " << myUserId.accountId() << '\n';
  buf << gTranslator.toUtf8(tr("Name")) << ": " << myFirstName << ' ' << myLastName << '\n';
  buf << gTranslator.toUtf8(tr("Email")) << ": " << myEmail << '\n';
  if (!myReason.empty())
    buf << gTranslator.toUtf8(tr("Authorization Request")) << ":\n" << myReason << '\n';
  myText = buf.str();
}

Licq::EventAuthRequest* Licq::EventAuthRequest::Copy() const
{
  EventAuthRequest* e = new EventAuthRequest(myUserId, myAlias, myFirstName,
      myLastName, myEmail, myReason, m_tTime, m_nFlags);
  e->CopyBase(this);
  return e;
}

void Licq::EventAuthRequest::AddToHistory(User* u, bool isReceiver) const
{
  stringstream buf;
  buf << historyHeader(isReceiver);
  buf << ':' << myUserId.accountId() << '\n';
  buf << ':' << myAlias << '\n';
  buf << ':' << myFirstName << '\n';
  buf << ':' << myLastName << '\n';
  buf << ':' << myEmail << '\n';
  buf << addStrWithColons(myReason);
  writeUserHistory(u, buf.str());
}



//=====CEventAuthGranted========================================================
Licq::EventAuthGranted::EventAuthGranted(const UserId& userId, const string& message,
    time_t _tTime, unsigned long _nFlags)
  : UserEvent(TypeAuthGranted, 0, _tTime, _nFlags),
    myUserId(userId),
    myMessage(message)
{
  // Empty
}

void Licq::EventAuthGranted::CreateDescription() const
{
  stringstream buf;
  buf << gTranslator.toUtf8(tr("User ")) << myUserId.toString() << gTranslator.toUtf8(tr(" authorized you"));
  if (!myMessage.empty())
    buf << ":\n" << myMessage << '\n';
  else
    buf << ".\n";
  myText = buf.str();
}

Licq::EventAuthGranted* Licq::EventAuthGranted::Copy() const
{
  EventAuthGranted* e = new EventAuthGranted(myUserId, myMessage,
      m_tTime, m_nFlags);
  e->CopyBase(this);
  return e;
}

void Licq::EventAuthGranted::AddToHistory(User* u, bool isReceiver) const
{
  string str;
  str = historyHeader(isReceiver);
  str += ':';
  str += myUserId.accountId();
  str += '\n';
  str += addStrWithColons(myMessage);
  writeUserHistory(u, str);
}



//=====CEventAuthRefused==========================================================
Licq::EventAuthRefused::EventAuthRefused(const UserId& userId, const string& message,
    time_t _tTime, unsigned long _nFlags)
  : UserEvent(TypeAuthRefused, 0, _tTime, _nFlags),
    myUserId(userId),
    myMessage(message)
{
  // Empty
}

void Licq::EventAuthRefused::CreateDescription() const
{
  stringstream buf;
  buf << gTranslator.toUtf8(tr("User ")) << myUserId.toString() << gTranslator.toUtf8(tr(" refused to authorize you"));
  if (!myMessage.empty())
    buf << ":\n" << myMessage << '\n';
  else
    buf << ".\n";
  myText = buf.str();
}

Licq::EventAuthRefused* Licq::EventAuthRefused::Copy() const
{
  EventAuthRefused* e = new EventAuthRefused(myUserId, myMessage, m_tTime, m_nFlags);
  e->CopyBase(this);
  return e;
}

void Licq::EventAuthRefused::AddToHistory(User* u, bool isReceiver) const
{
  string str;
  str = historyHeader(isReceiver);
  str += ':';
  str += myUserId.accountId();
  str += '\n';
  str += addStrWithColons(myMessage);
  writeUserHistory(u, str);
}



//====CEventWebPanel===========================================================
Licq::EventWebPanel::EventWebPanel(const string& name, const string& email,
    const string& message, time_t _tTime, unsigned long _nFlags)
  : UserEvent(TypeWebPanel, 0, _tTime, _nFlags),
    myName(name),
    myEmail(email),
    myMessage(message)
{
  // Empty
}

void Licq::EventWebPanel::CreateDescription() const
{
  stringstream buf;
  buf << gTranslator.toUtf8(tr("Message from ")) << myName << " (" << myEmail
      << ')' << gTranslator.toUtf8(tr(" through web panel")) << ":\n";
  buf << myMessage << '\n';
  myText = buf.str();
}

Licq::EventWebPanel* Licq::EventWebPanel::Copy() const
{
  EventWebPanel* e = new EventWebPanel(myName, myEmail, myMessage,
      m_tTime, m_nFlags);
  e->CopyBase(this);
  return e;
}

void Licq::EventWebPanel::AddToHistory(User* u, bool isReceiver) const
{
  stringstream buf;
  buf << historyHeader(isReceiver);
  buf << ':' << myName << '\n';
  buf << ':' << myEmail << '\n';
  buf << addStrWithColons(myMessage);
  writeUserHistory(u, buf.str());
}


//====CEventEmailPager==========================================================
Licq::EventEmailPager::EventEmailPager(const string& name, const string& email,
    const string& message, time_t _tTime, unsigned long _nFlags)
  : UserEvent(TypeEmailPager, 0, _tTime, _nFlags),
    myName(name),
    myEmail(email),
    myMessage(message)
{
  // Empty
}

void Licq::EventEmailPager::CreateDescription() const
{
  stringstream buf;
  buf << gTranslator.toUtf8(tr("Message from ")) << myName << " (" << myEmail
      << ')' << gTranslator.toUtf8(tr(" through email pager")) << ":\n";
  buf << myMessage << '\n';
  myText = buf.str();
}

Licq::EventEmailPager* Licq::EventEmailPager::Copy() const
{
  EventEmailPager* e = new EventEmailPager(myName, myEmail, myMessage,
      m_tTime, m_nFlags);
  e->CopyBase(this);
  return e;
}

void Licq::EventEmailPager::AddToHistory(User* u, bool isReceiver) const
{
  stringstream buf;
  buf << historyHeader(isReceiver);
  buf << ':' << myName << '\n';
  buf << ':' << myEmail << '\n';
  buf << addStrWithColons(myMessage);
  writeUserHistory(u, buf.str());
}

Licq::EventContactList::Contact::Contact(const UserId& userId, const string& alias)
  : myUserId(userId),
    myAlias(alias)
{
  // Empty
}

//====CEventContactList========================================================
Licq::EventContactList::EventContactList(const ContactList& cl, bool bDeep,
    time_t tTime, unsigned long nFlags)
  : UserEvent(TypeContactList, 0, tTime, nFlags)
{
  if (bDeep)
    for(ContactList::const_iterator it = cl.begin(); it != cl.end(); ++it)
      m_vszFields.push_back(new Contact((*it)->userId(), (*it)->alias()));
  else
    m_vszFields = cl;
}


void Licq::EventContactList::CreateDescription() const
{
  stringstream buf;
  buf << gTranslator.toUtf8(tr("Contact list")) << " (" << m_vszFields.size()
      << ' ' << gTranslator.toUtf8(tr("contacts")) << "):\n";
  ContactList::const_iterator iter;
  for (iter = m_vszFields.begin(); iter != m_vszFields.end(); ++iter)
    buf << (*iter)->alias() << " (" << (*iter)->userId().toString() << ")\n";
  myText = buf.str();
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
      m_tTime, m_nFlags);
  e->CopyBase(this);
  return e;
}

void Licq::EventContactList::AddToHistory(User* u, bool isReceiver) const
{
  stringstream buf;
  buf << historyHeader(isReceiver);
  ContactList::const_iterator iter;
  for (iter = m_vszFields.begin(); iter != m_vszFields.end(); ++iter)
  {
    buf << ':' << (*iter)->userId().accountId() << '\n';
    buf << ':' << (*iter)->alias() << '\n';
  }
  writeUserHistory(u, buf.str());
}

//=====CEventSms===============================================================
Licq::EventSms::EventSms(const string& number, const string& message,
    time_t _tTime, unsigned long _nFlags)
  : UserEvent(TypeSms, 0, _tTime, _nFlags),
    myNumber(number),
    myMessage(message)
{
  // Empty
}

void Licq::EventSms::CreateDescription() const
{
  myText = gTranslator.toUtf8(tr("Phone"));
  myText += ": ";
  myText += myNumber;
  myText += '\n';
  myText += myMessage;
  myText += '\n';
}

Licq::EventSms* Licq::EventSms::Copy() const
{
  EventSms* e = new EventSms(myNumber, myMessage, m_tTime, m_nFlags);
  e->CopyBase(this);
  return e;
}

void Licq::EventSms::AddToHistory(User* u, bool isReceiver) const
{
  string str;
  str = historyHeader(isReceiver);
  str += ':';
  str += myNumber;
  str += '\n';
  str += addStrWithColons(myMessage);
  writeUserHistory(u, str);
}

//=====CEventServerMessage=====================================================
Licq::EventServerMessage::EventServerMessage(const string& name,
    const string& email, const string& message, time_t _tTime)
  : UserEvent(TypeMsgServer, 0, _tTime, 0),
    myName(name),
    myEmail(email),
    myMessage(message)
{
  // Empty
}

void Licq::EventServerMessage::CreateDescription() const
{
  myText = gTranslator.toUtf8(tr("System Server Message from"));
  myText += ' ';
  myText += myName;
  myText += " (";
  myText += myEmail;
  myText += "):\n";
  myText += myMessage;
  myText += '\n';
}

Licq::EventServerMessage* Licq::EventServerMessage::Copy() const
{
  EventServerMessage* e = new EventServerMessage(myName, myEmail, myMessage, m_tTime);
  e->CopyBase(this);
  return e;
}

void Licq::EventServerMessage::AddToHistory(User* u, bool isReceiver) const
{
  stringstream buf;
  buf << historyHeader(isReceiver);
  buf << ':' << myName << '\n';
  buf << ':' << myEmail << '\n';
  buf << addStrWithColons(myMessage);
  writeUserHistory(u, buf.str());
}

//=====CEventEmailAlert=====================================================
Licq::EventEmailAlert::EventEmailAlert(const string& name, const string& to,
    const string& email, const string& subject, time_t _tTime, const string& mspAuth,
    const string& sid, const string& kv, const string& id, const string& postUrl,
    const string& msgUrl, const string& creds, unsigned long sessionLength)
  : UserEvent(TypeEmailAlert, 0, _tTime, FlagDirect),
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
  myText = gTranslator.toUtf8(tr("New Email from"));
  myText += ' ';
  myText += myName;
  myText += " (";
  myText += myEmail;
  myText += "):\n";
  myText += gTranslator.toUtf8(tr("Subject"));
  myText += ": ";
  myText += mySubject;
  myText += '\n';
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
  stringstream buf;
  buf << historyHeader(isReceiver);
  buf << ':' << myName << '\n';
  buf << ':' << myEmail << '\n';
  buf << addStrWithColons(mySubject);
  writeUserHistory(u, buf.str());
}

//=====CEventUnknownSysMsg=====================================================
Licq::EventUnknownSysMsg::EventUnknownSysMsg(unsigned short _nSubCommand,
    unsigned short _nCommand, const UserId& userId, const string& message,
                             time_t _tTime, unsigned long _nFlags)
  : UserEvent(TypeUnknownSys, 0, _tTime, _nFlags | FlagUnknown),
    m_nCommand(_nCommand),
    m_nSubCommand(_nSubCommand),
    myUserId(userId),
    myMessage(message)
{
  // Empty
}

void Licq::EventUnknownSysMsg::CreateDescription() const
{
  stringstream buf;
  buf << gTranslator.toUtf8(tr("Unknown system message")) << " (0x";
  buf.width(4);
  buf.fill('0');
  buf.setf(stringstream::hex, stringstream::basefield);
  buf << m_nSubCommand;
  buf << ") " << gTranslator.toUtf8(tr("from")) << ' ' << myUserId.toString() << ":\n";
  buf << myMessage << '\n';
  myText = buf.str();
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
/*
  string str;
  str = historyHeader(isReceiver);
  str += ':';
  str += myUserId.accountId();
  str += '\n';
  str += addStrWithColons(myMessage);
  writeUserHistory(u, str);
*/
}


string UserEvent::description() const
{
  string desc = eventName();
  if (IsCancelled())
    desc += tr(" (cancelled)");
  return desc;
}

