#ifndef LICQGUI_H
#define LICQGUI_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_KDE
#include <kapp.h>
#else
#include <qapplication.h>
#endif

//#include "mainwin.h"

class CICQDaemon;
class CMainWindow;
class CSignalManager;
class CQtLogWindow;

#ifdef USE_KDE
class CLicqGui : public KApplication
#else
class CLicqGui : public QApplication
#endif
{
public:
  CLicqGui(int, char **, const char *, const char *, const char *);
  ~CLicqGui(void);
  int Run(CICQDaemon *);
  void Shutdown(void);
protected:
  char *m_szSkin, *m_szIcons;
  CMainWindow *licqMainWindow;
  CSignalManager *licqSignalManager;
  CQtLogWindow *licqLogWindow;

  QStyle *SetStyle(const char *_szStyle);
};

#endif
