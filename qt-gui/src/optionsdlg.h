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
   QWidget *tab[7];
   CMainWindow *mainwin;

   // network tab
   QLabel *lblServers, *lblDefServerPort, *lblTcpServerPort,
          *lblAutoAway, *lblAutoNa, *lblAutoLogon, *lblMaxUsersPerPacket;
   QComboBox *cmbServers, *cmbAutoLogon;
   QSpinBox *spnDefServerPort, *spnTcpServerPort, *spnAutoAway, *spnAutoNa,
            *spnMaxUsersPerPacket;
   QPushButton *btnAddServer;
   QCheckBox  *chkAutoLogonInvisible;

   // plugin tab
   QLabel *lblUrlViewer, *lblTrans;
   QLineEdit *edtUrlViewer;
   QComboBox *cmbTrans;
   QLabel *lblTerminal;
   QLineEdit *edtTerminal;

   // appearance tab
   QGroupBox *boxFont;
   QLabel *lblFont;
   QLineEdit *edtFont;
   QPushButton *btnFont;
   QCheckBox *chkGridLines, *chkHeader, *chkAutoClose, *chkShowDividers,
             *chkFontStyles, *chkUseDock, *chkDockFortyEight;

   // columns tab
   QLabel *lblColWidth, *lblColAlign, *lblColTitle, *lblColFormat;
   QCheckBox *chkColEnabled[4];
   QSpinBox *spnColWidth[4];
   QComboBox *cmbColAlign[4];
   QLineEdit *edtColTitle[4], *edtColFormat[4];

   // sounds tab
   QLabel *lblSndPlayer, *lblSndMsg, *lblSndChat, *lblSndUrl,
          *lblSndFile, *lblSndNotify, *lblSndSysMsg;
   QCheckBox *chkOnEvents;
   QLineEdit *edtSndPlayer, *edtSndMsg, *edtSndChat, *edtSndUrl,
             *edtSndFile, *edtSndNotify, *edtSndSysMsg;
   QGroupBox *boxSndEvents;

   // paranoia tab
   QCheckBox *chkWebPresence, *chkHideIp, *chkIgnoreNewUsers,
             *chkIgnoreWebPanel, *chkIgnoreMassMsg, *chkIgnoreEmailPager;


   // status tab
   QComboBox* cmbSARgroup, *cmbSARmsg;
   MLEditWrap* edtSARtext;

   virtual void hide();

protected:
  QWidget* new_network_options();
  QWidget* new_status_options();
  QWidget* new_column_options();
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
