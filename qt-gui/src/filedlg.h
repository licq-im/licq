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
  virtual ~CFileDlg();

  bool SendFiles(ConstFileList filelist, unsigned short nPort);
  bool ReceiveFiles();

  unsigned short LocalPort();
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
   QPushButton *btnCancel;
   MLEditWrap *mleStatus;
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



