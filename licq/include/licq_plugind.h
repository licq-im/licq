#ifndef PLUGIND_H
#define PLUGIND_H

#include <pthread.h>
#include <list>

#include "licq_constants.h"

class CICQDaemon;
class LicqEvent;
class LicqSignal;

typedef std::list<LicqEvent*> EventList;
typedef std::list<class LicqSignal*> SignalList;

//=====CPlugin==================================================================
class CPlugin
{
public:
  // Object function pointers
  const char *Name()    { return (*fName)(); }
  const char *Version() { return (*fVersion)(); }
  const char *Description() { return (*fDescription)(); }
  const char *Status() { return (*fStatus)(); }
  const char *Usage() { return (*fUsage)(); }
  const char *BuildDate() { return (*fBuildDate)(); }
  const char *BuildTime() { return (*fBuildTime)(); }
  const char *ConfigFile() { return fConfigFile == NULL ? NULL : (*fConfigFile)(); }
  unsigned short Id()   { return *nId; }

  int Pipe() { return pipe_plugin[PIPE_READ]; }
  void SetSignalMask(unsigned long);

  const char *LibName() { return m_szLibName; }

protected:
  CPlugin(const char *);
  ~CPlugin();

  void pushSignal(LicqSignal* signal);
  void PushEvent(LicqEvent* event);
  LicqSignal* popSignal();
  LicqEvent* PopEvent();

  void Enable();
  void Disable();
  void Shutdown();

  bool CompareThread(pthread_t);
  bool CompareMask(unsigned long);

  // Daemon stuff
  EventList list_events;
  SignalList list_signals;
  pthread_mutex_t mutex_events;
  pthread_mutex_t mutex_signals;
  int pipe_plugin[2];
  unsigned long m_nSignalMask;

  // DLL stuff
  void *dl_handle;
  pthread_t thread_plugin;
  int localargc;
  char **localargv;
  char *m_szLibName;

  // Function pointers
  const char *(*fName)();
  const char *(*fVersion)();
  const char *(*fStatus)();
  const char *(*fDescription)();
  const char *(*fBuildDate)();
  const char *(*fBuildTime)();
  const char *(*fUsage)();
  const char *(*fConfigFile)();
  bool (*fInit)(int, char **);
  int (*fMain)(CICQDaemon *);
  void *(*fMain_tep)(void *);
  unsigned short *nId;

friend class CICQDaemon;
friend class CLicq;
};


typedef std::list<CPlugin *> PluginsList;
typedef std::list<CPlugin *>::iterator PluginsListIter;


#endif
