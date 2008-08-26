#ifndef LOG_H
#define LOG_H

#include <cstdarg>
#include <cstdio>
#include <vector>
#include <list>
#include <pthread.h>

#include "licq_constants.h"

// Info: basic information about what's going on
const unsigned short L_INFO     = 0x0001;
// Unknown: unknown packets or bytes
const unsigned short L_UNKNOWN  = 0x0002;
// Error: critical errors which should be brought to the attention of the user
const unsigned short L_ERROR    = 0x0004;
// Warn: warnings which are not critical but could be important
const unsigned short L_WARN     = 0x0008;
// Packet: packet dumps
const unsigned short L_PACKET   = 0x0010;
// Message: popup messages from plugins to be displayed to the user
const unsigned short L_MESSAGE  = 0x8000;

const unsigned short L_MOST     = L_INFO | L_UNKNOWN | L_ERROR | L_WARN | L_MESSAGE;
const unsigned short L_ALL      = L_MOST | L_PACKET;
const unsigned short L_NONE     = 0;

const char L_WARNxSTR[]    = "[WRN] ";
const char L_TCPxSTR[]     = "[TCP] ";
const char L_SRVxSTR[]     = "[SRV] ";
const char L_ERRORxSTR[]   = "[ERR] ";
const char L_UNKNOWNxSTR[] = "[???] ";
const char L_BLANKxSTR[]   = "                ";
const char L_SBLANKxSTR[]  = "      ";
const char L_PACKETxSTR[]  = "[PKT] ";
const char L_INITxSTR[]    = "[INI] ";
const char L_ENDxSTR[]     = "[END] ";
const char L_FIFOxSTR[]    = "[FIF] ";
const char L_SSLxSTR[]     = "[SSL] ";

const unsigned short S_STDERR   = 1;
const unsigned short S_FILE     = 2;
const unsigned short S_PLUGIN   = 4;
const unsigned short S_ALL      = S_STDERR | S_FILE | S_PLUGIN;

const unsigned short LOG_PREFIX_OFFSET = 10;

const unsigned short MAX_MSG_SIZE = 2048;


//-----CLogService---------------------------------------------------------------
class CLogService
{
public:
  CLogService(unsigned short _nLogTypes);
  virtual ~CLogService();

  virtual void LogMessage(const char *_szPrefix, const char *_szMessage, 
                          unsigned short _nLogType) = 0;
  void SetLogTypes(unsigned short _nLogTypes);
  unsigned short ServiceType();
  unsigned short LogType(unsigned short _nLogType);
  void AddLogType(unsigned short _nLogType);
  void RemoveLogType(unsigned short _nLogType);
  void SetData(void *);

  static int LoggingPackets()  {  return s_nLoggingPackets; }

protected:
  unsigned short m_nLogTypes;
  unsigned short m_nServiceType;
  void *m_pData;

  static int s_nLoggingPackets;
};


//-----StdErr-------------------------------------------------------------------
class CLogService_StdErr : public CLogService
{
public:
  CLogService_StdErr(unsigned short _nLogTypes, bool _bUseColor);
  virtual void LogMessage(const char *_szPrefix, const char *_szMessage, 
                          unsigned short _nLogType);
protected:
  bool m_bUseColor;
};


//-----File---------------------------------------------------------------------
class CLogService_File : public CLogService
{
public:
   CLogService_File(unsigned short _nLogTypes);
   virtual ~CLogService_File();

   bool SetLogFile(const char *_szFile, const char *_szFlags);
   virtual void LogMessage(const char *_szPrefix, const char *_szMessage, 
                           unsigned short _nLogType);
protected:
   FILE *m_fLog;
};


//-----Plugin------------------------------------------------------------------
class CPluginLog
{
public:
  CPluginLog();
  ~CPluginLog();

