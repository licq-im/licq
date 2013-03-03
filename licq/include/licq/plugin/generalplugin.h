/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2011-2013 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQ_GENERALPLUGIN_H
#define LICQ_GENERALPLUGIN_H

#include "generalplugininstance.h"
#include "plugin.h"

namespace Licq
{

/**
 * Represents a loaded general plugin.
 */
class GeneralPlugin : public virtual Plugin
{
public:
  /// A smart pointer to a GeneralPlugin instance
  typedef boost::shared_ptr<GeneralPlugin> Ptr;

  /// Get the plugin's description
  virtual std::string description() const = 0;

  /// Get the plugin's usage instructions
  virtual std::string usage() const = 0;

  /// Configuration file for the plugin. Empty string if none. Path is relative
  /// to BASE_DIR
  virtual std::string configFile() const = 0;

  /// Get the plugin instance for this plugin. May be NULL if the instance has
  /// exited.
  virtual GeneralPluginInstance::Ptr instance() const = 0;

protected:
  /// Destructor
  virtual ~GeneralPlugin() { }
};

} // namespace Licq

#endif
