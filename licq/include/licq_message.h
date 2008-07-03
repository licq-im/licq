#ifndef MESSAGE_H
#define MESSAGE_H

#include <cstdlib>
#include <ctime>
#include <list>

#include "licq_constants.h"
#include "licq_color.h"

// Define for marking functions as deprecated
#ifndef LICQ_DEPRECATED
# if defined(__GNUC__) && !defined(__INTEL_COMPILER) && (__GNUC__ - 0 > 3 || (__GNUC__ - 0 == 3 && __GNUC_MINOR__ - 0 >= 2))
#  define LICQ_DEPRECATED __attribute__ ((__deprecated__))
# elif defined(_MSC_VER) && (_MSC_VER >= 1300)
#  define LICQ_DEPRECATED __declspec(deprecated)
# else
#  define LICQ_DEPRECATED
# endif
#endif


typedef std::list<const char *> ConstFileList;

#define LICQ_PPID 0x4C696371  // "Licq"
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
              unsigned short _nSequence, time_t _tTime,
              unsigned long _nFlags, unsigned long _nConvoId = 0);
   CUserEvent(const CUserEvent *);
   virtual ~CUserEvent();

  virtual CUserEvent* Copy() const = 0;
  const char* Text() const;
  const char* Description() const;
  time_t Time() const { return m_tTime; }
  const char* LicqVersionStr() const;
   static const char *LicqVersionToString(unsigned long);
  unsigned short Sequence() const { return m_nSequence; }
  unsigned short Command() const { return m_nCommand; }
  unsigned short SubCommand() const { return m_nSubCommand; }
  int Id() const { return m_nId; }
  bool IsDirect() const { return m_nFlags & E_DIRECT; }
  bool IsMultiRec() const { return m_nFlags & E_MULTIxREC; }
  bool IsUrgent() const { return m_nFlags & E_URGENT; }
  bool IsCancelled() const { return m_nFlags & E_CANCELLED; }
  bool IsLicq() const { return LicqVersion() != 0; };
  bool IsEncrypted() const { return m_nFlags & E_ENCRYPTED; };
  unsigned short LicqVersion() const { return m_nFlags & E_LICQxVER; }
  direction Direction() const { return m_eDir; }
  const CICQColor* Color() const { return &m_sColor; }
  unsigned long ConvoId() const { return m_nConvoId; }

  bool Pending() const { return m_bPending; }
   void SetPending(bool b)  { m_bPending = b; }

protected:
  virtual void AddToHistory(ICQUser* user, unsigned long ppid, direction dir) const = 0;
  int AddToHistory_Header(direction _nDir, char* szOut) const;
  void AddToHistory_Flush(ICQUser* u, unsigned long nPPID, const char* szOut) const;

   void SetDirection(direction d)  { m_eDir = d; }
   void Cancel() { m_nFlags |= E_CANCELLED; }
   void SetColor(unsigned long fore, unsigned long back)  { m_sColor.Set(fore, back); }
  void SetColor(CICQColor const* p) { m_sColor.Set(p); }

  void CopyBase(const CUserEvent* e);

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

   direction      m_eDir;
   bool           m_bPending;
   CICQColor      m_sColor;
   unsigned long  m_nConvoId;

friend class CICQDaemon;
friend class CMSN;
friend class CUserHistory;
};



//-----CEventMsg-------------------------------------------------------------
class CEventMsg : public CUserEvent
{
public:
   CEventMsg(const char *_szMessage, unsigned short _nCommand,
             time_t _tTime, unsigned long _nFlags, unsigned long _nConvoId = 0);
   virtual ~CEventMsg();

  virtual CEventMsg* Copy() const;
  const char* Message() const { return m_szMessage; }
  virtual void AddToHistory(ICQUser* u, unsigned long _nPPID, direction _nDir) const;

   static CEventMsg *Parse(char *sz, unsigned short nCmd, time_t nTime,
                           unsigned long nFlags, unsigned long nConvoId = 0);
protected:
  void CreateDescription() const;
   char *m_szMessage;
};


