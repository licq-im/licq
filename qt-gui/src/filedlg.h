#ifndef FILEDLG_H
#define FILEDLG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qdialog.h>
#include <qtimer.h>

#include "licq_user.h"
#include "licq_socket.h"
#include "licq_buffer.h"
#include "ewidgets.h"

class QPushButton;
class QProgressBar;
class QLabel;
class QSocketNotifier;
class CICQDaemon;
class CFileTransferManager;


class CFileDlg : public QDialog
{
   Q_OBJECT
public:
  CFileDlg(unsigned long _nUin, CICQDaemon *daemon,
     QWidget *parent = NULL, char *name = NULL);
  virtual ~CFileDlg();

  bool SendFiles(const char *szFile, unsigned short nPort);
  bool ReceiveFiles();

  unsigned short LocalPort();
  unsigned long Uin()  { return m_nUin; };

public slots:
   virtual void hide();

protected:
   QLabel *lblTransferFileName, *lblLocalFileName, *lblFileSize,
	  *lblTrans, *lblBatch, *lblTime, *lblBPS, *lblETA;
   CInfoField *nfoFileSize,
              *nfoTransferFileName,
              *nfoTotalFiles,
              *nfoBatchSize,
              *nfoLocalFileName,
              *nfoBPS,
              *nfoETA,
              *nfoTime;
   QPushButton *btnCancel;
   QLabel *lblStatus;
   QProgressBar *barTransfer, *barBatchTransfer;

   CFileTransferManager *ftman;

   unsigned long m_nUin;
   CICQDaemon *licqDaemon;
   QSocketNotifier *sn;

   QTimer m_tUpdate;

   QString encodeFSize(unsigned long size);

protected slots:
  void slot_ft();
  void slot_update();
  void slot_cancel();
};

#endif



