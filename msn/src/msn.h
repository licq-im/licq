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

struct SStartMessage
{
  CMSNPacket *m_pPacket;
  ICQEvent *m_pEvent;
  CICQSignal *m_pSignal;
  char *m_szUser;
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
  
private:
  void ProcessSignal(CSignal *);
  void ProcessPipe();
  void ProcessServerPacket(CMSNBuffer &);
  void ProcessNexusPacket(CMSNBuffer &);
  void ProcessSSLServerPacket(CMSNBuffer &);
  void ProcessSBPacket(char *, CMSNBuffer *);
  
  // Network functions
  void SendPacket(CMSNPacket *);
  void Send_SB_Packet(string &, CMSNPacket *, bool = true);
  void MSNLogon(const char *, int);
  void MSNGetServer();
  void MSNAuthenticateRedirect(string &, string &);
  void MSNAuthenticate(char *);
  bool MSNSBConnectStart(string &, string &);
  bool MSNSBConnectAnswer(string &, string &, string &, string &);
  
  void MSNSendMessage(char *, char *, pthread_t);
  void MSNSendTypingNotification(char *);
  void MSNChangeStatus(unsigned long);
  void MSNLogoff();
  void MSNAddUser(char *);
  void MSNRemoveUser(char *);
  void MSNRenameUser(char *);
  void MSNGrantAuth(char *);
  
  // Internal functions
  int HashValue(int n) { return n % 211; }
  void StorePacket(SBuffer *, int);
  void RemovePacket(string, int);
  SBuffer *RetrievePacket(string, int);
  ICQEvent *RetrieveEvent(unsigned long);
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
  
  pthread_t m_tMSNPing;
  
  char *m_szUserName,
       *m_szPassword,
       *m_szCookie;
};

#endif // __MSN_H
