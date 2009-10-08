// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998 - 2009 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>

#include "licq_events.h"
#include "licq_packets.h"
#include "licq_log.h"
#include "licq_user.h"

//-----CSearchAck------------------------------------------------------------
CSearchAck::CSearchAck(const UserId& userId)
  : myUserId(userId)
{
  m_szAlias = m_szFirstName = m_szLastName = m_szEmail = NULL;
}

CSearchAck::~CSearchAck()
{
  free(m_szAlias);
  free(m_szFirstName);
  free(m_szLastName);
  free(m_szEmail);
}


//-----CExtendedAck----------------------------------------------------------
CExtendedAck::CExtendedAck(bool bAccepted, unsigned short nPort, const char* szResponse)
{
  m_bAccepted = bAccepted;
  m_nPort = nPort;
  m_szResponse = strdup(szResponse);
}

CExtendedAck::~CExtendedAck()
{
  free(m_szResponse);
}


unsigned long ICQEvent::s_nNextEventId = 1;

//-----ICQEvent::constructor----------------------------------------------------
LicqEvent::LicqEvent(CICQDaemon *_pDaemon, int _nSocketDesc, CPacket *p,
    ConnectType _eConnect, const UserId& userId, CUserEvent *e)
//   : m_xBuffer(p.getBuffer())
{
  // set up internal variables
  m_bCancelled = false;
  m_Deleted = false;
  m_NoAck = false;
  if (p)
  {
    m_pPacket = p;
    m_nChannel = p->Channel();
    m_nCommand = p->Command();
    m_nSNAC = p->SNAC();
    m_nSubCommand = p->SubCommand();
    m_nSequence = p->Sequence();
    m_nSubSequence = p->SubSequence();
    m_nSubType = (p->SNAC() & 0xFFFF);
    m_nExtraInfo = p->ExtraInfo();
  } else
  {
    m_pPacket = NULL;
    m_nChannel = 0;
    m_nCommand = 0;
    m_nSNAC = 0;
    m_nSubCommand = 0;
    m_nSequence = 0;
    m_nSubSequence = 0;
    m_nSubType = 0;
    m_nExtraInfo = 0;
  }
  myUserId = userId;
  m_eConnect = _eConnect;
  m_pUserEvent = e;
  m_nSocketDesc = _nSocketDesc;
  m_pExtendedAck = NULL;
  m_pSearchAck = NULL;
  m_pUnknownUser = NULL;
  m_nSubResult = ICQ_TCPxACK_ACCEPT;
  m_pDaemon = _pDaemon;
  thread_plugin = pthread_self();
  thread_running = false;

  // pthread_mutex_lock
  m_nEventId = s_nNextEventId++;
  // pthread_mutex_unlock
}

//-----ICQEvent::constructor----------------------------------------------------
LicqEvent::LicqEvent(const LicqEvent* e)
//   : m_xBuffer(e->m_xBuffer)
{
  m_nEventId = e->m_nEventId;

  // set up internal variables
  m_pPacket = NULL;
  m_Deleted = false;
  m_NoAck = false;
  m_bCancelled = e->m_bCancelled;
  m_nChannel = e->m_nChannel;
  m_nSNAC = e->m_nSNAC;
  m_nCommand = e->m_nCommand;
  m_nSubCommand = e->m_nSubCommand;
  m_nSequence = e->m_nSequence;
  m_nSubSequence = e->m_nSubSequence;
  m_nSubType = e->m_nSubType;
  m_nExtraInfo = e->m_nExtraInfo;
  myUserId = e->myUserId;
  m_eConnect = e->m_eConnect;
  m_eResult = e->m_eResult;
  m_nSubResult = e->m_nSubResult;
  if (e->m_pUserEvent != NULL)
    m_pUserEvent = e->m_pUserEvent->Copy();
  else
    m_pUserEvent = NULL;
  m_nSocketDesc = e->m_nSocketDesc;
  m_pExtendedAck = NULL;
  m_pSearchAck = NULL;
  m_pDaemon = e->m_pDaemon;
  m_pUnknownUser = NULL;

  thread_plugin = e->thread_plugin;
  thread_send = e->thread_send;
  thread_running = e->thread_running;
}



