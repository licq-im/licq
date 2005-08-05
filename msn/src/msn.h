/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// written by Jon Keating <jon@licq.org>

#ifndef __MSN_H
#define __MSN_H

#include "licq_events.h"
#include "licq_icqd.h"
#include "licq_socket.h"

#include "msnbuffer.h"

#define MSN_PPID 0x4D534E5F
#define L_MSNxSTR "[MSN] "

const char CONTACT_LIST[] = "FL";
const char ALLOW_LIST[] = "AL";
const char BLOCK_LIST[] = "BL";

const unsigned short FLAG_CONTACT_LIST = 1;
const unsigned short FLAG_ALLOW_LIST   = 2;
const unsigned short FLAG_BLOCK_LIST   = 4;
const unsigned short FLAG_REVERSE_LIST = 8;

#include <string>
#include <list>
#include <vector>
#include <cctype>
#include <pthread.h>

using std::string;
using std::list;
using std::vector;
using std::isalnum;

class CMSNPacket;

struct SBuffer
{
  CMSNBuffer *m_pBuf;
  string m_strUser;
};

typedef list<SBuffer *> BufferList;

struct SStartMessage
{
  CMSNPacket *m_pPacket;
  ICQEvent *m_pEvent;
  CICQSignal *m_pSignal;
  char *m_szUser;
  unsigned long m_nSeq;
};

typedef list<SStartMessage *> StartList;

class CMSN
{
public:
  CMSN(CICQDaemon *, int);
  ~CMSN();
  
  void Run();
  
  void MSNPing();
  bool Connected() { return m_nServerSocket != -1; }
  void MSNLogoff(bool = false);
  
  bool WaitingPingReply()          { return m_bWaitingPingReply; }
  void SetWaitingPingReply(bool b) { m_bWaitingPingReply = b; }
private:
  void ProcessSignal(CSignal *);
  void ProcessPipe();
  void ProcessServerPacket(CMSNBuffer &);
  void ProcessNexusPacket(CMSNBuffer &);
  void ProcessSSLServerPacket(CMSNBuffer &);
  void ProcessSBPacket(char *, CMSNBuffer *, int);
  
  // Network functions
  void SendPacket(CMSNPacket *);
  void Send_SB_Packet(string &, CMSNPacket *, int = -1, bool = true);
  void MSNLogon(const char *, int, unsigned long);
  void MSNGetServer();
  void MSNAuthenticateRedirect(string &, string &);
  void MSNAuthenticate(char *);
  bool MSNSBConnectStart(string &, string &);
  bool MSNSBConnectAnswer(string &, string &, string &, string &);
  
  void MSNSendMessage(char *, char *, pthread_t, unsigned long);
  void MSNSendTypingNotification(char *, unsigned long);
  void MSNChangeStatus(unsigned long);
  void MSNAddUser(char *);
  void MSNRemoveUser(char *);
  void MSNRenameUser(char *);
  void MSNGrantAuth(char *);
  void MSNUpdateUser(char *);
  void MSNBlockUser(char *);
  void MSNUnblockUser(char *);
  
  // Internal functions
  int HashValue(int n) { return n % 211; }
  void StorePacket(SBuffer *, int);
  void RemovePacket(string, int);
  SBuffer *RetrievePacket(string, int);
  ICQEvent *RetrieveEvent(unsigned long);
  unsigned long SocketToCID(int);
  static string Decode(const string &);
  static string Encode(const string &);
  
  // Config
  unsigned long m_nListVersion;
    
  // Variables
  CICQDaemon *m_pDaemon;
  bool m_bExit;
  int m_nPipe;
  int m_nServerSocket;
  int m_nNexusSocket;
  int m_nSSLSocket;
  CMSNBuffer *m_pPacketBuf,
             *m_pNexusBuff,
             *m_pSSLPacket;
  vector<BufferList> m_vlPacketBucket;
  list<ICQEvent *> m_pEvents;
  StartList m_lStart;
  bool m_bWaitingPingReply;
  
  // Server variables
  unsigned long m_nStatus,
                m_nSessionStart;
  string m_strMSPAuth,
         m_strSID,
         m_strKV;
         
  pthread_t m_tMSNPing;
  pthread_mutex_t mutex_StartList;
  
  char *m_szUserName,
       *m_szPassword,
       *m_szCookie;
};

extern CSocketManager gSocketMan;

#endif // __MSN_H
