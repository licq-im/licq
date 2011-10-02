/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998-2011 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

#include "config.h"

#include "icq.h"

#include <ctime>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <fcntl.h>
#include <ctype.h>
#include <langinfo.h>

#include <licq/byteorder.h>
#include <licq/contactlist/owner.h>
#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>
#include <licq/event.h>
#include <licq/gpghelper.h>
#include <licq/icq/chat.h>
#include <licq/icq/filetransfer.h>
#include <licq/oneventmanager.h>
#include <licq/plugin/pluginmanager.h>
#include <licq/pluginsignal.h>
#include <licq/protocolsignal.h>
#include <licq/socket.h>
#include <licq/statistics.h>
#include <licq/translator.h>
#include <licq/userevents.h>
#include <licq/logging/log.h>
#include <licq/version.h>

#include "../daemon.h"
#include "../gettext.h"
#include "oscarservice.h"
#include "packet.h"

using namespace std;
using Licq::Log;
using Licq::OnEventData;
using Licq::StringList;
using Licq::gLog;
using Licq::gOnEventManager;
using Licq::gTranslator;
using LicqDaemon::Daemon;
using LicqDaemon::gDaemon;


void IcqProtocol::icqSendMessage(unsigned long eventId, const Licq::UserId& userId, const string& message,
    unsigned flags, const Licq::Color* pColor)
{
  const string accountId = userId.accountId();
  string m = gTranslator.clientToServer(message, true);

  bool bUserOffline = true;
  Licq::EventMsg* e = NULL;

  unsigned long f = Licq::EventMsg::FlagLicqVerMask | Licq::EventMsg::FlagSender;
  bool useGpg = false;
  {
    Licq::UserReadGuard u(userId);
    if (u.isLocked())
    {
      bUserOffline = !u->isOnline();
      if (!bUserOffline)
        useGpg = u->UseGPG();
    }
  }
  if (useGpg)
  {
    char* cipher = Licq::gGpgHelper.Encrypt(m.c_str(), userId);
    if (cipher != NULL)
    {
      m = cipher;
      free(cipher);
      f |= Licq::UserEvent::FlagEncrypted;
    }
    else
      useGpg = false;
  }

  unsigned short nLevel = ICQ_TCPxMSG_NORMAL;
  if (flags & Licq::ProtocolSignal::SendUrgent)
  {
    f |= Licq::UserEvent::FlagUrgent;
    nLevel = ICQ_TCPxMSG_URGENT;
  }
  else if (flags & Licq::ProtocolSignal::SendToList)
    nLevel = ICQ_TCPxMSG_LIST;
  if (flags & Licq::ProtocolSignal::SendToMultiple)
    f |= Licq::UserEvent::FlagMultiRec;

  string fromEncoding;
  {
    Licq::UserReadGuard u(userId);
    if (u.isLocked() && !u->userEncoding().empty())
      fromEncoding = u->userEncoding();
  }
  if (fromEncoding.empty())
    fromEncoding = nl_langinfo(CODESET);

  if ((flags & Licq::ProtocolSignal::SendDirect) == 0)
  {
    unsigned short nCharset = CHARSET_ASCII;

    if (!useGpg && !gTranslator.isAscii(m))
    {
      nCharset = CHARSET_UNICODE;
      m = gTranslator.toUtf16(m, fromEncoding);
    }

    e = new Licq::EventMsg(message, Licq::EventMsg::TimeNow, f);
     unsigned short nMaxSize = bUserOffline ? MaxOfflineMessageSize : MaxMessageSize;
    if (m.size() > nMaxSize)
    {
      gLog.warning(tr("Truncating message to %d characters to send through server."), nMaxSize);
      m.resize(nMaxSize);
    }
     icqSendThroughServer(eventId, userId,
        ICQ_CMDxSUB_MSG | ((flags & Licq::ProtocolSignal::SendToMultiple) ? ICQ_CMDxSUB_FxMULTIREC : 0),
        m, e, nCharset);
  }

  Licq::UserWriteGuard u(userId);

  if (flags & Licq::ProtocolSignal::SendDirect)
  {
    f |= Licq::UserEvent::FlagDirect;

    if (!u.isLocked())
      return;
    if (u->Secure())
      f |= Licq::UserEvent::FlagEncrypted;
    e = new Licq::EventMsg(message, Licq::EventMsg::TimeNow, f);
    if (pColor != NULL) e->SetColor(pColor);
    bool isUtf8 = (!gTranslator.isAscii(m) && fromEncoding == "UTF-8");
    CPT_Message* p = new CPT_Message(m, nLevel, flags & Licq::ProtocolSignal::SendToMultiple, pColor, *u, isUtf8);
    gLog.info(tr("Sending %smessage to %s (#%hu)."),
        (flags & Licq::ProtocolSignal::SendUrgent) ? tr("urgent ") : "",
        u->getAlias().c_str(), -p->Sequence());
    SendExpectEvent_Client(eventId, *u, p, e);
  }

  if (u.isLocked())
  {
    u->SetSendServer((flags & Licq::ProtocolSignal::SendDirect) == 0);
    u->SetSendLevel(nLevel);
  }

  if (pColor != NULL)
    Licq::Color::setDefaultColors(pColor);
}

unsigned long IcqProtocol::icqFetchAutoResponse(const Licq::UserId& userId, bool bServer)
{
  unsigned long eventId = gDaemon.getNextEventId();
  if (Licq::gUserManager.isOwner(userId))
    return 0;

  if (isalpha(userId.accountId()[0]))
  {
    icqFetchAutoResponseServer(eventId, userId);
    return eventId;
  }

  Licq::UserWriteGuard u(userId);

  if (bServer)
  {
    // Generic read, gets changed in constructor
    CSrvPacketTcp *s = new CPU_AdvancedMessage(*u, ICQ_CMDxTCP_READxAWAYxMSG, 0, false, 0, 0, 0);
    gLog.info(tr("Requesting auto response from %s."), u->getAlias().c_str());
    SendExpectEvent_Server(eventId, userId, s, NULL);
  }
  else
  {
    CPT_ReadAwayMessage *p = new CPT_ReadAwayMessage(*u);
    gLog.info(tr("Requesting auto response from %s (#%hu)."),
        u->getAlias().c_str(), -p->Sequence());
    SendExpectEvent_Client(eventId, *u, p, NULL);
  }

  return eventId;
}

void IcqProtocol::icqSendUrl(unsigned long eventId, const Licq::UserId& userId, const string& url,
    const string& message, unsigned flags, const Licq::Color* pColor)
{
  if (Licq::gUserManager.isOwner(userId))
    return;

  const string accountId = userId.accountId();
  const char* description = message.c_str();

  // make the URL info string
  char *szDescDos = NULL;
  Licq::EventUrl* e = NULL;
  string m = gTranslator.clientToServer(message, true);
  int n = url.size() + m.size() + 2;
  if ((flags & Licq::ProtocolSignal::SendDirect) == 0 && n > MaxMessageSize)
    m.erase(MaxMessageSize - url.size() - 2);
  m += '\xFE';
  m += url;

  unsigned long f = Licq::EventUrl::FlagLicqVerMask | Licq::EventUrl::FlagSender;
  unsigned short nLevel = ICQ_TCPxMSG_NORMAL;
  if (flags & Licq::ProtocolSignal::SendDirect)
    f |= Licq::UserEvent::FlagDirect;
  if (flags & Licq::ProtocolSignal::SendUrgent)
  {
    f |= Licq::UserEvent::FlagUrgent;
    nLevel = ICQ_TCPxMSG_URGENT;
  }
  else if (flags & Licq::ProtocolSignal::SendToList)
  {
    nLevel = ICQ_TCPxMSG_LIST;
  }
  if (flags & Licq::ProtocolSignal::SendToMultiple)
    f |= Licq::UserEvent::FlagMultiRec;

  if ((flags & Licq::ProtocolSignal::SendDirect) == 0)
  {
    unsigned short nCharset = 0;
    {
      Licq::UserReadGuard u(userId);
      if (u.isLocked() && !u->userEncoding().empty())
        nCharset = 3;
    }

    e = new Licq::EventUrl(url.c_str(), description,
        Licq::EventUrl::TimeNow, f);
    icqSendThroughServer(eventId, userId,
        ICQ_CMDxSUB_URL | ((flags & Licq::ProtocolSignal::SendToMultiple) ? ICQ_CMDxSUB_FxMULTIREC : 0), m, e, nCharset);
  }

  Licq::UserWriteGuard u(userId);

  if (flags & Licq::ProtocolSignal::SendDirect)
  {
    if (!u.isLocked())
      return;
    if (u->Secure())
      f |= Licq::UserEvent::FlagEncrypted;
    e = new Licq::EventUrl(url.c_str(), description, Licq::EventUrl::TimeNow, f);
    if (pColor != NULL) e->SetColor(pColor);
    CPT_Url* p = new CPT_Url(m, nLevel, flags & Licq::ProtocolSignal::SendToMultiple, pColor, *u);
    gLog.info(tr("Sending %sURL to %s (#%hu)."),
        (flags & Licq::ProtocolSignal::SendUrgent) ? tr("urgent ") : "",
        u->getAlias().c_str(), -p->Sequence());
    SendExpectEvent_Client(eventId, *u, p, e);
  }
  if (u.isLocked())
  {
    u->SetSendServer((flags & Licq::ProtocolSignal::SendDirect) == 0);
    u->SetSendLevel(nLevel);
  }

  if (pColor != NULL)
    Licq::Color::setDefaultColors(pColor);

  if (szDescDos)
    delete [] szDescDos;
}

void IcqProtocol::icqFileTransfer(unsigned long eventId, const Licq::UserId& userId, const string& filename,
    const string& message, const list<string>& lFileList, unsigned flags)
{
  if (Licq::gUserManager.isOwner(userId))
    return;

  string dosDesc = gTranslator.clientToServer(message, true);
  Licq::EventFile* e = NULL;

  Licq::UserWriteGuard u(userId);
  if (!u.isLocked())
    return;

  unsigned short nLevel;

  if ((flags & Licq::ProtocolSignal::SendDirect) == 0)
  {
    unsigned long f = LICQ_VERSION | Licq::EventFile::FlagSender;
    //flags through server are a little different
    if (flags & Licq::ProtocolSignal::SendUrgent)
    {
      f |= Licq::UserEvent::FlagUrgent;
      nLevel = ICQ_TCPxMSG_URGENT2;
    }
    else if (flags & Licq::ProtocolSignal::SendToList)
      nLevel = ICQ_TCPxMSG_LIST2;
    else
      nLevel = ICQ_TCPxMSG_NORMAL2;

    CPU_FileTransfer* p = new CPU_FileTransfer(*u, lFileList, filename,
        dosDesc, nLevel, (u->Version() > 7));

    if (!p->IsValid())
    {
      delete p;
    }
    else
    {
      e = new Licq::EventFile(filename, p->description(), p->GetFileSize(),
          lFileList, p->Sequence(), Licq::EventFile::TimeNow, f);
      gLog.info(tr("Sending file transfer to %s (#%hu)."),
          u->getAlias().c_str(), -p->Sequence());

      SendExpectEvent_Server(userId, p, e);
    }
  }
  else
  {
    unsigned long f = Licq::EventFile::FlagLicqVerMask | Licq::EventFile::FlagDirect | Licq::EventFile::FlagSender;

    if (flags & Licq::ProtocolSignal::SendUrgent)
    {
      f |= Licq::UserEvent::FlagUrgent;
      nLevel = ICQ_TCPxMSG_URGENT;
    }
    else if (flags & Licq::ProtocolSignal::SendToList)
      nLevel = ICQ_TCPxMSG_LIST;
    else
      nLevel = ICQ_TCPxMSG_NORMAL;
    if (u->Secure())
      f |= Licq::UserEvent::FlagEncrypted;

    CPT_FileTransfer* p = new CPT_FileTransfer(lFileList, filename, dosDesc, nLevel, *u);

    if (!p->IsValid())
    {
      delete p;
    }
    else
    {
      e = new Licq::EventFile(filename, p->description(), p->GetFileSize(),
          lFileList, p->Sequence(), Licq::EventFile::TimeNow, f);
      gLog.info(tr("Sending %sfile transfer to %s (#%hu)."),
          (flags & Licq::ProtocolSignal::SendUrgent) ? tr("urgent ") : "",
          u->getAlias().c_str(), -p->Sequence());

      SendExpectEvent_Client(eventId, *u, p, e);
    }
  }

  u->SetSendServer((flags & Licq::ProtocolSignal::SendDirect) == 0);
  u->SetSendLevel(nLevel);
}

//-----CICQDaemon::sendContactList-------------------------------------------
unsigned long IcqProtocol::icqSendContactList(const Licq::UserId& userId,
   const StringList& users, unsigned flags, const Licq::Color* pColor)
{
  unsigned long eventId = gDaemon.getNextEventId();
  if (Licq::gUserManager.isOwner(userId))
    return 0;

  char *m = new char[3 + users.size() * 80];
  int p = sprintf(m, "%d%c", int(users.size()), char(0xFE));
  Licq::EventContactList::ContactList vc;

  StringList::const_iterator iter;
  for (iter = users.begin(); iter != users.end(); ++iter)
  {
    Licq::UserId uId(*iter, LICQ_PPID);
    Licq::UserReadGuard u(uId);
    p += sprintf(&m[p], "%s%c%s%c", iter->c_str(), char(0xFE),
       !u.isLocked() ? "" : u->getAlias().c_str(), char(0xFE));
    vc.push_back(new Licq::EventContactList::Contact(uId, !u.isLocked() ? "" : u->getAlias()));
  }

  if ((flags & Licq::ProtocolSignal::SendDirect) == 0 && p > MaxMessageSize)
  {
    gLog.warning(tr("Contact list too large to send through server."));
    delete []m;
    return 0;
  }

  Licq::EventContactList* e = NULL;

  unsigned long f = Licq::EventContactList::FlagLicqVerMask | Licq::EventContactList::FlagSender;
  unsigned short nLevel = ICQ_TCPxMSG_NORMAL;
  if (flags & Licq::ProtocolSignal::SendDirect)
    f |= Licq::UserEvent::FlagDirect;
  if (flags & Licq::ProtocolSignal::SendUrgent)
  {
    f |= Licq::UserEvent::FlagUrgent;
    nLevel = ICQ_TCPxMSG_URGENT;
  }
  else if (flags & Licq::ProtocolSignal::SendToList)
    nLevel = ICQ_TCPxMSG_LIST;
  if (flags & Licq::ProtocolSignal::SendToMultiple)
    f |= Licq::UserEvent::FlagMultiRec;

  if ((flags & Licq::ProtocolSignal::SendDirect) == 0) // send offline
  {
    e = new Licq::EventContactList(vc, false, Licq::EventContactList::TimeNow, f);
    icqSendThroughServer(eventId, userId,
      ICQ_CMDxSUB_CONTACTxLIST | ((flags & Licq::ProtocolSignal::SendToMultiple) ? ICQ_CMDxSUB_FxMULTIREC : 0),
      m, e);
  }

  Licq::UserWriteGuard u(userId);
  if (flags & Licq::ProtocolSignal::SendDirect)
  {
    if (!u.isLocked())
      return 0;
    if (u->Secure())
      f |= Licq::UserEvent::FlagEncrypted;
    e = new Licq::EventContactList(vc, false, Licq::EventContactList::TimeNow, f);
    if (pColor != NULL) e->SetColor(pColor);
    CPT_ContactList *p = new CPT_ContactList(m, nLevel, flags & Licq::ProtocolSignal::SendToMultiple, pColor, *u);
    gLog.info(tr("Sending %scontact list to %s (#%hu)."),
        (flags & Licq::ProtocolSignal::SendUrgent) ? tr("urgent ") : "",
        u->getAlias().c_str(), -p->Sequence());
    SendExpectEvent_Client(eventId, *u, p, e);
  }
  if (u.isLocked())
  {
    u->SetSendServer((flags & Licq::ProtocolSignal::SendDirect) == 0);
    u->SetSendLevel(nLevel);
  }

  if (pColor != NULL)
    Licq::Color::setDefaultColors(pColor);

  delete []m;
  return eventId;
}

