#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <ctype.h>

#ifdef HAVE_ERRNO_H
#include <errno.h>
#else
extern int errno;
#endif

#include "licq_file.h"
#include "licq_log.h"


//=====Pre class helper functions==============================================
/*-----Trim--------------------------------------------------------------------
 * Removes leading and trailing spaces from a string
 *---------------------------------------------------------------------------*/
void Trim(char *_sz)
{
  char* b, *e;

  b = _sz; while(*b && isspace(*b))  b++;

  // b is now on the first non space character
  e = b; while(*e)  e++;

  // zero string
  if(b == e) {
    *_sz = '\0';
    return;
  }

  // e is now on the last character
  e--;  while(e != b && isspace(*e))  e--;  e++;

  // now b is the beginning and e on the supposed 0 byte
  // of the new string, lets copy if necessary
  if(b != _sz) {
    char* p = _sz;
    while(b != e)
      *p++ = *b++;
    *p = '\0';
  }

  // and now fix the end byte
  *e = '\0';
}


/*-----AddNewLines-------------------------------------------------------------
 * Replaces all occurences of "\n" in a string by '\n'
 *---------------------------------------------------------------------------*/
void AddNewLines(char *_szDest, const char *_szSource)
{
  if (_szSource == NULL || _szDest == NULL)
  {
    gLog.Warn("%sInternal Error: AddNewLines(): _szDest == NULL or _szSource == NULL.\n", L_WARNxSTR);
    return;
  }
  int i = 0, j = 0;
  while (_szSource[i] != '\0')
  {
    if (_szSource[i] == '\\' && _szSource[i + 1] == 'n')
    {
      _szDest[j++] = '\n';
      i += 2;
    }
    else
    {
      _szDest[j++] = _szSource[i];
      i++;
    }
  }
  _szDest[j] = '\0';
}


/*-----RemoveNewLines----------------------------------------------------------
 * Replaces all occurences of '\n' in a string by "\n"
 *---------------------------------------------------------------------------*/
void RemoveNewLines(char *_szDest, const char *_szSource)
{
  if (_szSource == NULL || _szDest == NULL)
  {
    gLog.Warn("%sInternal Error: RemoveNewLines(): _szDest == NULL or _szSource == NULL.\n", L_WARNxSTR);
    return;
  }
  int i = 0, j = 0;
  while (_szSource[i] != '\0')
  {
    if (_szSource[i] == '\n')
    {
      _szDest[j++] = '\\';
      _szDest[j++] = 'n';
    }
    else
      _szDest[j++] = _szSource[i];
    i++;
  }
  _szDest[j] = '\0';
}


//=====CIniFile================================================================


//-----Constructor-------------------------------------------------------------
CIniFile::CIniFile(unsigned short _nFlags)
{
  m_nBufPos = 0;
  m_nBufSize = 0;
  m_nSectionStart = 0;
  m_nSectionEnd = 0;
  m_nError = 0;
  m_nFlags = _nFlags;
  m_szBuffer = NULL;
  m_szSectionName = NULL;
  m_szFilename = NULL;
  /*m_bEof = m_bEos = */m_bChanged = false;
}


//-----Destructor--------------------------------------------------------------
CIniFile::~CIniFile()
{
  CloseFile();
  if (m_szFilename != NULL) free (m_szFilename);
}


//-----CloseFile---------------------------------------------------------------
void CIniFile::CloseFile()
{
  if (m_szBuffer != NULL)
  {
    free (m_szBuffer);
    m_szBuffer = NULL;
  }
  if (m_szSectionName != NULL)
  {
    free (m_szSectionName);
    m_szSectionName = NULL;
  }

  m_nBufPos = 0;
  m_nBufSize = 0;
  m_nSectionStart = 0;
  m_nSectionEnd = 0;
  m_nError = 0;
  /*m_bEof = m_bEos = */m_bChanged = false;
}


