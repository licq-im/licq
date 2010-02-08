/*
 * Copyright (C) 2008-2010 Anders Olofsson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This header attempts to find endian for the local system and define byte
 * swap functions.
 * As far as possible it uses system headers as these should know best.
 * System headers may also give us native byte swap functions.
 *
 * Known limitations:
 *   Middle endian (BYTE_ORDER == PDP_ENDIAN) systems are not handled
 *
 * The following will be set by this header:
 *   - Either IS_LITTLE_ENDIAN or IS_BIG_ENDIAN set depending on endian type
 *   - Byte swap functions defined as: BSWAP_16, BSWAP_32 and BSWAP_64
 *   - Conversion functions to/from big endian: BE_16, BE_32, BE_64
 *   - Conversion functions to/from little endian: LE_16, LE_32, LE_64
 */
#ifndef LICQ_BYTEORDER_H
#define LICQ_BYTEORDER_H

#include <stdint.h>


// GNU header for endian and byteswap
// (stdint.h gives us __GLIBC__ to check for)
# ifdef __GLIBC__
# include <endian.h>
# include <byteswap.h>

// GNU defines endian by setting __BYTE_ORDER to __BIG_ENDIAN or __LITTLE_ENDIAN
# if __BYTE_ORDER == __LITTLE_ENDIAN
#  define IS_LITTLE_ENDIAN
# endif
# if __BYTE_ORDER == __BIG_ENDIAN
#  define IS_BIG_ENDIAN
# endif

// GNU defines bswap functions: bswap_16, bswap_32, bswap_64
# define BSWAP_16(x) bswap_16(x)
# define BSWAP_32(x) bswap_32(x)
# define BSWAP_64(x) bswap_64(x)


// Mac OS X has __BIG_ENDIAN__ or __LITTLE_ENDIAN__ automatically set by the compiler (at least with GCC)
#elif defined(__APPLE__) && defined(__MACH__)
# ifdef __BIG_ENDIAN__
#  define IS_BIG_ENDIAN
# endif
# ifdef __LITTLE_ENDIAN__
#  define IS_LITTLE_ENDIAN
# endif


// BSD header for endian and byte swap
// Compiler gives us __*BSD__ variables to check for
#elif defined(__FreeBSD__) || defined(__NetBSD__)
# include <sys/endian.h>

// BSD defines endian by setting _BYTE_ORDER to _BIG_ENDIAN or _LITTLE_ENDIAN
# if _BYTE_ORDER == _LITTLE_ENDIAN
#  define IS_LITTLE_ENDIAN
# endif
# if _BYTE_ORDER == _BIG_ENDIAN
#  define IS_BIG_ENDIAN
# endif

// BSD defines bswap functions: bswap16, bswap32, bswap64
# define BSWAP_16(x) bswap16(x)
# define BSWAP_32(x) bswap32(x)
# define BSWAP_64(x) bswap64(x)

// BSD defines conversion functions: be16toh, be32toh, be64toh, le16toh, le32toh, le64toh
# define BE_16(x) be16toh(x)
# define BE_32(x) be32toh(x)
# define BE_64(x) be64toh(x)
# define LE_16(x) le16toh(x)
# define LE_32(x) le32toh(x)
# define LE_64(x) le64toh(x)


// OpenBSD differs from the other BSD systems
#elif defined(__OpenBSD__)
# include <machine/endian.h>

// BSD defines endian by setting _BYTE_ORDER to _BIG_ENDIAN or _LITTLE_ENDIAN
# if _BYTE_ORDER == _LITTLE_ENDIAN
#  define IS_LITTLE_ENDIAN
# endif
# if _BYTE_ORDER == _BIG_ENDIAN
#  define IS_BIG_ENDIAN
# endif

// OpenBSD defines bswap functions: swap16, swap32, swap64
# define BSWAP_16(x) swap16(x)
# define BSWAP_32(x) swap32(x)
# define BSWAP_64(x) swap64(x)

