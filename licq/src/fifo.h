/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2013 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQDAEMON_FIFO_H
#define LICQDAEMON_FIFO_H

#include <boost/noncopyable.hpp>
#include <cstdio>
#include <string>

#include <licq/mainloop.h>

namespace LicqDaemon
{

class Fifo : public Licq::MainLoopCallback, private boost::noncopyable
{
public:
  Fifo();
  ~Fifo();

  /**
   * Initialize fifo
   * Only called once at startup
   */
  void initialize(Licq::MainLoop& mainLoop);

  /**
   * Shut down fifo
   * Only called once at shutdown
   */
  void shutdown();

private:
  // From Licq::MainLoopCallback
  void rawFileEvent(int fd, int revents);

  int fifo_fd;
  std::string myInputBuffer;
};

extern Fifo gFifo;

} // namespace LicqDaemon

#endif
