#ifndef PLUGINDLG_H
#define PLUGINDLG_H

#include <qwidget.h>


class QTable;
class CICQDaemon;

class PluginDlg : public QWidget
{
  Q_OBJECT
public:
  PluginDlg();
  virtual ~PluginDlg();

protected:
  QTable *tblStandard;
  QTable *tblProtocol;

signals:
  void signal_done();
  void pluginUnloaded(unsigned long);

protected slots:
  void slot_standard(int, int);
  void slot_stdConfig(int, int, int, const QPoint &);
  void slot_protocol(int, int);
  void slot_prtConfig(int, int, int, const QPoint &);
  void slot_details();
  void slot_refresh();
};

#endif
