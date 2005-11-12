#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include <unistd.h>
#include "console.h"
#include "licq_plugin.h"

CLicqConsole *licqConsole;

const char *LP_Usage()
{
  static const char usage[] =
      "Usage:  Licq [ options ] -p console\n";
  return usage;
}

const char *LP_Name()
{
  static const char name[] = "Console";
  return name;
}


const char *LP_Version()
{
  static const char version[] = VERSION;
  return version;
}

const char *LP_Status()
{
  static const char status[] = "running";
  return status;
}

const char *LP_Description()
{
  static const char desc[] = "Console plugin based on ncurses";
  return desc;
}

const char *LP_ConfigFile()
{
  return "licq_console.conf";
}

bool LP_Init(int argc, char **argv)
{
  //char *LocaleVal = new char;
  //LocaleVal = setlocale (LC_ALL, "");
  //bindtextdomain (PACKAGE, LOCALEDIR);
  //textdomain (PACKAGE);
  setlocale(LC_ALL, "");

  // parse command line for arguments
  int i = 0;
  while( (i = getopt(argc, argv, "h")) > 0)
  {
    switch (i)
    {
    case 'h':  // help
      puts(LP_Usage());
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


