#ifndef PLUGINDLG_H
#define PLUGINDLG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


class QListView;
class QListBox;
class CICQDaemon;

class PluginDlg : public QWidget
{
  Q_OBJECT
public:
  PluginDlg(CICQDaemon *d);

public slots:
  virtual void hide();

protected:
  QListBox *lstAvailable;
  QListView *lstLoaded;
  CICQDaemon *licqDaemon;

protected slots:
  void slot_details();
  void slot_load();
  void slot_enable();
  void slot_disable();
  void slot_unload();
  void slot_refresh();
};

#endif