//-----ICQEvent::destructor-----------------------------------------------------
LicqEvent::~LicqEvent()
{
  assert(!m_Deleted);
  m_Deleted = true;

  delete m_pPacket;
  delete m_pUserEvent;
  delete m_pExtendedAck;
  delete m_pSearchAck;
  delete m_pUnknownUser;
}


//-----ICQEvent::AttachPacket---------------------------------------------------
void ICQEvent::AttachPacket(CPacket *p)
{
  m_pPacket = p;
  m_nChannel = p->Channel();
  m_nCommand = p->Command();
  m_nSNAC = p->SNAC();
  m_nSubCommand = p->SubCommand();
  m_nSequence = p->Sequence();
  m_nSubSequence = p->SubSequence();
  m_nSubType = (p->SNAC() & 0xFFFF);
  m_nExtraInfo = p->ExtraInfo();
}
  
//-----ICQEvent::CompareEvent---------------------------------------------------
bool ICQEvent::CompareEvent(int sockfd, unsigned short _nSequence) const
{
   return(m_nSocketDesc == sockfd && m_nSequence == _nSequence);
}

bool ICQEvent::CompareEvent(unsigned short nEventId) const
{
  return (m_nEventId == nEventId);
}

bool ICQEvent::CompareSubSequence(unsigned long _nSubSequence) const
{
  return (m_nSubSequence == _nSubSequence);
}

bool ICQEvent::Equals(unsigned long nEventId) const
{
  return (this == NULL && nEventId == 0) || CompareEvent(nEventId);
}


unsigned long ICQEvent::EventId() const
{
  return this == NULL ? 0 : m_nEventId;
}


// Returns the event and transfers ownership to the calling function
CUserEvent *ICQEvent::GrabUserEvent()
{
  CUserEvent *e = m_pUserEvent; m_pUserEvent = NULL; return e;
}

CSearchAck *ICQEvent::GrabSearchAck()
{
  CSearchAck *a = m_pSearchAck; m_pSearchAck = NULL; return a;
}

ICQUser *ICQEvent::GrabUnknownUser()
{
  ICQUser *u = m_pUnknownUser; m_pUnknownUser = NULL; return u;
}

LicqSignal::LicqSignal(unsigned long signal, unsigned long subSignal,
    const UserId& userId, int argument, unsigned long cid)
  : mySignal(signal),
    mySubSignal(subSignal),
    myUserId(userId),
    myArgument(argument),
    myCid(cid)
{
}

LicqSignal::LicqSignal(const LicqSignal* s)
  : mySignal(s->mySignal),
    mySubSignal(s->mySubSignal),
    myUserId(s->myUserId),
    myArgument(s->myArgument),
    myCid(s->myCid)
{
}


CSignal::CSignal(SIGNAL_TYPE e, const char *szId, unsigned long nCID)
{
  m_eType = e;
  m_nCID = nCID;
  if (szId)
    m_szId = strdup(szId);
  else
    m_szId = 0;
  thread_plugin = pthread_self();
}

CSignal::CSignal(CSignal *s)
{
  m_eType = s->m_eType;
  m_nCID = s->m_nCID;
  m_szId = s->m_szId ? strdup(s->m_szId) : 0;
  thread_plugin = s->thread_plugin;
}

CSignal::~CSignal()
{
  free(m_szId);
}

CLogonSignal::CLogonSignal(unsigned long nLogonStatus)
  : CSignal(PROTOxLOGON, 0)
{
  m_nLogonStatus = nLogonStatus;
}

CLogonSignal::~CLogonSignal()
{
  // Empty
}

CLogoffSignal::CLogoffSignal()
  : CSignal(PROTOxLOGOFF, 0)
{
}

CLogoffSignal::~CLogoffSignal()
{
  // Empty
}

CChangeStatusSignal::CChangeStatusSignal(unsigned long nStatus)
  : CSignal(PROTOxCHANGE_STATUS, 0)
{
  m_nStatus = nStatus;
}

CChangeStatusSignal::~CChangeStatusSignal()
{
  // Empty
}

