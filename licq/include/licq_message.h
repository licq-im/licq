#ifndef MESSAGE_H
#define MESSAGE_H

#include <list>
#include <time.h>
#include "licq_buffer.h"
#include "licq_constants.h"
#include "licq_color.h"

#define EVENT_HEADER_SIZE  80

// Define some event flags, leave the 2 LSB's for the licq version
const unsigned long E_LICQxVER      = 0x0000FFFF;
const unsigned long E_DIRECT        = 0x00010000;
const unsigned long E_MULTIxREC     = 0x00020000;
const unsigned long E_URGENT        = 0x00040000;
const unsigned long E_CANCELLED     = 0x00080000;
const unsigned long E_ENCRYPTED     = 0x00100000;
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
   const char *Text();
   const char *Description();
   time_t Time()  {  return m_tTime; }
   const char *LicqVersionStr();
   static const char *LicqVersionToString(unsigned long);
   unsigned long Sequence()  { return m_nSequence; }
   unsigned short Command()  { return m_nCommand; }
   unsigned short SubCommand()  { return m_nSubCommand; }
   int Id()  { return m_nId; }
   bool IsDirect()  { return m_nFlags & E_DIRECT; }
   bool IsMultiRec()  { return m_nFlags & E_MULTIxREC; }
   bool IsUrgent()    { return m_nFlags & E_URGENT; }
   bool IsCancelled() { return m_nFlags & E_CANCELLED; }
   bool IsLicq()  { return LicqVersion() != 0; };
   bool IsEncrypted()  { return m_nFlags & E_ENCRYPTED; };
   unsigned short LicqVersion()  { return m_nFlags & E_LICQxVER; }
   direction Direction()  {  return m_eDir; }
   CICQColor *Color() { return &m_sColor; }

   bool Pending() { return m_bPending; }
   void SetPending(bool b)  { m_bPending = b; }

protected:
   virtual void AddToHistory(ICQUser *, direction) = 0;
   int AddToHistory_Header(direction, char *);
   void AddToHistory_Flush(ICQUser *, char *);

   void SetDirection(direction d)  { m_eDir = d; }
   void Cancel() { m_nFlags |= E_CANCELLED; }
   void SetColor(unsigned long fore, unsigned long back)  { m_sColor.Set(fore, back); }
   void SetColor(CICQColor *p)  { m_sColor.Set(p); }

   void CopyBase(CUserEvent *);

   virtual void CreateDescription() = 0;
   static int s_nId;

   char          *m_szText;
   unsigned short m_nCommand;
   unsigned short m_nSubCommand;
   unsigned long  m_nSequence;
   int            m_nId;
   time_t         m_tTime;
   unsigned long  m_nFlags;

   direction      m_eDir;
   bool           m_bPending;
   CICQColor      m_sColor;

friend class CICQDaemon;
friend class CUserHistory;
};



//-----CEventMsg-------------------------------------------------------------
class CEventMsg : public CUserEvent
{
public:
   CEventMsg(const char *_szMessage, unsigned short _nCommand,
             time_t _tTime, unsigned long _nFlags);
   virtual ~CEventMsg();
   virtual CEventMsg *Copy()
      {
        CEventMsg *e = new CEventMsg(m_szMessage, m_nCommand, m_tTime, m_nFlags);
        e->CopyBase(this);
        return e;
      }
   const char *Message()  { return m_szMessage; }
   virtual void AddToHistory(ICQUser *, direction);

   static CEventMsg *Parse(char *sz, unsigned short nCmd, time_t nTime, unsigned long nFlags);
protected:
   void CreateDescription();
   char *m_szMessage;
};


//-----CEventFile---------------------------------------------------------------
class CEventFile : public CUserEvent
{
public:
   CEventFile(const char *_szFilename, const char *_szFileDescription,
              unsigned long _nFileSize, unsigned long _nSequence, time_t _tTime,
              unsigned long _nFlags, unsigned long _nMsgID1 = 0,
              unsigned long _nMsgID2 = 0);
   virtual ~CEventFile();
   virtual void AddToHistory(ICQUser *, direction);
   virtual CEventFile *Copy()
      {
        CEventFile *e = new CEventFile(m_szFilename, m_szFileDescription,
         m_nFileSize, m_nSequence, m_tTime, m_nFlags, m_nMsgID[0], m_nMsgID[1]);
        e->CopyBase(this);
        return e;
      }

