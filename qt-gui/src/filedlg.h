#ifndef FILEDLG_H
#define FILEDLG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qpushbutton.h>
#include <qlabel.h>
#include <qprogressbar.h>
#include <qtimer.h>
#include <qsocketnotifier.h>

#include "user.h"
#include "socket.h"
#include "buffer.h"
#include "ewidgets.h"

struct SFileInfo
{
   char szName[MAX_FILENAME_LEN];
   unsigned long nSize;
};


class CFileDlg : public QWidget
{
   Q_OBJECT
public:
   CFileDlg(unsigned long _nUin,
            const char *_szRemoteFileName, unsigned long _nFileSize,
            bool _bServer, unsigned short _nPort,
            QWidget *parent = NULL, char *name = NULL);
   virtual ~CFileDlg(void);

   bool startAsClient(void);
   bool startAsServer(void);

   unsigned short getPort(void)  { return m_nPort; };
   unsigned short getLocalPort(void)  { return m_xSocketFile.LocalPort(); };
   void setPort(unsigned short _nPort)  { m_nPort = _nPort; };
   unsigned long Uin(void)  { return m_nUin; };
   bool IsServer(void)  { return m_bServer; };

public slots:
   virtual void hide();

protected:
   void resizeEvent (QResizeEvent *);

   QLabel *lblTransferFileName, *lblLocalFileName, *lblFileSize;
   CInfoField *nfoFileSize,
              *nfoTransferFileName,
              *nfoTotalFiles,
              *nfoBatchSize,
              *nfoLocalFileName,
              *nfoTrans,
              *nfoBPS,
              *nfoETA,
              *nfoTime,
              *nfoBatchTrans,
              *nfoBatchBPS,
              *nfoBatchETA,
              *nfoBatchTime;
   QPushButton *btnCancel;
   QLabel *lblStatus;
   QProgressBar *barTransfer, *barBatchTransfer;

   unsigned long m_nUin;
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

   bool GetLocalFileName(void);

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



