#ifndef SIGNAL_MANAGER_H
#define SIGNAL_MANAGER_H

#include <qobject.h>
#include <qsocketnotifier.h>

class CICQDaemon;
class CICQSignal;
class ICQEvent;

//=====CSignalManager===========================================================
class CSignalManager: public QObject
{
  Q_OBJECT
public:
  CSignalManager(CICQDaemon *, int);

protected:
  CICQDaemon *licqDaemon;
  int m_nPipe;
  QSocketNotifier *sn;

  void ProcessSignal(CICQSignal *s);
  void ProcessEvent(ICQEvent *e);

protected slots:
  void slot_incoming();

signals:
  // Signal signals
  void signal_updatedList(unsigned long, unsigned long);
  void signal_updatedUser(unsigned long, unsigned long);
  void signal_updatedStatus();
  void signal_logon();

  // Event signals
  void signal_doneOwnerFcn(ICQEvent *);
  void signal_doneUserFcn(ICQEvent *);
  void signal_searchResult(ICQEvent *);
};


#endif
