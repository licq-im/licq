// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998 - 2009 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#else
extern int errno;
#endif
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <unistd.h>
#include "licq_log.h"

using namespace std;

CLogServer gLog;

const char *COLOR_NORMAL = "[39m";
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

int CLogService::s_nLoggingPackets = 0;

//-----CLogService---------------------------------------------------------------
CLogService::CLogService(unsigned short _nLogTypes)
{
  m_nLogTypes = 0;
  SetLogTypes(_nLogTypes);
  SetData(NULL);
}

CLogService::~CLogService()
{
  // Empty
}

void CLogService::SetLogTypes(unsigned short _nLogTypes)
{
  if ( (m_nLogTypes & L_PACKET) && !(_nLogTypes & L_PACKET) )
    s_nLoggingPackets--;
  else if ( !(m_nLogTypes & L_PACKET) && (_nLogTypes & L_PACKET) )
    s_nLoggingPackets++;

  m_nLogTypes = _nLogTypes;
}

void CLogService::SetData(void *_pData)
{
  m_pData = _pData;
}

unsigned short CLogService::ServiceType()
{
  return m_nServiceType;
}

unsigned short CLogService::LogType(unsigned short _nLogType)
{
  return m_nLogTypes & _nLogType;
}

void CLogService::AddLogType(unsigned short _nLogType)
{
  SetLogTypes(m_nLogTypes | _nLogType);
}

void CLogService::RemoveLogType(unsigned short _nLogType)
{
  SetLogTypes(m_nLogTypes & ~_nLogType);
}



//-----StdErr-------------------------------------------------------------------
CLogService_StdErr::CLogService_StdErr(unsigned short _nLogTypes, bool _bUseColor)
   : CLogService(_nLogTypes)
{
  m_nServiceType = S_STDERR;
  m_bUseColor = _bUseColor;
}


void CLogService_StdErr::LogMessage(const char *_szPrefix, 
                                    const char *_szMessage,
                                    const unsigned short _nLogType)
{
  if (m_bUseColor)
    fprintf(stderr, "%s%s%s", COLOR_PREFIX, _szPrefix, COLOR_MSG[_nLogType == L_MESSAGE ? L_INFO : _nLogType]);
  else
    fprintf(stderr, "%s", _szPrefix);
  fprintf(stderr, "%s", _szMessage);
  if (m_bUseColor) fprintf(stderr, "%s", COLOR_NORMAL);
  fflush(stderr);
}



//-----File---------------------------------------------------------------------
CLogService_File::CLogService_File(unsigned short _nLogTypes)
   : CLogService(_nLogTypes)
{
   m_fLog = NULL;
   m_nServiceType = S_FILE;
}

CLogService_File::~CLogService_File()
{
   if (m_fLog)
      fclose(m_fLog);
}

bool CLogService_File::SetLogFile(const char *_szFile, const char *_szFlags)
{
   if (m_fLog != NULL) fclose (m_fLog);
   m_fLog = fopen(_szFile, _szFlags);
   if (m_fLog != NULL)
     chmod(_szFile, 00600);
   return (m_fLog != NULL);
}

void CLogService_File::LogMessage(const char *_szPrefix, 
                                  const char *_szMessage,
                                  const unsigned short /* _nLogType */)
{
  if (m_fLog == NULL) return;
  fprintf(m_fLog, "%s%s", _szPrefix, _szMessage);
  fflush(m_fLog);
}


//-----Window-------------------------------------------------------------------
CPluginLog::CPluginLog()
{
  pipe(pipe_log);
  pthread_mutex_init(&mutex, NULL);
}

CPluginLog::~CPluginLog()
{
  // Empty
}


char *CPluginLog::NextLogMsg()
{
  pthread_mutex_lock(&mutex);
  char *sz = (m_vszLogs.size() > 0 ? m_vszLogs.front() : NULL);
  pthread_mutex_unlock(&mutex);
  return sz;
}

unsigned short CPluginLog::NextLogType()
{
  pthread_mutex_lock(&mutex);
  unsigned short n = (m_vnLogTypes.size() > 0 ? m_vnLogTypes.front() : 0);
  pthread_mutex_unlock(&mutex);
  return n;
}

void CPluginLog::ClearLog()
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

void CLogService_Plugin::LogMessage(const char *_szPrefix, 
                                    const char *_szMessage,
                                    const unsigned short _nLogType)
{
  static char szMsgMax[MAX_MSG_SIZE];

  if (m_xLogWindow == NULL) return;

  snprintf(szMsgMax, MAX_MSG_SIZE, "%s%s", _szPrefix, _szMessage);
  szMsgMax[MAX_MSG_SIZE - 1] = '\0';
  m_xLogWindow->AddLog(strdup(szMsgMax), _nLogType);
}


//-----CLogServer---------------------------------------------------------------
CLogServer::CLogServer()
{
  pthread_mutex_init(&mutex, NULL);
}

CLogServer::~CLogServer()
{
  pthread_mutex_lock(&mutex);
  vector<CLogService *>::iterator iter;
  for (iter = m_vxLogServices.begin(); iter != m_vxLogServices.end(); ++iter)
    delete *iter; 
  pthread_mutex_unlock(&mutex);

  int nResult = 0;
  do
  {
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex);
    nResult = pthread_mutex_destroy(&mutex);
  } while (nResult);
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
   for (iter = m_vxLogServices.begin(); iter != m_vxLogServices.end(); ++iter)
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
   for (iter = m_vxLogServices.begin(); iter != m_vxLogServices.end(); ++iter)
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
   for (iter = m_vxLogServices.begin(); iter != m_vxLogServices.end(); ++iter)
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
  for (iter = m_vxLogServices.begin(); iter != m_vxLogServices.end(); ++iter)
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
  for (iter = m_vxLogServices.begin(); iter != m_vxLogServices.end(); ++iter)
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


void CLogServer::Message(const char *_szFormat, ...)
{
   va_list argp;
   va_start(argp, _szFormat);
   Log(L_MESSAGE, _szFormat, argp);
   va_end(argp);
}
void CLogServer::Message(unsigned short _nServiceTypes, const char *_szFormat, ...)
{
   va_list argp;
   va_start(argp, _szFormat);
   Log(_nServiceTypes, L_MESSAGE, _szFormat, argp);
   va_end(argp);
}


void CLogServer::Log(const unsigned short _nLogType, const char *_szFormat, va_list argp)
{
  CLogServer::Log(S_ALL, _nLogType, _szFormat, argp);
}

void CLogServer::Log(const unsigned short _nServiceTypes, const unsigned short _nLogType, const char *_szFormat, va_list argp)
{
  static char szTime[32];
  static struct tm stm;
  static char szMsgMax[MAX_MSG_SIZE];

  pthread_mutex_lock(&mutex);

  // Create a time string for the log
  time_t t = time(NULL);
  localtime_r(&t, &stm);
  strftime(szTime, 32, "%T: ", &stm);

  vsnprintf(szMsgMax, MAX_MSG_SIZE, _szFormat, argp);
  szMsgMax[MAX_MSG_SIZE - 2] = '\n';
  szMsgMax[MAX_MSG_SIZE - 1] = '\0';

  // Log the event to each server
  vector<CLogService *>::iterator iter;
  for (iter = m_vxLogServices.begin(); iter != m_vxLogServices.end(); ++iter)
  {
    if ((*iter)->LogType(_nLogType) && ((*iter)->ServiceType() & _nServiceTypes))
        (*iter)->LogMessage(szTime, szMsgMax, _nLogType);
  }  
  pthread_mutex_unlock(&mutex);
}

