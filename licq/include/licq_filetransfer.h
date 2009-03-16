#ifndef FILET_H
#define FILET_H

/*---------------------------------------------------------------------------
 * FileTransfer Manager - CFileTransferManager
 *
 * This class is used to handle file transfers from plugins.  It is much
 * the chat manager only a lot simpler.  It is used in the following manner:
 *
 * 1.  Construct a CFileTransferManager object.  The constructor just takes
 *     a pointer to the Licq daemon and the UIN of the user being sent or
 *     received from.
 *     Listen on the CFileTransferManager::Pipe() for a signal that an event
 *     is available for processing.
 *     To receive automatic notification every x seconds (removes the need
 *     for a seperate timer in the plugin) call SetUpdatesEnabled(x) where
 *     x is the interval (2 seconds is good).  This will cause an FT_UPDATE
 *     signal to be sent every x seconds while a transfer is going on.
 * 2a. To receive files, call CFileTransferManager::ReceiveFiles().
 * 2b. To send files, call CFileTransferManager::SendFiles(<files>, <port>),
 *     where <files> is a ConstFileList of files to send, and <port> is the
 *     port of the remote user to connect to (taken from CExtendedAck::Port()
 *     from the file transfer request acceptance).
 * 3.  Process the events using PopFileTransferEvent().  Each event contains
 *     a command and possibly a string as well:
 *     FT_STARTxTRANSFER: Signals the start of the transfer.  At this point
 *       certain values can be extracted from the FileManager, including the
 *       number of files, the total size, the name of the remote host...
 *     FT_STARTxFILE: Signals the start of an actual file being sent.  At
 *       this point more values can be found, including the file name, file
 *       size.  Also, from this point until receiving FT_DONExFILE the
 *       current position and bytes remaining are always available from the
 *       CFileTransferManager.  The data will be the name of the file being
 *       sent, but this information is also available directly from the
 *       CFileTransferManager.
 *     FT_DONExFILE: Signals the successful completion of the file.  The
 *       data argument will be the full pathname of the file.
 *     FT_DONExTRANSFER: Signals that the transfer is done and the other side
 *       has disconnected.
 *     FT_ERRORx<...>: This means some kind of error has occured either
 *       reading/writing to files or talking to the network.  More details
 *       are available in the log.  The type of error is also specified
 *       as FT_ERRORxFILE (file read/write error, PathName() contains the
 *       name of the offending file), FT_ERRORxHANDSHAKE (handshaking error
 *       by the other side), FT_ERRORxCLOSED (the remote side closed
 *       the connection unexpectedly), FT_ERRORxCONNECT (error reaching
 *       the remote host), FT_ERRORxBIND (error binding a port when D_RECEIVER)
 *       or FT_ERRORxRESOURCES (error creating a new thread).
 * 4.  Call CloseFileTransfer() when done or to cancel, or simply delete the
 *       CFileTransferManager object.
 *
 * Note that if a file already exists, it will be automatically appended
 * to, unless it is the same size or greater then the incoming file, in
 * which case the file will be saved as <filename>.<timestamp>
 *-------------------------------------------------------------------------*/

#include <list>

// Order of declaration is significant here!
typedef std::list<char *> FileList;
typedef std::list<const char *> ConstFileList;

#include "licq_packets.h"
#include "licq_socket.h"
class CICQDaemon;


// FileTransferEvent codes
const unsigned char FT_STARTxBATCH   = 1;
const unsigned char FT_STARTxFILE    = 2;
const unsigned char FT_UPDATE        = 3;
const unsigned char FT_DONExFILE     = 4;
const unsigned char FT_DONExBATCH    = 5;
const unsigned char FT_CONFIRMxFILE  = 6;
const unsigned char FT_ERRORxFILE      = 0xFF;
const unsigned char FT_ERRORxHANDSHAKE = 0xFE;
const unsigned char FT_ERRORxCLOSED    = 0xFD;
const unsigned char FT_ERRORxCONNECT   = 0xFC;
const unsigned char FT_ERRORxBIND      = 0xFB;
const unsigned char FT_ERRORxRESOURCES = 0xFA;

