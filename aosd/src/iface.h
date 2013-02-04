/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2010, 2013 Licq developers <licq-dev@googlegroups.com>
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

#ifndef IFACE_H
#define IFACE_H

#include <ctime>
#include <map>
#include <string>

#include <aosd.h>
#include <aosd-text.h>
#undef Status

namespace Licq
{
class PluginSignal;
}

class Conf;

class Iface
{
public:
  Iface();
  ~Iface();

  void processSignal(const Licq::PluginSignal* sig);
  void updateTextRenderData();

private:
  Aosd* aosd;
  TextRenderData trd;
  Conf* conf;
  std::map<unsigned long, time_t> ppidTimers;

  bool filterSignal(const Licq::PluginSignal* sig, unsigned long ppid);

  void displayLayout(std::string& msg, bool control);
};

#endif

/* vim: set ts=2 sw=2 et : */
