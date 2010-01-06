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

#ifndef LICQ_PLUGIN_H
#define LICQ_PLUGIN_H

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace Licq
{

class Plugin : private boost::noncopyable
{
public:
  /// @return The plugin's unique id.
  virtual unsigned short getId() const = 0;

  /// @return The plugin's name.
  virtual const char* getName() const = 0;

  /// @return The plugin's version.
  virtual const char* getVersion() const = 0;

  /// @return The name of the library from where the plugin was loaded.
  virtual const std::string& getLibraryName() const = 0;

  /// Ask the plugin to shutdown.
  virtual void shutdown() = 0;

protected:
  virtual ~Plugin() { /* Empty */ }
};

class GeneralPlugin : public virtual Plugin
{
public:
  typedef boost::shared_ptr<GeneralPlugin> Ptr;

  virtual const char* getStatus() const = 0;
  virtual const char* getDescription() const = 0;
  virtual const char* getUsage() const = 0;
  virtual const char* getConfigFile() const = 0;
  virtual const char* getBuildDate() const = 0;
  virtual const char* getBuildTime() const = 0;

  /// Ask the plugin to enable itself.
  virtual void enable() = 0;

  /// Ask the plugin to disable itself.
  virtual void disable() = 0;

protected:
  virtual ~GeneralPlugin() { /* Empty */ }
};

class ProtocolPlugin : public virtual Plugin
{
public:
  typedef boost::shared_ptr<ProtocolPlugin> Ptr;

  virtual unsigned long getProtocolId() const = 0;
  virtual unsigned long getSendFunctions() const = 0;

protected:
  virtual ~ProtocolPlugin() { /* Empty */ }
};

} // namespace Licq

#endif
