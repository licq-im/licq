/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2004-2012 Licq developers <licq-dev@googlegroups.com>
 *
 * Licq is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Licq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Licq; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef LICQMSN_MSN_H
#define LICQMSN_MSN_H

#include <licq/plugin/protocolplugin.h>

#include <list>
#include <pthread.h>
#include <string>
#include <vector>

#include <licq/socketmanager.h>
#include <licq/userid.h>

#include "msnbuffer.h"
#include "msnevent.h"

namespace Licq
{
class Event;
}

namespace LicqMsn
{

const char CONTACT_LIST[] = "FL";
const char ALLOW_LIST[] = "AL";
const char BLOCK_LIST[] = "BL";

const unsigned short FLAG_CONTACT_LIST = 1;
const unsigned short FLAG_ALLOW_LIST   = 2;
const unsigned short FLAG_BLOCK_LIST   = 4;
const unsigned short FLAG_REVERSE_LIST = 8;

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
  Licq::Event* m_pEvent;
  Licq::UserId userId;
  unsigned long m_nSeq;
  bool m_bConnecting,
       m_bDataConnection;
};

typedef std::list<SStartMessage*> StartList;

class CMSN : public Licq::ProtocolPlugin
{
public:
  CMSN(Params& p);
  ~CMSN();

  // From Licq::ProtocolPlugin
  std::string name() const;
  std::string version() const;
  unsigned long protocolId() const;
  unsigned long capabilities() const;
  std::string defaultServerHost() const;
  int defaultServerPort() const;

  void MSNPing();
  bool Connected() { return m_nServerSocket != -1; }
  bool CanSendPing() { return m_bCanPing; }
  void Logon(unsigned status, std::string host = std::string(), int port = 0);
  void MSNLogoff(bool = false);
  unsigned status() const { return myStatus; }

  bool WaitingPingReply()          { return m_bWaitingPingReply; }
  void SetWaitingPingReply(bool b) { m_bWaitingPingReply = b; }

  pthread_mutex_t mutex_ServerSocket; // Ugly, but whatever.

protected:
  // From Licq::ProtocolPlugin
  bool init(int, char**);
  int run();
  void destructor();
  Licq::User* createUser(const Licq::UserId& id, bool temporary = false);
  Licq::Owner* createOwner(const Licq::UserId& id);

private:
  void ProcessSignal(Licq::ProtocolSignal* s);
  void ProcessPipe();
  void ProcessServerPacket(CMSNBuffer *);
  void ProcessNexusPacket(CMSNBuffer &);
  void ProcessSSLServerPacket(CMSNBuffer &);
  void ProcessSBPacket(char *, CMSNBuffer *, int);

  // Network functions
  void SendPacket(CMSNPacket *);
  void Send_SB_Packet(const Licq::UserId& userId, CMSNPacket* p, int nSocket = -1,
      bool bDelete = true);
  void MSNAuthenticate(const std::string& host = "loginnet.passport.com",
      const std::string& path = "/login2.srf");
  bool MSNSBConnectStart(const std::string& server, const std::string& cookie);
  bool MSNSBConnectAnswer(const std::string& server, const std::string& sessionId,
      const std::string& cookie, const std::string& user);

  void MSNSendInvitation(const char* _szUser, CMSNPacket* _pPacket);
  void MSNSendMessage(unsigned long eventId, const Licq::UserId& userId, const std::string& message,
      pthread_t _tPlugin, unsigned long _nCID);
  void MSNSendTypingNotification(const Licq::UserId& userId, unsigned long convoId);
  void MSNChangeStatus(unsigned status);
  void MSNAddUser(const Licq::UserId& userId);
  void MSNRemoveUser(const Licq::UserId& userId);
  void MSNRenameUser(const Licq::UserId& userId);
  void MSNGrantAuth(const Licq::UserId& userId);
  void MSNUpdateUser(const std::string& alias);
  void MSNBlockUser(const Licq::UserId& userId);
  void MSNUnblockUser(const Licq::UserId& userId);
  void MSNGetDisplayPicture(const Licq::UserId& userId, const std::string& msnObject);

  // Internal functions
  int HashValue(int n) { return n % 211; }
  void StorePacket(SBuffer *, int);
  void RemovePacket(const std::string& user, int socketId, int size = 0);
  SBuffer *RetrievePacket(const std::string& user, int socketId);
  Licq::Event* RetrieveEvent(unsigned long);
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

  // Variables
  bool m_bExit;
  int m_nServerSocket;
  int m_nNexusSocket;
  int m_nSSLSocket;
  CMSNBuffer *m_pPacketBuf,
             *m_pNexusBuff,
             *m_pSSLPacket;
  std::vector<BufferList> m_vlPacketBucket;
  std::list<Licq::Event*> m_pEvents;
  std::list<CMSNDataEvent*> m_lMSNEvents;
  StartList m_lStart;
  bool m_bWaitingPingReply,
       m_bCanPing;
  
  // Server variables
  unsigned myStatus;
  unsigned long m_nSessionStart;
  std::string m_strMSPAuth,
         m_strSID,
         m_strKV;
         
  pthread_t m_tMSNPing;
  pthread_mutex_t mutex_StartList,
                  mutex_MSNEventList,
                  mutex_Bucket;
    
  char *m_szUserName;
  std::string myCookie;
  std::string myPassword;

  friend class CMSNDataEvent;
};

} // namespace LicqMsn

extern Licq::SocketManager gSocketMan;

#endif