CAddUserSignal::CAddUserSignal(const char *szId, bool bAuthRequired)
  : CSignal(PROTOxADD_USER, szId)
{
  m_bAuthRequired = bAuthRequired;
}

CAddUserSignal::~CAddUserSignal()
{
  // Empty
}

CRemoveUserSignal::CRemoveUserSignal(const char *szId)
  : CSignal(PROTOxREM_USER, szId)
{
}

CRemoveUserSignal::~CRemoveUserSignal()
{
  // Empty
}

CRenameUserSignal::CRenameUserSignal(const char *szId)
  : CSignal(PROTOxRENAME_USER, szId)
{
}

CRenameUserSignal::~CRenameUserSignal()
{
  // Empty
}

CSendMessageSignal::CSendMessageSignal(const char *szId, const char *szMsg, unsigned long nCID)
  : CSignal(PROTOxSENDxMSG, szId, nCID)
{
  m_szMsg = szMsg ? strdup(szMsg) : 0;
}

CSendMessageSignal::~CSendMessageSignal()
{
  free(m_szMsg);
}

CTypingNotificationSignal::CTypingNotificationSignal(const char *szId,
  bool bActive, unsigned long nCID)
    : CSignal(PROTOxSENDxTYPING_NOTIFICATION, szId, nCID), m_bActive(bActive)
{
}

CTypingNotificationSignal::~CTypingNotificationSignal()
{
  // Empty
}

CGrantAuthSignal::CGrantAuthSignal(const char *szId, const char *szMsg)
  : CSignal(PROTOxSENDxGRANTxAUTH, szId)
{
  m_szMsg = szMsg ? strdup(szMsg) : 0;
}

CGrantAuthSignal::~CGrantAuthSignal()
{
  free(m_szMsg);
}

CRefuseAuthSignal::CRefuseAuthSignal(const char *szId, const char *szMsg)
  : CSignal(PROTOxSENDxREFUSExAUTH, szId)
{
  m_szMsg = szMsg ? strdup(szMsg) : 0;
}

CRefuseAuthSignal::~CRefuseAuthSignal()
{
  free(m_szMsg);
}

CRequestInfo::CRequestInfo(const char *szId)
  : CSignal(PROTOxREQUESTxINFO, szId)
{
}

CRequestInfo::~CRequestInfo()
{
  // Empty
}

CUpdateInfoSignal::CUpdateInfoSignal(const char *szAlias, const char *szFirstName,
  const char *szLastName, const char *szEmail,
  const char *szCity,
  const char *szState, const char *szPhoneNumber,
  const char *szFaxNumber, const char *szAddress,
  const char *szCellNumber, const char *szZipCode)
  : CSignal(PROTOxUPDATExINFO, 0)
{
  m_szAlias = szAlias ? strdup(szAlias) : 0;
  m_szFirstName = szFirstName ? strdup(szFirstName) : 0;
  m_szLastName = szLastName ? strdup(szLastName) : 0;
  m_szEmail = szEmail ? strdup(szEmail) : 0;
  m_szCity = szCity ? strdup(szCity) : 0;
  m_szState = szState ? strdup(szState) : 0;
  m_szPhoneNumber = szPhoneNumber ? strdup(szPhoneNumber) : 0;
  m_szFaxNumber = szFaxNumber ? strdup(szFaxNumber) : 0;
  m_szAddress = szAddress ? strdup(szAddress) : 0;
  m_szCellNumber = szCellNumber ? strdup(szCellNumber) : 0;
  m_szZipCode = szZipCode ? strdup(szZipCode) : 0;
}

CUpdateInfoSignal::~CUpdateInfoSignal()
{
  free(m_szAlias);
  free(m_szFirstName);
  free(m_szLastName);
  free(m_szEmail);
  free(m_szCity);
  free(m_szState);
  free(m_szPhoneNumber);
  free(m_szFaxNumber);
  free(m_szAddress);
  free(m_szCellNumber);
  free(m_szZipCode);
}

CRequestPicture::CRequestPicture(const char *szId)
  : CSignal(PROTOxREQUESTxPICTURE, szId)
{
}

CRequestPicture::~CRequestPicture()
{
}

