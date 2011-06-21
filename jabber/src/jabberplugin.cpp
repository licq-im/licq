/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2011 Licq Developers <licq-dev@googlegroups.com>
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

#include <licq/protocolbase.h>
#include <licq/version.h>

#include "plugin.h"

Licq::ProtocolPlugin* JabberPluginFactory(int id, Licq::Plugin::LibraryPtr lib,
    Licq::Plugin::ThreadPtr thread)
{
  return new Jabber::Plugin(id, lib, thread);
}

struct Licq::ProtocolPluginData LicqProtocolPluginData = {
    {'L', 'i', 'c', 'q' },      // licqMagic
    LICQ_VERSION,               // licqVersion
    &JabberPluginFactory,       // pluginFactory
};
