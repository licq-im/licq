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

#ifndef LICQ_MAINLOOP_H
#define LICQ_MAINLOOP_H

#include <boost/noncopyable.hpp>
#include <poll.h>

#include "macro.h"

namespace Licq
{
class INetSocket;
class MainLoop;

/**
 * Interface to implement by users of MainLoop to get callbacks.
 * All functions have default implementations that does nothing so only
 * relevant functions needs to be implemented.
 */
class MainLoopCallback
{
protected:
  /**
   * Callback for raw files
   *
   * @param fd File descriptor
   * @param revents Returned events (POLLIN, POLLOUT, etc.)
   */
  virtual void rawFileEvent(int fd, int revents);

  /**
   * Callback for sockets
   *
   * @param inetSocket Socket object
   * @param revents Returned events (POLLIN, POLLOUT, etc.)
   */
  virtual void socketEvent(INetSocket* inetSocket, int revents);

  /**
   * Callback for timeouts
   *
   * @param id Timeout id provided to addTimeout()
   */
  virtual void timeoutEvent(int id);

  /**
   * Destructor
   */
  virtual ~MainLoopCallback();

  friend class MainLoop;
};

/**
 * Mainloop implementation that can be used in plugins to monitor file
 * descriptors and keep track of timeouts.
 *
 * Note: These functions are not thread safe. It is assumed that the MainLoop
 * is the only thread affecting the files and timeouts managed by it.
 */
class MainLoop : private boost::noncopyable
{
public:
  /**
   * Constructor
   */
  MainLoop();

  /**
   * Destructor
   */
  ~MainLoop();

  /**
   * Run the main loop
   * Will not return untill quit() is called
   */
  void run();

  /**
   * Exit mainloop and let run function return
   */
  void quit();

  /**
   * Start monitoring a raw file descriptor
   *
   * @param fd File descriptor to monitor
   * @param events Events to monitor for (POLLIN and/or POLLOUT)
   * @param callback Object to call rawFileEvent on when events occour
   */
  void addRawFile(int fd, MainLoopCallback* callback, int events = POLLIN);

  /**
   * Stop monitoring a raw file descriptor
   *
   * @param fd The file descriptor number
   */
  void removeRawFile(int fd);

  /**
   * Start monitoring a socket
   *
   * @param inetSocket Socket to monitor
   * @param callback Object to call socketEvent on when events occour
   * @param events Events to monitor for (POLLIN and/or POLLOUT)
   */
  void addSocket(INetSocket* inetSocket, MainLoopCallback* callback, int events = POLLIN);

  /**
   * Stop monitoring a socket
   *
   * @param inetSocket Socket to stop monitoring
   */
  void removeSocket(INetSocket* inetSocket);

  /**
   * Add a timeout
   *
   * @param callback Object to call timeoutEvent at timeout
   * @param timeout Timeout in milliseconds
   * @param id An id for this timeout, passed on to callback
   * @param once True to delete timeout after first occurance
   */
  void addTimeout(int timeout, MainLoopCallback* callback, int id = 0, bool once = true);

  /**
   * Cancel a timeout
   *
   * @param id Id of the timeout
   */
  void removeTimeout(int id);

  /**
   * Cancels all timeouts and sockets associated with a callback object
   *
   * @param callback Object to stop callbacks for
   * @param closeDelete True to close all files and delete sockets
   */
  void removeCallback(const MainLoopCallback* callback, bool closeDelete = false);

  /**
   * Convenience function to get socket object for a file descriptor
   *
   * @param fd File descriptor
   * @return The socket object for fd or NULL if not registered
   */
  INetSocket* getSocketFromFd(int fd);

private:
  LICQ_DECLARE_PRIVATE();
};

} // namespace Licq

#endif
