/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2013 Licq Developers <licq-dev@googlegroups.com>
 *
 * Please refer to the COPYRIGHT file distributed with this source
 * distribution for the names of the individual contributors.
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

#ifndef LICQ_GENERALPLUGININSTANCE_H
#define LICQ_GENERALPLUGININSTANCE_H

#include "plugininstance.h"

namespace Licq
{

class GeneralPlugin;

/**
 * Represents an instance of a loaded general plugin.
 */
class GeneralPluginInstance : public virtual PluginInstance
{
public:
  typedef boost::shared_ptr<GeneralPluginInstance> Ptr;

  /// Get the plugin for this instance
  virtual boost::shared_ptr<GeneralPlugin> plugin() const = 0;

  /// Get the plugin's status.
  virtual bool isEnabled() const = 0;

  /// Ask the plugin to enable itself
  virtual void enable() = 0;

  /// Ask the plugin to disable itself
  virtual void disable() = 0;

protected:
  virtual ~GeneralPluginInstance() { /* Empty */ }
};

} // namespace Licq

#endif
