#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include "plugin.h"
#include "autoreply.h"

CLicqAutoReply *licqAutoReply;

const char *LP_Usage()
{
  static const char usage[] =
    "Usage:  Licq [options] -p autoreply -- [ -h ] [ -e ] [ -l <staus> ]\n"
    "         -h          : help\n"
    "         -e          : start enabled\n"
    "         -l <status> : log on at startup\n";
  return usage;
}

const char *LP_Name()
{
  static const char name[] = "ICQ Auto Replyer";
  return name;
}


const char *LP_Description()
{
  static const char desc[] = "ICQ message Auto Replyer";
  return desc;
}


const char *LP_Version()
{
  static const char version[] = "0.10";
  return version;
}

const char *LP_Status()
{
  static const char enabled[] = "autoreply enabled";
  static const char disabled[] = "autoreply disabled";
  return licqAutoReply->Enabled() ? enabled : disabled;
}


bool LP_Init(int argc, char **argv)
{
  //char *LocaleVal = new char;
  //LocaleVal = setlocale (LC_ALL, "");
  //bindtextdomain (PACKAGE, LOCALEDIR);
  //textdomain (PACKAGE);

  // parse command line for arguments
  bool bEnable = false;
  char *szStatus = NULL;
  int i = 0;
  while( (i = getopt(argc, argv, "hel:")) > 0)
  {
    switch (i)
    {
    case 'h':  // help
      LP_Usage();
      return false;
    case 'e': // enable
      bEnable = true;
      break;
    case 'l': //log on
      szStatus = strdup(optarg);
      break;
    }
  }
  licqAutoReply = new CLicqAutoReply(bEnable, szStatus);
  if (szStatus != NULL) free(szStatus);
  return (licqAutoReply != NULL);
}


int LP_Main(CICQDaemon *_licqDaemon)
{
  int nResult = licqAutoReply->Run(_licqDaemon);
  licqAutoReply->Shutdown();
  delete licqAutoReply;
  return nResult;
}


