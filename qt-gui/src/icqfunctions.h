#ifndef ICQFUNCTIONS_H
#define ICQFUNCTIONS_H

#include <qdialog.h>

class QBoxLayout;
class QSplitter;
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QButtonGroup;
class QRadioButton;
class QTextView;
class QTabWidget;
class QListViewItem;
class QTimer;
class QSpinBox;
class QVGroupBox;
class QGridLayout;

class ICQUser;
class CICQSignal;
class CICQDaemon;
class CEComboBox;
class ICQEvent;
class CICQEventTag;
class CInfoField;
class CSignalManager;
class MLEditWrap;
class MsgView;
class MLEditWrap;
class CUserEvent;
class CMainWindow;
class CEButton;
class CMMUserView;

//=====ICQFunctions=============================================================

const int TAB_READ = 0;
const int TAB_SEND = 1;

struct STab
{
  QString label;
  QWidget *tab;
  bool loaded;
};

class ICQFunctions : public QWidget
{
   Q_OBJECT
public:
  ICQFunctions(CICQDaemon *s, CSignalManager *theSigMan, CMainWindow *m,
               unsigned long _nUin, bool isAutoClose, QWidget* parent = 0);
  virtual ~ICQFunctions();
  void setupTabs(int);
  unsigned long Uin()  {  return m_nUin;  }

protected:
  unsigned long m_nUin;
  struct STab tabList[8];
  int currentTab;
  CICQDaemon *server;
  CMainWindow *mainwin;
  CSignalManager *sigman;
  QString m_sBaseTitle, m_sProgressMsg;
  CICQEventTag *icqEventTag;
  bool m_bOwner, m_bDeleteUser;
  QCheckBox *chkAutoClose;
  QTabWidget *tabs;
  QPushButton  *btnOk;
  CEButton *btnCancel;

  // common part
  CInfoField *nfoStatus, *nfoTimezone;
  time_t m_nRemoteTimeOffset;
  QTimer *tmrTime;

  // Read Event tab
  void CreateReadEventTab();
  QSplitter *splRead;
  MLEditWrap *mleRead;
  MsgView *msgView;
  QPushButton *btnRead1, *btnRead2, *btnRead3, *btnRead4, *btnReadNext;
  CUserEvent *m_xCurrentReadEvent;

  // Send Event tab
  void CreateSendEventTab();
  void SendTab_grpOpt();
  void SendTab_grpMR();
  QGridLayout *laySend;
  MLEditWrap *mleSend;
  QCheckBox *chkSendServer, *chkSpoof, *chkUrgent, *chkMass;
  QLineEdit *edtSpoof;
  CInfoField *edtItem;
  QLabel *lblItem;
  QGroupBox *grpOpt;
  QVGroupBox *grpMR;
  QButtonGroup *grpCmd;
  QPushButton *btnItem;
  QRadioButton *rdbUrl, *rdbChat, *rdbFile, *rdbMsg;
  QString m_szMPChatClients;
  unsigned short m_nMPChatPort;
  CMMUserView *lstMultipleRecipients;

  static unsigned short s_nX;
  static unsigned short s_nY;

  virtual void keyPressEvent(QKeyEvent *);
  virtual void closeEvent(QCloseEvent *);
  void RetrySend(ICQEvent *e, bool bOnline, unsigned short nLevel);
  void SetInfo(ICQUser *);
  void generateReply();

public slots:
  void SendMsg(QString msg);
  void SendUrl(QString url, QString desc);
  void SendFile(QString file, QString desc);

protected slots:
   void callFcn();
   void doneFcn(ICQEvent *);
   void slot_updatedUser(CICQSignal *);
   void tabSelected(const QString &);
   void slot_printMessage(QListViewItem *);
   void setSpoofed();
   void specialFcn(int);
   void slot_masstoggled(bool);
   void slot_updatetime();
   void slot_readbtn1();
   void slot_readbtn2();
   void slot_readbtn3();
   void slot_readbtn4();
   void slot_sendbtn();
   void slot_close();
   void slot_resettitle();

signals:
   void signal_finished(unsigned long);
   void signal_updatedUser(CICQSignal *);
};


#endif
