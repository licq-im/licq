#ifndef AUTHUSERDLG_H
#define AUTHUSERDLG_H

#include "licqdialog.h"

class QPushButton;
class QLabel;
class QGroupBox;
class QLineEdit;

class CICQDaemon;
class MLEditWrap;

class AuthUserDlg : public LicqDialog
{
   Q_OBJECT
public:
   AuthUserDlg (CICQDaemon *s, unsigned long nUin, bool bGrant,
      QWidget *parent = 0);
#ifdef QT_PROTOCOL_PLUGIN
   AuthUserDlg (CICQDaemon *s, const char *szId, unsigned long nPPID,
      bool bGrant, QWidget *parent = 0);
#endif

protected:
   CICQDaemon *server;
   QPushButton *btnOk, *btnCancel;
   QLabel *lblUin;
   QGroupBox *grpResponse;
   QLineEdit *edtUin;
   MLEditWrap* mleResponse;

   unsigned long m_nUin;
#ifdef QT_PROTOCOL_PLUGIN
   char *m_szId;
   unsigned long m_nPPID;
#endif

   bool m_bGrant;
protected slots:
   void ok();
};


#endif
