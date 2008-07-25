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

#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "time-fix.h"

#ifdef HAVE_PATHS_H
#include <paths.h>
#else
#define _PATH_BSHELL "/bin/sh"
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#else
extern int errno;
#endif

// Localization
#include "gettext.h"

#include "licq_utility.h"
#include "licq_log.h"
#include "licq_user.h"
#include "support.h"

CUtilityManager gUtilityManager;

using namespace std;

//=====CUtilityManager==========================================================
int SelectUtility(const struct dirent *d)
{
  char *pcDot = strrchr(d->d_name, '.');
  if (pcDot == NULL) return (0);
  return (strcmp(pcDot, ".utility") == 0);
}

CUtilityManager::CUtilityManager()
{
  // does nothing for now
}

CUtilityManager::~CUtilityManager()
{
  std::vector <CUtility *>::iterator iter;
  for (iter = m_vxUtilities.begin(); iter != m_vxUtilities.end(); ++iter)
    delete *iter;
}

unsigned short CUtilityManager::LoadUtilities(const char *_szDir)
{
  struct dirent **namelist;

  gLog.Info(tr("%sLoading utilities.\n"), L_INITxSTR);
  int n = scandir_alpha_r(_szDir, &namelist, SelectUtility);
  if (n < 0)
  {
    gLog.Error("%sError reading utility directory \"%s\":\n%s%s.\n", L_ERRORxSTR,
              _szDir, L_BLANKxSTR, strerror(errno));
    return (0);
  }

  CUtility *p;
  for (unsigned short i = 0; i < n; i++)
  {
    char szFile[MAX_FILENAME_LEN];
    snprintf(szFile, MAX_FILENAME_LEN, "%s/%s", _szDir, namelist[i]->d_name);
    szFile[MAX_FILENAME_LEN - 1] = '\0';
    free (namelist[i]);
    p = new CUtility(szFile);
    if (p->Exception())
    {
      gLog.Warn(tr("%sWarning: unable to load utility \"%s\".\n"), L_WARNxSTR, namelist[i]->d_name);
      continue;
    }
    m_vxUtilities.push_back(p);
  }
  free(namelist);

  return m_vxUtilities.size();
}


//=====CUtility==================================================================
CUtility::CUtility(const char *_szFileName)
{
  // Assumes the given filename is in the form <directory>/<pluginname>.plugin
  bException = false;
  CIniFile fUtility(INI_FxWARN);
  if (!fUtility.LoadFile(_szFileName))
  {
    bException = true;
    return;
  }

  fUtility.SetSection("utility");
  char szTemp[MAX_LINE_LEN];

  // Read in the window
  fUtility.ReadStr("Window", szTemp, "GUI");
  if (strcmp(szTemp, "GUI") == 0)
    m_eWinType = UtilityWinGui;
  else if (strcmp(szTemp, "TERM") == 0)
    m_eWinType = UtilityWinTerm;
  else if (strcmp(szTemp, "LICQ") == 0)
    m_eWinType = UtilityWinLicq;
  else
  {
    gLog.Warn(tr("%sWarning: Invalid entry in plugin \"%s\":\nWindow = %s\n"),
              L_WARNxSTR, _szFileName, szTemp);
    bException = true;
    return;
  }

  // Read in the command
  if (!fUtility.ReadStr("Command", szTemp))
  {
    bException = true;
    return;
  }
  m_szCommand = strdup(szTemp);
  fUtility.ReadStr("Description", szTemp, tr("none"));
  m_szDescription = strdup(szTemp);
  m_szFullCommand = NULL;

  // Parse command for %# user fields
  char *pcField = m_szCommand;
  int nField, nCurField = 1;
  while ((pcField = strchr(pcField, '%')) != NULL)
  {
    char cField = *(pcField + 1);
    if (isdigit(cField))
    {
      nField = cField - '0';
      if (nField == 0 || nField > nCurField)
      {
        gLog.Warn("%sWarning: Out-of-order user field id (%d) in plugin \"%s\".\n",
                  L_WARNxSTR, nField, _szFileName);
      }
      else if (nField == nCurField)
      {
        char szTitle[MAX_LINE_LEN], szDefault[MAX_LINE_LEN];
        sprintf(szTemp, "User%d.Title", nField);
        fUtility.ReadStr(szTemp, szTitle, "User field");
        sprintf(szTemp, "User%d.Default", nField);
        fUtility.ReadStr(szTemp, szDefault, "");
        m_vxUserField.push_back(new CUtilityUserField(szTitle, szDefault));
        nCurField = nField + 1;
      }
    }
    pcField++;
    if (*pcField == '\0') break;
    pcField++;
  }

  strncpy(szTemp, _szFileName, MAX_LINE_LEN - 1);
  szTemp[MAX_LINE_LEN - 1] = '\0';
  // Replace the terminating .plugin by '\0'plugin
  pcField = strrchr(szTemp, '.');
  if (pcField != NULL) *pcField = '\0';
  // Find the beginning of the plugin name
  pcField = strrchr(szTemp, '/');
  if (pcField == NULL)
    pcField = szTemp;
  else
    pcField++;

  m_szName = strdup(pcField);
}


