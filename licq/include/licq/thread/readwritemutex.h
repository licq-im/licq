/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2010 Licq developers
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

#ifndef LICQ_READWRITEMUTEX_H
#define LICQ_READWRITEMUTEX_H

#include "../macro.h"

#include <boost/noncopyable.hpp>
#include <string>

namespace Licq
{

/**
 * A read/write mutex.
 *
 * @ingroup thread
 */
class ReadWriteMutex : private boost::noncopyable
{
public:
  /**
   * Construct a read write mutex.
   */
  ReadWriteMutex();

  /**
   * Destructor
   */
  ~ReadWriteMutex();

  /**
   * Get a read lock.
   */
  void lockRead();

  /**
   * Release a read lock.
   */
  void unlockRead();

  /**
   * Get the write lock.
   */
  void lockWrite();

  /**
   * Release the write lock.
   */
  void unlockWrite();

  /**
   * Set mutex name, used for debugging.
   *
   * @param name New name for mutex.
   */
  void setName(const std::string& name);

private:
  LICQ_DECLARE_PRIVATE();
};

} // namespace Licq

#endif
