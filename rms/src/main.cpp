#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include "licq_plugin.h"
#include "rms.h"


const char *LP_Usage()
{
  static const char usage[] =
    "Usage:  Licq [options] -p rms -- [ -h ] [ -d ]\n"
    "         -h          : help\n"
    "         -d          : start disabled\n";
  return usage;
}

const char *LP_Name()
{
  static const char name[] = "ICQ Remote Management Server";
  return name;
}


const char *LP_Description()
{
  static const char desc[] = "ICQ remote management server for telnet ICQ access.";
  return desc;
}


const char *LP_Version()
{
  static const char version[] = VERSION;
  return version;
}


const char *LP_ConfigFile()
{
  return "licq_rms.conf";
}


const char *LP_Status()
{
  static const char enabled[] = "RMS  enabled";
  static const char disabled[] = "RMS disabled";
  return licqRMS->Enabled() ? enabled : disabled;
}


bool LP_Init(int argc, char **argv)
{
  //char *LocaleVal = new char;
  //LocaleVal = setlocale (LC_ALL, "");
  //bindtextdomain (PACKAGE, LOCALEDIR);
  //textdomain (PACKAGE);

  // parse command line for arguments
  bool bEnable = true;
  unsigned short nPort = 0;
  int i = 0;
  while( (i = getopt(argc, argv, "hdp:")) > 0)
  {
    switch (i)
    {
    case 'h':  // help
      puts(LP_Usage());
      return false;
    case 'd': // enable
      bEnable = false;
      break;
    case 'p':
      nPort = atol(optarg);
      break;
    }
  }
  licqRMS = new CLicqRMS(bEnable, nPort);
  return true;
}


int LP_Main(CICQDaemon *_licqDaemon)
{
  int nResult = licqRMS->Run(_licqDaemon);
  licqRMS->Shutdown();
  delete licqRMS;
  return nResult;
}


