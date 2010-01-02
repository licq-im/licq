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

#ifndef LICQDAEMON_GENERALPLUGIN_H
#define LICQDAEMON_GENERALPLUGIN_H

#include "plugin.h"

namespace LicqDaemon
{

class GeneralPlugin : public Plugin
{
public:
  explicit GeneralPlugin(boost::shared_ptr<DynamicLibrary> lib);
  virtual ~GeneralPlugin();

  bool init(int argc, char** argv);
  const char* getStatus() const;
  const char* getDescription() const;
  const char* getUsage() const;
  const char* getConfigFile() const;
  const char* getBuildDate() const;
  const char* getBuildTime() const;

private:
  bool (*myInit)(int, char**);
  const char* (*myStatus)();
  const char* (*myDescription)();
  const char* (*myUsage)();
  const char* (*myConfigFile)();
  char* (*myBuildDate)();
  char* (*myBuildTime)();
};

inline bool GeneralPlugin::init(int argc, char** argv)
{
  return (*myInit)(argc, argv);
}

inline const char* GeneralPlugin::getStatus() const
{
  return (*myStatus)();
}

inline const char* GeneralPlugin::getDescription() const
{
  return (*myDescription)();
}

inline const char* GeneralPlugin::getUsage() const
{
  return (*myUsage)();
}

inline const char* GeneralPlugin::getConfigFile() const
{
  if (myConfigFile)
    return (*myConfigFile)();
  else
    return NULL;
}

inline const char* GeneralPlugin::getBuildDate() const
{
  return (*myBuildDate)();
}

inline const char* GeneralPlugin::getBuildTime() const
{
  return (*myBuildTime)();
}

} // namespace LicqDaemon

#endif
