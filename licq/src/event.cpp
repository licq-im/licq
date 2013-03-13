/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1998-2013 Licq developers <licq-dev@googlegroups.com>
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

#include <licq/event.h>

#include "config.h"

#include <assert.h>

#include <licq/packet.h>
#include <licq/protocolmanager.h>
#include <licq/protocolsignal.h>
#include <licq/userevents.h>
#include <licq/logging/log.h>

using Licq::Event;
using Licq::ProtocolSignal;
using Licq::UserId;
using Licq::UserEvent;

Event::Event(const ProtocolSignal* ps, ResultType result, UserEvent* ue)
  : m_eResult(result),
    myUserId(ps->userId()),
    thread_plugin(ps->callerThread()),
    m_pUserEvent(ue),
    m_nEventId(ps->eventId())
{
  m_bCancelled = false;
  m_Deleted = false;
  m_NoAck = false;
  m_pPacket = NULL;
  m_nSNAC = 0;
  m_nSequence = 0;
  m_nSubSequence = 0;
  m_nSubType = 0;
  m_nExtraInfo = 0;
  myCommand = CommandOther;
  myFlags = 0;
  m_eConnect = ConnectServer;
  m_nSocketDesc = 0;
  m_pExtendedAck = NULL;
  m_pSearchAck = NULL;
  mySubResult = SubResultAccept;
  thread_running = false;
}

Event::Event(pthread_t caller, unsigned long id, int _nSocketDesc, Licq::Packet* p,
    ConnectType _eConnect, const UserId& userId, Licq::UserEvent* e)
//   : m_xBuffer(p.getBuffer())
{
  // set up internal variables
  m_bCancelled = false;
  m_Deleted = false;
  m_NoAck = false;
  if (p)
  {
    m_pPacket = p;
    m_nSNAC = p->SNAC();
    m_nSequence = p->Sequence();
    m_nSubSequence = p->SubSequence();
    m_nSubType = (p->SNAC() & 0xFFFF);
    m_nExtraInfo = p->ExtraInfo();
  } else
  {
    m_pPacket = NULL;
    m_nSNAC = 0;
    m_nSequence = 0;
    m_nSubSequence = 0;
    m_nSubType = 0;
    m_nExtraInfo = 0;
  }
  myCommand = CommandOther;
  myFlags = 0;
  myUserId = userId;
  m_eConnect = _eConnect;
  m_pUserEvent = e;
  m_nSocketDesc = _nSocketDesc;
  m_pExtendedAck = NULL;
  m_pSearchAck = NULL;
  mySubResult = SubResultAccept;
  thread_plugin = caller;
  thread_running = false;

  m_nEventId = id;
}

Event::Event(int _nSocketDesc, Licq::Packet* p, ConnectType _eConnect,
    const UserId& userId, Licq::UserEvent* e)
{
  // set up internal variables
  m_bCancelled = false;
  m_Deleted = false;
  m_NoAck = false;
  if (p)
  {
    m_pPacket = p;
    m_nSNAC = p->SNAC();
    m_nSequence = p->Sequence();
    m_nSubSequence = p->SubSequence();
    m_nSubType = (p->SNAC() & 0xFFFF);
    m_nExtraInfo = p->ExtraInfo();
  } else
  {
    m_pPacket = NULL;
    m_nSNAC = 0;
    m_nSequence = 0;
    m_nSubSequence = 0;
    m_nSubType = 0;
    m_nExtraInfo = 0;
  }
  myCommand = CommandOther;
  myFlags = 0;
  myUserId = userId;
  m_eConnect = _eConnect;
  m_pUserEvent = e;
  m_nSocketDesc = _nSocketDesc;
  m_pExtendedAck = NULL;
  m_pSearchAck = NULL;
  mySubResult = SubResultAccept;
  thread_plugin = pthread_self();
  thread_running = false;

  m_nEventId = gProtocolManager.getNextEventId();
}

//-----ICQEvent::constructor----------------------------------------------------
Event::Event(const Event* e)
//   : m_xBuffer(e->m_xBuffer)
{
  m_nEventId = e->m_nEventId;

  // set up internal variables
  m_pPacket = NULL;
  m_Deleted = false;
  m_NoAck = false;
  m_bCancelled = e->m_bCancelled;
  m_nSNAC = e->m_nSNAC;
  m_nSequence = e->m_nSequence;
  m_nSubSequence = e->m_nSubSequence;
  m_nSubType = e->m_nSubType;
  m_nExtraInfo = e->m_nExtraInfo;
  myCommand = e->myCommand;
  myFlags = e->myFlags;
  myUserId = e->myUserId;
  m_eConnect = e->m_eConnect;
  m_eResult = e->m_eResult;
  mySubResult = e->mySubResult;
  if (e->m_pUserEvent != NULL)
    m_pUserEvent = e->m_pUserEvent->Copy();
  else
    m_pUserEvent = NULL;
  m_nSocketDesc = e->m_nSocketDesc;
  m_pExtendedAck = NULL;
  m_pSearchAck = NULL;

  thread_plugin = e->thread_plugin;
  thread_send = e->thread_send;
  thread_running = e->thread_running;
}



//-----ICQEvent::destructor-----------------------------------------------------
Event::~Event()
{
  assert(!m_Deleted);
  m_Deleted = true;

  delete m_pPacket;
  delete m_pUserEvent;
  delete m_pExtendedAck;
  delete m_pSearchAck;
}


//-----ICQEvent::AttachPacket---------------------------------------------------
void Event::AttachPacket(Licq::Packet* p)
{
  m_pPacket = p;
  m_nSNAC = p->SNAC();
  m_nSequence = p->Sequence();
  m_nSubSequence = p->SubSequence();
  m_nSubType = (p->SNAC() & 0xFFFF);
  m_nExtraInfo = p->ExtraInfo();
}
  
//-----ICQEvent::CompareEvent---------------------------------------------------
bool Event::CompareEvent(int sockfd, unsigned short _nSequence) const
{
   return(m_nSocketDesc == sockfd && m_nSequence == _nSequence);
}

bool Event::CompareEvent(unsigned short nEventId) const
{
  return (m_nEventId == nEventId);
}

bool Event::CompareSubSequence(unsigned long _nSubSequence) const
{
  return (m_nSubSequence == _nSubSequence);
}

bool Event::Equals(unsigned long nEventId) const
{
  return (this == NULL && nEventId == 0) || CompareEvent(nEventId);
}

unsigned long Event::EventId() const
{
  return this == NULL ? 0 : m_nEventId;
}

// Returns the event and transfers ownership to the calling function
Licq::UserEvent* Event::GrabUserEvent()
{
  Licq::UserEvent* e = m_pUserEvent;
  m_pUserEvent = NULL;
  return e;
}

Licq::SearchData* Event::GrabSearchAck()
{
  Licq::SearchData* a = m_pSearchAck;
  m_pSearchAck = NULL;
  return a;
}
