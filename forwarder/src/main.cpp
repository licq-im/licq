/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2011 Licq developers
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

#include <licq/pluginbase.h>
#include <licq/version.h>

#include "forwarder.h"


Licq::GeneralPlugin* ForwarderPluginFactory(int id,
    Licq::Plugin::LibraryPtr lib, Licq::Plugin::ThreadPtr thread)
{
  return new CLicqForwarder(id, lib, thread);
}

LICQ_GENERAL_PLUGIN_DATA(&ForwarderPluginFactory);
