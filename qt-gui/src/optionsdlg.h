#ifndef OPTIONSDLG_H
#define OPTIONSDLG_H

#include <qdialog.h>

#include "ewidgets.h"

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

class OptionsDlg : public QDialog
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
   QLabel *lblServers, *lblDefServerPort,
      *lblAutoAway, *lblAutoNa, *lblAutoOffline, *lblAutoLogon;
   QComboBox *cmbServers, *cmbAutoLogon;
   QComboBox *cmbAutoAwayMess, *cmbAutoNAMess;
   QSpinBox *spnDefServerPort, *spnAutoAway, *spnAutoNa,
      *spnAutoOffline, *spnPortLow, *spnPortHigh;
   QPushButton *btnAddServer;
   QCheckBox  *chkAutoLogonInvisible, *chkTCPEnabled;
   //*chkFirewall
   //QLineEdit *edtFirewallHost;

   // appearance tab
   QWidget* new_appearance_options();
   QGroupBox *boxFont, *boxUserWin, *boxDocking, *boxLocale;
   QLabel *lblFont, *lblEditFont, *lblFrameStyle, *lblHotKey;
   QLineEdit *edtFont, *edtEditFont, *edtFrameStyle, *edtHotKey;
   QPushButton *btnFont, *btnEditFont;
   QCheckBox *chkGridLines, *chkHeader, *chkAutoClose, *chkShowDividers,
             *chkFontStyles, *chkUseDock, *chkDockFortyEight,
             *chkTransparent, *chkSortByStatus, *chkShowGroupIfNoMsg,
             *chkAutoPopup, *chkAutoRaise, *chkFlashUrgent, *chkFlashAll,
             *chkBoldOnMsg, *chkManualNewUser,
             *chkAlwaysShowONU, *chkScrollBar, *chkShowExtIcons,
             *chkSysBack, *chkSendFromClipboard, *chkMsgChatView, *chkAutoPosReplyWin,
	     *chkAutoSendThroughServer, *chkEnableMainwinMouseMovement;
   QRadioButton *rdbDockDefault, *rdbDockThemed;
   QComboBox *cmbDockTheme;

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
   QLineEdit *edtSndPlayer, *edtSndMsg, *edtSndChat, *edtSndUrl,
      *edtSndFile, *edtSndNotify, *edtSndSysMsg, *edtSndMsgSent;

   // misc tab
   QWidget* new_misc_options();
   QGroupBox *boxParanoia, *boxExtensions;
   QLabel *lblUrlViewer, *lblTrans;
   QLineEdit *edtUrlViewer;
   QComboBox *cmbTrans;
   QLabel *lblTerminal;
   QLineEdit *edtTerminal;
   QCheckBox *chkWebPresence, *chkHideIp, *chkIgnoreNewUsers,
             *chkIgnoreWebPanel, *chkIgnoreMassMsg, *chkIgnoreEmailPager;

   // status tab
   QWidget* new_status_options();
   QComboBox* cmbSARgroup, *cmbSARmsg;
   MLEditWrap* edtSARtext;

   // display tab
   QWidget* new_popup_options();
   QGroupBox *boxPopWin;
   QCheckBox *popEmail, *popPhone, *popFax, *popCellular, *popIP, *popLastOnline;

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
  void slot_socks();
  void slot_useDockToggled(bool);
  void slot_ok();
};


#endif
