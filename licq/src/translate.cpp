//
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

#include "licq_translate.h"
#include "licq_log.h"

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
    if (_szOldStr[i] == (char)0x0A) szNewStr[j++] = 0x0D;
    szNewStr[j++] = _szOldStr[i];
  }
  return (szNewStr);
}



