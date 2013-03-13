/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1998-2013 Licq developers <licq-dev@googlegroups.com>
 *
 * Licq is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Licq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Licq; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"

#include <licq/utility.h>

#include <cerrno>
#include <ctime>
#include <ctype.h>
#include <dirent.h>
#ifdef __sun
# define _PATH_BSHELL "/bin/sh"
#else
# include <paths.h>
#endif
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <licq/contactlist/user.h>
#include <licq/inifile.h>
#include <licq/logging/log.h>

#include "gettext.h"

using Licq::Utility;
using Licq::UtilityInternalWindow;
using Licq::UtilityManager;
using Licq::UtilityUserField;
using std::string;
using std::vector;

Licq::UtilityManager Licq::gUtilityManager;


UtilityManager::UtilityManager()
{
  // Empty
}

UtilityManager::~UtilityManager()
{
  std::vector<Utility*>::iterator iter;
  for (iter = myUtilities.begin(); iter != myUtilities.end(); ++iter)
    delete *iter;
}

int UtilityManager::loadUtilities(const string& dirname)
{
  gLog.info(tr("Loading utilities"));

  DIR* dir = opendir(dirname.c_str());
  if (dir == NULL)
  {
    gLog.error(tr("Error reading utility directory \"%s\":\n%s"),
        dirname.c_str(), strerror(errno));
    return (0);
  }

  char* ent = new char[offsetof(struct dirent, d_name) +
      pathconf(dirname.c_str(), _PC_NAME_MAX) + 1];
  struct dirent* res;

  while (readdir_r(dir, (struct dirent*)ent, &res) == 0 && res != NULL)
  {
    const char* dot = strrchr(res->d_name, '.');
    if (dot == NULL || strcmp(dot, ".utility") != 0)
      continue;

    string filename = dirname + "/" + res->d_name;
    Utility* p = new Utility(filename);
    if (p->isFailed())
    {
      gLog.warning(tr("Warning: unable to load utility \"%s\""), res->d_name);
      continue;
    }
    myUtilities.push_back(p);
  }
  delete [] ent;
  closedir(dir);

  return myUtilities.size();
}


Utility::Utility(const string& filename)
{
  // Assumes the given filename is in the form <directory>/<pluginname>.plugin
  myIsFailed = false;
  Licq::IniFile utilConf(filename);
  if (!utilConf.loadFile())
  {
    myIsFailed = true;
    return;
  }

  utilConf.setSection("utility");

  // Read in the window
  string window;
  utilConf.get("Window", window, "GUI");
  if (window == "GUI")
    myWinType = WinGui;
  else if (window ==  "TERM")
    myWinType = WinTerm;
  else if (window == "LICQ")
    myWinType = WinLicq;
  else
  {
    gLog.warning(tr("Invalid entry in plugin \"%s\": Window = %s"),
        filename.c_str(), window.c_str());
    myIsFailed = true;
    return;
  }

  // Read in the command
  if (!utilConf.get("Command", myCommand))
  {
    myIsFailed = true;
    return;
  }
  utilConf.get("Description", myDescription, tr("none"));

  // Parse command for %# user fields
  size_t pcField = 0;
  int nField, nCurField = 1;
  while ((pcField = myCommand.find('%', pcField)) != string::npos)
  {
    char cField = myCommand[pcField + 1];
    if (isdigit(cField))
    {
      nField = cField - '0';
      if (nField == 0 || nField > nCurField)
      {
        gLog.warning(tr("Warning: Out-of-order user field id (%d) in plugin \"%s\""),
            nField, filename.c_str());
      }
      else if (nField == nCurField)
      {
        char key[30];
        string title, defaultValue;
        sprintf(key, "User%d.Title", nField);
        utilConf.get(key, title, "User field");
        sprintf(key, "User%d.Default", nField);
        utilConf.get(key, defaultValue, "");
        myUserFields.push_back(new UtilityUserField(title, defaultValue));
        nCurField = nField + 1;
      }
    }
    pcField += 2;
  }

  size_t startPos = filename.rfind('/');
  size_t endPos = filename.rfind('.');
  if (startPos == string::npos)
    startPos = 0;
  else
    ++startPos;
  if (endPos == string::npos)
    myName = filename.substr(startPos);
  else
    myName = filename.substr(startPos, endPos - startPos);
}


Utility::~Utility()
{
  std::vector<UtilityUserField *>::iterator iter;
  for (iter = myUserFields.begin(); iter != myUserFields.end(); ++iter)
    delete *iter;
}

bool Utility::setFields(const UserId& userId)
{
  Licq::UserReadGuard u(userId);
  if (!u.isLocked())
    return false;
  myFullCommand = u->usprintf(myCommand, Licq::User::usprintf_quoteall, false, false);
  vector<UtilityUserField *>::iterator iter;
  for (iter = myUserFields.begin(); iter != myUserFields.end(); ++iter)
    (*iter)->setFields(*u);
  return true;
}

void Utility::setUserFields(const vector<string>& userFields)
{
  if (static_cast<int>(userFields.size()) != numUserFields())
  {
    gLog.warning(tr("Internal error: %s: incorrect number of data fields (%d/%d)"),
        __func__, int(userFields.size()), numUserFields());
    return;
  }
  // Do a quick check to see if there are any users fields at all
  if (numUserFields() == 0)
    return;

  size_t pcField;
  while ((pcField = myFullCommand.find('%')) != string::npos)
  {
    char c = myFullCommand[pcField+1];
    if (isdigit(c))
    {
      // We know that any user field numbers are valid from the constructor
      myFullCommand.replace(pcField, 2,  userFields[c - '1']);
    }
    else
    {
      // Anything non-digit at this point we just ignore
      myFullCommand.erase(pcField, 2);
    }
  }
}


UtilityUserField::UtilityUserField(const string& title, const string& defaultValue)
  : myTitle(title),
    myDefaultValue(defaultValue)
{
  // Empty
}

UtilityUserField::~UtilityUserField()
{
  // Empty
}

bool UtilityUserField::setFields(const User* u)
{
  myFullDefault = u->usprintf(myDefaultValue, Licq::User::usprintf_quoteall, false, false);
  return true;
}


UtilityInternalWindow::UtilityInternalWindow()
{
  fStdOut = fStdErr = NULL;
  pid = -1;
}

UtilityInternalWindow::~UtilityInternalWindow()
{
  if (Running()) PClose();
}

bool UtilityInternalWindow::POpen(const string& command)
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
      execl(_PATH_BSHELL, "sh", "-c", command.c_str(), NULL);
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


int UtilityInternalWindow::PClose()
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
