#ifndef LICQ_H
#define LICQ_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <pthread.h>
#include <list.h>
#include "icqd.h"


class CLicq
{
public:
  CLicq();
  ~CLicq();
  bool Init(int argc, char **argv);
  int Main();
  const char *Version();
  CPluginFunctions *LoadPlugin(const char *, int, char **);
  void StartPlugin(CPluginFunctions *);
protected:
  void PrintUsage();
  bool Install();
  CICQDaemon *licqDaemon;
  unsigned short m_nNextId;
  PluginsList m_vPluginFunctions;
  pthread_mutex_t mutex_pluginfunctions;

friend class CICQDaemon;
};


#endif
