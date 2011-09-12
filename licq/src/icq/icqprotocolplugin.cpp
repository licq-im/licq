/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2011 Licq developers
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

#include <licq/plugin/protocolbase.h>

#include <licq/version.h>
#include "icq.h"

#define LicqProtocolPluginData IcqProtocolPluginData

class IcqProtocolPlugin : public Licq::ProtocolPlugin
{
public:
  IcqProtocolPlugin(Params& p);

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

private:

};

IcqProtocolPlugin::IcqProtocolPlugin(Params& p)
  : ProtocolPlugin(p)
{
  // Empty
}

std::string IcqProtocolPlugin::name() const
{
  return "ICQ";
}

std::string IcqProtocolPlugin::version() const
{
  return LICQ_VERSION_STRING;
}

unsigned long IcqProtocolPlugin::protocolId() const
{
  return LICQ_PPID;
}

unsigned long IcqProtocolPlugin::capabilities() const
{
  return ProtocolPlugin::CanSendMsg | ProtocolPlugin::CanSendUrl |
      ProtocolPlugin::CanSendFile | ProtocolPlugin::CanSendChat |
      ProtocolPlugin::CanSendContact | ProtocolPlugin::CanSendAuth |
      ProtocolPlugin::CanSendAuthReq | ProtocolPlugin::CanSendSms |
      ProtocolPlugin::CanSendSecure | ProtocolPlugin::CanSendDirect |
      ProtocolPlugin::CanHoldStatusMsg;
}

std::string IcqProtocolPlugin::defaultServerHost() const
{
  return "login.icq.com";
}

int IcqProtocolPlugin::defaultServerPort() const
{
  return 5190;
}

bool IcqProtocolPlugin::init(int, char**)
{
  gIcqProtocol.initialize();
  return true;
}

int IcqProtocolPlugin::run()
{
  if (!gIcqProtocol.start())
    return 1;
  return 0;
}

void IcqProtocolPlugin::destructor()
{
  delete this;
}

Licq::ProtocolPlugin* IcqPluginFactory(Licq::ProtocolPlugin::Params& p)
{
  return new IcqProtocolPlugin(p);
}

struct Licq::ProtocolPluginData IcqProtocolPluginData = {
    {'L', 'i', 'c', 'q' },      // licqMagic
    LICQ_VERSION,               // licqVersion
    &IcqPluginFactory,          // pluginFactory
};
