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
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>
#include <string.h>
#include <string>

using namespace std;

#ifdef HAVE_ERRNO_H
#include <errno.h>
#else
extern int errno;
#endif

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

// Localization
#include "gettext.h"

#include "licq_icq.h"
#include "licq.h"
#include "licq_log.h"
#include "licq_countrycodes.h"
#include "licq_utility.h"
#include "support.h"
#include "licq_sar.h"
#include "licq_user.h"
#include "licq_icqd.h"
#include "licq_socket.h"
#include "licq_protoplugind.h"

#include "licq.conf.h"

using namespace std;

/*-----Start OpenSSL code--------------------------------------------------*/

#ifdef USE_OPENSSL
#include <openssl/ssl.h>
#include <openssl/dh.h>
#include <openssl/opensslv.h>

extern SSL_CTX *gSSL_CTX;
extern SSL_CTX *gSSL_CTX_NONICQ;

// AUTOGENERATED by dhparam
static DH *get_dh512()
        {
        static unsigned char dh512_p[]={
                0xFF,0xD3,0xF9,0x7C,0xEB,0xFE,0x45,0x2E,0x47,0x41,0xC1,0x8B,
                0xF7,0xB9,0xC6,0xF2,0x40,0xCF,0x10,0x8B,0xF3,0xD7,0x08,0xC7,
                0xF0,0x3F,0x46,0x7A,0xAD,0x71,0x6A,0x70,0xE1,0x76,0x8F,0xD9,
                0xD4,0x46,0x70,0xFB,0x31,0x9B,0xD8,0x86,0x58,0x03,0xE6,0x6F,
                0x08,0x9B,0x16,0xA0,0x78,0x70,0x6C,0xB1,0x78,0x73,0x52,0x3F,
                0xD2,0x74,0xED,0x9B,
                };
        static unsigned char dh512_g[]={
                0x02,
                };
        DH *dh;

        if ((dh=DH_new()) == NULL) return(NULL);
        dh->p=BN_bin2bn(dh512_p,sizeof(dh512_p),NULL);
        dh->g=BN_bin2bn(dh512_g,sizeof(dh512_g),NULL);
        if ((dh->p == NULL) || (dh->g == NULL))
                { DH_free(dh); return(NULL); }
        return(dh);
        }

#ifdef SSL_DEBUG
void ssl_info_callback(SSL *s, int where, int ret)
{
    const char *str;
    int w;

    w = where & ~SSL_ST_MASK;

    if (w & SSL_ST_CONNECT) str="SSL_connect";
    else if (w & SSL_ST_ACCEPT) str="SSL_accept";
    else str="undefined";

    if (where & SSL_CB_LOOP)
    {
        gLog.Info("%s%s:%s\n",L_SSLxSTR,str,SSL_state_string_long(s));
    }
    else if (where & SSL_CB_ALERT)
    {
        str=(where & SSL_CB_READ)?"read":"write";
        gLog.Info("%sSSL3 alert %s:%s:%s\n",L_SSLxSTR,
            str,
            SSL_alert_type_string_long(ret),
            SSL_alert_desc_string_long(ret));
    }
    else if (where & SSL_CB_EXIT)
    {
        if (ret == 0)
            gLog.Info("%s%s:failed in %s\n",L_SSLxSTR,
                str,SSL_state_string_long(s));
        else if (ret < 0)
        {
        gLog.Info("%s%s:%s\n",L_SSLxSTR,str,SSL_state_string_long(s));
        }
    }
    else if (where & SSL_CB_ALERT)
    {
        str=(where & SSL_CB_READ)?"read":"write";
        gLog.Info("%sSSL3 alert %s:%s:%s\n",L_SSLxSTR,
            str,
            SSL_alert_type_string_long(ret),
            SSL_alert_desc_string_long(ret));
    }
    else if (where & SSL_CB_EXIT)
    {
        if (ret == 0)
            gLog.Info("%s%s:failed in %s\n",L_SSLxSTR,
                str,SSL_state_string_long(s));
        else if (ret < 0)
        {
            gLog.Info("%s%s:error in %s\n",L_SSLxSTR,
                str,SSL_state_string_long(s));
        }
    }
}
#endif
#endif
/*-----End of OpenSSL code-------------------------------------------------*/


/*-----Helper functions for CLicq::UpgradeLicq-----------------------------*/
int SelectUserUtility(const struct dirent *d)
{
  char *pcDot = strrchr(d->d_name, '.');
  if (pcDot == NULL) return (0);
  return (strcmp(pcDot, ".uin") == 0);
}

int SelectHistoryUtility(const struct dirent *d)
{
  char *pcDot = strchr(d->d_name, '.');
  if (pcDot == NULL) return (0);
  return (strcmp(pcDot, ".history") == 0 ||
          strcmp(pcDot, ".history.removed") == 0);
}

char **global_argv = NULL;
int global_argc = 0;

// Plugin variables
pthread_cond_t LP_IdSignal;
pthread_mutex_t LP_IdMutex;
list<unsigned short> LP_Ids;

CLicq::CLicq()
{
  DEBUG_LEVEL = 0;
  licqDaemon = NULL;
  pthread_mutex_init(&mutex_plugins, NULL);
  pthread_mutex_init(&mutex_protoplugins, NULL);
  m_bDeletePID = true;

  //FIXME ICQ should be put into its own plugin.
  CProtoPlugin *p = new CProtoPlugin;
  p->m_nPPID = LICQ_PPID;
  p->m_szLibName = strdup("");
  list_protoplugins.push_back(p);
}

