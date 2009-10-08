// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998 - 2009 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

#include "licq_plugind.h"

#include <cstring>
#include <unistd.h>

#include "licq_events.h"
#include "licq_socket.h"


CPlugin::CPlugin(const char *szLibName)
{
  m_nSignalMask = 0;
  localargv = NULL;
  localargc = 0;

  pthread_mutex_init(&mutex_signals, NULL);
  pthread_mutex_init(&mutex_events, NULL);
  pipe(pipe_plugin);

  m_szLibName = strdup(szLibName);
}


void CPlugin::SetSignalMask(unsigned long nMask)
{
  m_nSignalMask = nMask;
}


CPlugin::~CPlugin()
{
  if (localargv != NULL)
    free(localargv);
  free(m_szLibName);
}


void CPlugin::pushSignal(LicqSignal* s)
{
  pthread_mutex_lock(&mutex_signals);
  list_signals.push_back(s);
  pthread_mutex_unlock(&mutex_signals);
  write(pipe_plugin[PIPE_WRITE], "S", 1);
}


void CPlugin::PushEvent(ICQEvent *e)
{
  pthread_mutex_lock(&mutex_events);
  list_events.push_back(e);
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

LicqSignal* CPlugin::popSignal()
{
  LicqSignal* s = NULL;
  pthread_mutex_lock(&mutex_signals);
  if (list_signals.size() != 0)
  {
    s = list_signals.front();
    list_signals.pop_front();
  }
  pthread_mutex_unlock(&mutex_signals);
  return s;
}



ICQEvent *CPlugin::PopEvent()
{
  ICQEvent *e = NULL;
  pthread_mutex_lock(&mutex_events);
  if (list_events.size() != 0)
  {
    e = list_events.front();
    list_events.pop_front();
  }
  pthread_mutex_unlock(&mutex_events);
  return e;
}


