#ifndef PLUGIN_H
#define PLUGIN_H

// Prototype class definition (defined in icq.h)
class CICQDaemon;

/*------------------------------------------------------------------------------
 * Plugin header file
 *
 * Note that except for LP_Exit these functions must be implemented in each
 * plugin.
 *----------------------------------------------------------------------------*/
#include <pthread.h>
#include <list.h>
#include "licq_sighandler.h"

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
 * thread so may block.  It is passed a pointer to the licq daemon with which
 * it *must* register before sending any requests.  It returns an integer
 * return code.
 *----------------------------------------------------------------------------*/
int LP_Main(CICQDaemon *);


/*------------------------------------------------------------------------------
 * Exit
 *
 * This function is called to exit the plugin other then when LP_Main
 * terminates.  It is implemented internally and should not be reimplemented.
 * It takes as argument the exit value for the plugin (0 for successful
 * termination).
 *----------------------------------------------------------------------------*/
void LP_Exit(int);


/*==============================================================================
 * INTERNAL USE ONLY
 *============================================================================*/


extern pthread_cond_t LP_IdSignal;
extern pthread_mutex_t LP_IdMutex;
extern list<unsigned short> LP_Ids;
unsigned short LP_Id;

/*void signal_handler_pluginThread(int s)
{
  if (s == SIGSEGV)
    licq_handle_sigsegv(LP_Name());
}*/
void signal_handler_pluginThread(int s, siginfo_t *si, void *context)
{
  if (s == SIGSEGV)
    licq_handle_sigsegv(LP_Name(), si, context);
}




void *LP_Main_tep(void *p)
{
  //signal(SIGSEGV, &signal_handler_pluginThread);
  licq_segv_handler(&signal_handler_pluginThread);

  LP_Exit(LP_Main((CICQDaemon *)p));
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