bool CLicq::Init(int argc, char **argv)
{
  char *szRedirect = NULL;
  char szFilename[MAX_FILENAME_LEN];
  vector <char *> vszPlugins;

  // parse command line for arguments
  bool bHelp = false;
  bool bFork = false;
  bool bBaseDir = false;
  bool bForceInit = false;
  bool bCmdLinePlugins = false;
  bool bRedirect_ok = false;
  bool bUseColor = true;
  // Check the no one is trying session management on us
  if (argc > 1 && strcmp(argv[1], "-session") == 0)
  {
    fprintf(stderr, tr("Session management is not supported by Licq.\n"));
  }
  else
  {
    int i = 0;
#ifdef __GLIBC__
    while( (i = getopt(argc, argv, "--hd:b:p:Io:fc")) > 0)
#else
    while( (i = getopt(argc, argv, "hd:b:p:Io:fc")) > 0)
#endif
    {
      switch (i)
      {
      case 'h':  // help
        PrintUsage();
        bHelp = true;
        break;
      case 'b':  // base directory
        snprintf(BASE_DIR, MAX_FILENAME_LEN, "%s", optarg);
        BASE_DIR[MAX_FILENAME_LEN - 1] = '\0';
        bBaseDir = true;
        break;
      case 'd':  // DEBUG_LEVEL
        DEBUG_LEVEL = atol(optarg);
        break;
      case 'c':  // use color
        bUseColor = false;
        break;
      case 'I':  // force init
        bForceInit = true;
        break;
      case 'p':  // new plugin
        vszPlugins.push_back(strdup(optarg));
        bCmdLinePlugins = true;
        break;
      case 'o':  // redirect stderr
        szRedirect = strdup(optarg);
        break;
      case 'f':  // fork
        bFork = true;
        break;
      }
    }
  }

  // Save the command line arguments in case anybody cares
  global_argc = argc;
  global_argv = argv;

  // Fork into the background
  if (bFork && fork()) exit(0);

  // See if redirection works, set bUseColor to false if we redirect
  // to a file.
  if (szRedirect)
    bRedirect_ok = Redirect(szRedirect);

  if(!isatty(STDERR_FILENO))
    bUseColor = false;

  // Initialise the log server for standard output and dump all initial errors
  // and warnings to it regardless of DEBUG_LEVEL
  gLog.AddService(new CLogService_StdErr(DEBUG_LEVEL | L_ERROR | L_WARN, bUseColor));

  // Redirect stdout and stderr if asked to
  if (szRedirect) {
    if (bRedirect_ok)
      gLog.Info(tr("%sOutput redirected to \"%s\".\n"), L_INITxSTR, szRedirect);
    else
      gLog.Warn(tr("%sRedirection to \"%s\" failed:\n%s%s.\n"), L_WARNxSTR,
                szRedirect, L_BLANKxSTR, strerror(errno));
    free (szRedirect);
    szRedirect = NULL;
  }

  // if no base directory set on the command line then get it from HOME
  if (!bBaseDir)
  {
     char *home;
     if ((home = getenv("HOME")) == NULL)
     {
       gLog.Error("%sLicq: $HOME not set, unable to determine config base directory.\n", L_ERRORxSTR);
       return false;
     }
     snprintf(BASE_DIR, MAX_FILENAME_LEN, "%s/.licq", home);
     BASE_DIR[MAX_FILENAME_LEN - 1] = '\0';
  }

  // check if user has conf files installed, install them if not
  if ( (access(BASE_DIR, F_OK) < 0 || bForceInit) && !Install() )
    return false;

  // Define the directory for all the shared data
  sprintf(SHARE_DIR, "%s/%s", INSTALL_PREFIX, BASE_SHARE_DIR);
  sprintf(LIB_DIR, "%s/%s", INSTALL_PREFIX, BASE_LIB_DIR);

  // Check pid
  char szConf[MAX_FILENAME_LEN], szKey[32];
  snprintf(szConf, MAX_FILENAME_LEN, "%s/licq.pid", BASE_DIR);
  szConf[MAX_FILENAME_LEN - 1] = '\0';
  FILE *fs = fopen(szConf, "r");
  if (fs != NULL)
  {
    fgets(szKey, 32, fs);
    pid_t pid = atol(szKey);
    if (pid != 0)
    {
      if (kill(pid, 0) == -1) {
        gLog.Warn(tr("%sLicq: Ignoring stale lockfile (pid %d)\n"), L_WARNxSTR, pid);
      }
      else
      {
        gLog.Error(tr("%sLicq: Already running at pid %d.\n"
                      "%s      Kill process or remove %s.\n"),
                   L_ERRORxSTR, pid, L_BLANKxSTR, szConf);
        m_bDeletePID = false;
        return false;
      }
    }
    fclose(fs);
  }
  fs = fopen(szConf, "w");
  if (fs != NULL)
  {
    chmod(szConf, 00600);
    fprintf(fs, "%d\n", getpid());
    fclose(fs);
  }
  else
    gLog.Warn(tr("%sLicq: %s cannot be opened for writing.\n"
                 "%s      skipping lockfile protection.\n"),
              L_WARNxSTR, szConf, L_BLANKxSTR);

  // Open the config file
  CIniFile licqConf(INI_FxWARN | INI_FxALLOWxCREATE);
  snprintf(szConf, MAX_FILENAME_LEN, "%s/licq.conf", BASE_DIR);
  szConf[MAX_FILENAME_LEN - 1] = '\0';
  if (licqConf.LoadFile(szConf) == false)
    return false;

  // Verify the version
  licqConf.SetSection("licq");
  unsigned short nVersion;
  licqConf.ReadNum("Version", nVersion, 0);
  if (nVersion < 1028)
  {
    gLog.Info("%sUpgrading config file formats.\n", L_SBLANKxSTR);
    if (UpgradeLicq(licqConf))
      gLog.Info("%sUpgrade completed.\n", L_SBLANKxSTR);
    else
    {
      gLog.Warn("%sUpgrade failed. Please save your licq directory and\n"
                "%sreport this as a bug.\n", L_ERRORxSTR, L_BLANKxSTR);
      return false;
    }
  }
  else if (nVersion < INT_VERSION)
  {
    licqConf.WriteNum("Version", (unsigned short)INT_VERSION);
    licqConf.FlushFile();
  }

  // Load up the plugins
  m_nNextId = 1;
  vector <char *>::iterator iter;
  for (iter = vszPlugins.begin(); iter != vszPlugins.end(); iter++)
  {
    if (!LoadPlugin(*iter, argc, argv)) return false;
    if (bHelp)
    {
      fprintf(stderr, "Licq Plugin: %s %s\n%s\n----------\n",
          list_plugins.back()->Name(),
          list_plugins.back()->Version(),
          (*(list_plugins.back())->fUsage)() );
      list_plugins.pop_back();
    }
    free(*iter);
  }
  if (bHelp) return false;

  // Find and load the plugins from the conf file
  if (!bHelp && !bCmdLinePlugins)
  {
    unsigned short nNumPlugins = 0;
    char szData[MAX_FILENAME_LEN];
    if (licqConf.SetSection("plugins") && licqConf.ReadNum("NumPlugins", nNumPlugins) && nNumPlugins > 0)
    {
      for (int i = 0; i < nNumPlugins; i++)
      {
        sprintf(szKey, "Plugin%d", i + 1);
        if (!licqConf.ReadStr(szKey, szData)) continue;
        if (LoadPlugin(szData, argc, argv) == NULL) return false;
      }
    }
    else  // If no plugins, try some defaults one by one
    {
      if (LoadPlugin("qt-gui", argc, argv) == NULL)
        if (LoadPlugin("kde-gui", argc, argv) == NULL)
          if (LoadPlugin("jons-gtk-gui", argc, argv) == NULL)
            if (LoadPlugin("console", argc, argv) == NULL)
              return false;
    }
  }

  // Close the conf file
  licqConf.CloseFile();

#ifdef USE_OPENSSL
  // Initialize SSL
  SSL_library_init();
  gSSL_CTX = SSL_CTX_new(TLSv1_method());
  gSSL_CTX_NONICQ = SSL_CTX_new(TLSv1_method());
#if OPENSSL_VERSION_NUMBER >= 0x00905000L
  SSL_CTX_set_cipher_list(gSSL_CTX, "ADH:@STRENGTH");
#else
  SSL_CTX_set_cipher_list(gSSL_CTX, "ADH");
#endif

#ifdef SSL_DEBUG
  SSL_CTX_set_info_callback(gSSL_CTX, (void (*)())ssl_info_callback);
#endif

  DH *dh = get_dh512();
  SSL_CTX_set_tmp_dh(gSSL_CTX, dh);
  DH_free(dh);
#endif

  //TODO Load protocol plugins

  // Start things going
  if (!gUserManager.Load())
    return false;
  gSARManager.Load();
  sprintf(szFilename, "%s%s", SHARE_DIR, UTILITY_DIR);
  gUtilityManager.LoadUtilities(szFilename);

  // Create the daemon
  licqDaemon = new CICQDaemon(this);

  return true;
}

