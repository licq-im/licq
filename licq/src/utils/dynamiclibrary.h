/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2010 Licq developers
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

#ifndef LICQDAEMON_DYNAMICLIBRARY_H
#define LICQDAEMON_DYNAMICLIBRARY_H

#include <licq/exceptions/exception.h>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <string>

namespace LicqDaemon
{

/**
 * Used to load dynamic libraries into memory.
 * @ingroup internal
 *
 * Example usage:
 * @code
 * DynamicLibrary libm("libm.so");
 * double (*cos)(double);
 * libm.getSymbol("cos", &cos);
 * double res = cos(3.14);
 * @endcode
 */
class DynamicLibrary : private boost::noncopyable
{
public:
  typedef boost::shared_ptr<DynamicLibrary> Ptr;

  /**
   * The exception type thrown by DynamicLibrary.
   */
  class Exception : public Licq::Exception
  {
  public:
    Exception(const char* error);

    /// @returns The error string as returned from the system.
    std::string getSystemError() const;
  };

  /**
   * Loads a dynamic library.
   *
   * @param[in] filename Library to load. See dlopen(3) on your system
   * for how this name is resolved. Passing an empty string is the
   * same as passing NULL to dlopen().
   *
   * @throws Exception If the library could not be loaded.
  */
  explicit DynamicLibrary(const std::string& filename);

  /**
   * Closes the dynamic library.
   *
   * Before calling the destructor, make absolutely sure that no
   * symbols residing in this library are still in use. Otherwise
   * prepare for a crash.
   */
  ~DynamicLibrary() throw();

  /**
   * Fetches a symbol from the dynamic library.
   *
   * @param[in] name Name of symbol.
   * @param[out] symbol This parameter will point to the symbol if it
   * is found.
   *
   * @throws Exception If the symbol isn't found in the library.
   */
  template<typename SymbolType>
  void getSymbol(const std::string& name, SymbolType** symbol);

  const std::string& getName() const { return myName; }

private:
  const std::string myName;
  void* myDlHandle;
};

template<>
void DynamicLibrary::getSymbol(const std::string& name, void** symbol);

template<typename SymbolType>
inline void DynamicLibrary::getSymbol(
    const std::string& name, SymbolType** symbol)
{
  getSymbol(name, reinterpret_cast<void**>(symbol));
}

} // namespace LicqDaemon

#endif