//-----CICQDaemon::sendInfoPluginReq--------------------------------------------
unsigned long IcqProtocol::icqRequestInfoPlugin(Licq::User* u, bool bServer,
                                               const char *GUID)
{
  Licq::Event* result = NULL;
  if (bServer)
  {
    CPU_InfoPluginReq *p = new CPU_InfoPluginReq(u, GUID, 0);
    result = SendExpectEvent_Server(u->id(), p, NULL);
  }
  else
  {
    CPT_InfoPluginReq *p = new CPT_InfoPluginReq(u, GUID, 0);
    result = SendExpectEvent_Client(u, p, NULL);
  }

  if (result != NULL)
    return result->EventId();
  return 0;
}

//-----CICQDaemon::sendInfoPluginListReq----------------------------------------
unsigned long IcqProtocol::icqRequestInfoPluginList(const Licq::UserId& userId, bool bServer)
{
  if (Licq::gUserManager.isOwner(userId))
    return 0;

  Licq::UserWriteGuard u(userId);
  if (!u.isLocked())
    return 0;

  if (bServer)
    gLog.info(tr("Requesting info plugin list from %s through server."), u->getAlias().c_str());
  else
    gLog.info(tr("Requesting info plugin list from %s."), u->getAlias().c_str());

  unsigned long result = icqRequestInfoPlugin(*u, bServer, PLUGIN_QUERYxINFO);

  return result;
}

//-----CICQDaemon::sendPhoneBookReq--------------------------------------------
unsigned long IcqProtocol::icqRequestPhoneBook(const Licq::UserId& userId, bool bServer)
{
  if (Licq::gUserManager.isOwner(userId))
    return 0;

  Licq::UserWriteGuard u(userId);
  if (!u.isLocked())
    return 0;

  if (bServer)
    gLog.info(tr("Requesting Phone Book from %s through server."), u->getAlias().c_str());
  else
    gLog.info(tr("Requesting Phone Book from %s."), u->getAlias().c_str());

  unsigned long result = icqRequestInfoPlugin(*u, bServer, PLUGIN_PHONExBOOK);

  return result;
}

//-----CICQDaemon::sendPictureReq-----------------------------------------------
unsigned long IcqProtocol::icqRequestPicture(const Licq::UserId& userId, bool bServer, size_t iconHashSize)
{
  if (UseServerSideBuddyIcons() && iconHashSize > 0)
    return m_xBARTService->SendEvent(userId, ICQ_SNACxBART_DOWNLOADxREQUEST, true);

  if (Licq::gUserManager.isOwner(userId))
     return 0;

  Licq::UserWriteGuard u(userId);
  if (!u.isLocked())
    return 0;

  if (bServer)
    gLog.info(tr("Requesting Picture from %s through server."), u->getAlias().c_str());
  else
    gLog.info(tr("Requesting Picture from %s."), u->getAlias().c_str());

  return icqRequestInfoPlugin(*u, bServer, PLUGIN_PICTURE);
}

//-----CICQDaemon::sendStatusPluginReq------------------------------------------
unsigned long IcqProtocol::icqRequestStatusPlugin(Licq::User* u, bool bServer,
                                                 const char *GUID)
{
  Licq::Event* result = NULL;
  if (bServer)
  {
    CPU_StatusPluginReq *p = new CPU_StatusPluginReq(u, GUID, 0);
    result = SendExpectEvent_Server(u->id(), p, NULL);
  }
  else
  {
    CPT_StatusPluginReq *p = new CPT_StatusPluginReq(u, GUID, 0);
    result = SendExpectEvent_Client(u, p, NULL);
  }

  if (result != NULL)
    return result->EventId();
  return 0;
}

//-----CICQDaemon::sendStatusPluginListReq--------------------------------------
unsigned long IcqProtocol::icqRequestStatusPluginList(const Licq::UserId& userId, bool bServer)
{
  if (Licq::gUserManager.isOwner(userId))
    return 0;

  Licq::UserWriteGuard u(userId);
  if (!u.isLocked())
    return 0;

  if (bServer)
    gLog.info(tr("Requesting status plugin list from %s through server."), u->getAlias().c_str());
  else
    gLog.info(tr("Requesting status plugin list from %s."), u->getAlias().c_str());

  unsigned long result = icqRequestStatusPlugin(*u, bServer, PLUGIN_QUERYxSTATUS);

  return result;
}

//-----CICQDaemon::sendSharedFilesReq--------------------------------------
unsigned long IcqProtocol::icqRequestSharedFiles(const Licq::UserId& userId, bool bServer)
{
  if (Licq::gUserManager.isOwner(userId))
    return 0;

  Licq::UserWriteGuard u(userId);
  if (!u.isLocked())
    return 0;

  if (bServer)
    gLog.info(tr("Requesting file server status from %s through server."), u->getAlias().c_str());
  else
    gLog.info(tr("Requesting file server status from %s."), u->getAlias().c_str());

  unsigned long result = icqRequestStatusPlugin(*u, bServer, PLUGIN_FILExSERVER);

  return result;
}

//-----CICQDaemon::sendPhoneFollowMeReq--------------------------------------
unsigned long IcqProtocol::icqRequestPhoneFollowMe(const Licq::UserId& userId, bool bServer)
{
  if (Licq::gUserManager.isOwner(userId))
    return 0;

  Licq::UserWriteGuard u(userId);
  if (!u.isLocked())
    return 0;

  if (bServer)
    gLog.info(tr("Requesting Phone \"Follow Me\" status from %s through server."), u->getAlias().c_str());
  else
    gLog.info(tr("Requesting Phone \"Follow Me\" status from %s."), u->getAlias().c_str());

  unsigned long result = icqRequestStatusPlugin(*u, bServer, PLUGIN_FOLLOWxME);

  return result;
}

//-----CICQDaemon::sendICQphoneReq--------------------------------------
unsigned long IcqProtocol::icqRequestICQphone(const Licq::UserId& userId, bool bServer)
{
  if (Licq::gUserManager.isOwner(userId))
    return 0;

  Licq::UserWriteGuard u(userId);
  if (!u.isLocked())
    return 0;

  if (bServer)
    gLog.info(tr("Requesting ICQphone status from %s through server."), u->getAlias().c_str());
  else
    gLog.info(tr("Requesting ICQphone status from %s."), u->getAlias().c_str());

  unsigned long result = icqRequestStatusPlugin(*u, bServer, PLUGIN_FILExSERVER);

  return result;
}

void IcqProtocol::icqFileTransferCancel(const Licq::UserId& userId, unsigned short nSequence)
{
  // add to history ??
  Licq::UserWriteGuard u(userId);
  if (!u.isLocked())
    return;
  gLog.info(tr("Cancelling file transfer to %s (#%hu)."), u->getAlias().c_str(), -nSequence);
  CPT_CancelFile p(nSequence, *u);
  AckTCP(p, u->normalSocketDesc());
}

void IcqProtocol::icqFileTransferAccept(const Licq::UserId& userId, unsigned short nPort,
    unsigned short nSequence, const unsigned long nMsgID[2], bool viaServer,
    const string& message, const string& filename, unsigned long nFileSize)
{
   // basically a fancy tcp ack packet which is sent late
  Licq::UserWriteGuard u(userId);
  if (!u.isLocked())
    return;
  gLog.info(tr("Accepting file transfer from %s (#%hu)."), u->getAlias().c_str(), -nSequence);
  if (!viaServer)
  {
    CPT_AckFileAccept p(nPort, nSequence, *u);
    AckTCP(p, u->normalSocketDesc());
  }
  else
  {
    CPU_AckFileAccept *p = new CPU_AckFileAccept(*u, nMsgID,
        nSequence, nPort, message, filename, nFileSize);
    SendEvent_Server(p);
  }
}

void IcqProtocol::icqFileTransferRefuse(const Licq::UserId& userId, const string& message,
    unsigned short nSequence, const unsigned long nMsgID[2], bool viaServer)
{
   // add to history ??
  string reasonDos = gTranslator.clientToServer(message, true);
  Licq::UserWriteGuard u(userId);
  if (!u.isLocked())
    return;
  gLog.info(tr("Refusing file transfer from %s (#%hu)."), u->getAlias().c_str(), -nSequence);

  if (!viaServer)
  {
    CPT_AckFileRefuse p(reasonDos, nSequence, *u);
    AckTCP(p, u->normalSocketDesc());
  }
  else
  {
    CPU_AckFileRefuse *p = new CPU_AckFileRefuse(*u, nMsgID, nSequence,
        reasonDos);
    SendEvent_Server(p);
  }
}

unsigned long IcqProtocol::icqChatRequest(const Licq::UserId& userId, const string& reason,
    unsigned flags)
{
  return icqMultiPartyChatRequest(userId, reason, "", 0, flags);
}

unsigned long IcqProtocol::icqMultiPartyChatRequest(const Licq::UserId& userId,
   const string& reason, const string& chatUsers, unsigned short nPort,
   unsigned flags)
{
  if (Licq::gUserManager.isOwner(userId))
    return 0;

  Licq::UserWriteGuard u(userId);
  if (!u.isLocked())
    return 0;
  string reasonDos = gTranslator.clientToServer(reason, true);

  unsigned long f;
  unsigned short nLevel;
  Licq::Event* result = NULL;
  if ((flags & Licq::ProtocolSignal::SendDirect) == 0)
  {
    f = Licq::EventChat::FlagLicqVerMask | Licq::EventChat::FlagSender;

    //flags through server are a little different
    if (flags & Licq::ProtocolSignal::SendUrgent)
    {
      f |= Licq::UserEvent::FlagUrgent;
      nLevel = ICQ_TCPxMSG_URGENT2;
    }
    else if (flags & Licq::ProtocolSignal::SendToList)
      nLevel = ICQ_TCPxMSG_LIST2;
    else
      nLevel = ICQ_TCPxMSG_NORMAL2;

    CPU_ChatRequest *p = new CPU_ChatRequest(reasonDos,
        chatUsers, nPort, nLevel, *u, (u->Version() > 7));

    Licq::EventChat* e = new Licq::EventChat(reason, chatUsers.c_str(), nPort, p->Sequence(),
        Licq::EventChat::TimeNow, f);
    gLog.info(tr("Sending chat request to %s (#%hu)."), u->getAlias().c_str(), -p->Sequence());

      result = SendExpectEvent_Server(u->id(), p, e);
    }
  else
  {
    f = Licq::EventChat::FlagLicqVerMask | Licq::EventChat::FlagDirect | Licq::EventChat::FlagSender;
    nLevel = ICQ_TCPxMSG_NORMAL;
    if (flags & Licq::ProtocolSignal::SendUrgent)
    {
      f |= Licq::UserEvent::FlagUrgent;
      nLevel = ICQ_TCPxMSG_URGENT;
    }
    else if (flags & Licq::ProtocolSignal::SendToList)
      nLevel = ICQ_TCPxMSG_LIST;
    if (u->Secure())
      f |= Licq::UserEvent::FlagEncrypted;

    CPT_ChatRequest* p = new CPT_ChatRequest(reasonDos, chatUsers, nPort,
        nLevel, *u, (u->Version() > 7));
    Licq::EventChat* e = new Licq::EventChat(reason, chatUsers.c_str(), nPort, p->Sequence(),
        Licq::UserEvent::TimeNow, f);
    gLog.info(tr("Sending %schat request to %s (#%hu)."),
        (flags & Licq::ProtocolSignal::SendUrgent) ? tr("urgent ") : "",
        u->getAlias().c_str(), -p->Sequence());
    result = SendExpectEvent_Client(*u, p, e);
	}
	
  u->SetSendServer((flags & Licq::ProtocolSignal::SendDirect) == 0);
  u->SetSendLevel(nLevel);

  if (result != NULL)
    return result->EventId();
  return 0;
}

void IcqProtocol::icqChatRequestCancel(const Licq::UserId& userId, unsigned short nSequence)
{
  Licq::UserWriteGuard u(userId);
  if (!u.isLocked())
    return;
  gLog.info(tr("Cancelling chat request with %s (#%hu)."), u->getAlias().c_str(), -nSequence);
  CPT_CancelChat p(nSequence, *u);
  AckTCP(p, u->normalSocketDesc());
}

void IcqProtocol::icqChatRequestRefuse(const Licq::UserId& userId, const string& reason,
    unsigned short nSequence, const unsigned long nMsgID[2], bool bDirect)
{
  // add to history ??
  Licq::UserWriteGuard u(userId);
  if (!u.isLocked())
    return;
  gLog.info(tr("Refusing chat request with %s (#%hu)."), u->getAlias().c_str(), -nSequence);
  string reasonDos = gTranslator.clientToServer(reason, true);

	if (bDirect)
  {
    CPT_AckChatRefuse p(reasonDos, nSequence, *u);
    AckTCP(p, u->normalSocketDesc());
  }
  else
  {
    CPU_AckChatRefuse* p = new CPU_AckChatRefuse(*u, nMsgID, nSequence, reasonDos);
		SendEvent_Server(p);
	}
}

void IcqProtocol::icqChatRequestAccept(const Licq::UserId& userId, unsigned short nPort,
    const string& clients, unsigned short nSequence,
    const unsigned long nMsgID[2], bool bDirect)
{
  // basically a fancy tcp ack packet which is sent late
  // add to history ??
  Licq::UserWriteGuard u(userId);
  if (!u.isLocked())
    return;
  gLog.info(tr("Accepting chat request with %s (#%hu)."), u->getAlias().c_str(), -nSequence);

	if (bDirect)
  {
    CPT_AckChatAccept p(nPort, clients, nSequence, *u, u->Version() > 7);
    AckTCP(p, u->normalSocketDesc());
  }
  else
  {
    CPU_AckChatAccept* p = new CPU_AckChatAccept(*u, clients, nMsgID, nSequence, nPort);
		SendEvent_Server(p);
	}
}

/*---------------------------------------------------------------------------
 * OpenSSL stuff
 *-------------------------------------------------------------------------*/

void IcqProtocol::icqOpenSecureChannel(unsigned long eventId, const Licq::UserId& userId)
{
#ifdef USE_OPENSSL
  Licq::UserWriteGuard u(userId);
  if (!u.isLocked())
    return;

  CPT_OpenSecureChannel *pkt = new CPT_OpenSecureChannel(*u);
  gLog.info(tr("Sending request for secure channel to %s (#%hu)."),
      u->getAlias().c_str(), -pkt->Sequence());
  SendExpectEvent_Client(eventId, *u, pkt, NULL);

  u->SetSendServer(false);

#else // No OpenSSL
  gLog.warning("icqOpenSecureChannel() to %s called when we do not support OpenSSL.",
      userId.toString().c_str());
#endif
}

void IcqProtocol::icqCloseSecureChannel(unsigned long eventId, const Licq::UserId& userId)
{
#ifdef USE_OPENSSL
  Licq::UserWriteGuard u(userId);
  if (!u.isLocked())
    return;

  CPT_CloseSecureChannel *pkt = new CPT_CloseSecureChannel(*u);
  gLog.info(tr("Closing secure channel with %s (#%hu)."),
      u->getAlias().c_str(), -pkt->Sequence());
  SendExpectEvent_Client(eventId, *u, pkt, NULL);

  u->SetSendServer(false);

#else // No OpenSSL
  gLog.warning("icqCloseSecureChannel() to %s called when we do not support OpenSSL.",
      userId.toString().c_str());
#endif
}

void IcqProtocol::icqOpenSecureChannelCancel(const Licq::UserId& userId,
  unsigned short nSequence)
{
  Licq::UserWriteGuard u(userId);
  if (!u.isLocked())
    return;
  gLog.info(tr("Cancelling secure channel request to %s (#%hu)."),
      u->getAlias().c_str(), -nSequence);
  // XXX Tear down tcp connection ??
}

