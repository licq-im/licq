/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2011 Licq developers
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

#ifndef LICQAOSD_PLUGIN_H
#define LICQAOSD_PLUGIN_H

#include <licq/plugin/generalplugin.h>


class AosdPlugin : public Licq::GeneralPlugin
{
public:
  AosdPlugin(Params& p);

  // From Licq::GeneralPlugin
  std::string name() const;
  std::string description() const;
  std::string version() const;
  std::string usage() const;
  std::string configFile() const;
  bool isEnabled() const;

protected:
  // From Licq::GeneralPlugin
  bool init(int argc, char** argv);
  int run();
  void destructor();

private:
  bool blocked;
};

#endif
