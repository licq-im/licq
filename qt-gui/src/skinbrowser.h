#ifndef SKINBROWSER_H
#define SKINBROWSER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qpushbt.h>
#include <qgroupbox.h>
#include <qlistbox.h>

#include "ewidgets.h"
#include "mainwin.h"

class SkinBrowserDlg : public QWidget
{
  Q_OBJECT
public:
  SkinBrowserDlg (CMainWindow *_mainwin, QWidget *parent = 0, const char *name = 0 );
protected:
  QListBox *lstSkins, *lstIcons;
  QGroupBox *grpSkins, *grpIcons;
  QPushButton *btnApplySkin, *btnApplyIcons, *btnDone;

  CInfoField *nfoSkin, *nfoIcons;
  CMainWindow *mainwin;

  void resizeEvent (QResizeEvent *);
public slots:
  virtual void hide();
protected slots:
  void slot_applyskin();
  void slot_applyicons();
};


#endif
