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


unsigned long ICQEvent::s_nNextEventId = 1;

//-----ICQEvent::constructor----------------------------------------------------
ICQEvent::ICQEvent(CICQDaemon *_pDaemon, int _nSocketDesc, CPacket *p,
                   ConnectType _eConnect, unsigned long _nUin, CUserEvent *e)
//   : m_xBuffer(p.getBuffer())
{
  // set up internal variables
  m_pPacket = p;
  m_bCancelled = false;
  m_nCommand = p->Command();
  m_nSubCommand = p->SubCommand();
  m_nSequence = p->Sequence();
  m_nSubSequence = p->SubSequence();
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

bool ICQEvent::CompareEvent(unsigned long nEventId) const
{
  return (m_nEventId == nEventId);
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
                       unsigned long nUin, int nArgument,char *nParameters)
{
  m_nSignal = nSignal;
  m_nSubSignal = nSubSignal;
  m_nUin = nUin;
  m_nArgument = nArgument;
  m_szParameters = (nParameters!=NULL)?strdup(nParameters):NULL;
}


CICQSignal::CICQSignal(CICQSignal *s)
{
  m_nSignal = s->Signal();
  m_nSubSignal = s->SubSignal();
  m_nUin = s->Uin();
  m_nArgument = s->Argument();
  m_szParameters = (s->Parameters()!= NULL)?strdup(s->Parameters()):NULL;
}


CICQSignal::~CICQSignal()
{
    if (m_szParameters != NULL) {
        free(m_szParameters);
    }
}

