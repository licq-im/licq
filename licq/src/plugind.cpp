#include "plugind.h"
#include "licq_socket.h"

CPlugin::CPlugin(unsigned long _nSignalMask)
{
  m_nSignalMask = _nSignalMask;
  thread_plugin = pthread_self();
  pthread_mutex_init(&mutex_signals, NULL);
  pthread_mutex_init(&mutex_events, NULL);
  pipe(pipe_plugin);
}


void CPlugin::PushSignal(CICQSignal *s)
{
  pthread_mutex_lock(&mutex_signals);
  m_lSignals.push_back(s);
  pthread_mutex_unlock(&mutex_signals);
  write(pipe_plugin[PIPE_WRITE], "S", 1);
}


void CPlugin::PushEvent(ICQEvent *e)
{
  pthread_mutex_lock(&mutex_events);
  m_lEvents.push_back(e);
  pthread_mutex_unlock(&mutex_events);
  write(pipe_plugin[PIPE_WRITE], "E", 1);
}

void CPlugin::Shutdown()
{
  write(pipe_plugin[PIPE_WRITE], "X", 1);
}

void CPlugin::Enable()
{
  write(pipe_plugin[PIPE_WRITE], "1", 1);
}

void CPlugin::Disable()
{
  write(pipe_plugin[PIPE_WRITE], "0", 1);
}


bool CPlugin::CompareThread(pthread_t t)
{
  return pthread_equal(t, thread_plugin);
}

bool CPlugin::CompareMask(unsigned long n)
{
  return (n & m_nSignalMask);
}


CICQSignal *CPlugin::PopSignal()
{
  CICQSignal *s = NULL;
  pthread_mutex_lock(&mutex_signals);
  if (m_lSignals.size() != 0)
  {
    s = m_lSignals.front();
    m_lSignals.pop_front();
  }
  pthread_mutex_unlock(&mutex_signals);
  return s;
}



ICQEvent *CPlugin::PopEvent()
{
  ICQEvent *e = NULL;
  pthread_mutex_lock(&mutex_events);
  if (m_lEvents.size() != 0)
  {
    e = m_lEvents.front();
    m_lEvents.pop_front();
  }
  pthread_mutex_unlock(&mutex_events);
  return e;
}


