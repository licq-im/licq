#ifndef FILEDLG_H
#define FILEDLG_H

#include <qwidget.h>
#include <qtimer.h>

#include "ewidgets.h"
#include "licq_filetransfer.h"

class QPushButton;
class QProgressBar;
class QLabel;
class QSocketNotifier;
class CICQDaemon;
class CFileTransferManager;
class MLEditWrap;

class CFileDlg : public QWidget
{
   Q_OBJECT
public:
  CFileDlg(unsigned long _nUin, CICQDaemon *daemon, QWidget* parent = 0);
  CFileDlg(const char *szId, unsigned long nPPID, CICQDaemon *daemon,
    QWidget *parent = 0);
  virtual ~CFileDlg();

  bool SendFiles(ConstFileList filelist, unsigned short nPort);
  bool ReceiveFiles();

  unsigned short LocalPort();
  char *Id()  { return m_szId; }
  unsigned long PPID()  { return m_nPPID; }
  unsigned long Uin()  { return m_nUin; };

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
   QPushButton *btnCancel, *btnOpen, *btnOpenDir;
   MLEditWrap *mleStatus;
   QProgressBar *barTransfer, *barBatchTransfer;

   CFileTransferManager *ftman;

   unsigned long m_nUin;
   char *m_szId;
   unsigned long m_nPPID;
   CICQDaemon *licqDaemon;
   QSocketNotifier *sn;

   QTimer m_tUpdate;

   QString encodeFSize(unsigned long size);

protected slots:
  void slot_ft();
  void slot_update();
  void slot_cancel();
  void slot_open();
  void slot_opendir();
};

#endif



