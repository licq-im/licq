#ifndef MESSAGE_H
#define MESSAGE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "time-fix.h"

#include <vector.h>
#include "buffer.h"
#include "constants.h"


// Define some event flags, leave the 2 LSB's for the licq version
const unsigned long E_LICQxVER      = 0x0000FFFF;
const unsigned long E_DIRECT        = 0x00010000;
const unsigned long E_MULTIxREC     = 0x00020000;
const unsigned long E_URGENT        = 0x00040000;

class ICQUser;

//=====CUserEvent===============================================================
class CUserEvent
{
public:
   CUserEvent(unsigned short _nSubCommand, unsigned short _nCommand,
              unsigned long _nSequence, time_t _tTime,
              unsigned long _nFlags);
   CUserEvent(const CUserEvent *);
   virtual ~CUserEvent(void);
   virtual CUserEvent *Copy(void) = 0;

   void AddToHistory(ICQUser *, direction);

   const char *Text(void)  {  return m_szText; };
   const char *Description(void) {  return m_szDescription; }
   const char *Time(void);
   const char *LicqVersionStr(void);
   unsigned long Sequence(void)  { return m_nSequence; }
   unsigned short Command(void)  { return m_nCommand; }
   unsigned short SubCommand(void)  { return m_nSubCommand; }
   bool IsDirect(void)  { return m_nFlags & E_DIRECT; }
   bool IsMultiRec(void)  { return m_nFlags & E_MULTIxREC; }
   bool IsUrgent(void)    { return m_nFlags & E_URGENT; }
   bool IsLicq(void)  { return LicqVersion() != 0; };
   unsigned short LicqVersion(void)  { return m_nFlags & E_LICQxVER; };

protected:
   char *m_szText,
        *m_szDescription;
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
   virtual ~CEventMsg(void);
   virtual CEventMsg *Copy(void)
      { return (new CEventMsg(m_szMessage, m_nCommand, m_tTime, m_nFlags)); };
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
   virtual ~CEventFile(void);
   virtual CEventFile *Copy(void)
      { return (new CEventFile(m_szFilename, m_szFileDescription, m_nFileSize, 
                               m_nSequence, m_tTime, m_nFlags)); };

   const char *Filename(void)  { return m_szFilename; };
   unsigned long FileSize(void)  {  return m_nFileSize; };
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
   virtual ~CEventFileCancel(void);
   virtual CEventFileCancel *Copy(void)
      { return (new CEventFileCancel(m_nSequence, m_tTime, m_nFlags)); };
};


//-----CEventUrl----------------------------------------------------------------
class CEventUrl : public CUserEvent
{
public:
   CEventUrl(const char *_szUrl, const char *_szUrlDescription, 
             unsigned short _nCommand, time_t _tTime, 
             unsigned long _nFlags);
   virtual ~CEventUrl(void);
   virtual CEventUrl *Copy(void)
      { return (new CEventUrl(m_szUrl, m_szUrlDescription, m_nCommand, m_tTime, 
                              m_nFlags)); }
   const char *Url(void)  { return m_szUrl; }
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
   virtual ~CEventChat(void);
   virtual CEventChat *Copy(void)
      { return (new CEventChat(m_szReason, m_nSequence, m_tTime, m_nFlags)); };
protected:
   char *m_szReason;
};


//-----CEventChatCancel---------------------------------------------------------
class CEventChatCancel : public CUserEvent
{
public:
   CEventChatCancel(unsigned long _nSequence, time_t _tTime, 
                    unsigned long _nFlags);
   virtual ~CEventChatCancel(void);
   virtual CEventChatCancel *Copy(void)
      { return (new CEventChatCancel(m_nSequence, m_tTime, m_nFlags)); };
};


//-----CEventAdded--------------------------------------------------------------
class CEventAdded : public CUserEvent
{
public:
   CEventAdded(unsigned long _nUin, const char *_szAlias, const char *_szFirstName,
               const char *_szLastName, const char *_szEmail, 
               unsigned short _nCommand, time_t _tTime, unsigned long _nFlags);
   virtual ~CEventAdded(void);
   virtual CEventAdded *Copy(void)
      { return (new CEventAdded(m_nUin, m_szAlias, m_szFirstName, m_szLastName, 
                                m_szEmail, m_nCommand, m_tTime, m_nFlags)); };

protected:
   unsigned long m_nUin;
   char *m_szAlias;
   char *m_szFirstName;
   char *m_szLastName;
   char *m_szEmail;
};



//-----CEventAuth---------------------------------------------------------------
class CEventAuth : public CUserEvent
{
public:
   CEventAuth(unsigned long _nUin, const char *_szAlias, const char *_szFirstName,
              const char *_szLastName, const char *_szEmail, const char *_szReason, 
              unsigned short _nCommand, time_t _tTime, unsigned long _nFlags);
   virtual ~CEventAuth(void);
   virtual CEventAuth *Copy(void)
      { return (new CEventAuth(m_nUin, m_szAlias, m_szFirstName, m_szLastName, 
                               m_szEmail, m_szReason, m_nCommand, m_tTime, 
                               m_nFlags)); };
   unsigned long Uin(void)  { return m_nUin; };
protected:
   unsigned long m_nUin;
   char *m_szAlias;
   char *m_szFirstName;
   char *m_szLastName;
   char *m_szEmail;
   char *m_szReason;
};


//-----CEventWebPanel----------------------------------------------------------
class CEventWebPanel : public CUserEvent
{
public:
   CEventWebPanel(const char *_szName, char *_szEmail, const char *_szMessage, 
                   unsigned short _nCommand, time_t _tTime, unsigned long _nFlags);
   virtual ~CEventWebPanel(void);
   virtual CEventWebPanel *Copy(void)
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
   virtual ~CEventEmailPager(void);
   virtual CEventEmailPager *Copy(void)
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
  CEventContactList(vector <char *> &_vszFields, unsigned short _nCommand,
                    time_t _tTime, unsigned long _nFlags);
  virtual ~CEventContactList(void);
  virtual CEventContactList *Copy(void)
    { return (new CEventContactList(m_vszFields, m_nCommand, m_tTime, m_nFlags)); }
protected:
  vector <char *> m_vszFields;
};

//-----CEventSaved--------------------------------------------------------------
class CEventSaved : public CUserEvent
{
public:
   CEventSaved(unsigned short _nNumEvents);
   virtual CEventSaved *Copy(void)
      { return (new CEventSaved(m_nNumEvents)); };

protected:
   unsigned short m_nNumEvents;
};


class CEventUnknown : public CUserEvent
{
public:
   CEventUnknown(CBuffer *_xBuf, unsigned short _nSubCommand, 
                 unsigned short _nCommand, unsigned long _nSequence, 
                 time_t _tTime, unsigned long _nFlags);
   virtual CEventUnknown *Copy(void)
      { return (new CEventUnknown(&m_xBuf, m_nSubCommand, m_nCommand, 
                                  m_nSequence, m_tTime, m_nFlags)); };
protected:
   CBuffer m_xBuf;
};

#endif
