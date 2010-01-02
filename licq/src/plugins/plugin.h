/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Erik Johansson <erijo@licq.org>
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

#ifndef LICQDAEMON_PLUGIN_H
#define LICQDAEMON_PLUGIN_H

#include "utils/dynamiclibrary.h"
#include "utils/pipe.h"

#include <boost/exception/info.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <list>
#include <pthread.h>
#include <string>

class CICQDaemon;
class LicqSignal;

namespace LicqDaemon
{

class Plugin : private boost::noncopyable
{
public:
  typedef boost::
  error_info<struct tag_errinfo_symbol_name, std::string> errinfo_symbol_name;

  Plugin(boost::shared_ptr<DynamicLibrary> lib, const std::string& prefix);
  virtual ~Plugin();

  int getReadPipe() const;

  /**
   * Start the plugin in a new thread.
   */
  void startThread(CICQDaemon* daemon);

  /**
   * Wait for the plugin to stop.
   * @returns The plugins exit code.
   */
  int joinThread();

  const char* getName() const;
  const char* getVersion() const;

  unsigned short getId() const;
  void setId(unsigned short id);

  /** Ask the plugin to shutdown. */
  void shutdown();

  void pushSignal(LicqSignal* signal);
  LicqSignal* popSignal();

protected:
  boost::shared_ptr<DynamicLibrary> myLib;
  Pipe myPipe;

  template<typename SymbolType>
  void loadSymbol(const std::string& name, SymbolType*& symbol);

private:
  pthread_t myThread;

  typedef std::list<LicqSignal*> SignalList;
  SignalList mySignals;
  pthread_mutex_t mySignalsMutex;

  // Function pointers
  int (*myMain)(CICQDaemon*);
  void* (*myMainThreadEntryPoint)(void*);
  void (*myExit)(int);
  const char* (*myName)();
  const char* (*myVersion)();

  // Unique plugin id
  unsigned short* myId;
};

inline int Plugin::getReadPipe() const
{
  return myPipe.getReadFd();
}

inline const char* Plugin::getName() const
{
  return (*myName)();
}

inline const char* Plugin::getVersion() const
{
  return (*myVersion)();
}

inline unsigned short Plugin::getId() const
{
  return *myId;
}

inline void Plugin::setId(unsigned short id)
{
  *myId = id;
}

template<typename SymbolType>
inline void Plugin::loadSymbol(const std::string& name, SymbolType*& symbol)
{
  try
  {
    myLib->getSymbol(name, &symbol);
  }
  catch (DynamicLibrary::Exception& ex)
  {
    ex << errinfo_symbol_name(name);
    throw;
  }
}

} // namespace LicqDaemon

#endif
