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

#ifndef LICQ_USEREVENTS_H
#define LICQ_USEREVENTS_H

#include <cstdlib>
#include <ctime>
#include <list>

#include "color.h"
#include "userid.h"

class CMSN;
class IcqProtocol;
class Jabber;

namespace LicqDaemon
{
class Daemon;
class UserHistory;
}

namespace Licq
{
class User;


//=====CUserEvent===============================================================
class UserEvent
{
public:
  // Define some event flags, leave the 2 LSB's for the licq version
  enum Flags
  {
    FlagLicqVerMask     = 0x0000ffff,
    FlagDirect          = 0x00010000,
    FlagMultiRec        = 0x00020000,
    FlagUrgent          = 0x00040000,
    FlagCancelled       = 0x00080000,
    FlagEncrypted       = 0x00100000,
    FlagUnknown         = 0x80000000,
  };

  // Use this constant for constructor to get current time
  static const time_t TimeNow = 0;

  UserEvent(unsigned short _nSubCommand, unsigned short _nCommand,
              unsigned short _nSequence, time_t _tTime,
              unsigned long _nFlags, unsigned long _nConvoId = 0);
  UserEvent(const UserEvent *);
  virtual ~UserEvent();

  virtual UserEvent* Copy() const = 0;
  const char* Text() const;
  const char* Description() const;
  time_t Time() const { return m_tTime; }
  const char* LicqVersionStr() const;
   static const char *LicqVersionToString(unsigned long);
  unsigned short Sequence() const { return m_nSequence; }
  unsigned short Command() const { return m_nCommand; }
  unsigned short SubCommand() const { return m_nSubCommand; }
  int Id() const { return m_nId; }
  bool IsDirect() const { return m_nFlags & FlagDirect; }
  bool IsMultiRec() const { return m_nFlags & FlagMultiRec; }
  bool IsUrgent() const { return m_nFlags & FlagUrgent; }
  bool IsCancelled() const { return m_nFlags & FlagCancelled; }
  bool IsLicq() const { return LicqVersion() != 0; };
  bool IsEncrypted() const { return m_nFlags & FlagEncrypted; };
  unsigned short LicqVersion() const { return m_nFlags & FlagLicqVerMask; }
  bool isReceiver() const { return myIsReceiver; }
  const Color* color() const { return &myColor; }
  unsigned long ConvoId() const { return m_nConvoId; }

  bool Pending() const { return m_bPending; }
   void SetPending(bool b)  { m_bPending = b; }

protected:
  virtual void AddToHistory(User* user, bool isReceiver) const = 0;
  int AddToHistory_Header(bool isReceiver, char* szOut) const;
  void AddToHistory_Flush(User* u, const char* szOut) const;

  void setIsReceiver(bool isReceiver) { myIsReceiver = isReceiver; }
  void Cancel() { m_nFlags |= FlagCancelled; }
  void SetColor(unsigned fore, unsigned back) { myColor.set(fore, back); }
  void SetColor(const Color* c) { myColor.set(c); }

  void CopyBase(const UserEvent* e);

  // CreateDescription() is const even though it changes m_szText since
  // it may be called to perform delayed initialization in const context.
  virtual void CreateDescription() const = 0;
   static int s_nId;

  // m_szText is not initialized until it is accessed. Allow this delayed
  // initialization even if called in const context.
  mutable char* m_szText;
   unsigned short m_nCommand;
   unsigned short m_nSubCommand;
   unsigned short m_nSequence;
   int            m_nId;
   time_t         m_tTime;
   unsigned long  m_nFlags;

  bool myIsReceiver;
   bool           m_bPending;
  Color myColor;
   unsigned long  m_nConvoId;

  friend class ::IcqProtocol;
  friend class LicqDaemon::Daemon;
  friend class ::CMSN;
  friend class ::Jabber;
  friend class LicqDaemon::UserHistory;
  friend class User;
};


//-----CEventMsg-------------------------------------------------------------
class EventMsg : public UserEvent
{
public:
  EventMsg(const char *_szMessage, unsigned short _nCommand,
             time_t _tTime, unsigned long _nFlags, unsigned long _nConvoId = 0);
  virtual ~EventMsg();