/*---------------------------------------------------------------------------
 * Handshake
 *
 * Shake hands on the given socket with the given user.
 *-------------------------------------------------------------------------*/
bool IcqProtocol::handshake_Send(Licq::TCPSocket* s, const Licq::UserId& userId,
   unsigned short nPort, unsigned short nVersion, bool bConfirm,
   unsigned long nId)
{
  s->SetVersion(nVersion);
  s->setUserId(userId);

  unsigned long nUin = strtoul(userId.accountId().c_str(), NULL, 10);

  switch (nVersion)
  {
    case 2:
    case 3:
    {
      CPacketTcp_Handshake_v2 p(s->getLocalPort());
      if (!s->SendPacket(p.getBuffer())) goto sock_error;
      break;
    }
    case 4:
    case 5:
    {
      CPacketTcp_Handshake_v4 p(s->getLocalPort());
      if (!s->SendPacket(p.getBuffer())) goto sock_error;
      break;
    }
    case 6:
    {
      // Send the hanshake
      CPacketTcp_Handshake_v6 p(nUin, 0, nPort);
      if (!s->SendPacket(p.getBuffer())) goto sock_error;

      // Wait for the handshake ack
      do
      {
        if (!s->RecvPacket()) goto sock_error;
      } while (!s->RecvBufferFull());
      unsigned long nOk = s->RecvBuffer().UnpackUnsignedLong();
      s->ClearRecvBuffer();
      if (nOk != 1)
      {
        gLog.warning(tr("Bad handshake ack: %ld."), nOk);
        return false;
      }

      // Wait for the reverse handshake
      do
      {
        if (!s->RecvPacket()) goto sock_error;
      } while (!s->RecvBufferFull());
      CPacketTcp_Handshake_v6 p_in(&s->RecvBuffer());
      s->ClearRecvBuffer();
      if (p.SessionId() != p_in.SessionId())
      {
        gLog.warning(tr("Bad handshake session id: received %ld, expecting %ld."),
            p_in.SessionId(), p.SessionId());
        return false;
      }

      // Send the hanshake ack
      CPacketTcp_Handshake_Ack p_ack;
      if (!s->SendPacket(p_ack.getBuffer())) goto sock_error;

      break;
    }

    case 7:
    case 8:
    {
      // Send the hanshake
      CPacketTcp_Handshake_v7 p(nUin, 0, nPort, nId);
      if (!s->SendPacket(p.getBuffer())) goto sock_error;

      // Wait for the handshake ack
      do
      {
        if (!s->RecvPacket()) goto sock_error;
      } while (!s->RecvBufferFull());
      unsigned long nOk = s->RecvBuffer().UnpackUnsignedLong();
      s->ClearRecvBuffer();
      if (nOk != 1)
      {
        gLog.warning(tr("Bad handshake ack: %ld."), nOk);
        return false;
      }

      // Wait for the reverse handshake
      do
      {
        if (!s->RecvPacket()) goto sock_error;
      } while (!s->RecvBufferFull());
      CPacketTcp_Handshake_v7 p_in(&s->RecvBuffer());
      s->ClearRecvBuffer();
      if (p_in.SessionId() && p.SessionId() != p_in.SessionId())
      {
        gLog.warning(tr("Bad handshake cookie: received %ld, expecting %ld."),
            p_in.SessionId(), p.SessionId());
        return false;
      }

      // Send the hanshake ack
      CPacketTcp_Handshake_Ack p_ack;
      if (!s->SendPacket(p_ack.getBuffer())) goto sock_error;

			// Files and chats don't get this.
			// They do in icq2002a but for some reason icq2002a does not
			// reply to this when licq sends it.  It will reply from a normal
			// handshake though, just not file or chat...
			if (bConfirm)
			{
                          if (nId == 0)
                          {
                            if (!Handshake_SendConfirm_v7(s))
                              return false;
                          }
                          else
                          {
                            if (!Handshake_RecvConfirm_v7(s))
                              goto sock_error;
                          }
                        }

      break;
    }

    default:
      // Should never happen
      gLog.error(tr("Unknown ICQ TCP version (%d)."), nVersion);
      return false;
  }

  return true;
  
sock_error:
  if (s->Error() == 0)
    gLog.warning(tr("Handshake error, remote side closed connection."));
  else
    gLog.warning(tr("Handshake socket error: %s."), s->errorStr().c_str());
  return false;
}


/*------------------------------------------------------------------------------
 * ConnectToUser
 *
 * Creates a new TCPSocket and connects it to a given user.  Adds the socket
 * to the global socket manager and to the user.
 *----------------------------------------------------------------------------*/
int IcqProtocol::connectToUser(const Licq::UserId& userId, int channel)
{
  {
    Licq::UserReadGuard u(userId);
    if (!u.isLocked())
      return -1;

    // Check that we need to connect at all
    int sd = u->socketDesc(channel);
    if (sd != -1)
    {
      gLog.warning(tr("Connection attempted to already connected user (%s)."),
          userId.toString().c_str());
      return sd;
    }
  }

  // Poll if there is a connection in progress already
  while (1)
  {
    {
      Licq::UserReadGuard u(userId);
      if (u.isLocked() && !u->ConnectionInProgress())
        break;
    }
    struct timeval tv = { 2, 0 };
    if (select(0, NULL, NULL, NULL, &tv) == -1 && errno == EINTR) return -1;
  }

  string alias;
  unsigned nPort;
  unsigned nVersion;

  {
    Licq::UserWriteGuard u(userId);
    int sd = u->normalSocketDesc();
    if (sd == -1)
      u->SetConnectionInProgress(true);
    else
      return sd;

    alias = u->getAlias();
    nPort = u->Port();
    nVersion = u->ConnectionVersion();
  }

  Licq::TCPSocket* s = new Licq::TCPSocket(userId);
  if (!openConnectionToUser(userId, s, nPort))
  {
    Licq::UserWriteGuard u(userId);
    if (u.isLocked())
      u->SetConnectionInProgress(false);
    delete s;
    return -1;
  }
  s->setChannel(channel);

  gLog.info(tr("Shaking hands with %s (%s) [v%d]."),
      alias.c_str(), userId.toString().c_str(), nVersion);
  nPort = s->getLocalPort();

  if (!handshake_Send(s, userId, 0, nVersion))
  {
    Licq::UserWriteGuard u(userId);
    if (u.isLocked())
      u->SetConnectionInProgress(false);
    delete s;
    return -1;
  }
  s->SetVersion(nVersion);
  int nSD = s->Descriptor();

  // Set the socket descriptor in the user
  {
    Licq::UserWriteGuard u(userId);
    if (!u.isLocked())
      return -1;
    u->setSocketDesc(s);
    u->SetConnectionInProgress(false);
  }

  // Add the new socket to the socket manager
  gSocketManager.AddSocket(s);
  gSocketManager.DropSocket(s);

  // Alert the select thread that there is a new socket
  myNewSocketPipe.putChar('S');

  return nSD;
}



/*------------------------------------------------------------------------------
 * OpenConnectionToUser
 *
 * Connects a socket to a given user on a given port.
 *----------------------------------------------------------------------------*/
bool IcqProtocol::openConnectionToUser(const Licq::UserId& userId,
    Licq::TCPSocket* sock, unsigned short nPort)
{
  string name;
  unsigned long ip;
  unsigned long intip;
  bool bSendIntIp;
  {
    Licq::UserReadGuard u(userId);
    if (!u.isLocked())
      return false;

    name = u->getAlias() + " (" + u->accountId() + ")";
    ip = u->Ip();
    intip = u->IntIp();
    bSendIntIp = u->SendIntIp();
  }

  return OpenConnectionToUser(name, ip, intip, sock, nPort, bSendIntIp);
}


bool IcqProtocol::OpenConnectionToUser(const string& name, unsigned long nIp,
   unsigned long nIntIp, Licq::TCPSocket* sock, unsigned short nPort, bool bSendIntIp)
{
  char buf[128];

  // Sending to internet ip
  if (!bSendIntIp)
  {
    gLog.info(tr("Connecting to %s at %s:%d."), name.c_str(),
        Licq::ip_ntoa(nIp, buf), nPort);
    // If we fail to set the remote address, the ip must be 0
    if (!sock->connectTo(nIp, nPort))
    {
      gLog.warning(tr("Connect to %s failed: %s."), name.c_str(),
          sock->errorStr().c_str());

      // Now try the internal ip if it is different from this one and we are behind a firewall
      if (sock->Error() != EINTR && nIntIp != nIp &&
          nIntIp != 0 && CPacket::Firewall())
      {
        gLog.info(tr("Connecting to %s at %s:%d."), name.c_str(),
            Licq::ip_ntoa(nIntIp, buf), nPort);

        if (!sock->connectTo(nIntIp, nPort))
        {
          gLog.warning(tr("Connect to %s real ip failed: %s."), name.c_str(),
              sock->errorStr().c_str());
          return false;
        }
      }
      else
      {
        return false;
      }
    }
  }

  // Sending to Internal IP
  else
  {
    gLog.info(tr("Connecting to %s at %s:%d."), name.c_str(),
       Licq::ip_ntoa(nIntIp, buf), nPort);
    if (!sock->connectTo(nIntIp, nPort))
    {
      gLog.warning(tr("Connect to %s real ip failed: %s."), name.c_str(),
          sock->errorStr().c_str());
      return false;
    }
  }


  return true;
}



/*------------------------------------------------------------------------------
 * ReverseConnectToUser
 *
 * Creates a new TCPSocket and connects it to a given user.  Adds the socket
 * to the global socket manager and to the user.
 *----------------------------------------------------------------------------*/
int IcqProtocol::reverseConnectToUser(const Licq::UserId& userId, unsigned long nIp,
   unsigned short nPort, unsigned short nVersion, unsigned short nFailedPort,
   unsigned long nId, unsigned long nMsgID1, unsigned long nMsgID2)
{
  // Find which socket this is for
  Licq::TCPSocket* tcp = dynamic_cast<Licq::TCPSocket*>(gSocketManager.FetchSocket(m_nTCPSocketDesc));
  unsigned short tcpPort = tcp != NULL ? tcp->getLocalPort() : 0;
  gSocketManager.DropSocket(tcp);

  CFileTransferManager *ftm = CFileTransferManager::FindByPort(nFailedPort);
  CChatManager *cm = CChatManager::FindByPort(nFailedPort);

  if (nFailedPort != tcpPort && nFailedPort != 0 && cm == NULL && ftm == NULL)
  {
    gLog.warning(tr("Reverse connection to unknown port (%d)."), nFailedPort);
    return -1;
  }

  Licq::TCPSocket* s = new Licq::TCPSocket(userId);
  char buf[32];

  gLog.info(tr("Reverse connecting to %s at %s:%d."), userId.toString().c_str(),
      Licq::ip_ntoa(nIp, buf), nPort);

  // If we fail to set the remote address, the ip must be 0
  if (!s->connectTo(nIp, nPort))
  {
    gLog.warning(tr("Reverse connect to %s failed: %s."),
        userId.toString().c_str(), s->errorStr().c_str());

    CPU_ReverseConnectFailed* p = new CPU_ReverseConnectFailed(userId.accountId(), nMsgID1,
        nMsgID2, nPort, nFailedPort, nId);
    SendEvent_Server(p);
    return -1;
  }

  gLog.info(tr("Reverse shaking hands with %s."), userId.toString().c_str());
  bool bConfirm = ftm == NULL && cm == NULL;

  // Make sure we use the right version
  nVersion = VersionToUse(nVersion);

  if (!handshake_Send(s, userId, 0, nVersion, bConfirm, nId))
  {
    delete s;
    return -1;
  }
  s->SetVersion(nVersion);
  int nSD = s->Descriptor();

  // File transfer port
  if (ftm != NULL)
  {
     ftm->AcceptReverseConnection(s);
     delete s;
  }

  // Chat port
  else if (cm != NULL)
  {
     cm->AcceptReverseConnection(s);
     delete s;
  }

  // It's the main port
  else
  {
    // Set the socket descriptor in the user if this user is on our list
    {
      Licq::UserWriteGuard u(userId);
      if (u.isLocked())
        u->setSocketDesc(s);
    }

    // Add the new socket to the socket manager, alert the thread
    gSocketManager.AddSocket(s);
    gSocketManager.DropSocket(s);
    myNewSocketPipe.putChar('S');
  }

  return nSD;
}

