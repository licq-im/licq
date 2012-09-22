/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2012 Licq developers <licq-dev@googlegroups.com>
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

#include <licq/protocolsignal.h>

using namespace Licq;
using std::list;
using std::string;


ProtocolSignal::ProtocolSignal(SignalType signal, const UserId& userId, unsigned long eventId)
: mySignal(signal),
  myUserId(userId),
  myEventId(eventId),
  myCallerThread(pthread_self())
{ /* Empty */ }

ProtocolSignal::~ProtocolSignal()
{ /* Empty */ }


ProtoLogonSignal::ProtoLogonSignal(const UserId& ownerId, unsigned status)
  : ProtocolSignal(SignalLogon, ownerId),
    myStatus(status)
{ /* Empty */ }

ProtoLogonSignal::~ProtoLogonSignal()
{ /* Empty */ }


ProtoLogoffSignal::ProtoLogoffSignal(const UserId& ownerId)
  : ProtocolSignal(SignalLogoff, ownerId)
{ /* Empty */ }

ProtoLogoffSignal::~ProtoLogoffSignal()
{ /* Empty */ }


ProtoChangeStatusSignal::ProtoChangeStatusSignal(const UserId& ownerId, unsigned status)
  : ProtocolSignal(SignalChangeStatus, ownerId),
    myStatus(status)
{ /* Empty */ }

ProtoChangeStatusSignal::~ProtoChangeStatusSignal()
{ /* Empty */ }


ProtoAddUserSignal::ProtoAddUserSignal(const UserId& userId)
  : ProtocolSignal(SignalAddUser, userId)
{ /* Empty */ }

ProtoAddUserSignal::~ProtoAddUserSignal()
{ /* Empty */ }


ProtoRemoveUserSignal::ProtoRemoveUserSignal(const UserId& userId)
  : ProtocolSignal(SignalRemoveUser, userId)
{ /* Empty */ }

ProtoRemoveUserSignal::~ProtoRemoveUserSignal()
{ /* Empty */ }


ProtoRenameUserSignal::ProtoRenameUserSignal(const UserId& userId)
  : ProtocolSignal(SignalRenameUser, userId)
{ /* Empty */ }

ProtoRenameUserSignal::~ProtoRenameUserSignal()
{ /* Empty */ }


ProtoChangeUserGroupsSignal::ProtoChangeUserGroupsSignal(const UserId& userId)
  : ProtocolSignal(SignalChangeUserGroups, userId)
{ /* Empty */ }

ProtoChangeUserGroupsSignal::~ProtoChangeUserGroupsSignal()
{ /* Empty */ }


ProtoSendMessageSignal::ProtoSendMessageSignal(unsigned long eventId, const UserId& userId,
    const string& message, unsigned flags, const Color* color,
    unsigned long convoId)
  : ProtocolSignal(SignalSendMessage, userId, eventId),
    myMessage(message),
    myFlags(flags),
    myColor(color),
    myConvoId(convoId)
{ /* Empty */ }

ProtoSendMessageSignal::~ProtoSendMessageSignal()
{ /* Empty */ }


ProtoTypingNotificationSignal::ProtoTypingNotificationSignal(const UserId& userId,
    bool active, unsigned long convoId)
  : ProtocolSignal(SignalNotifyTyping, userId),
    myActive(active),
    myConvoId(convoId)
{ /* Empty */ }

ProtoTypingNotificationSignal::~ProtoTypingNotificationSignal()
{ /* Empty */ }


ProtoGrantAuthSignal::ProtoGrantAuthSignal(unsigned long eventId, const UserId& userId,
    const string& message)
  : ProtocolSignal(SignalGrantAuth, userId, eventId),
    myMessage(message)
{ /* Empty */ }

ProtoGrantAuthSignal::~ProtoGrantAuthSignal()
{ /* Empty */ }


ProtoRefuseAuthSignal::ProtoRefuseAuthSignal(unsigned long eventId, const UserId& userId,
    const string& message)
  : ProtocolSignal(SignalRefuseAuth, userId, eventId),
    myMessage(message)
{ /* Empty */ }

ProtoRefuseAuthSignal::~ProtoRefuseAuthSignal()
{ /* Empty */ }


ProtoRequestInfo::ProtoRequestInfo(unsigned long eventId, const UserId& userId)
  : ProtocolSignal(SignalRequestInfo, userId, eventId)
{ /* Empty */ }

ProtoRequestInfo::~ProtoRequestInfo()
{ /* Empty */ }


ProtoUpdateInfoSignal::ProtoUpdateInfoSignal(unsigned long eventId, const UserId& ownerId)
  : ProtocolSignal(SignalUpdateInfo, ownerId, eventId)
{ /* Empty */ }

ProtoUpdateInfoSignal::~ProtoUpdateInfoSignal()
{ /* Empty */ }


ProtoRequestPicture::ProtoRequestPicture(unsigned long eventId, const UserId& userId)
  : ProtocolSignal(SignalRequestPicture, userId, eventId)
{ /* Empty */ }

ProtoRequestPicture::~ProtoRequestPicture()
{ /* Empty */ }


ProtoBlockUserSignal::ProtoBlockUserSignal(const UserId& userId)
  : ProtocolSignal(SignalBlockUser, userId)
{ /* Empty */ }

ProtoBlockUserSignal::~ProtoBlockUserSignal()
{ /* Empty */ }