CLicq::~CLicq()
{
  // Close the plugins
  //...
  // Kill the daemon
  if (licqDaemon != NULL) delete licqDaemon;

  // Remove the lock file
  if (m_bDeletePID)
  {
    char szConf[MAX_FILENAME_LEN];
    snprintf(szConf, MAX_FILENAME_LEN, "%s/licq.pid", BASE_DIR);
    remove(szConf);
  }
}


const char *CLicq::Version()
{
  static const char version[] = VERSION;
  return version;
}


/*-----------------------------------------------------------------------------
 * UpgradeLicq
 *
 * Upgrades the config files to the current version.
 *---------------------------------------------------------------------------*/
bool CLicq::UpgradeLicq(CIniFile &licqConf)
{  
  CIniFile ownerFile(INI_FxERROR);
  string strBaseDir = BASE_DIR;
  string strOwnerFile = strBaseDir + "/owner.uin";
  if (!ownerFile.LoadFile(strOwnerFile.c_str()))
    return false;

  // Get the UIN
  unsigned long nUin;
  ownerFile.SetSection("user");
  ownerFile.ReadNum("Uin", nUin, 0);
  ownerFile.CloseFile();

  // Set the new version number
  licqConf.SetSection("licq");
  licqConf.WriteNum("Version", (unsigned short)INT_VERSION);  
 
  // Create the owner section and fill it
  licqConf.SetSection("owners");
  licqConf.WriteNum("NumOfOwners", (unsigned short)1);
  licqConf.WriteNum("Owner1.Id", nUin);
  licqConf.WriteStr("Owner1.PPID", "Licq");
  
  // Add the protocol plugins info
  licqConf.SetSection("plugins");
  licqConf.WriteNum("NumProtoPlugins", (unsigned short)0);
  licqConf.FlushFile();
  
  // Rename owner.uin to owner.Licq
  string strNewOwnerFile = strBaseDir + "/owner.Licq";
  if (rename(strOwnerFile.c_str(), strNewOwnerFile.c_str()))
    return false;

  // Update all the user files and update users.conf
  struct dirent **UinFiles;
  string strUserDir = strBaseDir + "/users";
  string strUsersConf = strBaseDir + "/users.conf";
  int n = scandir_alpha_r(strUserDir.c_str(), &UinFiles, SelectUserUtility);
  if (n != 0)
  {
    CIniFile userConfFile(INI_FxERROR);
    if (!userConfFile.LoadFile(strUsersConf.c_str()))
      return false;
    userConfFile.SetSection("users");  
    userConfFile.WriteNum("NumOfUsers", (unsigned short)n);
    for (unsigned short i = 0; i < n; i++)
    {
      char szKey[20];
      snprintf(szKey, sizeof(szKey), "User%d", i+1);
      string strFileName = strUserDir + "/" + UinFiles[i]->d_name;
      string strNewName = UinFiles[i]->d_name;
      strNewName.replace(strNewName.find(".uin", 0), 4, ".Licq");
      string strNewFile = strUserDir + "/" + strNewName;
      if (rename(strFileName.c_str(), strNewFile.c_str()))
        return false;
      userConfFile.WriteStr(szKey, strNewName.c_str());
    }
    
    userConfFile.FlushFile();
  }
  
  // Rename the history files
  struct dirent **HistoryFiles;
  string strHistoryDir = strBaseDir + "/history";
  int nNumHistory = scandir_alpha_r(strHistoryDir.c_str(), &HistoryFiles,
    SelectHistoryUtility);
  if (nNumHistory)
  {
    for (unsigned short i = 0; i < nNumHistory; i++)
    {
      string strFileName = strHistoryDir + "/" + HistoryFiles[i]->d_name;
      string strNewFile = strHistoryDir + "/" + HistoryFiles[i]->d_name;
      strNewFile.replace(strNewFile.find(".history", 0), 8, ".Licq.history");
      if (rename(strFileName.c_str(), strNewFile.c_str()))
        return false;
    }
  }
  
  return true;
}