// OpenBSD defines conversion functions: betoh16, betoh32, betoh64, letoh16, letoh32, letoh64
# define BE_16(x) betoh16(x)
# define BE_32(x) betoh32(x)
# define BE_64(x) betoh64(x)
# define LE_16(x) letoh16(x)
# define LE_32(x) letoh32(x)
# define LE_64(x) letoh64(x)


// Solaris header for endian and byte swap
#elif defined(__sun) || defined(sun)
# include <sys/byteorder.h>

// Solaris defines endian by setting _LITTLE_ENDIAN or _BIG_ENDIAN
# ifdef _BIG_ENDIAN
#  define IS_BIG_ENDIAN
# endif
# ifdef _LITTLE_ENDIAN
#  define IS_LITTLE_ENDIAN
# endif

// Solaris 10 defines bswap functions: BSWAP_16, BSWAP_32, BSWAP_64
// Solaris 10 defines conversion functions: BE_16, BE_32, BE_64, LE_16, LE_32, LE_64


// No system specific headers to get endian from, let boost guess from platform type
#else
# include <boost/detail/endian.hpp>

# ifdef BOOST_BIG_ENDIAN
#  define IS_BIG_ENDIAN
# endif
# ifdef BOOST_LITTLE_ENDIAN
#  define IS_LITTLE_ENDIAN
# endif

#endif


// Make sure we got some kind of endian (but not both)
#if defined(IS_BIG_ENDIAN) == defined(IS_LITTLE_ENDIAN)
# error "Failed to get endian type for this system"
#endif


// Define bswap functions if we didn't get any from the system headers
#ifndef BSWAP_16
# define BSWAP_16(x) ( \
    ((uint16_t)(x) & 0x00ffU) << 8 | \
    ((uint16_t)(x) & 0xff00U) >> 8)
#endif
#ifndef BSWAP_32
# define BSWAP_32(x) ( \
    ((uint32_t)(x) & 0x000000ffU) << 24 | \
    ((uint32_t)(x) & 0x0000ff00U) << 8 | \
    ((uint32_t)(x) & 0x00ff0000U) >> 8 | \
    ((uint32_t)(x) & 0xff000000U) >> 24)
#endif
#ifndef BSWAP_64
# define BSWAP_64(x) ( \
    ((uint64_t)(x) & 0x00000000000000ffULL) << 56 | \
    ((uint64_t)(x) & 0x000000000000ff00ULL) << 40 | \
    ((uint64_t)(x) & 0x0000000000ff0000ULL) << 24 | \
    ((uint64_t)(x) & 0x00000000ff000000ULL) << 8 | \
    ((uint64_t)(x) & 0x000000ff00000000ULL) >> 8 | \
    ((uint64_t)(x) & 0x0000ff0000000000ULL) >> 24 | \
    ((uint64_t)(x) & 0x00ff000000000000ULL) >> 40 | \
    ((uint64_t)(x) & 0xff00000000000000ULL) >> 56)
#endif


// Define conversion functions if we didn't get any from the system headers
#ifndef BE_16

// Big endian system, swap when converting to/from little endian
# if defined IS_BIG_ENDIAN
#  define BE_16(x) (x)
#  define BE_32(x) (x)
#  define BE_64(x) (x)
#  define LE_16(x) BSWAP_16(x)
#  define LE_32(x) BSWAP_32(x)
#  define LE_64(x) BSWAP_64(x)

// Little endian system, swap when converting to/from big endian
# elif defined IS_LITTLE_ENDIAN
#  define BE_16(x) BSWAP_16(x)
#  define BE_32(x) BSWAP_32(x)
#  define BE_64(x) BSWAP_64(x)
#  define LE_16(x) (x)
#  define LE_32(x) (x)
#  define LE_64(x) (x)

# endif

#endif


#endif
