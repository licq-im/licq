#ifndef FILET_H
#define FILET_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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
 *     FT_CLOSED: This means that the other side disconnected unexpectedly.
 *     FT_ERRORx<...>: This means some kind of I/O error has occured either
 *       reading/writing to files or talking to the network.  More details
 *       are available in the log.  The type of error is also specified
 *       as FT_ERRORxFILE (file read/write error, PathName() contains the
 *       name of the offending file) or FT_ERRORxHANDSHAKE (handshaking error
 *       by the other side).
 * 4.  Call CloseFileTransfer() when done or to cancel, or simply delete the
 *       CFileTransferManager object.
 *
 * Note that if a file already exists, it will be automatically appended
 * to, unless it is the same size or greater then the incoming file, in
 * which case the file will be saved as <filename>.<timestamp>
 *-------------------------------------------------------------------------*/

#include "licq_packets.h"
class CICQDaemon;


// FileTransferEvent codes
const unsigned char FT_STARTxBATCH   = 1;
const unsigned char FT_STARTxFILE    = 2;
const unsigned char FT_DONExFILE     = 3;
const unsigned char FT_DONExBATCH    = 4;
const unsigned char FT_CLOSED        = 5;
const unsigned char FT_ERRORxFILE      = 0xFF;
const unsigned char FT_ERRORxHANDSHAKE = 0xFE;


//=====File=====================================================================
class CPacketFile : public CPacket
{
public:
  virtual const unsigned long  getSequence()   { return 0; };
  virtual const unsigned short SubSequence()   { return 0; };
  virtual const unsigned short getCommand()    { return 0; };
  virtual const unsigned short getSubCommand() { return 0; };
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
};


//-----File_InitServer----------------------------------------------------------
/* 01 64 00 00 00 08 00 38 35 36 32 30 30 30 00 */
class CPFile_InitServer : public CPacketFile
{
public:
   CPFile_InitServer(char *_szLocalName);
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
   CPFile_Start(unsigned long _nFilePos);
};


//=====FileTransferManager===================================================
void *FileTransferManager_tep(void *);

class CFileTransferEvent
{
public:
  unsigned char Command() { return m_nCommand; }
  const char *Data() { return m_szData; }

  ~CFileTransferEvent() { if (m_szData != NULL) free(m_szData); }

protected:
  CFileTransferEvent(unsigned char t, char *d = NULL);
  unsigned char m_nCommand;
  char *m_szData;

friend class CFileTransferManager;
};

typedef list<CFileTransferEvent *> FileTransferEventList;
typedef list<const char *> ConstFileList;
typedef list<char *> FileList;


class CFileTransferManager
{
public:
  CFileTransferManager(CICQDaemon *d, unsigned long nUin);
  ~CFileTransferManager();

  bool ReceiveFiles(const char *szDirectory);
  bool SendFiles(ConstFileList lPathNames, unsigned short nPort);

  void CloseFileTransfer();

  // Available after construction
  unsigned short LocalPort() { return ftServer.LocalPort(); }
  const char *LocalName()  { return m_szLocalName; }
  direction Direction() { return m_nDirection; }

  // Available after FT_STARTxBATCH
  const char *RemoteName()  { return m_szRemoteName; }
  unsigned short BatchFiles() { return m_nBatchFiles; }
  unsigned long BatchSize() { return m_nBatchSize; }
  time_t BatchStartTime() { return m_nBatchStartTime; }

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

  int Pipe() { return pipe_events[PIPE_READ]; }
  CFileTransferEvent *PopFileTransferEvent();

protected:
  CICQDaemon *licqDaemon;
  int pipe_events[2], pipe_thread[2];
  FileTransferEventList ftEvents;
  pthread_t thread_ft;
  FileList m_lPathNames;
  direction m_nDirection;

  unsigned char m_nResult;
  unsigned long m_nUin;
  unsigned short m_nSession, m_nSpeed, m_nState;

  char m_szLocalName[64], m_szRemoteName[64];
  unsigned long m_nFilePos, m_nBatchPos, m_nBytesTransfered, m_nBatchBytesTransfered;
  unsigned short m_nCurrentFile, m_nBatchFiles;
  unsigned long m_nFileSize, m_nBatchSize;
  time_t m_nStartTime, m_nBatchStartTime;
  char m_szFileName[128], m_szPathName[MAX_FILENAME_LEN];

  char m_szDirectory[MAX_FILENAME_LEN];
  int m_nFileDesc;
  FileList::iterator m_iPathName;

  TCPSocket ftSock, ftServer;

  CSocketManager sockman;

  bool StartFileTransferServer();
  bool ConnectToFileServer(unsigned short nPort);
  bool ProcessPacket();
  bool SendFilePacket();
  void PushFileTransferEvent(unsigned char);
  void PushFileTransferEvent(CFileTransferEvent *);
  void CloseConnection();

  bool SendBuffer(CBuffer *);
  bool SendPacket(CPacket *);

friend void *FileTransferManager_tep(void *);

};



#endif