   const char *Filename()  { return m_szFilename; }
   unsigned long FileSize()  {  return m_nFileSize; }
   const char *FileDescription() { return m_szFileDescription; }
	 unsigned long *MessageID() { return m_nMsgID; }
protected:
   void CreateDescription();
   char *m_szFilename;
   char *m_szFileDescription;
   unsigned long m_nFileSize;
   unsigned long m_nMsgID[2];
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
      {
        CEventUrl *e = new CEventUrl(m_szUrl, m_szUrlDescription, m_nCommand,
         m_tTime, m_nFlags);
        e->CopyBase(this);
        return e;
      }
   const char *Url()  { return m_szUrl; }
   const char *Description()  { return m_szUrlDescription; }

   static CEventUrl *Parse(char *sz, unsigned short nCmd, time_t nTime, unsigned long nFlags);
protected:
   void CreateDescription();
   char *m_szUrl;
   char *m_szUrlDescription;
};


//-----CEventChat---------------------------------------------------------------
class CEventChat : public CUserEvent
{
public:
   CEventChat(const char *szReason, unsigned long nSequence, time_t tTime,
      unsigned long nFlags, unsigned long nMsgID1 = 0, unsigned long nMsgID2=0);
   CEventChat(const char *szReason, const char *szClients, unsigned short nPort,
      unsigned long nSequence, time_t tTime, unsigned long nFlags,
      unsigned long nMsgID1 = 0, unsigned long nMsgID2 = 0);
  virtual ~CEventChat();
  virtual void AddToHistory(ICQUser *, direction);
  virtual CEventChat *Copy()
     {
       CEventChat *e = new CEventChat(m_szText, m_szClients, m_nPort,
        m_nSequence, m_tTime, m_nFlags, m_nMsgID[0], m_nMsgID[1]);
       e->CopyBase(this);
       return e;
     }
  const char *Reason()  { return m_szReason; }
  const char *Clients()  { return m_szClients; }
  unsigned short Port()   { return m_nPort; }
  unsigned long *MessageID() { return m_nMsgID; }
protected:
  void CreateDescription();
  char *m_szReason;
  char *m_szClients;
  unsigned short m_nPort;
  unsigned long m_nMsgID[2];
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
      {
        CEventAdded *e = new CEventAdded(m_nUin, m_szAlias, m_szFirstName,
         m_szLastName, m_szEmail, m_nCommand, m_tTime, m_nFlags);
        e->CopyBase(this);
        return e;
      }
   unsigned long Uin()  { return m_nUin; };

protected:
   void CreateDescription();
   unsigned long m_nUin;
   char *m_szAlias;
   char *m_szFirstName;
   char *m_szLastName;
   char *m_szEmail;
};



//-----CEventAuthReq---------------------------------------------------------
class CEventAuthRequest : public CUserEvent
{
public:
   CEventAuthRequest(unsigned long _nUin, const char *_szAlias, const char *_szFirstName,
                 const char *_szLastName, const char *_szEmail, const char *_szReason,
                 unsigned short _nCommand, time_t _tTime, unsigned long _nFlags);
   virtual ~CEventAuthRequest();
   virtual void AddToHistory(ICQUser *, direction);
   virtual CEventAuthRequest *Copy()
      {
        CEventAuthRequest *e = new CEventAuthRequest(m_nUin, m_szAlias,
         m_szFirstName, m_szLastName, m_szEmail, m_szReason, m_nCommand,
         m_tTime, m_nFlags);
        e->CopyBase(this);
        return e;
      }
   unsigned long Uin()  { return m_nUin; };
protected:
   void CreateDescription();
   unsigned long m_nUin;
   char *m_szAlias;
   char *m_szFirstName;
   char *m_szLastName;
   char *m_szEmail;
   char *m_szReason;
};


//-----CEventAuthGranted-------------------------------------------------------
class CEventAuthGranted : public CUserEvent
{
public:
   CEventAuthGranted(unsigned long _nUin, const char *_szMessage,
              unsigned short _nCommand, time_t _tTime, unsigned long _nFlags);
   virtual ~CEventAuthGranted();
   virtual void AddToHistory(ICQUser *, direction);
   virtual CEventAuthGranted *Copy()
      {
        CEventAuthGranted *e = new CEventAuthGranted(m_nUin, m_szMessage,
         m_nCommand, m_tTime, m_nFlags);
        e->CopyBase(this);
        return e;
      }
   unsigned long Uin()  { return m_nUin; };
protected:
   void CreateDescription();
   unsigned long m_nUin;
   char *m_szMessage;
};