//-----LoadFile----------------------------------------------------------------
bool CIniFile::LoadFile(const char *_szFilename)
{
  CloseFile();
  m_szFilename = strdup(_szFilename);

  int nFD;
  if (GetFlag(INI_FxALLOWxCREATE))
    nFD = open(m_szFilename, O_RDONLY | O_CREAT, 00664);
  else
    nFD = open(m_szFilename, O_RDONLY);
  if (nFD < 0)
  {
    m_nError = errno;
    Warn(INI_ExIOREAD);
    return false;
  }

  struct stat sFileStats;
  fstat(nFD, &sFileStats);
  m_szBuffer = (char *)malloc((unsigned int)(sFileStats.st_size + 2));
  m_nBufSize = read(nFD, m_szBuffer, (unsigned int)sFileStats.st_size);
  close (nFD);

  if (m_nBufSize < 0)
  {
    free(m_szBuffer);
    m_nError = errno;
    Warn(INI_ExIOREAD);
    return false;
  }
  // Ensure that the file ends in a newline and null
  m_szBuffer[m_nBufSize] = '\0';
  if (m_nBufSize == 0 || m_szBuffer[m_nBufSize - 1] != '\n')
  {
     m_szBuffer[m_nBufSize] = '\n';
     m_szBuffer[m_nBufSize + 1] = '\0';
     m_nBufSize++;
  }
  m_nBufPos = 0;
  m_nSectionStart = 0;
  m_nSectionEnd = m_nBufSize;
  /*m_bEof = m_bEos = */
  m_bChanged = false;
  return true;
}


//-----ReloadFile--------------------------------------------------------------
bool CIniFile::ReloadFile()
{
  return(LoadFile(m_szFilename));
}


//-----FlushFile---------------------------------------------------------------
bool CIniFile::FlushFile()
{
  // Write files atomically to avoid config trashing
  char tempname[MAX_FILENAME_LEN];

  strcpy(tempname, m_szFilename);
  strcat(tempname, ".new");

  int nFD = open(tempname, O_WRONLY | O_CREAT | O_TRUNC, 00664);
  if (nFD < 0)
  {
     m_nError = errno;
     Warn(INI_ExIOWRITE);
     return false;
  }

  if (write(nFD, m_szBuffer, m_nBufSize) != m_nBufSize) 
  {
    // something went wrong, clean up temp file
    m_nError = errno;
    Warn(INI_ExIOWRITE);
    close (nFD);
    unlink(tempname);
    return false;
  }
  else
  {
    close (nFD);
    if(rename(tempname, m_szFilename))
    {
        // rename failed..
        unlink(tempname);
        return false;
    }
    return true;
  }
}


void CIniFile::ResetFile()
{
   m_nBufPos = m_nSectionStart = 0;
   m_nSectionEnd = m_nBufSize;
   //m_bEos = m_bEof = false;
}


void CIniFile::ResetSection()
{
   m_nBufPos = m_nSectionStart;
   //m_bEos = m_bEof = false;
}


void CIniFile::SetFileName(const char *_szFilename)
{
  if (m_szFilename != NULL) free(m_szFilename);
  m_szFilename = strdup(_szFilename);
}


/*-----Warn--------------------------------------------------------------------
 * Print out an error message using the standard log.  Can output warnings
 * and/or errors.  If the fatal flag is set, the program will terminate.
 *---------------------------------------------------------------------------*/
