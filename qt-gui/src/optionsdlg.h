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
   QLabel *lblServers, *lblDefServerPort, *lblTcpServerPort,
          *lblAutoAway, *lblAutoNa, *lblAutoLogon, *lblMaxUsersPerPacket;
   QComboBox *cmbServers, *cmbAutoLogon;
   QSpinBox *spnDefServerPort, *spnTcpServerPort, *spnAutoAway, *spnAutoNa,
            *spnMaxUsersPerPacket;
   QPushButton *btnAddServer;
   QCheckBox  *chkAutoLogonInvisible, *chkWebPresence, *chkHideIp,
       *chkAllowNewUsers;

   // plugin tab
   QLabel *lblErrorLog, *lblUrlViewer, *lblTrans;
   QLineEdit *edtErrorLog, *edtUrlViewer;
   QComboBox *cmbTrans;
   CInfoField *nfoTerminal;

   // appearance tab
   QGroupBox *boxFont;
   CInfoField *nfoFont;
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

   // status tab
   QComboBox* cmbSARgroup, *cmbSARmsg;
   MLEditWrap* edtSARtext;

   virtual void hide(void);

protected:
  void new_network_options();
  void new_status_options();
  void SetupOptions(void);
  void ApplyOptions(void);

protected slots:
  void colEnable(bool);
  void slot_apply();
  void slot_selectfont();
  void slot_whatsthis();
  void slot_SARmsg_act(int);
};


#endif
