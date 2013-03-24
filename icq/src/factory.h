/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2013 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQICQ_FACTORY_H
#define LICQICQ_FACTORY_H

#include <licq/plugin/protocolpluginfactory.h>
#include <licq/icq/icqdata.h>

namespace LicqIcq
{

class Factory : public Licq::ProtocolPluginFactory, public Licq::IcqData
{
public:
  // From Licq::PluginFactory
  std::string name() const;
  std::string version() const;
  void destroyPlugin(Licq::PluginInterface* plugin);

  // From Licq::ProtocolPluginFactory
  unsigned long protocolId() const;
  unsigned long capabilities() const;
  Licq::ProtocolPluginInterface* createPlugin();
  Licq::User* createUser(const Licq::UserId& id, bool temporary);
  Licq::Owner* createOwner(const Licq::UserId& id);

  // From Licq::IcqData
  const struct Licq::IcqCategory* getCategoryByCode(enum Licq::IcqCategoryType type, unsigned short code);
  const struct Licq::IcqCategory* getCategoryByIndex(enum Licq::IcqCategoryType type, unsigned short index);
  const struct Licq::IcqCategory* getCategoryByName(enum Licq::IcqCategoryType type, const char* name);
  const struct Licq::IcqCountry* getCountryByCode(unsigned short countryCode);
  const struct Licq::IcqCountry* getCountryByIndex(unsigned short index);
  const struct Licq::IcqCountry* getCountryByName(const char* name);
  const struct Licq::IcqProvider* getProviderByGateway(const char* gateway);
  const struct Licq::IcqProvider* getProviderByIndex(unsigned short index);
  const struct Licq::IcqProvider* getProviderByName(const char* name);
};

} // namespace LicqIcq

#endif
