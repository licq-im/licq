/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2012 Licq developers <licq-dev@googlegroups.com>
 *
 * Licq is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Licq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Licq; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef LICQ_H
#define LICQ_H

#include "config.h"

#include <pthread.h>
#include <list>

#include <licq/plugin/generalplugin.h>
#include <licq/plugin/protocolplugin.h>
#include <licq/pipe.h>

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

  void shutdown();

  void PrintUsage();
  bool Install();
  void SaveLoadedPlugins();

  // Notifications that can be sent to main thread
  static const char NotifyReapPlugin = 'P';
  static const char NotifyShuttingDown = 'X';

  /**
   * Send a notification to the main thread
   *
   * @param c A character
   */
  void notify(char c)
  { myPipe.putChar(c); }

protected:
  bool upgradeLicq128(Licq::IniFile& licqConf);

  Licq::GeneralPlugin::Ptr LoadPlugin(const std::string& name, int argc,
      char** argv, bool keep = true);
  Licq::ProtocolPlugin::Ptr LoadProtoPlugin(const std::string& name, bool keep = true);

private:
  boost::shared_ptr<LicqDaemon::StreamLogSink> myConsoleLog;
  int myConsoleLogLevel;
  Licq::Pipe myPipe;
};

#endif
