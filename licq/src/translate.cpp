// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998 - 2003 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

// Many parts of this source code were 'inspired' by the ircII4.4 translat.c source.
// RIPPED FROM KVirc: http://www.kvirc.org
// Original by Szymon Stefanek (kvirc@tin.it).
// Modified by Andrew Frolov (dron@linuxer.net)
// Further modified by Graham Roff

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#else
extern int errno;
#endif
#include <string.h>
#include <iconv.h>
#include <stdlib.h>

#include "licq_translate.h"
#include "licq_log.h"
#include "licq_icq.h"

using namespace std;

CTranslator gTranslator;

CTranslator::CTranslator() :
  myMapName(NULL),
  myMapFileName(NULL)
{
  setDefaultTranslationMap();
}

CTranslator::~CTranslator()
{
  if (myMapName != NULL)
    free(myMapName);
  if (myMapFileName != NULL)
    free(myMapFileName);
}

void CTranslator::setDefaultTranslationMap()
{
  if (myMapDefault)
    return;

  for (int i = 0; i < 256; i++)
  {
    serverToClientTab[i]=i;
    clientToServerTab[i]=i;
  }

  myMapDefault = true;

  if (myMapName != NULL)
  {
    free(myMapName);
    myMapName = NULL;
  }
  if (myMapFileName != NULL)
  {
    free(myMapFileName);
    myMapFileName = NULL;
  }
}

bool CTranslator::setTranslationMap(const char* mapFileName)
{
  // Map name is the file name with no path
  char* sep = strrchr(mapFileName, '/');
  const char* mapName = (sep == NULL ? mapFileName : sep + 1);

  if (strcmp(mapName, "LATIN_1") == 0)
  {
    setDefaultTranslationMap();
    return true;
  }

  FILE* mapFile = fopen(mapFileName, "r");
  if (mapFile == NULL)
  {
    gLog.Error("%sCould not open the translation file (%s) for reading:\n%s%s.\n",
        L_ERRORxSTR, mapFileName,
        L_BLANKxSTR, strerror(errno));
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
      gLog.Error("%sSyntax error in translation file '%s'.\n",
          L_ERRORxSTR, mapFileName);
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
    gLog.Error("%sTranslation file '%s' corrupted.\n",
        L_ERRORxSTR, mapFileName);
    setDefaultTranslationMap();
    return false;
  }

  myMapDefault = false;
  if (myMapName != NULL)
    free(myMapName);
  myMapName = strdup(mapName);
  if (myMapFileName != NULL)
    free(myMapFileName);
  myMapFileName = strdup(mapFileName);
  return true;
}

void CTranslator::ServerToClient(char* array)
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

void CTranslator::ServerToClient(char& value)
{
  if (!myMapDefault)
    value = serverToClientTab[(unsigned char)(value)];
}

void CTranslator::ClientToServer(char* array)
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

void CTranslator::ClientToServer(char& value)
{
  if (!myMapDefault)
    value = clientToServerTab[(unsigned char)(value)];
}

bool CTranslator::isAscii(const char* array, int length)
{
  bool ascii = true;

  if (length == -1)
    length = strlen(array);

  for (int i = 0; i < length; i++)
  {
    if ((unsigned char)array[i] >= 0x80)
    {
      ascii = false;
      break;
    }
  }
  
  return ascii;
}

char* CTranslator::nameForIconv(const char* licqName)
{
  size_t i = 0, j = 0;
  size_t len = (licqName == NULL ? 0 : strlen(licqName));
  char* iconvName = new char[len + 1];

  for (; i < len; i++)
    if (licqName[i] != ' ')
      iconvName[j++] = licqName[i];

  iconvName[j] = '\0';

  return iconvName;
}

char* CTranslator::ToUnicode(const char* array, const char* fromEncoding)
{
  if (array == NULL)
    return NULL;

  bool ok = true;
  char* from = nameForIconv(fromEncoding);
  char* result = iconvConvert(array, "UTF-8", from, ok);

  if (!ok)
  {
    delete [] result;
    result = iconvConvert(array, "UCS-2BE", from, ok);
    if (!ok)
    {
      delete [] result;
      result = iconvConvert(array, "UCS-2BE", "UTF-8", ok);
    }
  }

  delete [] from;

  return result;
}

