#ifndef LICQGUI_H
#define LICQGUI_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_KDE
#include <kapp.h>
#else
#include <qapp.h>
#endif

#include "mainwin.h"
#include "icq.h"

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

};

#endif
