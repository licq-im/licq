#include <stdio.h>

#include "onevent.h"
#include "support.h"
#include "user.h"

//=====COnEventManager==========================================================

COnEventManager::COnEventManager()
{
  m_szCommand = NULL;
  for (unsigned short i = 0; i < MAX_ON_EVENT; i++)
    m_aszParameters[i] = NULL;
  pthread_mutex_init(&mutex, NULL);
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
  // Check if globally command should be run
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  unsigned long s = o->Status();
  gUserManager.DropOwner();
  if (s == ICQ_STATUS_OCCUPIED || s == ICQ_STATUS_DND) return;

  pthread_mutex_lock(&mutex);

  switch (m_nCommandType)
  {
  case ON_EVENT_IGNORE:
    break;

  case ON_EVENT_RUN:
  {
    char *szParam = m_aszParameters[_nEvent];
    char szFullParam[MAX_CMD_LEN] = {'\0'};
    if (u != NULL)
      u->usprintf(szFullParam, szParam);
    else
      strcpy(szFullParam, szParam);
    char szCmd[strlen(m_szCommand) + strlen(szFullParam) + 8];
    sprintf(szCmd, "%s %s &", m_szCommand, szFullParam);
    system(szCmd);
    break;
  }
  }
  pthread_mutex_unlock(&mutex);
}