void CIniFile::Warn(int nError, const char *_sz)
{
  switch(nError)
  {
  case INI_ExNOKEY:
    if (GetFlag(INI_FxWARN))
      gLog.Warn("%sIniFile: Warning, failed to find key.\n%sFile    = %s\n%sSection = [%s]\n%sKey     = \"%s\"\n",
                L_WARNxSTR, L_BLANKxSTR, m_szFilename, L_BLANKxSTR,
                m_szSectionName, L_BLANKxSTR, _sz);
    if(GetFlag(INI_FxERROR))
      gLog.Error("%sIniFile: Warning, failed to find key.\n%sFile    = %s\n%sSection = [%s]\n%sKey     = \"%s\"\n",
                L_ERRORxSTR, L_BLANKxSTR, m_szFilename, L_BLANKxSTR,
                 m_szSectionName, L_BLANKxSTR, _sz);
    break;

  case INI_ExNOSECTION:
    if (GetFlag(INI_FxWARN))
      gLog.Warn("%sIniFile: Warning, failed to find section.\n%sFile    = %s\n%sSection = [%s]\n",
                L_WARNxSTR, L_BLANKxSTR, m_szFilename, L_BLANKxSTR, _sz);
    if(GetFlag(INI_FxERROR))
      gLog.Error("%sIniFile: Warning, failed to find section.\n%sFile    = %s\n%sSection = [%s]\n",
                L_ERRORxSTR, L_BLANKxSTR, m_szFilename, L_BLANKxSTR, _sz);
    break;

  case INI_ExFORMAT:
    if (GetFlag(INI_FxWARN))
      gLog.Warn("%sIniFile: Warning, invalid file format.\n%sFile = %s\n%sLine = %s\n",
                L_WARNxSTR, L_BLANKxSTR, m_szFilename, L_BLANKxSTR, _sz);
    if(GetFlag(INI_FxERROR))
      gLog.Error("%sIniFile: Warning, invalid file format.\n%sFile = %s\n%sLine = %s\n",
                L_ERRORxSTR, L_BLANKxSTR, m_szFilename, L_BLANKxSTR, _sz);
    break;

  case INI_ExIOREAD:
    if (GetFlag(INI_FxWARN))
      gLog.Warn("%sIniFile: Warning, I/O read error.\n%sFile  = %s\n%sError = %s\n",
                L_WARNxSTR, L_BLANKxSTR, m_szFilename, L_BLANKxSTR,
                strerror(Error()));
    if(GetFlag(INI_FxERROR))
      gLog.Error("%sIniFile: Warning, I/O read error.\n%sFile  = %s\n%sError = %s\n",
                 L_ERRORxSTR, L_BLANKxSTR, m_szFilename, L_BLANKxSTR,
                 strerror(Error()));
    break;

  case INI_ExIOWRITE:
    if (GetFlag(INI_FxWARN))
      gLog.Warn("%sIniFile: Warning, I/O write error.\n%sFile  = %s\n%sError = %s\n",
                L_WARNxSTR, L_BLANKxSTR, m_szFilename, L_BLANKxSTR,
                strerror(Error()));
    if(GetFlag(INI_FxERROR))
      gLog.Error("%sIniFile: Warning, I/O write error.\n%sFile  = %s\n%sError = %s\n",
                 L_ERRORxSTR, L_BLANKxSTR, m_szFilename, L_BLANKxSTR,
                 strerror(Error()));
    break;
  }

  if (GetFlag(INI_FxFATAL))
  {
    gLog.Error("%sIniFile: Fatal error, terminating program.\n", L_ERRORxSTR);
    exit(1);
  }
}


/*-----ReadLine----------------------------------------------------------------
 * Reads in characters from the file starting with the current position and
 * ending at the first new line.  Returns NULL if we are already at the EOF
 * or EOS.  Will not return lines beginning with a # (comments)
 *---------------------------------------------------------------------------*/
char *CIniFile::ReadLine(char *_szBuffer)
{
  int i = 0;
  if (m_nBufPos >= m_nBufSize)
  {
     //m_bEof = true;
     //m_bEos = true;
     return (NULL);
  }
  else if (m_nBufPos >= m_nSectionEnd)
  {
     //m_bEos = true;
     return (NULL);
  }

  // the buffer will always end in a newline, so we can just check for it
  while (m_szBuffer[m_nBufPos] != '\n')
    _szBuffer[i++] = m_szBuffer[m_nBufPos++];
  // Increase the buffer pos to get past the newline
  m_nBufPos++;
  // Replace the newline with a null
  _szBuffer[i] = '\0';

  return(_szBuffer);
}


/*-----GetSectionFromLine------------------------------------------------------
 * Extracts the section name from a given line of text.  Returns an empty
 * string if the line does not contain a section, or NULL is the given line
 * is NULL or there is an open ([) with no closing (]).
 *---------------------------------------------------------------------------*/
