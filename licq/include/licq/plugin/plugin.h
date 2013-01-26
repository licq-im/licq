/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2013 Licq developers <licq-dev@googlegroups.com>
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
#include <string>

namespace Licq
{

/**
 * Represents a loaded plugin.
 */
class Plugin : private boost::noncopyable
{
public:
  /// Get the plugin's unique id.
  virtual int id() const = 0;

  /// Get the plugin's name.
  virtual std::string name() const = 0;

  /// Get the plugin's version.
  virtual std::string version() const = 0;

  /// Get the name of the library from where the plugin was loaded.
  virtual std::string libraryName() const = 0;

protected:
  /// Destructor
  virtual ~Plugin() { }
};

} // namespace Licq

#endif