bool IcqProtocol::ProcessTcpPacket(Licq::TCPSocket* pSock)
{
  unsigned long senderIp, localIp,
                senderPort, junkLong, nPort, nPortReversed;
  unsigned short version, command, junkShort, newCommand, messageLen,
                 headerLen, ackFlags, msgFlags, licqVersion, theSequence;
  char licqChar = '\0', junkChar;
  bool errorOccured = false;
  char *message = 0;
  Licq::UserId userId;

  // only used for v7,v8
	headerLen = 0;

  CBuffer &packet = pSock->RecvBuffer();
  int sockfd = pSock->Descriptor();

  unsigned short nInVersion = pSock->Version();

  switch (nInVersion)
  {
    case 1:
    case 2:
    case 3:
    {
      unsigned long nUin;
      packet >> nUin
             >> version
             >> command      // main tcp command (start, cancel, ack)
             >> junkShort    // 00 00 to fill in the MSB of the command long int which is read in as a short
             >> nUin
             >> newCommand   // sub command (message/chat/read away message/...)
             >> messageLen   // length of incoming message
      ;
      char id[16];
      snprintf(id, 15, "%lu", nUin);
      userId = Licq::UserId(id, LICQ_PPID);
      break;
    }
    case 4:
    case 5:
    {
      if (!Decrypt_Client(&packet, 4))
      {
        packet.log(Log::Unknown, "Invalid TCPv4 encryption");
        return false;
      }
      unsigned long nUin;
      packet >> nUin
             >> version
             >> junkLong     // checksum
             >> command      // main tcp command (start, cancel, ack)
             >> junkShort    // 00 00 to fill in the MSB of the command long int which is read in as a short
             >> nUin
             >> newCommand   // sub command (message/chat/read away message/...)
             >> messageLen   // length of incoming message
      ;
      char id[16];
      snprintf(id, 15, "%lu", nUin);
      userId = Licq::UserId(id, LICQ_PPID);
      break;
    }
    case 6:
    {
      userId = pSock->userId();
      if (!Decrypt_Client(&packet, 6))
      {
        packet.log(Log::Unknown, "Invalid TCPv6 encryption");
        return false;
      }
      packet.UnpackUnsignedLong(); // Checksum
      command = packet.UnpackUnsignedShort(); // Command
      packet.UnpackUnsignedShort(); // 0x000E
      theSequence = (signed short)packet.UnpackUnsignedShort();
      unsigned long junkLong1, junkLong2, junkLong3;
      packet >> junkLong1 >> junkLong2 >> junkLong3; // maybe always zero ??!
      newCommand = packet.UnpackUnsignedShort();
      ackFlags = packet.UnpackUnsignedShort();
      msgFlags = packet.UnpackUnsignedShort();
      packet >> messageLen;
      break;
    }
    case 7:
    case 8:
    {
      userId = pSock->userId();
      if (!Decrypt_Client(&packet, nInVersion))
      {
        packet.log(Log::Unknown, "Unknown TCPv%d packet", nInVersion);
        break;
      }

      packet.UnpackChar(); // 0x02
      packet.UnpackUnsignedLong(); // Checksum
      command = packet.UnpackUnsignedShort(); // Command
      headerLen = packet.UnpackUnsignedShort();
      theSequence = (signed short)packet.UnpackUnsignedShort();
      packet.incDataPosRead(headerLen - 2);
      newCommand = packet.UnpackUnsignedShort();

      if (pSock->channel() == Licq::TCPSocket::ChannelNormal)
      {
        ackFlags = packet.UnpackUnsignedShort();
        msgFlags = packet.UnpackUnsignedShort();
        packet >> messageLen;

        // Stupid AOL
        if (msgFlags & ICQ_TCPxMSG_URGENT2)
          msgFlags |= ICQ_TCPxMSG_URGENT;
        if (msgFlags & ICQ_TCPxMSG_LIST2)
          msgFlags |= ICQ_TCPxMSG_LIST;
      }
      
      break;
    }
    default:
    {
      gLog.warning(tr("Unknown TCP version %d from socket."), nInVersion);
      break;
    }
  }

  // Some simple validation of the packet
  if (!userId.isValid() || command == 0)
  {
    packet.log(Log::Unknown, "Invalid TCP packet (uin: %s, cmd: %04x)",
               userId.toString().c_str(), command);
    return false;
  }

  bool isOwner = Licq::gUserManager.isOwner(userId);
  if (isOwner || userId != pSock->userId())
  {
    if (isOwner)
      packet.log(Log::Warning, tr("TCP message from self (probable spoof)"));
    else
      packet.log(Log::Warning, tr("TCP message from invalid UIN (%s, expect %s)"),
                 userId.toString().c_str(), pSock->userId().toString().c_str());
    return false;
  }

  // Store our status for later use
  unsigned ownerStatus;
  {
    Licq::OwnerReadGuard o(LICQ_PPID);
    ownerStatus = o->status();
  }

  // find which user was sent
  bool bNewUser = false;
  Licq::UserWriteGuard u(userId, true, &bNewUser);
  if (bNewUser)
    u->setSocketDesc(pSock);

  // Check for spoofing
  if (u->socketDesc(pSock->channel()) != sockfd)
  {
    gLog.warning(tr("User %s (%s) socket (%d) does not match incoming message (%d)."),
        u->getAlias().c_str(), u->accountId().c_str(),
              u->socketDesc(pSock->channel()), sockfd);
  }

  if (pSock->channel() != Licq::TCPSocket::ChannelNormal)
  {
    errorOccured = processPluginMessage(packet, *u, pSock->channel(),
                                        command == ICQ_CMDxTCP_ACK,
                                        0, 0, theSequence, pSock);
  }
  else
  {
  
  // read in the message minus any stupid DOS \r's
  string messageTmp = packet.unpackRawString(messageLen);
  size_t pos;
  while ((pos = messageTmp.find(0x0D)) != string::npos)
    messageTmp.erase(pos, 1);

  message = strdup(parseRtf(messageTmp).c_str());

    if (nInVersion < 6)
    {
    // read in some more stuff common to all tcp packets
    packet >> senderIp
           >> localIp
           >> senderPort
           >> junkChar      // whether use can receive tcp packets directly
           >> ackFlags
           >> msgFlags
    ;
    senderIp = LE_32(senderIp);
    localIp = LE_32(localIp);
  }

  unsigned long nMask = Licq::UserEvent::FlagDirect |
      ((newCommand & ICQ_CMDxSUB_FxMULTIREC) ? (int)Licq::UserEvent::FlagMultiRec : 0) |
      ((msgFlags & ICQ_TCPxMSG_URGENT) ? (int)Licq::UserEvent::FlagUrgent : 0) |
      (pSock->Secure() ? (int)Licq::UserEvent::FlagEncrypted : 0);
  newCommand &= ~ICQ_CMDxSUB_FxMULTIREC;
  bool bAccept = msgFlags & ICQ_TCPxMSG_URGENT || msgFlags & ICQ_TCPxMSG_LIST;
  // Flag as sent urgent as well if we are in occ or dnd and auto-accept is on
  if ( (((ownerStatus & Licq::User::OccupiedStatus) || (u->statusToUser() & Licq::User::OccupiedStatus))
          && u->AcceptInOccupied() ) ||
      (((ownerStatus & Licq::User::DoNotDisturbStatus) || (u->statusToUser() & Licq::User::DoNotDisturbStatus))
          && u->AcceptInDND() ) ||
      (u->statusToUser() != Licq::User::OfflineStatus
          && (u->statusToUser() & (Licq::User::OccupiedStatus | Licq::User::DoNotDisturbStatus)) == 0) )
    bAccept = true;

  //fprintf(stderr, "status: %04X (%04X)  msgtype: %04X\n", ackFlags, u->Status(), msgFlags);

  switch(command)
  {

  //-----START------------------------------------------------------------------
  case ICQ_CMDxTCP_START:
  {
    // Process the status bits
    unsigned short s = 0, ns = 0;

    // Stupid AOL
    if (nInVersion >= 7)
    {
      s = 0;
      ns = ackFlags;
    }
    else
      s = msgFlags & 0xFF80;

    if (s & ICQ_TCPxMSG_FxINVISIBLE)
    {
      s &= ~ICQ_TCPxMSG_FxINVISIBLE;
      ns |= ICQ_STATUS_FxPRIVATE;
    }
    switch(s)
    {
      case ICQ_TCPxMSG_FxONLINE: ns |= ICQ_STATUS_ONLINE; break;
      case ICQ_TCPxMSG_FxAWAY: ns |= ICQ_STATUS_AWAY; break;
      case ICQ_TCPxMSG_FxOCCUPIED: ns |= ICQ_STATUS_OCCUPIED; break;
      case ICQ_TCPxMSG_FxNA: ns |= ICQ_STATUS_NA; break;
      case ICQ_TCPxMSG_FxDND: ns |= ICQ_STATUS_DND; break;
      default:
        ns = ICQ_STATUS_OFFLINE;
          gLog.warning(tr("Unknown TCP status: %04X"), msgFlags);
          break;
      }
      //fprintf(stderr, "%08lX\n", addStatusFlags(ns, u));
    /*if (!bNewUser && ns != ICQ_STATUS_OFFLINE &&
        !((ns & ICQ_STATUS_FxPRIVATE) && !u->isOnline()))*/
    if (!bNewUser && ns != ICQ_STATUS_OFFLINE &&
          !(ns == ICQ_STATUS_ONLINE && (u->status() & Licq::User::FreeForChatStatus)) &&
          ns != (icqStatusFromStatus(u->status()) | (u->isInvisible() ? ICQ_STATUS_FxPRIVATE : 0)))
      {
      bool r = u->OfflineOnDisconnect() || !u->isOnline();
        u->statusChanged(statusFromIcqStatus(ns));
        gLog.info(tr("%s (%s) is %s to us."), u->getAlias().c_str(),
            u->id().toString().c_str(), u->statusString().c_str());
      if (r) u->SetOfflineOnDisconnect(true);
    }

    // Process the command
    switch(newCommand)
    {
      case ICQ_CMDxSUB_MSG:  // straight message from a user
      {
            string msg = Licq::gTranslator.serverToClient(message);

        unsigned long back = 0xFFFFFF, fore = 0x000000;
            if (nInVersion < 6)
            {
          if (packet.getDataPosRead() + 4 >
                              (packet.getDataStart() + packet.getDataSize()))
          {
            theSequence = (signed short)packet.UnpackUnsignedShort();
          }
          else
            packet >> theSequence;

              packet >> licqChar >> licqVersion;
              nMask |= licqVersion;
            }
            else
            {
          packet >> fore >> back;
          if( fore == back ) {
            back = 0xFFFFFF;
            fore = 0x000000;
          }

              // Check if message is marked as UTF8
              unsigned long guidlen;
              packet >> guidlen;
              if (guidlen == sizeof(ICQ_CAPABILITY_UTF8_STR)-1)
              {
                string guid = packet.unpackRawString(guidlen);
                if (guid == ICQ_CAPABILITY_UTF8_STR)
                {
                  // Message is UTF8
                  msg = Licq::gTranslator.fromUnicode(msg);
                }
              }
              // TODO: Could there be multiple GUIDs that we need to check?
            }

				if (licqChar == 'L')
            gLog.info(tr("Message from %s (%s) [Licq %s]."),
                u->getAlias().c_str(), userId.toString().c_str(),
                Licq::UserEvent::licqVersionToString(licqVersion).c_str());
	  else
            gLog.info(tr("Message from %s (%s)."), u->getAlias().c_str(), userId.toString().c_str());

          CPT_AckGeneral p(newCommand, theSequence, true, bAccept, *u);
        AckTCP(p, pSock);

          Licq::EventMsg* e = new Licq::EventMsg(msg, Licq::EventMsg::TimeNow, nMask);
        e->SetColor(fore, back);

        // If we are in DND or Occupied and message isn't urgent then we ignore it
        if (!bAccept)
        {
            if (ownerStatus & (Licq::User::OccupiedStatus | Licq::User::DoNotDisturbStatus))
            {
            delete e;
            break;
          }
        }
        // Add the user to our list if they are new
        if (bNewUser)
        {
            if (gDaemon.ignoreType(Daemon::IgnoreNewUsers))
            {
            // FIXME should log a message here or in reject event
            // FIXME should either refuse the event or have a special auto response
            // for rejected events instead of pretending to accept the user
              gDaemon.rejectEvent(userId, e);
              break;
            }
          bNewUser = false;
        }

          if (!gDaemon.addUserEvent(*u, e))
            break;
          gOnEventManager.performOnEvent(OnEventData::OnEventMessage, *u);
          break;
        }
      case ICQ_CMDxTCP_READxNAxMSG:
      case ICQ_CMDxTCP_READxDNDxMSG:
      case ICQ_CMDxTCP_READxOCCUPIEDxMSG:
      case ICQ_CMDxTCP_READxFFCxMSG:
      case ICQ_CMDxTCP_READxAWAYxMSG:  // read away message
          {
            if (nInVersion < 6)
            {
          if (packet.getDataPosRead() + 4 >
                              (packet.getDataStart() + packet.getDataSize()))
          {
            theSequence = (signed short)packet.UnpackUnsignedShort();
          }
          else
            packet >> theSequence;

              packet >> licqChar >> licqVersion;
            }
            else
          packet >> junkLong >> junkLong;

        if (licqChar == 'L')
            gLog.info(tr("%s (%s) requested auto response [Licq %s]."),
                u->getAlias().c_str(), userId.toString().c_str(),
                Licq::UserEvent::licqVersionToString(licqVersion).c_str());
          else
            gLog.info(tr("%s (%s) requested auto response."), u->getAlias().c_str(), userId.toString().c_str());

          CPT_AckGeneral p(newCommand, theSequence, true, false, *u);
        AckTCP(p, pSock);

          Licq::gStatistics.increase(Licq::Statistics::AutoResponseCheckedCounter);
        u->SetLastCheckedAutoResponse();

          Licq::gPluginManager.pushPluginSignal(new Licq::PluginSignal(
              Licq::PluginSignal::SignalUser,
              Licq::PluginSignal::UserEvents, u->id()));
          break;
      }

      case ICQ_CMDxSUB_URL:  // url sent
      {
        unsigned long back = 0xFFFFFF, fore = 0x000000;
            if (nInVersion < 6)
            {
          if (packet.getDataPosRead() + 4 >
                              (packet.getDataStart() + packet.getDataSize()))
          {
            theSequence = (signed short)packet.UnpackUnsignedShort();
          }
          else
            packet >> theSequence;

              packet >> licqChar >> licqVersion;
              nMask |= licqVersion;
            }
            else
            {
          packet >> fore >> back;
          if(fore == back)
          {
            fore = 0x000000;
            back = 0xFFFFFF;
          }
        }
        if (licqChar == 'L')
            gLog.info(tr("URL from %s (%s) [Licq %s]."), u->getAlias().c_str(),
                userId.toString().c_str(), Licq::UserEvent::licqVersionToString(licqVersion).c_str());
          else
            gLog.info(tr("URL from %s (%s)."), u->getAlias().c_str(), userId.toString().c_str());

          Licq::EventUrl* e = Licq::EventUrl::Parse(message, Licq::EventUrl::TimeNow, nMask);
        if (e == NULL)
        {
          packet.log(Log::Warning, tr("Invalid URL message"));
          errorOccured = true;
          break;
        }
        e->SetColor(fore, back);

          CPT_AckGeneral p(newCommand, theSequence, true, bAccept, *u);
        AckTCP(p, pSock);

        // If we are in DND or Occupied and message isn't urgent then we ignore it
        if (!bAccept)
        {
            if (ownerStatus & (Licq::User::OccupiedStatus | Licq::User::DoNotDisturbStatus))
            {
            delete e;
            break;
          }
        }
        // Add the user to our list if they are new
        if (bNewUser)
        {
            if (gDaemon.ignoreType(Daemon::IgnoreNewUsers))
            {
              gDaemon.rejectEvent(userId, e);
              break;
            }
          bNewUser = false;
        }

          if (!gDaemon.addUserEvent(*u, e))
            break;
          gOnEventManager.performOnEvent(OnEventData::OnEventUrl, *u);
          break;
        }

      // Contact List
      case ICQ_CMDxSUB_CONTACTxLIST:
      {
        unsigned long back = 0xFFFFFF, fore = 0x000000;
            if (nInVersion < 6)
            {
          if (packet.getDataPosRead() + 4 >
                              (packet.getDataStart() + packet.getDataSize()))
          {
            theSequence = (signed short)packet.UnpackUnsignedShort();
          }
          else
            packet >> theSequence;

              packet >> licqChar >> licqVersion;
              nMask |= licqVersion;
            }
            else
            {
          packet >> fore >> back;
          if(fore == back) {
            fore = 0x000000;
            back = 0xFFFFFF;
          }
        }
        if (licqChar == 'L')
            gLog.info(tr("Contact list from %s (%s) [Licq %s]."),
                u->getAlias().c_str(), userId.toString().c_str(),
                Licq::UserEvent::licqVersionToString(licqVersion).c_str());
          else
            gLog.info(tr("Contact list from %s (%s).\n"),
                u->getAlias().c_str(), userId.toString().c_str());

          Licq::EventContactList* e = Licq::EventContactList::Parse(message, Licq::EventContactList::TimeNow, nMask);
        if (e == NULL)
        {
          packet.log(Log::Warning, tr("Invalid contact list message"));
          errorOccured = true;
          break;
        }
        e->SetColor(fore, back);

          CPT_AckGeneral p(newCommand, theSequence, true, bAccept, *u);
        AckTCP(p, pSock);

        // If we are in DND or Occupied and message isn't urgent then we ignore it
        if (!bAccept)
        {
            if (ownerStatus & (Licq::User::OccupiedStatus | Licq::User::DoNotDisturbStatus))
            {
            delete e;
            break;
          }
        }
        // Add the user to our list if they are new
        if (bNewUser)
        {
            if (gDaemon.ignoreType(Daemon::IgnoreNewUsers))
            {
              gDaemon.rejectEvent(userId, e);
              break;
            }
          bNewUser = false;
        }

          if (!gDaemon.addUserEvent(*u, e))
            break;
          gOnEventManager.performOnEvent(OnEventData::OnEventMessage, *u);
          break;
        }

      // Chat Request
      case ICQ_CMDxSUB_CHAT:
      {
        char szChatClients[1024];
        packet.UnpackString(szChatClients, sizeof(szChatClients));
        packet.UnpackUnsignedLong(); // reversed port
        unsigned short nPort = packet.UnpackUnsignedLong();
            if (nInVersion < 6)
            {
          if (packet.getDataPosRead() + 4 >
                              (packet.getDataStart() + packet.getDataSize()))
          {
            theSequence = (signed short)packet.UnpackUnsignedShort();
          }
          else
            packet >> theSequence;

              packet >> licqChar >> licqVersion;
            }

        if (licqChar == 'L')
            gLog.info(tr("Chat request from %s (%s) [Licq %s]."),
                u->getAlias().c_str(), userId.toString().c_str(),
                Licq::UserEvent::licqVersionToString(licqVersion).c_str());
          else
            gLog.info(tr("Chat request from %s (%s)."),
                u->getAlias().c_str(), userId.toString().c_str());

        // translating string with translation table
        gTranslator.ServerToClient (message);
          Licq::EventChat* e = new Licq::EventChat(message, szChatClients, nPort, theSequence,
              Licq::EventChat::TimeNow, nMask | licqVersion);

        // Add the user to our list if they are new
        if (bNewUser)
        {
            if (gDaemon.ignoreType(Daemon::IgnoreNewUsers))
            {
              gDaemon.rejectEvent(userId, e);
              break;
            }
          bNewUser = false;
        }

          if (!gDaemon.addUserEvent(*u, e))
            break;
          gOnEventManager.performOnEvent(OnEventData::OnEventChat, *u);
          break;
        }

      // File transfer
      case ICQ_CMDxSUB_FILE:
      {
        unsigned short nLenFilename;
        unsigned long nFileLength;
        packet >> junkLong
               >> nLenFilename;
          string filename = packet.unpackRawString(nLenFilename);
        packet >> nFileLength
               >> junkLong;
            if (nInVersion < 6)
            {
          if (packet.getDataPosRead() + 4 >
                              (packet.getDataStart() + packet.getDataSize()))
          {
            theSequence = (signed short)packet.UnpackUnsignedShort();
          }
          else
            packet >> theSequence;

              packet >> licqChar >> licqVersion;
            }

        if (licqChar == 'L')
            gLog.info(tr("File transfer request from %s (%s) [Licq %s]."),
                u->getAlias().c_str(), userId.toString().c_str(),
                Licq::UserEvent::licqVersionToString(licqVersion).c_str());
          else
            gLog.info(tr("File transfer request from %s (%s)."),
                u->getAlias().c_str(), userId.toString().c_str());

        list<string> filelist;
        filelist.push_back(filename);

        // translating string with translation table
        gTranslator.ServerToClient (message);
          Licq::EventFile* e = new Licq::EventFile(filename.c_str(), message, nFileLength,
              filelist, theSequence, Licq::EventFile::TimeNow, nMask | licqVersion);
        // Add the user to our list if they are new
        if (bNewUser)
        {
            if (gDaemon.ignoreType(Daemon::IgnoreNewUsers))
            {
              gDaemon.rejectEvent(userId, e);
              break;
            }
          bNewUser = false;
        }

          if (!gDaemon.addUserEvent(*u, e))
            break;
          gOnEventManager.performOnEvent(OnEventData::OnEventFile, *u);
          break;
        }

			// Yuck, ICBM
			// XXX If we are in DND or OCC, don't accept the message!
			case ICQ_CMDxSUB_ICBM:
			{
				unsigned short nLen;
				unsigned long nLongLen;

				packet >> nLen;
				packet.incDataPosRead(18);
				packet >> nLongLen; // plugin name len

          string plugin = packet.unpackRawString(nLongLen);

				packet.incDataPosRead(nLen - 22 - nLongLen);
				packet.incDataPosRead(4); // bytes left in packet
				packet >> nLongLen; // message len

				int nICBMCommand = 0;
          if (plugin.find("File") != string::npos)
					nICBMCommand = ICQ_CMDxSUB_FILE;
          else if (plugin.find("URL") != string::npos)
					nICBMCommand = ICQ_CMDxSUB_URL;
          else if (plugin.find("Chat") != string::npos)
					nICBMCommand = ICQ_CMDxSUB_CHAT;
          else if (plugin.find("Contacts") != string::npos)
					nICBMCommand = ICQ_CMDxSUB_CONTACTxLIST;
          else
          {
            gLog.info(tr("Unknown ICBM plugin type: %s"), plugin.c_str());
            break;
          }

          char* szMessage = new char[nLongLen+1];
				for (unsigned long i = 0; i < nLongLen; i++)
					packet >> szMessage[i];
				szMessage[nLongLen] = '\0';

				switch (nICBMCommand)
				{
				case ICQ_CMDxSUB_FILE:
				{
					unsigned long nFileSize;
					packet.incDataPosRead(2); // port (BE)
					packet.incDataPosRead(2); // unknown
					packet >> nLen; // filename len, including NULL
              string filename = packet.unpackRawString(nLen);
					packet >> nFileSize;
					packet.incDataPosRead(2); // reversed port (BE)

              gLog.info(tr("File transfer request from %s (%s)."),
                  u->getAlias().c_str(), userId.toString().c_str());

              list<string> filelist;
              filelist.push_back(filename);

					// translating string with translation table
					gTranslator.ServerToClient(szMessage);
                Licq::EventFile* e = new Licq::EventFile(filename.c_str(), szMessage, nFileSize,
                    filelist, theSequence, Licq::EventFile::TimeNow, nMask);
					if (bNewUser)
					{
                  if (gDaemon.ignoreType(Daemon::IgnoreNewUsers))
                  {
                  gDaemon.rejectEvent(userId, e);
                  break;
                }
						bNewUser = false;
					}

                if (!gDaemon.addUserEvent(*u, e))
                  break;
                gOnEventManager.performOnEvent(OnEventData::OnEventFile, *u);
                break;
              }
				case ICQ_CMDxSUB_CHAT:
				{
					char szChatClients[1024];
 					packet.UnpackString(szChatClients, sizeof(szChatClients));
					nPort = packet.UnpackUnsignedShortBE();
					packet >> nPortReversed;

					if (nPort == 0)
						nPort = nPortReversed;

              gLog.info(tr("Chat request from %s (%s)."),
                  u->getAlias().c_str(), userId.toString().c_str());

					// translating string with translation table
					gTranslator.ServerToClient(szMessage);
                Licq::EventChat* e = new Licq::EventChat(szMessage, szChatClients, nPort,
                    theSequence, Licq::EventChat::TimeNow, nMask);
					if (bNewUser)
					{
                  if (gDaemon.ignoreType(Daemon::IgnoreNewUsers))
                  {
                  gDaemon.rejectEvent(userId, e);
                  break;
                }
						bNewUser = false;
					}

                if (!gDaemon.addUserEvent(*u, e))
                  break;
                gOnEventManager.performOnEvent(OnEventData::OnEventChat, *u);
                break;
              }
				case ICQ_CMDxSUB_URL:
				{
              gLog.info(tr("URL from %s (%s)."), u->getAlias().c_str(), userId.toString().c_str());
                Licq::EventUrl* e = Licq::EventUrl::Parse(szMessage, Licq::EventUrl::TimeNow, nMask);
					if (e == NULL)
					{
                                          packet.log(Log::Warning, tr("Invalid URL message"));
                                          errorOccured = true;
                                          break;
					}

					if (bNewUser)
					{
                  if (gDaemon.ignoreType(Daemon::IgnoreNewUsers))
                  {
                  gDaemon.rejectEvent(userId, e);
                  break;
                }
						bNewUser = false;
					}

                if (!gDaemon.addUserEvent(*u, e))
                  break;
                gOnEventManager.performOnEvent(OnEventData::OnEventUrl, *u);
                break;
              }
				case ICQ_CMDxSUB_CONTACTxLIST:
				{
              gLog.info(tr("Contact list from %s (%s)."),
                  u->getAlias().c_str(), userId.toString().c_str());
                Licq::EventContactList* e = Licq::EventContactList::Parse(szMessage,
                    Licq::EventContactList::TimeNow, nMask);
					if (e == NULL)
					{
                                          packet.log(Log::Warning, tr("Invalid contact list message"));
                                          errorOccured = true;
                                          break;
					}

					if (bNewUser)
					{
                  if (gDaemon.ignoreType(Daemon::IgnoreNewUsers))
                  {
                  gDaemon.rejectEvent(userId, e);
                  break;
                }
						bNewUser = false;
					}

                if (!gDaemon.addUserEvent(*u, e))
                  break;
                gOnEventManager.performOnEvent(OnEventData::OnEventMessage, *u);
                break;
              }
          } // switch nICBMCommand
          delete [] szMessage;

          break;
        }

      // Old-style encryption request:
      case ICQ_CMDxSUB_SECURExOLD:
      {
          gLog.info(tr("Received old-style key request from %s (%s) but we do not support it."),
              u->getAlias().c_str(), userId.toString().c_str());
        // Send the nack back
          CPT_AckOldSecureChannel p(theSequence, *u);
        AckTCP(p, pSock);
        break;
      }

      // Secure channel request
      case ICQ_CMDxSUB_SECURExOPEN:
      {
#ifdef USE_OPENSSL
            if (nInVersion < 6)
            {
          if (packet.getDataPosRead() + 4 >
                              (packet.getDataStart() + packet.getDataSize()))
          {
            theSequence = (signed short)packet.UnpackUnsignedShort();
          }
          else
            packet >> theSequence;

              packet >> licqChar >> licqVersion;
            }

        if (licqChar == 'L')
            gLog.info(tr("Secure channel request from %s (%s) [Licq %s]."),
                u->getAlias().c_str(), userId.toString().c_str(),
                Licq::UserEvent::licqVersionToString(licqVersion).c_str());
          else
            gLog.info(tr("Secure channel request from %s (%s)."),
                u->getAlias().c_str(), userId.toString().c_str());

          CPT_AckOpenSecureChannel p(theSequence, true, *u);
        AckTCP(p, pSock);

        if (!pSock->SecureListen())
        {
          errorOccured = true;
          break;
        }
        u->SetSecure(true);

        // Add the user to our list if they are new
        if (bNewUser)
        {
          if (gDaemon.ignoreType(Daemon::IgnoreNewUsers))
            break;
          bNewUser = false;
        }

        u->SetSendServer(false);
          Licq::gPluginManager.pushPluginSignal(new Licq::PluginSignal(
              Licq::PluginSignal::SignalUser,
              Licq::PluginSignal::UserSecurity, u->id(), 1));

          gLog.info(tr("Secure channel established with %s (%s)"),
              u->getAlias().c_str(), userId.toString().c_str());

        break;

#else // We do not support OpenSSL
          gLog.info(tr("Received secure channel request from %s (%s) but we do not support OpenSSL."),
              u->getAlias().c_str(), userId.toString().c_str());
        // Send the nack back
          CPT_AckOpenSecureChannel p(theSequence, false, *u);
        AckTCP(p, pSock);
        break;
#endif
      }


      // Secure channel close request
      case ICQ_CMDxSUB_SECURExCLOSE:
      {
#ifdef USE_OPENSSL
            if (nInVersion < 6)
            {
          if (packet.getDataPosRead() + 4 >
                              (packet.getDataStart() + packet.getDataSize()))
          {
            theSequence = (signed short)packet.UnpackUnsignedShort();
          }
          else
            packet >> theSequence;

              packet >> licqChar >> licqVersion;
            }

        if (licqChar == 'L')
            gLog.info(tr("Secure channel closed by %s (%s) [Licq %s]."),
                u->getAlias().c_str(), userId.toString().c_str(),
                Licq::UserEvent::licqVersionToString(licqVersion).c_str());
          else
            gLog.info(tr("Secure channel closed by %s (%s)."),
                u->getAlias().c_str(), userId.toString().c_str());

        // send ack
          CPT_AckCloseSecureChannel p(theSequence, *u);
        AckTCP(p, pSock);

        pSock->SecureStop();
        u->SetSecure(false);
          Licq::gPluginManager.pushPluginSignal(new Licq::PluginSignal(
              Licq::PluginSignal::SignalUser,
              Licq::PluginSignal::UserSecurity, u->id(), 0));
          break;

#else // We do not support OpenSSL
          gLog.info(tr("Received secure channel close from %s (%s) but we do not support OpenSSL."),
              u->getAlias().c_str(), userId.toString().c_str());
        // Send the nack back
          CPT_AckCloseSecureChannel p(theSequence, *u);
        AckTCP(p, pSock);
        break;
#endif
      }

        default:
          packet.log(Log::Unknown, "Unknown TCP message type (%04x)",
                     newCommand);
          errorOccured = true;
      }
      break;
    }

  //-----ACK--------------------------------------------------------------------
  case ICQ_CMDxTCP_ACK:  // message received packet
  {
    // If this is not from a user on our list then ignore it
    if (bNewUser) break;

      Licq::ExtendedData *pExtendedAck = NULL;

    switch (newCommand)
    {
      case ICQ_CMDxSUB_MSG:
      case ICQ_CMDxTCP_READxNAxMSG:
      case ICQ_CMDxTCP_READxDNDxMSG:
      case ICQ_CMDxTCP_READxOCCUPIEDxMSG:
      case ICQ_CMDxTCP_READxAWAYxMSG:
      case ICQ_CMDxTCP_READxFFCxMSG:
      case ICQ_CMDxSUB_URL:
      case ICQ_CMDxSUB_CONTACTxLIST:
            if (nInVersion < 6)
            {
          if (packet.getDataPosRead() + 4 >
                              (packet.getDataStart() + packet.getDataSize()))
          {
            theSequence = (signed short)packet.UnpackUnsignedShort();
          }
          else
            packet >> theSequence;

              packet >> licqChar >> licqVersion;
            }
            else
          packet >> junkLong >> junkLong;
        break;

      case ICQ_CMDxSUB_CHAT:
      {
        char ul[1024];
        packet.UnpackString(ul, sizeof(ul));
        packet >> nPortReversed   // port backwards
               >> nPort;    // port to connect to for chat
            if (nInVersion < 6)
            {
          if (packet.getDataPosRead() + 4 >
                              (packet.getDataStart() + packet.getDataSize()))
          {
            theSequence = (signed short)packet.UnpackUnsignedShort();
          }
          else
            packet >> theSequence;

              packet >> licqChar >> licqVersion;
            }

        if (nPort == 0) nPort = (nPortReversed >> 8) | ((nPortReversed & 0xFF) << 8);

          pExtendedAck = new Licq::ExtendedData(nPort != 0, nPort, message);
          break;
        }

      case ICQ_CMDxSUB_FILE:
      {
         /* 50 A5 82 00 03 00 DA 07 00 00 50 A5 82 00 03 00 0A 00 6E 6F 20 74 68 61
            6E 6B 73 00 D1 EF 04 9F 7F 00 00 01 4A 1F 00 00 04 01 00 00 00 00 00 00
            00 01 00 00 00 00 00 00 00 00 00 00 03 00 00 00 */
         packet >> nPortReversed
                >> junkShort;
         for (int i = 0; i < junkShort; i++) packet >> junkChar;
         packet >> junkLong
                >> nPort;
            if (nInVersion < 6)
            {
           if (packet.getDataPosRead() + 4 >
                              (packet.getDataStart() + packet.getDataSize()))
           {
             theSequence = (signed short)packet.UnpackUnsignedShort();
           }
           else
             packet >> theSequence;

              packet >> licqChar >> licqVersion;
            }

         // Some clients only send the first port (reversed)
         if (nPort == 0) nPort = (nPortReversed >> 8) | ((nPortReversed & 0xFF) << 8);

          pExtendedAck = new Licq::ExtendedData(nPort != 0, nPort, message);
          break;
        }

		  case ICQ_CMDxSUB_ICBM:
			{
				unsigned short nLen;
				unsigned long nLongLen;

				packet >> nLen;
				packet.incDataPosRead(18); // eh?
				packet >> nLongLen; // Plugin name len

          string plugin = packet.unpackRawString(nLongLen);

				packet.incDataPosRead(nLen - 22 - nLongLen);
				packet.incDataPosRead(4); // left in packet

				int nICBMCommand = 0;
          if (plugin.find("File") != string::npos)
					nICBMCommand = ICQ_CMDxSUB_FILE;
          else if (plugin.find("Chat") != string::npos)
					nICBMCommand = ICQ_CMDxSUB_CHAT;
          else if (plugin.find("URL") != string::npos)
					nICBMCommand = ICQ_CMDxSUB_URL;
          else if (plugin.find("Contacts") != string::npos)
					nICBMCommand = ICQ_CMDxSUB_CONTACTxLIST;
          else
          {
            gLog.info(tr("Unknown direct ack ICBM plugin type: %s"), plugin.c_str());

            if (bNewUser)
            {
              u.unlock();
              Licq::gUserManager.removeUser(userId, false);
            }
					return true;
				}

				packet >> nLongLen;
          string msg = packet.unpackRawString(nLongLen);

				switch (nICBMCommand)
				{
				case ICQ_CMDxSUB_FILE:
				{
					nPort = packet.UnpackUnsignedShortBE();
					packet.incDataPosRead(2); // unknown
					packet >> nLen; // filename len, including NULL
					packet.incDataPosRead(nLen); // filename
					packet.incDataPosRead(4); // file size
					packet >> nPortReversed;

					if (nPort == 0)
						nPort = nPortReversed;

              pExtendedAck = new Licq::ExtendedData(nPort != 0, nPort,
                  message[0] != '\0' ? message : msg.c_str());
	      break;
	    }
				case ICQ_CMDxSUB_CHAT:
				{
					char ul[1024];
					packet.UnpackString(ul, sizeof(ul));
					nPort = packet.UnpackUnsignedShortBE();
					packet >> nPortReversed;

					if (nPort == 0)
						nPort = nPortReversed;

          /* this is silly, but appearantly the only way to tell if chat is
             accepted (multiparty chat is accepted if port is 0, and rejected
             otherwise, normal chat is accepted if the port is given and
             rejected if it's 0) */
          bool bAccepted = (nPort != 0 && ul[0] == '\0') ||
                           (nPort == 0 && ul[0] != '\0');
              pExtendedAck = new Licq::ExtendedData(bAccepted, nPort,
                  message[0] != '\0' ? message : msg.c_str());
	      break;
	    }
	  } // switch nICBMCommand

	  break;
	}

#ifdef USE_OPENSSL
      case ICQ_CMDxSUB_SECURExOPEN:
      {
            if (nInVersion < 6)
            {
          if (packet.getDataPosRead() + 4 >
                              (packet.getDataStart() + packet.getDataSize()))
          {
            theSequence = (signed short)packet.UnpackUnsignedShort();
          }
          else
            packet >> theSequence;

              packet >> licqChar >> licqVersion;
            }

        char l[32] = "";
          if (licqChar == 'L')
            sprintf(l, " [Licq %s]", Licq::UserEvent::licqVersionToString(licqVersion).c_str());
          gLog.info(tr("Secure channel response from %s (%s)%s."),
              u->getAlias().c_str(), userId.toString().c_str(), l);

          Licq::Event* e = NULL;

        // Check if the response is ok
        if (message[0] == '\0')
        {
            gLog.info(tr("%s (%s) does not support OpenSSL."),
                u->getAlias().c_str(), userId.toString().c_str());
          u->SetSecure(false);
            Licq::gPluginManager.pushPluginSignal(new Licq::PluginSignal(
                Licq::PluginSignal::SignalUser,
                Licq::PluginSignal::UserSecurity, u->id(), 0));
          // find the event, fail it
            e = DoneEvent(sockfd, theSequence, Licq::Event::ResultFailed);
          }
        else
        {
          // Find the event, succeed it
            e = DoneEvent(sockfd, theSequence, Licq::Event::ResultSuccess);

          // Check that a request was in progress...should always be ok
          if (e == NULL)
          {
              gLog.warning(tr("Secure channel response from %s (%s) when no request in progress."),
                  u->getAlias().c_str(), userId.toString().c_str());
            // Close the connection as we are in trouble
            u->SetSecure(false);
              u.unlock();
              if (bNewUser)
                Licq::gUserManager.removeUser(userId, false);
              Licq::gPluginManager.pushPluginSignal(new Licq::PluginSignal(
                  Licq::PluginSignal::SignalUser,
                  Licq::PluginSignal::UserSecurity, userId, 0));
            return false;
          }

          if (!pSock->SecureConnect())
          {
            errorOccured = true;
              e->m_eResult = Licq::Event::ResultFailed;
            }
          else
          {
              gLog.info(tr("Secure channel established with %s (%s)"),
                  u->getAlias().c_str(), userId.toString().c_str());
            u->SetSecure(true);
              Licq::gPluginManager.pushPluginSignal(new Licq::PluginSignal(
                  Licq::PluginSignal::SignalUser,
                  Licq::PluginSignal::UserSecurity, u->id(), 1));
            }
          }

          // finish up
          e->mySubResult = Licq::Event::SubResultAccept;
          u.unlock();
          if (bNewUser)
            Licq::gUserManager.removeUser(userId, false);
        ProcessDoneEvent(e);

        // get out of here now as we don't want standard ack processing
        return !errorOccured;
      }


      case ICQ_CMDxSUB_SECURExCLOSE:
      {
            if (nInVersion < 6)
            {
          if (packet.getDataPosRead() + 4 >
                              (packet.getDataStart() + packet.getDataSize()))
          {
            theSequence = (signed short)packet.UnpackUnsignedShort();
          }
          else
            packet >> theSequence;

              packet >> licqChar >> licqVersion;
            }

        char l[32] = "";
          if (licqChar == 'L')
            sprintf(l, " [Licq %s]", Licq::UserEvent::licqVersionToString(licqVersion).c_str());
          gLog.info(tr("Secure channel with %s (%s) closed %s."),
              u->getAlias().c_str(), userId.toString().c_str(), l);

        // Find the event, succeed it
          Licq::Event* e = DoneEvent(sockfd, theSequence, Licq::Event::ResultSuccess);

        // Check that a request was in progress...should always be ok
        if (e == NULL)
        {
          // Close the connection as we are in trouble
            if (bNewUser)
            {
              u.unlock();
              Licq::gUserManager.removeUser(userId, false);
            }
          delete e;
          return false;
        }

        pSock->SecureStop();
        u->SetSecure(false);
          Licq::gPluginManager.pushPluginSignal(new Licq::PluginSignal(
              Licq::PluginSignal::SignalUser,
              Licq::PluginSignal::UserSecurity, u->id(), 0));

          u.unlock();
          if (bNewUser)
            Licq::gUserManager.removeUser(userId, false);

          // finish up
          e->mySubResult = Licq::Event::SubResultAccept;
        ProcessDoneEvent(e);

        // get out of here now as we don't want standard ack processing
        return true;
      }
#endif

        default:
          packet.log(Log::Unknown, "Unknown TCP Ack subcommand (%04x)",
                     newCommand);
          errorOccured = true;
      }

    char l[32] = "";
      if (licqChar == 'L')
        sprintf(l, " [Licq %s]", Licq::UserEvent::licqVersionToString(licqVersion).c_str());

    // translating string with translation table
    gTranslator.ServerToClient (message);
    // output the away message if there is one (ie if user status is not online)
      unsigned subResult;
    if (ackFlags == ICQ_TCPxACK_REFUSE)
    {
      gLog.info(tr("Refusal from %s (#%hu)%s."), u->getAlias().c_str(), -theSequence, l);
        subResult = Licq::Event::SubResultRefuse;
      }
      else
      {
      // Update the away message if it's changed
      if (u->autoResponse() != message)
      {
          u->setAutoResponse(message);
        u->SetShowAwayMsg(*message);
        gLog.info(tr("Auto response from %s (#%hu)%s."), u->getAlias().c_str(), -theSequence, l);
      }

      switch(ackFlags)
      {
        case ICQ_TCPxACK_ONLINE:
          gLog.info(tr("Ack from %s (#%hu)%s."), u->getAlias().c_str(), -theSequence, l);
            if (pExtendedAck && !pExtendedAck->accepted())
              subResult = Licq::Event::SubResultReturn;
            else
              subResult = Licq::Event::SubResultAccept;
            break;
        case ICQ_TCPxACK_AWAY:
        case ICQ_TCPxACK_NA:
        case ICQ_TCPxACK_OCCUPIEDx2: //auto decline due to occupied mode
          gLog.info(tr("Ack from %s (#%hu)%s."), u->getAlias().c_str(), -theSequence, l);
            subResult = Licq::Event::SubResultRefuse;
            break;
        case ICQ_TCPxACK_OCCUPIED:
        case ICQ_TCPxACK_DND:
          gLog.info(tr("Returned from %s (#%hu)%s."), u->getAlias().c_str(), -theSequence, l);
            subResult = Licq::Event::SubResultReturn;
            break;
        case ICQ_TCPxACK_OCCUPIEDxCAR:
        case ICQ_TCPxACK_DNDxCAR:
          gLog.info(tr("Custom %s response from %s (#%hu)%s."),
              (ackFlags == ICQ_TCPxACK_DNDxCAR ? tr("DnD") : tr("Occupied")),
              u->getAlias().c_str(), -theSequence, l);
            subResult = Licq::Event::SubResultAccept; // FIXME: or should this be ACK_RETURN ?
            break;
          default:
          gLog.unknown(tr("Unknown ack flag from %s (#%hu): %04x %s"),
              u->getAlias().c_str(), -theSequence, ackFlags, l);
            subResult = Licq::Event::SubResultAccept;
      }
    }

      Licq::Event *e = DoneEvent(sockfd, theSequence, Licq::Event::ResultAcked);
    if (e != NULL)
    {
      e->m_pExtendedAck = pExtendedAck;
        e->mySubResult = subResult;

        u.unlock();
        if (bNewUser)
          Licq::gUserManager.removeUser(userId, false);

      ProcessDoneEvent(e);
      return true;
    }
    else
    {
      gLog.warning(tr("Ack for unknown event."));
      errorOccured = true;
      delete pExtendedAck;
    }
    break;
  }

  //-----CANCEL-----------------------------------------------------------------
  case ICQ_CMDxTCP_CANCEL:
    // If from a new user, ignore it
    if (bNewUser) break;

    switch (newCommand)
    {
      case ICQ_CMDxSUB_CHAT:
      {
          gLog.info(tr("Chat request from %s (%s) cancelled."),
              u->getAlias().c_str(), userId.toString().c_str());
            if (nInVersion < 6)
            {
          packet >> junkLong >> junkLong >> junkShort >> junkChar;
          if (packet.getDataPosRead() + 4 >
                              (packet.getDataStart() + packet.getDataSize()))
          {
            theSequence = (signed short)packet.UnpackUnsignedShort();
          }
          else
            packet >> theSequence;
        }

        for (unsigned short i = 0; i < u->NewMessages(); i++)
        {
          if (u->EventPeek(i)->Sequence() == theSequence)
          {
            u->CancelEvent(i);
            break;
          }
        }
        break;
      }
      case ICQ_CMDxSUB_FILE:
      {
          gLog.info(tr("File transfer request from %s (%s) cancelled."),
              u->getAlias().c_str(), userId.toString().c_str());
            if (nInVersion < 6)
            {
          packet >> junkLong >> junkShort >> junkChar >> junkLong >> junkLong;
          if (packet.getDataPosRead() + 4 >
                              (packet.getDataStart() + packet.getDataSize()))
          {
            theSequence = (signed short)packet.UnpackUnsignedShort();
          }
          else
            packet >> theSequence;
        }

        for (unsigned short i = 0; i < u->NewMessages(); i++)
        {
          if (u->EventPeek(i)->Sequence() == theSequence)
          {
            u->CancelEvent(i);
            break;
          }
        }
        break;
      }

      default:
         break;
    }

    break;

    default:
      packet.log(Log::Unknown, "Unknown TCP packet (command 0x%04x)",
                 command);
      errorOccured = true;
      break;
  }
  
  }
  if (bNewUser)
  {
    u.unlock();
    Licq::gUserManager.removeUser(userId, false);
    return false;
  }
  if (message)
    delete [] message;
  return !errorOccured;
}

