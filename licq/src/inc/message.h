#ifndef MESSAGE_H
#define MESSAGE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "time-fix.h"

#include <vector.h>
#include "buffer.h"
#include "constants.h"

#define EVENT_HEADER_SIZE  80

// Define some event flags, leave the 2 LSB's for the licq version
const unsigned long E_LICQxVER      = 0x0000FFFF;
const unsigned long E_DIRECT        = 0x00010000;
const unsigned long E_MULTIxREC     = 0x00020000;
const unsigned long E_URGENT        = 0x00040000;
const unsigned long E_UNKNOWN       = 0x80000000;

class ICQUser;

//=====CUserEvent===============================================================
class CUserEvent
{
public:
   CUserEvent(unsigned short _nSubCommand, unsigned short _nCommand,
              unsigned long _nSequence, time_t _tTime,
              unsigned long _nFlags);
   CUserEvent(const CUserEvent *);
   virtual ~CUserEvent();
   virtual CUserEvent *Copy() = 0;
   virtual void AddToHistory(ICQUser *, direction) = 0;

   int AddToHistory_Header(direction, char *);
   void AddToHistory_Flush(ICQUser *, char *);

   const char *Text()  {  return m_szText; }
   time_t Time()  {  return m_tTime; }
   const char *LicqVersionStr();
   unsigned long Sequence()  { return m_nSequence; }
   unsigned short Command()  { return m_nCommand; }
   unsigned short SubCommand()  { return m_nSubCommand; }
   bool IsDirect()  { return m_nFlags & E_DIRECT; }
   bool IsMultiRec()  { return m_nFlags & E_MULTIxREC; }
   bool IsUrgent()    { return m_nFlags & E_URGENT; }
   bool IsLicq()  { return LicqVersion() != 0; };
   unsigned short LicqVersion()  { return m_nFlags & E_LICQxVER; }
   direction Direction()  {  return m_eDir; }
   void SetDirection(direction d)  { m_eDir = d; }

protected:
   char *m_szText;
   direction m_eDir;
   unsigned short m_nCommand,
                  m_nSubCommand;
   unsigned long  m_nSequence;
   time_t         m_tTime;
   unsigned long  m_nFlags;
};


//-----CEventMsg----------------------------------------------------------------
class CEventMsg : public CUserEvent
{
public:
   CEventMsg(const char *_szMessage, unsigned short _nCommand,
             time_t _tTime, unsigned long _nFlags);
   virtual ~CEventMsg();
   virtual CEventMsg *Copy()
      { return (new CEventMsg(m_szMessage, m_nCommand, m_tTime, m_nFlags)); };
   const char *Message()  { return m_szMessage; }
   virtual void AddToHistory(ICQUser *, direction);
protected:
   char *m_szMessage;
};


//-----CEventFile---------------------------------------------------------------
class CEventFile : public CUserEvent
{
public:
   CEventFile(const char *_szFilename, const char *_szFileDescription,
              unsigned long _nFileSize, unsigned long _nSequence, time_t _tTime,
              unsigned long _nFlags);
   virtual ~CEventFile();
   virtual void AddToHistory(ICQUser *, direction);
   virtual CEventFile *Copy()
      { return (new CEventFile(m_szFilename, m_szFileDescription, m_nFileSize,
                               m_nSequence, m_tTime, m_nFlags)); };

   const char *Filename()  { return m_szFilename; };
   unsigned long FileSize()  {  return m_nFileSize; };
protected:
   char *m_szFilename;
   char *m_szFileDescription;
   unsigned long m_nFileSize;
};


//-----CEventFileCancel---------------------------------------------------------
class CEventFileCancel : public CUserEvent
{
public:
   CEventFileCancel(unsigned long _nSequence, time_t _tTime,
                    unsigned long _nFlags);
   virtual ~CEventFileCancel();
   virtual void AddToHistory(ICQUser *, direction);
   virtual CEventFileCancel *Copy()
      { return (new CEventFileCancel(m_nSequence, m_tTime, m_nFlags)); };
};


//-----CEventUrl----------------------------------------------------------------
class CEventUrl : public CUserEvent
{
public:
   CEventUrl(const char *_szUrl, const char *_szUrlDescription,
             unsigned short _nCommand, time_t _tTime,
             unsigned long _nFlags);
   virtual ~CEventUrl();
   virtual void AddToHistory(ICQUser *, direction);
   virtual CEventUrl *Copy()
      { return (new CEventUrl(m_szUrl, m_szUrlDescription, m_nCommand, m_tTime,
                              m_nFlags)); }
   const char *Url()  { return m_szUrl; }
   const char *Description()  { return m_szUrlDescription; }
protected:
   char *m_szUrl;
   char *m_szUrlDescription;
};


//-----CEventChat---------------------------------------------------------------
class CEventChat : public CUserEvent
{
public:
   CEventChat(const char *_szReason, unsigned long _nSequence, time_t _tTime,
              unsigned long _nFlags);
   virtual ~CEventChat();
   virtual void AddToHistory(ICQUser *, direction);
   virtual CEventChat *Copy()
      { return (new CEventChat(m_szText, m_nSequence, m_tTime, m_nFlags)); };
};


//-----CEventChatCancel---------------------------------------------------------
class CEventChatCancel : public CUserEvent
{
public:
   CEventChatCancel(unsigned long _nSequence, time_t _tTime,
                    unsigned long _nFlags);
   virtual ~CEventChatCancel();
   virtual void AddToHistory(ICQUser *, direction);
   virtual CEventChatCancel *Copy()
      { return (new CEventChatCancel(m_nSequence, m_tTime, m_nFlags)); };
};


