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

#ifndef LICQ_PLUGININTERFACE_H
#define LICQ_PLUGININTERFACE_H

#include <string>

namespace Licq
{

/**
 * Base interface for plugins
 *
 * Note: When the class implementing this interface (or to be precise, one of
 * GeneralPluginInterface or ProtocolPluginInterface) is constructed, it should
 * only perform minimal initialization. Licq will call init() afterwards to
 * properly initialze the plugin before run() is called to start the plugin.
 *
 * Although a plugin will run in a separate thread, calls to the public
 * functions can be made from any thread. It is the responsibility of the
 * plugin to make sure these functions are thread safe when needed.
 */
class PluginInterface
{
public:
  virtual ~PluginInterface() { /* Empty */ }

  /**
   * Initialize the plugin
   * Called in plugin thread by PluginManager
   *
   * @param argc Number of command line parameters
   * @param argv Command line parameters
   * @return True if initialization was successful
   */
  virtual bool init(int argc, char** argv) = 0;

  /**
   * Run the plugin
   * Called in plugin thread by PluginManager
   *
   * This function will be called in a separate thread and may block
   *
   * @return Exit code for the plugin
   */
  virtual int run() = 0;

  /// Shut down the plugin
  virtual void shutdown() = 0;
};

} // namespace Licq

#endif
