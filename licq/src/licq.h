#ifndef LICQ_H
#define LICQ_H

#include "config.h"
#include "plugins/pluginmanager.h"

#include <pthread.h>
#include <list>
#include "licq_plugind.h"
#include "licq_protoplugind.h"

extern char **global_argv;
extern int global_argc;

class CIniFile;

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

  LicqDaemon::PluginManager& getPluginManager();
  const LicqDaemon::PluginManager& getPluginManager() const;

protected:
  bool UpgradeLicq(CIniFile &);

  LicqDaemon::GeneralPlugin::Ptr
  LoadPlugin(const char *, int, char **, bool keep = true);
  LicqDaemon::ProtocolPlugin::Ptr
  LoadProtoPlugin(const char *, bool keep = true);

  CICQDaemon *licqDaemon;

private:
  LicqDaemon::PluginManager myPluginManager;
};

inline LicqDaemon::PluginManager& CLicq::getPluginManager()
{
  return myPluginManager;
}

inline const LicqDaemon::PluginManager& CLicq::getPluginManager() const
{
  return myPluginManager;
}

#endif
