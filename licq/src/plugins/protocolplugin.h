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

#ifndef LICQDAEMON_PROTOCOLPLUGIN_H
#define LICQDAEMON_PROTOCOLPLUGIN_H

#include "plugin.h"

namespace LicqDaemon
{

class ProtocolPlugin : public Plugin
{
public:
  explicit ProtocolPlugin(boost::shared_ptr<DynamicLibrary> lib);
  virtual ~ProtocolPlugin();

  bool init();
  const char* getProtocolId() const;
  unsigned long getSendFunctions() const;

private:
  bool (*myInit)();
  char* (*myPpid)();
  unsigned long (*mySendFunctions)();
};

inline bool ProtocolPlugin::init()
{
  return (*myInit)();
}

inline const char* ProtocolPlugin::getProtocolId() const
{
  return (*myPpid)();
}

inline unsigned long ProtocolPlugin::getSendFunctions() const
{
  return (*mySendFunctions)();
}

} // namespace LicqDaemon

#endif
