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
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

// Localization
#include "gettext.h"

#include "licq_sar.h"
#include "licq_log.h"
#include "sar.conf.h"
#include "licq_constants.h"

class CSARManager gSARManager;



/*---------------------------------------------------------------------------
 * CSavedAutoResponse
 *-------------------------------------------------------------------------*/
CSavedAutoResponse::CSavedAutoResponse(const char *_szName, const char *_szAutoResponse)
{
  m_szName = strdup(_szName);
  m_szAutoResponse = strdup(_szAutoResponse);
}

/*---------------------------------------------------------------------------
 * ~CSavedAutoResponse
 *-------------------------------------------------------------------------*/
CSavedAutoResponse::~CSavedAutoResponse()
{
  free(m_szName);
  free(m_szAutoResponse);
}


/*---------------------------------------------------------------------------
 * CSARManager
 *-------------------------------------------------------------------------*/
CSARManager::CSARManager()
{
}


/*---------------------------------------------------------------------------
 * ~CSARManager
 *-------------------------------------------------------------------------*/
CSARManager::~CSARManager()
{
  for (unsigned short i = 0; i < SAR_NUM_SECTIONS; i++)
    for (SARListIter iter = m_lSAR[i].begin(); iter != m_lSAR[i].end(); ++iter)
      delete (*iter);
}


/*---------------------------------------------------------------------------
 * CSARManager::Load
 *-------------------------------------------------------------------------*/
bool CSARManager::Load()
{
  char filename[128], szTemp1[32], szTemp2[128], szTemp3[512];
  unsigned short nTemp;
  const char *n[] = SAR_SECTIONS;

  gLog.Info(tr("%sLoading saved auto-responses.\n"), L_INITxSTR);

  // Get data from the config file
  snprintf(filename, sizeof(filename), "%s/sar.conf", BASE_DIR);
  filename[sizeof(filename) - 1] = '\0';
  // Try and load the file
  if (!m_fConf.LoadFile(filename))
  {
    //gLog.Warn("%sUnable to open SAR config file \"%s\", creating default.\n",
    //          L_WARNxSTR, filename);
    // Create sar.conf
    FILE *f = fopen(filename, "w");
    chmod(filename, 00600);
    fprintf(f, "%s", SAR_CONF);
    fclose(f);
    if (!m_fConf.ReloadFile()) return false;
  }

  // Read in auto response information
  for (unsigned short i = 0; i < SAR_NUM_SECTIONS; i++)
  {
    m_fConf.SetSection(n[i]);
    m_fConf.ReadNum("NumSAR", nTemp, 0);
    for (unsigned short j = 0; j < nTemp; j++)
    {
      sprintf(szTemp1, "SAR%d.Name", j + 1);
      m_fConf.ReadStr(szTemp1, szTemp2, "");
      sprintf(szTemp1, "SAR%d.Text", j + 1);
      m_fConf.ReadStr(szTemp1, szTemp3, "", true, sizeof(szTemp3));
      m_lSAR[i].push_back(new CSavedAutoResponse(szTemp2, szTemp3));
    }
  }

  return true;
}


/*---------------------------------------------------------------------------
 * CSARManager::Save
 *-------------------------------------------------------------------------*/
void CSARManager::Save()
{
  const char* n[] = SAR_SECTIONS;
  char sz[32];
  for (unsigned short i = 0; i < SAR_NUM_SECTIONS; i ++)
  {
    m_fConf.SetSection(n[i]);
    m_fConf.WriteNum("NumSAR", (unsigned short)m_lSAR[i].size());
    unsigned short j = 1;
    for (SARListIter iter = m_lSAR[i].begin(); iter != m_lSAR[i].end(); ++iter, j++)
    {
      sprintf(sz, "SAR%d.Name", j);
      m_fConf.WriteStr(sz, (*iter)->Name());
      sprintf(sz, "SAR%d.Text", j);
      m_fConf.WriteStr(sz, (*iter)->AutoResponse());
    }
  }
  m_fConf.FlushFile();
}


/*---------------------------------------------------------------------------
 * SARManager::SavedAutoResponses
 *-------------------------------------------------------------------------*/
SARList &CSARManager::Fetch(unsigned short n)
{
  pthread_mutex_lock(&mutex);
  if (n >= SAR_NUM_SECTIONS) n = 0;
  return m_lSAR[n];
}


void CSARManager::Drop()
{
  pthread_mutex_unlock(&mutex);
}


