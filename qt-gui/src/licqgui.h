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
#include <qstringlist.h>

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
  CLicqGui(int, char **);
  ~CLicqGui(void);
  int Run(CICQDaemon *);
  void Shutdown(void);
  virtual bool x11EventFilter(XEvent *);

  virtual void commitData(QSessionManager& sm);
  virtual void saveState(QSessionManager& sm);

  bool grabKey(QString key);

protected:
  char *m_szSkin, *m_szIcons, *m_szExtendedIcons;
  bool m_bStartHidden;
  bool m_bDisableDockIcon;
  CMainWindow *licqMainWindow;
  CSignalManager *licqSignalManager;
  CQtLogWindow *licqLogWindow;
  QStringList cmdLineParams;
  int grabKeysym;

  QStyle *SetStyle(const char *_szStyle);
};

#endif
