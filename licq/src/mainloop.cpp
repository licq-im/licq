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

#include <licq/mainloop.h>

#include <cassert>
#include <cerrno>
#include <ctime>
#include <map>
#include <unistd.h>

#include <licq/socket.h>

using namespace Licq;


// Base class dummy functions
// These should always be overloaded if used so anything ending up here is a bug

void MainLoopCallback::rawFileEvent(int /* fd */, int /* revents */)
{
  assert(0);
}

void MainLoopCallback::socketEvent(INetSocket* /* inetSocket */, int /* revents */)
{
  assert(0);
}

void MainLoopCallback::timeoutEvent(int /* id */)
{
  assert(0);
}

MainLoopCallback::~MainLoopCallback()
{
  // Empty
}


class MainLoop::Private
{
public:
  // Data for each monitored file
  struct File
  {
    int events;
    MainLoopCallback* callback;
    INetSocket* inetSocket;
  };
  typedef std::map<int, File> FileMap;

  // Data for each timeout
  struct Timeout
  {
    long long last;
    int timeout;
    MainLoopCallback* callback;
    bool once;
    bool removed;
  };
  typedef std::map<int, Timeout> TimeoutMap;

  static long long getMonotonicClock();

  bool myIsRunning;
  bool myFilesHasChanged;
  bool myTimeoutsHasChanged;
  FileMap myFiles;
  TimeoutMap myTimeouts;
};

