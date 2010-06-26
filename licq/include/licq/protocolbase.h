/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq developers
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

#ifndef LICQ_PROTOCOLBASE_H
#define LICQ_PROTOCOLBASE_H

#include <cstdlib>
#include <list>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

char *LProto_Name();

char *LProto_Version();

char *LProto_PPID();

bool LProto_Init();

unsigned long LProto_SendFuncs();

int LProto_Main();

/*--------INTERNAL USE ONLY------------*/

extern pthread_cond_t LP_IdSignal;
extern pthread_mutex_t LP_IdMutex;
extern std::list<unsigned short> LP_Ids;
unsigned short LP_Id;

void LProto_Exit(int _nResult)
{
  int *p = (int *)malloc(sizeof(int));
  *p = _nResult;
  pthread_mutex_lock(&LP_IdMutex);
  LP_Ids.push_back(LP_Id);
  pthread_mutex_unlock(&LP_IdMutex);
  pthread_cond_signal(&LP_IdSignal);
  pthread_exit(p);
}

void *LProto_Main_tep(void* /* argument */)
{
  LProto_Exit(LProto_Main());
  return 0;
}

#ifdef __cplusplus
}
#endif

#endif // PROTOPLUGIN_H