//-----CEventFile---------------------------------------------------------------
class CEventFile : public CUserEvent
{
public:
   CEventFile(const char *_szFilename, const char *_szFileDescription,
      unsigned long _nFileSize, const ConstFileList& lFileList,
              unsigned short _nSequence, time_t _tTime,
              unsigned long _nFlags, unsigned long _nConovId = 0,
              unsigned long _nMsgID1 = 0, unsigned long _nMsgID2 = 0);
   virtual ~CEventFile();
  virtual CEventFile* Copy() const;
  virtual void AddToHistory(ICQUser* u, unsigned long _nPPID, direction _nDir) const;

  const char* Filename() const { return m_szFilename; }
  unsigned long FileSize() const {  return m_nFileSize; }
  const char* FileDescription() const { return m_szFileDescription; }
  ConstFileList FileList() const { return m_lFileList; }
  const unsigned long* MessageID() const { return m_nMsgID; }
protected:
  void CreateDescription() const;
   char *m_szFilename;
   char *m_szFileDescription;
   unsigned long m_nFileSize;
   ConstFileList m_lFileList;
   unsigned long m_nMsgID[2];
};


//-----CEventUrl----------------------------------------------------------------
class CEventUrl : public CUserEvent
{
public:
   CEventUrl(const char *_szUrl, const char *_szUrlDescription,
             unsigned short _nCommand, time_t _tTime,
             unsigned long _nFlags, unsigned long _nConvoId = 0);
   virtual ~CEventUrl();
  virtual CEventUrl* Copy() const;
  virtual void AddToHistory(ICQUser* u, unsigned long _nPPID, direction _nDir) const;
  const char* Url() const { return m_szUrl; }
  const char* Description() const { return m_szUrlDescription; }

   static CEventUrl *Parse(char *sz, unsigned short nCmd, time_t nTime,
     unsigned long nFlags, unsigned long nConvoId = 0);
protected:
  void CreateDescription() const;
   char *m_szUrl;
   char *m_szUrlDescription;
};


//-----CEventChat---------------------------------------------------------------
class CEventChat : public CUserEvent
{
public:
   CEventChat(const char *szReason, unsigned short nSequence, time_t tTime,
      unsigned long nFlags, unsigned long nConvoId = 0, unsigned long nMsgID1 = 0,
      unsigned long nMsgID2 = 0);
   CEventChat(const char *szReason, const char *szClients, unsigned short nPort,
      unsigned short nSequence, time_t tTime, unsigned long nFlags,
      unsigned long _nConvoId = 0, unsigned long nMsgID1 = 0, unsigned long nMsgID2 = 0);
  virtual ~CEventChat();
  virtual CEventChat* Copy() const;
  virtual void AddToHistory(ICQUser* u, unsigned long _nPPID, direction _nDir) const;
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
class CEventAdded : public CUserEvent
{
public:
   CEventAdded(const char *_szId, unsigned long _nPPID, const char *_szAlias,
               const char *_szFirstName, const char *_szLastName, const char *_szEmail,
               unsigned short _nCommand, time_t _tTime, unsigned long _nFlags);
   virtual ~CEventAdded();
  virtual CEventAdded* Copy() const;
  virtual void AddToHistory(ICQUser* u, unsigned long _nPPID, direction _nDir) const;
  const char* IdString() const { return m_szId; }
  unsigned long PPID() const { return m_nPPID; }

  // Deprecated functions, to be removed
  CEventAdded(unsigned long _nUin, const char* _szAlias,
      const char* _szFirstName, const char *_szLastName, const char *_szEmail,
      unsigned short _nCommand, time_t _tTime, unsigned long _nFlags)
      LICQ_DEPRECATED;
  LICQ_DEPRECATED unsigned long Uin() const { return strtoul(m_szId, NULL, 10); };

protected:
  void CreateDescription() const;
   char *m_szId;
   unsigned long m_nPPID;
   char *m_szAlias;
   char *m_szFirstName;
   char *m_szLastName;
   char *m_szEmail;
};



//-----CEventAuthReq---------------------------------------------------------
class CEventAuthRequest : public CUserEvent
{
public:
   CEventAuthRequest(const char *_szId, unsigned long _nPPID, const char *_szAlias,
                     const char *_szFirstName, const char *_szLastName, const char *_szEmail,
                     const char *_szReason, unsigned short _nCommand, time_t _tTime,
                     unsigned long _nFlags);
   virtual ~CEventAuthRequest();
  virtual CEventAuthRequest* Copy() const;
  virtual void AddToHistory(ICQUser* u, unsigned long _nPPID, direction _nDir) const;
  const char* IdString() const { return m_szId; }
  unsigned long PPID() const { return m_nPPID; }