//-----CEventAdded--------------------------------------------------------------
class CEventAdded : public CUserEvent
{
public:
   CEventAdded(unsigned long _nUin, const char *_szAlias, const char *_szFirstName,
               const char *_szLastName, const char *_szEmail,
               unsigned short _nCommand, time_t _tTime, unsigned long _nFlags);
   virtual ~CEventAdded();
   virtual void AddToHistory(ICQUser *, direction);
   virtual CEventAdded *Copy()
      { return (new CEventAdded(m_nUin, m_szAlias, m_szFirstName, m_szLastName,
                                m_szEmail, m_nCommand, m_tTime, m_nFlags)); };

protected:
   unsigned long m_nUin;
   char *m_szAlias;
   char *m_szFirstName;
   char *m_szLastName;
   char *m_szEmail;
};



//-----CEventAuthReq---------------------------------------------------------
class CEventAuthReq : public CUserEvent
{
public:
   CEventAuthReq(unsigned long _nUin, const char *_szAlias, const char *_szFirstName,
                 const char *_szLastName, const char *_szEmail, const char *_szReason,
                 unsigned short _nCommand, time_t _tTime, unsigned long _nFlags);
   virtual ~CEventAuthReq();
   virtual void AddToHistory(ICQUser *, direction);
   virtual CEventAuthReq *Copy()
      { return (new CEventAuthReq(m_nUin, m_szAlias, m_szFirstName, m_szLastName,
                               m_szEmail, m_szReason, m_nCommand, m_tTime,
                               m_nFlags)); };
   unsigned long Uin()  { return m_nUin; };
protected:
   unsigned long m_nUin;
   char *m_szAlias;
   char *m_szFirstName;
   char *m_szLastName;
   char *m_szEmail;
   char *m_szReason;
};


//-----CEventAuth------------------------------------------------------------
class CEventAuth : public CUserEvent
{
public:
   CEventAuth(unsigned long _nUin, const char *_szMessage,
              unsigned short _nCommand, time_t _tTime, unsigned long _nFlags);
   virtual ~CEventAuth();
   virtual void AddToHistory(ICQUser *, direction);
   virtual CEventAuth *Copy()
      { return (new CEventAuth(m_nUin, m_szMessage, m_nCommand, m_tTime,
                               m_nFlags)); };
   unsigned long Uin()  { return m_nUin; };
protected:
   unsigned long m_nUin;
   char *m_szMessage;
};


//-----CEventWebPanel----------------------------------------------------------
class CEventWebPanel : public CUserEvent
{
public:
   CEventWebPanel(const char *_szName, char *_szEmail, const char *_szMessage,
                   unsigned short _nCommand, time_t _tTime, unsigned long _nFlags);
   virtual ~CEventWebPanel();
   virtual void AddToHistory(ICQUser *, direction);
   virtual CEventWebPanel *Copy()
      { return (new CEventWebPanel(m_szName, m_szEmail, m_szMessage, m_nCommand,
                                    m_tTime, m_nFlags)); }
protected:
   char *m_szName;
   char *m_szEmail;
   char *m_szMessage;
};


//-----CEventEmailPager----------------------------------------------------------
class CEventEmailPager : public CUserEvent
{
public:
   CEventEmailPager(const char *_szName, char *_szEmail, const char *_szMessage,
                    unsigned short _nCommand, time_t _tTime, unsigned long _nFlags);
   virtual ~CEventEmailPager();
   virtual void AddToHistory(ICQUser *, direction);
   virtual CEventEmailPager *Copy()
      { return (new CEventEmailPager(m_szName, m_szEmail, m_szMessage, m_nCommand,
                                     m_tTime, m_nFlags)); }
protected:
   char *m_szName;
   char *m_szEmail;
   char *m_szMessage;
};


//-----CEventContactList----------------------------------------------------------
class CEventContactList : public CUserEvent
{
public:
  CEventContactList(vector <char *> &_vszFields,
                    unsigned short _nCommand,
                    time_t _tTime, unsigned long _nFlags);
  virtual ~CEventContactList();
  virtual void AddToHistory(ICQUser *, direction);
  virtual CEventContactList *Copy()
    { return (new CEventContactList(m_vszFields, m_nCommand, m_tTime, m_nFlags)); }
protected:
  vector <char *> m_vszFields;
};

//-----CEventSaved--------------------------------------------------------------
class CEventSaved : public CUserEvent
{
public:
  CEventSaved(unsigned short _nNumEvents);
  virtual void AddToHistory(ICQUser *, direction) {}
  virtual CEventSaved *Copy()
    { return (new CEventSaved(m_nNumEvents)); };

protected:
   unsigned short m_nNumEvents;
};


class CEventUnknownSysMsg : public CUserEvent
{
public:
  CEventUnknownSysMsg(unsigned short _nSubCommand,
                unsigned short _nCommand, unsigned long _nUin,
                const char *_szMsg,
                time_t _tTime, unsigned long _nFlags);
  ~CEventUnknownSysMsg();
  virtual void AddToHistory(ICQUser *, direction);
  virtual CEventUnknownSysMsg *Copy()
    { return (new CEventUnknownSysMsg(m_nSubCommand, m_nCommand,
                                  m_nUin, m_szMsg,
                                  m_tTime, m_nFlags)); }
protected:
   unsigned long m_nUin;
   char *m_szMsg;
};

#endif
