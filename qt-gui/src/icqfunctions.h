#ifndef ICQFUNCTIONS_H
#define ICQFUNCTIONS_H

#include <qdialog.h>

#include "userfcndlg.h"
#include "history.h"

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

class ICQUser;
class CICQDaemon;
class CEComboBox;
class ICQEvent;
class CICQEventTag;
class CInfoField;
class CSignalManager;
class MLEditWrap;
class MsgView;
class MLEditWrap;

//=====ICQFunctions=============================================================

const int TAB_READ = 0;
const int TAB_SEND = 1;
const int TAB_GENERALINFO = 2;
const int TAB_MOREINFO = 3;
const int TAB_WORKINFO = 4;
const int TAB_ABOUT = 5;
const int TAB_HISTORY = 6;
const int TAB_SECURITY = 7;

class ICQFunctions : public QDialog, public CUserFunctionDlg
{
   Q_OBJECT
public:
  ICQFunctions(CICQDaemon *s, CSignalManager *theSigMan,
               unsigned long _nUin, bool _bIsOwner, bool isAutoClose,
               QWidget *parent = 0, const char *name = 0);
  virtual ~ICQFunctions();
  void setupTabs(int);

protected:
  QString tabLabel[8];
  int currentTab;
  CICQDaemon *server;
  CSignalManager *sigman;
  QString m_sBaseTitle, m_sProgressMsg;
  CICQEventTag *icqEventTag;
  bool m_bOwner;
  QCheckBox *chkAutoClose;
  QTabWidget* tabs;
  QWidget *fcnTab[8];
  QPushButton *btnSave, *btnOk, *btnCancel;

  // Read Event tab
  void CreateReadEventTab();
  QSplitter *splRead;
  MLEditWrap *mleRead;
  MsgView *msgView;

  // Send Event tab
  void CreateSendEventTab();
  QBoxLayout* m_selay;
  MLEditWrap *mleSend;
  QCheckBox *chkSendServer, *chkSpoof, *chkUrgent;
  QLineEdit *edtSpoof, *edtItem;
  QLabel *lblItem;
  QGroupBox *grpOpt;
  QButtonGroup *grpCmd;
  QRadioButton *rdbUrl, *rdbChat, *rdbFile, *rdbMsg;

  // User Info tab
  void CreateGeneralInfoTab();
  CInfoField *nfoFirstName, *nfoLastName, *nfoEmail1, *nfoEmail2,
             *nfoAlias,*nfoStatus, *nfoIp, *nfoUin, *nfoCity, *nfoState,
             *nfoZipCode, *nfoAddress, *nfoCountry, *nfoFax, *nfoCellular,
             *nfoPhone;
  CEComboBox *cmbCountry;
  QCheckBox *chkAuthorization;

  // More info
  void CreateMoreInfoTab();
  CInfoField *nfoAge, *nfoBirthday, *nfoLanguage[3], *nfoHomepage;
  CEComboBox *cmbLanguage[3], *cmbGender;

  // Work info
  void CreateWorkInfoTab();
  CInfoField *nfoCompanyName, *nfoCompanyCity, *nfoCompanyState,
             *nfoCompanyAddress, *nfoCompanyPhone, *nfoCompanyFax,
             *nfoCompanyHomepage, *nfoCompanyPosition, *nfoCompanyDepartment;

  // About
  void CreateAboutTab();
  QLabel *lblAbout;
  MLEditWrap *mleAbout;

  // History tab
  void CreateHistoryTab();
  CInfoField *nfoHistory;
  QTextView *mleHistory;
  QLabel *lblHistory;
  HistoryList m_lHistoryList;
  HistoryListIter m_iHistoryIter;
  unsigned short m_nHistoryIndex, m_nHistoryShowing;

  static unsigned short s_nX;
  static unsigned short s_nY;

  virtual void keyPressEvent(QKeyEvent *);
  virtual void closeEvent(QCloseEvent *);
  void SetGeneralInfo(ICQUser *);
  void SetMoreInfo(ICQUser *);
  void SetWorkInfo(ICQUser *);
  void SetAbout(ICQUser *);
  void SaveGeneralInfo();
  void SaveMoreInfo();
  void SaveWorkInfo();
  void SaveAbout();
  void saveHistory();
  void generateReply();
  void SetupHistory();
  void ShowHistory();

public slots:
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
