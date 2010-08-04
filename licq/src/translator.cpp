// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998-2010 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

// Many parts of this source code were 'inspired' by the ircII4.4 translat.c source.
// RIPPED FROM KVirc: http://www.kvirc.org
// Original by Szymon Stefanek (kvirc@tin.it).
// Modified by Andrew Frolov (dron@linuxer.net)
// Further modified by Graham Roff

#include "config.h"

#include <licq/translator.h>

#include <cerrno>
#include <cstdio>
#include <string.h>
#include <iconv.h>
#include <stdlib.h>

#include <licq/logging/log.h>

using namespace std;
using Licq::Translator;

Licq::Translator Licq::gTranslator;

Translator::Translator()
{
  setDefaultTranslationMap();
}

Translator::~Translator()
{
  // Empty
}

void Translator::setDefaultTranslationMap()
{
  if (myMapDefault)
    return;

  for (int i = 0; i < 256; i++)
  {
    serverToClientTab[i]=i;
    clientToServerTab[i]=i;
  }

  myMapDefault = true;

  myMapName = "";
  myMapFileName = "";
}

bool Translator::setTranslationMap(const string& mapFileName)
{
  // Map name is the file name with no path
  size_t sep = mapFileName.rfind('/');
  string mapName = (sep == string::npos ? mapFileName : mapFileName.substr(sep+1));

  if (mapName == "LATIN_1")
  {
    setDefaultTranslationMap();
    return true;
  }

  FILE* mapFile = fopen(mapFileName.c_str(), "r");
  if (mapFile == NULL)
  {
    gLog.error("Could not open the translation file (%s) for reading:\n%s",
        mapFileName.c_str(), strerror(errno));
    setDefaultTranslationMap();
    return false;
  }

  // translat.c :
  // Any problems in the translation tables between hosts are
  // almost certain to be caused here.
  // many scanf implementations do not work as defined. In particular,
  // scanf should ignore white space including new lines (many stop
  // at the new line character, hence the fgets and sscanf workaround),
  // many fail to read 0xab as a hexadecimal number (failing on the
  // x) despite the 0x being defined as optionally existing on input,
  // and others zero out all the output variables if there is trailing
  // non white space in the format string which doesn't appear on the
  // input. Overall, the standard I/O libraries have a tendancy not
  // to be very standard.

  char buffer[80];
  int inputs[8];
  unsigned char temp_table[512];
  int c = 0;

  while (fgets(buffer, 80, mapFile) != NULL &&
      c < 512)
  {
    if (sscanf(buffer, "0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x",
          inputs+0, inputs+1, inputs+2, inputs+3,
          inputs+4, inputs+5, inputs+6, inputs+7) < 8)
    {
      gLog.error("%sSyntax error in translation file '%s'.\n",
          L_ERRORxSTR, mapFileName.c_str());
      setDefaultTranslationMap();
      fclose(mapFile);
      return false;
    }

    for (int j = 0; j < 8; j++)
      temp_table[c++] = (unsigned char)inputs[j];
  }

  fclose(mapFile);

  if (c == 512)
  {
    for (c = 0; c < 256; c++)
    {
      serverToClientTab[c] = temp_table[c];
      clientToServerTab[c] = temp_table[c | 256];
    }
  }
  else
  {
    gLog.error("%sTranslation file '%s' corrupted.\n",
        L_ERRORxSTR, mapFileName.c_str());
    setDefaultTranslationMap();
    return false;
  }

  myMapDefault = false;
  myMapName = mapName;
  myMapFileName = mapFileName;
  return true;
}

string Translator::serverToClient(const string& s, bool removeCR)
{
  if (myMapDefault)
    return s;

  string s2(removeCR ? returnToUnix(s) : s);

  string ret;
  for (size_t i = 0; i < s.size(); ++i)
    ret += serverToClientTab[(unsigned)s2[i]];
  return ret;
}

char Translator::serverToClient(char c)
{
  return myMapDefault ? c : serverToClientTab[(unsigned)c];
}

string Translator::clientToServer(const string& s, bool addCR)
{
  if (myMapDefault)
    return s;

  string s2(addCR ? returnToDos(s) : s);

  string ret;
  for (size_t i = 0; i < s.size(); ++i)
    ret += clientToServerTab[(unsigned)s[i]];
  return ret;
}

char Translator::clientToServer(char c)
{
  return myMapDefault ? c : clientToServerTab[(unsigned)c];
}

void Translator::ServerToClient(char* array)
{
  if (array == NULL || myMapDefault)
    return;

  char* ptr = array;
  while (*ptr)
  {
    *ptr = serverToClientTab[(unsigned char)(*ptr)];
    ptr++;
  }
}

void Translator::ServerToClient(char& value)
{
  if (!myMapDefault)
    value = serverToClientTab[(unsigned char)(value)];
}

void Translator::ClientToServer(char* array)
{
  if (array == NULL || myMapDefault)
    return;

  char *ptr = array;
  while (*ptr)
  {
    *ptr = clientToServerTab[(unsigned char)(*ptr)];
    ptr++;
  }
}

void Translator::ClientToServer(char& value)
{
  if (!myMapDefault)
    value = clientToServerTab[(unsigned char)(value)];
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
    bool& ok, int length, size_t* outDone)
{
  ok = true;

  size_t inLen = (length > -1 ? length : s.size());
  size_t outLen = inLen * (length == -2 ? 3 : 2);
  size_t outSize = outLen;

  char* result = new char[outLen + 1];

  char* outPtr = result;
  iconv_t tr;

  tr = iconv_open(to.c_str(), from.c_str());
  if (tr == (iconv_t)(-1))
  {
    ok = false;
    gLog.warning("Unsupported encoding conversion from %s to %s.\n",
        from.empty() ? "[LOCALE]" : from.c_str(),
        to.empty() ? "[LOCALE]" : to.c_str());
  }
  else
  {
    const char* inPtr = s.c_str();
    size_t ret = iconv(tr, (ICONV_CONST char**)&inPtr, &inLen, &outPtr, &outLen);
    iconv_close(tr);

    if (outDone != NULL)
      *outDone = outSize - outLen;

    if (ret == (size_t)(-1))
    {
      ok = false;
      gLog.warning("Unable to encode from %s to %s.\n",
          from.empty() ? "[LOCALE]" : from.c_str(),
          to.empty() ? "[LOCALE]" : to.c_str());
    }
  }

  *outPtr = '\0';
  string ret(result);
  delete[] result;
  return ret;
}
