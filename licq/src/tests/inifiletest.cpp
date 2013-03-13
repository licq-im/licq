/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2013 Licq Developers <licq-dev@googlegroups.com>
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

#include <licq/inifile.h>

#include <gtest/gtest.h>
#include <string>

using Licq::IniFile;
using std::list;
using std::string;

namespace LicqTest {

TEST(IniFile, rawConfigAccess)
{
  IniFile ini("/tmp/testini.conf");

  // Verify the default config
  EXPECT_EQ("", ini.getRawConfiguration());

  // Verify the raw load and get functions
  string miniConfig("[Section1]\nparam=value\n");
  ini.loadRawConfiguration(miniConfig);
  EXPECT_EQ(miniConfig, ini.getRawConfiguration());

  // Verify that missing newline at end of config is added
  ini.loadRawConfiguration("[Section1]\nparam=0");
  EXPECT_EQ("[Section1]\nparam=0\n", ini.getRawConfiguration());

  // Verify that config is migrated from old format
  ini.loadRawConfiguration("[Section1]\nparam = 0\n[Section2]\ndata = value\n");
  EXPECT_EQ("[Section1]\nparam=0\n[Section2]\ndata=value\n", ini.getRawConfiguration());
}

TEST(IniFile, setSection)
{
  IniFile ini("/tmp/testini.conf");

  // Empty config, try to set non-existing section
  EXPECT_FALSE(ini.setSection("Missing", false));

  // Empty config, try to create non-existing section
  EXPECT_TRUE(ini.setSection("NewSection", true));

  // Find created section
  EXPECT_TRUE(ini.setSection("NewSection", false));

  // Verify that section was created
  EXPECT_EQ("[NewSection]\n", ini.getRawConfiguration());


  // Existing configuration, find sections
  ini.loadRawConfiguration("[FirstSection]\nval=0\n[EmptySection]\n#Comment\n[LastSection]\n");
  EXPECT_TRUE(ini.setSection("EmptySection", false));
  EXPECT_TRUE(ini.setSection("FirstSection", false));
  EXPECT_TRUE(ini.setSection("LastSection", false));
  EXPECT_FALSE(ini.setSection("Missing", false));

  // Create a section, find it and make sure last sections wasn't destroyed
  EXPECT_TRUE(ini.setSection("NewSection", true));
  EXPECT_TRUE(ini.setSection("NewSection", false));
  EXPECT_TRUE(ini.setSection("LastSection", false));

  // Verify that section was created
  EXPECT_EQ("[FirstSection]\nval=0\n[EmptySection]\n#Comment\n[LastSection]\n\n[NewSection]\n", ini.getRawConfiguration());
}

TEST(IniFile, getDefault)
{
  IniFile ini("/tmp/testini.conf");

  // Test with empty config, no section set

  string strRet("");
  EXPECT_FALSE(ini.get("missing", strRet, "default"));
  EXPECT_EQ("default", strRet);

  int signedRet(0);
  EXPECT_FALSE(ini.get("missing", signedRet, -10));
  EXPECT_EQ(-10, signedRet);

  unsigned unsignedRet(0);
  EXPECT_FALSE(ini.get("missing", unsignedRet, 10));
  EXPECT_EQ(10u, unsignedRet);

  bool boolRet(false);
  EXPECT_FALSE(ini.get("missing", boolRet, true));
  EXPECT_EQ(true, boolRet);

  // Test with section set
  ini.loadRawConfiguration("[Section1]\nparam1=1\nparam2=2\n");
  EXPECT_TRUE(ini.setSection("Section1", false));

  string strRet2("");
  EXPECT_FALSE(ini.get("missing", strRet2, "default"));
  EXPECT_EQ("default", strRet2);

  int signedRet2(0);
  EXPECT_FALSE(ini.get("missing", signedRet2, -10));
  EXPECT_EQ(-10, signedRet2);

  unsigned unsignedRet2(0);
  EXPECT_FALSE(ini.get("missing", unsignedRet2, 10));
  EXPECT_EQ(10u, unsignedRet2);

  bool boolRet2(false);
  EXPECT_FALSE(ini.get("missing", boolRet2, true));
  EXPECT_EQ(true, boolRet2);
}

TEST(IniFile, get)
{
  IniFile ini("/tmp/testini.conf");

  ini.loadRawConfiguration("[Section1]\nparam1=0\nparam2=1\n;Comment\nparam3=-10\nparam4=test\n[Section2]\nparam1=1\nparam2=0\n");

  string strRet("");
  int signedRet(0);
  unsigned unsignedRet(0);
  bool boolRet(false);

  // Verify parsing of the different data types
  EXPECT_TRUE(ini.setSection("Section1"));
  EXPECT_TRUE(ini.get("param4", strRet));
  EXPECT_EQ("test", strRet);
  EXPECT_TRUE(ini.get("param2", unsignedRet));
  EXPECT_EQ(1u, unsignedRet);
  EXPECT_TRUE(ini.get("param3", signedRet));
  EXPECT_EQ(-10, signedRet);
  EXPECT_TRUE(ini.get("param2", boolRet));
  EXPECT_TRUE(boolRet);
  EXPECT_TRUE(ini.get("param1", boolRet));
  EXPECT_FALSE(boolRet);

  // Verify that same parameter in different sections aren't mixed
  EXPECT_TRUE(ini.setSection("Section2"));
  EXPECT_TRUE(ini.get("param1", strRet));
  EXPECT_EQ("1", strRet);
  EXPECT_TRUE(ini.get("param2", strRet));
  EXPECT_EQ("0", strRet);
}

TEST(IniFile, set)
{
  IniFile ini("/tmp/testini.conf");

  // Can't set a value with no section set
  EXPECT_FALSE(ini.set("NoSection", "0"));

  // Add a new parameter at the end
  EXPECT_TRUE(ini.setSection("Section", true));
  EXPECT_TRUE(ini.set("param", "value"));
  EXPECT_EQ("[Section]\nparam=value\n", ini.getRawConfiguration());

  // Modify the last parameter
  EXPECT_TRUE(ini.set("param", "newdata"));
  EXPECT_EQ("[Section]\nparam=newdata\n", ini.getRawConfiguration());

  // Add a new parameter in the middle
  ini.loadRawConfiguration("[FirstSection]\nparam1=0\n[Section2]\nparam1=1\n");
  EXPECT_TRUE(ini.setSection("FirstSection", false));
  EXPECT_TRUE(ini.set("param2", 2));
  EXPECT_EQ("[FirstSection]\nparam1=0\nparam2=2\n[Section2]\nparam1=1\n", ini.getRawConfiguration());

  // Modify a parameter in the middle
  EXPECT_TRUE(ini.set("param1", true));
  EXPECT_EQ("[FirstSection]\nparam1=1\nparam2=2\n[Section2]\nparam1=1\n", ini.getRawConfiguration());
}

TEST(IniFile, unset)
{
  IniFile ini("/tmp/testini.conf");

  ini.loadRawConfiguration("[Section]\nparam1=1\nparam2=2\nparam3=3\n");
  EXPECT_TRUE(ini.setSection("Section", true));

  // Remove non-existing parameter
  EXPECT_FALSE(ini.unset("param"));
  EXPECT_EQ("[Section]\nparam1=1\nparam2=2\nparam3=3\n", ini.getRawConfiguration());

  // Remove middle parameter
  EXPECT_TRUE(ini.unset("param2"));
  EXPECT_EQ("[Section]\nparam1=1\nparam3=3\n", ini.getRawConfiguration());

  // Remove last parameter
  EXPECT_TRUE(ini.unset("param3"));
  EXPECT_EQ("[Section]\nparam1=1\n", ini.getRawConfiguration());

  // Remove only parameter
  EXPECT_TRUE(ini.unset("param1"));
  EXPECT_EQ("[Section]\n", ini.getRawConfiguration());
}

TEST(IniFile, getSections)
{
  IniFile ini("/tmp/testini.conf");

  ini.loadRawConfiguration("[Section1]\nparam1=1\n#[Section2]\n[section3]\n[Section4]\n[Section5\n]\n");

  // Get all sections
  list<string> ret;
  ini.getSections(ret, "");
  EXPECT_EQ(3u, ret.size());
  ret.clear();

  // Verify prefix parameter
  ini.getSections(ret, "Section");
  EXPECT_EQ(2u, ret.size());
  ret.clear();
}

TEST(IniFile, removeSection)
{
  IniFile ini("/tmp/testini.conf");

  ini.loadRawConfiguration("[FirstSection]\nparam1=0\nparam2=1\n[EmptySection]\n[AnotherSection]\nparam3=4\n\n[LastSection]\n");

  // Remove empty section
  ini.removeSection("EmptySection");
  EXPECT_EQ("[FirstSection]\nparam1=0\nparam2=1\n[AnotherSection]\nparam3=4\n\n[LastSection]\n", ini.getRawConfiguration());

  // Remove section with empty line at end
  ini.removeSection("AnotherSection");
  EXPECT_EQ("[FirstSection]\nparam1=0\nparam2=1\n[LastSection]\n", ini.getRawConfiguration());

  // Remove last section
  ini.removeSection("LastSection");
  EXPECT_EQ("[FirstSection]\nparam1=0\nparam2=1\n", ini.getRawConfiguration());

  // Remove first (and only) section
  ini.removeSection("FirstSection");
  EXPECT_EQ("", ini.getRawConfiguration());
}

TEST(IniFile, getKeyList)
{
  IniFile ini("/tmp/testini.conf");

  ini.loadRawConfiguration("[Section1]\nparam1=1\nData=2\n#param3=3\nparam4=4\n[Section2]\nparam1=1\nparam2=5\n");

  // No section set should return nothing
  list<string> ret;
  ini.getKeyList(ret, "");
  EXPECT_EQ(0u, ret.size());
  ret.clear();

  // Get all parameters for Section1
  ini.setSection("Section1");
  ini.getKeyList(ret, "");
  EXPECT_EQ(3u, ret.size());
  EXPECT_EQ("param1", *ret.begin());
  ret.clear();

  // Verify prefix parameter
  ini.getKeyList(ret, "param");
  EXPECT_EQ(2u, ret.size());
  EXPECT_EQ("param1", *ret.begin());
  ret.clear();
}

TEST(IniFile, escapedChars)
{
  IniFile ini("/tmp/testini.conf");

  // Verify special characters in set
  EXPECT_TRUE(ini.setSection("Section", true));
  EXPECT_TRUE(ini.set("key", "line\\1\nline\\2\nline\\3"));
  EXPECT_EQ("[Section]\nkey=line\\\\1\\nline\\\\2\\nline\\\\3\n", ini.getRawConfiguration());

  // Verify special characters in get
  string retStr("");
  EXPECT_TRUE(ini.get("key", retStr));
  EXPECT_EQ("line\\1\nline\\2\nline\\3", retStr);
}

TEST(IniFile, hexData)
{
  IniFile ini("/tmp/testini.conf");

  ini.loadRawConfiguration("[Section]\nparam=000123456789ABCDEF00\n");
  EXPECT_TRUE(ini.setSection("Section"));

  // Verify getHex
  string strRet("");
  EXPECT_TRUE(ini.getHex("param", strRet));
  EXPECT_EQ(string("\0\x01\x23\x45\x67\x89\xAB\xCD\xEF\0", 10), strRet);

  // Verify setHex
  EXPECT_TRUE(ini.setHex("param", string("\0\xFE\xDC\xBA\x98\x76\x54\x32\x10\0", 10)));
  EXPECT_EQ("[Section]\nparam=00FEDCBA987654321000\n", ini.getRawConfiguration());
}

} // namespace LicqTest
