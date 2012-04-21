/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2012 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQICQ_ICQPROTOCOLPLUGIN_H
#define LICQICQ_ICQPROTOCOLPLUGIN_H

#include <licq/plugin/protocolbase.h>

namespace LicqIcq
{

class IcqProtocolPlugin : public Licq::ProtocolPlugin
{
public:
  IcqProtocolPlugin(Params& p);

  /// Read and process next event from plugin pipe
  void processPipe();

  // Make read pipe available to monitor thread
  using Licq::ProtocolPlugin::getReadPipe;

  // From Licq::ProtocolPlugin
  std::string name() const;
  std::string version() const;
  unsigned long protocolId() const;
  unsigned long capabilities() const;
  std::string defaultServerHost() const;
  int defaultServerPort() const;

protected:
  // From Licq::ProtocolPlugin
  bool init(int, char**);
  int run();
  void destructor();
  Licq::User* createUser(const Licq::UserId& id, bool temporary = false);
  Licq::Owner* createOwner(const Licq::UserId& id);

private:

};

extern IcqProtocolPlugin* gIcqProtocolPlugin;

} // namespace LicqIcq

#endif
