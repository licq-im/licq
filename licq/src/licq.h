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
  CLicq(void);
  ~CLicq(void);
  bool Init(int argc, char **argv);
  int Main(void);
  const char *Version(void);
  bool LoadPlugin(const char *, int, char **);
protected:
  void PrintUsage(void);
  bool Install(void);
  CICQDaemon *licqDaemon;
  unsigned short m_nNextId;
  PluginsList m_vPluginFunctions;

friend class CICQDaemon;
};


#endif
