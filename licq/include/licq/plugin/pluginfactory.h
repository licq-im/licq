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

#ifndef LICQ_PLUGINFACTORY_H
#define LICQ_PLUGINFACTORY_H

#include <string>

namespace Licq
{

class PluginInterface;

/**
 * Base interface for plugin factories
 */
class PluginFactory
{
public:
  /// Return the plugin's name
  virtual std::string name() const = 0;

  /// Return the plugin's version.
  virtual std::string version() const = 0;

  /// Called to destroy a plugin previously created by the factory
  virtual void destroyPlugin(PluginInterface* plugin) = 0;

protected:
  virtual ~PluginFactory() { /* Empty */ }
};

} // namespace Licq

#endif