/*-----------------------------------------------------------------------------
 * LoadPlugin
 *
 * Loads the given plugin using the given command line arguments.
 *---------------------------------------------------------------------------*/
CPlugin *CLicq::LoadPlugin(const char *_szName, int argc, char **argv)
{
  void *handle;
  const char *error;
  CPlugin *p = new CPlugin(_szName);
  char szPlugin[MAX_FILENAME_LEN];

  // First check if the plugin is in the shared location
  if ( _szName[0] != '/' && _szName[0] != '.')
  {
    snprintf(szPlugin, MAX_FILENAME_LEN, "%slicq_%s.so", LIB_DIR, _szName);
  }
  else
  {
    strncpy(szPlugin, _szName, MAX_FILENAME_LEN);
  }
  szPlugin[MAX_FILENAME_LEN - 1] = '\0';

  handle = dlopen (szPlugin, DLOPEN_POLICY);
  if (handle == NULL)
  {
    const char *error = dlerror();
    gLog.Error("%sUnable to load plugin (%s): %s.\n", L_ERRORxSTR, _szName,
     error);

    if (!strstr(error, "No such file"))
    {
      gLog.Warn("%sThis usually happens when your plugin\n"
                "%sis not kept in sync with the daemon.\n"
                "%sPlease try recompiling the plugin.\n"
                "%sIf you are still having problems, see\n"
                "%sthe FAQ at www.licq.org\n",
                L_WARNxSTR, L_BLANKxSTR, L_BLANKxSTR, L_BLANKxSTR,
                L_BLANKxSTR);
    }

    delete p;
    return NULL;
  }

  // LP_Name
  p->fName = (const char * (*)())dlsym(handle, "LP_Name");
  if ((error = dlerror()) != NULL)
  {
    p->fName = (const char * (*)())dlsym(handle, "_LP_Name");
    if ((error = dlerror()) != NULL)
    {
      gLog.Error("%sFailed to find LP_Name() function in plugin (%s): %s\n",
                 L_ERRORxSTR, _szName, error);
      delete p;
      return NULL;
    }
  }
  // LP_Version
  p->fVersion = (const char * (*)())dlsym(handle, "LP_Version");
  if ((error = dlerror()) != NULL)
  {
    p->fVersion = (const char * (*)())dlsym(handle, "_LP_Version");
    if ((error = dlerror()) != NULL)
    {
      gLog.Error("%sFailed to find LP_Version() function in plugin (%s): %s\n",
                 L_ERRORxSTR, p->Name(), error);
      delete p;
      return NULL;
    }
  }
  // LP_Status
  p->fStatus = (const char * (*)())dlsym(handle, "LP_Status");
  if ((error = dlerror()) != NULL)
  {
    p->fStatus = (const char * (*)())dlsym(handle, "_LP_Status");
    if ((error = dlerror()) != NULL)
    {
      gLog.Error("%sFailed to find LP_Status() function in plugin (%s): %s\n",
                 L_ERRORxSTR, p->Name(), error);
      delete p;
      return NULL;
    }
  }
  // LP_Description
  p->fDescription = (const char * (*)())dlsym(handle, "LP_Description");
  if ((error = dlerror()) != NULL)
  {
    p->fDescription = (const char * (*)())dlsym(handle, "_LP_Description");
    if ((error = dlerror()) != NULL)
    {
      gLog.Error("%sFailed to find LP_Description() function in plugin (%s): %s\n",
                 L_ERRORxSTR, p->Name(), error);
      delete p;
      return NULL;
    }
  }
  // LP_BuildDate
  p->fBuildDate = (const char * (*)())dlsym(handle, "LP_BuildDate");
  if ((error = dlerror()) != NULL)
  {
    p->fBuildDate = (const char * (*)())dlsym(handle, "_LP_BuildDate");
    if ((error = dlerror()) != NULL)
    {
      gLog.Error("%sFailed to find LP_BuildDate() function in plugin (%s): %s.\n",
                 L_ERRORxSTR, p->Name(), error);
      delete p;
      return NULL;
    }
  }
  // LP_BuildTime
  p->fBuildTime = (const char * (*)())dlsym(handle, "LP_BuildTime");
  if ((error = dlerror()) != NULL)
  {
    p->fBuildTime = (const char * (*)())dlsym(handle, "_LP_BuildTime");
    if ((error = dlerror()) != NULL)
    {
      gLog.Error("%sFailed to find LP_BuildTime() function in plugin (%s): %s\n",
                 L_ERRORxSTR, p->Name(), error);
      delete p;
      return NULL;
    }
  }
  // LP_Init
  p->fInit = (bool (*)(int, char **))dlsym(handle, "LP_Init");
  if ((error = dlerror()) != NULL)
  {
    p->fInit = (bool (*)(int, char **))dlsym(handle, "_LP_Init");
    if ((error = dlerror()) != NULL)
    {
      gLog.Error("%sFailed to find LP_Init() function in plugin (%s): %s\n",
                 L_ERRORxSTR, p->Name(), error);
      delete p;
      return NULL;
    }
  }
  // LP_Usage
  p->fUsage = (const char * (*)())dlsym(handle, "LP_Usage");
  if ((error = dlerror()) != NULL)
  {
    p->fUsage = (const char * (*)())dlsym(handle, "_LP_Usage");
    if ((error = dlerror()) != NULL)
    {
      gLog.Error("%sFailed to find LP_Usage() function in plugin (%s): %s\n",
                 L_ERRORxSTR, p->Name(), error);
      delete p;
      return NULL;
    }
  }
  // LP_Main
  p->fMain = (int (*)(CICQDaemon *))dlsym(handle, "LP_Main");
  if ((error = dlerror()) != NULL)
  {
    p->fMain = (int (*)(CICQDaemon *))dlsym(handle, "_LP_Main");
    if ((error = dlerror()) != NULL)
    {
      gLog.Error("%sFailed to find LP_Main() function in plugin (%s): %s\n",
                 L_ERRORxSTR, p->Name(), error);
      delete p;
      return NULL;
    }
  }
  // LP_Main_tep
  p->fMain_tep = (void * (*)(void *))dlsym(handle, "LP_Main_tep");
  if ((error = dlerror()) != NULL)
  {
    p->fMain_tep = (void * (*)(void *))dlsym(handle, "_LP_Main_tep");
    if ((error = dlerror()) != NULL)
    {
      gLog.Error("%sFailed to find LP_Main_tep() function in plugin (%s): %s\n",
                 L_ERRORxSTR, p->Name(), error);
      delete p;
      return NULL;
    }
  }
  // LP_ConfigFile
  p->fConfigFile = (const char * (*)())dlsym(handle, "LP_ConfigFile");
  if ((error = dlerror()) != NULL)
  {
    p->fConfigFile = (const char * (*)())dlsym(handle, "_LP_ConfigFile");
    if ((error = dlerror()) != NULL)
      p->fConfigFile = NULL;
  }
  // LP_Id
  p->nId = (unsigned short *)dlsym(handle, "LP_Id");
  if ((error = dlerror()) != NULL)
  {
    p->nId = (unsigned short *)dlsym(handle, "_LP_Id");
    if ((error = dlerror()) != NULL)
    {
      gLog.Error("%sFailed to find LP_Id variable in plugin (%s): %s\n",
                 L_ERRORxSTR, p->Name(), error);
      delete p;
      return NULL;
    }
  }

  // Set up the argument vector
  static int argcndx = 0;
  int argccnt = 0;
  // Step up to the first delimiter if we have done nothing yet
  if (argcndx == 0)
  {
    while (++argcndx < argc && strcmp(argv[argcndx], "--") != 0);
  }
  if (argcndx < argc)
  {
    while (++argcndx < argc && strcmp(argv[argcndx], "--") != 0)
      argccnt++;
  }
  //Setup the argv vector, the plugin as argv[0] ..
  p->localargv = (char **)calloc(sizeof(char *), argccnt + 2);
  p->localargv[0] = argv[0];
  for(int i = argcndx - argccnt; i < argcndx; i++)
  {
    p->localargv[i - argcndx + argccnt + 1] = argv[i];
  }
  p->localargv[argccnt+1] = NULL;
  // Set optind to 0 so plugins can use getopt
  optind = 0;
  p->localargc = argccnt + 1;

  // Init the plugin
  if (!(*p->fInit)(p->localargc, p->localargv))
  {
    gLog.Error("%sFailed to initialize plugin (%s).\n", L_ERRORxSTR, p->Name());
    delete p;
    return NULL;
  }

  *p->nId = m_nNextId++;
  p->dl_handle = handle;
  pthread_mutex_lock(&mutex_plugins);
  list_plugins.push_back(p);
  pthread_mutex_unlock(&mutex_plugins);
  return p;
}