char *CIniFile::GetSectionFromLine(char *_szLine, char *_szBuffer)
{
  //static char s_szSectionName[MAX_SECTIONxNAME_LEN];

  if (_szLine == NULL) return NULL;
  if (_szLine[0] != '[')
  {
    _szBuffer[0] = '\0';
  }
  else
  {
    int i = 1;
    int j = 0;
    while (_szLine[i] != ']' && _szLine[i] != '\0')
      _szBuffer[j++] = _szLine[i++];
    if (_szLine[i] == '\0')
    {
       Warn(INI_ExFORMAT, _szLine);
       return NULL;
    }

    _szBuffer[j] = '\0';
    Trim(_szBuffer);
  }
  return (_szBuffer);
}


/*-----GetKeyFromLine----------------------------------------------------------
 * Extracts a key name from a given line of text.  Returns NULL if the given
 * line is NULL or if there is no = on a non-empty the line.
 *---------------------------------------------------------------------------*/
char *CIniFile::GetKeyFromLine(char *_szLine, char *_szBuffer)
{
  if (_szLine == NULL) return NULL;

  // Skip the line if it is blank or a comment
  if (_szLine[0] == '\n' || _szLine[0] == '#')
  {
    _szBuffer[0] = '\0';
  }
  else
  {
    int i = 0;
    int j = 0;
    while (_szLine[i] != '=' && _szLine[i] != '\0')
      _szBuffer[j++] = _szLine[i++];
    _szBuffer[j] = '\0';
    Trim(_szBuffer);

    // If we didn't find an '=' but we found non-space characters, bad format
    if (_szLine[i] == '\0' && _szBuffer[0] != '\0')
    {
       Warn(INI_ExFORMAT, _szLine);
       return NULL;
    }
  }
  return (_szBuffer);
}


/*-----GetDataFromLine---------------------------------------------------------
 * Extracts the data from a given line, ie the characters after the '='.
 * Returns NULL if the given line is NULL or there is no '=' on the line.
 *---------------------------------------------------------------------------*/
char *CIniFile::GetDataFromLine(char *_szLine, char *_szBuffer, bool bTrim)
{
  //static char s_szData[MAX_LINE_LEN];
  char *szPostEquals;
  char szData[MAX_LINE_LEN];

  // Skip the line if it is blank or a comment
  if (_szLine[0] == '\n'|| _szLine[0] == '#')
  {
    _szBuffer[0] = '\0';
  }
  else
  {
    /* Check for a NULL string, and if not, then get the position of the
     * first '=' */
    if (_szLine == NULL || (szPostEquals = strchr(_szLine, '=')) == NULL)
    {
       Warn(INI_ExFORMAT, _szLine);
       return NULL;
    }

    strcpy(szData, szPostEquals + 1);
    if (bTrim) Trim(szData);
    AddNewLines(_szBuffer, szData);
  }
  return (_szBuffer);
}


/*-----SetSection--------------------------------------------------------------
 * Sets the section pointers to the relevant positions in the buffer.
 * Returns false if the section name is NULL or the section cannot be found
 * and ALLOWxCREATE is not set.
 *---------------------------------------------------------------------------*/
bool CIniFile::SetSection(const char *_szSection)
{
  if (_szSection == NULL) return (false);

  ResetFile();
  if (m_szSectionName != NULL)
  {
    free (m_szSectionName);
    m_szSectionName = NULL;
  }

  // Loop until we hit the end of the file or the start of the section
  char *sz, szLineBuffer[MAX_LINE_LEN], szSectionBuffer[MAX_SECTIONxNAME_LEN];
  do
  {
    sz = GetSectionFromLine(ReadLine(szLineBuffer), szSectionBuffer);
    if (sz == NULL)
    {
      if (GetFlag(INI_FxALLOWxCREATE))
      {
        CreateSection(_szSection);
        return (SetSection(_szSection));
      }
      else
      {
        Warn(INI_ExNOSECTION, _szSection);
        return(false);
      }
    }
  }
  while (strcmp(sz, _szSection) != 0);

  // If we get here it means we found the section

  // This positions the section start at the beginning of the first line
  // in the section
  m_nSectionStart = m_nBufPos;

  // Loop through until we find the beginning of a new section or the
  // end of file
  int nTempPos;
  do
  {
    nTempPos = m_nBufPos;
    sz = ReadLine(szLineBuffer);
  }
  while(sz != NULL && sz[0] != '[');

  // Backtrack until we find a non-space character, then move forward to the
  // next time, thus putting the end of section where it belongs
  while (isspace(m_szBuffer[--nTempPos]));
  while (m_szBuffer[++nTempPos] != '\n');

  m_nSectionEnd = nTempPos + 1;
  m_szSectionName = strdup(_szSection);
  ResetSection();
  return (true);
}


