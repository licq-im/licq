#ifndef LICQ_H
#define LICQ_H

#include "config.h"
#include "logging/logservice.h"

#include <pthread.h>
#include <list>

#include "licq/plugin.h"

extern char **global_argv;
extern int global_argc;

namespace Licq
{
class IniFile;
}

namespace LicqDaemon
{
class StreamLogSink;
}

class CLicq
{
public:
  CLicq();
  ~CLicq();
  bool Init(int argc, char **argv);
  int Main();
  const char *Version();

  void ShutdownPlugins();

  void PrintUsage();
  bool Install();
  void SaveLoadedPlugins();

  inline LicqDaemon::LogService& getLogService();

protected:
  bool upgradeLicq128(Licq::IniFile& licqConf);

  Licq::GeneralPlugin::Ptr
  LoadPlugin(const char *, int, char **, bool keep = true);
  Licq::ProtocolPlugin::Ptr
  LoadProtoPlugin(const char *, bool keep = true);

private:
  LicqDaemon::LogService myLogService;
  boost::shared_ptr<LicqDaemon::StreamLogSink> myConsoleLog;
  int myConsoleLogLevel;
};

inline LicqDaemon::LogService& CLicq::getLogService()
{
  return myLogService;
}

#endif
