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

#include <assert.h>

#include "licq_events.h"
#include "licq_packets.h"
#include "licq_log.h"
#include "licq_user.h"

//-----CSearchAck------------------------------------------------------------
CSearchAck::CSearchAck(unsigned long _nUin)
{
  m_nUin = _nUin;
  m_szAlias = m_szFirstName = m_szLastName = m_szEmail = NULL;
  
  char szUin[24];
  sprintf(szUin, "%lu", _nUin);
  m_szId = strdup(szUin);
  m_nPPID = LICQ_PPID;
}

CSearchAck::CSearchAck(const char *_szId, unsigned long _nPPID)
{
  m_nUin = 0;
  m_szAlias = m_szFirstName = m_szLastName = m_szEmail = NULL;
  m_szId = _szId ? strdup(_szId) : NULL;
  m_nPPID = _nPPID;
}

CSearchAck::~CSearchAck()
{
  if (m_szAlias != NULL) free(m_szAlias);
  if (m_szFirstName != NULL) free(m_szFirstName);
  if (m_szLastName != NULL) free(m_szLastName);
  if (m_szEmail != NULL) free(m_szEmail);
  if (m_szId != NULL) free(m_szId);
}


//-----CExtendedAck----------------------------------------------------------
CExtendedAck::CExtendedAck(bool bAccepted, unsigned short nPort, char *szResponse)
{
  m_bAccepted = bAccepted;
  m_nPort = nPort;
  m_szResponse = strdup(szResponse);
}

CExtendedAck::~CExtendedAck()
{
  if (m_szResponse != NULL)
    free(m_szResponse);
}


unsigned long ICQEvent::s_nNextEventId = 1;

//-----ICQEvent::constructor----------------------------------------------------
ICQEvent::ICQEvent(CICQDaemon *_pDaemon, int _nSocketDesc, CPacket *p,
                   ConnectType _eConnect, unsigned long _nUin, CUserEvent *e)
//   : m_xBuffer(p.getBuffer())
{
  // set up internal variables
  m_pPacket = p;
  m_bCancelled = false;
  m_Deleted = false;
  m_NoAck = false;
  m_nChannel = p->Channel();
  m_nCommand = p->Command();
  m_nSNAC = p->SNAC();
  m_nSubCommand = p->SubCommand();
  m_nSequence = p->Sequence();
  m_nSubSequence = p->SubSequence();
  m_nSubType = (p->SNAC() & 0xFFFF);
  m_nExtraInfo = p->ExtraInfo();
  m_nDestinationUin = _nUin;
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

  char szUin[24];
  sprintf(szUin, "%lu", _nUin);
  m_szId = strdup(szUin);
  m_nPPID = LICQ_PPID;
  
  // pthread_mutex_lock
  m_nEventId = s_nNextEventId++;
  // pthread_mutex_unlock
}

ICQEvent::ICQEvent(CICQDaemon *_pDaemon, int _nSocketDesc, CPacket *p,
                   ConnectType _eConnect, const char *_szId, unsigned long _nPPID,
                   CUserEvent *e)
