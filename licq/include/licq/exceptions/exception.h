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

#ifndef LICQ_EXCEPTION_H
#define LICQ_EXCEPTION_H

#include <boost/exception/exception.hpp>
#include <boost/throw_exception.hpp>
#include <stdexcept>

#define LICQ_THROW(x) BOOST_THROW_EXCEPTION(x)

namespace Licq
{

/**
 * The base type for all exceptions thrown by Licq.
 */
class Exception : public virtual boost::exception,
                  public virtual std::exception
{
  // Empty
};

} // namespace Licq

#endif
