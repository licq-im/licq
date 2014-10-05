/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2014 Licq Developers <licq-dev@googlegroups.com>
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

#include <licq/exec.h>

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#ifdef __sun
# define _PATH_BSHELL "/bin/sh"
#else
# include <paths.h>
#endif


using namespace Licq;

class Exec::Private
{
public:
  int myPid;
  int myStdIn;
  int myStdOut;
  int myStdErr;
};


Exec::Exec(const char* command)
  : myPrivate(new Private)
{
  int stds[3][2];
  if (pipe(stds[0]) < 0)
    throw std::exception();
  if (pipe(stds[1]) < 0)
  {
    ::close(stds[0][0]);
    ::close(stds[0][1]);
    throw std::exception();
  }
  if (pipe(stds[2]) < 0)
  {
    ::close(stds[0][0]);
    ::close(stds[0][1]);
    ::close(stds[1][0]);
    ::close(stds[1][1]);
    throw std::exception();
  }

  int pid = ::fork();
  if (pid < 0) {
    ::close(stds[0][0]);
    ::close(stds[0][1]);
    ::close(stds[1][0]);
    ::close(stds[1][1]);
    ::close(stds[2][0]);
    ::close(stds[2][1]);
    throw std::exception();
  }

  // Child
  if (pid == 0) {
    ::close(stds[0][1]);
    if (stds[0][0] != STDIN_FILENO)
    {
      ::dup2(stds[0][0], STDIN_FILENO);
      ::close(stds[0][0]);
    }
    ::close(stds[1][0]);
    if (stds[1][1] != STDOUT_FILENO)
    {
      ::dup2(stds[1][1], STDOUT_FILENO);
      ::close(stds[1][1]);
    }
    ::close(stds[2][0]);
    if (stds[2][1] != STDERR_FILENO)
    {
      ::dup2(stds[2][1], STDERR_FILENO);
      ::close(stds[2][1]);
    }

    ::execl(_PATH_BSHELL, "sh", "-c", command, NULL);
    ::_exit(127);
  }


  // Parent
  LICQ_D();
  ::close(stds[0][0]);
  ::close(stds[1][1]);
  ::close(stds[2][1]);
  d->myPid = pid;
  d->myStdIn = stds[0][1];
  d->myStdOut = stds[1][0];
  d->myStdErr = stds[2][0];
}

Exec::~Exec()
{
  LICQ_D();

  kill(0);
  if (d->myStdOut != -1)
    ::close(d->myStdOut);
  if (d->myStdErr != -1)
    ::close(d->myStdErr);
}

int Exec::kill(int timeout)
{
  LICQ_D();

  if (d->myPid == -1)
    return -1;

  if (d->myStdIn != -1)
  {
    ::close(d->myStdIn);
    d->myStdIn = -1;
  }
  // Leave StdOut and StdErr open in case there is data remaining to be read

  int signal = SIGTERM;
  while (true)
  {
    // Is child still alive?
    int pstat;
    int r = ::waitpid(d->myPid, &pstat, WNOHANG);
    if (r == d->myPid || r == -1)
    {
      d->myPid = -1;
      return (WIFEXITED(pstat) ? WEXITSTATUS(pstat) : -1);
    }

    if (signal != 0 && ::kill(d->myPid, signal) < 0)
      return -1;

    if (signal != SIGKILL)
    {
      // Give the process some time to shut down
      ::usleep(timeout);

      if (signal == 0)
        signal = SIGTERM;
      else
        signal = SIGKILL;
    }
  }
}

std::string Exec::process(const std::string& input, bool eoi)
{
  LICQ_D();

  // Feed everything from input to the process
  size_t len = input.length();
  const char *data = input.data();
  while (len > 0)
  {
    ssize_t ret = ::write(d->myStdIn, data, len);
    if (ret <= 0)
      break;
    data += ret;
    len -= ret;
  }

  if (eoi && d->myStdIn != -1)
  {
    ::close(d->myStdIn);
    d->myStdIn = -1;
  }

  // Get output from process
  std::string output;
  while (true)
  {
    char buf[4096];
    ssize_t ret = ::read(d->myStdOut, buf, sizeof(buf));
    if (ret <= 0)
      break;

    output.append(buf, ret);
  }

  return output;
}