void CLicq::StartPlugin(CPlugin *p)
{
  gLog.Info(tr("%sStarting plugin %s (version %s).\n"), L_INITxSTR, p->Name(),
            p->Version());
  pthread_create( &p->thread_plugin, NULL, p->fMain_tep, licqDaemon);
}

CProtoPlugin *CLicq::LoadProtoPlugin(const char *_szName)
{
  void *handle;
  const char *error = NULL;
  CProtoPlugin *p = new CProtoPlugin(_szName);
  char szFileName[MAX_FILENAME_LEN];

  if (_szName[0] != '/' && _szName[0] != '.')
    snprintf(szFileName, MAX_FILENAME_LEN, "%sprotocol_%s.so", LIB_DIR, _szName);
  else
    snprintf(szFileName, MAX_FILENAME_LEN, "%s", _szName);
  szFileName[MAX_FILENAME_LEN - 1] = '\0';

  handle = dlopen(szFileName, DLOPEN_POLICY);

  if (handle == NULL)
  {
    error = dlerror();
    gLog.Error("%sUnable to load plugin (%s): %s\n", L_ERRORxSTR, _szName,
               error);
    delete p;
    return NULL;
  }

  // Get pointers to the functions
  p->fName = (char *(*)())FindFunction(handle, "LProto_Name");
  if (p->fName == NULL)
  {
    error = dlerror();
    gLog.Error("%sFailed to find LProto_Name in plugin (%s): %s\n",
      L_ERRORxSTR, _szName, error);
    delete p;
    return NULL;
  }

  p->fVersion = (char *(*)())FindFunction(handle, "LProto_Version");
  if (p->fVersion == NULL)
  {
    error = dlerror();
    gLog.Error("%sFailed to find LProto_Version in plugin (%s): %s\n",
      L_ERRORxSTR, _szName, error);
    delete p;
    return NULL;
  }

  p->nId = (unsigned short *)FindFunction(handle, "LP_Id");
  if (p->nId == NULL)
  {
    error = dlerror();
    gLog.Error("%sFailed to find LProto_Id in plugin (%s): %s\n",
      L_ERRORxSTR, _szName, error);
    delete p;
    return NULL;
  }

  p->fPPID = (char *(*)())FindFunction(handle, "LProto_PPID");
  if (p->fPPID == NULL)
  {
    error = dlerror();
    gLog.Error("%sFailed to find LProto_PPID in plugin (%s): %s\n",
      L_ERRORxSTR, _szName, error);
    delete p;
    return NULL;  
  }
  
  p->fInit = (bool (*)())FindFunction(handle, "LProto_Init");
  if (p->fInit == NULL)
  {
    error = dlerror();
    gLog.Error("%sFailed to find LProto_Init in plugin (%s): %s\n",
      L_ERRORxSTR, _szName, error);
    delete p;
    return NULL;
  }

  p->fMain = (void (*)(CICQDaemon *))FindFunction(handle, "LProto_Main");
  if (p->fMain == NULL)
  {
    error = dlerror();
    gLog.Error("%sFailed to find LProto_Main in plugin (%s): %s\n",
      L_ERRORxSTR, _szName, error);
    delete p;
    return NULL;
  }

  p->fMain_tep = (void *(*)(void *))FindFunction(handle, "LProto_Main_tep");
  if (p->fMain_tep == NULL)
  {
    error = dlerror();
    gLog.Error("%sFailed to find LProto_Main_tep in plugin (%s): %s\n",
      L_ERRORxSTR, _szName, error);
    delete p;
    return NULL;
  }

  if (!(*p->fInit)())
  {
    gLog.Error("%sFailed to initialize plugin (%s).\n", L_ERRORxSTR, p->Name());
    delete p;
    return NULL;
  }

  // PPID
  p->m_nPPID = p->fPPID()[0] << 24 | p->fPPID()[1] << 16 | p->fPPID()[2] << 8 | p->fPPID()[3];

  // Finish it up
  *p->nId = m_nNextId++;
  p->m_pHandle = handle;
  pthread_mutex_lock(&mutex_protoplugins);
  list_protoplugins.push_back(p);
  pthread_mutex_unlock(&mutex_protoplugins);

  // Let the gui plugins know about the new protocol plugin
  if (licqDaemon)
    licqDaemon->PushPluginSignal(new CICQSignal(SIGNAL_NEWxPROTO_PLUGIN,p->PPID(),
                                                0));
  return p;
}

