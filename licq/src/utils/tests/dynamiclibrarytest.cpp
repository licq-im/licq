/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2013 Licq developers <licq-dev@googlegroups.com>
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

#include "../dynamiclibrary.h"

#include <gtest/gtest.h>

#ifdef __APPLE__
#define LIBM "libm.dylib"
#else
#define LIBM "libm.so"
#endif

extern "C" int testfunc()
{
  return 42;
}

using namespace LicqDaemon;

namespace LicqTest {

TEST(DynamicLibrary, loadLib)
{
  ASSERT_NO_THROW(DynamicLibrary libm(LIBM));
}

TEST(DynamicLibrary, getSymbol)
{
  DynamicLibrary libm(LIBM);

  double (*cos)(double) = NULL;
  ASSERT_NO_THROW(libm.getSymbol("cos", &cos));
  EXPECT_NEAR(cos(3.14), -1.0, 0.01);
}

TEST(DynamicLibrary, getName)
{
  DynamicLibrary libm(LIBM);
  EXPECT_EQ(LIBM, libm.getName());
}

TEST(DynamicLibrary, getMissingSymbolShouldFail)
{
  DynamicLibrary libm(LIBM);

  double (*cos)(double);
  EXPECT_THROW(libm.getSymbol("missingcos", &cos), DynamicLibrary::Exception);
}

TEST(DynamicLibrary, loadMissingLibShouldFail)
{
  ASSERT_THROW(DynamicLibrary("missinglib.foo"), DynamicLibrary::Exception);
}

TEST(DynamicLibrary, loadSelf)
{
  DynamicLibrary self("");

  int (*testfuncSymbol)() = NULL;
  ASSERT_NO_THROW(self.getSymbol("testfunc", &testfuncSymbol));
  EXPECT_TRUE(testfunc == testfuncSymbol);
  EXPECT_EQ(42, testfuncSymbol());
}

TEST(DynamicLibrary, exceptionGetSystemError)
{
  try {
    DynamicLibrary dummy("missinglib.foo");
    FAIL() << "Should not get here";
  }
  catch (DynamicLibrary::Exception& ex) {
    std::string error = ex.getSystemError();
    EXPECT_FALSE(error.empty());
  }
}

} // namespace LicqTest