ProtoUnblockUserSignal::ProtoUnblockUserSignal(const UserId& userId)
  : ProtocolSignal(SignalUnblockUser, userId)
{ /* Empty */ }

ProtoUnblockUserSignal::~ProtoUnblockUserSignal()
{ /* Empty */ }


ProtoAcceptUserSignal::ProtoAcceptUserSignal(const UserId& userId)
  : ProtocolSignal(SignalAcceptUser, userId)
{ /* Empty */ }

ProtoAcceptUserSignal::~ProtoAcceptUserSignal()
{ /* Empty */ }


ProtoUnacceptUserSignal::ProtoUnacceptUserSignal(const UserId& userId)
  : ProtocolSignal(SignalUnacceptUser, userId)
{ /* Empty */ }

ProtoUnacceptUserSignal::~ProtoUnacceptUserSignal()
{ /* Empty */ }


ProtoIgnoreUserSignal::ProtoIgnoreUserSignal(const UserId& userId)
  : ProtocolSignal(SignalIgnoreUser, userId)
{ /* Empty */ }

ProtoIgnoreUserSignal::~ProtoIgnoreUserSignal()
{ /* Empty */ }


ProtoUnignoreUserSignal::ProtoUnignoreUserSignal(const UserId& userId)
  : ProtocolSignal(SignalUnignoreUser, userId)
{ /* Empty */ }

ProtoUnignoreUserSignal::~ProtoUnignoreUserSignal()
{ /* Empty */ }


ProtoSendFileSignal::ProtoSendFileSignal(unsigned long eventId, const UserId& userId,
    const string& filename, const string& message, const list<string>& files, unsigned flags)
  : ProtocolSignal(SignalSendFile, userId, eventId),
    myFilename(filename),
    myMessage(message),
    myFiles(files),
    myFlags(flags)
{ /* Empty */ }

ProtoSendFileSignal::~ProtoSendFileSignal()
{ /* Empty */ }


ProtoSendChatSignal::ProtoSendChatSignal(const UserId& userId, const string& message)
  : ProtocolSignal(SignalSendChat, userId),
    myMessage(message)
{ /* Empty */ }

ProtoSendChatSignal::~ProtoSendChatSignal()
{ /* Empty */ }


ProtoCancelEventSignal::ProtoCancelEventSignal(const UserId& userId, unsigned long eventId)
  : ProtocolSignal(SignalCancelEvent, userId, eventId)
{ /* Empty */ }

ProtoCancelEventSignal::~ProtoCancelEventSignal()
{ /* Empty */ }


ProtoSendEventReplySignal::ProtoSendEventReplySignal(const UserId& userId, const string& message,
    bool accepted, unsigned short port, unsigned long sequence, unsigned long flag1,
    unsigned long flag2, bool direct, const string& filename, unsigned long filesize)
  : ProtocolSignal(SignalSendReply, userId),
    myMessage(message),
    myAccept(accepted),
    myPort(port),
    mySequence(sequence),
    myFlag1(flag1),
    myFlag2(flag2),
    myDirect(direct),
    myFilename(filename),
    myFilesize(filesize)
{ /* Empty */ }

ProtoSendEventReplySignal::~ProtoSendEventReplySignal()
{ /* Empty */ }


ProtoOpenSecureSignal::ProtoOpenSecureSignal(unsigned long eventId, const UserId& userId)
  : ProtocolSignal(SignalOpenSecure, userId, eventId)
{ /* Empty */ }

ProtoOpenSecureSignal::~ProtoOpenSecureSignal()
{ /* Empty */ }


ProtoCloseSecureSignal::ProtoCloseSecureSignal(unsigned long eventId, const UserId& userId)
  : ProtocolSignal(SignalCloseSecure, userId, eventId)
{ /* Empty */ }

ProtoCloseSecureSignal::~ProtoCloseSecureSignal()
{ /* Empty */ }


ProtoRequestAuthSignal::ProtoRequestAuthSignal(const UserId& userId, const string& message)
  : ProtocolSignal(SignalRequestAuth, userId),
    myMessage(message)
{ /* Empty */ }

ProtoRequestAuthSignal::~ProtoRequestAuthSignal()
{ /* Empty */ }


ProtoRenameGroupSignal::ProtoRenameGroupSignal(const UserId& ownerId, int groupId)
  : ProtocolSignal(SignalRenameGroup, ownerId),
    myGroupId(groupId)
{ /* Empty */ }

ProtoRenameGroupSignal::~ProtoRenameGroupSignal()
{ /* Empty */ }


ProtoRemoveGroupSignal::ProtoRemoveGroupSignal(const UserId& ownerId, int groupId,
    unsigned long groupServerId, const string& groupName)
  : ProtocolSignal(SignalRemoveGroup, ownerId),
    myGroupId(groupId),
    myGroupServerId(groupServerId),
    myGroupName(groupName)
{ /* Empty */ }

ProtoRemoveGroupSignal::~ProtoRemoveGroupSignal()
{ /* Empty */ }


ProtoSendUrlSignal::ProtoSendUrlSignal(unsigned long eventId, const UserId& userId,
    const string& url, const string& message, unsigned flags, const Color* color)
  : ProtocolSignal(SignalSendUrl, userId, eventId),
    myUrl(url),
    myMessage(message),
    myFlags(flags),
    myColor(color)
{ /* Empty */ }

ProtoSendUrlSignal::~ProtoSendUrlSignal()
{ /* Empty */ }