void CLicq::StartProtoPlugin(CProtoPlugin *p)
{
  gLog.Info(tr("%sStarting protocol plugin %s (version %s).\n"), L_INITxSTR, p->Name(),
            p->Version());
  pthread_create(&p->thread_plugin, NULL, p->fMain_tep, licqDaemon);
}


void *CLicq::FindFunction(void *_pHandle, const char *_szSymbolName)
{
  void *pFunc = dlsym(_pHandle, _szSymbolName);
  if (pFunc == NULL)
  {
    char *szSymbol = new char[strlen(_szSymbolName) + 2];
    sprintf(szSymbol, "_%s", _szSymbolName);
    szSymbol[strlen(_szSymbolName)+1] = '\0';

    pFunc = dlsym(_pHandle, szSymbol);
    delete [] szSymbol;
  }

  return pFunc;
}

int CLicq::Main()
{
  int nResult = 0;

  if (list_plugins.size() == 0)
  {
    gLog.Warn(tr("%sNo plugins specified on the command-line (-p option).\n"
                 "%sSee the README for more information.\n"),
              L_WARNxSTR, L_BLANKxSTR);
    return nResult;
  }

  if (!licqDaemon->Start()) return 1;

  // Run the plugins
  pthread_cond_init(&LP_IdSignal, NULL);
  PluginsListIter iter;
  ProtoPluginsListIter p_iter;
  pthread_mutex_lock(&mutex_plugins);
  pthread_mutex_lock(&mutex_protoplugins);
  for (iter = list_plugins.begin(); iter != list_plugins.end(); iter++)
  {
    StartPlugin(*iter);
  }

  gLog.ModifyService(S_STDERR, DEBUG_LEVEL);

  unsigned short nExitId;
  int *nPluginResult;
  bool bDaemonShutdown = false;
  
  //FIXME ICQ Plugin can't be taken out really
  while (list_plugins.size() > 0 || list_protoplugins.size() > 1)
  {
    bool bUIPlugin = true;
    pthread_mutex_lock(&LP_IdMutex);
    pthread_mutex_unlock(&mutex_protoplugins);
    pthread_mutex_unlock(&mutex_plugins);
    while (LP_Ids.size() == 0)
    {
      if (bDaemonShutdown)
      {
        struct timespec abstime;
        abstime.tv_sec = time(TIME_NOW) + MAX_WAIT_PLUGIN;
        abstime.tv_nsec = 0;
        if (pthread_cond_timedwait(&LP_IdSignal, &LP_IdMutex, &abstime) == ETIMEDOUT)
        {
          pthread_mutex_lock(&mutex_plugins);
          pthread_mutex_lock(&mutex_protoplugins);
          pthread_mutex_unlock(&LP_IdMutex);
          goto timed_out;
        }
      }
      else
        pthread_cond_wait(&LP_IdSignal, &LP_IdMutex);
    }
    nExitId = LP_Ids.front();
    LP_Ids.pop_front();

    pthread_mutex_lock(&mutex_plugins);
    pthread_mutex_lock(&mutex_protoplugins);
    pthread_mutex_unlock(&LP_IdMutex);

    if (nExitId == 0)
    {
      bDaemonShutdown = true;
      continue;
    }

    // Check UI plugins first
    for (iter = list_plugins.begin(); iter != list_plugins.end(); iter++)
    {
      if (*(*iter)->nId == nExitId)
      {
        bUIPlugin = true;
        break;
      }
    }
    
    for (p_iter = list_protoplugins.begin(); p_iter != list_protoplugins.end();
         p_iter++)
    {
      if ((*p_iter)->PPID() != LICQ_PPID && *(*p_iter)->nId == nExitId)
      {
        bUIPlugin = false;
        break;
      }
    }
        
    if (iter == list_plugins.end() && p_iter == list_protoplugins.end())
    {
      gLog.Error("%sInvalid plugin id (%d) in exit signal.\n", L_ERRORxSTR, nExitId);
      continue;
    }

    if (bUIPlugin)
    {
      pthread_join((*iter)->thread_plugin, (void **)&nPluginResult);
      gLog.Info(tr("%sPlugin %s exited with code %d.\n"), L_ENDxSTR, (*iter)->Name(), *nPluginResult);
      free (nPluginResult);
      // We should close the dynamic link but under linux this makes Qt crash
      //dlclose((*iter)->dl_handle);
      delete *iter;
      list_plugins.erase(iter);
    }
    else
    {
      //FIXME
      if ((*p_iter)->PPID() != LICQ_PPID)
      {
        pthread_join((*p_iter)->thread_plugin, (void **)&nPluginResult);
        gLog.Info(tr("%sPlugin %s exited with code %d.\n"), L_ENDxSTR, (*p_iter)->Name(), *nPluginResult);
        free (nPluginResult);
        //dlclose((*p_iter)->m_pHandle);
        delete *p_iter;
        list_protoplugins.erase(p_iter);
      }
    }
  }

  timed_out:

  for (iter = list_plugins.begin(); iter != list_plugins.end(); iter++)
  {
    gLog.Info(tr("%sPlugin %s failed to exit.\n"), L_WARNxSTR, (*iter)->Name());
    pthread_cancel( (*iter)->thread_plugin);
  }
  pthread_mutex_unlock(&mutex_plugins);

  for (p_iter = list_protoplugins.begin(); p_iter != list_protoplugins.end();
       p_iter++)
  {
    if ((*p_iter)->PPID() == LICQ_PPID) //FIXME
    {
      delete *p_iter;
    }
    else
    {
      gLog.Info(tr("%sPlugin %s failed to exit.\n"), L_WARNxSTR, (*p_iter)->Name());
      pthread_cancel((*p_iter)->thread_plugin);
    }
  }
  pthread_mutex_unlock(&mutex_protoplugins);

  pthread_t *t = licqDaemon->Shutdown();
  pthread_join(*t, NULL);

  // Remove the pid flag
  char sz[MAX_FILENAME_LEN];
  snprintf(sz, MAX_FILENAME_LEN, "%s/licq.pid", BASE_DIR);
  sz[MAX_FILENAME_LEN - 1] = '\0';
  remove(sz);

  return list_plugins.size();
}


