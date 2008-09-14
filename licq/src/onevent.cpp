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

#include <stdio.h>

#include "licq_onevent.h"
#include "licq_user.h"
#include "licq_icqd.h"
#include "support.h"

using namespace std;

//=====COnEventManager==========================================================

COnEventManager::COnEventManager()
{
  pthread_mutex_init(&mutex, NULL);
}

COnEventManager::~COnEventManager()
{
}

void COnEventManager::SetCommandType(unsigned short _nCommandType)
{
  pthread_mutex_lock(&mutex);
  m_nCommandType = _nCommandType;
  pthread_mutex_unlock(&mutex);
}

unsigned short COnEventManager::CommandType()
{
  pthread_mutex_lock(&mutex);
  unsigned short n = m_nCommandType;
  pthread_mutex_unlock(&mutex);
  return n;
}

void COnEventManager::setCommand(const string& command)
{
  pthread_mutex_lock(&mutex);
  myCommand = command;
  pthread_mutex_unlock(&mutex);
}

void COnEventManager::setParameter(unsigned short event, const string& parameter)
{
  pthread_mutex_lock(&mutex);
  myParameters[event] = parameter;
  pthread_mutex_unlock(&mutex);
}

//-----COnEventManager::SetParameters-------------------------------------------
void COnEventManager::setParameters(const string& command, const string parameters[])
{
  pthread_mutex_lock(&mutex);
  myCommand = command;
  for (unsigned short i = 0; i < MAX_ON_EVENT; i++)
    myParameters[i] = parameters[i];
  pthread_mutex_unlock(&mutex);
}


//-----COnEventManager::Do------------------------------------------------------
void COnEventManager::Do(unsigned short _nEvent, ICQUser *u)
{
  unsigned long nPPID = LICQ_PPID;
  if (u)
    nPPID = u->PPID();

  // Check if globally command should be run
  const ICQOwner* o = gUserManager.FetchOwner(nPPID, LOCK_R);
  if (!o) return;
  unsigned long s = o->Status();

  // Messy Mode / Accept Stuff by Andypoo (andypoo@ihug.com.au)
  bool no = false;
  if (u == NULL)
  {
    no = ( (s == ICQ_STATUS_AWAY && !o->AcceptInAway()) ||
           (s == ICQ_STATUS_NA && !o->AcceptInNA() ) ||
           (s == ICQ_STATUS_OCCUPIED && !o->AcceptInOccupied()) ||
           (s == ICQ_STATUS_DND && !o->AcceptInDND() ));
  }
  else
  {
    no = ( (s == ICQ_STATUS_AWAY && !u->AcceptInAway() && !o->AcceptInAway()) ||
           (s == ICQ_STATUS_NA && !u->AcceptInNA() && !o->AcceptInNA() ) ||
           (s == ICQ_STATUS_OCCUPIED && !u->AcceptInOccupied() && !o->AcceptInOccupied()) ||
           (s == ICQ_STATUS_DND && !u->AcceptInDND() && !o->AcceptInDND() ));
  }
  gUserManager.DropOwner(o);
  if (no) return;

  pthread_mutex_lock(&mutex);
  if (m_nCommandType == ON_EVENT_RUN)
  {
    string param = myParameters[_nEvent];
    if (u != NULL)
    {
      char* szFullParam = u->usprintf(param.c_str(), USPRINTF_LINEISCMD);
      param = szFullParam;
      free(szFullParam);
    }
    if (!param.empty())
    {
      string fullCmd = myCommand + " " + param + " &";
      system(fullCmd.c_str());
    }
  }
  pthread_mutex_unlock(&mutex);
}