  // Deprecated functions, to be removed
  CEventAuthRequest(unsigned long _nUin, const char* _szAlias,
      const char* _szFirstName, const char* _szLastName, const char* _szEmail,
      const char* _szReason, unsigned short _nCommand, time_t _tTime,
      unsigned long _nFlags) LICQ_DEPRECATED;
  LICQ_DEPRECATED unsigned long Uin() const { return strtoul(m_szId, NULL, 10); };

protected:
  void CreateDescription() const;
   char *m_szId;
   unsigned long m_nPPID;
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
   CEventAuthGranted(const char *_szId, unsigned long _nPPID, const char *_szMsg,
                     unsigned short _nCommand, time_t _tTime, unsigned long _nFlags);
   virtual ~CEventAuthGranted();
  virtual CEventAuthGranted* Copy() const;
  virtual void AddToHistory(ICQUser* u, unsigned long _nPPID, direction _nDir) const;
  const char* IdString() const { return m_szId; }
  unsigned long PPID() const { return m_nPPID; }

  // Deprecated functions, to be removed
  CEventAuthGranted(unsigned long _nUin, const char* _szMessage,
      unsigned short _nCommand, time_t _tTime, unsigned long _nFlags)
      LICQ_DEPRECATED;
  LICQ_DEPRECATED unsigned long Uin() const { return strtoul(m_szId, NULL, 10); };

protected:
  void CreateDescription() const;
   char *m_szId;
   unsigned long m_nPPID;
   char *m_szMessage;
};


//-----CEventAuthRefused------------------------------------------------------
class CEventAuthRefused : public CUserEvent
{
public:
   CEventAuthRefused(const char *_szId, unsigned long _nPPID, const char *_szMsg,
                     unsigned short _nCommand, time_t _tTime, unsigned long _nFlags);
   virtual ~CEventAuthRefused();
  virtual CEventAuthRefused* Copy() const;
  virtual void AddToHistory(ICQUser* u, unsigned long _nPPID, direction _nDir) const;
  const char* IdString() const { return m_szId; }
  unsigned long PPID() const { return m_nPPID; }

  // Deprecated functions, to be removed
  CEventAuthRefused(unsigned long _nUin, const char* _szMessage,
      unsigned short _nCommand, time_t _tTime, unsigned long _nFlags)
      LICQ_DEPRECATED;
  LICQ_DEPRECATED unsigned long Uin() const { return strtoul(m_szId, NULL, 10); };

protected:
  void CreateDescription() const;
   char *m_szId;
   unsigned long m_nPPID;
   char *m_szMessage;
};


//-----CEventWebPanel----------------------------------------------------------
class CEventWebPanel : public CUserEvent
{
public:
   CEventWebPanel(const char *_szName, char *_szEmail, const char *_szMessage,
                   unsigned short _nCommand, time_t _tTime, unsigned long _nFlags);
   virtual ~CEventWebPanel();
  virtual CEventWebPanel* Copy() const;
  virtual void AddToHistory(ICQUser* u, unsigned long _nPPID, direction _nDir) const;
protected:
  void CreateDescription() const;
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
  virtual CEventEmailPager* Copy() const;
  virtual void AddToHistory(ICQUser* u, unsigned long _nPPID, direction _nDir) const;
protected:
  void CreateDescription() const;
   char *m_szName;
   char *m_szEmail;
   char *m_szMessage;
};


//-----CEventContactList----------------------------------------------------------
class CContact
{
public:
  CContact(const char *s, unsigned long n, const char *a);
  ~CContact();

