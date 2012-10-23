/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2012 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQ_PIPE_H
#define LICQ_PIPE_H

#include <unistd.h>

namespace Licq
{

class Pipe
{
public:
  Pipe();
  ~Pipe();

  /**
   * Reads up to @a count bytes into @a buf.
   *
   * @returns The number of bytes read.
   */
  ssize_t read(void* buf, size_t count);

  /**
   * Writes up to @a count bytes from @a buf.
   *
   * @returns The number of bytes written.
   */
  ssize_t write(const void* buf, size_t count);

  /**
   * (Un)block the write end of the pipe
   *
   * @param block False to make write fail if it would block
   */
  void setWriteBlocking(bool block);

  /**
   * Reads one byte using read().
   */
  char getChar()
  {
    char ch;
    read(&ch, sizeof(ch));
    return ch;
  }

  /**
   * Writes one byte using write().
   */
  bool putChar(char ch)
  { return (write(&ch, sizeof(ch)) > 0); }

  int getReadFd() const { return myFds[0]; }
  int getWriteFd() const { return myFds[1]; }

private:
  int myFds[2];
};

} // namespace Licq

#endif