/*-----ReadStr-----------------------------------------------------------------
 * Finds a key and sets the data.  Returns false if the key does not exist.
 *---------------------------------------------------------------------------*/
bool CIniFile::ReadStr(const char *szKey, char *szData,
   const char *szDefault, bool bTrim)
{
  char *sz, *szLine, szLineBuffer[MAX_LINE_LEN], szKeyBuffer[MAX_KEYxNAME_LEN];

  ResetSection();

  do
  {
    szLine = ReadLine(szLineBuffer);
    sz = GetKeyFromLine(szLine, szKeyBuffer);
    if (sz == NULL)
    {
       if (szLine == NULL) Warn(INI_ExNOKEY, szKey);
       if (szDefault != NULL) strcpy(szData, szDefault);
       return (false);
    }
  }
  while (strcmp(sz, szKey) != 0);

  if ((sz = GetDataFromLine(szLine, szData, bTrim)) == NULL)
  {
    if (szDefault != NULL) strcpy(szData, szDefault);
    return (false);
  }
  return (true);
}


/*-----ReadNum-----------------------------------------------------------------
 * Finds a key and sets the numeric data.  Returns false if the key does not
 * exist.
 *---------------------------------------------------------------------------*/
bool CIniFile::ReadNum(const char *_szKey, unsigned long &data, const unsigned long _nDefault)
{
  char szData[MAX_LINE_LEN];
  if (!ReadStr(_szKey, szData, NULL))
  {
    data = _nDefault;
    return (false);
  }

  data = (unsigned long)atoi(szData);
  return(true);
}

bool CIniFile::ReadNum(const char *_szKey, unsigned short &data, const unsigned short _nDefault)
{
  char szData[MAX_LINE_LEN];
  if (!ReadStr(_szKey, szData, NULL))
  {
    data = _nDefault;
    return (false);
  }

  data = (unsigned short)atoi(szData);
  return(true);
}


bool CIniFile::ReadNum(const char *_szKey, char &data, const char _nDefault)
{
  char szData[MAX_LINE_LEN];
  if (!ReadStr(_szKey, szData, NULL))
  {
    data = _nDefault;
    return (false);
  }

  data = (char)atoi(szData);
  return(true);
}

bool CIniFile::ReadNum(const char *_szKey, signed short &data, const signed short _nDefault)
{
  char szData[MAX_LINE_LEN];
  if (!ReadStr(_szKey, szData, NULL))
  {
    data = _nDefault;
    return (false);
  }

  data = (signed short)atoi(szData);
  return(true);
}


bool CIniFile::ReadBool(const char *_szKey, bool &data, const bool _bDefault)
{
  char szData[MAX_LINE_LEN];
  if (!ReadStr(_szKey, szData, NULL))
  {
    data = _bDefault;
    return (false);
  }

  data = (atoi(szData) == 0 ? false : true);
  return(true);
}




/*-----InsertStr---------------------------------------------------------------
 * Inserts a string into the buffer, creating a new buffer in the process.
 * Used by WriteStr and CreateSection.
 *---------------------------------------------------------------------------*/
void CIniFile::InsertStr(const char *_szNewStr, int _nCutStart, int _nCutEnd)
{
  int nNewStrLen = strlen(_szNewStr);
  int nNewBufSize = _nCutStart + nNewStrLen + m_nBufSize - _nCutEnd;

  char *szNewBuffer = (char *)malloc(nNewBufSize + 1);
  memcpy(szNewBuffer, m_szBuffer, _nCutStart);
  memcpy(szNewBuffer + _nCutStart, _szNewStr, nNewStrLen);
  memcpy(szNewBuffer + _nCutStart + nNewStrLen, m_szBuffer + _nCutEnd, m_nBufSize - _nCutEnd + 1);

  free (m_szBuffer);
  m_szBuffer = szNewBuffer;
  m_nBufSize = nNewBufSize;
  m_bChanged = true;
}


