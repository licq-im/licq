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

//=====COnEventManager==========================================================

COnEventManager::COnEventManager()
{
  m_szCommand = NULL;
  for (unsigned short i = 0; i < MAX_ON_EVENT; i++)
    m_aszParameters[i] = NULL;
  pthread_mutex_init(&mutex, NULL);
}

COnEventManager::~COnEventManager()
{
  if (m_szCommand)
    free(m_szCommand);

  for (unsigned short i = 0; i < MAX_ON_EVENT; i++)
    if (m_aszParameters[i])
      free(m_aszParameters[i]);
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

//-----COnEventManager::SetParameters-------------------------------------------
void COnEventManager::SetParameters(const char *_szCommand, const char **_aszParams)
{
  pthread_mutex_lock(&mutex);
  SetString(&m_szCommand, _szCommand);
  for (unsigned short i = 0; i < MAX_ON_EVENT; i++)
    SetString(&m_aszParameters[i], _aszParams[i]);
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
    char *szParam = m_aszParameters[_nEvent];
    char *szFullParam;
    if (u != NULL)
      szFullParam = u->usprintf(szParam, USPRINTF_LINEISCMD);
    else
      szFullParam = strdup(szParam);

    if (strlen(szFullParam))
    {
      char szCmd[strlen(m_szCommand) + strlen(szFullParam) + 8];
      sprintf(szCmd, "%s %s &", m_szCommand, szFullParam);
      system(szCmd);
    }

    if (szFullParam)
      free(szFullParam);
  }
  pthread_mutex_unlock(&mutex);
}

