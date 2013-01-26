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

#include "plugin.h"

#include <boost/shared_ptr.hpp>

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

  /// Get the plugin's status.
  virtual bool isEnabled() const = 0;

  /// Ask the plugin to enable itself
  virtual void enable() = 0;

  /// Ask the plugin to disable itself
  virtual void disable() = 0;

protected:
  /// Destructor
  virtual ~GeneralPlugin() { }
};

} // namespace Licq

#endif