class CFileTransferManager;

struct SFileReverseConnectInfo
{
  int nId;
  bool bTryDirect;
  CFileTransferManager *m;
};

//=====File=====================================================================
class CPacketFile : public CPacket
{
public:
  CPacketFile();
  virtual ~CPacketFile();

  virtual unsigned short Sequence()    { return 0; };
  virtual unsigned short SubSequence() { return 0; };
  virtual unsigned short Command()     { return 0; };
  virtual unsigned short SubCommand()  { return 0; };
protected:
   void InitBuffer()   { buffer = new CBuffer(m_nSize); };
};

//-----File_InitClient----------------------------------------------------------
/* 00 00 00 00 00 01 00 00 00 45 78 00 00 64 00 00 00 08 00 38 35 36 32 30
   30 30 00 */
class CPFile_InitClient : public CPacketFile
{
public:
  CPFile_InitClient(char *_szLocalName, unsigned long _nNumFiles,
                    unsigned long _nTotalSize);
  virtual ~CPFile_InitClient();
};


//-----File_InitServer----------------------------------------------------------
/* 01 64 00 00 00 08 00 38 35 36 32 30 30 30 00 */
class CPFile_InitServer : public CPacketFile
{
public:
  CPFile_InitServer(char *_szLocalName);
  virtual ~CPFile_InitServer();
};


//-----File_Info---------------------------------------------------------------
/* 02 00 0D 00 63 75 72 72 65 6E 74 2E 64 69 66 66 00 01 00 00 45 78 00 00
   00 00 00 00 64 00 00 00 */
class CPFile_Info : public CPacketFile
{
public:
  CPFile_Info(const char *_szFileName);
  virtual ~CPFile_Info();

  bool IsValid()  { return m_bValid; };
  unsigned long GetFileSize()
    { return m_nFileSize; };
  const char *GetFileName()
    { return m_szFileName; }
  const char *ErrorStr()
    { return strerror(m_nError); }
protected:
  bool m_bValid;
  int m_nError;
  char *m_szFileName;
  unsigned long m_nFileSize;
};


//-----File_Start---------------------------------------------------------------
/* 03 00 00 00 00 00 00 00 00 64 00 00 00 */
class CPFile_Start : public CPacketFile
{
public:
  CPFile_Start(unsigned long nFilePos, unsigned long nFile);
  virtual ~CPFile_Start();
};


//-----File_SetSpeed---------------------------------------------------------------
/* 03 00 00 00 00 00 00 00 00 64 00 00 00 */
class CPFile_SetSpeed : public CPacketFile
{
public:
  CPFile_SetSpeed(unsigned long nSpeed);
  virtual ~CPFile_SetSpeed();
};


//=====FileTransferManager===================================================
extern "C"
{
  void *FileTransferManager_tep(void *);
  void *FileWaitForSignal_tep(void *);
  void FileWaitForSignal_cleanup(void *);
}

class CFileTransferEvent
{
public:
  unsigned char Command() { return m_nCommand; }
  const char *Data() { return m_szData; }

  ~CFileTransferEvent();

protected:
  CFileTransferEvent(unsigned char t, char *d = NULL);
  unsigned char m_nCommand;
  char *m_szData;

friend class CFileTransferManager;
};

typedef std::list<CFileTransferEvent *> FileTransferEventList;
typedef std::list<class CFileTransferManager *> FileTransferManagerList;

class CFileTransferManager
{
public:
  CFileTransferManager(CICQDaemon* d, const char* accountId);
  ~CFileTransferManager();

  bool ReceiveFiles(const char *szDirectory);
  void SendFiles(ConstFileList lPathNames, unsigned short nPort);

