#ifndef OPTIONSDLG_H
#define OPTIONSDLG_H

#include <qtabdialog.h>

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

class OptionsDlg : public QTabDialog
{
   Q_OBJECT
friend class CMainWindow;
public:

   enum tabs { ODlgAppearance, ODlgColumns, ODlgOnEvent, ODlgNetwork,
               ODlgStatus, ODlgMiscellaneous };

   OptionsDlg (CMainWindow *, tabs tab = ODlgAppearance,
               QWidget *parent = NULL, char *name = NULL);

protected:
   QWidget *tab[6];
   CMainWindow *mainwin;

   // network tab
   QWidget* new_network_options();
   QLabel *lblServers, *lblDefServerPort, *lblTcpServerPort,
          *lblAutoAway, *lblAutoNa, *lblAutoLogon;
   QComboBox *cmbServers, *cmbAutoLogon;
   QSpinBox *spnDefServerPort, *spnTcpServerPort, *spnAutoAway, *spnAutoNa,
            *spnPortLow, *spnPortHigh;
   QPushButton *btnAddServer;
   QCheckBox  *chkAutoLogonInvisible, *chkFirewall, *chkTCPEnabled;
   QLineEdit *edtFirewallHost;

   // appearance tab
   QWidget* new_appearance_options();
   QGroupBox *boxFont, *boxUserWin, *boxDocking, *boxLocale;
   QLabel *lblFont, *lblEditFont, *lblFrameStyle;
   QLineEdit *edtFont, *edtEditFont, *edtFrameStyle;
   QPushButton *btnFont, *btnEditFont;
   QCheckBox *chkGridLines, *chkHeader, *chkAutoClose, *chkShowDividers,
             *chkFontStyles, *chkUseDock, *chkDockFortyEight,
             *chkTransparent, *chkSortByStatus, *chkShowGroupIfNoMsg,
             *chkAutoPopup, *chkAutoRaise;
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
   QCheckBox *chkOnEvents, *chkOEAway, *chkOENA, *chkOEOccupied, *chkOEDND;
   QLineEdit *edtSndPlayer, *edtSndMsg, *edtSndChat, *edtSndUrl,
             *edtSndFile, *edtSndNotify, *edtSndSysMsg;

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

signals:
  void signal_done();

protected:
  void hideEvent(QHideEvent*) { emit signal_done(); close(true); };
  void SetupOptions();
  void setupFontName(QLineEdit*, const QFont&);

protected slots:
  void colEnable(bool);
  void ApplyOptions();
  void slot_selectfont();
  void slot_selecteditfont();
  void slot_whatsthis();
  void slot_SARmsg_act(int);
  void slot_SARgroup_act(int);
  void slot_SARsave_act();
  void slot_socks();
  void slot_useDockToggled(bool);
};


#endif