  virtual EventMsg* Copy() const;
  const char* Message() const { return m_szMessage; }
  virtual void AddToHistory(User* u, bool isReceiver) const;

  static EventMsg *Parse(char *sz, unsigned short nCmd, time_t nTime,
                           unsigned long nFlags, unsigned long nConvoId = 0);
protected:
  void CreateDescription() const;
   char *m_szMessage;
};


//-----CEventFile---------------------------------------------------------------
class EventFile : public UserEvent
{
public:
  EventFile(const char *_szFilename, const char *_szFileDescription,
      unsigned long _nFileSize, const std::list<std::string>& lFileList,
              unsigned short _nSequence, time_t _tTime,
              unsigned long _nFlags, unsigned long _nConovId = 0,
              unsigned long _nMsgID1 = 0, unsigned long _nMsgID2 = 0);
  virtual ~EventFile();
  virtual EventFile* Copy() const;
  virtual void AddToHistory(User* u, bool isReceiver) const;

  const char* Filename() const { return m_szFilename; }
  unsigned long FileSize() const {  return m_nFileSize; }
  const char* FileDescription() const { return m_szFileDescription; }
  const std::list<std::string>& FileList() const { return m_lFileList; }
  const unsigned long* MessageID() const { return m_nMsgID; }
protected:
  void CreateDescription() const;
   char *m_szFilename;
   char *m_szFileDescription;
   unsigned long m_nFileSize;
  std::list<std::string> m_lFileList;
   unsigned long m_nMsgID[2];
};


//-----CEventUrl----------------------------------------------------------------
class EventUrl : public UserEvent
{
public:
  EventUrl(const char *_szUrl, const char *_szUrlDescription,
             unsigned short _nCommand, time_t _tTime,
             unsigned long _nFlags, unsigned long _nConvoId = 0);
  virtual ~EventUrl();
  virtual EventUrl* Copy() const;
  virtual void AddToHistory(User* u, bool isReceiver) const;
  const char* Url() const { return m_szUrl; }
  const char* Description() const { return m_szUrlDescription; }

