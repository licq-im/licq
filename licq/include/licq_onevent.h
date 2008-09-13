#ifndef ONEVENT_H
#define ONEVENT_H

#include <pthread.h>
#include <string>

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

class COnEventManager
{
public:
  COnEventManager();
  ~COnEventManager();

  const std::string command() const { return myCommand; }
  const std::string parameter(unsigned short event) const { return myParameters[event]; }
  void Do(unsigned short m_nEvent, ICQUser *u);
  void SetCommandType(unsigned short _nCommandType);
  void setCommand(const std::string& command);
  void setParameter(unsigned short event, const std::string& parameter);
  void setParameters(const std::string& command, const std::string parameters[]);
  unsigned short CommandType();
  void Lock()     { pthread_mutex_lock(&mutex); }
  void Unlock()   { pthread_mutex_unlock(&mutex); }
protected:
  unsigned short m_nCommandType;
  std::string myCommand;
  std::string myParameters[MAX_ON_EVENT];
  mutable pthread_mutex_t mutex;
};


#endif