  char *NextLogMsg();
  unsigned short NextLogType();
  void ClearLog();
  void AddLog(char *_szLog, unsigned short _nLogType);
  int Pipe()  { return pipe_log[PIPE_READ]; }
protected:
  pthread_mutex_t mutex;
  std::list <char *> m_vszLogs;
  std::list <unsigned short> m_vnLogTypes;
  int pipe_log[2];
};

class CLogService_Plugin : public CLogService
{
public:
  CLogService_Plugin(CPluginLog *_xWindow, unsigned short _nLogTypes);
  bool SetLogWindow(CPluginLog *_xWindow);
  CPluginLog *LogWindow() { return m_xLogWindow; }
  virtual void LogMessage(const char *_szPrefix, const char *_szMessage, 
      const unsigned short _nLogType);
protected:
  CPluginLog *m_xLogWindow;
};


//-----CLogServer---------------------------------------------------------------
class CLogServer
{
public:
  CLogServer();
  ~CLogServer();

  void AddService(CLogService *_xService);
  void AddLogTypeToService(unsigned short _nServiceType, unsigned short _nLogType);
  void RemoveLogTypeFromService(unsigned short _nServiceType, unsigned short _nLogType);
  void ModifyService(unsigned short _nServiceType, unsigned short _nLogTypes);
  unsigned short ServiceLogTypes(unsigned short _nServiceType);
  void SetServiceData(unsigned short _nServiceType, void *_pData);

  bool LoggingPackets()  {  return CLogService::LoggingPackets() > 0;  }

  void Info(const char *, ...)
#ifdef __GNUC__
    __attribute__ ((format (printf, 2, 3)))
#endif
;
  void Info(unsigned short _nServiceTypes, const char *_szFormat, ...)
#ifdef __GNUC__
    __attribute__ ((format (printf, 3, 4)))
#endif
;
  void Unknown(const char *, ...)
#ifdef __GNUC__
    __attribute__ ((format (printf, 2, 3)))
#endif
;
  void Unknown(unsigned short _nServiceTypes, const char *_szFormat, ...)
#ifdef __GNUC__
    __attribute__ ((format (printf, 3, 4)))
#endif
;
  void Error(const char *_szFormat, ...)
#ifdef __GNUC__
    __attribute__ ((format (printf, 2, 3)))
#endif
;
  void Error(unsigned short _nServiceTypes, const char *_szFormat, ...)
#ifdef __GNUC__
    __attribute__ ((format (printf, 3, 4)))
#endif
;
  void Warn(const char *_szFormat, ...)
#ifdef __GNUC__
    __attribute__ ((format (printf, 2, 3)))
#endif
;
  void Warn(unsigned short _nServiceTypes, const char *_szFormat, ...)
#ifdef __GNUC__
    __attribute__ ((format (printf, 3, 4)))
#endif
;
  void Packet(const char *_szFormat, ...)
#ifdef __GNUC__
    __attribute__ ((format (printf, 2, 3)))
#endif
;
  void Packet(unsigned short _nServiceTypes, const char *_szFormat, ...)
#ifdef __GNUC__
    __attribute__ ((format (printf, 3, 4)))
#endif
;
  void Message(const char *_szFormat, ...)
#ifdef __GNUC__
    __attribute__ ((format (printf, 2, 3)))
#endif
;
  void Message(unsigned short _nServiceTypes, const char *_szFormat, ...)
#ifdef __GNUC__
    __attribute__ ((format (printf, 3, 4)))
#endif
;

protected:
  std::vector <CLogService *> m_vxLogServices;
  pthread_mutex_t mutex;
  void Log(const unsigned short _nLogType, const char *_szFormat, va_list argp)
#ifdef __GNUC__
    __attribute__ ((format (printf, 3, 0)))
#endif
;
  void Log(const unsigned short _nServiceTypes, const unsigned short _nLogType, const char *_szFormat, va_list argp)
#ifdef __GNUC__
    __attribute__ ((format (printf, 4, 0)))
#endif
;
};


// Define an external log server to be started in log.cpp
extern CLogServer gLog;


#endif