//   : m_xBuffer(p.getBuffer())
{
  // set up internal variables
  m_pPacket = p;
  m_bCancelled = false;
  m_Deleted = false;
  m_NoAck = false;
  m_nChannel = p->Channel();
  m_nCommand = p->Command();
  m_nSNAC = p->SNAC();
  m_nSubCommand = p->SubCommand();
  m_nSequence = p->Sequence();
  m_nSubSequence = p->SubSequence();
  m_nSubType = (p->SNAC() & 0xFFFF);
  m_nExtraInfo = p->ExtraInfo();
  m_nDestinationUin = 0;
  m_szId = _szId ? strdup(_szId) : 0;
  m_nPPID = _nPPID;
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
ICQEvent::ICQEvent(ICQEvent *e)
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
  m_nDestinationUin = e->m_nDestinationUin;
  if (e->m_szId)
    m_szId = strdup(e->m_szId);
  else
    m_szId = 0;
  m_nPPID = e->m_nPPID;
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
ICQEvent::~ICQEvent()
{
  assert(!m_Deleted);
  m_Deleted = true;

  if (m_szId)
    free(m_szId);
  delete m_pPacket;
  delete m_pUserEvent;
  delete m_pExtendedAck;
  delete m_pSearchAck;
  delete m_pUnknownUser;
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



//=====CICQSignal===============================================================
CICQSignal::CICQSignal(unsigned long nSignal, unsigned long nSubSignal,
                       const char *szId, unsigned long nPPID,
                       int nArg, unsigned long nCID)
{
  m_nSignal = nSignal;
  m_nSubSignal = nSubSignal;
  m_nUin = 0;
  m_szId = szId ? strdup(szId) : 0;
  m_nPPID = nPPID;
  m_nArgument = nArg;
  m_nCID = nCID;
}

CICQSignal::CICQSignal(unsigned long nSignal, unsigned long nSubSignal,
                       unsigned long nUin, int nArg, unsigned long nCID)
{
  m_nSignal = nSignal;
  m_nSubSignal = nSubSignal;
  m_nUin = nUin;
  m_nArgument = nArg;
  m_nCID = nCID;

  char szUin[24];
  sprintf(szUin, "%lu", nUin);
  m_szId = strdup(szUin);
  m_nPPID = LICQ_PPID;
}

CICQSignal::CICQSignal(CICQSignal *s)
{
  m_nSignal = s->Signal();
  m_nSubSignal = s->SubSignal();
  m_nUin = s->Uin();
  m_szId = s->Id() ? strdup(s->Id()) : 0;
  m_nPPID = s->PPID();
  m_nArgument = s->Argument();
  m_nCID = s->CID();
}


CICQSignal::~CICQSignal()
{
  if (m_szId)
    free(m_szId);
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
  if (m_szId)
    free(m_szId);
}

CLogonSignal::CLogonSignal(unsigned long nLogonStatus)
  : CSignal(PROTOxLOGON, 0)
{
  m_nLogonStatus = nLogonStatus;
}

CLogoffSignal::CLogoffSignal()
  : CSignal(PROTOxLOGOFF, 0)
{
}

CChangeStatusSignal::CChangeStatusSignal(unsigned long nStatus)
  : CSignal(PROTOxCHANGE_STATUS, 0)
{
  m_nStatus = nStatus;
}

CAddUserSignal::CAddUserSignal(const char *szId, bool bAuthRequired)
  : CSignal(PROTOxADD_USER, szId)
{
  m_bAuthRequired = bAuthRequired;
}

CRemoveUserSignal::CRemoveUserSignal(const char *szId)
  : CSignal(PROTOxREM_USER, szId)
{
}

CRenameUserSignal::CRenameUserSignal(const char *szId)
  : CSignal(PROTOxRENAME_USER, szId)
{
}

CSendMessageSignal::CSendMessageSignal(const char *szId, const char *szMsg, unsigned long nCID)
  : CSignal(PROTOxSENDxMSG, szId, nCID)
{
  m_szMsg = szMsg ? strdup(szMsg) : 0;
}

CTypingNotificationSignal::CTypingNotificationSignal(const char *szId,
  bool bActive, unsigned long nCID)
    : CSignal(PROTOxSENDxTYPING_NOTIFICATION, szId, nCID), m_bActive(bActive)
{
}

CGrantAuthSignal::CGrantAuthSignal(const char *szId, const char *szMsg)
  : CSignal(PROTOxSENDxGRANTxAUTH, szId)
{
  m_szMsg = szMsg ? strdup(szMsg) : 0;
}

CRefuseAuthSignal::CRefuseAuthSignal(const char *szId, const char *szMsg)
  : CSignal(PROTOxSENDxREFUSExAUTH, szId)
{
  m_szMsg = szMsg ? strdup(szMsg) : 0;
}

CRequestInfo::CRequestInfo(const char *szId)
  : CSignal(PROTOxREQUESTxINFO, szId)
{
}