  static EventUrl *Parse(char *sz, unsigned short nCmd, time_t nTime,
     unsigned long nFlags, unsigned long nConvoId = 0);
protected:
  void CreateDescription() const;
   char *m_szUrl;
   char *m_szUrlDescription;
};


//-----CEventChat---------------------------------------------------------------
class EventChat : public UserEvent
{
public:
  EventChat(const char *szReason, unsigned short nSequence, time_t tTime,
      unsigned long nFlags, unsigned long nConvoId = 0, unsigned long nMsgID1 = 0,
      unsigned long nMsgID2 = 0);
  EventChat(const char *szReason, const char *szClients, unsigned short nPort,
      unsigned short nSequence, time_t tTime, unsigned long nFlags,
      unsigned long _nConvoId = 0, unsigned long nMsgID1 = 0, unsigned long nMsgID2 = 0);
  virtual ~EventChat();
  virtual EventChat* Copy() const;
  virtual void AddToHistory(User* u, bool isReceiver) const;
  const char* Reason() const { return m_szReason; }
  const char* Clients() const { return m_szClients; }
  unsigned short Port() const { return m_nPort; }
  const unsigned long* MessageID() const { return m_nMsgID; }
protected:
  void CreateDescription() const;
  char *m_szReason;
  char *m_szClients;
  unsigned short m_nPort;
  unsigned long m_nMsgID[2];
};


//-----CEventAdded--------------------------------------------------------------
class EventAdded : public UserEvent
{
public:
  EventAdded(const UserId& userId, const char *_szAlias,
               const char *_szFirstName, const char *_szLastName, const char *_szEmail,
               unsigned short _nCommand, time_t _tTime, unsigned long _nFlags);
  virtual ~EventAdded();
  virtual EventAdded* Copy() const;
  virtual void AddToHistory(User* u, bool isReceiver) const;
  const UserId& userId() const { return myUserId; }

protected:
  void CreateDescription() const;
  UserId myUserId;
   char *m_szAlias;
   char *m_szFirstName;
   char *m_szLastName;
   char *m_szEmail;
};


//-----CEventAuthReq---------------------------------------------------------
class EventAuthRequest : public UserEvent
{
public:
  EventAuthRequest(const UserId& userId, const char *_szAlias,
                     const char *_szFirstName, const char *_szLastName, const char *_szEmail,
                     const char *_szReason, unsigned short _nCommand, time_t _tTime,
                     unsigned long _nFlags);
  virtual ~EventAuthRequest();
  virtual EventAuthRequest* Copy() const;
  virtual void AddToHistory(User* u, bool isReceiver) const;
  const UserId& userId() const { return myUserId; }

protected:
  void CreateDescription() const;
  UserId myUserId;
   char *m_szAlias;
   char *m_szFirstName;
   char *m_szLastName;
   char *m_szEmail;
   char *m_szReason;
};


//-----CEventAuthGranted-------------------------------------------------------
class EventAuthGranted : public UserEvent
{
public:
  EventAuthGranted(const UserId& userId, const char *_szMsg,
                     unsigned short _nCommand, time_t _tTime, unsigned long _nFlags);
  virtual ~EventAuthGranted();
  virtual EventAuthGranted* Copy() const;
  virtual void AddToHistory(User* u, bool isReceiver) const;
  const UserId& userId() const { return myUserId; }

protected:
  void CreateDescription() const;
  UserId myUserId;
   char *m_szMessage;
};


//-----CEventAuthRefused------------------------------------------------------
class EventAuthRefused : public UserEvent
{
public:
  EventAuthRefused(const UserId& userId, const char *_szMsg,
                     unsigned short _nCommand, time_t _tTime, unsigned long _nFlags);
  virtual ~EventAuthRefused();
  virtual EventAuthRefused* Copy() const;
  virtual void AddToHistory(User* u, bool isReceiver) const;
  const UserId& userId() const { return myUserId; }

protected:
  void CreateDescription() const;
  UserId myUserId;
   char *m_szMessage;
};


//-----CEventWebPanel----------------------------------------------------------
class EventWebPanel : public UserEvent
{
public:
  EventWebPanel(const char *_szName, char *_szEmail, const char *_szMessage,
                   unsigned short _nCommand, time_t _tTime, unsigned long _nFlags);
  virtual ~EventWebPanel();
  virtual EventWebPanel* Copy() const;
  virtual void AddToHistory(User* u, bool isReceiver) const;
protected:
  void CreateDescription() const;
   char *m_szName;
   char *m_szEmail;
   char *m_szMessage;
};


//-----CEventEmailPager----------------------------------------------------------
class EventEmailPager : public UserEvent
{
public:
  EventEmailPager(const char *_szName, char *_szEmail, const char *_szMessage,
                    unsigned short _nCommand, time_t _tTime, unsigned long _nFlags);
  virtual ~EventEmailPager();
  virtual EventEmailPager* Copy() const;
  virtual void AddToHistory(User* u, bool isReceiver) const;
protected:
  void CreateDescription() const;
   char *m_szName;
   char *m_szEmail;
   char *m_szMessage;
};


//-----CEventContactList----------------------------------------------------------
class EventContactList : public UserEvent
{
public:
  class Contact
  {
  public:
    Contact(const UserId& userId, const char *a);
    ~Contact();

    const UserId& userId() const { return myUserId; }
    const char* Alias() const { return m_szAlias; }

  protected:
    UserId myUserId;
    char *m_szAlias;
  };
  typedef std::list<Contact *> ContactList;


  EventContactList(const ContactList& cl, bool bDeep, unsigned short nCommand,
     time_t tTime, unsigned long nFlags);
  virtual ~EventContactList();
  virtual EventContactList* Copy() const;
  virtual void AddToHistory(User* u, bool isReceiver) const;

  const ContactList &Contacts() const { return m_vszFields; }