char* CTranslator::FromUnicode(const char* array, const char* toEncoding)
{
  if (array == NULL)
    return NULL;

  bool ok = true;
  char* to = nameForIconv(toEncoding);
  char* result = iconvConvert(array, to, "UTF-8", ok);

  if (!ok)
  {
    delete [] result;
    result = iconvConvert(array, to, "UCS-2BE", ok);
  }

  delete [] to;

  return result;
}

char* CTranslator::FromUTF16(const char* array, const char* toEncoding, int length)
{
  if (array == NULL)
    return NULL;

  bool ok = true;
  char* to = nameForIconv(toEncoding);
  char* result = iconvConvert(array, to, "UCS-2BE", ok, length);

  if (!ok)
  {
    delete [] result;
    result = iconvConvert(array, "UTF-8", "UCS-2BE", ok, length);
  }

  delete [] to;
  
  return result;
}

char* CTranslator::ToUTF16(const char* array, const char* fromEncoding, size_t& outDone)
{
  if (array == NULL)
    return NULL;

  bool ok = true;
  char* from = nameForIconv(fromEncoding);
  char* result = iconvConvert(array, "UCS-2BE", from, ok, -2, &outDone);

  delete [] from;

  return result;
}

bool CTranslator::utf16to8(unsigned long c, string& s)
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

char* CTranslator::NToRN(const char* array)
// convert a unix style string (0x0A for returns) to a dos style string (0x0A 0x0D)
// also encodes the string if necessary
{
  if (array == NULL)
    return NULL;

  unsigned long len = strlen(array);
  char* result = new char[(len << 1) + 1];

  unsigned long i = 0, j = 0;

  for (i = 0, j = 0; i <= len; i++)
  {
    if (array[i] == (char)0x0A &&
        (i == 0 || array[i-1] != (char)0x0D))
      result[j++] = 0x0D;

    result[j++] = array[i];
  }

  return result;
}

char* CTranslator::RNToN(const char* array)
// converts a dos (CRLF) or mac style (CR) style string to
// a unix style string (LF only)
{
  if (array == NULL)
    return NULL;

  unsigned long len = strlen(array);

  char* result = new char[len + 1];
  unsigned long j = 0;

  bool skipCR = false;
  bool skipLF = false;

  for (const char* ptr = array; *ptr != '\0'; ptr++)
  {
    if (skipCR && *ptr != '\r')
      skipCR = false;
    if (skipLF && *ptr != '\n')
      skipLF = false;

    if (skipCR || skipLF)
    {
      skipCR = skipLF = false;
      continue; // skip it
    }

    if (*ptr == '\r')
    {
      result[j++] = '\n';
      skipLF = true;
      continue;
    }

    if (*ptr == '\n')
    {
      result[j++] = '\n';
      skipCR = true;
      continue;
    }

    result[j++] = *ptr;
  }

  result[j] = '\0';

  return result;
}

char* CTranslator::iconvConvert(const char* array, const char* to, const char* from,
    bool& ok, int length, size_t* outDone)
{
  ok = true;

  if (array == NULL)
  {
    ok = false;
    return NULL;
  }

  size_t inLen = (length > -1 ? length : strlen(array));
  size_t outLen = inLen * (length == -2 ? 3 : 2);
  size_t outSize = outLen;

  char* result = new char[outLen + 1];

  char* outPtr = result;
  iconv_t tr;

  tr = iconv_open(to, from);
  if (tr == (iconv_t)(-1))
  {
    ok = false;
    gLog.Warn("Unsupported encoding conversion from %s to %s.\n",
        from[0] == '\0' ? "[LOCALE]" : from,
        to[0] == '\0' ? "[LOCALE]" : to);
  }
  else
  {
    size_t ret = iconv(tr, (ICONV_CONST char**)&array, &inLen, &outPtr, &outLen);
    iconv_close(tr);

    if (outDone != NULL)
      *outDone = outSize - outLen;

    if (ret == (size_t)(-1))
    {
      ok = false;
      gLog.Warn("Unable to encode from %s to %s.\n",
          from[0] == '\0' ? "[LOCALE]" : from,
          to[0] == '\0' ? "[LOCALE]" : to);
    }
  }

  *outPtr = '\0';

  return result;
}