bool IcqProtocol::processPluginMessage(CBuffer &packet, Licq::User* u,
    int channel,
                                      bool bIsAck,
                                      unsigned long nMsgID1,
                                      unsigned long nMsgID2,
                                      unsigned short nSequence,
    Licq::TCPSocket* pSock)
{
  bool errorOccured = false;

  switch (channel)
  {
    case Licq::TCPSocket::ChannelInfo:
    {
    packet.incDataPosRead(2);
    char error_level = packet.UnpackChar();

    if (!bIsAck)
    {
      if (error_level != ICQ_PLUGIN_REQUEST)
      {
          gLog.warning("Info plugin request with unknown level %u from %s.",
              error_level, u->getAlias().c_str());
        errorOccured = true;
        break;
      }
      char GUID[GUID_LENGTH];
      for (int i = 0 ; i < GUID_LENGTH; i ++)
        packet >> GUID[i];

      if (memcmp(GUID, PLUGIN_QUERYxINFO, GUID_LENGTH) == 0)
      {
          gLog.info(tr("Info plugin list request from %s."), u->getAlias().c_str());
        if (pSock)
        {
          CPT_InfoPluginListResp p(u, nSequence);
          AckTCP(p, pSock);
        }
        else
        {
          CPU_InfoPluginListResp *p = new CPU_InfoPluginListResp(u, nMsgID1,
                                                    nMsgID2, nSequence);
          SendEvent_Server(p);
        }
      }
      else if (memcmp(GUID, PLUGIN_PHONExBOOK, GUID_LENGTH) == 0)
      {
          gLog.info(tr("Phone Book request from %s."), u->getAlias().c_str());
        if (pSock)
        {
          CPT_InfoPhoneBookResp p(u, nSequence);
          AckTCP(p, pSock);
        }
        else
        {
          CPU_InfoPhoneBookResp *p = new CPU_InfoPhoneBookResp(u, nMsgID1,
                                                       nMsgID2, nSequence);
          SendEvent_Server(p);
        }
      }
      else if (memcmp(GUID, PLUGIN_PICTURE, GUID_LENGTH) == 0)
      {
          gLog.info(tr("Picture request from %s."), u->getAlias().c_str());

        if (pSock)
        {
          CPT_InfoPictureResp p(u, nSequence);
          AckTCP(p, pSock);
        }
        else
        {
          CPU_InfoPictureResp *p = new CPU_InfoPictureResp(u, nMsgID1, nMsgID2,
                                                                nSequence);
          SendEvent_Server(p);
        }
      }
      else
      {
          gLog.warning(tr("Unknown info request from %s."), u->getAlias().c_str());
        if (pSock)
        {
            CPT_PluginError p(u, nSequence, Licq::TCPSocket::ChannelInfo);
          AckTCP(p, pSock);
        }
        else
        {
          CPU_PluginError *p = new CPU_PluginError(u, nMsgID1, nMsgID2, nSequence,
                                                   PLUGIN_INFOxMANAGER);
          SendEvent_Server(p);
        }
        errorOccured = true;
      }
    }
    else
    {
        Licq::Event::ResultType result;

      switch (error_level)
      {
      case ICQ_PLUGIN_SUCCESS:
      {
        packet.incDataPosRead(4); //Unknown
        //Time of last update
        unsigned long nTime = packet.UnpackUnsignedLong();
        if (nTime == u->ClientInfoTimestamp())
          u->SetOurClientInfoTimestamp(nTime);

        //bytes remaining in packet
        unsigned long len = packet.UnpackUnsignedLong();
        if (len < 8)
        {
          //this could be no plugins or no picture, need to check
              Licq::Event* e = pSock ? DoneEvent(pSock->Descriptor(), nSequence,
                      Licq::Event::ResultAcked) :
              DoneServerEvent(nMsgID2, Licq::Event::ResultAcked);

          if (e == NULL)
          {
                gLog.warning(tr("Ack for unknown event from %s."), u->getAlias().c_str());
                return true;
              }

          const char *GUID;
              CPacketTcp* packetTcp = dynamic_cast<CPacketTcp*>(e->m_pPacket);
          if (e->SNAC() ==
                    MAKESNAC(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SENDxSERVER) &&
              e->ExtraInfo() == ServerInfoPluginRequest)
          {
            GUID = ((CPU_InfoPluginReq *)e->m_pPacket)->RequestGUID();
              }
              else if (packetTcp != NULL && packetTcp->channel() == Licq::TCPSocket::ChannelInfo &&
                   e->ExtraInfo() == DirectInfoPluginRequest)
          {
            GUID = ((CPT_InfoPluginReq *)e->m_pPacket)->RequestGUID();
          }
          else
          {
                gLog.warning(tr("Ack for the wrong event from %s."), u->getAlias().c_str());
                delete e;
                return true;
              }

          if (memcmp(GUID, PLUGIN_PICTURE, GUID_LENGTH) == 0)
          {
                gLog.info(tr("%s has no picture."), u->getAlias().c_str());

                if (remove(u->pictureFileName().c_str()) != 0 && errno != ENOENT)
                {
                  gLog.error(tr("Unable to delete %s's picture file (%s): %s."),
                      u->getAlias().c_str(), u->pictureFileName().c_str(), strerror(errno));
                }

            u->SetEnableSave(false);
            u->SetPicturePresent(false);
            u->SetEnableSave(true);
              u->save(Licq::User::SavePictureInfo);

              Licq::gPluginManager.pushPluginSignal(new Licq::PluginSignal(
                  Licq::PluginSignal::SignalUser,
                  Licq::PluginSignal::UserPicture, u->id()));
            }
          else if (memcmp(GUID, PLUGIN_QUERYxINFO, GUID_LENGTH) == 0)
          {
                gLog.info(tr("%s has no info plugins."), u->getAlias().c_str());
              }
              else
              {
                gLog.unknown(tr("Unknown info response with no data from %s."), u->getAlias().c_str());
              }

          ProcessDoneEvent(e);
          return false;
        }
        else
        {
          unsigned long nRequest = packet.UnpackUnsignedLong(); 
          unsigned long nEntries = packet.UnpackUnsignedLong();
          switch (nRequest)
          {
          case ICQ_PLUGIN_RESP_INFOxLIST:
          case ICQ_PLUGIN_RESP_INFOxLISTx0:
          {
            for (; nEntries > 0; nEntries --)
            {
              packet.incDataPosRead(GUID_LENGTH); // GUID of plugin
              packet.incDataPosRead(4); //Unknown
              unsigned long nLen = packet.UnpackUnsignedLong();
                    string name = packet.unpackRawString(nLen);

              nLen = packet.UnpackUnsignedLong();
                    string fullName = packet.unpackRawString(nLen);

              packet.incDataPosRead(4); //Unknown (always 0?)

                    gLog.info(tr("%s has %s (%s)."), u->getAlias().c_str(), name.c_str(), fullName.c_str());
                  }
                  break;
                }

                case ICQ_PLUGIN_RESP_PHONExBOOK:
                {
                  gLog.info(tr("Phone Book reply from %s."), u->getAlias().c_str());
                  struct Licq::PhoneBookEntry* pb = new Licq::PhoneBookEntry[nEntries];
                  char* buf;
            for (unsigned long i = 0; i < nEntries; i ++)
            {
              unsigned long nLen = packet.UnpackUnsignedLong();
                    buf = new char[nLen + 1];
              for (unsigned long j = 0; j < nLen; j++)
                      packet >> buf[j];
                    buf[nLen] = '\0';
                    gTranslator.ServerToClient(buf);
                    pb[i].description = buf;
                    delete[] buf;

              nLen = packet.UnpackUnsignedLong();
                    buf = new char[nLen + 1];
              for (unsigned long j = 0; j < nLen; j++)
                      packet >> buf[j];
                    buf[nLen] = '\0';
                    gTranslator.ServerToClient(buf);
                    pb[i].areaCode = buf;
                    delete[] buf;

              nLen = packet.UnpackUnsignedLong();
                    buf = new char[nLen + 1];
              for (unsigned long j = 0; j < nLen; j++)
                      packet >> buf[j];
                    buf[nLen] = '\0';
                    gTranslator.ServerToClient(buf);
                    pb[i].phoneNumber = buf;
                    delete[] buf;

              nLen = packet.UnpackUnsignedLong();
                    buf = new char[nLen + 1];
              for (unsigned long j = 0; j < nLen; j++)
                      packet >> buf[j];
                    buf[nLen] = '\0';
                    gTranslator.ServerToClient(buf);
                    pb[i].extension = buf;
                    delete[] buf;

              nLen = packet.UnpackUnsignedLong();
                    buf = new char[nLen + 1];
              for (unsigned long j = 0; j < nLen; j++)
                      packet >> buf[j];
                    buf[nLen] = '\0';
                    gTranslator.ServerToClient(buf);
                    pb[i].country = buf;
                    delete[] buf;

              pb[i].nActive = packet.UnpackUnsignedLong();
            }
            for (unsigned long i = 0; i < nEntries; i ++)
            {
              packet.UnpackUnsignedLong(); // entry length

              pb[i].nType = packet.UnpackUnsignedLong();

              unsigned long nLen = packet.UnpackUnsignedLong();
                    buf = new char[nLen + 1];
              for (unsigned long j = 0; j < nLen; j++)
                      packet >> buf[j];
                    buf[nLen] = '\0';
                    gTranslator.ServerToClient(buf);
                    pb[i].gateway = buf;
                    delete[] buf;

              pb[i].nGatewayType = packet.UnpackUnsignedLong();
              pb[i].nSmsAvailable = packet.UnpackUnsignedLong();
              pb[i].nRemoveLeading0s = packet.UnpackUnsignedLong();
              pb[i].nPublish = packet.UnpackUnsignedLong();
            }

            u->SetEnableSave(false);
            u->GetPhoneBook()->Clean();
            for (unsigned long i = 0; i < nEntries; i++)
            {
              u->GetPhoneBook()->AddEntry(&pb[i]);
            }
            u->SetEnableSave(true);
                u->save(Licq::User::SavePhoneBook);
                  delete [] pb;

                Licq::gPluginManager.pushPluginSignal(new Licq::PluginSignal(
                    Licq::PluginSignal::SignalUser,
                    Licq::PluginSignal::UserInfo, u->id()));
                break;
              }

                case ICQ_PLUGIN_RESP_PICTURE:
                {
                  gLog.info(tr("Picture reply from %s."), u->getAlias().c_str());
            packet.incDataPosRead(nEntries); // filename, don't care
            unsigned long nLen = packet.UnpackUnsignedLong();
            if (nLen == 0)	// do not create empty .pic files
              break;

                  int nFD = open(u->pictureFileName().c_str(), O_WRONLY | O_CREAT | O_TRUNC, 00664);
            if (nFD == -1)
            {
                    gLog.error(tr("Unable to open picture file (%s): %s."),
                        u->pictureFileName().c_str(), strerror(errno));
                    break;
                  }

                  string data = packet.unpackRawString(nLen);
                  write(nFD, data.c_str(), nLen);
            close(nFD);

            u->SetEnableSave(false);
            u->SetPicturePresent(true);
            u->SetEnableSave(true);
                u->save(Licq::User::SavePictureInfo);

                Licq::gPluginManager.pushPluginSignal(new Licq::PluginSignal(
                    Licq::PluginSignal::SignalUser,
                    Licq::PluginSignal::UserPicture, u->id()));
                break;
              }
            }
          }

            result = Licq::Event::ResultAcked;
            break;
      }
      case ICQ_PLUGIN_ERROR:
      {
            gLog.warning(tr("Info plugin not available from %s."), u->getAlias().c_str());
            result = Licq::Event::ResultError;
            break;
          }
          case ICQ_PLUGIN_REJECTED:
          {
            gLog.info(tr("%s refused our request."), u->getAlias().c_str());
            result = Licq::Event::ResultFailed;
            break;
          }
          case ICQ_PLUGIN_AWAY:
          {
            gLog.info(tr("Our request was refused because %s is away."), u->getAlias().c_str());
            result = Licq::Event::ResultFailed;
            break;
          }
          default:
          {
            gLog.warning(tr("Unknown reply level %u from %s"),
                error_level, u->getAlias().c_str());
        errorOccured = true;
            result = Licq::Event::ResultError;
            break;
          }
        }

        Licq::Event* e = pSock ? DoneEvent(pSock->Descriptor(), nSequence,
                                      result) :
                            DoneServerEvent(nMsgID2, result);
      if (e == NULL)
          gLog.warning(tr("Ack for unknown event from %s."), u->getAlias().c_str());
      else
        ProcessDoneEvent(e);

    }

      break;
    }
    case Licq::TCPSocket::ChannelStatus:
    {
    packet.incDataPosRead(2);
    char error_level = packet.UnpackChar();

    if (!bIsAck)
    {
      if (error_level != ICQ_PLUGIN_REQUEST)
      {
          gLog.warning(tr("Unknown status plugin request level %u from %s."),
              error_level, u->getAlias().c_str());
        errorOccured = true;
        break;
      }

      char GUID[GUID_LENGTH];
      for (int i = 0 ; i < GUID_LENGTH; i ++)
        packet >> GUID[i];

      if (memcmp(GUID, PLUGIN_QUERYxSTATUS, GUID_LENGTH) == 0)
      {
          gLog.info(tr("Status plugin list request from %s."), u->getAlias().c_str());
        if (pSock)
        {
          CPT_StatusPluginListResp p(u, nSequence);
          AckTCP(p, pSock);
        }
        else
        {
          CPU_StatusPluginListResp *p = new CPU_StatusPluginListResp(u, nMsgID1,
                                                        nMsgID2, nSequence);
          SendEvent_Server(p);
        }
      }
      else if (memcmp(GUID, PLUGIN_FILExSERVER, GUID_LENGTH) == 0)
      {
          gLog.info(tr("File server status request from %s."), u->getAlias().c_str());
        unsigned long nStatus;
        {
          Licq::OwnerReadGuard o(LICQ_PPID);
          switch (o->sharedFilesStatus())
          {
            case IcqPluginActive: nStatus = ICQ_PLUGIN_STATUSxACTIVE; break;
            case IcqPluginBusy: nStatus = ICQ_PLUGIN_STATUSxBUSY; break;
            default: nStatus = ICQ_PLUGIN_STATUSxINACTIVE; break;
          }
        }
        if (pSock)
        {
          CPT_StatusPluginResp p(u, nSequence, nStatus);
          AckTCP(p, pSock);
        }
        else
        {
          CPU_StatusPluginResp *p = new CPU_StatusPluginResp(u, nMsgID1, nMsgID2,
                                           nSequence, nStatus);
          SendEvent_Server(p);
        }
      }
      else if (memcmp(GUID, PLUGIN_ICQxPHONE, GUID_LENGTH) == 0)
      {
          gLog.info(tr("ICQphone status request from %s."), u->getAlias().c_str());
        unsigned long nStatus;
        {
          Licq::OwnerReadGuard o(LICQ_PPID);
          switch (o->icqPhoneStatus())
          {
            case IcqPluginActive: nStatus = ICQ_PLUGIN_STATUSxACTIVE; break;
            case IcqPluginBusy: nStatus = ICQ_PLUGIN_STATUSxBUSY; break;
            default: nStatus = ICQ_PLUGIN_STATUSxINACTIVE; break;
          }
        }
        if (pSock)
        {
          CPT_StatusPluginResp p(u, nSequence, nStatus);
          AckTCP(p, pSock);
        }
        else
        {
          CPU_StatusPluginResp *p = new CPU_StatusPluginResp(u, nMsgID1, nMsgID2,
                                           nSequence, nStatus);
          SendEvent_Server(p);
        }
      }
      else if (memcmp(GUID, PLUGIN_FOLLOWxME, GUID_LENGTH) == 0)
      {
          gLog.info(tr("Phone \"Follow Me\" status request from %s."), u->getAlias().c_str());
        unsigned long nStatus;
        {
          Licq::OwnerReadGuard o(LICQ_PPID);
          switch (o->phoneFollowMeStatus())
          {
            case IcqPluginActive: nStatus = ICQ_PLUGIN_STATUSxACTIVE; break;
            case IcqPluginBusy: nStatus = ICQ_PLUGIN_STATUSxBUSY; break;
            default: nStatus = ICQ_PLUGIN_STATUSxINACTIVE; break;
          }
        }
        if (pSock)
        {
          CPT_StatusPluginResp p(u, nSequence, nStatus);
          AckTCP(p, pSock);
        }
        else
        {
          CPU_StatusPluginResp *p = new CPU_StatusPluginResp(u, nMsgID1, nMsgID2,
                                           nSequence, nStatus);
          SendEvent_Server(p);
        }
      }
      else
      {
          gLog.warning(tr("Unknown status request from %s."), u->getAlias().c_str());
        if (pSock)
        {
            CPT_PluginError p(u, nSequence, Licq::TCPSocket::ChannelStatus);
          AckTCP(p, pSock);
        }
        else
        {
          CPU_PluginError *p = new CPU_PluginError(u, nMsgID1, nMsgID2, nSequence,
                                                   PLUGIN_STATUSxMANAGER);
          SendEvent_Server(p);
        }
        errorOccured = true;
      }
    }
    else
    {
        Licq::Event::ResultType result;

      switch (error_level)
      {
      case ICQ_PLUGIN_SUCCESS:
      {
        packet.incDataPosRead(13); //Unknown
        //Time of last installation of new status plugins
        unsigned long nTime = packet.UnpackUnsignedLong();
        if (nTime == u->ClientStatusTimestamp())
          u->SetOurClientStatusTimestamp(nTime);

        //bytes remaining in packet
        unsigned long len = packet.UnpackUnsignedLong(); 
        if (len < 8)
        {
              gLog.info(tr("%s has no status plugins.\n"), u->getAlias().c_str());
            }
            else
            {
          packet.incDataPosRead(4); // Unknown
          unsigned long nEntries = packet.UnpackUnsignedLong();
          for (; nEntries > 0; nEntries --)
          {
            packet.incDataPosRead(GUID_LENGTH); // GUID of plugin
            packet.incDataPosRead(4); //Unknown
            unsigned long nLen = packet.UnpackUnsignedLong();
                string name = packet.unpackRawString(nLen);

            nLen = packet.UnpackUnsignedLong();
                string fullName = packet.unpackRawString(nLen);

            packet.incDataPosRead(4); //Unknown (always 0?)

                gLog.info(tr("%s has %s (%s)."), u->getAlias().c_str(), name.c_str(), fullName.c_str());
              }
            }

            result = Licq::Event::ResultAcked;
            break;
      }
      case ICQ_PLUGIN_STATUSxREPLY:
      {
            Licq::Event* e = pSock ? DoneEvent(pSock->Descriptor(), nSequence,
                    Licq::Event::ResultAcked) :
                DoneServerEvent(nMsgID2, Licq::Event::ResultAcked);

        if (e == NULL)
        {
              gLog.warning(tr("Ack for unknown event from %s."), u->getAlias().c_str());
              return true;
            }

        const char *GUID;
            CPacketTcp* packetTcp = dynamic_cast<CPacketTcp*>(e->m_pPacket);
        if (e->SNAC() ==
                  MAKESNAC(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SENDxSERVER) &&
            e->ExtraInfo() == ServerStatusPluginRequest)
        {
          GUID = ((CPU_StatusPluginReq *)e->m_pPacket)->RequestGUID();
            }
            else if (packetTcp != NULL && packetTcp->channel() == Licq::TCPSocket::ChannelInfo &&
                 e->ExtraInfo() == DirectStatusPluginRequest)
        {
          GUID = ((CPT_StatusPluginReq *)e->m_pPacket)->RequestGUID();
        }
        else
        {
              gLog.warning(tr("Ack for the wrong event from %s."), u->getAlias().c_str());
              delete e;
              return true;
            }

        packet.incDataPosRead(4); // Unknown
        unsigned long nState = packet.UnpackUnsignedLong();
        unsigned long nTime = packet.UnpackUnsignedLong();

        if (nTime == u->ClientStatusTimestamp())
          u->SetOurClientStatusTimestamp(nTime);

        const char* szState;
            unsigned pluginState;
        switch (nState)
        {
              case ICQ_PLUGIN_STATUSxINACTIVE:
                szState = "inactive";
                pluginState = IcqPluginInactive;
                break;
              case ICQ_PLUGIN_STATUSxACTIVE:
                szState = "active";
                pluginState = IcqPluginActive;
                break;
              case ICQ_PLUGIN_STATUSxBUSY:
                szState = "busy";
                pluginState = IcqPluginBusy;
                break;
              default:
                szState = "unknown";
                pluginState = IcqPluginInactive;
                break;
            }

        if (memcmp(GUID, PLUGIN_FILExSERVER, GUID_LENGTH) == 0)
        {
              gLog.info(tr("%s's Shared Files Directory is %s."), u->getAlias().c_str(), szState);
              u->setSharedFilesStatus(pluginState);
            }
        else if (memcmp(GUID, PLUGIN_FOLLOWxME, GUID_LENGTH) == 0)
        {
              gLog.info(tr("%s's Phone \"Follow Me\" is %s."), u->getAlias().c_str(), szState);
              u->setPhoneFollowMeStatus(pluginState);
            }
        else if (memcmp(GUID, PLUGIN_ICQxPHONE, GUID_LENGTH) == 0)
        {
              gLog.info(tr("%s's ICQphone is %s."), u->getAlias().c_str(), szState);
              u->setIcqPhoneStatus(pluginState);
            }

        // Which plugin?
            Licq::gPluginManager.pushPluginSignal(new Licq::PluginSignal(
                Licq::PluginSignal::SignalUser,
                Licq::PluginSignal::UserPluginStatus, u->id(), 0));

        ProcessDoneEvent(e);
        return false;
      }
      case ICQ_PLUGIN_ERROR:
      {
            gLog.warning(tr("Status plugin not available from %s."), u->getAlias().c_str());
            result = Licq::Event::ResultError;
            break;
          }
          case ICQ_PLUGIN_REJECTED:
          {
            gLog.info(tr("%s refused our request."), u->getAlias().c_str());
            result = Licq::Event::ResultFailed;
            break;
          }
          case ICQ_PLUGIN_AWAY:
          {
            gLog.info(tr("Our request was refused because %s is away."), u->getAlias().c_str());
            result = Licq::Event::ResultFailed;
            break;
          }
          default:
          {
            gLog.warning(tr("Unknown reply level %u from %s"),
                error_level, u->getAlias().c_str());
        errorOccured = true;
            result = Licq::Event::ResultError;
            break;
          }
        }

        Licq::Event* e = pSock ? DoneEvent(pSock->Descriptor(), nSequence,
                                      result) :
                            DoneServerEvent(nMsgID2, result);
      if (e == NULL)
          gLog.warning(tr("Ack for unknown event from %s."), u->getAlias().c_str());
      else
        ProcessDoneEvent(e);

    }

    break;
  }
    default:
    {
      gLog.warning(tr("Unknown channel %u from %s"), channel, u->getAlias().c_str());
    if (!pSock)
    {
      CPU_NoManager *p = new CPU_NoManager(u, nMsgID1, nMsgID2);
      SendEvent_Server(p);
    }

    errorOccured = true;
    break;
  }
  }
  return errorOccured;
}

