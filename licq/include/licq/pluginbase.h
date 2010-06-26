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

#ifndef LICQ_PLUGINBASE_H
#define LICQ_PLUGINBASE_H

/*------------------------------------------------------------------------------
 * Plugin header file
 *
 * Note that except for LP_Exit these functions must be implemented in each
 * plugin.
 *----------------------------------------------------------------------------*/
#include <pthread.h>
#include <list>
#include <cstdlib>

#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------------------------------------------------
 * Name
 *
 * Returns the name of the plugin.  Should use a static character array.
 *----------------------------------------------------------------------------*/
const char *LP_Name();


/*------------------------------------------------------------------------------
 * Version
 *
 * Returns the version of the plugin.  Should use a static character array.
 *----------------------------------------------------------------------------*/
const char *LP_Version();


/*------------------------------------------------------------------------------
 * Status
 *
 * Returns the status of the plugin.  Typically this will be enabled or
 * disabled.  Rarely relevant for a main interface plugin.
 * Should use a static character array.
 *----------------------------------------------------------------------------*/
const char *LP_Status();


/*------------------------------------------------------------------------------
 * Description
 *
 * Returns a brief (one line) description of the plugin
 * Should use a static character array.
 *----------------------------------------------------------------------------*/
const char *LP_Description();


/*------------------------------------------------------------------------------
 * Usage
 *
 * Returns a usage string.
 *----------------------------------------------------------------------------*/
const char *LP_Usage();


/*------------------------------------------------------------------------------
 * Init
 *
 * This function is called to initialize the plugin.  It is passed the
 * relevant command line parameters to be parsed using getopt().  It should
 * return whether or not it started successfully.
 *----------------------------------------------------------------------------*/
bool LP_Init(int, char **);


/*------------------------------------------------------------------------------
 * Main
 *
 * This function is called to actually run the plugin.  It is run in it's own
 * thread so may block. It *must* register with the daemon before sending any
 * requests. It returns an integer return code.
 *----------------------------------------------------------------------------*/
int LP_Main();


/*------------------------------------------------------------------------------
 * Exit
 *
 * This function is called to exit the plugin other then when LP_Main
 * terminates.  It is implemented internally and should not be reimplemented.
 * It takes as argument the exit value for the plugin (0 for successful
 * termination).
 *----------------------------------------------------------------------------*/
void LP_Exit(int);


/*------------------------------------------------------------------------------
 * ConfigFile
 *
 * This function returns the name of the configuration file referenced
 * from the BASE_DIR.  It can be left undefined if no config file is
 * used.
 *----------------------------------------------------------------------------*/
const char *LP_ConfigFile();


/*==============================================================================
 * INTERNAL USE ONLY
 *============================================================================*/


extern pthread_cond_t LP_IdSignal;
extern pthread_mutex_t LP_IdMutex;
extern std::list<unsigned short> LP_Ids;
unsigned short LP_Id;



void *LP_Main_tep(void* /* argument */)
{
  LP_Exit(LP_Main());
  return NULL;
}


char *LP_BuildDate()
{
  static char szDate[] = __DATE__;
  return szDate;
}

char *LP_BuildTime()
{
  static char szTime[] = __TIME__;
  return szTime;
}

void LP_Exit(int _nResult)
{
  int *p = (int *)malloc(sizeof(int));
  *p = _nResult;
  pthread_mutex_lock(&LP_IdMutex);
  LP_Ids.push_back(LP_Id);
  pthread_mutex_unlock(&LP_IdMutex);
  pthread_cond_signal(&LP_IdSignal);
  pthread_exit(p);
}

#ifdef __cplusplus
}
#endif


#endif
