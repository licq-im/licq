#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "licq_events.h"
#include "licq_packets.h"
#include "licq_log.h"
#include "licq_user.h"

//-----CSearchAck------------------------------------------------------------
CSearchAck::CSearchAck(unsigned long _nUin)
{
  m_nUin = _nUin;
  m_szAlias = m_szFirstName = m_szLastName = m_szEmail = NULL;
}


CSearchAck::~CSearchAck()
{
  if (m_szAlias != NULL) free(m_szAlias);
  if (m_szFirstName != NULL) free(m_szFirstName);
  if (m_szLastName != NULL) free(m_szLastName);
  if (m_szEmail != NULL) free(m_szEmail);
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


//-----ICQEvent::constructor----------------------------------------------------
ICQEvent::ICQEvent(CICQDaemon *_pDaemon, int _nSocketDesc, CPacket *p,
                   ConnectType _eConnect, unsigned long _nUin, CUserEvent *e)
//   : m_xBuffer(p.getBuffer())
{
  // set up internal variables
  m_pPacket = p;
  m_bCancelled = false;
  m_nCommand = p->getCommand();
  m_nSubCommand = p->getSubCommand();
  m_nSequence = p->getSequence();
  m_nSubSequence = p->SubSequence();
  m_nDestinationUin = _nUin;
  m_eConnect = _eConnect;
  m_pUserEvent = e;
  m_nSocketDesc = _nSocketDesc;
  m_pExtendedAck = NULL;
  m_pSearchAck = NULL;
  m_pUnknownUser = NULL;
  m_nSubResult = 0;
  m_pDaemon = _pDaemon;
  thread_plugin = pthread_self();
}


//-----ICQEvent::constructor----------------------------------------------------
ICQEvent::ICQEvent(ICQEvent *e)
//   : m_xBuffer(e->m_xBuffer)
{
  // set up internal variables
  m_pPacket = NULL; //e->m_xPacket;
  m_bCancelled = e->m_bCancelled;
  m_nCommand = e->m_nCommand;
  m_nSubCommand = e->m_nSubCommand;
  m_nSequence = e->m_nSequence;
  m_nSubSequence = e->m_nSubSequence;
  m_nDestinationUin = e->m_nDestinationUin;
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
}



//-----ICQEvent::destructor-----------------------------------------------------
ICQEvent::~ICQEvent()
{
  delete m_pPacket;
  delete m_pUserEvent;
  delete m_pExtendedAck;
  delete m_pSearchAck;
  delete m_pUnknownUser;
}


//-----ICQEvent::CompareEvent---------------------------------------------------
bool ICQEvent::CompareEvent(int sockfd, unsigned long _nSequence) const
{
   return(m_nSocketDesc == sockfd && m_nSequence == _nSequence);
}


//=====CICQEventTag==========================================================
CICQEventTag::CICQEventTag(const ICQEvent *e)
    : m_nSocketDesc(e->m_nSocketDesc),
      m_nSequence(e->m_nSequence),
      m_nUin(e->m_nDestinationUin)
{
}

bool CICQEventTag::Equals(const ICQEvent *e)
{
  if (e == NULL) return false;
  if (m_nSocketDesc == -1)
  {
    if (m_nUin == 0) return (e->m_nSequence == m_nSequence);
    ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
    m_nSocketDesc = u->SocketDesc();
    gUserManager.DropUser(u);
    if (m_nSocketDesc == -1)
      return (m_nUin == e->m_nDestinationUin);
  }
  return (e->CompareEvent(m_nSocketDesc, m_nSequence));
}


//=====CICQSignal===============================================================
CICQSignal::CICQSignal(unsigned long nSignal, unsigned long nSubSignal,
                       unsigned long nUin, int nArgument)
{
  m_nSignal = nSignal;
  m_nSubSignal = nSubSignal;
  m_nUin = nUin;
  m_nArgument = nArgument;
}


CICQSignal::CICQSignal(CICQSignal *s)
{
  m_nSignal = s->Signal();
  m_nSubSignal = s->SubSignal();
  m_nUin = s->Uin();
  m_nArgument = s->Argument();
}


CICQSignal::~CICQSignal()
{
}

