#ifndef PROTOPLUGIND_H
#define PROTOPLUGIND_H

#include "licq_constants.h"

#include <pthread.h>
#include <list>

class CICQDaemon;
class CLicq;
class CSignal;

typedef std::list<CSignal *>  ProtoSignalList;

class CProtoPlugin
{
public:
  const char *Name()    { if (fName) return (*fName)(); else return "Licq"; }
  const char *Version() { if (fVersion) return (*fVersion)(); else return ""; }
  unsigned short Id()   { if (nId) return *nId; else return 0xFFFF; }
  unsigned long PPID()  { return m_nPPID; }
  
  unsigned long SendFunctions() { return m_nSendFunctions; }

  int Pipe() { return pipe_plugin[PIPE_READ]; }
  void SetSignals(bool b) { m_bSignals = b; }
  
  char *LibName() { return m_szLibName; }
  bool SendSignals() { return m_bSignals; }
  
protected:
  CProtoPlugin(const char *);
  CProtoPlugin(); //FIXME ICQ should be its own plugin
  ~CProtoPlugin();

  void PushSignal(CSignal *);
  CSignal *PopSignal();

  void Shutdown();

  bool CompareThread(pthread_t);
  unsigned long m_nPPID;

  ProtoSignalList list_signals;
  pthread_mutex_t mutex_signals;
  int pipe_plugin[2];
  bool m_bSignals;
  
  // Plugin info for the UI
  unsigned long m_nSendFunctions;

  // DLL items
  void *m_pHandle;
  pthread_t thread_plugin;
  char *m_szLibName;

  // Function pointers needed to export
  bool (*fInit)();
  void *(*fMain_tep)(void *);
  void (*fMain)(CICQDaemon *);
  char *(*fName)();
  char *(*fVersion)();
  char *(*fPPID)();
  unsigned short *nId;
  
  friend class CICQDaemon;
  friend class CLicq;
};

typedef std::list<CProtoPlugin *> ProtoPluginsList;
typedef std::list<CProtoPlugin *>::iterator ProtoPluginsListIter;

#endif // PROTOPLUGIND_H
