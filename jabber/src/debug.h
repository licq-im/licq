/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2013 Licq Developers <licq-dev@googlegroups.com>
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

#ifndef LICQJABBER_DEBUG_H
#define LICQJABBER_DEBUG_H

#ifndef TRACE_FORMAT
#define TRACE_FORMAT ""
#endif

#ifdef __GNUC__

#ifdef TRACE_ARGS
#define TRACE(fmt, ...)                                                 \
  Licq::gLog.debug(TRACE_FORMAT fmt, TRACE_ARGS, ## __VA_ARGS__)
#else
#define TRACE(fmt, ...)                                                 \
  Licq::gLog.debug(TRACE_FORMAT fmt, ## __VA_ARGS__)
#endif

#else

#define TRACE(...) 

#endif

#endif
