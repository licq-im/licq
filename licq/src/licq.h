#ifndef LICQ_H
#define LICQ_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <pthread.h>
#include <list.h>
#include "licq_plugind.h"

extern char **global_argv;
extern int global_argc;

class CLicq
{
public:
  CLicq();
  ~CLicq();
  bool Init(int argc, char **argv);
  int Main();
  const char *Version();
  CPlugin *LoadPlugin(const char *, int, char **);
  void StartPlugin(CPlugin *);

  void ShutdownPlugins();

  void PrintUsage();
  bool Install();
  void SaveLoadedPlugins();

protected:
  CICQDaemon *licqDaemon;
  unsigned short m_nNextId;
  PluginsList list_plugins;
  pthread_mutex_t mutex_plugins;

friend class CICQDaemon;
};


#endif
