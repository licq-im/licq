#ifndef SIGNAL_MANAGER_H
#define SIGNAL_MANAGER_H

#include <qobject.h>

class QSocketNotifier;
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
  void signal_updatedList(CICQSignal *);
  void signal_updatedUser(CICQSignal *);
  void signal_updatedStatus(CICQSignal *);
  void signal_logon();
  void signal_logoff();
  void signal_ui_viewevent(unsigned long);
  void signal_ui_viewevent(const char *);
  void signal_ui_message(const char *, unsigned long);
  void signal_protocolPlugin(unsigned long);
  void signal_eventTag(const char *, unsigned long, unsigned long);
  void signal_socket(const char *, unsigned long, int);
  void signal_convoJoin(const char *, unsigned long, int);
  void signal_convoLeave(const char *, unsigned long, int);
  
  // Event signals
  void signal_doneOwnerFcn(ICQEvent *);
  void signal_doneUserFcn(ICQEvent *);
  void signal_searchResult(ICQEvent *);
};


#endif
