#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include "plugin.h"
#include "email.h"

CLicqEmail *licqEmail;

void LP_Usage(void)
{
  fprintf(stderr, "Licq Plugin: %s %s\n", LP_Name(), LP_Version());
  fprintf(stderr, "Usage:  Licq [options] -p email -- [ -h ] [ -e ] [ -l <staus> ]\n"
                  "         -h          : help\n"
                  "         -e          : start enabled\n"
                  "         -l <status> : log on at startup\n\n");
}

const char *LP_Name(void)
{
  static const char name[] = "Email Forwarder";
  return name;
}


const char *LP_Description(void)
{
  static const char desc[] = "ICQ to Email forwarder";
  return desc;
}


const char *LP_Version(void)
{
  static const char version[] = "0.01";
  return version;
}

const char *LP_Status(void)
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


