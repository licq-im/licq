#ifndef INIFILE_H
#define INIFILE_H

#include "licq/inifile.h"

#include <cerrno>
#include <cstring>
#include <list>
#include <string>
#include <boost/any.hpp>

#include "licq_log.h"

#define MAX_SECTIONxNAME_LEN 160
#define MAX_KEYxNAME_LEN 160
#define MAX_LINE_LEN 4096

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
class CIniFile : public Licq::IniFile
{
public:
  CIniFile(unsigned short _nFlags = 0)
    : Licq::IniFile(""), m_nFlags(_nFlags)
  { /* Empty */ }

  ~CIniFile() {}

  bool LoadFile(const char *_szFilename)
  { setFilename(_szFilename); return ReloadFile(); }

  bool ReloadFile()
  {
    bool ret = loadFile();
    if (!ret && GetFlag(INI_FxALLOWxCREATE))
      return writeFile(true);
    return ret;
  }

  bool FlushFile()
  { return writeFile(GetFlag(INI_FxALLOWxCREATE)); }

  void CloseFile()
  { /* Empty */ }

  void SetFileName(const char *_szFilename)
  { setFilename(_szFilename); }

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

  bool SetSection(const char *_szSectionName)
  { return setSection(_szSectionName, GetFlag(INI_FxALLOWxCREATE)); }

  bool CreateSection(const char *_szSectionName)
  { return setSection(_szSectionName, true); }

  /**
   * Read a value from the configuration file
   *
   * @param key Key of value to read
   * @param data variable to put value in
   * @return True if value was found or false if default value was used
   */
  bool readVar(const std::string& key, boost::any& data)
  { return get(key, data); }

  /**
   * Read a string value from the configuration file
   *
   * @param key Key of value to read
   * @param data String to put value in
   * @param defValue Default value to set if key doesn't exist
   * @param trim True if leading and trailing white spaces should be trimmed
   * @return True if value was found or false if default value was used
   */
  bool readString(const std::string& key, std::string& data,
      const std::string& defValue = "", bool trim = true)
  {
    bool ret = get(key, data, defValue);
    if (ret && !trim)
      data = ' ' + data;
    return ret;
  }

  bool ReadStr(const std::string& Key, char* data, const char* defValue = NULL, bool trim = true, int maxLength = 0)
  {
    (void)trim; (void)maxLength;
    std::string retStr;
    bool ret = get(Key, retStr);
    if (ret) {
      if (trim) {
        strcpy(data, retStr.c_str());
      } else {
        data[0] = ' ';
        strcpy(data+1, retStr.c_str());
      }
    } else if (defValue != NULL) {
      strcpy(data, defValue);
    }
    return ret;
  }

  bool ReadNum(const std::string& key, unsigned long &data, unsigned long defValue = 0)
  {
    unsigned retInt;
    bool ret = get(key, retInt, defValue);
    data = retInt;
    return ret;
  }

  bool ReadNum(const std::string& key, signed long &data, signed long defValue = 0)
  {
    int retInt;
    bool ret = get(key, retInt, defValue);
    data = retInt;
    return ret;
  }

  bool ReadNum(const std::string& key, unsigned int &data, unsigned int defValue = 0)
  { return get(key, data, defValue); }

  bool ReadNum(const std::string& key, signed int &data, signed int defValue = 0)
  { return get(key, data, defValue); }

  bool ReadNum(const std::string& key, unsigned short &data, unsigned short defValue = 0)
  {
    unsigned retInt;
    bool ret = get(key, retInt, defValue);
    data = retInt;
    return ret;
  }

  bool ReadNum(const std::string& key, signed short &data, signed short defValue = 0)
  {
    int retInt;
    bool ret = get(key, retInt, defValue);
    data = retInt;
    return ret;
  }

  bool ReadNum(const std::string& key, char &data, char defValue = 0)
  {
    int retInt;
    bool ret = get(key, retInt, defValue);
    data = retInt;
    return ret;
  }

  bool ReadBool(const std::string& key, bool &data, const bool defValue = false)
  { return get(key, data, defValue); }

  /**
   * Write a value to the configuration file
   *
   * @param key Key of value to write
   * @param data Value to write
   */
  void writeVar(const std::string& key, const boost::any& data)
  { set(key, data); }

  /**
   * Write a string value to the configuration file
   *
   * @param key Key of value to write
   * @param data Value to write
   */
  void writeString(const std::string& key, const std::string& data)
  { set(key, data); }

  bool WriteStr(const std::string& key, const char* data)
  { return set(key, std::string(data)); }

  bool WriteNum(const std::string& key, unsigned long data)
  { return set(key, static_cast<unsigned>(data)); }
  bool WriteNum(const std::string& key, signed long data)
  { return set(key, static_cast<int>(data)); }
  bool WriteNum(const std::string& key, unsigned int data)
  { return set(key, data); }
  bool WriteNum(const std::string& key, signed int data)
  { return set(key, data); }
  bool WriteNum(const std::string& key, unsigned short data)
  { return set(key, data); }
  bool WriteNum(const std::string& key, signed short data)
  { return set(key, data); }
  bool WriteNum(const std::string& key, char data)
  { return set(key, data); }
  bool WriteBool(const std::string& key, bool data)
  { return set(key, data); }

  int Error()  { return errno; }
  const char *FileName()  { return filename().c_str(); }

protected:
  unsigned short m_nFlags;
};


#endif
