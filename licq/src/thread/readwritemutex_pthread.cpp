/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq Developers <licq-dev@googlegroups.com>
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

#include <licq/thread/readwritemutex.h>

#include <pthread.h>

using Licq::ReadWriteMutex;

class ReadWriteMutex::Private
{
public:
  Private()
  {
    ::pthread_rwlock_init(&myReadWriteMutex, NULL);
  }

  ~Private()
  {
    ::pthread_rwlock_destroy(&myReadWriteMutex);
  }

  pthread_rwlock_t myReadWriteMutex;
};

ReadWriteMutex::ReadWriteMutex() :
  myPrivate(new Private)
{
  // Empty
}

ReadWriteMutex::~ReadWriteMutex()
{
  delete myPrivate;
}

void ReadWriteMutex::lockRead()
{
  LICQ_D();
  ::pthread_rwlock_rdlock(&d->myReadWriteMutex);
}

void ReadWriteMutex::unlockRead()
{
  LICQ_D();
  ::pthread_rwlock_unlock(&d->myReadWriteMutex);
}

void ReadWriteMutex::lockWrite()
{
  LICQ_D();
  ::pthread_rwlock_wrlock(&d->myReadWriteMutex);
}

void ReadWriteMutex::unlockWrite()
{
  LICQ_D();
  ::pthread_rwlock_unlock(&d->myReadWriteMutex);
}

void ReadWriteMutex::setName(const std::string& /*name*/)
{
  // Empty
}
