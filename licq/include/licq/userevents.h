/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2011 Licq developers
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
namespace Jabber { class Plugin; }

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
    FlagSender          = 0x00200000,
    FlagOffline         = 0x00400000,
    FlagUnknown         = 0x80000000,
  };

  // Event type, for now these must be the same as the ICQ protocol sub commands
  enum EventType
  {
    TypeMessage         = 0x0001,
    TypeChat            = 0x0002,
    TypeFile            = 0x0003,
    TypeUrl             = 0x0004,
    TypeAuthRequest     = 0x0006,
    TypeAuthRefused     = 0x0007,
    TypeAuthGranted     = 0x0008,
    TypeMsgServer       = 0x0009,
    TypeAdded           = 0x000C,
    TypeWebPanel        = 0x000D,
    TypeEmailPager      = 0x000E,
    TypeContactList     = 0x0013,
    TypeSms             = 0x001A,
    TypeEmailAlert      = 0x00EC,
    TypeUnknownSys      = 0xFFFF,
  };

  // Only used for history files, must be same as ICQ protocol commands
  enum HistoryCommand
  {
    CommandDirect       = 0x07EE,
    CommandSent         = 0x010E,
    CommandRcvOnline    = 0x0104,
    CommandRcvOffline   = 0x00DC,
    CommandCancelled    = 0x07D0,
  };

  static const time_t TimeNow = 0;

  UserEvent(EventType eventType,
              unsigned short _nSequence, time_t _tTime,
              unsigned long _nFlags, unsigned long _nConvoId = 0);
  UserEvent(const UserEvent *);
  virtual ~UserEvent();

  virtual UserEvent* Copy() const = 0;
  const std::string& text() const;
  std::string description() const;
  time_t Time() const { return m_tTime; }
  const std::string licqVersionStr() const
  { return licqVersionToString(LicqVersion()); }

  static const std::string licqVersionToString(unsigned long);
  unsigned short Sequence() const { return m_nSequence; }

  /// Get type of event
  unsigned eventType() const { return myEventType; }

  /// Returns translated event name
  virtual std::string eventName() const = 0;

  int Id() const { return m_nId; }
  bool IsDirect() const { return m_nFlags & FlagDirect; }
  bool IsMultiRec() const { return m_nFlags & FlagMultiRec; }
  bool IsUrgent() const { return m_nFlags & FlagUrgent; }
  bool IsCancelled() const { return m_nFlags & FlagCancelled; }
  bool IsLicq() const { return LicqVersion() != 0; };
  bool IsEncrypted() const { return m_nFlags & FlagEncrypted; };
  unsigned short LicqVersion() const { return m_nFlags & FlagLicqVerMask; }

  /// Was message sent from us
  bool isSender() const { return m_nFlags & FlagSender; }

  /// Was message sent while receiver was offline
  bool isOffline() const { return m_nFlags & FlagOffline; }

  bool isReceiver() const { return !isSender(); }
  const Color* color() const { return &myColor; }
  unsigned long ConvoId() const { return m_nConvoId; }

  bool Pending() const { return m_bPending; }
   void SetPending(bool b)  { m_bPending = b; }

protected:
  virtual void AddToHistory(User* user, bool isReceiver) const = 0;

  /**
   * Construct a header for the history entry
   *
   * @param isReceived True if message was received
   * @return String with header data formated for history
   */
  std::string historyHeader(bool isReceiver) const;

  /**
   * Write to user history
   *
   * @param User to write to, must be write locked
   * @param text History entry, including header and formatting
   */
  void writeUserHistory(User* u, const std::string& text) const;

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
  mutable std::string myText;
  unsigned myEventType;
   unsigned short m_nSequence;
   int            m_nId;
   time_t         m_tTime;
   unsigned long  m_nFlags;

   bool           m_bPending;
  Color myColor;
   unsigned long  m_nConvoId;

  friend class ::IcqProtocol;
  friend class LicqDaemon::Daemon;
  friend class ::CMSN;
  friend class Jabber::Plugin;
  friend class User;
};