//-----CICQDaemon::AckTCP--------------------------------------------------------------
void IcqProtocol::AckTCP(CPacketTcp &p, int nSd)
{
#if ICQ_VERSION_TCP == 3
  Licq::TCPSocket* s = dynamic_cast<Licq::TCPSocket*>(gSocketManager.FetchSocket(nSD));
  if (s != NULL)
  {
    s->Send(p.getBuffer());
    gSocketManager.DropSocket(s);
  }
#else
  SendEvent(nSd, p, false);
#endif
}

void IcqProtocol::AckTCP(CPacketTcp &p, Licq::TCPSocket* tcp)
{
#if ICQ_VERSION_TCP == 3
  tcp->Send(p.getBuffer());
#else
  SendEvent(tcp, p, false);
#endif
}


bool IcqProtocol::Handshake_Recv(Licq::TCPSocket* s, unsigned short nPort, bool bConfirm, bool bChat)
{
  char cHandshake;
  unsigned short nVersionMajor, nVersionMinor;
  CBuffer &b = s->RecvBuffer();
  b >> cHandshake >> nVersionMajor >> nVersionMinor;

  unsigned long nUin = 0;
  unsigned short nVersion = 0;
  char id[16];
  Licq::UserId userId;

  switch (VersionToUse(nVersionMajor))
  {
    case 8:
    case 7:
    {
      b.Reset();
      CPacketTcp_Handshake_v7 p_in(&b);
      nUin = p_in.SourceUin();
      snprintf(id, 16, "%lu", nUin);
      userId = Licq::UserId(id, LICQ_PPID);

      unsigned long nCookie;
      {
        Licq::UserReadGuard u(userId);
        if (!u.isLocked() && !bChat)
        {
          gLog.warning(tr("Connection from unknown user."));
          return false;
        }
        nCookie = u.isLocked() ? u->Cookie() : 0;
      }

      if (nCookie != p_in.SessionId())
      {
        gLog.warning(tr("Spoofed connection from %s as uin %s."),
            s->getRemoteIpString().c_str(), userId.toString().c_str());
        return false;
      }

      // Send the ack
      CPacketTcp_Handshake_Ack p_ack;
      if (!s->SendPacket(p_ack.getBuffer())) goto sock_error;

      // Send the handshake
      CPacketTcp_Handshake_v7 p_out(nUin, p_in.SessionId(), nPort);
      if (!s->SendPacket(p_out.getBuffer())) goto sock_error;

      // Wait for the ack (this is very bad form...blocking recv here)
      s->ClearRecvBuffer();
      do
      {
        if (!s->RecvPacket()) goto sock_error;
      } while (!s->RecvBufferFull());
      
      if (s->RecvBuffer().getDataSize() != 4)
      {
        gLog.warning(tr("Handshake ack not the right size."));
        return false;
      }

      unsigned long nOk = s->RecvBuffer().UnpackUnsignedLong();
      if (nOk != 1)
      {
        gLog.warning(tr("Bad handshake ack: %ld."), nOk);
        return false;
      }

      if (bConfirm)
      {
        if (p_in.Id() == 0)
        {
          if (!Handshake_RecvConfirm_v7(s))
          goto sock_error;
        }
        else
        {
          pthread_mutex_lock(&mutex_reverseconnect);
          std::list<CReverseConnectToUserData *>::iterator iter;
          for (iter = m_lReverseConnect.begin(); ; ++iter)
          {
            if (iter == m_lReverseConnect.end())
            {
              gLog.warning(tr("Reverse connection with unknown id (%lu)"), p_in.Id());
              pthread_mutex_unlock(&mutex_reverseconnect);
              return false;
            }
            if ((*iter)->nId == p_in.Id() && (*iter)->myIdString == id)
            {
              s->setChannel((*iter)->nData);
              (*iter)->bSuccess = true;
              (*iter)->bFinished = true;
              if (!Handshake_SendConfirm_v7(s))
              {
                pthread_mutex_unlock(&mutex_reverseconnect);
                return false;
              }
              break;
            }
          }
          pthread_mutex_unlock(&mutex_reverseconnect);
        }
      }

      nVersion = VersionToUse(nVersionMajor);

      break;
    }

    case 6:
    {
      b.Reset();
      CPacketTcp_Handshake_v6 p_in(&b);
      nUin = p_in.SourceUin();
      snprintf(id, 16, "%lu", nUin);
      userId = Licq::UserId(id, LICQ_PPID);

      unsigned long nCookie;
      {
        Licq::UserReadGuard u(userId);
        if (!u.isLocked())
        {
          gLog.warning(tr("Connection from unknown user."));
          return false;
        }
        nCookie = u->Cookie();
      }

      if (nCookie != p_in.SessionId())
      {
        gLog.warning(tr("Spoofed connection from %s as uin %s."),
            s->getRemoteIpString().c_str(), userId.toString().c_str());
        return false;
      }

      // Send the ack
      CPacketTcp_Handshake_Ack p_ack;
      if (!s->SendPacket(p_ack.getBuffer())) goto sock_error;

      // Send the handshake
      CPacketTcp_Handshake_v6 p_out(nUin, p_in.SessionId(), nPort);
      if (!s->SendPacket(p_out.getBuffer())) goto sock_error;

      // Wait for the ack (this is very bad form...blocking recv here)
      s->ClearRecvBuffer();
      do
      {
        if (!s->RecvPacket()) goto sock_error;
      } while (!s->RecvBufferFull());
      unsigned long nOk = s->RecvBuffer().UnpackUnsignedLong();
      s->ClearRecvBuffer();
      if (nOk != 1)
      {
        gLog.warning(tr("Bad handshake ack: %ld."), nOk);
        return false;
      }
      nVersion = 6;
     
      pthread_mutex_lock(&mutex_reverseconnect);
      std::list<CReverseConnectToUserData *>::iterator iter;
      for (iter = m_lReverseConnect.begin(); iter != m_lReverseConnect.end();
        ++iter)
      {
        // For v6 there is no connection id, so just use uin
        if ((*iter)->myIdString == id)
        {
          (*iter)->bSuccess = true;
          (*iter)->bFinished = true;
          break;
        }
      }
      pthread_mutex_unlock(&mutex_reverseconnect);

      break;
    }

    case 5:
    case 4:
    {
      b.UnpackUnsignedLong(); // port number
      nUin = b.UnpackUnsignedLong();
      snprintf(id, 16, "%lu", nUin);
      userId = Licq::UserId(id, LICQ_PPID);
      nVersion = VersionToUse(nVersionMajor);

      unsigned long nIntIp;
      unsigned long nIp;
      {
        Licq::UserReadGuard u(userId);
        if (!u.isLocked())
        {
          gLog.warning(tr("Connection from unknown user."));
          return false;
        }
        nIntIp = u->IntIp();
        nIp = u->Ip();
      }
      /* This might prevent connections from clients behind assymetric
         connections (i.e. direct to ICQ server and through socks to clients)
         but they should be using v6+ anyway */
      if (nIntIp != s->getRemoteIpInt() && nIp != s->getRemoteIpInt())
      {
        gLog.warning(tr("Connection from %s as %s possible spoof."),
            s->getRemoteIpString().c_str(), userId.toString().c_str());
        return false;
      }

      pthread_mutex_lock(&mutex_reverseconnect);
      std::list<CReverseConnectToUserData *>::iterator iter;
      for (iter = m_lReverseConnect.begin(); iter != m_lReverseConnect.end();
        ++iter)
      {
        // For v4 there is no connection id, so just use uin
        if ((*iter)->myIdString == id)
        {
          (*iter)->bSuccess = true;
          (*iter)->bFinished = true;
          break;
        }
      }
      pthread_mutex_unlock(&mutex_reverseconnect);

      break;
    }

    case 3:
    case 2:
    case 1:
    {
      b.UnpackUnsignedLong(); // port number
      nUin = b.UnpackUnsignedLong();
      snprintf(id, 16, "%lu", nUin);
      userId = Licq::UserId(id, LICQ_PPID);
      nVersion = 2;

      unsigned long nIntIp;
      unsigned long nIp;
      {
        Licq::UserReadGuard u(userId);
        if (!u.isLocked())
        {
          gLog.warning(tr("Connection from unknown user."));
          return false;
        }
        nIntIp = u->IntIp();
        nIp = u->Ip();
      }

      /* This might prevent connections from clients behind assymetric
         connections (i.e. direct to ICQ server and through socks to clients)
         but they should be using v6+ anyway */
      if (nIntIp != s->getRemoteIpInt() && nIp != s->getRemoteIpInt())
      {
        gLog.warning(tr("Connection from %s as %s possible spoof."),
            s->getRemoteIpString().c_str(), userId.toString().c_str());
        return false;
      }

      pthread_mutex_lock(&mutex_reverseconnect);
      std::list<CReverseConnectToUserData *>::iterator iter;
      for (iter = m_lReverseConnect.begin(); iter != m_lReverseConnect.end();
        ++iter)
      {
        // For v2 there is no connection id, so just use uin
        if ((*iter)->myIdString == id)
        {
          (*iter)->bSuccess = true;
          (*iter)->bFinished = true;
          break;
        }
      }
      pthread_mutex_unlock(&mutex_reverseconnect);

      break;
    }

    default:
      b.log(Log::Unknown, "Unknown TCP handshake packet");
      return false;
  }

  s->setUserId(userId);
  s->SetVersion(nVersion);

  return true;

sock_error:
  if (s->Error() == 0)
    gLog.warning(tr("Handshake error, remote side closed connection."));
  else
    gLog.warning(tr("Handshake socket error: %s."), s->errorStr().c_str());
  return false;
}

