#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include "plugin.h"
#include "email.h"

CLicqEmail *licqEmail;

const char *LP_Usage()
{
  static const char usage[] =
    "Usage:  Licq [options] -p email -- [ -h ] [ -e ] [ -l <staus> ]\n"
    "         -h          : help\n"
    "         -e          : start enabled\n"
    "         -l <status> : log on at startup\n";
  return usage;
}

const char *LP_Name()
{
  static const char name[] = "Email Forwarder";
  return name;
}


const char *LP_Description()
{
  static const char desc[] = "ICQ to Email forwarder";
  return desc;
}


const char *LP_Version()
{
  static const char version[] = "0.01";
  return version;
}

const char *LP_Status()
{
  static const char enabled[] = "forwarding enabled";
  static const char disabled[] = "forwarding disabled";
  return licqEmail->Enabled() ? enabled : disabled;
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
  licqEmail = new CLicqEmail(bEnable, szStatus);
  if (szStatus != NULL) free(szStatus);
  return (licqEmail != NULL);
}


int LP_Main(CICQDaemon *_licqDaemon)
{
  int nResult = licqEmail->Run(_licqDaemon);
  licqEmail->Shutdown();
  delete licqEmail;
  return nResult;
}