void CLicq::PrintUsage()
{
  printf(tr("%s version %s.\n"
         "Usage:  Licq [-h] [-d #] [-b configdir] [-I] [-p plugin] [-o file] [ -- <plugin #1 parameters>] [-- <plugin #2 parameters>...]\n\n"
         " -h : this help screen (and any plugin help screens as well)\n"
         " -d : set what information is logged to standard output:\n"
         "        1  status information\n"
         "        2  unknown packets\n"
         "        4  errors\n"
         "        8  warnings\n"
         "       16  all packets\n"
         "      add values together for multiple options\n"
         " -c : disable color at standard output\n"
         " -b : set the base directory for the config and data files (~/.licq by default)\n"
         " -I : force initialization of the given base directory\n"
         " -p : load the given plugin library\n"
         " -o : redirect stderr to <file>, which can be a device (ie /dev/ttyp4)\n"),
         PACKAGE, VERSION);
}


void CLicq::SaveLoadedPlugins()
{
  char szConf[MAX_FILENAME_LEN];
  char szKey[20];

  CIniFile licqConf(INI_FxWARN | INI_FxALLOWxCREATE);
  sprintf(szConf, "%s/licq.conf", BASE_DIR);
  licqConf.LoadFile(szConf);

  licqConf.SetSection("plugins");
  licqConf.WriteNum("NumPlugins", (unsigned short)list_plugins.size());
  PluginsListIter iter;
  unsigned short i = 1;
  for (iter = list_plugins.begin(); iter != list_plugins.end(); iter++)
  {
    sprintf(szKey, "Plugin%d", i++);
    licqConf.WriteStr(szKey, (*iter)->LibName());
  }
  
  licqConf.WriteNum("NumProtoPlugins", (unsigned short)(list_protoplugins.size() - 1));
  ProtoPluginsListIter it;
  i = 1;
  for (it = list_protoplugins.begin(); it != list_protoplugins.end(); it++)
  {
    if (strcmp((*it)->LibName(), "") != 0)
    {
      sprintf(szKey, "ProtoPlugin%d", i++);
      licqConf.WriteStr(szKey, (*it)->LibName());
    }
  }

  licqConf.FlushFile();
}