  const char* Alias() const { return m_szAlias; }
  const char* IdString() const { return m_szId; }
  unsigned long PPID() const { return m_nPPID; }

  // Deprecated functions, to be removed
  CContact(unsigned long n, const char* a) LICQ_DEPRECATED;
  LICQ_DEPRECATED unsigned long Uin() const { return strtoul(m_szId, NULL, 10); }

protected:
  char *m_szAlias;
  char *m_szId;
  unsigned long m_nPPID;
};
typedef std::list<CContact *> ContactList;


class CEventContactList : public CUserEvent
{
public:
  CEventContactList(const ContactList& cl, bool bDeep, unsigned short nCommand,
     time_t tTime, unsigned long nFlags);
  virtual ~CEventContactList();
  virtual CEventContactList* Copy() const;
  virtual void AddToHistory(ICQUser* u, unsigned long _nPPID, direction _nDir) const;

  const ContactList &Contacts() const { return m_vszFields; }

  static CEventContactList *Parse(char *sz, unsigned short nCmd, time_t nTime, unsigned long nFlags);
protected:
  void CreateDescription() const;
  ContactList m_vszFields;
};


//-----CEventSms---------------------------------------------------------------
class CEventSms : public CUserEvent
{
public:
   CEventSms(const char *_szNumber, const char *_szMessage,
             unsigned short _nCommand, time_t _tTime, unsigned long _nFlags);
   virtual ~CEventSms();
  virtual CEventSms* Copy() const;
  const char* Number() const { return m_szNumber; }
  const char* Message() const { return m_szMessage; }
  virtual void AddToHistory(ICQUser* u, unsigned long _nPPID, direction _nDir) const;

   static CEventSms *Parse(char *sz, unsigned short nCmd, time_t nTime, unsigned long nFlags);
protected:
  void CreateDescription() const;
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
  virtual CEventServerMessage* Copy() const;
  virtual void AddToHistory(ICQUser* u, unsigned long _nPPID, direction _nDir) const;

  static CEventServerMessage *Parse(char *, unsigned short, time_t, unsigned long);

protected:
 void CreateDescription() const;

  char *m_szName,
       *m_szEmail,
       *m_szMessage;
};

//-----CEventEmailAlert-----------------------------------------------------
class CEventEmailAlert : public CUserEvent
{
public:
  CEventEmailAlert(const char *_szName, const char *_szEmail,
                   const char *_szTo, const char *_szSubject, time_t _tTime,
                   const char *_szMSPAuth = 0, const char *_szSID = 0,
                   const char *_szKV = 0, const char *_szId = 0,
                   const char *_szPostURL = 0, const char *_szMsgURL = 0,
                   const char *_szCreds = 0, unsigned long _nSessionLength = 0);
  virtual ~CEventEmailAlert();
  virtual CEventEmailAlert* Copy() const;
  virtual void AddToHistory(ICQUser* u, unsigned long _nPPID, direction _nDir) const;

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
class CEventPlugin : public CUserEvent
{
public:
  CEventPlugin(const char *sz, unsigned short nSubCommand,
     time_t tTime, unsigned long nFlags);
  ~CEventPlugin();
  virtual CEventPlugin* Copy() const;
  virtual void AddToHistory(ICQUser* user, unsigned long ppid, direction dir) const;
protected:
  void CreateDescription() const;
  char *m_sz;
};


//-----CEventUnknownSysMsg-----------------------------------------------------
class CEventUnknownSysMsg : public CUserEvent
{
public:
  CEventUnknownSysMsg(unsigned short _nSubCommand,
                unsigned short _nCommand, const char *_szId, unsigned long _nPPID,
                const char *_szMsg,
                time_t _tTime, unsigned long _nFlags);
  ~CEventUnknownSysMsg();
  virtual CEventUnknownSysMsg* Copy() const;
  virtual void AddToHistory(ICQUser* u, unsigned long _nPPID, direction _nDir) const;
protected:
  void CreateDescription() const;
   char *m_szMsg;
   char *m_szId;
   unsigned long m_nPPID;
};

#endif