  static EventContactList *Parse(char *sz, unsigned short nCmd, time_t nTime, unsigned long nFlags);
protected:
  void CreateDescription() const;
  ContactList m_vszFields;
};


//-----CEventSms---------------------------------------------------------------
class EventSms : public UserEvent
{
public:
  EventSms(const char *_szNumber, const char *_szMessage,
             unsigned short _nCommand, time_t _tTime, unsigned long _nFlags);
  virtual ~EventSms();
  virtual EventSms* Copy() const;
  const char* Number() const { return m_szNumber; }
  const char* Message() const { return m_szMessage; }
  virtual void AddToHistory(User* u, bool isReceiver) const;

  static EventSms *Parse(char *sz, unsigned short nCmd, time_t nTime, unsigned long nFlags);
protected:
  void CreateDescription() const;
   char *m_szNumber;
   char *m_szMessage;
};

//-----CEventServerMessage-----------------------------------------------------
class EventServerMessage : public UserEvent
{
public:
  EventServerMessage(const char *_szName, const char *_szEmail,
                      const char *_szMessage, time_t _tTime);
  virtual ~EventServerMessage();
  virtual EventServerMessage* Copy() const;
  virtual void AddToHistory(User* u, bool isReceiver) const;

  static EventServerMessage *Parse(char *, unsigned short, time_t, unsigned long);

protected:
 void CreateDescription() const;

  char *m_szName,
       *m_szEmail,
       *m_szMessage;
};

//-----CEventEmailAlert-----------------------------------------------------
class EventEmailAlert : public UserEvent
{
public:
  EventEmailAlert(const char *_szName, const char *_szEmail,
                   const char *_szTo, const char *_szSubject, time_t _tTime,
                   const char *_szMSPAuth = 0, const char *_szSID = 0,
                   const char *_szKV = 0, const char *_szId = 0,
                   const char *_szPostURL = 0, const char *_szMsgURL = 0,
                   const char *_szCreds = 0, unsigned long _nSessionLength = 0);
  virtual ~EventEmailAlert();
  virtual EventEmailAlert* Copy() const;
  virtual void AddToHistory(User* u, bool isReceiver) const;

  const char* From() const { return m_szName; }
  const char* To() const { return m_szTo; }
  const char* Email() const { return m_szEmail; }
  const char* Subject() const { return m_szSubject; }

  const char* MSPAuth() const { return m_szMSPAuth; }
  const char* SID() const { return m_szSID; }
  const char* KV() const { return m_szKV; }
  const char* Id() const { return m_szId; }
  const char* PostURL() const { return m_szPostURL; }
  const char* MsgURL() const { return m_szMsgURL; }
  const char* Creds() const { return m_szCreds; }
  unsigned long SessionLength() const { return m_nSessionLength; }

protected:
  void CreateDescription() const;

  // Info
  char *m_szName,
       *m_szTo,
       *m_szEmail,
       *m_szSubject;

  // For Licq to view an MSN email
  char *m_szMSPAuth,
       *m_szSID,
       *m_szKV,
       *m_szId,
       *m_szPostURL,
       *m_szMsgURL,
       *m_szCreds;
  unsigned long m_nSessionLength;
};

//-----CEventPlugin------------------------------------------------------------
class EventPlugin : public UserEvent
{
public:
  EventPlugin(const char *sz, unsigned short nSubCommand,
     time_t tTime, unsigned long nFlags);
  ~EventPlugin();
  virtual EventPlugin* Copy() const;
  virtual void AddToHistory(User* user, bool isReceiver) const;
protected:
  void CreateDescription() const;
  char *m_sz;
};


//-----CEventUnknownSysMsg-----------------------------------------------------
class EventUnknownSysMsg : public UserEvent
{
public:
  EventUnknownSysMsg(unsigned short _nSubCommand, unsigned short _nCommand,
      const UserId& userId, const char *_szMsg, time_t _tTime, unsigned long _nFlags);
  ~EventUnknownSysMsg();
  virtual EventUnknownSysMsg* Copy() const;
  virtual void AddToHistory(User* u, bool isReceiver) const;
protected:
  void CreateDescription() const;
  UserId myUserId;
   char *m_szMsg;
};

} // namespace Licq

#endif