CBlockUserSignal::CBlockUserSignal(const char *szId)
  : CSignal(PROTOxBLOCKxUSER, szId)
{
}

CBlockUserSignal::~CBlockUserSignal()
{
  // Empty
}

CUnblockUserSignal::CUnblockUserSignal(const char *szId)
  : CSignal(PROTOxUNBLOCKxUSER, szId)
{
}

CUnblockUserSignal::~CUnblockUserSignal()
{
  // Empty
}

CAcceptUserSignal::CAcceptUserSignal(const char *szId)
  : CSignal(PROTOxACCEPTxUSER, szId)
{
}

CAcceptUserSignal::~CAcceptUserSignal()
{
  // Empty
}

CUnacceptUserSignal::CUnacceptUserSignal(const char *szId)
  : CSignal(PROTOxUNACCEPTxUSER, szId)
{
}

CUnacceptUserSignal::~CUnacceptUserSignal()
{
  // Empty
}

CIgnoreUserSignal::CIgnoreUserSignal(const char *szId)
  : CSignal(PROTOxIGNORExUSER, szId)
{
}

CIgnoreUserSignal::~CIgnoreUserSignal()
{
  // Empty
}

CUnignoreUserSignal::CUnignoreUserSignal(const char *szId)
  : CSignal(PROTOxUNIGNORExUSER, szId)
{
}

CUnignoreUserSignal::~CUnignoreUserSignal()
{
  // Empty
}

CSendFileSignal::CSendFileSignal(const char *szId, const char *szFile,
                                 const char *szMessage, ConstFileList &_lFileList)
  : CSignal(PROTOxSENDxFILE, szId)
{
  m_szFile = (szFile ? strdup(szFile) : NULL);

  m_szMessage = (szMessage ? strdup(szMessage) : NULL);

  m_lFileList = _lFileList;
}

CSendFileSignal::~CSendFileSignal()
{
  free(m_szFile);
  free(m_szMessage);
}

CSendChatSignal::CSendChatSignal(const char *szId, const char *szMessage)
  : CSignal(PROTOxSENDxCHAT, szId)
{
  m_szMessage = (szMessage ? strdup(szMessage) : 0);
}

CSendChatSignal::~CSendChatSignal()
{
  free(m_szMessage);
}

CCancelEventSignal::CCancelEventSignal(const char *szId, unsigned long nFlag)
  : CSignal(PROTOxCANCELxEVENT, szId)
{
  m_nFlag = nFlag;
}

CCancelEventSignal::~CCancelEventSignal()
{
  // Empty
}

CSendEventReplySignal::CSendEventReplySignal(const char *szId,
  const char *szMessage, bool bAccept, unsigned short nPort,
  unsigned long nSequence, unsigned long nFlag1, unsigned long nFlag2,
  bool bDirect)
  : CSignal(PROTOxSENDxEVENTxREPLY, szId)
{
  m_szMessage = (szMessage ? strdup(szMessage) : 0);

  m_bAccept = bAccept;
  m_bDirect = bDirect;
  m_nPort = nPort;
  m_nSequence = nSequence;
  m_nFlag = nFlag1;
  m_nFlag2 = nFlag2;
}

CSendEventReplySignal::~CSendEventReplySignal()
{
  free(m_szMessage);
}

COpenedWindowSignal::COpenedWindowSignal(const char *szId)
  : CSignal(PROTOxOPENEDxWINDOW, szId)
{
}

COpenedWindowSignal::~COpenedWindowSignal()
{
  // Empty
}

CClosedWindowSignal::CClosedWindowSignal(const char *szId)
  : CSignal(PROTOxCLOSEDxWINDOW, szId)
{
}

CClosedWindowSignal::~CClosedWindowSignal()
{
  // Empty
}

COpenSecureSignal::COpenSecureSignal(const char *szId)
  : CSignal(PROTOxOPENxSECURE, szId)
{
}

COpenSecureSignal::~COpenSecureSignal()
{
  // Empty
}

CCloseSecureSignal::CCloseSecureSignal(const char *szId)
  : CSignal(PROTOxCLOSExSECURE, szId)
{
}

CCloseSecureSignal::~CCloseSecureSignal()
{
  // Empty
}
