// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998 - 2003 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

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

// Localization
#include "gettext.h"

#include "licq_file.h"
#include "licq_log.h"

using namespace std;

//=====Pre class helper functions==============================================

//---Trim----------------------------------------------------------------------
/*! \brief Removes leading and trailing spaces from a string 
 */
void Trim(char *_sz)
{
  if (_sz == NULL)
  {
    gLog.Warn("%sInternal Error: Trim(): _sz == NULL.\n", L_WARNxSTR);
    return;
  }
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


//---AddNewLines---------------------------------------------------------------
/*! \brief Replaces all occurences of "\n" in a string by '\n'
 */
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
    else if (_szSource[i] == '\\' && _szSource[i + 1] == '\\')
    {
      _szDest[j++] = '\\';
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


//---RemoveNewLines------------------------------------------------------------
/*! \brief Replaces all occurences of '\n' in a string by "\n"
 */
void RemoveNewLines(char *_szDest, int _nDestSize, const char *_szSource)
{
  if (_szSource == NULL || _szDest == NULL)
  {
    gLog.Warn("%sInternal Error: RemoveNewLines(): _szDest == NULL or "
              "_szSource == NULL.\n", L_WARNxSTR);
    return;
  }
  int i = 0, j = 0;
  while (_szSource[i] != '\0')
  {
    if (j >= _nDestSize) break;
    if (_szSource[i] == '\n')
    {
      _szDest[j++] = '\\';
      if (j >= _nDestSize) break;
      _szDest[j++] = 'n';
    }
    else if (_szSource[i] == '\\')
    {
      _szDest[j++] = '\\';
      if (j >= _nDestSize) break;
      _szDest[j++] = '\\';
    }
    else
      _szDest[j++] = _szSource[i];
    i++;
  }
  if (j < _nDestSize)
    _szDest[j] = '\0';
  else
  {
    if (_nDestSize > 0) _szDest[_nDestSize - 1] = '\0';
    gLog.Warn("%sInternal Error: RemoveNewLines(): Destination buffer too "
              "small.\n", L_WARNxSTR);
    return;
  }
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
/*! \brief Free's the buffer which contains the file content.
 *
 * Free's all memory that is occupied by the current buffer.
 * No file descriptors are closed herein because the file is closed 
 * immediately after beeing loaded at CIniFile::LoadFile.
 */
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
/*! \brief Loads file _szFilename into a buffer and closes it afterwards
 *
 * Loads the specified file. If it does not already exist and INI_FxALLOWxCREATE 
 * is set it will be created. If file does not exist and it cannot be created 
 * a warning is generated and "false" is beeing returned.
 * After the file has been loaded into memory it is closed immediately.
 */
bool CIniFile::LoadFile(const char *_szFilename)
{
  CloseFile();
  if (m_szFilename && (m_szFilename != _szFilename))
  {
    free(m_szFilename);
    m_szFilename = NULL;
  }

  if (m_szFilename != _szFilename)
    m_szFilename = strdup(_szFilename);

  int nFD;
  if (GetFlag(INI_FxALLOWxCREATE))
    nFD = open(m_szFilename, O_RDONLY | O_CREAT, 00600);
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
    m_szBuffer = NULL;
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
/*! \brief Reloads currently loaded file.
 */
bool CIniFile::ReloadFile()
{
  return(LoadFile(m_szFilename));
}


//-----FlushFile---------------------------------------------------------------
/* FlushFile is susceptible to a symlink attack. If [m_szFilename].new
 * already exists, it will be truncated and removed. */

/*! \brief Writes the buffer to disk.
 *
 * Writes the current buffer contents to disk. Everything is written to
 * <filename>.new first and on success the resulting file is renamed to
 * <filename>. The permissions of the original file are beeing preserved.
 * Returns true on success, otherwise returns false.
 */
bool CIniFile::FlushFile()
{
  // Write files atomically to avoid config trashing.
  char tempname[MAX_FILENAME_LEN];

  if (strlen(m_szFilename) + 4 >= MAX_FILENAME_LEN)
  {
    Warn(INI_ExIOWRITE);
    return false;
  }
  strcpy(tempname, m_szFilename);
  strcat(tempname, ".new");

  struct stat fStats;
  stat(m_szFilename, &fStats); // create new file with same mode as original file
  int nFD = open(tempname, O_WRONLY | O_CREAT | O_TRUNC, fStats.st_mode);
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
    if (close(nFD))
    {
        // close failed, data may have not been written
        unlink(tempname);
        return false;
    }

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


//-----Warn--------------------------------------------------------------------
/*! \brief Print out an error message.
 *
 * Print out an error message using the standard log. Can output warnings
 * and/or errors. If the INI_FxFATAL flag is set, the program will terminate.
 */
void CIniFile::Warn(int nError, const char *_sz)
{
  switch(nError)
  {
  case INI_ExNOKEY:
    if (GetFlag(INI_FxWARN))
      gLog.Warn(tr("%sIniFile: Warning, failed to find key.\n%sFile    = "
                   "%s\n%sSection = [%s]\n%sKey     = \"%s\"\n"),
                   L_WARNxSTR, L_BLANKxSTR, m_szFilename, L_BLANKxSTR,
                   m_szSectionName, L_BLANKxSTR, _sz);
    if(GetFlag(INI_FxERROR))
      gLog.Error(tr("%sIniFile: Warning, failed to find key.\n%sFile    = "
                    "%s\n%sSection = [%s]\n%sKey     = \"%s\"\n"),
                    L_ERRORxSTR, L_BLANKxSTR, m_szFilename, L_BLANKxSTR,
                    m_szSectionName, L_BLANKxSTR, _sz);
    break;

  case INI_ExNOSECTION:
    if (GetFlag(INI_FxWARN))
      gLog.Warn(tr("%sIniFile: Warning, failed to find section.\n%sFile    = "
                   "%s\n%sSection = [%s]\n"),
                   L_WARNxSTR, L_BLANKxSTR, m_szFilename, L_BLANKxSTR, _sz);
    if(GetFlag(INI_FxERROR))
      gLog.Error("%sIniFile: Warning, failed to find section.\n%sFile    = %s\n%sSection = [%s]\n",
                L_ERRORxSTR, L_BLANKxSTR, m_szFilename, L_BLANKxSTR, _sz);
    break;

  case INI_ExFORMAT:
    if (GetFlag(INI_FxWARN))
      gLog.Warn(tr("%sIniFile: Warning, invalid file format.\n%sFile = %s\n%sLine = %s\n"),
                L_WARNxSTR, L_BLANKxSTR, m_szFilename, L_BLANKxSTR, _sz);
    if(GetFlag(INI_FxERROR))
      gLog.Error("%sIniFile: Warning, invalid file format.\n%sFile = %s\n%sLine = %s\n",
                L_ERRORxSTR, L_BLANKxSTR, m_szFilename, L_BLANKxSTR, _sz);
    break;

  case INI_ExIOREAD:
    if (GetFlag(INI_FxWARN))
      gLog.Warn(tr("%sIniFile: Warning, I/O read error.\n%sFile  = %s\n%sError = %s\n"),
                L_WARNxSTR, L_BLANKxSTR, m_szFilename, L_BLANKxSTR,
                strerror(Error()));
    if(GetFlag(INI_FxERROR))
      gLog.Error("%sIniFile: Warning, I/O read error.\n%sFile  = %s\n%sError = %s\n",
                 L_ERRORxSTR, L_BLANKxSTR, m_szFilename, L_BLANKxSTR,
                 strerror(Error()));
    break;

  case INI_ExIOWRITE:
    if (GetFlag(INI_FxWARN))
      gLog.Warn(tr("%sIniFile: Warning, I/O write error.\n%sFile  = %s\n%sError = %s\n"),
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


//-----ReadLine----------------------------------------------------------------
/*! \brief Reads in characters from the file.
 *
 * Reads in characters from the file starting with the current position and 
 * ending at the first new line.  Returns NULL if we are already at the EOF 
 * or EOS.  Will not return lines beginning with a # (comments)
 */
char *CIniFile::ReadLine(char *_szDest, int _nDestSize)
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
  while (m_szBuffer[m_nBufPos] != '\n' && i < _nDestSize - 1)
    _szDest[i++] = m_szBuffer[m_nBufPos++];
  // Increase the buffer pos to get past the newline
  m_nBufPos++;
  // Replace the newline with a null
  _szDest[i] = '\0';

  return(_szDest);
}


//-----GetSectionFromLine------------------------------------------------------
/*! \brief Extracts the section name from a given line of text.
 *
 * Extracts the section name from a given line of text.  Returns an empty 
 * string if the line does not contain a section, or NULL if the given line 
 * is NULL or there is an open ([) with no closing (]).
 */
char *CIniFile::GetSectionFromLine(char *_szBuffer, const char *_szLine)
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
    while (_szLine[i] != ']' && _szLine[i] != '\0' &&
           j < MAX_SECTIONxNAME_LEN)
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


//-----GetKeyFromLine----------------------------------------------------------
/*! \brief Extracts a key name from a given line of text.
 *
 * Extracts a key name from a given line of text.  Returns NULL if the given
 * line is NULL or if there is no = on a non-empty the line.
 */
char *CIniFile::GetKeyFromLine(char *_szBuffer, const char *_szLine)
{
  if (_szLine == NULL) return NULL;

  // Skip the line if it is blank or a comment
  if (_szLine[0] == '\n' || _szLine[0] == '#' || _szLine[0] == ';')
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


//-----GetDataFromLine---------------------------------------------------------
/*! \brief Extracts data from a given line.
 *
 * Extracts the data from a given line, ie the characters after the '='.
 * Returns NULL if the given line is NULL or there is no '=' on the line.
 */
char *CIniFile::GetDataFromLine(char *_szBuffer, const char *_szLine,
                                bool bTrim, int _nMax)
{
  //static char s_szData[MAX_LINE_LEN];
  char *szPostEquals;
  char szData[MAX_LINE_LEN];
  int nMax = (_nMax > 0 ? _nMax : MAX_LINE_LEN);

  if (_szLine == NULL)
  {
    Warn(INI_ExFORMAT, _szLine);
    return NULL;
  }

  // Skip the line if it is blank or a comment
  if (_szLine[0] == '\n'|| _szLine[0] == '#')
  {
    _szBuffer[0] = '\0';
  }
  else
  {
    if ((szPostEquals = strchr(_szLine, '=')) == NULL)
    {
       Warn(INI_ExFORMAT, _szLine);
       return NULL;
    }

    strncpy(szData, szPostEquals + 1, nMax);
    szData[nMax - 1] = '\0';
    if (bTrim) Trim(szData);
    AddNewLines(_szBuffer, szData);
  }
  return (_szBuffer);
}


//-----SetSection--------------------------------------------------------------
/*! \brief Sets the section pointer to the relevant position.
 *
 * Sets the section pointers to the relevant positions in the buffer.
 * Returns false if the section name is NULL or the section cannot be found
 * and ALLOWxCREATE is not set.
 */
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
    sz = GetSectionFromLine(szSectionBuffer,
                            ReadLine(szLineBuffer, MAX_LINE_LEN));
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
    sz = ReadLine(szLineBuffer, MAX_LINE_LEN);
  }
  while(sz != NULL && sz[0] != '[');

  // Backtrack until we find a non-space character, then move forward to the
  // next time, thus putting the end of section where it belongs
  while (isspace(m_szBuffer[--nTempPos]))
    ;
  while (m_szBuffer[++nTempPos] != '\n')
    ;

  m_nSectionEnd = nTempPos + 1;
  m_szSectionName = strdup(_szSection);
  ResetSection();
  return (true);
}

bool CIniFile::readString(const string& key, string& data,
    const string& defValue, bool trim)
{
  char buffer[MAX_LINE_LEN];
  bool ret = ReadStr(key, buffer, defValue.c_str(), trim, MAX_LINE_LEN);
  data = buffer;
  return ret;
}

//-----ReadStr-----------------------------------------------------------------
/*! \brief Finds a key and sets the data.  Returns false if the key does not exist.
 */
bool CIniFile::ReadStr(const string& key, char *szData,
                       const char *szDefault, bool bTrim, int _nMax)
{
  char *sz, *szLine, szLineBuffer[MAX_LINE_LEN], szKeyBuffer[MAX_KEYxNAME_LEN];

  ResetSection();

  do
  {
    szLine = ReadLine(szLineBuffer, MAX_LINE_LEN);
    sz = GetKeyFromLine(szKeyBuffer, szLine);
    if (sz == NULL)
    {
      if (szLine == NULL)
        Warn(INI_ExNOKEY, key.c_str());
       if (szDefault != NULL) strcpy(szData, szDefault);
       return (false);
    }
  }
  while (sz != key);

  if ((sz = GetDataFromLine(szData, szLine, bTrim, _nMax)) == NULL)
  {
    if (szDefault != NULL) strcpy(szData, szDefault);
    return (false);
  }
  return (true);
}

//-----ReadNum-----------------------------------------------------------------
/*! \brief Reads numeric data from a key.
 *
 * Finds a key and sets the numeric data.  Returns false if the key does not
 * exist.
 */

#define MAKE_READNUM(valueType, convertion) \
    bool CIniFile::ReadNum(const string& key, valueType &data, valueType defValue) \
    { \
      char strData[MAX_LINE_LEN]; \
      if (!ReadStr(key, strData, NULL)) \
      { \
        data = defValue; \
        return false; \
      } \
      data = convertion; \
      return true; \
    }

MAKE_READNUM(unsigned long, strtoul(strData, (char**)NULL, 10))
MAKE_READNUM(signed long, strtoul(strData, (char**)NULL, 10))
MAKE_READNUM(unsigned int, strtoul(strData, (char**)NULL, 10))
MAKE_READNUM(signed int, atoi(strData))
MAKE_READNUM(unsigned short, (unsigned short)atoi(strData))
MAKE_READNUM(signed short, (signed short)atoi(strData))
MAKE_READNUM(char, (char)atoi(strData))

//-----ReadBool----------------------------------------------------------------
/*! \brief Reads bool info from a key.
 *
 * Finds a key and sets true or false. "0" means false, all other values 
 * mean true.  Returns false if the key does not exist.
 */
bool CIniFile::ReadBool(const string& key, bool &data, const bool _bDefault)
{
  char szData[MAX_LINE_LEN];
  if (!ReadStr(key, szData, NULL))
  {
    data = _bDefault;
    return (false);
  }

  data = (atoi(szData) == 0 ? false : true);
  return(true);
}




//-----InsertStr---------------------------------------------------------------
/*! \brief Insert a string into the buffer
 *
 * Inserts a string into the buffer, creating a new buffer in the process. 
 * Used by WriteStr and CreateSection.
 */
void CIniFile::InsertStr(const char *_szNewStr, int _nCutStart, int _nCutEnd)
{
  int nNewStrLen = strlen(_szNewStr);
  int nNewBufSize = _nCutStart + nNewStrLen + m_nBufSize - _nCutEnd;
  
  char *szNewBuffer = (char *)malloc(nNewBufSize + 1);
  memcpy(szNewBuffer, m_szBuffer, _nCutStart);
  memcpy(szNewBuffer + _nCutStart, _szNewStr, nNewStrLen);
  memcpy(szNewBuffer + _nCutStart + nNewStrLen, m_szBuffer + _nCutEnd,
         m_nBufSize - _nCutEnd + 1);

  free(m_szBuffer);
  m_szBuffer = szNewBuffer;
  m_nBufSize = nNewBufSize;
  m_bChanged = true;
}


//-----CreateSection-----------------------------------------------------------
/*! \brief Creates a new section in the buffer.
 *
 * Creates a new section in the buffer.  Checks first to see if the section
 * exists.  Returns true if a new section was created.
 */
bool CIniFile::CreateSection(const char *_szSectionName)
{
  // Try setting the section, if we succeed the section exists already
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
  szNewSect[sizeof (szNewSect) - 1] = '\0';
  m_nSectionStart = m_nBufSize + strlen(szNewSect);
  InsertStr(szNewSect, m_nBufPos, m_nBufPos);
  m_nSectionEnd = m_nBufSize;
  m_szSectionName = strdup(_szSectionName);

  return(true);
}

void CIniFile::writeString(const std::string& key, const std::string& data)
{
  WriteStr(key, data.c_str());
}

//-----WriteStr----------------------------------------------------------------
/*! \brief Writes a string to the buffer.
 *
 * Writes a string to the buffer replacing the given key or creating a new 
 * key at the end of the current section.  Always returns true.
 */
bool CIniFile::WriteStr(const string& key, const char *_szData)
{
  char *sz, *szLine, szLineBuffer[MAX_LINE_LEN], szKeyBuffer[MAX_KEYxNAME_LEN];
  int nCutStart;

  ResetSection();

  // Find the position of the key or the end of the section / file
  do
  {
    nCutStart = m_nBufPos;
    szLine = ReadLine(szLineBuffer, MAX_LINE_LEN);
    sz = GetKeyFromLine(szKeyBuffer, szLine);
  }
  while (sz != NULL && sz != key);
  int nCutEnd = m_nBufPos;

  char szNewLine[MAX_LINE_LEN], szDataNoNL[MAX_LINE_LEN];
  if (_szData != NULL)
    RemoveNewLines(szDataNoNL, MAX_LINE_LEN, _szData);
  else
  {
    gLog.Warn("%sInternal Error: CIniFile::WriteStr(%s, NULL).\n",
        L_WARNxSTR, key.c_str());
    strcpy(szDataNoNL, "");
  }
  snprintf(szNewLine, MAX_LINE_LEN, "%s = %s\n", key.c_str(), szDataNoNL);
  szNewLine[MAX_LINE_LEN - 1] = '\0';
  // Don't lose the following key if szNewLine was truncated by snprintf
  szNewLine[MAX_LINE_LEN - 2] = '\n'; 

  // Adjust the section end as it will change if line is added or if new value
  // is not same length as old value
  m_nSectionEnd = m_nSectionEnd - (nCutEnd - nCutStart) + strlen(szNewLine);

  InsertStr(szNewLine, nCutStart, nCutEnd);

  return (true);
}

#define MAKE_WRITENUM(valueType, convertion) \
    bool CIniFile::WriteNum(const string& key, valueType data) \
    { \
      char strData[32]; \
      snprintf(strData, sizeof(strData), convertion, data); \
      return WriteStr(key, strData); \
    }

MAKE_WRITENUM(unsigned long, "%lu")
MAKE_WRITENUM(signed long, "%lu")
MAKE_WRITENUM(unsigned int, "%u")
MAKE_WRITENUM(signed int, "%i")
MAKE_WRITENUM(unsigned short, "%i")
MAKE_WRITENUM(signed short, "%i")
MAKE_WRITENUM(char, "%i")

bool CIniFile::WriteBool(const string& key, bool data)
{
   return(WriteStr(key, data ? "1" : "0"));
}

