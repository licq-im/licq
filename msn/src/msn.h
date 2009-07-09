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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <list>
#include <pthread.h>
#include <string>
#include <vector>

#include "licq_events.h"
#include "licq_icqd.h"
#include "licq_socket.h"

#include "msnbuffer.h"
#include "msnevent.h"

#define MSN_PPID 0x4D534E5F
#define L_MSNxSTR "[MSN] "

const char CONTACT_LIST[] = "FL";
const char ALLOW_LIST[] = "AL";
const char BLOCK_LIST[] = "BL";

const unsigned short FLAG_CONTACT_LIST = 1;
const unsigned short FLAG_ALLOW_LIST   = 2;
const unsigned short FLAG_BLOCK_LIST   = 4;
const unsigned short FLAG_REVERSE_LIST = 8;

const char MSN_DEFAULT_SERVER_ADDRESS[]         = "messenger.hotmail.com";
const unsigned short MSN_DEFAULT_SERVER_PORT    = 1863;

#ifndef HAVE_STRNDUP
char *strndup(const char *s, size_t n);
#endif


  const unsigned long MSN_DP_EVENT = 1;

class CMSNPacket;
class CMSNDataEvent;

struct SBuffer
{
  CMSNBuffer *m_pBuf;
  std::string m_strUser;
  bool m_bStored;
};

typedef std::list<SBuffer *> BufferList;

struct SStartMessage
{
  CMSNPacket *m_pPacket;
  ICQEvent *m_pEvent;
  LicqSignal *m_pSignal;
  char *m_szUser;
  unsigned long m_nSeq;
  bool m_bConnecting,
       m_bDataConnection;
};

typedef std::list<SStartMessage*> StartList;

class CMSN
{
public:
  CMSN(CICQDaemon *, int);
  ~CMSN();

  void Run();

  void MSNPing();
  bool Connected() { return m_nServerSocket != -1; }
  bool CanSendPing() { return m_bCanPing; }
  void MSNLogoff(bool = false);
  void MSNLogon(const char *, int);

  bool WaitingPingReply()          { return m_bWaitingPingReply; }
  void SetWaitingPingReply(bool b) { m_bWaitingPingReply = b; }

  const std::string& serverAddress() const   { return myServerAddress; }
  unsigned short serverPort() const     { return myServerPort; }

  pthread_mutex_t mutex_ServerSocket; // Ugly, but whatever.

private:
  void ProcessSignal(CSignal *);
  void ProcessPipe();
  void ProcessServerPacket(CMSNBuffer *);
  void ProcessNexusPacket(CMSNBuffer &);
  void ProcessSSLServerPacket(CMSNBuffer &);
  void ProcessSBPacket(char *, CMSNBuffer *, int);

  // Network functions
  void SendPacket(CMSNPacket *);
  void Send_SB_Packet(const std::string& user, CMSNPacket* p, int nSocket = -1,
      bool bDelete = true);
  void MSNLogon(const char *, int, unsigned long);
  void MSNGetServer();
  void MSNAuthenticateRedirect(const std::string& host, const std::string& param);
  void MSNAuthenticate(char *);
  bool MSNSBConnectStart(const std::string& server, const std::string& cookie);
  bool MSNSBConnectAnswer(const std::string& server, const std::string& sessionId,
      const std::string& cookie, const std::string& user);

  void MSNSendInvitation(const char* _szUser, CMSNPacket* _pPacket);
  void MSNSendMessage(const char* _szUser, const char* _szMsg,
      pthread_t _tPlugin, unsigned long _nCID);
  void MSNSendTypingNotification(const char* _szUser, unsigned long _nCID);
  void MSNChangeStatus(unsigned long);
  void MSNAddUser(const char* szUser);
  void MSNRemoveUser(const char* szUser);
  void MSNRenameUser(const char* szUser);
  void MSNGrantAuth(const char* szUser);
  void MSNUpdateUser(const char* szUser);
  void MSNBlockUser(const char* szUser);
  void MSNUnblockUser(const char* szUser);
  void MSNGetDisplayPicture(const std::string& user, const std::string& msnObject);

  // Internal functions
  int HashValue(int n) { return n % 211; }
  void StorePacket(SBuffer *, int);
  void RemovePacket(const std::string& user, int socketId, int size = 0);
  SBuffer *RetrievePacket(const std::string& user, int socketId);
  ICQEvent *RetrieveEvent(unsigned long);
  void HandlePacket(int, CMSNBuffer &, const char *);
  unsigned long SocketToCID(int);
  static std::string Decode(const std::string& strIn);
  static std::string Encode(const std::string& strIn);
  void WaitDataEvent(CMSNDataEvent *);
  bool RemoveDataEvent(CMSNDataEvent *);
  CMSNDataEvent* FetchDataEvent(const std::string& user, int socketId);
  CMSNDataEvent* FetchStartDataEvent(const std::string& user);

  /**
   * Kill a conversation and all users associated with it
   * Called when a new socket is opened to make sure it isn't associated with any old conversations
   *
   * @param sock Socket to clear conversations for
   */
  void killConversation(int sock);

  // Interface to CICQDaemon
  void pushPluginSignal(LicqSignal* p);

  // Config
  unsigned long m_nListVersion;
  std::string myServerAddress;
  unsigned short myServerPort;

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
  std::vector<BufferList> m_vlPacketBucket;
  std::list<ICQEvent*> m_pEvents;
  std::list<CMSNDataEvent*> m_lMSNEvents;
  StartList m_lStart;
  bool m_bWaitingPingReply,
       m_bCanPing;
  
  // Server variables
  unsigned long m_nStatus,
                m_nOldStatus,
                m_nSessionStart;
  std::string m_strMSPAuth,
         m_strSID,
         m_strKV;
         
  pthread_t m_tMSNPing;
  pthread_mutex_t mutex_StartList,
                  mutex_MSNEventList,
                  mutex_Bucket;
    
  char *m_szUserName,
       *m_szPassword,
       *m_szCookie;

  friend class CMSNDataEvent;
};

extern CSocketManager gSocketMan;

#endif // __MSN_H
