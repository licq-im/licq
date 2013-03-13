/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1998-2013 Licq developers <licq-dev@googlegroups.com>
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

// Many parts of this source code were 'inspired' by the ircII4.4 translat.c source.
// RIPPED FROM KVirc: http://www.kvirc.org
// Original by Szymon Stefanek (kvirc@tin.it).
// Modified by Andrew Frolov (dron@linuxer.net)
// Further modified by Graham Roff

#include "config.h"

#include <licq/translator.h>

#include <string.h>
#include <iconv.h>
#include <stdlib.h>

#include <licq/logging/log.h>

#include "gettext.h"

using Licq::Translator;
using std::string;

Licq::Translator Licq::gTranslator;

Translator::Translator()
{
  // Empty
}

Translator::~Translator()
{
  // Empty
}

bool Translator::isAscii(const string& s)
{
  for (size_t i = 0; i < s.size(); ++i)
    if ((unsigned char)s[i] >= 0x80)
      return false;

  return true;
}

string Translator::nameForIconv(const string& licqName)
{
  string ret = licqName;

  size_t i;
  while ((i = ret.find(' ')) != string::npos)
    ret.erase(i, 1);

  return ret;
}

string Translator::fromUnicode(const string& s, const string& toEncoding)
{
  bool ok = true;
  string to = nameForIconv(toEncoding);

  string result = iconvConvert(s, to, "UTF-8", ok);
  if (ok)
    return result;

  result = iconvConvert(s, to, "UCS-2BE", ok);
  return result;
}

string Translator::toUnicode(const string& s, const string& fromEncoding)
{
  bool ok = true;
  string from = nameForIconv(fromEncoding);

  string result = iconvConvert(s, "UTF-8", from, ok);
  if (ok)
    return result;

  result = iconvConvert(s, "UCS-2BE", from, ok);
  if (ok)
    return result;

  result = iconvConvert(s, "UCS-2BE", "UTF-8", ok);
  return result;
}

string Translator::fromUtf16(const string& s, const string& toEncoding)
{
  bool ok = true;
  string to = nameForIconv(toEncoding);

  string result = iconvConvert(s, to, "UCS-2BE", ok);
  if (ok)
    return result;

  result = iconvConvert(s, "UTF-8", "UCS-2BE", ok);
  return result;
}

string Translator::toUtf16(const string& s, const string& fromEncoding)
{
  bool ok = true;
  string from = nameForIconv(fromEncoding);

  return iconvConvert(s, "UCS-2BE", from, ok);
}

string Translator::fromUtf8(const string& s, const string& toEncoding)
{
  bool ok;
  return iconvConvert(s, nameForIconv(toEncoding), "UTF-8", ok);
}

string Translator::toUtf8(const string& s, const string& fromEncoding)
{
  bool ok;
  return iconvConvert(s, "UTF-8", nameForIconv(fromEncoding), ok);
}

bool Translator::utf16to8(unsigned long c, string& s)
{
  if (c <= 0x7F)
  {
    /* Leave ASCII encoded */
    s += (char)c;
  }
  else if (c <= 0x07FF)
  {
    /* 110xxxxx 10xxxxxx */
    s += (char)(0xC0 | (c >> 6));
    s += (char)(0x80 | (c & 0x3F));
  }
  else if (c <= 0xFFFF)
  {
    /* 1110xxxx + 2 */
    s += (char)(0xE0 | (c >> 12));
    s += (char)(0x80 | ((c >> 6) & 0x3F));
    s += (char)(0x80 | (c & 0x3F));
  }
  else if (c <= 0x1FFFFF)
  {
    /* 11110xxx + 3 */
    s += (char)(0xF0 | (c >> 18));
    s += (char)(0x80 | ((c >> 12) & 0x3F));
    s += (char)(0x80 | ((c >> 6) & 0x3F));
    s += (char)(0x80 | (c & 0x3F));
  }
  else if (c <= 0x3FFFFFF)
  {
    /* 111110xx + 4 */
    s += (char)(0xF8 | (c >> 24));
    s += (char)(0x80 | ((c >> 18) & 0x3F));
    s += (char)(0x80 | ((c >> 12) & 0x3F));
    s += (char)(0x80 | ((c >> 6) & 0x3F));
    s += (char)(0x80 | (c & 0x3F));
  }
  else if (c <= 0x7FFFFFFF)
  {
    /* 1111110x + 5 */
    s += (char)(0xFC | (c >> 30));
    s += (char)(0x80 | ((c >> 24) & 0x3F));
    s += (char)(0x80 | ((c >> 18) & 0x3F));
    s += (char)(0x80 | ((c >> 12) & 0x3F));
    s += (char)(0x80 | ((c >> 6) & 0x3F));
    s += (char)(0x80 | (c & 0x3F));
  }
  else
  {
    return false;
  }
  return true;
}

string Translator::returnToDos(const string& s)
{
  string ret = s;
  size_t pos = 0;
  while (1)
  {
    pos = ret.find('\n', pos);
    if (pos == string::npos)
      break;

    if (pos+1 == ret.size() || ret[pos+1] != '\r')
      ret.insert(pos+1, "\r");
    ++pos;
  }
  return ret;
}

string Translator::returnToUnix(const string& s)
{
  string ret = s;

  // Replace all \n\r with \n, and make sure \n\r\r becomes \n\r, not just \n
  size_t pos = 0;
  while ((pos = ret.find("\n\r", pos)) != string::npos)
    ret.replace(pos++, 2, "\n");

  // Replace all remaining \r with \n
  while ((pos = ret.find('\r')) != string::npos)
    ret.replace(pos, 1, "\n");

  return ret;
}

string Translator::iconvConvert(const string& s, const string& to, const string& from,
    bool& ok)
{
  ok = true;

  if (to == from)
    return s;

  size_t inLen = s.size();
  size_t outLen = inLen * 2;
  size_t outSize = outLen;

  char* result = new char[outLen + 1];

  char* outPtr = result;
  iconv_t tr;

  tr = iconv_open(to.c_str(), from.c_str());
  if (tr == (iconv_t)(-1))
  {
    ok = false;
    gLog.warning(tr("Unsupported encoding conversion from %s to %s."),
        from.empty() ? "[LOCALE]" : from.c_str(),
        to.empty() ? "[LOCALE]" : to.c_str());
  }
  else
  {
    const char* inPtr = s.c_str();
    size_t ret = iconv(tr, (ICONV_CONST char**)&inPtr, &inLen, &outPtr, &outLen);
    iconv_close(tr);

    if (ret == (size_t)(-1))
    {
      ok = false;
      gLog.warning(tr("Unable to encode from %s to %s."),
          from.empty() ? "[LOCALE]" : from.c_str(),
          to.empty() ? "[LOCALE]" : to.c_str());
    }
  }

  *outPtr = '\0';
  string ret(result, outSize - outLen);
  delete[] result;
  return ret;
}


#if defined(__APPLE__) && defined(__amd64__)
#define LIBICONV_PLUG 1
#include <iconv.h>
// The following symbols are not defined on 64-bit OS X but are needed by
// libintl. Taken in part from
// http://opensource.apple.com/source/libiconv/libiconv-26/patches/unix03.patch
iconv_t libiconv_open(const char* tocode, const char* fromcode)
{
  return iconv_open(tocode, fromcode);
}

size_t libiconv(iconv_t cd, const char** inbuf, size_t* inbytesleft,
		char** outbuf, size_t* outbytesleft)
{
  return iconv(cd, (char **)inbuf, inbytesleft, outbuf, outbytesleft);
}

int libiconv_close(iconv_t cd)
{
  return iconv_close(cd);
}
#endif
