#ifndef LICQ_H
#define LICQ_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <pthread.h>
#include <list>
#include "licq_plugind.h"
#ifdef PROTOCOL_PLUGIN
#include "licq_protoplugind.h"
#endif

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

#ifdef PROTOCOL_PLUGIN
  CProtoPlugin *LoadProtoPlugin(const char *);
  void StartProtoPlugin(CProtoPlugin *);
  void *FindFunction(void *, const char *);
#endif

  void ShutdownPlugins();

  void PrintUsage();
  bool Install();
  void SaveLoadedPlugins();

protected:
  CICQDaemon *licqDaemon;
  unsigned short m_nNextId;
  PluginsList list_plugins;
  pthread_mutex_t mutex_plugins;
#ifdef PROTOCOL_PLUGIN
  ProtoPluginsList list_protoplugins;
  pthread_mutex_t mutex_protoplugins;
#endif

friend class CICQDaemon;
};


#endif
