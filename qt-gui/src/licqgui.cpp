#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "licqgui.h"
#include "sigman.h"
#include "outputwin.h"
#include "log.h"
#include "plugin.h"
#include "licq-locale.h"
#ifdef USE_KDE
#include <kthemestyle.h>
#endif
#include <qwindowsstyle.h>
#include <qmotifstyle.h>
#include <qplatinumstyle.h>
#include <qcdestyle.h>

#include "icqd.h"

CLicqGui *licqQtGui;

void LP_Usage(void)
{
  fprintf(stderr, "Licq Plugin: %s %s\n", LP_Name(), LP_Version());
  fprintf(stderr, "Usage:  Licq [options] -p qt-gui -- [-h] [-s skinname] [-i iconpack] [-g gui style]\n");
  fprintf(stderr, " -h : this help screen\n");
  fprintf(stderr, " -s : set the skin to use (must be in {base dir}/qt-gui/skin.skinname)\n");
  fprintf(stderr, " -i : set the icons to use (must be in {base dir}/qt-gui/icons.iconpack)\n");
  fprintf(stderr, " -g : set the gui style (MOTIF / WINDOWS / MAC / CDE)\n\n");
}

const char *LP_Name(void)
{
#ifdef USE_KDE
  static const char name[] = "KDE GUI";
#else
  static const char name[] = "Qt GUI";
#endif
  return name;
}


const char *LP_Version(void)
{
  static const char version[] = VERSION;
  return version;
}

bool LP_Init(int argc, char **argv)
{
  char skinName[32] = "";
  char iconsName[32] = "";
  char styleName[32] = "";
  char *LocaleVal = new char;

  LocaleVal = setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  // parse command line for arguments
  int i = 0;
  while( (i = getopt(argc, argv, "hs:i:g:")) > 0)
  {
    switch (i)
    {
    case 'h':  // help
      LP_Usage();
      return false;
    case 's':  // skin name
      sprintf(skinName, "%s", optarg);
      break;
    case 'i':  // icons name
      sprintf(iconsName, "%s", optarg);
      break;
    case 'g': // gui style
      strcpy(styleName, optarg);
      break;
    }
  }
  if (qApp != NULL)
  {
    gLog.Error("%sA Qt application is already loaded.\n%sRemove the plugin from the command line.\n", L_ERRORxSTR, L_BLANKxSTR);
    return false;
  }
  licqQtGui = new CLicqGui(argc, argv, skinName, iconsName, styleName);
  return (licqQtGui != NULL);
}


int LP_Main(CICQDaemon *_licqDaemon)
{
  int nResult = licqQtGui->Run(_licqDaemon);
  licqQtGui->Shutdown();
  return nResult;
}




CLicqGui::CLicqGui(int argc, char **argv, const char *_szSkin, const char *_szIcons, const char *_szStyle)
#ifdef USE_KDE
: KApplication(argc, argv)
#else
: QApplication(argc, argv)
#endif
{
  QStyle *style = NULL;
  if (strcmp(_szStyle, "MOTIF") == 0)
    style = new QMotifStyle;
  else if (strcmp(_szStyle, "WINDOWS") == 0)
    style = new QWindowsStyle;
  else if (strcmp(_szStyle, "MAC") == 0)
    style = new QPlatinumStyle;
  else if (strcmp(_szStyle, "CDE") == 0)
    style = new QCDEStyle;
#ifdef USE_KDE
  else if (strcmp(_szStyle, "KDE") == 0)
    style = new KThemeStyle;
#endif

  if (style == NULL)
  {
#ifdef USE_KDE
    style = new KThemeStyle;
#else
    if (strcmp(STYLE, "MOTIF") == 0)
      style = new QMotifStyle;
    else if (strcmp(STYLE, "WINDOWS") == 0)
      style = new QWindowsStyle;
    else if (strcmp(STYLE, "MAC") == 0)
      style = new QPlatinumStyle;
    else if (strcmp(optarg, "CDE") == 0)
      style = new QCDEStyle;
    else
      style = new QWindowsStyle;
#endif
  }

  setStyle(style);
  m_szSkin = strdup(_szSkin);
  m_szIcons = strdup(_szIcons);
}


CLicqGui::~CLicqGui(void)
{
  delete licqSignalManager;
  delete licqLogWindow;
}

void CLicqGui::Shutdown(void)
{
  gLog.Info("%sShutting down gui.\n", L_ENDxSTR);
  gLog.ModifyService(S_PLUGIN, 0);
  delete licqMainWindow;
}


int CLicqGui::Run(CICQDaemon *_licqDaemon)
{
  // Register with the daemon, we want to receive all signals
  int nPipe = _licqDaemon->RegisterPlugin(SIGNAL_ALL);

  // Create the main widgets
  licqSignalManager = new CSignalManager(_licqDaemon, nPipe);
  licqLogWindow = new CQtLogWindow;
  gLog.AddService(new CLogService_Plugin(licqLogWindow, L_INFO | L_WARN | L_ERROR));
  licqMainWindow = new CMainWindow(_licqDaemon, licqSignalManager, licqLogWindow, m_szSkin, m_szIcons);

  setMainWidget(licqMainWindow);
  licqMainWindow->show();
  return exec();
}
