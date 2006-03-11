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

#include "licq_translate.h"
#include "licq_log.h"
#include "licq_icq.h"

CTranslator gTranslator;

//============ CTranslator ============//

CTranslator::CTranslator()
{
  m_szMapFileName = NULL;
  m_szMapName = NULL;
  setDefaultTranslationMap();
}

//============ ~CTranslator ============//

CTranslator::~CTranslator()
{
  if (m_szMapFileName != NULL) free(m_szMapFileName);
  if (m_szMapName != NULL) free(m_szMapName);
}

//============ setDefaultTranslationMap ============//

void CTranslator::setDefaultTranslationMap()
{
  for(int i = 0; i < 256; i++)
  {
    serverToClientTab[i]=i;
    clientToServerTab[i]=i;
  }

  m_bDefault = true;
  if (m_szMapFileName != NULL) free(m_szMapFileName);
  if (m_szMapName != NULL) free(m_szMapName);
  m_szMapFileName = strdup("none");
  m_szMapName = strdup("none");

}

//============ setTranslationMap ============//

bool CTranslator::setTranslationMap(const char *_szMapFileName)
{
  // Map name is the file name with no path
  char *szMapName = strrchr(_szMapFileName, '/');
  if (szMapName == NULL)
    m_szMapName = strdup(_szMapFileName);
  else
    m_szMapName = strdup(szMapName + 1);

  if(strcmp(m_szMapName, "LATIN_1") == 0)
  {
    setDefaultTranslationMap();
    return true;
  }

  FILE *mapFile = fopen(_szMapFileName, "r");
  if (mapFile == NULL)
  {
    gLog.Error("%sCould not open the translation file (%s) for reading:\n%s%s.\n",
               L_ERRORxSTR, _szMapFileName, L_BLANKxSTR, strerror(errno));
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

  while(fgets(buffer, 80, mapFile) != NULL && c < 512)
  {
    if(sscanf(buffer, "0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x",
              inputs+0, inputs+1, inputs+2, inputs+3,
              inputs+4, inputs+5, inputs+6, inputs+7) < 8)
    {
      gLog.Error("%sSyntax error in translation file '%s'.\n", L_ERRORxSTR, _szMapFileName);
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
    gLog.Error("%sTranslation file '%s' corrupted.\n", L_ERRORxSTR, _szMapFileName);
    setDefaultTranslationMap();
    return false;
  }

  m_bDefault = false;
  if (m_szMapFileName != NULL) free(m_szMapFileName);
  m_szMapFileName = strdup(_szMapFileName);
  return true;
}

//============ CheckEncoding ================//

unsigned short CTranslator::CheckEncoding(const char *szCheck, int nSize)
{
  // We can't really differentiate between CUSTOM and UNICODE so the
  // daemon will take care of. We just report ASCII or other (in this case
  // UNICODE).
  unsigned short nEncoding = CHARSET_ASCII;
  
  for (int i = 0; i < nSize; i++)
  {
    if ((unsigned char)szCheck[i] >= 0x80)
    {
      nEncoding = CHARSET_UNICODE;
      break;
    }
  }
  
  return nEncoding;
}

//============ translateToClient ============//

void CTranslator::ServerToClient(char *szString)
{
  if (szString == NULL) return;
  if (!m_bDefault)
  {
    char *pC = szString;
    while(*pC)
    {
      *pC = serverToClientTab[(unsigned char)(*pC)];
      pC++;
    }
  }
}


//============ translateToServer ============//

void CTranslator::ClientToServer(char *szString)
{
  if (m_bDefault || szString == NULL) return;
  char *pC = szString;
  while(*pC)
  {
    *pC = clientToServerTab[(unsigned char)(*pC)];
    pC++;
  }
}


//-----translateToClient (char)-------------------------------------------------
void CTranslator::ServerToClient(char &_cChar)
{
  if (m_bDefault) return;
  _cChar = serverToClientTab[(unsigned char)(_cChar)];
}


//-----translateToServer (char)-------------------------------------------------
void CTranslator::ClientToServer(char &_cChar)
{
  if (m_bDefault) return;
  _cChar = clientToServerTab[(unsigned char)(_cChar)];
}


//-----ToUTF8----------------------------------------------------------------
char *CTranslator::ToUnicode(char *_sz, char *_szFrom)
{
  if (_sz == NULL) return NULL;
  unsigned short nLen = strlen(_sz) * 2;
  char *szNewStr = new char[nLen + 1];
  size_t nInSize, nOutSize;

  char *szIn = _sz, *szOut = szNewStr;
  iconv_t tr;

  nInSize = nLen;
  nOutSize = nLen;

  // Clean up for iconv, remove any spaces
  int nFromLen = strlen(_szFrom);
  int j = 0;
  char *szFrom = new char [nFromLen+1];
  for (int i = 0; i < nFromLen; i++)
  {
    if (_szFrom[i] != ' ')
      szFrom[j++] = _szFrom[i];
  }
  szFrom[j] = '\0';
  
  
  tr = iconv_open("UTF-8", szFrom[0] == '\0' ? "" : szFrom);
  if (tr != (iconv_t)-1)
  {
    size_t ret = iconv(tr, (ICONV_CONST char**)&szIn, &nInSize, &szOut, &nOutSize);
    iconv_close(tr);

    if (ret == (size_t)(-1))
    {
      tr = iconv_open("UCS-2BE", szFrom[0] == '\0' ? "" : szFrom);
      if (tr == (iconv_t)-1)
      {
        iconv(tr, (ICONV_CONST char**)&szIn, &nInSize, &szOut, &nOutSize);
        iconv_close(tr);
      }
      else
      {
        gLog.Error("Error encoding to UCS-2BE from %s (unsupported conversion)\n",
                   szFrom);
      }
    }
  }
  else
  {
    gLog.Error("Error encoding to UTF-8 from %s (unsupported conversion)\n",
               szFrom);
  }
  
  *szOut = '\0';

  delete [] szFrom;

  return szNewStr;
}


//-----FromUTF8--------------------------------------------------------------
char *CTranslator::FromUnicode(char *_sz, char *_szTo)
{
  if (_sz == NULL) return NULL;
  unsigned short nLen = strlen(_sz) * 2;
  char *szNewStr = new char[nLen + 1];
  size_t nInSize, nOutSize;

  char *szIn = _sz, *szOut = szNewStr;
  iconv_t tr;

  nInSize = nLen;
  nOutSize = nLen;

  // Clean up for iconv, remove any spaces
  int nToLen = strlen(_szTo);
  int j = 0;
  char *szTo = new char [nToLen+1];
  for (int i = 0; i < nToLen; i++)
  {
    if (_szTo[i] != ' ')
      szTo[j++] = _szTo[i];
  }
  szTo[j] = '\0';


  tr = iconv_open(szTo, "UTF-8");
  if (tr != (iconv_t)-1)
  {
    size_t ret = iconv(tr, (ICONV_CONST char**)&szIn, &nInSize, &szOut, &nOutSize);
    iconv_close(tr);

    if (ret == (size_t)(-1))
    {
      tr = iconv_open(szTo, "UCS-2BE");
      if (tr != (iconv_t)-1)
      {
        iconv(tr, (ICONV_CONST char**)&szIn, &nInSize, &szOut, &nOutSize);
        iconv_close(tr);
      }
      else
      {
        gLog.Error("Error encoding to %s from UCS-2BE (unsupported conversion)\n",
                   szTo);
      }
    }
  }
  else
  {
    gLog.Error("Error encoding to %s from UTF-8 (unsupported conversion)\n",
               szTo);
  }
  
  *szOut = '\0';

  delete [] szTo;

  return szNewStr;
}

//-----FromUTF8--------------------------------------------------------------
char *CTranslator::FromUTF16(char *_sz, int nMsgLen)
{
  if (_sz == NULL) return NULL;
  unsigned short nLen = nMsgLen > 0 ? nMsgLen : strlen(_sz);
  char *szNewStr = new char[nLen * 2];
  size_t nInSize, nOutSize;
  
  char *szIn = _sz, *szOut = szNewStr;
  iconv_t tr;
  
  nInSize = nLen;
  nOutSize = nLen * 2;
  
  tr = iconv_open("", "UCS-2BE");
  if (tr != (iconv_t)-1)
  {
    size_t ret = iconv(tr, (ICONV_CONST char**)&szIn, &nInSize, &szOut, &nOutSize);
    iconv_close(tr);
  
    if (ret == (size_t)(-1))
    {
      gLog.Error("Error decoding a UTF-16 message.\n");
    }
  }
  else
  {
    gLog.Error("Error decoding to UCS-2BE (unsupported conversion)\n");
  }
  
  *szOut = '\0';
  
  return szNewStr;
}

//-----ToUTF16-----------------------------------------------------------------
char *CTranslator::ToUTF16(char *_sz, char *_szEncoding, size_t &nSize)
{
  if (_sz == NULL) return NULL;
  unsigned short nLen = strlen(_sz) * 3;
  char *szNewStr = new char[nLen + 1];
  size_t nInSize, nOutSize;
  char *szOut = szNewStr, *szIn = _sz;
  iconv_t tr;
  
  nInSize = strlen(szIn);
  nOutSize = nLen;

  // Clean up for iconv, remove any spaces
  int nFromLen = strlen(_szEncoding);
  int j = 0;
  char *szFrom = new char [nFromLen+1];
  for (int i = 0; i < nFromLen; i++)
  {
    if (_szEncoding[i] != ' ')
      szFrom[j++] = _szEncoding[i];
  }
  szFrom[j] = '\0';
  
  tr = iconv_open("UCS-2BE", szFrom);
  if (tr != (iconv_t)-1)
  {
    size_t ret = iconv(tr, (ICONV_CONST char**)&szIn, &nInSize, &szOut, &nOutSize);
    iconv_close(tr);
  
    if (ret == (size_t)-1)
      gLog.Error("Error encoding to UTF-16 from %s\n", szFrom);
  }
  else
  {
    gLog.Error("Error encoding to UTF-16 from %s (unsupported conversion)\n",
               szFrom);
  }

  *szOut = '\0';
  nSize = nLen - nOutSize;

  delete [] szFrom;
  
  return szNewStr;
}

bool CTranslator::utf16to8(unsigned long c, string &s)
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

//-----NToRN--------------------------------------------------------------------
char *CTranslator::NToRN(const char *_szOldStr)
// convert a unix style string (0x0A for returns) to a dos style string (0x0A 0x0D)
// also encodes the string if necessary
{
  if (_szOldStr == NULL) return NULL;
  unsigned short nLen = strlen(_szOldStr);
  char *szNewStr = new char[(nLen << 1) + 1];
  unsigned long j = 0;
  for (unsigned long i = 0; i <= nLen; i++)
  {
    if (_szOldStr[i] == (char)0x0A &&
       (!i || _szOldStr[i-1] != (char)0x0D))
      szNewStr[j++] = 0x0D;
    szNewStr[j++] = _szOldStr[i];
  }
  return (szNewStr);
}

//-----RNToN--------------------------------------------------------------------
char *CTranslator::RNToN(const char *_szOldStr)
// converts a dos (CRLF) or mac style (CR) style string to
// a unix style string (LF only)
{
  if (_szOldStr == NULL) return NULL;
  unsigned short nLen = strlen(_szOldStr);

  char *szNewStr = new char[nLen + 1]; // can't be getting longer than the old string
  unsigned long j = 0;

  bool skipCR = false;
  bool skipLF = false;

  for (const char* ptr = _szOldStr; *ptr; ++ptr) {
    if (skipCR && *ptr != '\r')
      skipCR = false;
    if (skipLF && *ptr != '\n')
      skipLF = false;

    if (skipCR || skipLF) {
      skipCR = skipLF = false;
      continue; // skip it
    }

    if (*ptr == '\r') {
      szNewStr[j++] = '\n';
      skipLF = true;
      continue;
    }

    if (*ptr == '\n') {
      szNewStr[j++] = '\n';
      skipCR = true;
      continue;
    }

    szNewStr[j++] = *ptr;
  }

  szNewStr[j] = '\0';

  return szNewStr;
}
