#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#else
extern int errno;
#endif

#include <time.h>

#include <unistd.h>
#include "log.h"

CLogServer gLog;

const char *COLOR_NORMAL = "[37m";
const char *COLOR_PREFIX = "[32m";
const char *COLOR_INFO = COLOR_NORMAL;
const char *COLOR_UNKNOWN = "[35m";
const char *COLOR_WARN = "[33m";
const char *COLOR_ERROR = "[31m";
const char *COLOR_PACKET = "[34m";
const char *COLOR_MSG[17] =
{
"",
COLOR_INFO,
COLOR_UNKNOWN, "",
COLOR_ERROR, "", "", "",
COLOR_WARN, "", "", "", "", "", "", "",
COLOR_PACKET
};

//-----CLogService---------------------------------------------------------------
CLogService::CLogService(unsigned short _nLogTypes)
{
   SetLogTypes(_nLogTypes);
   SetData(NULL);
}

inline
void CLogService::SetLogTypes(unsigned short _nLogTypes)
{
   m_nLogTypes = _nLogTypes;
}

inline
void CLogService::SetData(void *_pData)
{
  m_pData = _pData;
}

inline
unsigned short CLogService::ServiceType(void)
{
   return m_nServiceType;
}

inline
unsigned short CLogService::LogType(unsigned short _nLogType)
{
   return m_nLogTypes & _nLogType;
}

inline
void CLogService::AddLogType(unsigned short _nLogType)
{
   m_nLogTypes |= _nLogType;
}

inline
void CLogService::RemoveLogType(unsigned short _nLogType)
{
   m_nLogTypes &= ~_nLogType;
}



//-----StdOut-------------------------------------------------------------------
CLogService_StdOut::CLogService_StdOut(unsigned short _nLogTypes, bool _bUseColor)
   : CLogService(_nLogTypes)
{
   m_nServiceType = S_STDOUT;
   m_bUseColor = _bUseColor;
}



inline
void CLogService_StdOut::lprintf(unsigned short _nLogType, const char *_szPrefix,
                                 const char *_szFormat, va_list argp)
{
  if (m_bUseColor)
    printf("%s%s%s", COLOR_PREFIX, _szPrefix, COLOR_MSG[_nLogType]);
  else
    printf("%s", _szPrefix);
  vprintf(_szFormat, argp);
  if (m_bUseColor) printf("%s", COLOR_NORMAL);
  fflush(stdout);
}



//-----File---------------------------------------------------------------------
CLogService_File::CLogService_File(unsigned short _nLogTypes)
   : CLogService(_nLogTypes)
{
   m_fLog = NULL;
   m_nServiceType = S_FILE;
}

bool CLogService_File::SetLogFile(const char *_szFile, const char *_szFlags)
{
   if (m_fLog != NULL) fclose (m_fLog);
   m_fLog = fopen(_szFile, _szFlags);
   return (m_fLog != NULL);
}

inline
void CLogService_File::lprintf(unsigned short _nLogType, const char *_szPrefix,
                               const char *_szFormat, va_list argp)
{
  if (m_fLog == NULL) return;
  fprintf(m_fLog, "%s", _szPrefix);
  vfprintf(m_fLog, _szFormat, argp);
  fflush(m_fLog);
}


//-----Window-------------------------------------------------------------------
CPluginLog::CPluginLog(void)
{
  pipe(pipe_log);
  pthread_mutex_init(&mutex, NULL);
}

char *CPluginLog::NextLogMsg(void)
{
  pthread_mutex_lock(&mutex);
  char *sz = (m_vszLogs.size() > 0 ? m_vszLogs.front() : NULL);
  pthread_mutex_unlock(&mutex);
  return sz;
}

unsigned short CPluginLog::NextLogType(void)
{
  pthread_mutex_lock(&mutex);
  unsigned short n = (m_vnLogTypes.size() > 0 ? m_vnLogTypes.front() : 0);
  pthread_mutex_unlock(&mutex);
  return n;
}

