#ifndef OPTIONSDLG_H
#define OPTIONSDLG_H


#include "licqdialog.h"
#include "ewidgets.h"

#include <config.h>

class CMainWindow;
class MLEditWrap;

class QLineEdit;
class QComboBox;
class QLabel;
class QCheckBox;
class QSpinBox;
class QGroupBox;
class QFont;
class QRadioButton;
class QTabWidget;

#ifdef USE_KDE
class KURLRequester;
#else
class KURLRequester : public QLineEdit
{
public:
    KURLRequester( QWidget *parent=0, const char *name=0 )
        : QLineEdit(parent, name) {};
    void setURL(const QString& url) { setText(url); }
    QString url() const { return text(); }
};
#endif

class OptionsDlg : public LicqDialog
{
   Q_OBJECT
friend class CMainWindow;
public:

   enum tabs { ODlgAppearance, ODlgColumns, ODlgOnEvent, ODlgNetwork,
               ODlgStatus, ODlgMiscellaneous };

   OptionsDlg (CMainWindow *, tabs tab = ODlgAppearance, QWidget* parent=0);
   virtual ~OptionsDlg();

protected:
   QWidget *tab[8];
   QTabWidget* tabw;
   QPushButton *btnOk, *btnApply, *btnCancel;
   CMainWindow *mainwin;

   // network tab
   QWidget* new_network_options();
   QLabel *lblAutoAway, *lblAutoNa, *lblAutoOffline, *lblAutoLogon, *lblSortBy;
   QLabel *lblICQServer, *lblICQServerPort, *lblProxyType, *lblProxyHost,
          *lblProxyPort, *lblProxyLogin, *lblProxyPasswd;
   QComboBox *cmbAutoLogon, *cmbProxyType;
   QComboBox *cmbAutoAwayMess, *cmbAutoNAMess;
   QSpinBox *spnAutoAway, *spnAutoNa,
      *spnAutoOffline, *spnPortLow, *spnPortHigh, *spnICQServerPort, *spnProxyPort;
   QLineEdit *edtICQServer, *edtProxyHost, *edtProxyLogin, *edtProxyPasswd;
   QPushButton *btnAddServer;
   QCheckBox  *chkAutoLogonInvisible, *chkFirewall, *chkTCPEnabled, *chkProxyEnabled, 
              *chkProxyAuthEnabled, *chkReconnectAfterUinClash;

   // appearance tab
   QWidget* new_appearance_options();
   QGroupBox *boxFont, *boxUserWin, *boxDocking, *boxLocale;
   QLabel *lblFont, *lblEditFont, *lblFrameStyle, *lblHotKey;
   QLineEdit *edtFont, *edtEditFont, *edtFrameStyle, *edtHotKey;
   QPushButton *btnFont, *btnEditFont;
   QCheckBox *chkSSList, *chkGridLines, *chkHeader, *chkAutoClose,
             *chkShowDividers, *chkFontStyles, *chkUseDock, *chkDockFortyEight,
             *chkTransparent, *chkShowGroupIfNoMsg,
             *chkAutoPopup, *chkAutoRaise, *chkHidden, *chkFlashUrgent, *chkFlashAll,
             *chkBoldOnMsg, *chkManualNewUser,
             *chkAlwaysShowONU, *chkScrollBar, *chkShowExtIcons,
             *chkSysBack, *chkSendFromClipboard, *chkMsgChatView, *chkAutoPosReplyWin,
	     *chkAutoSendThroughServer, *chkTabbedChatting,
             *chkEnableMainwinMouseMovement;
   QRadioButton *rdbDockDefault, *rdbDockThemed, *rdbDockSmall;
   QComboBox *cmbDockTheme, *cmbSortBy;

   // columns tab
   QWidget* new_column_options();
   QLabel *lblColWidth, *lblColAlign, *lblColTitle, *lblColFormat;
   QCheckBox *chkColEnabled[4];
   QSpinBox *spnColWidth[4];
   QComboBox *cmbColAlign[4];
   QLineEdit *edtColTitle[4], *edtColFormat[4];

   // sounds tab
   QWidget* new_sounds_options();
   QCheckBox *chkOnEvents, *chkOEAway, *chkOENA, *chkOEOccupied, *chkOEDND,
      *chkAlwaysOnlineNotify;
   KURLRequester *edtSndPlayer, *edtSndMsg, *edtSndChat, *edtSndUrl,
      *edtSndFile, *edtSndNotify, *edtSndSysMsg, *edtSndMsgSent;

   // misc tab
   QWidget* new_misc_options();
   QGroupBox *boxParanoia, *boxExtensions, *boxAutoUpdate;
   QLabel *lblUrlViewer, *lblDefaultEncoding;
   QComboBox *cmbUrlViewer;
   QComboBox *cmbDefaultEncoding;
   QLabel *lblTerminal;
   QLineEdit *edtTerminal;
   QCheckBox *chkWebPresence, *chkHideIp, *chkIgnoreNewUsers,
             *chkIgnoreWebPanel, *chkIgnoreMassMsg, *chkIgnoreEmailPager,
             *chkShowAllEncodings, *chkAutoUpdateInfo,
             *chkAutoUpdateInfoPlugins, *chkAutoUpdateStatusPlugins;

   // status tab
   QWidget* new_status_options();
   QComboBox* cmbSARgroup, *cmbSARmsg;
   MLEditWrap* edtSARtext;

   // display tab
   QWidget* new_popup_options();
   QGroupBox *boxPopWin;
   QCheckBox *popEmail, *popPhone, *popFax, *popCellular, *popIP,
	*popLastOnline, *popOnlineSince, *popIdleTime;

   // Phone & celluar tab
   QWidget* new_phone_options();


signals:
  void signal_done();

protected:
  void SetupOptions();
  void setupFontName(QLineEdit*, const QFont&);
  void buildAutoStatusCombos(bool);

protected slots:
  virtual void accept();
  virtual void reject();
  void colEnable(bool);
  void ApplyOptions();
  void slot_selectfont();
  void slot_selecteditfont();
  void slot_SARmsg_act(int);
  void slot_SARgroup_act(int);
  void slot_SARsave_act();
  void slot_useProxy(bool);
  void slot_useDockToggled(bool);
  void slot_useFirewall(bool);
  void slot_usePortRange(bool b);
  void slot_useMsgChatView(bool);
  void slot_ok();
  void slot_chkOnEventsToggled(bool);
};


#endif
