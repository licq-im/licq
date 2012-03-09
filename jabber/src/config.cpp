/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2012 Licq developers <licq-dev@googlegroups.com>
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

using namespace LicqJabber;

Config::Config(const std::string& filename) :
  myFile(NULL),
  myTlsPolicy(gloox::TLSOptional),
  myResource("Licq")
{
  myFile = new Licq::IniFile(filename);

  if (!myFile->loadFile()) {
    return;
  }

  std::string value;
  myFile->setSection("network");

  myFile->get("TlsPolicy", value, "optional");
  if (value == "disabled")
    myTlsPolicy = gloox::TLSDisabled;
  else if (value == "required")
    myTlsPolicy = gloox::TLSRequired;
  else
    myTlsPolicy = gloox::TLSOptional;

  if (myFile->get("Resource", value) && !value.empty())
    myResource = value;
}

Config::~Config()
{
  myFile->setSection("network");

  if (myTlsPolicy == gloox::TLSDisabled)
    myFile->set("TlsPolicy", "disabled");
  else if (myTlsPolicy == gloox::TLSRequired)
    myFile->set("TlsPolicy", "required");
  else if (myTlsPolicy == gloox::TLSOptional)
    myFile->set("TlsPolicy", "optional");

  myFile->set("Resource", myResource);

  myFile->writeFile();
  delete myFile;
}