long long MainLoop::Private::getMonotonicClock()
{
  // Get monotonic time and convert to milliseconds
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

MainLoop::MainLoop()
  : myPrivate(new Private())
{
  // Empty
}

MainLoop::~MainLoop()
{
  delete myPrivate;
}

void MainLoop::run()
{
  LICQ_D();
  d->myIsRunning = true;
  d->myFilesHasChanged = true;
  d->myTimeoutsHasChanged = true;
  struct pollfd* pfds = NULL;
  Private::FileMap::size_type fdcount = 0;
  long long earliest;
  while (d->myIsRunning)
  {
    if (d->myFilesHasChanged)
    {
      // Reallocate pfd size if needed
      Private::FileMap::size_type c = d->myFiles.size();
      if (c != fdcount)
      {
        // Reallocate pfd
        delete[] pfds;
        if (c > 0)
          pfds = new struct pollfd[c];
        else
          pfds = NULL;
        fdcount = c;
      }

      if (fdcount > 0)
      {
        struct pollfd* p = pfds;
        for (Private::FileMap::const_iterator i = d->myFiles.begin(); i != d->myFiles.end(); ++i, ++p)
        {
          p->fd = i->first;
          p->events = i->second.events;
        }
      }

      d->myFilesHasChanged = false;
    }

    if (d->myTimeoutsHasChanged)
    {
      earliest = 0;
      for (Private::TimeoutMap::iterator i = d->myTimeouts.begin(); i != d->myTimeouts.end(); )
      {
        if (i->second.removed)
        {
          // Actually erase timeouts marked for removal
          d->myTimeouts.erase(i++);
          continue;
        }

        // Find the earliest timeout
        if (earliest == 0 || i->second.last + i->second.timeout < earliest)
          earliest = i->second.last + i->second.timeout;

        ++i;
      }
      d->myTimeoutsHasChanged = false;
    }

    int timeout; // Timeout in milliseconds
    if (earliest == 0)
    {
      // No timeouts, wait forever
      timeout = -1;
    }
    else
    {
      // Make timeout relative to current timestamp
      timeout = earliest - Private::getMonotonicClock();
      if (timeout < 0)
        timeout = 0;
    }

    int pollret = poll(pfds, fdcount, timeout);

    if (pollret < 0)
    {
      assert(errno == EINTR);
      continue;
    }

    if (pollret > 0)
    {
      // Check for file events to handle
      for (Private::FileMap::size_type i = 0; i < fdcount; ++i)
      {
        int revents = pfds[i].revents;
        if (revents == 0)
          continue;

        int fd = pfds[i].fd;
        Private::FileMap::iterator iter(d->myFiles.find(fd));
        if (iter == d->myFiles.end())
          // Could happen if one callback removes other files
          continue;

        Private::File& f(iter->second);
        if (f.inetSocket != NULL)
          f.callback->socketEvent(f.inetSocket, revents);
        else
          f.callback->rawFileEvent(fd, revents);
      }
    }

    if (earliest > 0)
    {
      long long now = Private::getMonotonicClock();
      if (now >= earliest)
      {
        // Check for expired timeouts
        for (Private::TimeoutMap::iterator i = d->myTimeouts.begin(); i != d->myTimeouts.end(); ++i)
        {
          Private::Timeout& t(i->second);

          if (t.removed)
            continue;

          if (t.last + t.timeout > now)
            // Not yet
            continue;

          if (t.once)
            // Timeout isn't reoccuring, mark it for removal
            t.removed = true;
          else
            // Timeout is reoccuring, update timestamp
            t.last += t.timeout;

          d->myTimeoutsHasChanged = true;
          t.callback->timeoutEvent(i->first);
        }
      }
    }
  }

  delete[] pfds;
}

void MainLoop::quit()
{
  LICQ_D();

  d->myIsRunning = false;
}

void MainLoop::addRawFile(int fd, MainLoopCallback* callback, int events)
{
  LICQ_D();

  assert(callback != NULL);
  assert(fd > -1);
  assert(d->myFiles.count(fd) == 0);

  Private::File& f = d->myFiles[fd];
  f.events = events;
  f.callback = callback;
  f.inetSocket = NULL;

  d->myFilesHasChanged = true;
}

void MainLoop::removeRawFile(int fd)
{
  LICQ_D();

  d->myFiles.erase(fd);
  d->myFilesHasChanged = true;
}

void MainLoop::addSocket(INetSocket* inetSocket, MainLoopCallback* callback, int events)
{
  LICQ_D();

  assert(callback != NULL);
  assert(inetSocket != NULL);

  int fd = inetSocket->Descriptor();
  assert(fd > -1);
  assert(d->myFiles.count(fd) == 0);

  Private::File& f(d->myFiles[fd]);
  f.events = events;
  f.callback = callback;
  f.inetSocket = inetSocket;;
  d->myFilesHasChanged = true;
}

void MainLoop::removeSocket(INetSocket* inetSocket)
{
  assert(inetSocket != NULL);
  removeRawFile(inetSocket->Descriptor());
}

void MainLoop::addTimeout(int timeout, MainLoopCallback* callback, int id, bool once)
{
  LICQ_D();

  assert(callback != NULL);

  // Don't allow a timeout of zero unless it's a oneshot
  assert(timeout > 0 || once);

  // Allow reusing an entry if it has been marked for removal
  Private::TimeoutMap::const_iterator iter(d->myTimeouts.find(id));
  assert(iter == d->myTimeouts.end() || iter->second.removed);

  Private::Timeout& t(d->myTimeouts[id]);
  t.last = Private::getMonotonicClock();
  t.timeout = timeout;
  t.callback = callback;
  t.once = once;
  t.removed = false;
  d->myTimeoutsHasChanged = true;
}

void MainLoop::removeTimeout(int id)
{
  LICQ_D();

  Private::TimeoutMap::iterator iter(d->myTimeouts.find(id));
  if (iter == d->myTimeouts.end())
    return;

  // Don't break main loop if it currently has an iterator in the list
  // Just mark the entry as removed and let main loop remove it when it's safe
  iter->second.removed = true;
  d->myTimeoutsHasChanged = true;
}

void MainLoop::removeCallback(const MainLoopCallback* callback, bool closeDelete)
{
  LICQ_D();

  // Find and remove all files with this callback object
  for (Private::FileMap::iterator i = d->myFiles.begin(); i != d->myFiles.end(); )
  {
    if (i->second.callback == callback)
    {
      if (closeDelete)
      {
        if (i->second.inetSocket != NULL)
          delete i->second.inetSocket;
        else
          close(i->first);
      }
      d->myFiles.erase(i++);
      d->myFilesHasChanged = true;
    }
    else
      ++i;
  }

  // Find and remove all timeouts with this callback object
  for (Private::TimeoutMap::iterator i = d->myTimeouts.begin(); i != d->myTimeouts.end(); ++i)
  {
    if (i->second.callback == callback)
    {
      i->second.removed = true;
      d->myTimeoutsHasChanged = true;
    }
  }
}

INetSocket* MainLoop::getSocketFromFd(int fd)
{
  LICQ_D();

  if (d->myFiles.count(fd) == 0)
    return NULL;

  return d->myFiles[fd].inetSocket;
}
