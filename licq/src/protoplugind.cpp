// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 2003 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

#include "licq_protoplugind.h"
#include "licq_constants.h"

#include <unistd.h>

class CSignal;

CProtoPlugin::CProtoPlugin(const char *_szLibName)
{
  m_szLibName = _szLibName ? strdup(_szLibName) : 0;
  pthread_mutex_init(&mutex_signals, NULL);
  pipe(pipe_plugin);
}

CProtoPlugin::~CProtoPlugin()
{
  if (m_szLibName)
    free(m_szLibName);
}

bool CProtoPlugin::CompareThread(pthread_t t)
{
  return pthread_equal(t, thread_plugin);
}

void CProtoPlugin::PushSignal(CSignal *s)
{
  pthread_mutex_lock(&mutex_signals);
  list_signals.push_back(s);
  pthread_mutex_unlock(&mutex_signals);
  write(pipe_plugin[PIPE_WRITE], "S", 1);
}

CSignal *CProtoPlugin::PopSignal()
{
  CSignal *s = NULL;
  pthread_mutex_lock(&mutex_signals);
  if (list_signals.size() != 0)
  {
    s = list_signals.front();
    list_signals.pop_front();
  }
  pthread_mutex_unlock(&mutex_signals);
  return s;
}
