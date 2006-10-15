// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2002-2006 Licq developers
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

/*
 * This will use libc malloc instead of trying to use any other malloc.
 */

#include <sys/types.h>
#include <dlfcn.h>

/* SVID2/XPG mallinfo structure */
struct mallinfo {
  int arena;    /* total space allocated from system */
  int ordblks;  /* number of non-inuse chunks */
  int smblks;   /* unused -- always zero */
  int hblks;    /* number of mmapped regions */
  int hblkhd;   /* total space in mmapped regions */
  int usmblks;  /* unused -- always zero */
  int fsmblks;  /* unused -- always zero */
  int uordblks; /* total allocated space */
  int fordblks; /* total non-inuse space */
  int keepcost; /* top-most, releasable (via malloc_trim) space */
};

namespace
{
void* (*malloc_ptr)(size_t);
void (*free_ptr)(void*);
void* (*realloc_ptr)(void*,size_t);
void* (*memalign_ptr)(size_t,size_t);
void* (*valloc_ptr)(size_t);
void* (*pvalloc_ptr)(size_t);
void* (*calloc_ptr)(size_t,size_t);
void (*cfree_ptr)(void*);
struct mallinfo (*mallinfo_ptr)();
int (*mallopt_ptr)(int,int);

void* dlopen_handle;

void init_symbols()
    {
    dlopen_handle = dlopen( "libc.so", RTLD_LAZY );
    if( dlopen_handle == 0 )
        dlopen_handle = dlopen( "libc.so.6", RTLD_LAZY );
    malloc_ptr = (void* (*)(size_t))dlsym( dlopen_handle, "malloc");
    free_ptr = (void (*)(void*))dlsym( dlopen_handle, "free");
    realloc_ptr = (void* (*)(void*,size_t))dlsym( dlopen_handle, "realloc");
    memalign_ptr = (void* (*)(size_t,size_t))dlsym( dlopen_handle, "memalign");
    valloc_ptr = (void* (*)(size_t))dlsym( dlopen_handle, "valloc");
    pvalloc_ptr = (void* (*)(size_t))dlsym( dlopen_handle, "pvalloc");
    calloc_ptr = (void* (*)(size_t,size_t))dlsym( dlopen_handle, "calloc");
    cfree_ptr = (void (*)(void*))dlsym( dlopen_handle, "cfree");
    mallinfo_ptr = (struct mallinfo (*)())dlsym( dlopen_handle, "mallinfo");
    mallopt_ptr = (int (*)(int,int))dlsym( dlopen_handle, "mallopt");
    if( cfree_ptr == 0 )
        cfree_ptr = free_ptr;
    dlclose( dlopen_handle );
    }

}; // namespace

extern "C"
{

void* malloc( size_t s_P )
    {
    if( malloc_ptr == 0 )
        init_symbols();
    return malloc_ptr( s_P );
    }
    
void free( void* ptr_P )
    {
    if( free_ptr == 0 )
        init_symbols();
    free_ptr( ptr_P );
    }
    
void* realloc( void* ptr_P, size_t s_P )
    {
    if( realloc_ptr == 0 )
        init_symbols();
    return realloc_ptr( ptr_P, s_P );
    }
    
void* memalign( size_t s1_P, size_t s2_P )
    {
    if( memalign_ptr == 0 )
        init_symbols();
    return memalign_ptr( s1_P, s2_P );
    }
    
void* valloc( size_t s_P )
    {
    if( valloc_ptr == 0 )
        init_symbols();
    return valloc_ptr( s_P );
    }
    
void* pvalloc( size_t s_P )
    {
    if( pvalloc_ptr == 0 )
        init_symbols();
    return pvalloc_ptr( s_P );
    }

void* calloc( size_t s1_P, size_t s2_P )
    {
    if( calloc_ptr == 0 )
        init_symbols();
    return calloc_ptr( s1_P, s2_P );
    }

void cfree( void* ptr_P )
    {
    if( cfree_ptr == 0 )
        init_symbols();
    return cfree_ptr( ptr_P );
    }

struct mallinfo mallinfo()
    {
    if( mallinfo_ptr == 0 )
        init_symbols();
    return mallinfo_ptr();
    }

int mallopt( int i1_P, int i2_P )
    {
    if( mallopt_ptr == 0 )
        init_symbols();
    return mallopt_ptr( i1_P, i2_P );
    }

}; // extern "C"