void CLicq::ShutdownPlugins()
{
  // Save plugins
  if (list_plugins.size() > 0)
    SaveLoadedPlugins();

  // Send shutdown signal to all the plugins
  PluginsListIter iter;
  pthread_mutex_lock(&mutex_plugins);
  for (iter = list_plugins.begin(); iter != list_plugins.end(); iter++)
  {
    (*iter)->Shutdown();
  }
  pthread_mutex_unlock(&mutex_plugins);
  
  ProtoPluginsListIter p_iter;
  pthread_mutex_lock(&mutex_protoplugins);
  for (p_iter = list_protoplugins.begin(); p_iter != list_protoplugins.end(); p_iter++)
  {
    (*p_iter)->Shutdown();
  }
  pthread_mutex_unlock(&mutex_protoplugins);
}


bool CLicq::Install()
{
  char cmd[MAX_FILENAME_LEN + 128];

  cmd[sizeof(cmd) - 1] = '\0';

  // Create the directory if necessary
  if (mkdir(BASE_DIR, 0700) == -1 && errno != EEXIST)
  {
    fprintf(stderr, "Couldn't mkdir %s: %s\n", BASE_DIR, strerror(errno));
    return (false);
  }
  snprintf(cmd, sizeof(cmd) - 1, "%s/%s", BASE_DIR, HISTORY_DIR);
  if (mkdir(cmd, 0700) == -1 && errno != EEXIST)
  {
    fprintf(stderr, "Couldn't mkdir %s: %s\n", cmd, strerror(errno));
    return (false);
  }
  snprintf(cmd, sizeof(cmd) - 1, "%s/%s", BASE_DIR, USER_DIR);
  if (mkdir(cmd, 0700) == -1 && errno != EEXIST)
  {
    fprintf(stderr, "Couldn't mkdir %s: %s\n", cmd, strerror(errno));
    return (false);
  }

  // Create licq.conf
  snprintf(cmd, sizeof(cmd) - 1, "%s/licq.conf", BASE_DIR);
  FILE *f = fopen(cmd, "w");
  chmod(cmd, 00600);
  fprintf(f, "%s", LICQ_CONF);
  fclose(f);


  // Create users.conf
  snprintf(cmd, sizeof(cmd) - 1, "%s/users.conf", BASE_DIR);
  CIniFile usersConf(INI_FxALLOWxCREATE);
  usersConf.LoadFile(cmd);
  usersConf.SetSection("users");
  usersConf.WriteNum("NumOfUsers", 0ul);
  usersConf.FlushFile();

  snprintf (cmd, sizeof(cmd) - 1, "%s/owner.Licq", BASE_DIR);
  CIniFile licqConf(INI_FxALLOWxCREATE);
  licqConf.LoadFile(cmd);
  licqConf.SetSection("user");
  licqConf.WriteStr("Alias", "None");
  licqConf.WriteStr("Password", "");
  licqConf.WriteNum("Uin", 0ul);
  licqConf.WriteBool("WebPresence", false);
  licqConf.WriteBool("HideIP", false);
  licqConf.FlushFile();

  return(true);
}
