#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include "console.h"
#include "plugin.h"

CLicqConsole *licqConsole;

void LP_Usage(void)
{
  fprintf(stderr, "Licq Plugin: %s %s\n", LP_Name(), LP_Version());
  fprintf(stderr, "Usage:  Licq [options] -p console\n\n");
}

const char *LP_Name(void)
{
  static const char name[] = "Console";
  return name;
}


const char *LP_Version(void)
{
  static const char version[] = "0.01";
  return version;
}

bool LP_Init(int argc, char **argv)
{
  //char *LocaleVal = new char;
  //LocaleVal = setlocale (LC_ALL, "");
  //bindtextdomain (PACKAGE, LOCALEDIR);
  //textdomain (PACKAGE);


  // parse command line for arguments
  int i = 0;
  while( (i = getopt(argc, argv, "h")) > 0)
  {
    switch (i)
    {
    case 'h':  // help
      LP_Usage();
      return false;
    }
  }
  licqConsole = new CLicqConsole(argc, argv);
  return (licqConsole != NULL);
}


int LP_Main(CICQDaemon *_licqDaemon)
{
  int nResult = licqConsole->Run(_licqDaemon);
  licqConsole->Shutdown();
  delete licqConsole;
  return nResult;
}