CUtility::~CUtility()
{
  std::vector <CUtilityUserField *>::iterator iter;
  for (iter = m_vxUserField.begin(); iter != m_vxUserField.end(); ++iter)
    delete *iter;

  free(m_szName);
  free(m_szCommand);
  free(m_szDescription);
  delete []m_szFullCommand;
}

bool CUtility::SetFields(unsigned long _nUin)
{
  char id[16];
  snprintf(id, 16, "%lu", _nUin);
  return SetFields(id, LICQ_PPID);
}

bool CUtility::SetFields(const char *szId, unsigned long nPPID)
{
  const ICQUser *u = gUserManager.FetchUser(szId, nPPID, LOCK_R);
  if (u == NULL) return false;
  if (m_szFullCommand != NULL) delete [] m_szFullCommand;
  char *szTmp;
  szTmp = u->usprintf(m_szCommand, USPRINTF_NOFW|USPRINTF_LINEISCMD);
  m_szFullCommand = new char[MAX_CMD_LEN];
  strncpy(m_szFullCommand, szTmp, MAX_CMD_LEN);
  m_szFullCommand[MAX_CMD_LEN - 1] = '\0';
  free(szTmp);
  vector<CUtilityUserField *>::iterator iter;
  for (iter = m_vxUserField.begin(); iter != m_vxUserField.end(); ++iter)
    (*iter)->SetFields(u);
  gUserManager.DropUser(u);
  return true;
}

void CUtility::SetUserFields(const vector <const char *> &_vszUserFields)
{
  if ( _vszUserFields.size() != NumUserFields())
  {
    gLog.Warn("%sInternal error: CUtility::SetUserFields(): incorrect number of data fields (%d/%d).\n",
              L_WARNxSTR, int(_vszUserFields.size()), NumUserFields());
    return;
  }
  // Do a quick check to see if there are any users fields at all
  if (NumUserFields() == 0) return;

  char *szTemp = strdup(m_szFullCommand);
  char *pcFieldStart = szTemp, *pcFieldEnd;
  m_szFullCommand[0] = '\0';
  while ((pcFieldEnd = strchr(pcFieldStart, '%')) != NULL)
  {
    *pcFieldEnd = '\0';
    pcFieldEnd++;
    strcat(m_szFullCommand, pcFieldStart);
    // Anything non-digit at this point we just ignore
    if (isdigit(*pcFieldEnd))
      // We know that any user field numbers are valid from the constructor
        strcat(m_szFullCommand, _vszUserFields[*pcFieldEnd - '1']);

    pcFieldStart = pcFieldEnd;
    if (pcFieldStart == '\0') break;
    pcFieldStart++;
  }
  strcat(m_szFullCommand, pcFieldStart);
  free(szTemp);
}