bool IcqProtocol::Handshake_SendConfirm_v7(Licq::TCPSocket* s)
{
  // Send handshake accepted
  CPacketTcp_Handshake_Confirm p_confirm(s->channel(), 0);
  if (!s->SendPacket(p_confirm.getBuffer()))
    return false;

  // Wait for reverse handshake accepted
  s->ClearRecvBuffer();
  do
  {
    if (!s->RecvPacket())
      return false;
  } while (!s->RecvBufferFull());
  s->ClearRecvBuffer();

  return true;
}

bool IcqProtocol::Handshake_RecvConfirm_v7(Licq::TCPSocket* s)
{
  // Get handshake confirmation
  s->ClearRecvBuffer();
  do
  {
    if (!s->RecvPacket()) goto sock_error;
  } while (!s->RecvBufferFull());

  { // damn scoping
    CBuffer &b = s->RecvBuffer();
    if (b.getDataSize() != 33)
    {
      gLog.warning(tr("Handshake confirm not the right size."));
      return false;
    }
    unsigned char c = b.UnpackChar();
    unsigned long l = b.UnpackUnsignedLong();
    if (c != 0x03 || l != 0x0000000A)
    {
      gLog.warning(tr("Unknown handshake response %2X,%8lX."), c, l);
      return false;
    }
    b.Reset();
    CPacketTcp_Handshake_Confirm p_confirm_in(&b);
    if (p_confirm_in.channel() != Licq::TCPSocket::ChannelUnknown)
      s->setChannel(p_confirm_in.channel());
    else
    {
      gLog.warning(tr("Unknown channel in ack packet."));
      return false;
    }

    s->ClearRecvBuffer();

    CPacketTcp_Handshake_Confirm p_confirm_out(p_confirm_in.channel(),
                                                       p_confirm_in.Id());

    if (s->SendPacket(p_confirm_out.getBuffer()))
      return true;
  }
 
 sock_error:
  if (s->Error() == 0)
    gLog.warning(tr("Handshake error, remote side closed connection."));
  else
    gLog.warning(tr("Handshake socket error: %s."), s->errorStr().c_str());
  return false;
}

