#ifndef INIFILE_H
#define INIFILE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define MAX_SECTIONxNAME_LEN 160
#define MAX_KEYxNAME_LEN 160
#define MAX_LINE_LEN 1024

const unsigned short INI_FxWARN           = 0x0001;
const unsigned short INI_FxERROR          = 0x0002;
const unsigned short INI_FxFATAL          = 0x0004;
const unsigned short INI_FxALLOWxCREATE   = 0x0008;

const unsigned short INI_ExNOKEY          = 0x0001;
const unsigned short INI_ExNOSECTION      = 0x0002;
const unsigned short INI_ExFORMAT         = 0x0004;
const unsigned short INI_ExIOREAD         = 0x0008;
const unsigned short INI_ExIOWRITE        = 0x0010;

void AddNewLines(char *_szDest, const char *_szSource);

//=====CIniFile================================================================
class CIniFile
{
public:
  CIniFile(unsigned short _nFlags = 0);
  ~CIniFile();

  bool LoadFile(const char *_szFilename);
  bool ReloadFile();
  bool FlushFile();
  void CloseFile();
  void SetFileName(const char *_szFilename);

  // Flag setting and clearing
  void SetFlags(unsigned short _nFlags)
    { m_nFlags = _nFlags; };
  void SetFlag(unsigned short _nFlag)
    { m_nFlags |= _nFlag; };
  void ClearFlag(unsigned short _nFlag)
    { m_nFlags &= ~_nFlag; };
  unsigned short GetFlags()
    { return (m_nFlags); };
  bool GetFlag(unsigned short _nFlag)
    { return (m_nFlags & _nFlag); };

  bool SetSection(const char *_szSectionName);
  bool CreateSection(const char *_szSectionName);
  bool ReadStr(const char *_szKey, char *_szData, const char *_szDefault = NULL, bool bTrim = true);
  bool ReadNum(const char *_szKey, unsigned long &data, const unsigned long _nDefault = 0);
  bool ReadNum(const char *_szKey, unsigned short &data, const unsigned short _nDefault = 0);
  bool ReadNum(const char *_szKey, signed short &data, const signed short _nDefault = 0);
  bool ReadNum(const char *_szKey, char &data, const char _nDefault = 0);
  bool ReadBool(const char *_szKey, bool &data, const bool _bDefault = false);

  bool WriteStr(const char *_szKey, const char *_szData);
  bool WriteNum(const char *_szKey, const unsigned long _szData);
  bool WriteNum(const char *_szKey, const unsigned short _szData);
  bool WriteNum(const char *_szKey, const signed short _szData);
  bool WriteNum(const char *_szKey, const char _szData);
  bool WriteBool(const char *_szKey, const bool _szData);

  int Error()  { return (m_nError); }
  const char *FileName()  { return m_szFilename; }

protected:
  // Data members
  char *m_szFilename,
       *m_szBuffer;
  int m_nBufPos,
      m_nBufSize,
      m_nError;
  char *m_szSectionName;
  int m_nSectionStart,
      m_nSectionEnd;
  unsigned short m_nFlags;
  bool //m_bEof,
       //m_bEos,
       m_bChanged;

  // Functions
  char *ReadLine(char *_szBuffer);
  char *GetSectionFromLine(char *_szLine, char *_szBuffer);
  char *GetKeyFromLine(char *_szLine, char *_szBuffer);
  char *GetDataFromLine(char *_szLine, char *_szBuffer, bool bTrim = true);
  void Warn(int nError, const char *_sz = NULL);
  void InsertStr(const char *_szNewStr, int _nCutStart, int _nCutEnd);

  /*bool Eof()
    { return (m_bEof); };
  bool Eos()
    { return (m_bEos); };*/
  void ResetFile();
  void ResetSection();
};


#endif