void CPluginLog::ClearLog(void)
{
  pthread_mutex_lock(&mutex);
  char *sz = m_vszLogs.front();
  m_vszLogs.pop_front();
  m_vnLogTypes.pop_front();
  pthread_mutex_unlock(&mutex);
  free (sz);
}

void CPluginLog::AddLog(char *_szLog, unsigned short _nLogType)
{
  pthread_mutex_lock(&mutex);
  m_vszLogs.push_back(_szLog);
  m_vnLogTypes.push_back(_nLogType);
  pthread_mutex_unlock(&mutex);
  write(pipe_log[PIPE_WRITE], "*", 1);
}

CLogService_Plugin::CLogService_Plugin(CPluginLog *_xWindow, unsigned short _nLogTypes)
   : CLogService(_nLogTypes)
{
   m_xLogWindow = _xWindow;
   m_nServiceType = S_PLUGIN;
}

bool CLogService_Plugin::SetLogWindow(CPluginLog *_xWindow)
{
   m_xLogWindow = _xWindow;
   return true;
}

inline
void CLogService_Plugin::lprintf(unsigned short _nLogType, const char *_szPrefix,
                                 const char *_szFormat, va_list argp)
{
  static char szMsgMax[MAX_MSG_SIZE];

  if (m_xLogWindow == NULL) return;

  unsigned n = sprintf(szMsgMax, "%s", _szPrefix);
  vsprintf(&szMsgMax[n], _szFormat, argp);
  m_xLogWindow->AddLog(strdup(szMsgMax), _nLogType);
}


//-----CLogServer---------------------------------------------------------------
CLogServer::CLogServer(void)
{
  pthread_mutex_init(&mutex, NULL);
}

void CLogServer::AddService(CLogService *_xService)
{
  pthread_mutex_lock(&mutex);
  m_vxLogServices.push_back(_xService);
  pthread_mutex_unlock(&mutex);
}


void CLogServer::AddLogTypeToService(unsigned short _nServiceType, unsigned short _nLogType)
{
  pthread_mutex_lock(&mutex);
  // Go through the vector setting the log types
   vector<CLogService *>::iterator iter;
   for (iter = m_vxLogServices.begin(); iter != m_vxLogServices.end(); iter++)
   {
      if ((*iter)->ServiceType() == _nServiceType)
         (*iter)->AddLogType(_nLogType);
   }
  pthread_mutex_unlock(&mutex);
}

void CLogServer::RemoveLogTypeFromService(unsigned short _nServiceType, unsigned short _nLogType)
{
  pthread_mutex_lock(&mutex);
   // Go through the vector setting the log types
   vector<CLogService *>::iterator iter;
   for (iter = m_vxLogServices.begin(); iter != m_vxLogServices.end(); iter++)
   {
      if ((*iter)->ServiceType() == _nServiceType)
         (*iter)->RemoveLogType(_nLogType);
   }
  pthread_mutex_unlock(&mutex);
}

void CLogServer::ModifyService(unsigned short _nServiceType, unsigned short _nLogTypes)
{
  pthread_mutex_lock(&mutex);
   // Go through the vector setting the log types
   vector<CLogService *>::iterator iter;
   for (iter = m_vxLogServices.begin(); iter != m_vxLogServices.end(); iter++)
   {
      if ((*iter)->ServiceType() == _nServiceType)
         (*iter)->SetLogTypes(_nLogTypes);
   }
  pthread_mutex_unlock(&mutex);
}

unsigned short CLogServer::ServiceLogTypes(unsigned short _nServiceType)
{
  unsigned short n = L_NONE;
  pthread_mutex_lock(&mutex);
  // Go through the vector setting the log types
  vector<CLogService *>::iterator iter;
  for (iter = m_vxLogServices.begin(); iter != m_vxLogServices.end(); iter++)
  {
    if ((*iter)->ServiceType() == _nServiceType)
    {
      n = (*iter)->LogType(L_ALL);
      break;
    }
  }
  pthread_mutex_unlock(&mutex);
  return n;
}

