#ifndef ONEVENT_H
#define ONEVENT_H

#include <pthread.h>
//#include "pthread_rdwr.h"

class ICQUser;

//=====COnEventManager==========================================================
const unsigned short ON_EVENT_MSG = 0;
const unsigned short ON_EVENT_URL = 1;
const unsigned short ON_EVENT_CHAT = 2;
const unsigned short ON_EVENT_FILE = 3;
const unsigned short ON_EVENT_NOTIFY = 4;
const unsigned short ON_EVENT_SYSMSG = 5;
const unsigned short ON_EVENT_MSGSENT = 6;
const unsigned short ON_EVENT_SMS = 7;
const unsigned short MAX_ON_EVENT = 8;

const unsigned short ON_EVENT_IGNORE =  0;
const unsigned short ON_EVENT_RUN    =  1;
const unsigned short ON_EVENT_BY_PLUGIN = 3;

class COnEventManager
{
public:
  COnEventManager();
  void Do(unsigned short m_nEvent, ICQUser *u);
  void SetParameters(const char *, const char **);
  void SetCommandType(unsigned short _nCommandType);
  unsigned short CommandType();
  const char *Parameter(unsigned short n)  { return m_aszParameters[n]; }
  const char *Command()  { return m_szCommand; }
  void Lock()     { pthread_mutex_lock(&mutex); }
  void Unlock()   { pthread_mutex_unlock(&mutex); }
protected:
  unsigned short m_nCommandType;
  char *m_szCommand;
  char *m_aszParameters[MAX_ON_EVENT];
  pthread_mutex_t mutex;
};


#endif
