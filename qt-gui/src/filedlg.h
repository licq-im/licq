#ifndef FILEDLG_H
#define FILEDLG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qdialog.h>
#include <qtimer.h>

#include "user.h"
#include "socket.h"
#include "buffer.h"
#include "ewidgets.h"

class QPushButton;
class QProgressBar;
class QLabel;
class QSocketNotifier;
class CICQDaemon;

struct SFileInfo
{
   char szName[MAX_FILENAME_LEN];
   unsigned long nSize;
};


class CFileDlg : public QDialog
{
   Q_OBJECT
public:
   CFileDlg(unsigned long _nUin,
            const char *_szRemoteFileName, unsigned long _nFileSize,
            CICQDaemon *daemon,
            QWidget *parent = NULL, char *name = NULL);
   virtual ~CFileDlg();

   bool StartAsClient(unsigned short nPort);
   bool StartAsServer();

   unsigned short LocalPort()
     { return m_bServer ? m_xSocketFileServer.LocalPort() : m_xSocketFile.LocalPort(); }
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

   unsigned long m_nUin;
   CICQDaemon *licqDaemon;
   char *m_szLocalName, *m_szRemoteName, buf[128];
   struct SFileInfo m_sFileInfo;
   QSocketNotifier *snFile, *snFileServer;
   TCPSocket m_xSocketFile,
             m_xSocketFileServer;
   unsigned short m_nPort, m_nCurrentFile, m_nState;
   unsigned long m_nFileSize, m_nBatchSize, m_nTotalFiles, m_nFilePos, m_nBatchPos;
   time_t m_nStartTime, m_nBatchStartTime;
   bool m_bServer;

   int m_nFileDesc;
   unsigned long m_nBytesTransfered, m_nBatchBytesTransfered;

   QTimer m_tUpdate;
   QSocketNotifier *m_snSend;

   bool GetLocalFileName();
   QString encodeFSize(unsigned long size);

protected slots:
   void fileSendFile();
   void fileRecvFile();
   void fileRecvConnection();
   void StateServer();
   void StateClient();
   void fileUpdate();
   void fileCancel();
};

#endif



