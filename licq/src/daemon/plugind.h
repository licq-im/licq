#ifndef PLUGIND_H
#define PLUGIND_H

#include <pthread.h>
#include <list.h>
#include "icqevent.h"


//=====CPlugin==================================================================
class CPlugin
{
public:
  CPlugin(unsigned long);
  void PushSignal(CICQSignal *);
  void PushEvent(ICQEvent *);
  CICQSignal *PopSignal(void);
  ICQEvent *PopEvent(void);

  void Shutdown(void);
  
  bool CompareThread(pthread_t);
  bool CompareMask(unsigned long);
  int Pipe(void) { return pipe_plugin[PIPE_READ]; }

protected:
  list <ICQEvent *> m_lEvents;
  list <CICQSignal *> m_lSignals;
  pthread_mutex_t mutex_events;
  pthread_mutex_t mutex_signals;
  pthread_t thread_plugin;
  int pipe_plugin[2];
  unsigned long m_nSignalMask;
};

#endif