//-----CEventMsg-------------------------------------------------------------
class EventMsg : public UserEvent
{
public:
  EventMsg(const std::string& message,
             time_t _tTime, unsigned long _nFlags, unsigned long _nConvoId = 0);

  virtual EventMsg* Copy() const;
  const std::string& message() const { return myMessage; }
  virtual void AddToHistory(User* u, bool isReceiver) const;

protected:
  void CreateDescription() const;
  std::string eventName() const;

  std::string myMessage;
};


//-----CEventFile---------------------------------------------------------------
class EventFile : public UserEvent
{
public:
  EventFile(const std::string& filename, const std::string& fileDescription,
      unsigned long _nFileSize, const std::list<std::string>& lFileList,
              unsigned short _nSequence, time_t _tTime,
              unsigned long _nFlags, unsigned long _nConovId = 0,
              unsigned long _nMsgID1 = 0, unsigned long _nMsgID2 = 0);
  virtual EventFile* Copy() const;
  virtual void AddToHistory(User* u, bool isReceiver) const;

  const std::string& filename() const { return myFilename; }
  unsigned long FileSize() const {  return m_nFileSize; }
  const std::string& fileDescription() const { return myFileDescription; }
  const std::list<std::string>& FileList() const { return m_lFileList; }
  const unsigned long* MessageID() const { return m_nMsgID; }
protected:
  void CreateDescription() const;
  std::string eventName() const;

  std::string myFilename;
  std::string myFileDescription;
   unsigned long m_nFileSize;
  std::list<std::string> m_lFileList;
   unsigned long m_nMsgID[2];
};


//-----CEventUrl----------------------------------------------------------------
class EventUrl : public UserEvent
{
public:
  EventUrl(const std::string& url, const std::string& urlDescription,
      time_t _tTime, unsigned long _nFlags, unsigned long _nConvoId = 0);
  virtual EventUrl* Copy() const;
  virtual void AddToHistory(User* u, bool isReceiver) const;
  const std::string& url() const { return myUrl; }
  const std::string& urlDescription() const { return myUrlDescription; }

  static EventUrl *Parse(char *sz, time_t nTime,
     unsigned long nFlags, unsigned long nConvoId = 0);
protected:
  void CreateDescription() const;
  std::string eventName() const;

  std::string myUrl;
  std::string myUrlDescription;
};


//-----CEventChat---------------------------------------------------------------
class EventChat : public UserEvent
{
public:
  EventChat(const std::string& reason, unsigned short nSequence, time_t tTime,
      unsigned long nFlags, unsigned long nConvoId = 0, unsigned long nMsgID1 = 0,
      unsigned long nMsgID2 = 0);
  EventChat(const std::string& reason, const std::string& clients, unsigned short nPort,
      unsigned short nSequence, time_t tTime, unsigned long nFlags,
      unsigned long _nConvoId = 0, unsigned long nMsgID1 = 0, unsigned long nMsgID2 = 0);
  virtual EventChat* Copy() const;
  virtual void AddToHistory(User* u, bool isReceiver) const;
  const std::string& reason() const { return myReason; }
  const std::string& clients() const { return myClients; }
  unsigned short Port() const { return m_nPort; }
  const unsigned long* MessageID() const { return m_nMsgID; }
protected:
  void CreateDescription() const;
  std::string eventName() const;

  std::string myReason;
  std::string myClients;
  unsigned short m_nPort;
  unsigned long m_nMsgID[2];
};


//-----CEventAdded--------------------------------------------------------------
class EventAdded : public UserEvent
{
public:
  EventAdded(const UserId& userId, const std::string& alias,
      const std::string& firstName, const std::string& lastName,
      const std::string& email, time_t _tTime, unsigned long _nFlags);
  virtual EventAdded* Copy() const;
  virtual void AddToHistory(User* u, bool isReceiver) const;
  const UserId& userId() const { return myUserId; }

protected:
  void CreateDescription() const;
  std::string eventName() const;

