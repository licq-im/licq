/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq Developers <licq-dev@googlegroups.com>
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

#include "config.h"

#include <licq/inifile.h>

using namespace Jabber;

Config::Proxy::Proxy(const std::string& name) :
  myName(name),
  myType(TYPE_DISABLED),
  myPort(-1)
{
  // Empty
}

Config::Config(const std::string& filename) :
  myFile(NULL),
  myPort(-1),
  myTlsPolicy(gloox::TLSOptional),
  myResource("Licq"),
  myProxy("default")
{
  myFile = new Licq::IniFile(filename);

  if (!myFile->loadFile()) {
    return;
  }

  std::string value;
  myFile->setSection("network");

  myFile->get("Server", myServer);
  myFile->get("Port", myPort, -1);

  myFile->get("TlsPolicy", value, "optional");
  if (value == "disabled")
    myTlsPolicy = gloox::TLSDisabled;
  else if (value == "required")
    myTlsPolicy = gloox::TLSRequired;
  else
    myTlsPolicy = gloox::TLSOptional;

  if (myFile->get("Resource", value) && !value.empty())
    myResource = value;

  myFile->get("Proxy", value);
  if (!value.empty() && myFile->setSection("proxy." + value, false))
  {
    myProxy.myName = value;

    myFile->get("Type", value, "disabled");
    if (value == "http")
      myProxy.myType = Proxy::TYPE_HTTP;
    else
      myProxy.myType = Proxy::TYPE_DISABLED;

    myFile->get("Server", myProxy.myServer);
    myFile->get("Port", myProxy.myPort, -1);
    myFile->get("Username", myProxy.myUsername);
    myFile->get("Password", myProxy.myPassword);
  }
}

Config::~Config()
{
  myFile->setSection("network");

  myFile->set("Server", myServer);
  myFile->set("Port", myPort);

  if (myTlsPolicy == gloox::TLSDisabled)
    myFile->set("TlsPolicy", "disabled");
  else if (myTlsPolicy == gloox::TLSRequired)
    myFile->set("TlsPolicy", "required");
  else if (myTlsPolicy == gloox::TLSOptional)
    myFile->set("TlsPolicy", "optional");

  myFile->set("Resource", myResource);

  myFile->set("Proxy", myProxy.myName);
  myFile->setSection("proxy." + myProxy.myName);

  if (myProxy.myType == Proxy::TYPE_DISABLED)
    myFile->set("Type", "disabled");
  else if (myProxy.myType == Proxy::TYPE_HTTP)
    myFile->set("Type", "http");

  myFile->set("Server", myProxy.myServer);
  myFile->set("Port", myProxy.myPort);
  myFile->set("Username", myProxy.myUsername);
  myFile->set("Password", myProxy.myPassword);

  myFile->writeFile();
  delete myFile;
}
