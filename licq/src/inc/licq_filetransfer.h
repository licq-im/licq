#ifndef FILET_H
#define FILET_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "licq_packets.h"
class CICQDaemon;


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

class CFileTransferEvent {};
typedef list<CFileTransferEvent *> FileTransferEventList;

class CFileTransferManager
{
public:
  CFileTransferManager(CICQDaemon *d, unsigned long nUin);
  ~CFileTransferManager();

  bool StartAsServer();
  bool StartAsClient(unsigned short nPort);

  void CloseFileTransfer();

  unsigned short LocalPort() { return ftSock.LocalPort(); }
  const char *LocalName()  { return m_szLocalName; }
  const char *RemoteName()  { return m_szRemoteName; }

  //void SendCharacter(char);
  void ChangeSpeed(unsigned short);

  int Pipe() { return pipe_events[PIPE_READ]; }
  CFileTransferEvent *PopFileTransferEvent();

protected:
  CICQDaemon *licqDaemon;
  int pipe_events[2], pipe_thread[2];
  FileTransferEventList ftEvents;
  pthread_t thread_ft;

  unsigned long m_nUin;
  unsigned short m_nSession, m_nSpeed, m_nState;
  char m_szLocalName[64], m_szRemoteName[64];

  unsigned long m_nFilePos, m_nBatchPos, m_nBytesTransfered, m_nBatchBytesTransfered;
  unsigned short m_nCurrentFile, m_nTotalFiles;
  unsigned long m_nFileSize, m_nBatchSize;
  time_t m_nStartTime, m_nBatchStartTime;
  int m_nFileDesc;
  char m_szPathName[MAX_FILENAME_LEN], m_szFileName[128];

  TCPSocket ftSock, ftServer;

  CSocketManager sockman;

  bool StartFileTransferServer();
  bool ConnectToFileServer(unsigned short nPort);
  bool ProcessPacket();
  bool SendFilePacket();
  void PushFileTransferEvent(CFileTransferEvent *);
  void CloseConnection();

  bool SendBuffer(CBuffer *);
  bool SendPacket(CPacket *);

friend void *FileTransferManager_tep(void *);

};



#endif
