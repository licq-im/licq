#ifndef SKINBROWSER_H
#define SKINBROWSER_H

#include <qdialog.h>

class QListView;
class QGroupBox;
class QPushButton;

class CMainWindow;
class CInfoField;

class SkinBrowserDlg : public QDialog
{
  Q_OBJECT
public:
  SkinBrowserDlg (CMainWindow *_mainwin, QWidget *parent = 0, const char *name = 0 );
protected:
  QListView *lstSkins, *lstIcons;
  QGroupBox *grpSkins, *grpIcons;
  QPushButton *btnApplySkin, *btnApplyIcons, *btnDone, *btnEditSkin;

  CInfoField *nfoSkin, *nfoIcons;
  CMainWindow *mainwin;

public slots:
  void slot_applyskin();
  void slot_editskin();
  void slot_applyicons();
};


#endif