/*------------------------------------------------------------------------------
 * ProcessTcpHandshake
 *
 * Takes the first buffer from a socket and parses it as a icq handshake.
 * Does not check that the given user already has a socket or not.
 *----------------------------------------------------------------------------*/
bool IcqProtocol::ProcessTcpHandshake(Licq::TCPSocket* s)
{
  if (!Handshake_Recv(s, 0)) return false;
  Licq::UserId userId = s->userId();
  if (!userId.isValid())
    return false;

  Licq::UserWriteGuard u(userId);
  if (u.isLocked())
  {
    gLog.info(tr("Connection from %s (%s) [v%ld]."),
        u->getAlias().c_str(), userId.toString().c_str(), s->Version());
    if (u->socketDesc(s->channel()) != s->Descriptor())
    {
      if (u->socketDesc(s->channel()) != -1)
      {
        gLog.warning(tr("User %s (%s) already has an associated socket."),
            u->getAlias().c_str(), userId.toString().c_str());
        return true;
/*        gSocketManager.CloseSocket(u->socketDesc(s->channel()), false);
        u->clearSocketDesc(s->channel());*/
      }
      u->setSocketDesc(s);
    }
  }
  else
  {
    gLog.info(tr("Connection from new user (%s) [v%ld]."),
        userId.toString().c_str(), s->Version());
  }

  //awaken waiting threads, maybe unnecessarily, but doesn't hurt
  pthread_cond_broadcast(&cond_reverseconnect_done);

  return true;
}

void IcqProtocol::StupidChatLinkageFix()
{
  CChatManager* chatman = new CChatManager(0);
  delete chatman;
  CFileTransferManager* ftman = new CFileTransferManager(Licq::UserId());
  delete ftman;
}