  UserId myUserId;
  std::string myAlias;
  std::string myFirstName;
  std::string myLastName;
  std::string myEmail;
};


//-----CEventAuthReq---------------------------------------------------------
class EventAuthRequest : public UserEvent
{
public:
  EventAuthRequest(const UserId& userId, const std::string& alias,
      const std::string& firstName, const std::string& lastName,
      const std::string& email, const std::string& reason,
      time_t _tTime, unsigned long _nFlags);
  virtual EventAuthRequest* Copy() const;
  virtual void AddToHistory(User* u, bool isReceiver) const;
  const UserId& userId() const { return myUserId; }
  const std::string& reason() const { return myReason; }

protected:
  void CreateDescription() const;
  std::string eventName() const;

  UserId myUserId;
  std::string myAlias;
  std::string myFirstName;
  std::string myLastName;
  std::string myEmail;
  std::string myReason;
};


//-----CEventAuthGranted-------------------------------------------------------
class EventAuthGranted : public UserEvent
{
public:
  EventAuthGranted(const UserId& userId, const std::string& message,
      time_t _tTime, unsigned long _nFlags);
  virtual EventAuthGranted* Copy() const;
  virtual void AddToHistory(User* u, bool isReceiver) const;
  const UserId& userId() const { return myUserId; }
  const std::string& message() const { return myMessage; }

protected:
  void CreateDescription() const;
  std::string eventName() const;

  UserId myUserId;
  std::string myMessage;
};


//-----CEventAuthRefused------------------------------------------------------
class EventAuthRefused : public UserEvent
{
public:
  EventAuthRefused(const UserId& userId, const std::string& message,
      time_t _tTime, unsigned long _nFlags);
  virtual EventAuthRefused* Copy() const;
  virtual void AddToHistory(User* u, bool isReceiver) const;
  const UserId& userId() const { return myUserId; }
  const std::string& message() const { return myMessage; }

protected:
  void CreateDescription() const;
  std::string eventName() const;

  UserId myUserId;
  std::string myMessage;
};


//-----CEventWebPanel----------------------------------------------------------
class EventWebPanel : public UserEvent
{
public:
  EventWebPanel(const std::string& name, const std::string& email,
      const std::string& message, time_t _tTime, unsigned long _nFlags);
  virtual EventWebPanel* Copy() const;
  virtual void AddToHistory(User* u, bool isReceiver) const;
  const std::string& message() const { return myMessage; }

protected:
  void CreateDescription() const;
  std::string eventName() const;

  std::string myName;
  std::string myEmail;
  std::string myMessage;
};


//-----CEventEmailPager----------------------------------------------------------
class EventEmailPager : public UserEvent
{
public:
  EventEmailPager(const std::string& name, const std::string& email,
      const std::string& message, time_t _tTime, unsigned long _nFlags);
  virtual EventEmailPager* Copy() const;
  virtual void AddToHistory(User* u, bool isReceiver) const;
  const std::string& message() const { return myMessage; }

protected:
  void CreateDescription() const;
  std::string eventName() const;

  std::string myName;
  std::string myEmail;
  std::string myMessage;
};


//-----CEventContactList----------------------------------------------------------
class EventContactList : public UserEvent
{
public:
  class Contact
  {
  public:
    Contact(const UserId& userId, const std::string& alias);

    const UserId& userId() const { return myUserId; }
    const std::string& alias() const { return myAlias; }

  protected:
    UserId myUserId;
    std::string myAlias;
  };
  typedef std::list<Contact *> ContactList;


  EventContactList(const ContactList& cl, bool bDeep,
     time_t tTime, unsigned long nFlags);
  virtual ~EventContactList();
  virtual EventContactList* Copy() const;
  virtual void AddToHistory(User* u, bool isReceiver) const;

