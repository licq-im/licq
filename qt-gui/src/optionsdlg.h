#ifndef OPTIONSDLG_H
#define OPTIONSDLG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qtabdialog.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qspinbox.h>

#include "ewidgets.h"

class CMainWindow;
class MLEditWrap;

class OptionsDlg : public QTabDialog
{
   Q_OBJECT
friend class CMainWindow;
public:
   OptionsDlg (CMainWindow *, QWidget *parent = NULL, char *name = NULL);

protected:
   QWidget *tab[6];
   CMainWindow *mainwin;

   // network tab
   QWidget* new_network_options();
   QLabel *lblServers, *lblDefServerPort, *lblTcpServerPort,
          *lblAutoAway, *lblAutoNa, *lblAutoLogon, *lblMaxUsersPerPacket;
   QComboBox *cmbServers, *cmbAutoLogon;
   QSpinBox *spnDefServerPort, *spnTcpServerPort, *spnAutoAway, *spnAutoNa,
            *spnMaxUsersPerPacket;
   QPushButton *btnAddServer;
   QCheckBox  *chkAutoLogonInvisible;

   // appearance tab
   QWidget* new_appearance_options();
   QGroupBox *boxFont, *boxUserWin, *boxDocking, *boxLocale;
   QLabel *lblFont, *lblFrameStyle;
   QLineEdit *edtFont, *edtFrameStyle;
   QPushButton *btnFont;
   QCheckBox *chkGridLines, *chkHeader, *chkAutoClose, *chkShowDividers,
             *chkFontStyles, *chkUseDock, *chkDockFortyEight, *chkTransparent;

   // columns tab
   QWidget* new_column_options();
   QLabel *lblColWidth, *lblColAlign, *lblColTitle, *lblColFormat;
   QCheckBox *chkColEnabled[4];
   QSpinBox *spnColWidth[4];
   QComboBox *cmbColAlign[4];
   QLineEdit *edtColTitle[4], *edtColFormat[4];

   // sounds tab
   QWidget* new_sounds_options();
   QLabel *lblSndPlayer, *lblSndMsg, *lblSndChat, *lblSndUrl,
          *lblSndFile, *lblSndNotify, *lblSndSysMsg;
   QCheckBox *chkOnEvents;
   QLineEdit *edtSndPlayer, *edtSndMsg, *edtSndChat, *edtSndUrl,
             *edtSndFile, *edtSndNotify, *edtSndSysMsg;
   QGroupBox *boxSndEvents;

   // misc tab
   QWidget* new_misc_options();
   QGroupBox *boxParanoia, *boxExtensions;
   QLabel *lblUrlViewer, *lblTrans, *lblLocale;
   QLineEdit *edtUrlViewer;
   QComboBox *cmbTrans, *cmbLocale;
   QLabel *lblTerminal;
   QLineEdit *edtTerminal;
   QCheckBox *chkWebPresence, *chkHideIp, *chkIgnoreNewUsers,
             *chkIgnoreWebPanel, *chkIgnoreMassMsg, *chkIgnoreEmailPager;

   // status tab
   QWidget* new_status_options();
   QComboBox* cmbSARgroup, *cmbSARmsg;
   MLEditWrap* edtSARtext;

   virtual void hide();

protected:
  void SetupOptions();
  void ApplyOptions();

protected slots:
  void colEnable(bool);
  void slot_apply();
  void slot_selectfont();
  void slot_whatsthis();
  void slot_SARmsg_act(int);
  void slot_SARgroup_act(int);
  void slot_SARsave_act();
};


#endif