//===========================================================================

CUtilityUserField::CUtilityUserField(const char *_szTitle, const char *_szDefault)
{
  m_szTitle = strdup(_szTitle);
  m_szDefault = strdup(_szDefault);
}

CUtilityUserField::~CUtilityUserField()
{
  free (m_szTitle);
  free (m_szDefault);
}

bool CUtilityUserField::SetFields(const ICQUser *u)
{
  char *szTmp;
  szTmp = u->usprintf(m_szDefault, USPRINTF_NOFW|USPRINTF_LINEISCMD);
  m_szFullDefault = new char[MAX_CMD_LEN];
  strncpy(m_szFullDefault, szTmp, MAX_CMD_LEN);
  m_szFullDefault[MAX_CMD_LEN - 1] = '\0';
  free(szTmp);
  return true;
}

//===========================================================================

CUtilityInternalWindow::CUtilityInternalWindow()
{
  fStdOut = fStdErr = NULL;
  pid = -1;
}

CUtilityInternalWindow::~CUtilityInternalWindow()
{
  if (Running()) PClose();
}


bool CUtilityInternalWindow::POpen(const char *cmd)
{
  int pdes_out[2], pdes_err[2];

  if (pipe(pdes_out) < 0) return false;
  if (pipe(pdes_err) < 0) return false;

  switch (pid = fork())
  {
    case -1:                        /* Error. */
    {
      close(pdes_out[0]);
      close(pdes_out[1]);
      close(pdes_err[0]);
      close(pdes_err[1]);
      return false;
      /* NOTREACHED */
    }
    case 0:                         /* Child. */
    {
      if (pdes_out[1] != STDOUT_FILENO)
      {
        dup2(pdes_out[1], STDOUT_FILENO);
        close(pdes_out[1]);
      }
      close(pdes_out[0]);
      if (pdes_err[1] != STDERR_FILENO)
      {
        dup2(pdes_err[1], STDERR_FILENO);
        close(pdes_err[1]);
      }
      close(pdes_err[0]);
      execl(_PATH_BSHELL, "sh", "-c", cmd, NULL);
      _exit(127);
      /* NOTREACHED */
    }
  }

  /* Parent; assume fdopen can't fail. */
  fStdOut = fdopen(pdes_out[0], "r");
  close(pdes_out[1]);
  fStdErr = fdopen(pdes_err[0], "r");
  close(pdes_err[1]);

  // Set both streams to line buffered
  setvbuf(fStdOut, (char*)NULL, _IOLBF, 0);
  setvbuf(fStdErr, (char*)NULL, _IOLBF, 0);

  return true;
}


int CUtilityInternalWindow::PClose()
{
   int r, pstat;
   struct timeval tv = { 0, 200000 };

   // Close the file descriptors
   fclose(fStdOut);
   fclose(fStdErr);
   fStdOut = fStdErr = NULL;

   // See if the child is still there
   r = waitpid(pid, &pstat, WNOHANG);
   // Return if child has exited or there was an error
   if (r == pid || r == -1) goto pclose_leave;

   // Give the process another .2 seconds to die
   select(0, NULL, NULL, NULL, &tv);

   // Still there?
   r = waitpid(pid, &pstat, WNOHANG);
   if (r == pid || r == -1) goto pclose_leave;

   // Try and kill the process
   if (kill(pid, SIGTERM) == -1) return -1;

   // Give it 1 more second to die
   tv.tv_sec = 1;
   tv.tv_usec = 0;
   select(0, NULL, NULL, NULL, &tv);

   // See if the child is still there
   r = waitpid(pid, &pstat, WNOHANG);
   if (r == pid || r == -1) goto pclose_leave;

   // Kill the bastard
   kill(pid, SIGKILL);
   // Now he will die for sure
   r = waitpid(pid, &pstat, 0);

pclose_leave:

   if (r == -1 || !WIFEXITED(pstat))
     return -1;
   return WEXITSTATUS(pstat);

}
