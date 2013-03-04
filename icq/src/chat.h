/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2012-2013 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQICQ_CHAT_H
#define LICQICQ_CHAT_H

#include <licq/icq/chat.h>

#include <deque>
#include <list>
#include <pthread.h>
#include <string>

#include <licq/pipe.h>
#include <licq/socketmanager.h>

#include "socket.h"

namespace LicqIcq
{
class CPChat_ColorFond;
class ChatManager;
class User;

void* ChatManager_tep(void*);
void* ChatWaitForSignal_tep(void*);
void ChatWaitForSignal_cleanup(void*);


struct SVoteInfo
{
  unsigned long nUin;
  unsigned short nNumUsers;
  unsigned short nYes;
  unsigned short nNo;
};

typedef std::list<SVoteInfo*> VoteInfoList;


class ChatClient
{
public:
  ChatClient();
  ChatClient(const User* u);
  ChatClient(const ChatClient &);
  ChatClient& operator=(const ChatClient &);
  ~ChatClient();

  // Initialize from the handshake buffer (does not set the session
  // or port fields however)
  bool LoadFromHandshake_v2(Licq::Buffer&);
  bool LoadFromHandshake_v4(Licq::Buffer&);
  bool LoadFromHandshake_v6(Licq::Buffer&);
  bool LoadFromHandshake_v7(Licq::Buffer&);

  unsigned long m_nVersion;
  unsigned short m_nPort;
  unsigned long myUin;
  unsigned long m_nIp;
  unsigned long m_nIntIp;
  char m_nMode;
  unsigned short m_nSession;
  unsigned long m_nHandshake;
  unsigned short m_nId;

protected:
  ChatClient(Licq::Buffer&);
  bool LoadFromBuffer(Licq::Buffer&);

  friend class CPChat_ColorFont;
};

typedef std::list<ChatClient> ChatClientList;
typedef std::list<ChatClient *> ChatClientPList;


class ChatUser : public Licq::IcqChatUser
{
public:
  ChatUser();
  ~ChatUser();

  ChatClient* m_pClient;
  DcSocket sock;
  std::deque<unsigned char> chatQueue;
  unsigned short state;
  std::string myLinebuf;

  pthread_mutex_t mutex;
};

struct SChatReverseConnectInfo
{
  int nId;
  bool bTryDirect;
  ChatUser* u;
  ChatManager* m;
};

typedef std::list<ChatUser*> ChatUserList;
typedef std::list<Licq::IcqChatEvent*> ChatEventList;
typedef std::list<ChatManager*> ChatManagerList;
typedef std::list<pthread_t> ThreadList;

class ChatManager : public Licq::IcqChatManager
{
public:
  ChatManager(const Licq::UserId& userId);
  ~ChatManager();

  static ChatManager* FindByPort(unsigned short);
  void AcceptReverseConnection(DcSocket*);

  // From Licq::IcqChatManager
  void init(const std::string& fontFamily = "courier",
     unsigned char fontEncoding = Licq::ENCODING_DEFAULT,
     unsigned char fontStyle = Licq::STYLE_DONTCARE | Licq::STYLE_DEFAULTxPITCH,
     unsigned short fontSize = 12, bool fontBold = false,
     bool fontItalic = false, bool fontUnderline = false,
     bool fontStrikeOut = false, int fr = 0xFF, int fg = 0xFF,
     int fb = 0xFF, int br = 0x00, int bg = 0x00, int bb = 0x00);

  bool StartAsServer();
  void StartAsClient(unsigned short nPort);
  void CloseChat();
  std::string clientsString() const;
  unsigned short ConnectedUsers() const;
  unsigned short LocalPort() const;
  void changeFontFamily(const std::string& fontFamily, unsigned char, unsigned char);
  void ChangeFontSize(unsigned short);
  void ChangeFontFace(bool, bool, bool, bool);
  void ChangeColorFg(int, int, int);
  void ChangeColorBg(int, int, int);
  void SendBeep();
  void SendLaugh();
  void SendNewline();
  void SendBackspace();
  void sendText(const std::string& text);
  void SendKick(const char* id);
  void SendKickNoVote(const char* id);
  void SendVoteYes(unsigned long);
  void SendVoteNo(unsigned long);
  void FocusOut();
  void FocusIn();
  void Sleep(bool);
  Licq::IcqChatEvent* PopChatEvent();
  int Pipe();

private:
  static ChatManagerList cmList;
  static pthread_mutex_t cmList_mutex;
  static pthread_mutex_t waiting_thread_cancel_mutex;

  Licq::Pipe myEventsPipe;
  Licq::Pipe myThreadPipe;
  Licq::UserId myUserId;
  unsigned short m_nSession;
  ChatUserList chatUsers;
  ChatUserList chatUsersClosed;
  ChatEventList chatEvents;
  VoteInfoList voteInfo;
  ThreadList waitingThreads;
  pthread_mutex_t thread_list_mutex;
  pthread_t thread_chat;
  ChatClient* m_pChatClient;

  Licq::TCPSocket chatServer;

  Licq::SocketManager sockman;
  bool m_bThreadCreated;

  bool StartChatServer();
  bool ConnectToChat(ChatClient *);
  bool SendChatHandshake(ChatUser*);
  ChatUser* FindChatUser(int);
  void CloseClient(ChatUser*);
  bool ProcessPacket(ChatUser*);
  bool ProcessRaw(ChatUser*);
  bool ProcessRaw_v2(ChatUser*);
  bool ProcessRaw_v6(ChatUser*);
  void PushChatEvent(Licq::IcqChatEvent*);
  void FinishKickVote(VoteInfoList::iterator, bool);

  void SendBuffer(Licq::Buffer*, unsigned char,
      const char* id = NULL, bool bNotIter = true);
  bool SendBufferToClient(Licq::Buffer*, unsigned char, ChatUser*);
  void SendBuffer_Raw(Licq::Buffer*);
  //void SendPacket(Licq::Packet*);

  std::string getEncoding(int chatEncoding);
  std::string userEncoding(const ChatUser* u);

  friend void *ChatManager_tep(void *);
  friend void *ChatWaitForSignal_tep(void *);
  friend void ChatWaitForSignal_cleanup(void *);
};

} // namespace LicqIcq

#endif