void CLogServer::SetServiceData(unsigned short _nServiceType, void *_pData)
{
  pthread_mutex_lock(&mutex);
  vector<CLogService *>::iterator iter;
  for (iter = m_vxLogServices.begin(); iter != m_vxLogServices.end(); iter++)
  {
     if ((*iter)->ServiceType() == _nServiceType)
       (*iter)->SetData(_pData);
  }
  pthread_mutex_unlock(&mutex);
}

void CLogServer::Info(const char *_szFormat, ...)
{
  va_list argp;
  va_start(argp, _szFormat);
  Log(L_INFO, _szFormat, argp);
  va_end(argp);
}
void CLogServer::Info(unsigned short _nServiceTypes, const char *_szFormat, ...)
{
  va_list argp;
  va_start(argp, _szFormat);
  Log(_nServiceTypes, L_INFO, _szFormat, argp);
  va_end(argp);
}


void CLogServer::Unknown(const char *_szFormat, ...)
{
   va_list argp;
   va_start(argp, _szFormat);
   Log(L_UNKNOWN, _szFormat, argp);
   va_end(argp);
}
void CLogServer::Unknown(unsigned short _nServiceTypes, const char *_szFormat, ...)
{
   va_list argp;
   va_start(argp, _szFormat);
   Log(_nServiceTypes, L_UNKNOWN, _szFormat, argp);
   va_end(argp);
}

void CLogServer::Error(const char *_szFormat, ...)
{
   va_list argp;
   va_start(argp, _szFormat);
   Log(L_ERROR, _szFormat, argp);
   va_end(argp);
}
void CLogServer::Error(unsigned short _nServiceTypes, const char *_szFormat, ...)
{
   va_list argp;
   va_start(argp, _szFormat);
   Log(_nServiceTypes, L_ERROR, _szFormat, argp);
   va_end(argp);
}

void CLogServer::Warn(const char *_szFormat, ...)
{
   va_list argp;
   va_start(argp, _szFormat);
   Log(L_WARN, _szFormat, argp);
   va_end(argp);
}
void CLogServer::Warn(unsigned short _nServiceTypes, const char *_szFormat, ...)
{
   va_list argp;
   va_start(argp, _szFormat);
   Log(_nServiceTypes, L_WARN, _szFormat, argp);
   va_end(argp);
}


void CLogServer::Packet(const char *_szFormat, ...)
{
   va_list argp;
   va_start(argp, _szFormat);
   Log(L_PACKET, _szFormat, argp);
   va_end(argp);
}
void CLogServer::Packet(unsigned short _nServiceTypes, const char *_szFormat, ...)
{
   va_list argp;
   va_start(argp, _szFormat);
   Log(_nServiceTypes, L_PACKET, _szFormat, argp);
   va_end(argp);
}


void CLogServer::Log(const unsigned short _nLogType, const char *_szFormat, va_list argp)
{
  static char szTime[32];
  static struct tm stm;

  pthread_mutex_lock(&mutex);

  // Create a time string for the log
  time_t t = time(NULL);
  localtime_r(&t, &stm);
  strftime(szTime, 32, "%T: ", &stm);

  // Log the event to each server
  vector<CLogService *>::iterator iter;
  for (iter = m_vxLogServices.begin(); iter != m_vxLogServices.end(); iter++)
  {
    if ((*iter)->LogType(_nLogType))
        (*iter)->lprintf(_nLogType, szTime, _szFormat, argp);
  }
  pthread_mutex_unlock(&mutex);
}

void CLogServer::Log(const unsigned short _nServiceTypes, const unsigned short _nLogType, const char *_szFormat, va_list argp)
{
  static char szTime[32];
  static struct tm stm;

  pthread_mutex_lock(&mutex);

  // Create a time string for the log
  time_t t = time(NULL);
  localtime_r(&t, &stm);
  strftime(szTime, 32, "%T: ", &stm);

  // Log the event to each server
  vector<CLogService *>::iterator iter;
  for (iter = m_vxLogServices.begin(); iter != m_vxLogServices.end(); iter++)
  {
    if ((*iter)->LogType(_nLogType) || (*iter)->ServiceType() & _nServiceTypes)
        (*iter)->lprintf(_nLogType, szTime, _szFormat, argp);
  }
  pthread_mutex_unlock(&mutex);
}

