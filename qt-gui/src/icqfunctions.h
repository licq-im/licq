#ifndef ICQFUNCTIONS_H
#define ICQFUNCTIONS_H

#include <qtabwidget.h>

#include "userfcndlg.h"
//#include "ewidgets.h"
#include "history.h"

class QSplitter;
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QButtonGroup;
class QRadioButton;
class QTextView;

class ICQUser;
class CICQDaemon;
class CEComboBox;
class ICQEvent;
class CInfoField;
class CSignalManager;
class MLEditWrap;
class MsgView;

//=====ICQFunctions=============================================================

const int TAB_READ = 0;
const int TAB_SEND = 1;
const int TAB_BASICINFO = 2;
const int TAB_DETAILINFO = 3;
const int TAB_HISTORY = 4;

class ICQFunctions : public QTabWidget, public CUserFunctionDlg
{
   Q_OBJECT
public:
  ICQFunctions(CICQDaemon *s, CSignalManager *theSigMan,
               unsigned long _nUin, bool _bIsOwner, bool isAutoClose,
               QWidget *parent = 0, const char *name = 0);
  virtual ~ICQFunctions(void);
  void setupTabs(int);

protected:
  QString tabLabel[5];
  int currentTab;
  CICQDaemon *server;
  CSignalManager *sigman;
  QString m_sBaseTitle, m_sProgressMsg;
  unsigned short m_nUnknownCountryCode;
  ICQEvent *icqEvent;
  bool m_bIsOwner;
  QCheckBox *chkAutoClose;

  QWidget *fcnTab[5];
  QPushButton *btnSave, *btnOk, *btnCancel;

  // Read Event tab
  QSplitter *splRead;
  MLEditWrap *mleRead;
  MsgView *msgView;

  // Send Event tab
  MLEditWrap *mleSend;
  QCheckBox *chkSendServer, *chkSpoof, *chkUrgent;
  QLineEdit *edtSpoof, *edtItem;
  QLabel *lblItem, *lblDescription;
  QButtonGroup *grpCmd;
  QRadioButton *rdbUrl, *rdbChat, *rdbFile, *rdbMsg;

  // User Info tab
  CInfoField *nfoFirstName, *nfoLastName, *nfoEMail, *nfoAlias,
             *nfoStatus, *nfoIp, *nfoUin, *nfoHistory;
  QCheckBox *chkAuthorization;

  // Extended Info tab
  CInfoField *nfoCity, *nfoState, *nfoAge,
            *nfoPhone, *nfoHomepage, *nfoZipcode;
  QLabel *lblSex, *lblCountry;
  CEComboBox *cmbSex, *cmbCountry;
  QGroupBox *boxAboutMsg;
  MLEditWrap *mleAboutMsg;

  // History tab
  QTextView *mleHistory;
  QLabel *lblHistory;
  HistoryList m_lHistoryList;
  HistoryListIter m_iHistoryIter;
  unsigned short m_nHistoryIndex, m_nHistoryShowing;

  static unsigned short s_nX;
  static unsigned short s_nY;

  virtual void resizeEvent (QResizeEvent *);
  virtual void keyPressEvent(QKeyEvent *);
  virtual void closeEvent(QCloseEvent *);
  void setBasicInfo(ICQUser *);
  void setExtInfo(ICQUser *);
  void saveBasicInfo();
  void saveExtInfo();
  void saveHistory();
  void generateReply();
  void SetupHistory(void);
  void ShowHistory(void);

public slots:
  //virtual void hide();
  void SendUrl(const char *url, const char *desc);
  void SendFile(const char *file, const char *desc);

protected slots:
   void callFcn();
   void doneFcn(ICQEvent *);
   void slot_updatedUser(unsigned long, unsigned long);
   void tabSelected(const QString &);
   void ShowHistoryPrev();
   void ShowHistoryNext();
   void printMessage(QListViewItem *);
   void save();
   void setSpoofed();
   void specialFcn(int);

signals:
   void signal_updatedUser(unsigned long, unsigned long);
   void signal_finished(unsigned long);
};


#endif
