/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_KDE
#include <kthemestyle.h>
#endif
#include <qwindowsstyle.h>
#include <qmotifstyle.h>
#include <qplatinumstyle.h>
#include <qcdestyle.h>

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#include "mainwin.h"
#include "licqgui.h"
#include "sigman.h"
#include "outputwin.h"
#include "log.h"
#include "plugin.h"

#include "icqd.h"

CLicqGui *licqQtGui;

const char *LP_Usage(void)
{
  static const char usage[] =
    "Usage:  Licq [options] -p qt-gui -- [-h] [-s skinname] [-i iconpack] [-g gui style]\n"
    " -h : this help screen\n"
    " -s : set the skin to use (must be in {base dir}/qt-gui/skin.skinname)\n"
    " -i : set the icons to use (must be in {base dir}/qt-gui/icons.iconpack)\n"
    " -g : set the gui style (MOTIF / WINDOWS / MAC / CDE), ignored by KDE support\n"
    " -d : start hidden (dock icon only)\n";
  return usage;
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

const char *LP_Status(void)
{
  static const char status[] = "running";
  return status;
}

const char *LP_Description(void)
{
  static const char desc[] = "Qt based GUI";
  return desc;
}

bool LP_Init(int argc, char **argv)
{
  char skinName[32] = "";
  char iconsName[32] = "";
  char styleName[32] = "";
  bool bStartHidden = false;

  // parse command line for arguments
  int i = 0;
  while( (i = getopt(argc, argv, "hs:i:g:d")) > 0)
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
    case 'd': // dock icon
      bStartHidden = true;
    }
  }
  if (qApp != NULL)
  {
    gLog.Error("%sA Qt application is already loaded.\n%sRemove the plugin from the command line.\n", L_ERRORxSTR, L_BLANKxSTR);
    return false;
  }
  licqQtGui = new CLicqGui(argc, argv, bStartHidden, skinName, iconsName, styleName);
  return (licqQtGui != NULL);
}


int LP_Main(CICQDaemon *_licqDaemon)
{
  int nResult = licqQtGui->Run(_licqDaemon);
  licqQtGui->Shutdown();
  return nResult;
}

QStyle *CLicqGui::SetStyle(const char *_szStyle)
{
#ifndef USE_KDE
  QStyle *s = NULL;
  if (strncmp(_szStyle, "MOTIF", 3) == 0)
    s = new QMotifStyle;
  else if (strncmp(_szStyle, "WINDOWS", 3) == 0)
    s = new QWindowsStyle;
  else if (strncmp(_szStyle, "MAC", 3) == 0)
    s = new QPlatinumStyle;
  else if (strncmp(_szStyle, "CDE", 3) == 0)
    s = new QCDEStyle;
  return s;
#else
  if(_szStyle); // no warning
  return &kapp->style();
#endif
}


CLicqGui::CLicqGui(int argc, char **argv, bool bStartHidden, const char *_szSkin, const char *_szIcons, const char *_szStyle)
#ifdef USE_KDE
: KApplication(argc, argv, "KLicq")
#else
: QApplication(argc, argv)
#endif
{
  char buf[64];
  sprintf(buf, "%s/licq_qt-gui.style", BASE_DIR);

  QStyle *style = SetStyle(_szStyle);

  // Write out the style if not NULL
  if (style != NULL)
  {
    FILE *f = fopen(buf, "w");
    if (f != NULL)
    {
      fprintf(f, "%s\n", _szStyle);
      fclose(f);
    }
  }
  // Otherwise try and load it from the file
  else
  {
#ifndef USE_KDE
    FILE *f = fopen(buf, "r");
    if (f != NULL)
    {
      if (fgets(buf, 64, f) != NULL)
        style = SetStyle(buf);
      fclose(f);
    }
    if (style == NULL) style = new STYLE;
#endif
  }

  setStyle(style);
  m_szSkin = strdup(_szSkin);
  m_szIcons = strdup(_szIcons);
  m_bStartHidden = bStartHidden;

  // Try and load a translation
  char *p;
#ifndef HAVE_LOCALE_H
  if ( (p = getenv("LANGUAGE")) || (p = getenv("LANG")) )
#else
  if ( (p = setlocale(LC_MESSAGES, NULL) ))
#endif
  {
    QString str;
    str.sprintf("%sqt-gui/locale/%s", SHARE_DIR, p);
    QTranslator *trans = new QTranslator(this);
    trans->load(str);
    installTranslator(trans);
  }
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
  gLog.AddService(new CLogService_Plugin(licqLogWindow, L_INFO | L_WARN | L_ERROR | L_UNKNOWN));
  licqMainWindow = new CMainWindow(_licqDaemon, licqSignalManager, licqLogWindow,
     m_bStartHidden, m_szSkin, m_szIcons);

  setMainWidget(licqMainWindow);
  licqMainWindow->show();
  int r = exec();
  _licqDaemon->UnregisterPlugin();

  return r;
}
/*
#include <X11/Xlib.h>

bool CLicqGui::x11EventFilter(XEvent *e)
{
  QETWidget *widget = (QETWidget*)QWidget::find( (WId)e->xany.window );
  printf("event %d %d %d\n", e->type, widget, e->xany.window);
  return QApplication::x11EventFilter(e);
}
*/