  const ContactList &Contacts() const { return m_vszFields; }

  static EventContactList *Parse(char *sz, time_t nTime, unsigned long nFlags);
protected:
  void CreateDescription() const;
  std::string eventName() const;

  ContactList m_vszFields;
};


//-----CEventSms---------------------------------------------------------------
class EventSms : public UserEvent
{
public:
  EventSms(const std::string& number, const std::string& message,
      time_t _tTime, unsigned long _nFlags);
  virtual EventSms* Copy() const;
  const std::string& number() const { return myNumber; }
  const std::string& message() const { return myMessage; }
  virtual void AddToHistory(User* u, bool isReceiver) const;

  static EventSms* Parse(const std::string& s, time_t nTime, unsigned long nFlags);
protected:
  void CreateDescription() const;
  std::string eventName() const;

  std::string myNumber;
  std::string myMessage;
};

//-----CEventServerMessage-----------------------------------------------------
class EventServerMessage : public UserEvent
{
public:
  EventServerMessage(const std::string& name, const std::string& email,
      const std::string& message, time_t _tTime);
  virtual EventServerMessage* Copy() const;
  const std::string& message() const { return myMessage; }
  virtual void AddToHistory(User* u, bool isReceiver) const;

  static EventServerMessage *Parse(char *, unsigned short, time_t, unsigned long);

protected:
 void CreateDescription() const;
  std::string eventName() const;

  std::string myName;
  std::string myEmail;
  std::string myMessage;
};

//-----CEventEmailAlert-----------------------------------------------------
class EventEmailAlert : public UserEvent
{
public:
  EventEmailAlert(const std::string& name, const std::string& email,
      const std::string& to, const std::string& subject, time_t _tTime,
      const std::string& mspAuth = "", const std::string& sid = "",
      const std::string& kv = "", const std::string& id = "",
      const std::string& postUrl = "", const std::string& msgUrl = "",
      const std::string& creds = "", unsigned long sessionLength = 0);
  virtual EventEmailAlert* Copy() const;
  virtual void AddToHistory(User* u, bool isReceiver) const;

  const std::string& from() const { return myName; }
  const std::string& to() const { return myTo; }
  const std::string& email() const { return myEmail; }
  const std::string& subject() const { return mySubject; }

  const std::string& mspAuth() const { return myMspAuth; }
  const std::string& sid() const { return mySid; }
  const std::string& kv() const { return myKv; }
  const std::string& id() const { return myId; }
  const std::string& postUrl() const { return myPostUrl; }
  const std::string& msgUrl() const { return myMsgUrl; }
  const std::string& creds() const { return myCreds; }
  unsigned long sessionLength() const { return mySessionLength; }

protected:
  void CreateDescription() const;
  std::string eventName() const;

  // Info
  std::string myName;
  std::string myTo;
  std::string myEmail;
  std::string mySubject;

  // For Licq to view an MSN email
  std::string myMspAuth;
  std::string mySid;
  std::string myKv;
  std::string myId;
  std::string myPostUrl;
  std::string myMsgUrl;
  std::string myCreds;
  unsigned long mySessionLength;
};

//-----CEventUnknownSysMsg-----------------------------------------------------
class EventUnknownSysMsg : public UserEvent
{
public:
  EventUnknownSysMsg(unsigned short _nSubCommand, unsigned short _nCommand,
      const UserId& userId, const std::string& message, time_t _tTime, unsigned long _nFlags);
  virtual EventUnknownSysMsg* Copy() const;
  virtual void AddToHistory(User* u, bool isReceiver) const;
  unsigned short subCommand() const { return m_nSubCommand; }
  unsigned short Command() const { return m_nCommand; }

protected:
  void CreateDescription() const;
  std::string eventName() const;

  unsigned short m_nCommand;
  unsigned short m_nSubCommand;
  UserId myUserId;
  std::string myMessage;
};

} // namespace Licq

#endif
