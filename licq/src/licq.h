#ifndef LICQ_H
#define LICQ_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <pthread.h>

#include "icqd.h"

struct SPluginFunctions
{
  const char *(*Name)(void);
  const char *(*Version)(void);
  void (*Usage)(void);
  bool (*Init)(int, char **);
  int (*Main)(CICQDaemon *);
  void *(*Main_tep)(void *);
  unsigned short *Id;

  void *dl_handle;
  pthread_t thread_plugin;
};


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
  vector<struct SPluginFunctions> m_vPluginFunctions;
};


#endif
