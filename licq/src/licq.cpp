#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <dlfcn.h>

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

#include "licq_icq.h"
#include "licq.h"
#include "licq_log.h"
#include "licq_countrycodes.h"
#include "licq_utility.h"
#include "support.h"
#include "licq_sar.h"
#include "licq_user.h"
#include "licq_icqd.h"

#include "licq.conf.h"

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
  pthread_mutex_init(&mutex_pluginfunctions, NULL);
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
    fprintf(stderr, "Session management is not supported by Licq.\n");
  }
  else
  {
    int i = 0;
    while( (i = getopt(argc, argv, "hd:b:p:Io:fc")) > 0)
    {
      switch (i)
      {
      case 'h':  // help
        PrintUsage();
        bHelp = true;
        break;
      case 'b':  // base directory
        sprintf(BASE_DIR, "%s", optarg);
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
      case 'o':  // redirect stdout and stderr
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

  if(!isatty(STDOUT_FILENO))
    bUseColor = false;

  // Initialise the log server for standard output and dump all initial errors
  // and warnings to it regardless of DEBUG_LEVEL
  gLog.AddService(new CLogService_StdOut(DEBUG_LEVEL | L_ERROR | L_WARN, bUseColor));

  // Redirect stdout and stderr if asked to
  if (szRedirect) {
    if (bRedirect_ok)
      gLog.Info("%sOutput redirected to \"%s\".\n", L_INITxSTR, szRedirect);
    else
      gLog.Warn("%sRedirection to \"%s\" failed:\n%s%s.\n", L_WARNxSTR,
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
     sprintf(BASE_DIR, "%s/.licq", home);
  }

  // check if user has conf files installed, install them if not
  if ( (access(BASE_DIR, F_OK) < 0 || bForceInit) && !Install() )
    return false;

  // Define the directory for all the shared data
  sprintf(SHARE_DIR, "%s/%s", INSTALL_PREFIX, BASE_SHARE_DIR);
  sprintf(LIB_DIR, "%s/%s", INSTALL_PREFIX, BASE_LIB_DIR);

  // Check pid
  char szConf[MAX_FILENAME_LEN], szKey[32];
  sprintf(szConf, "%s/licq.pid", BASE_DIR);
  FILE *fs = fopen(szConf, "r");
  if (fs != NULL)
  {
    fgets(szKey, 32, fs);
    pid_t pid = atol(szKey);
    if (pid != 0)
    {
      gLog.Error("%sLicq: Already running at pid %d.\n"
                 "%s      Kill process or remove %s.\n",
       L_ERRORxSTR, pid, L_BLANKxSTR, szConf);
      return false;
    }
    fclose(fs);
  }
  fs = fopen(szConf, "w");
  fprintf(fs, "%d\n", getpid());
  fclose(fs);

  // Open the config file
  CIniFile licqConf(INI_FxWARN | INI_FxALLOWxCREATE);
  sprintf(szConf, "%s/licq.conf", BASE_DIR);
  licqConf.LoadFile(szConf);

  // Verify the version
  licqConf.SetSection("licq");
  unsigned short nVersion;
  licqConf.ReadNum("Version", nVersion, 0);
  if (nVersion < 710)
  {
    fprintf(stderr, "Previous Licq config files detected.\n"
                    "Manual upgrade is necessary.  Follow the instructions\n"
                    "in the UPGRADE file included with the source tree or\n"
                    "in /usr/doc/licq-xxx/upgrade.\n");
    return false;
  }
  if (nVersion < INT_VERSION)
  {
    licqConf.WriteNum("Version", (unsigned short)INT_VERSION);
    licqConf.FlushFile();
  }

  // Save the plugins if necessary
  if (bCmdLinePlugins)
  {
    licqConf.SetSection("plugins");
    licqConf.WriteNum("NumPlugins", (unsigned short)vszPlugins.size());
    vector <char *>::iterator iter;
    unsigned short i = 1;
    for (iter = vszPlugins.begin(); iter != vszPlugins.end(); iter++)
    {
      sprintf(szKey, "Plugin%d", i++);
      licqConf.WriteStr(szKey, *iter);
    }
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
          m_vPluginFunctions.back()->Name(),
          m_vPluginFunctions.back()->Version(),
          (*(m_vPluginFunctions.back())->fUsage)() );
      m_vPluginFunctions.pop_back();
    }
  }
  if (bHelp) return false;

  // Find and load the plugins from the conf file
  if (!bHelp && !bCmdLinePlugins)
  {
    unsigned short nNumPlugins = 0;
    char szData[MAX_FILENAME_LEN];
    if (licqConf.SetSection("plugins") && licqConf.ReadNum("NumPlugins", nNumPlugins))
    {
      for (int i = 0; i < nNumPlugins; i++)
      {
        sprintf(szKey, "Plugin%d", i + 1);
        if (!licqConf.ReadStr(szKey, szData)) continue;
        if (LoadPlugin(szData, argc, argv) == NULL) return false;
      }
    }
  }

  // Close the conf file
  licqConf.CloseFile();

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
}


const char *CLicq::Version()
{
  static const char version[] = VERSION;
  return version;
}


/*-----------------------------------------------------------------------------
 * LoadPlugin
 *
 * Loads the given plugin using the given command line arguments.
 *---------------------------------------------------------------------------*/
CPluginFunctions *CLicq::LoadPlugin(const char *_szName, int argc, char **argv)
{
  void *handle;
  const char *error;
  CPluginFunctions *p = new CPluginFunctions;
  char szPlugin[MAX_FILENAME_LEN];

  // First check if the plugin is in the shared location
  if ( _szName[0] != '/' && _szName[0] != '.')
  {
    sprintf(szPlugin, "%slicq_%s.so", LIB_DIR, _szName);
  }
  else
    strcpy(szPlugin, _szName);
  handle = dlopen (szPlugin, DLOPEN_POLICY);
  if (handle == NULL)
  {
    gLog.Error("%sUnable to load plugin (%s): %s.\n ", L_ERRORxSTR, _szName, dlerror());
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
  p->localargv = (char **)calloc(sizeof(char *), argccnt + 1);
  p->localargv[0] = argv[0];
  for(int i = argcndx - argccnt; i < argcndx; i++)
  {
    p->localargv[i - argcndx + argccnt + 1] = argv[i];
  }
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
  pthread_mutex_lock(&mutex_pluginfunctions);
  m_vPluginFunctions.push_back(p);
  pthread_mutex_unlock(&mutex_pluginfunctions);
  return p;
}


void CLicq::StartPlugin(CPluginFunctions *p)
{
  gLog.Info("%sStarting plugin %s (version %s).\n", L_INITxSTR, p->Name(),
            p->Version());
  pthread_create( &p->thread_plugin, NULL, p->fMain_tep, licqDaemon);
}


int CLicq::Main()
{
  int nResult = 0;

  if (m_vPluginFunctions.size() == 0)
  {
    gLog.Warn("%sNo plugins specified on the command-line (-p option).\n%sSee the README for more information.\n",
              L_WARNxSTR, L_BLANKxSTR);
    return nResult;
  }

  if (!licqDaemon->Start()) return 1;

  // Run the plugins
  pthread_cond_init(&LP_IdSignal, NULL);
  PluginsListIter iter;
  pthread_mutex_lock(&mutex_pluginfunctions);
  for (iter = m_vPluginFunctions.begin(); iter != m_vPluginFunctions.end(); iter++)
  {
    StartPlugin(*iter);
  }

  gLog.ModifyService(S_STDOUT, DEBUG_LEVEL);

  unsigned short nExitId;
  int *nPluginResult;
  bool bDaemonShutdown = false;

  while (m_vPluginFunctions.size() > 0)
  {
    pthread_mutex_lock(&LP_IdMutex);
    pthread_mutex_unlock(&mutex_pluginfunctions);
    while (LP_Ids.size() == 0)
    {
      if (bDaemonShutdown)
      {
        struct timespec abstime;
        abstime.tv_sec = time(TIME_NOW) + MAX_WAIT_PLUGIN;
        abstime.tv_nsec = 0;
        if (pthread_cond_timedwait(&LP_IdSignal, &LP_IdMutex, &abstime) == ETIMEDOUT)
          break;
      }
      else
        pthread_cond_wait(&LP_IdSignal, &LP_IdMutex);
    }
    nExitId = LP_Ids.front();
    LP_Ids.pop_front();

    pthread_mutex_lock(&mutex_pluginfunctions);
    pthread_mutex_unlock(&LP_IdMutex);

    if (nExitId == 0)
    {
      bDaemonShutdown = true;
      continue;
    }

    for (iter = m_vPluginFunctions.begin(); iter != m_vPluginFunctions.end(); iter++)
      if (*(*iter)->nId == nExitId) break;

    if (iter == m_vPluginFunctions.end())
    {
      gLog.Error("%sInvalid plugin id (%d) in exit signal.\n", L_ERRORxSTR, nExitId);
      continue;
    }

    pthread_join((*iter)->thread_plugin, (void **)&nPluginResult);
    gLog.Info("%sPlugin %s exited with code %d.\n", L_ENDxSTR, (*iter)->Name(), *nPluginResult);
    free (nPluginResult);
    // We should close the dynamic link but under linux this makes Qt crash
    //dlclose((*iter).dl_handle);
    m_vPluginFunctions.erase(iter);
  }

  for (iter = m_vPluginFunctions.begin(); iter != m_vPluginFunctions.end(); iter++)
  {
    gLog.Info("%sPlugin %s failed to exit.\n", L_WARNxSTR, (*iter)->Name());
    pthread_cancel( (*iter)->thread_plugin);
  }
  //pthread_mutex_unlock(&mutex_pluginfunctions);

  pthread_t *t = licqDaemon->Shutdown();
  pthread_join(*t, NULL);

  // Remove the pid flag
  char sz[MAX_FILENAME_LEN];
  sprintf(sz, "%s/licq.pid", BASE_DIR);
  remove(sz);

  return m_vPluginFunctions.size();
}


void CLicq::PrintUsage()
{
  printf("%s version %s.\n"
         "Usage:  Licq [-h] [-d #] [-b configdir] [-i] [-p plugin] [-o file] [ -- <plugin #1 parameters>] [-- <plugin #2 parameters>...]\n\n"
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
         " -o : redirect stdout and stderr to <file>, which can be a device (ie /dev/ttyp4)\n",
         PACKAGE, VERSION);
}


bool CLicq::Install()
{
  char cmd[MAX_FILENAME_LEN + 128];

  // Create the directory if necessary
  if (mkdir(BASE_DIR, 0700) == -1 && errno != EEXIST)
  {
    printf("Couldn't mkdir %s: %s\n", BASE_DIR, strerror(errno));
    return (false);
  }
  sprintf(cmd, "%s/%s", BASE_DIR, HISTORY_DIR);
  if (mkdir(cmd, 0700) == -1 && errno != EEXIST)
  {
    printf("Couldn't mkdir %s: %s\n", cmd, strerror(errno));
    return (false);
  }
  sprintf(cmd, "%s/%s", BASE_DIR, USER_DIR);
  if (mkdir(cmd, 0700) == -1 && errno != EEXIST)
  {
    printf("Couldn't mkdir %s: %s\n", cmd, strerror(errno));
    return (false);
  }

  // Create licq.conf
  sprintf(cmd, "%s/licq.conf", BASE_DIR);
  FILE *f = fopen(cmd, "w");
  fprintf(f, "%s", LICQ_CONF);
  fclose(f);


  // Create users.conf
  sprintf(cmd, "%s/users.conf", BASE_DIR);
  CIniFile usersConf(INI_FxALLOWxCREATE);
  usersConf.LoadFile(cmd);
  usersConf.SetSection("users");
  usersConf.WriteNum("NumOfUsers", 0ul);
  usersConf.FlushFile();

  sprintf (cmd, "%s/owner.uin", BASE_DIR);
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
