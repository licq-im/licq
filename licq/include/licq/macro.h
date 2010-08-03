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

#ifndef LICQ_MACRO_H
#define LICQ_MACRO_H

#ifdef __GNUC__
#define LICQ_FORMAT(x, y) __attribute__((format (printf, x, y)))
#else
#define LICQ_FORMAT(x, y)
#endif

#define LICQ_DECLARE_PRIVATE() \
  class Private; \
  Private* const myPrivate; \
  friend class Private

#define LICQ_D() Private* const d = myPrivate
#define LICQ_D_CONST() const Private* const d = myPrivate


// Define for marking functions as deprecated
#ifndef LICQ_DEPRECATED
# if defined(__GNUC__) && !defined(__INTEL_COMPILER) && (__GNUC__ - 0 > 3 || (__GNUC__ - 0 == 3 && __GNUC_MINOR__ - 0 >= 2))
#  define LICQ_DEPRECATED __attribute__ ((__deprecated__))
# elif defined(_MSC_VER) && (_MSC_VER >= 1300)
#  define LICQ_DEPRECATED __declspec(deprecated)
# else
#  define LICQ_DEPRECATED
# endif
#endif


#endif
