#ifndef __MSN_H
#define __MSN_H

#include "licq_events.h"
#include "licq_icqd.h"
#include "licq_socket.h"

#include "msnbuffer.h"

#define MSN_PPID 0x4D534E5F
#define L_MSNxSTR "[MSN] "

#include <string>
#include <list>
#include <vector>
#include <pthread.h>

using std::string;
using std::list;
using std::vector;

class CMSNPacket;

struct SBuffer
{
  CMSNBuffer *m_pBuf;
  string m_strUser;
};

typedef list<SBuffer *> BufferList;

class CMSN
{
public:
  CMSN(CICQDaemon *, int);
  ~CMSN();
  
  void Run();
  
  void MSNPing();
  bool Connected() { return m_nServerSocket != -1; }
  
private:
  void ProcessSignal(CSignal *);
  void ProcessPipe();
  void ProcessServerPacket(CMSNBuffer &);
  void ProcessSSLServerPacket(CMSNBuffer &);
  void ProcessSBPacket(char *, CMSNBuffer *);
  
  // Network functions
  void SendPacket(CMSNPacket *);
  void Send_SB_Packet(string &, CMSNPacket *, bool = true);
  void MSNLogon(const char *, int);
  void MSNAuthenticate(char *);
  bool MSNSBConnectAnswer(string &, string &, string &, string &);
  
  void MSNSendMessage(char *, char *, pthread_t);
 

  
  // Internal functions
  int HashValue(int n) { return n % 211; }
  void StorePacket(SBuffer *, int);
  void RemovePacket(string, int);
  SBuffer *RetrievePacket(string, int);
  ICQEvent *RetrieveEvent(unsigned long);
  
  // Variables
  CICQDaemon *m_pDaemon;
  int m_nPipe;
  int m_nServerSocket;
  int m_nSSLSocket;
  CMSNBuffer *m_pPacketBuf;
  vector<BufferList> m_vlPacketBucket;
  list<ICQEvent *> m_pEvents;
  pthread_t m_tMSNPing;
  
  char *m_szUserName,
       *m_szPassword;
};

#endif // __MSN_H