//-----CEventAuthRefused------------------------------------------------------
class CEventAuthRefused : public CUserEvent
{
public:
   CEventAuthRefused(unsigned long _nUin, const char *_szMessage,
              unsigned short _nCommand, time_t _tTime, unsigned long _nFlags);
   virtual ~CEventAuthRefused();
   virtual void AddToHistory(ICQUser *, direction);
   virtual CEventAuthRefused *Copy()
      {
        CEventAuthRefused *e = new CEventAuthRefused(m_nUin, m_szMessage,
         m_nCommand, m_tTime, m_nFlags);
        e->CopyBase(this);
        return e;
      }
   unsigned long Uin()  { return m_nUin; };
protected:
   void CreateDescription();
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
      {
        CEventWebPanel *e = new CEventWebPanel(m_szName, m_szEmail, m_szMessage,
         m_nCommand, m_tTime, m_nFlags);
        e->CopyBase(this);
        return e;
      }
protected:
   void CreateDescription();
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
      {
        CEventEmailPager *e = new CEventEmailPager(m_szName, m_szEmail,
         m_szMessage, m_nCommand, m_tTime, m_nFlags);
        e->CopyBase(this);
        return e;
      }
protected:
   void CreateDescription();
   char *m_szName;
   char *m_szEmail;
   char *m_szMessage;
};


//-----CEventContactList----------------------------------------------------------
class CContact
{
public:
  CContact(unsigned long n, const char *a) { m_nUin = n; m_szAlias = strdup(a); }
  ~CContact() { free(m_szAlias); }

  unsigned long Uin() { return m_nUin; }
  const char *Alias() { return m_szAlias; }
protected:
  unsigned long m_nUin;
  char *m_szAlias;
};
typedef std::list<CContact *> ContactList;


class CEventContactList : public CUserEvent
{
public:
  CEventContactList(ContactList &cl, bool bDeep, unsigned short nCommand,
     time_t tTime, unsigned long nFlags);
  virtual ~CEventContactList();
  virtual void AddToHistory(ICQUser *, direction);
  virtual CEventContactList *Copy()
    {
      CEventContactList *e = new CEventContactList(m_vszFields, true,
       m_nCommand, m_tTime, m_nFlags);
      e->CopyBase(this);
      return e;
    }

  const ContactList &Contacts() { return m_vszFields; }

  static CEventContactList *Parse(char *sz, unsigned short nCmd, time_t nTime, unsigned long nFlags);
protected:
  void CreateDescription();
  ContactList m_vszFields;
};


//-----CEventSms---------------------------------------------------------------
class CEventSms : public CUserEvent
{
public:
   CEventSms(const char *_szNumber, const char *_szMessage, 
	     unsigned short _nCommand, time_t _tTime, unsigned long _nFlags);
   virtual ~CEventSms();
   virtual CEventSms *Copy()
   {
     CEventSms *e = new CEventSms(m_szNumber, m_szMessage, m_nCommand, m_tTime, m_nFlags);
     e->CopyBase(this);
     return e;
   }
   const char *Number()  { return m_szNumber; }
   const char *Message()  { return m_szMessage; }
   virtual void AddToHistory(ICQUser *, direction);

   static CEventSms *Parse(char *sz, unsigned short nCmd, time_t nTime, unsigned long nFlags);
protected:
   void CreateDescription();
   char *m_szNumber;
   char *m_szMessage;
};

//-----CEventServerMessage-----------------------------------------------------
class CEventServerMessage : public CUserEvent
{
public:
  CEventServerMessage(const char *_szName, const char *_szEmail,
                      const char *_szMessage, time_t _tTime);
  virtual ~CEventServerMessage();

  virtual void AddToHistory(ICQUser *, direction);

  virtual CEventServerMessage *Copy()
  {
    CEventServerMessage *e = new CEventServerMessage(m_szName, m_szEmail,
		    m_szMessage, m_tTime);
    e->CopyBase(this);
    return e;
  }

  static CEventServerMessage *Parse(char *, unsigned short, time_t, unsigned long);

protected:
  void CreateDescription();

  char *m_szName,
       *m_szEmail,
       *m_szMessage;
};

//-----CEventPlugin------------------------------------------------------------
class CEventPlugin : public CUserEvent
{
public:
  CEventPlugin(const char *sz, unsigned short nSubCommand,
     time_t tTime, unsigned long nFlags);
  ~CEventPlugin();
  virtual void AddToHistory(ICQUser *, direction);
  virtual CEventPlugin *Copy()
    { return new CEventPlugin(m_sz, m_nSubCommand, m_tTime, m_nFlags); }
protected:
  void CreateDescription();
  char *m_sz;
};


//-----CEventUnknownSysMsg-----------------------------------------------------
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
    {
      CEventUnknownSysMsg *e = new CEventUnknownSysMsg(m_nSubCommand,
       m_nCommand, m_nUin, m_szMsg, m_tTime, m_nFlags);
      e->CopyBase(this);
      return e;
    }
protected:
   void CreateDescription();
   unsigned long m_nUin;
   char *m_szMsg;
};

#endif