/*-----CreateSection-----------------------------------------------------------
 * Creates a new section in the buffer.  Checks first to see if the section
 * exists.  Returns true if a new section was created.
 *---------------------------------------------------------------------------*/
bool CIniFile::CreateSection(const char *_szSectionName)
{
  // Try setting the section, if we succeed the the section exists already
  // First remove the warning flag so that we don't spit out useless warnings
  unsigned short nFlags = m_nFlags;
  ClearFlag(INI_FxWARN | INI_FxALLOWxCREATE);
  if (SetSection(_szSectionName))
  {
     SetFlags(nFlags);
     return(false);
  }
  SetFlags(nFlags);

  // Otherwise our bufpos is at the end of the file and we need to create
  // the section
  char szNewSect[MAX_SECTIONxNAME_LEN + 4];
  snprintf(szNewSect, MAX_SECTIONxNAME_LEN + 4, "[%s]\n", _szSectionName);
  m_nSectionStart = m_nBufSize + strlen(szNewSect);
  InsertStr(szNewSect, m_nBufPos, m_nBufPos);
  m_nSectionEnd = m_nBufSize;
  m_szSectionName = strdup(_szSectionName);

  return(true);
}


/*-----WriteStr----------------------------------------------------------------
 * Writes a string to the buffer replacing the given key or creating a new
 * key at the end of the current section.  Always returns true.
 *---------------------------------------------------------------------------*/
bool CIniFile::WriteStr(const char *_szKey, const char *_szData)
{
  char *sz, *szLine, szLineBuffer[MAX_LINE_LEN], szKeyBuffer[MAX_KEYxNAME_LEN];
  int nCutStart;

  ResetSection();

  // Find the position of the key or the end of the section / file
  do
  {
    nCutStart = m_nBufPos;
    szLine = ReadLine(szLineBuffer);
    sz = GetKeyFromLine(szLine, szKeyBuffer);
  }
  while (sz != NULL && strcmp(sz, _szKey) != 0);
  int nCutEnd = m_nBufPos;

  char szNewLine[MAX_LINE_LEN], szDataNoNL[MAX_LINE_LEN];
  if (_szData != NULL)
    RemoveNewLines(szDataNoNL, _szData);
  else
  {
    gLog.Warn("%sInternal Error: CIniFile::WriteStr(%s, NULL).\n", L_WARNxSTR, _szKey);
    strcpy(szDataNoNL, "");
  }
  snprintf(szNewLine, MAX_LINE_LEN, "%s = %s\n", _szKey, szDataNoNL);

  // Check if we are appending a new key to the section
  if (sz == NULL) m_nSectionEnd += strlen(szNewLine);

  InsertStr(szNewLine, nCutStart, nCutEnd);

  return (true);
}


bool CIniFile::WriteNum(const char *_szKey, const unsigned short _nData)
{
   char szN[32];
   sprintf(szN, "%u", _nData);
   return(WriteStr(_szKey, szN));
}

bool CIniFile::WriteNum(const char *_szKey, const unsigned long _nData)
{
   char szN[32];
   sprintf(szN, "%lu", _nData);
   return(WriteStr(_szKey, szN));
}

bool CIniFile::WriteNum(const char *_szKey, const signed short _nData)
{
  char szN[32];
  sprintf(szN, "%d", _nData);
  return(WriteStr(_szKey, szN));
}

bool CIniFile::WriteNum(const char *_szKey, const char _nData)
{
  char szN[32];
  sprintf(szN, "%d", _nData);
  return(WriteStr(_szKey, szN));
}

bool CIniFile::WriteBool(const char *_szKey, const bool _nData)
{
   return(WriteStr(_szKey, _nData ? "1" : "0"));
}

