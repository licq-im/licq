/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2004-2013 Licq developers <licq-dev@googlegroups.com>
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

#include <licq/plugin/protocolpluginhelper.h>

#include <list>
#include <string>
#include <vector>

#include <licq/mainloop.h>
#include <licq/userid.h>

#include "msnbuffer.h"
#include "msnevent.h"

namespace Licq
{
class Event;
class INetSocket;
class TCPSocket;
class User;
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
  Licq::UserId myUserId;
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

struct TypingTimeout
{
  int timeoutId;
  Licq::UserId userId;
  unsigned long convoId;
};

typedef std::list<TypingTimeout> TypingTimeoutList;

class CMSN : public Licq::ProtocolPluginHelper, public Licq::MainLoopCallback
{
public:
  CMSN();
  ~CMSN();

  // From Licq::PluginInterface
  int run();

  std::string defaultServerHost() const;
  int defaultServerPort() const;

  bool Connected() { return myServerSocket != NULL; }
  void Logon(const Licq::UserId& ownerId, unsigned status, std::string host = std::string(), int port = 0);
  void MSNLogoff(bool = false);

  void closeSocket(Licq::TCPSocket* sock, bool clearUser = true);

private:
  // From Licq::MainLoopCallback
  void rawFileEvent(int fd, int revents);
  void socketEvent(Licq::INetSocket* inetSocket, int revents);
  void timeoutEvent(int id);

  void ProcessSignal(const Licq::ProtocolSignal* s);
  void ProcessPipe();
  void ProcessServerPacket(CMSNBuffer *);
  void ProcessSSLServerPacket(CMSNBuffer &);
  void ProcessSBPacket(const Licq::UserId& socketUserId, CMSNBuffer*,
      Licq::TCPSocket* sock);

  // Network functions
  void sendServerPing();
  void SendPacket(CMSNPacket *);
  void Send_SB_Packet(const Licq::UserId& userId, CMSNPacket* p, Licq::TCPSocket* sock,
      bool bDelete = true);
  void MSNAuthenticate(const std::string& host = "loginnet.passport.com",
      const std::string& path = "/login2.srf");
  bool MSNSBConnectStart(const std::string& server, const std::string& cookie);
  bool MSNSBConnectAnswer(const std::string& server, const std::string& sessionId,
      const std::string& cookie, const Licq::UserId& userId);

  void MSNSendInvitation(const Licq::UserId& userId, CMSNPacket* _pPacket);
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
  void RemovePacket(const Licq::UserId& userId, int socketId, int size = 0);
  SBuffer* RetrievePacket(const Licq::UserId& userId, int socketId);
  Licq::Event* RetrieveEvent(unsigned long);
  void HandlePacket(Licq::TCPSocket* sock, CMSNBuffer&, const Licq::UserId& userId);
  unsigned long SocketToCID(int);
  static std::string Decode(const std::string& strIn);
  static std::string Encode(const std::string& strIn);
  void WaitDataEvent(CMSNDataEvent *);
  bool RemoveDataEvent(CMSNDataEvent *);
  CMSNDataEvent* FetchDataEvent(const Licq::UserId& userId, Licq::TCPSocket* sock);
  CMSNDataEvent* FetchStartDataEvent(const Licq::UserId& userId);

  /**
   * Kill a conversation and all users associated with it
   * Called when a new socket is opened to make sure it isn't associated with any old conversations
   *
   * @param sock Socket to clear conversations for
   */
  void killConversation(Licq::TCPSocket* sock);

  /**
   * Get a unique id for scheudling a timeout callback
   */
  int getNextTimeoutId();

  /**
   * Update typing status for a user
   *
   * @param u User object
   * @param typing New typing status
   * @param cid Conversation id
   */
  void setIsTyping(Licq::User* u, bool typing, unsigned long cid);

  /**
   * Update our typing status towards a user
   *
   * @param u User object
   * @param typing New typing status
   * @param cid Conversation id
   */
  void sendIsTyping(const Licq::UserId& userId, bool typing, unsigned long cid);

  /**
   * Handle timeouts for typing notifications
   *
   * @param id Timeout id from mainloop
   */
  void typingTimeout(int id);

  // Variables
  Licq::UserId myOwnerId;
  Licq::MainLoop myMainLoop;
  Licq::TCPSocket* myServerSocket;
  Licq::TCPSocket* mySslSocket;
  CMSNBuffer *m_pPacketBuf,
             *m_pSSLPacket;
  std::vector<BufferList> m_vlPacketBucket;
  std::list<Licq::Event*> m_pEvents;
  std::list<CMSNDataEvent*> m_lMSNEvents;
  StartList m_lStart;
  bool m_bWaitingPingReply,
       m_bCanPing;
  TypingTimeoutList myUserTypingTimeouts;
  TypingTimeoutList myOwnerTypingTimeouts;
  int myNextTimeoutId;

  // Server variables
  unsigned myStatus;
  unsigned long m_nSessionStart;
  std::string m_strMSPAuth,
         m_strSID,
         m_strKV;
  std::string myCookie;
  std::string myPassword;

  friend class CMSNDataEvent;
};

} // namespace LicqMsn

#endif