  void CloseFileTransfer();

  // Available after construction
  uint16_t LocalPort() const            { return ftServer.getLocalPort(); }
  const char *LocalName()  { return m_szLocalName; }
  direction Direction() { return m_nDirection; }
  const char* Id() const { return myId; }

  // Available after FT_STARTxBATCH
  const char *RemoteName()  { return m_szRemoteName; }
  unsigned short BatchFiles() { return m_nBatchFiles; }
  unsigned long BatchSize() { return m_nBatchSize; }
  time_t BatchStartTime() { return m_nBatchStartTime; }

  // Available between FT_CONFIRMxFILE and FT_STATE_

  // You must use this function to start receiving the incoming file, possibly
  // giving it a different name on the local machine.
  bool StartReceivingFile(char *szFileName = NULL);

  // Available between FT_STARTxFILE and FT_DONExFILE
  unsigned long FilePos() { return m_nFilePos; }
  unsigned long BytesTransfered() { return m_nBytesTransfered; }
  unsigned long FileSize() { return m_nFileSize; }
  time_t StartTime() { return m_nStartTime; }
  const char *FileName() { return m_szFileName; }
  const char *PathName() { return m_szPathName; }

  // Batch information, available after first FT_STARTxFILE
  unsigned short CurrentFile() { return m_nCurrentFile; }
  unsigned long BatchBytesTransfered() { return m_nBatchBytesTransfered; }
  unsigned long BatchPos() { return m_nBatchPos; }

  void ChangeSpeed(unsigned short);
  void SetUpdatesEnabled(unsigned short n) { m_nUpdatesEnabled = n; }
  unsigned short UpdatesEnabled(void) { return m_nUpdatesEnabled; }

  int Pipe() { return pipe_events[PIPE_READ]; }
  CFileTransferEvent *PopFileTransferEvent();

  void AcceptReverseConnection(TCPSocket *);
  static CFileTransferManager *FindByPort(unsigned short);

protected:
  static FileTransferManagerList ftmList;

  static pthread_mutex_t thread_cancel_mutex;
  bool m_bThreadRunning;
  pthread_t m_tThread;

  CICQDaemon *licqDaemon;
  int pipe_events[2], pipe_thread[2];
  FileTransferEventList ftEvents;
  pthread_t thread_ft;
  FileList m_lPathNames;
  direction m_nDirection;

  struct timeval tv_lastupdate;
  unsigned short m_nUpdatesEnabled;

  unsigned char m_nResult;
  unsigned short m_nSession, m_nSpeed, m_nState;
  char myId[16];

  char m_szLocalName[64], m_szRemoteName[64];
  unsigned short m_nPort;
  unsigned long m_nFilePos, m_nBatchPos, m_nBytesTransfered, m_nBatchBytesTransfered;
  unsigned short m_nCurrentFile, m_nBatchFiles;
  unsigned long m_nFileSize, m_nBatchSize;
  time_t m_nStartTime, m_nBatchStartTime;
  char m_szFileName[128], m_szPathName[MAX_FILENAME_LEN];

  char m_szDirectory[MAX_FILENAME_LEN];
  int m_nFileDesc;
  FileList::iterator m_iPathName;
  bool m_bThreadCreated;

  TCPSocket ftSock, ftServer;

  CSocketManager sockman;

  bool StartFileTransferServer();
  bool ConnectToFileServer(unsigned short nPort);
  bool SendFileHandshake();
  bool ProcessPacket();
  bool SendFilePacket();
  void PushFileTransferEvent(unsigned char);
  void PushFileTransferEvent(CFileTransferEvent *);
  void CloseConnection();

  bool SendBuffer(CBuffer *);
  bool SendPacket(CPacket *);

friend void *FileTransferManager_tep(void *);
friend void *FileWaitForSignal_tep(void *);
friend void FileWaitForSignal_cleanup(void *);
};



#endif
