/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2011 Licq developers
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

#include "pluginthread.h"

#include <licq/pipe.h>
#include <licq/plugin.h>
#include <licq/thread/mutex.h>
#include "utils/dynamiclibrary.h"

#include <boost/exception/info.hpp>
#include <boost/shared_ptr.hpp>
#include <climits>
#include <string>

namespace LicqDaemon
{

class Plugin : public virtual Licq::Plugin
{
public:
  static const unsigned short INVALID_ID = USHRT_MAX;

  typedef boost::shared_ptr<Plugin> Ptr;

  typedef boost::
  error_info<struct tag_errinfo_symbol_name, std::string> errinfo_symbol_name;

  Plugin(DynamicLibrary::Ptr lib, PluginThread::Ptr pluginThread,
         const std::string& prefix);
  virtual ~Plugin();

  /// Get the read end of the pipe used to communicate with the plugin.
  int getReadPipe() const { return myPipe.getReadFd(); }

  /**
   * Start the plugin in a new thread.
   * @param startCallback will be called in the plugin's thread just before the
   * plugin's main function is called.
   * @param exitCallback will be called in the plugin's thread just after the
   * plugin returns from the main function.
   */
  void startThread(void (*startCallback)(const Plugin&) = NULL,
                   void (*exitCallback)(const Plugin&) = NULL);

  /**
   * Wait for the plugin to stop.
   * @returns The plugins exit code.
   */
  int joinThread();

  /// Cancels the plugin's thread.
  void cancelThread();

  /// @return True when called from the plugin's main thread.
  bool isThisThread() const { return isThread(::pthread_self()); }

  /// Check if @a thread is the plugin's thread.
  inline bool isThread(const pthread_t& thread) const;

  /// Set the plugin's unique id.
  void setId(unsigned short id) { myId = id; }

  void setSignalMask(unsigned long mask) { mySignalMask = mask; }

  /// Check if the plugin is interested in the @a signal.
  inline bool wantSignal(unsigned long signal);

  // From Licq::Plugin
  unsigned short getId() const;
  const char* getName() const;
  const char* getVersion() const;
  const char* getConfigFile() const;
  const std::string& getLibraryName() const;
  void shutdown();

protected:
  bool callInitInThread(void (*initCallback)(const Plugin&) = NULL);

  DynamicLibrary::Ptr myLib;
  Licq::Pipe myPipe;

  template<typename SymbolType>
  inline void loadSymbol(const std::string& name, SymbolType*& symbol);

private:
  static bool initThreadEntry(void* plugin);
  virtual bool initThreadEntry() = 0;

  static void* startThreadEntry(void* plugin);

  PluginThread::Ptr myThread;
  unsigned long mySignalMask;
  void (*myInitCallback)(const Plugin&);
  void (*myStartCallback)(const Plugin&);
  void (*myExitCallback)(const Plugin&);

  // Function pointers
  int (*myMain)();
  const char* (*myName)();
  const char* (*myVersion)();
  const char* (*myConfigFile)();

  // Unique plugin id
  unsigned short myId;
};

inline bool Plugin::isThread(const pthread_t& thread) const
{
  return myThread->isThread(thread);
}

inline bool Plugin::wantSignal(unsigned long signal)
{
  return (signal & mySignalMask);
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
