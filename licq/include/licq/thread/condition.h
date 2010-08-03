/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq developers
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

#ifndef LICQ_CONDITION_H
#define LICQ_CONDITION_H

#include "mutex.h"

#include <boost/noncopyable.hpp>
#include <climits>
#include <pthread.h>

namespace Licq
{

/**
 * @ingroup thread
 */
class Condition : private boost::noncopyable
{
public:
  static const unsigned int WAIT_FOREVER = UINT_MAX;

  Condition();
  ~Condition();

  void signal();
  void broadcast();
  bool wait(Mutex& mutex, unsigned int msec = WAIT_FOREVER);

private:
  pthread_cond_t myCondition;
};

inline Condition::Condition()
{
  ::pthread_cond_init(&myCondition, NULL);
}

inline Condition::~Condition()
{
  ::pthread_cond_destroy(&myCondition);
}

inline void Condition::signal()
{
  ::pthread_cond_signal(&myCondition);
}

inline void Condition::broadcast()
{
  ::pthread_cond_broadcast(&myCondition);
}

} // namespace Licq

#endif
