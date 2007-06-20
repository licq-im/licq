#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include "licq_plugin.h"
#include "forwarder.h"

CLicqForwarder *licqForwarder;

const char *LP_Usage()
{
  static const char usage[] =
    "Usage:  Licq [options] -p forwarder -- [ -h ] [ -e ] [ -l <status> ] [ -d ]\n"
    "         -h          : help\n"
    "         -e          : start enabled\n"
    "         -l <status> : log on at startup\n"
    "         -d          : delete new messages after forwarding\n";
  return usage;
}

const char *LP_Name()
{
  static const char name[] = "ICQ Forwarder";
  return name;
}


const char *LP_Description()
{
  static const char desc[] = "ICQ message forwarder to email/icq";
  return desc;
}


const char *LP_Version()
{
  static const char version[] = "1.3.0";
  return version;
}

const char *LP_Status()
{
  static const char enabled[] = "forwarding enabled";
  static const char disabled[] = "forwarding disabled";
  return licqForwarder->Enabled() ? enabled : disabled;
}

const char *LP_ConfigFile()
{
  return "licq_forwarder.conf";
}


bool LP_Init(int argc, char **argv)
{
  //char *LocaleVal = new char;
  //LocaleVal = setlocale (LC_ALL, "");
  //bindtextdomain (PACKAGE, LOCALEDIR);
  //textdomain (PACKAGE);

  // parse command line for arguments
  bool bEnable = false, bDelete = false;
  char *szStatus = NULL;
  int i = 0;
  while( (i = getopt(argc, argv, "hel:d")) > 0)
  {
    switch (i)
    {
    case 'h':  // help
      puts(LP_Usage());
      return false;
    case 'e': // enable
      bEnable = true;
      break;
    case 'l': //log on
      szStatus = strdup(optarg);
      break;
    case 'd':
      bDelete = true;
      break;
    }
  }
  licqForwarder = new CLicqForwarder(bEnable, bDelete, szStatus);
  if (szStatus != NULL) free(szStatus);
  return (licqForwarder != NULL);
}


int LP_Main(CICQDaemon *_licqDaemon)
{
  int nResult = licqForwarder->Run(_licqDaemon);
  licqForwarder->Shutdown();
  delete licqForwarder;
  return nResult;
}


