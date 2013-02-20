/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2011, 2013 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQDAEMON_GENERALPLUGIN_H
#define LICQDAEMON_GENERALPLUGIN_H

#include <licq/plugin/generalplugin.h>
#include "plugin.h"

namespace Licq
{
class Event;
class GeneralPluginFactory;
class GeneralPluginInterface;
class PluginSignal;
}

namespace LicqDaemon
{

class GeneralPlugin : public Plugin, public Licq::GeneralPlugin
{
public:
  typedef boost::shared_ptr<GeneralPlugin> Ptr;

  GeneralPlugin(int id, DynamicLibrary::Ptr lib, PluginThread::Ptr thread,
                boost::shared_ptr<Licq::GeneralPluginFactory> factory);
  ~GeneralPlugin();

  // From Licq::GeneralPlugin
  std::string description() const;
  std::string usage() const;
  std::string configFile() const;
  bool isEnabled() const;
  void enable();
  void disable();

  bool wantSignal(unsigned long signalType) const;
  void pushSignal(boost::shared_ptr<const Licq::PluginSignal> signal);
  void pushEvent(boost::shared_ptr<const Licq::Event> event);

protected:
  // From Plugin
  void createInterface();
  boost::shared_ptr<const Licq::PluginFactory> factory() const;
  boost::shared_ptr<Licq::PluginInterface> interface();
  boost::shared_ptr<const Licq::PluginInterface> interface() const;

private:
  boost::shared_ptr<Licq::GeneralPluginFactory> myFactory;
  boost::shared_ptr<Licq::GeneralPluginInterface> myInterface;
};

} // namespace LicqDaemon

#endif
