// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998-2010 Licq developers
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
#include <licq/icqchat.h>
#include <licq/icqfiletransfer.h>
#include <licq/oneventmanager.h>
#include <licq/pluginsignal.h>
#include <licq/socket.h>
#include <licq/statistics.h>
#include <licq/translator.h>
#include <licq/userevents.h>
#include <licq/logging/log.h>
#include <licq/version.h>

#include "../daemon.h"
#include "../gettext.h"
#include "../support.h"
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
   bool viaServer, unsigned short nLevel, bool bMultipleRecipients,
   const Licq::Color* pColor)
{
  const string accountId = userId.accountId();
  const char* m = message.c_str();

  char *mDos = NULL;
  char *szMessage = NULL;
  bool bUTF16 = false;
  bool bUserOffline = true;
  if (m != NULL)
  {
    string m2 = gTranslator.clientToServer(message, true);
    mDos = new char[m2.size()+1];
    strncpy(mDos, m2.c_str(), m2.size());
    mDos[m2.size()] = '\0';
  }
  Licq::EventMsg* e = NULL;

  unsigned long f = Licq::UserEvent::FlagLicqVerMask;

  char *cipher = NULL;
  bool useGpg = false;
  {
    {
      Licq::UserReadGuard u(userId);
      if (u.isLocked())
      {
        bUserOffline = !u->isOnline();
        useGpg = u->UseGPG();
      }
    }
    if (useGpg && !bUserOffline)
      cipher = Licq::gGpgHelper.Encrypt(mDos, userId);
  }

  if (cipher)
    f |= Licq::UserEvent::FlagEncrypted;
  if (!viaServer)
    f |= Licq::UserEvent::FlagDirect;
  if (nLevel == ICQ_TCPxMSG_URGENT)
    f |= Licq::UserEvent::FlagUrgent;
  if (bMultipleRecipients)
    f |= Licq::UserEvent::FlagMultiRec;

  // What kinda encoding do we have here?
  unsigned short nCharset = CHARSET_ASCII;
  size_t nUTFLen = 0;
  string fromEncoding;

  szMessage =  cipher ? cipher : mDos;

  if (viaServer)
  {
    if (!bUserOffline && cipher == 0)
    {
      if (!gTranslator.isAscii(mDos))
      {
        Licq::UserReadGuard u(userId);
        if (u.isLocked() && !u->userEncoding().empty())
          fromEncoding = u->userEncoding();

        if (u.isLocked() && isdigit(u->accountId()[0]))
        {
          // ICQ Users can send a flag that says UTF8/16 is ok
          if (u->SupportsUTF8())
            nCharset = CHARSET_UNICODE;
          else if (!u->userEncoding().empty())
            nCharset = CHARSET_CUSTOM;
        }
        else if (u.isLocked() && !(isdigit(u->accountId()[0])))
        {
          // AIM users support UTF8/16
          nCharset = CHARSET_UNICODE;
        }
      }

      if (nCharset == CHARSET_UNICODE)
      {
        bUTF16 = true;
        if (fromEncoding.empty())
          fromEncoding = nl_langinfo(CODESET);
        string msgUtf16 = gTranslator.toUtf16(mDos, fromEncoding);
        nUTFLen = msgUtf16.size();
        szMessage = new char[msgUtf16.size()+1];
        strncpy(szMessage, msgUtf16.c_str(), msgUtf16.size());
        szMessage[msgUtf16.size()] = '\0';
      }
    }

     e = new Licq::EventMsg(m, ICQ_CMDxSND_THRUxSERVER, Licq::EventMsg::TimeNow, f);
     unsigned short nMaxSize = bUserOffline ? MaxOfflineMessageSize : MaxMessageSize;
     if (strlen(szMessage) > nMaxSize)
     {
       gLog.warning(tr("%sTruncating message to %d characters to send through server.\n"),
                 L_WARNxSTR, nMaxSize);
       szMessage[nMaxSize] = '\0';
     }
     icqSendThroughServer(eventId, userId, ICQ_CMDxSUB_MSG | (bMultipleRecipients ? ICQ_CMDxSUB_FxMULTIREC : 0),
                          cipher ? cipher : szMessage, e, nCharset, nUTFLen);
  }

  Licq::UserWriteGuard u(userId);

  if (!viaServer)
  {
    if (!u.isLocked())
      return;
    if (u->Secure())
      f |= Licq::UserEvent::FlagEncrypted;
    e = new Licq::EventMsg(m, ICQ_CMDxTCP_START, Licq::EventMsg::TimeNow, f);
    if (pColor != NULL) e->SetColor(pColor);
    string message;
    if (cipher != NULL)
      message = cipher;
    else
      message.assign(szMessage, nUTFLen);
    CPT_Message* p = new CPT_Message(message, nLevel, bMultipleRecipients, pColor, *u);
    gLog.info(tr("%sSending %smessage to %s (#%hu).\n"), L_TCPxSTR,
       nLevel == ICQ_TCPxMSG_URGENT ? tr("urgent ") : "",
       u->GetAlias(), -p->Sequence());
    SendExpectEvent_Client(eventId, *u, p, e);
  }

  if (u.isLocked())
  {
    u->SetSendServer(viaServer);
    u->SetSendLevel(nLevel);
  }

  if (pColor != NULL)
    Licq::Color::setDefaultColors(pColor);

  if (bUTF16 && szMessage)
    delete [] szMessage;
  if (cipher)
    free(cipher);
  if (mDos)
    delete [] mDos;
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
    gLog.info(tr("%sRequesting auto response from %s.\n"), L_SRVxSTR,
              u->GetAlias());
    SendExpectEvent_Server(eventId, userId, s, NULL);
  }
  else
  {
    CPT_ReadAwayMessage *p = new CPT_ReadAwayMessage(*u);
    gLog.info(tr("%sRequesting auto response from %s (#%hu).\n"), L_TCPxSTR,
	      u->GetAlias(), -p->Sequence());
    SendExpectEvent_Client(eventId, *u, p, NULL);
  }

  return eventId;
}

void IcqProtocol::icqSendUrl(unsigned long eventId, const Licq::UserId& userId, const string& url,
   const string& message, bool viaServer, unsigned short nLevel,
   bool bMultipleRecipients, const Licq::Color* pColor)
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
  if (viaServer && n > MaxMessageSize)
    m.erase(MaxMessageSize - url.size() - 2);
  m += '\xFE';
  m += url;

  unsigned long f = Licq::UserEvent::FlagLicqVerMask;
  if (!viaServer)
    f |= Licq::UserEvent::FlagDirect;
  if (nLevel == ICQ_TCPxMSG_URGENT)
    f |= Licq::UserEvent::FlagUrgent;
  if (bMultipleRecipients)
    f |= Licq::UserEvent::FlagMultiRec;

  if (viaServer)
  {
    unsigned short nCharset = 0;
    {
      Licq::UserReadGuard u(userId);
      if (u.isLocked() && !u->userEncoding().empty())
        nCharset = 3;
    }

    e = new Licq::EventUrl(url.c_str(), description, ICQ_CMDxSND_THRUxSERVER,
        Licq::EventUrl::TimeNow, f);
    icqSendThroughServer(eventId, userId, ICQ_CMDxSUB_URL | (bMultipleRecipients ? ICQ_CMDxSUB_FxMULTIREC : 0), m, e, nCharset);
  }

  Licq::UserWriteGuard u(userId);

  if (!viaServer)
  {
    if (!u.isLocked())
      return;
    if (u->Secure())
      f |= Licq::UserEvent::FlagEncrypted;
    e = new Licq::EventUrl(url.c_str(), description, ICQ_CMDxTCP_START, Licq::EventUrl::TimeNow, f);
    if (pColor != NULL) e->SetColor(pColor);
    CPT_Url* p = new CPT_Url(m, nLevel, bMultipleRecipients, pColor, *u);
    gLog.info(tr("%sSending %sURL to %s (#%hu).\n"), L_TCPxSTR,
       nLevel == ICQ_TCPxMSG_URGENT ? tr("urgent ") : "",
       u->GetAlias(), -p->Sequence());
    SendExpectEvent_Client(eventId, *u, p, e);
  }
  if (u.isLocked())
  {
    u->SetSendServer(viaServer);
    u->SetSendLevel(nLevel);
  }

  if (pColor != NULL)
    Licq::Color::setDefaultColors(pColor);

  if (szDescDos)
    delete [] szDescDos;
}

void IcqProtocol::icqFileTransfer(unsigned long eventId, const Licq::UserId& userId, const string& filename,
    const string& message, const list<string>& lFileList, unsigned short nLevel, bool bServer)
{
  if (Licq::gUserManager.isOwner(userId))
    return;

  string dosDesc = gTranslator.clientToServer(message, true);
  Licq::EventFile* e = NULL;

  Licq::UserWriteGuard u(userId);
  if (!u.isLocked())
    return;

  if (bServer)
  {
    unsigned long f = LICQ_VERSION;
    //flags through server are a little different
    if (nLevel == ICQ_TCPxMSG_NORMAL)
      nLevel = ICQ_TCPxMSG_NORMAL2;
    else if (nLevel == ICQ_TCPxMSG_URGENT)
    {
      f |= Licq::UserEvent::FlagUrgent;
      nLevel = ICQ_TCPxMSG_URGENT2;
    }
    else if (nLevel == ICQ_TCPxMSG_LIST)
      nLevel = ICQ_TCPxMSG_LIST2;

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
      gLog.info(tr("%sSending file transfer to %s (#%hu).\n"), L_SRVxSTR, 
                u->GetAlias(), -p->Sequence());

      SendExpectEvent_Server(userId, p, e);
    }
  }
  else
  {
    CPT_FileTransfer* p = new CPT_FileTransfer(lFileList, filename, dosDesc, nLevel, *u);

    if (!p->IsValid())
    {
      delete p;
    }
    else
    {
      unsigned long f = Licq::UserEvent::FlagDirect | Licq::UserEvent::FlagLicqVerMask;
      if (nLevel == ICQ_TCPxMSG_URGENT)
        f |= Licq::UserEvent::FlagUrgent;
      if (u->Secure())
        f |= Licq::UserEvent::FlagEncrypted;

      e = new Licq::EventFile(filename, p->description(), p->GetFileSize(),
          lFileList, p->Sequence(), Licq::EventFile::TimeNow, f);
      gLog.info(tr("%sSending %sfile transfer to %s (#%hu).\n"), L_TCPxSTR,
                nLevel == ICQ_TCPxMSG_URGENT ? tr("urgent ") : "", 
                u->GetAlias(), -p->Sequence());

      SendExpectEvent_Client(eventId, *u, p, e);
    }
  }

  u->SetSendServer(bServer);
  u->SetSendLevel(nLevel);
}

//-----CICQDaemon::sendContactList-------------------------------------------
unsigned long IcqProtocol::icqSendContactList(const Licq::UserId& userId,
   const StringList& users, bool online, unsigned short nLevel,
   bool bMultipleRecipients, const Licq::Color* pColor)
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

  if (!online && p > MaxMessageSize)
  {
    gLog.warning(tr("%sContact list too large to send through server.\n"), L_WARNxSTR);
    delete []m;
    return 0;
  }

  Licq::EventContactList* e = NULL;

  unsigned long f = Licq::UserEvent::FlagLicqVerMask;
  if (online)
    f |= Licq::UserEvent::FlagDirect;
  if (nLevel == ICQ_TCPxMSG_URGENT)
    f |= Licq::UserEvent::FlagUrgent;
  if (bMultipleRecipients)
    f |= Licq::UserEvent::FlagMultiRec;

  if (!online) // send offline
  {
    e = new Licq::EventContactList(vc, false, ICQ_CMDxSND_THRUxSERVER, Licq::EventContactList::TimeNow, f);
    icqSendThroughServer(eventId, userId,
      ICQ_CMDxSUB_CONTACTxLIST | (bMultipleRecipients ? ICQ_CMDxSUB_FxMULTIREC : 0),
      m, e);
  }

  Licq::UserWriteGuard u(userId);
  if (online)
  {
    if (!u.isLocked())
      return 0;
    if (u->Secure())
      f |= Licq::UserEvent::FlagEncrypted;
    e = new Licq::EventContactList(vc, false, ICQ_CMDxTCP_START, Licq::EventContactList::TimeNow, f);
    if (pColor != NULL) e->SetColor(pColor);
    CPT_ContactList *p = new CPT_ContactList(m, nLevel, bMultipleRecipients, pColor, *u);
    gLog.info(tr("%sSending %scontact list to %s (#%hu).\n"), L_TCPxSTR,
       nLevel == ICQ_TCPxMSG_URGENT ? tr("urgent ") : "",
       u->GetAlias(), -p->Sequence());
    SendExpectEvent_Client(eventId, *u, p, e);
  }
  if (u.isLocked())
  {
    u->SetSendServer(!online);
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
    gLog.info("%sRequesting info plugin list from %s through server.\n",
              L_SRVxSTR, u->GetAlias());
  else
    gLog.info("%sRequesting info plugin list from %s.\n", L_TCPxSTR,
              u->GetAlias());

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
    gLog.info("%sRequesting Phone Book from %s through server.\n",
              L_SRVxSTR, u->GetAlias());
  else
    gLog.info("%sRequesting Phone Book from %s.\n", L_TCPxSTR, u->GetAlias());

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
    gLog.info("%sRequesting Picture from %s through server.\n",
              L_SRVxSTR, u->GetAlias());
  else
    gLog.info("%sRequesting Picture from %s.\n", L_TCPxSTR, u->GetAlias());

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
    gLog.info("%sRequesting status plugin list from %s through server.\n",
              L_SRVxSTR, u->GetAlias());
  else
    gLog.info("%sRequesting status plugin list from %s.\n", L_TCPxSTR,
              u->GetAlias());

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
    gLog.info("%sRequesting file server status from %s through server.\n",
              L_SRVxSTR, u->GetAlias());
  else
    gLog.info("%sRequesting file server status from %s.\n", L_TCPxSTR,
              u->GetAlias());

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
    gLog.info("%sRequesting Phone \"Follow Me\" status from %s through"
              " server.\n", L_SRVxSTR, u->GetAlias());
  else
    gLog.info("%sRequesting Phone \"Follow Me\" status from %s.\n", L_TCPxSTR,
              u->GetAlias());

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
    gLog.info("%sRequesting ICQphone status from %s through server.\n",
              L_SRVxSTR, u->GetAlias());
  else
    gLog.info("%sRequesting ICQphone status from %s.\n", L_TCPxSTR,
              u->GetAlias());

  unsigned long result = icqRequestStatusPlugin(*u, bServer, PLUGIN_FILExSERVER);

  return result;
}

void IcqProtocol::icqFileTransferCancel(const Licq::UserId& userId, unsigned short nSequence)
{
  // add to history ??
  Licq::UserWriteGuard u(userId);
  if (!u.isLocked())
    return;
  gLog.info(tr("%sCancelling file transfer to %s (#%hu).\n"), L_TCPxSTR, 
            u->GetAlias(), -nSequence);
  CPT_CancelFile p(nSequence, *u);
  AckTCP(p, u->SocketDesc(ICQ_CHNxNONE));
}

void IcqProtocol::icqFileTransferAccept(const Licq::UserId& userId, unsigned short nPort,
    unsigned short nSequence, const unsigned long nMsgID[2], bool viaServer,
    const string& message, const string& filename, unsigned long nFileSize)
{
   // basically a fancy tcp ack packet which is sent late
  Licq::UserWriteGuard u(userId);
  if (!u.isLocked())
    return;
  gLog.info(tr("%sAccepting file transfer from %s (#%hu).\n"),
      viaServer ? L_SRVxSTR : L_TCPxSTR, u->GetAlias(), -nSequence);
  if (!viaServer)
  {
    CPT_AckFileAccept p(nPort, nSequence, *u);
    AckTCP(p, u->SocketDesc(ICQ_CHNxNONE));
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
  gLog.info(tr("%sRefusing file transfer from %s (#%hu).\n"), 
      viaServer ? L_SRVxSTR : L_TCPxSTR, u->GetAlias(), -nSequence);

  if (!viaServer)
  {
    CPT_AckFileRefuse p(reasonDos, nSequence, *u);
    AckTCP(p, u->SocketDesc(ICQ_CHNxNONE));
  }
  else
  {
    CPU_AckFileRefuse *p = new CPU_AckFileRefuse(*u, nMsgID, nSequence,
        reasonDos);
    SendEvent_Server(p);
  }
}

unsigned long IcqProtocol::icqChatRequest(const Licq::UserId& userId, const string& reason,
                                         unsigned short nLevel, bool bServer)
{
  return icqMultiPartyChatRequest(userId, reason, "", 0, nLevel, bServer);
}

unsigned long IcqProtocol::icqMultiPartyChatRequest(const Licq::UserId& userId,
   const string& reason, const string& chatUsers, unsigned short nPort,
   unsigned short nLevel, bool bServer)
{
  if (Licq::gUserManager.isOwner(userId))
    return 0;

  Licq::UserWriteGuard u(userId);
  if (!u.isLocked())
    return 0;
  string reasonDos = gTranslator.clientToServer(reason, true);

  unsigned long f;
  Licq::Event* result = NULL;
  if (bServer)
  {
    f = Licq::UserEvent::FlagLicqVerMask;

    //flags through server are a little different
    if (nLevel == ICQ_TCPxMSG_NORMAL)
      nLevel = ICQ_TCPxMSG_NORMAL2;
    else if (nLevel == ICQ_TCPxMSG_URGENT)
    {
      f |= Licq::UserEvent::FlagUrgent;
      nLevel = ICQ_TCPxMSG_URGENT2;
    }
    else if (nLevel == ICQ_TCPxMSG_LIST)
      nLevel = ICQ_TCPxMSG_LIST2;

    CPU_ChatRequest *p = new CPU_ChatRequest(reasonDos,
        chatUsers, nPort, nLevel, *u, (u->Version() > 7));

    Licq::EventChat* e = new Licq::EventChat(reason, chatUsers.c_str(), nPort, p->Sequence(),
        Licq::EventChat::TimeNow, f);
		gLog.info(tr("%sSending chat request to %s (#%hu).\n"), L_SRVxSTR,
			  u->GetAlias(), -p->Sequence());

      result = SendExpectEvent_Server(u->id(), p, e);
    }
  else
  {
    CPT_ChatRequest* p = new CPT_ChatRequest(reasonDos, chatUsers, nPort,
        nLevel, *u, (u->Version() > 7));
    f = Licq::UserEvent::FlagDirect | Licq::UserEvent::FlagLicqVerMask;
    if (nLevel == ICQ_TCPxMSG_URGENT)
      f |= Licq::UserEvent::FlagUrgent;
    if (u->Secure())
      f |= Licq::UserEvent::FlagEncrypted;
    Licq::EventChat* e = new Licq::EventChat(reason, chatUsers.c_str(), nPort, p->Sequence(),
        Licq::UserEvent::TimeNow, f);
		gLog.info(tr("%sSending %schat request to %s (#%hu).\n"), L_TCPxSTR,
							nLevel == ICQ_TCPxMSG_URGENT ? tr("urgent ") : "",
							u->GetAlias(), -p->Sequence());
    result = SendExpectEvent_Client(*u, p, e);
	}
	
	u->SetSendServer(bServer);
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
  gLog.info(tr("%sCancelling chat request with %s (#%hu).\n"), L_TCPxSTR, 
            u->GetAlias(), -nSequence);
  CPT_CancelChat p(nSequence, *u);
  AckTCP(p, u->SocketDesc(ICQ_CHNxNONE));
}

void IcqProtocol::icqChatRequestRefuse(const Licq::UserId& userId, const string& reason,
    unsigned short nSequence, const unsigned long nMsgID[2], bool bDirect)
{
  // add to history ??
  Licq::UserWriteGuard u(userId);
  if (!u.isLocked())
    return;
  gLog.info(tr("%sRefusing chat request with %s (#%hu).\n"), 
            bDirect ? L_TCPxSTR : L_SRVxSTR, u->GetAlias(), -nSequence);
  string reasonDos = gTranslator.clientToServer(reason, true);

	if (bDirect)
  {
    CPT_AckChatRefuse p(reasonDos, nSequence, *u);
		AckTCP(p, u->SocketDesc(ICQ_CHNxNONE));
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
  gLog.info(tr("%sAccepting chat request with %s (#%hu).\n"), 
            bDirect ? L_TCPxSTR : L_SRVxSTR, u->GetAlias(), -nSequence);

	if (bDirect)
  {
    CPT_AckChatAccept p(nPort, clients, nSequence, *u, u->Version() > 7);
		AckTCP(p, u->SocketDesc(ICQ_CHNxNONE));
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
  gLog.info(tr("%sSending request for secure channel to %s (#%hu).\n"), L_TCPxSTR,
            u->GetAlias(), -pkt->Sequence());
  SendExpectEvent_Client(eventId, *u, pkt, NULL);

  u->SetSendServer(false);

#else // No OpenSSL
  gLog.warning("%sicqOpenSecureChannel() to %s called when we do not support OpenSSL.\n",
      L_WARNxSTR, userId.toString().c_str());
#endif
}

void IcqProtocol::icqCloseSecureChannel(unsigned long eventId, const Licq::UserId& userId)
{
#ifdef USE_OPENSSL
  Licq::UserWriteGuard u(userId);
  if (!u.isLocked())
    return;

  CPT_CloseSecureChannel *pkt = new CPT_CloseSecureChannel(*u);
  gLog.info(tr("%sClosing secure channel with %s (#%hu).\n"), L_TCPxSTR,
            u->GetAlias(), -pkt->Sequence());
  SendExpectEvent_Client(eventId, *u, pkt, NULL);

  u->SetSendServer(false);

#else // No OpenSSL
  gLog.warning("%sicqCloseSecureChannel() to %s called when we do not support OpenSSL.\n",
      L_WARNxSTR, userId.toString().c_str());
#endif
}

void IcqProtocol::icqOpenSecureChannelCancel(const Licq::UserId& userId,
  unsigned short nSequence)
{
  Licq::UserWriteGuard u(userId);
  if (!u.isLocked())
    return;
  gLog.info(tr("%sCancelling secure channel request to %s (#%hu).\n"), L_TCPxSTR,
            u->GetAlias(), -nSequence);
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
        gLog.warning(tr("%sBad handshake ack: %ld.\n"), L_WARNxSTR, nOk);
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
        gLog.warning(tr("%sBad handshake session id: received %ld, expecting %ld.\n"),
           L_WARNxSTR, p_in.SessionId(), p.SessionId());
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
        gLog.warning(tr("%sBad handshake ack: %ld.\n"), L_WARNxSTR, nOk);
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
        gLog.warning(tr("%sBad handshake cookie: received %ld, expecting %ld.\n"),
           L_WARNxSTR, p_in.SessionId(), p.SessionId());
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
      gLog.error("%sUnknown ICQ TCP version (%d).\n", L_ERRORxSTR, nVersion);
      return false;
  }

  return true;
  
sock_error:
  if (s->Error() == 0)
    gLog.warning(tr("%sHandshake error, remote side closed connection.\n"), L_WARNxSTR);
  else
    gLog.warning(tr("%sHandshake socket error:\n%s%s.\n"), L_WARNxSTR, L_BLANKxSTR, s->errorStr().c_str());
  return false;
}


/*------------------------------------------------------------------------------
 * ConnectToUser
 *
 * Creates a new TCPSocket and connects it to a given user.  Adds the socket
 * to the global socket manager and to the user.
 *----------------------------------------------------------------------------*/
int IcqProtocol::connectToUser(const Licq::UserId& userId, unsigned char nChannel)
{
  {
    Licq::UserReadGuard u(userId);
    if (!u.isLocked())
      return -1;

    // Check that we need to connect at all
    int sd = u->SocketDesc(nChannel);
    if (sd != -1)
    {
      gLog.warning(tr("%sConnection attempted to already connected user (%s).\n"),
          L_WARNxSTR, userId.toString().c_str());
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
    int sd = u->SocketDesc(ICQ_CHNxNONE);
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
  s->SetChannel(nChannel);

  gLog.info(tr("%sShaking hands with %s (%s) [v%d].\n"), L_TCPxSTR,
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
    u->SetSocketDesc(s);
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
    gLog.info(tr("%sConnecting to %s at %s:%d.\n"), L_TCPxSTR, name.c_str(),
        Licq::ip_ntoa(nIp, buf), nPort);
    // If we fail to set the remote address, the ip must be 0
    if (!sock->connectTo(nIp, nPort))
    {
      gLog.warning(tr("%sConnect to %s failed:\n%s%s.\n"), L_WARNxSTR, name.c_str(),
          L_BLANKxSTR, sock->errorStr().c_str());

      // Now try the internal ip if it is different from this one and we are behind a firewall
      if (sock->Error() != EINTR && nIntIp != nIp &&
          nIntIp != 0 && CPacket::Firewall())
      {
        gLog.info(tr("%sConnecting to %s at %s:%d.\n"), L_TCPxSTR, name.c_str(),
            Licq::ip_ntoa(nIntIp, buf), nPort);

        if (!sock->connectTo(nIntIp, nPort))
        {
          gLog.warning(tr("%sConnect to %s real ip failed:\n%s%s.\n"), L_WARNxSTR, name.c_str(),
              L_BLANKxSTR, sock->errorStr().c_str());
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
    gLog.info(tr("%sConnecting to %s at %s:%d.\n"), L_TCPxSTR, name.c_str(),
       Licq::ip_ntoa(nIntIp, buf), nPort);
    if (!sock->connectTo(nIntIp, nPort))
    {
      gLog.warning(tr("%sConnect to %s real ip failed:\n%s%s.\n"), L_WARNxSTR, name.c_str(),
          L_BLANKxSTR, sock->errorStr().c_str());
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
    gLog.warning("%sReverse connection to unknown port (%d).\n", L_WARNxSTR,
                                                              nFailedPort);
    return -1;
  }

  Licq::TCPSocket* s = new Licq::TCPSocket(userId);
  char buf[32];

  gLog.info(tr("%sReverse connecting to %s at %s:%d.\n"), L_TCPxSTR, userId.toString().c_str(),
      Licq::ip_ntoa(nIp, buf), nPort);

  // If we fail to set the remote address, the ip must be 0
  if (!s->connectTo(nIp, nPort))
  {
    gLog.warning(tr("%sReverse connect to %s failed:\n%s%s.\n"), L_WARNxSTR,
        userId.toString().c_str(), L_BLANKxSTR, s->errorStr().c_str());

    CPU_ReverseConnectFailed* p = new CPU_ReverseConnectFailed(userId.accountId(), nMsgID1,
        nMsgID2, nPort, nFailedPort, nId);
    SendEvent_Server(p);
    return -1;
  }

  gLog.info(tr("%sReverse shaking hands with %s.\n"), L_TCPxSTR, userId.toString().c_str());
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
        u->SetSocketDesc(s);
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

      if (pSock->Channel() == ICQ_CHNxNONE)
      {
        ackFlags = packet.UnpackUnsignedShort();
        msgFlags = packet.UnpackUnsignedShort();
        packet >> messageLen;

        // Stupid AOL
        Licq::UserReadGuard u(userId);
        if (u.isLocked() && (u->LicqVersion() == 0 || u->LicqVersion() >= 1022))
        {
          msgFlags <<= 4;
          msgFlags &= 0x0060;
          if (msgFlags & ICQ_TCPxMSG_URGENT)
            msgFlags = ICQ_TCPxMSG_LIST;
          else if (msgFlags & ICQ_TCPxMSG_LIST)
            msgFlags = ICQ_TCPxMSG_URGENT;
        }
      }
      
      break;
    }
    default:
    {
      gLog.warning(tr("%sUnknown TCP version %d from socket.\n"), L_WARNxSTR, nInVersion);
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
  unsigned short nOwnerStatus;
  {
    Licq::OwnerReadGuard o(LICQ_PPID);
    nOwnerStatus = o->Status();
  }

  // find which user was sent
  bool bNewUser = false;
  Licq::UserWriteGuard u(userId, true, &bNewUser);
  if (bNewUser)
    u->SetSocketDesc(pSock);

  // Check for spoofing
  if (u->SocketDesc(pSock->Channel()) != sockfd)
  {
    gLog.warning("%sUser %s (%s) socket (%d) does not match incoming message (%d).\n",
        L_TCPxSTR, u->getAlias().c_str(), u->accountId().c_str(),
              u->SocketDesc(pSock->Channel()), sockfd);
  }

  if (pSock->Channel() != ICQ_CHNxNONE)
  {
    errorOccured = ProcessPluginMessage(packet, *u, pSock->Channel(),
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

  if (nInVersion <= 4)
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
  if ( ((nOwnerStatus == ICQ_STATUS_OCCUPIED || u->StatusToUser() == ICQ_STATUS_OCCUPIED)
         && u->AcceptInOccupied() ) ||
       ((nOwnerStatus == ICQ_STATUS_DND || u->StatusToUser() == ICQ_STATUS_DND)
         && u->AcceptInDND() ) ||
       (u->StatusToUser() != ICQ_STATUS_OFFLINE && u->StatusToUser() != ICQ_STATUS_OCCUPIED
         && u->StatusToUser() != ICQ_STATUS_DND) )
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
    if (nInVersion >= 7  && (u->LicqVersion() == 0 || u->LicqVersion() >= 1022))
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
        gLog.warning(tr("%sUnknown TCP status: %04X\n"), L_WARNxSTR, msgFlags);
        break;
    }
    //fprintf(stderr, "%08lX\n", (u->StatusFull() & ICQ_STATUS_FxFLAGS) | ns);
    /*if (!bNewUser && ns != ICQ_STATUS_OFFLINE &&
        !((ns & ICQ_STATUS_FxPRIVATE) && !u->isOnline()))*/
    if (!bNewUser && ns != ICQ_STATUS_OFFLINE &&
        !(ns == ICQ_STATUS_ONLINE && u->Status() == ICQ_STATUS_FREEFORCHAT) &&
        ns != (u->Status() | (u->isInvisible() ? ICQ_STATUS_FxPRIVATE : 0)))
    {
      bool r = u->OfflineOnDisconnect() || !u->isOnline();
        ChangeUserStatus(*u, (u->StatusFull() & ICQ_STATUS_FxFLAGS) | ns);
      gLog.info(tr("%s%s (%s) is %s to us.\n"), L_TCPxSTR, u->GetAlias(),
            u->id().toString().c_str(), u->statusString().c_str());
      if (r) u->SetOfflineOnDisconnect(true);
    }

    // Process the command
    switch(newCommand)
    {
      case ICQ_CMDxSUB_MSG:  // straight message from a user
      {
        unsigned long back = 0xFFFFFF, fore = 0x000000;
        if (nInVersion <= 4)
        {
          if (packet.getDataPosRead() + 4 >
                              (packet.getDataStart() + packet.getDataSize()))
          {
            theSequence = (signed short)packet.UnpackUnsignedShort();
          }
          else
            packet >> theSequence;
        }
        else {
          packet >> fore >> back;
          if( fore == back ) {
            back = 0xFFFFFF;
            fore = 0x000000;
          }
        }

				packet >> licqChar >> licqVersion;
				nMask |= licqVersion;
				if (licqChar == 'L')
            gLog.info(tr("%sMessage from %s (%s) [Licq %s].\n"), L_TCPxSTR,
                u->getAlias().c_str(), userId.toString().c_str(),
                Licq::UserEvent::licqVersionToString(licqVersion).c_str());
	  else
            gLog.info(tr("%sMessage from %s (%s).\n"), L_TCPxSTR, u->GetAlias(), userId.toString().c_str());

          CPT_AckGeneral p(newCommand, theSequence, true, bAccept, *u);
        AckTCP(p, pSock);

          Licq::EventMsg* e = new Licq::EventMsg(Licq::gTranslator.serverToClient(message),
              ICQ_CMDxTCP_START, Licq::EventMsg::TimeNow, nMask);
        e->SetColor(fore, back);

        // If we are in DND or Occupied and message isn't urgent then we ignore it
        if (!bAccept)
        {
          if (nOwnerStatus == ICQ_STATUS_OCCUPIED || nOwnerStatus == ICQ_STATUS_DND)
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
        if (nInVersion <= 4)
        {
          if (packet.getDataPosRead() + 4 >
                              (packet.getDataStart() + packet.getDataSize()))
          {
            theSequence = (signed short)packet.UnpackUnsignedShort();
          }
          else
            packet >> theSequence;
        }
        else
          packet >> junkLong >> junkLong;
        packet >> licqChar >> licqVersion;
        if (licqChar == 'L')
            gLog.info(tr("%s%s (%s) requested auto response [Licq %s].\n"), L_TCPxSTR,
                u->getAlias().c_str(), userId.toString().c_str(),
                Licq::UserEvent::licqVersionToString(licqVersion).c_str());
          else
            gLog.info(tr("%s%s (%s) requested auto response.\n"), L_TCPxSTR, u->GetAlias(), userId.toString().c_str());

          CPT_AckGeneral p(newCommand, theSequence, true, false, *u);
        AckTCP(p, pSock);

          Licq::gStatistics.increase(Licq::Statistics::AutoResponseCheckedCounter);
        u->SetLastCheckedAutoResponse();

          gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
              Licq::PluginSignal::UserEvents, u->id()));
          break;
      }

      case ICQ_CMDxSUB_URL:  // url sent
      {
        unsigned long back = 0xFFFFFF, fore = 0x000000;
        if (nInVersion <= 4)
        {
          if (packet.getDataPosRead() + 4 >
                              (packet.getDataStart() + packet.getDataSize()))
          {
            theSequence = (signed short)packet.UnpackUnsignedShort();
          }
          else
            packet >> theSequence;
        }
        else {
          packet >> fore >> back;
          if(fore == back)
          {
            fore = 0x000000;
            back = 0xFFFFFF;
          }
        }
        packet >> licqChar >> licqVersion;
        nMask |= licqVersion;
        if (licqChar == 'L')
            gLog.info(tr("%sURL from %s (%s) [Licq %s].\n"), L_TCPxSTR, u->GetAlias(),
                userId.toString().c_str(), Licq::UserEvent::licqVersionToString(licqVersion).c_str());
          else
            gLog.info(tr("%sURL from %s (%s).\n"), L_TCPxSTR, u->GetAlias(), userId.toString().c_str());

          Licq::EventUrl* e = Licq::EventUrl::Parse(message, ICQ_CMDxTCP_START, Licq::EventUrl::TimeNow, nMask);
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
          if (nOwnerStatus == ICQ_STATUS_OCCUPIED || nOwnerStatus == ICQ_STATUS_DND)
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
        if (nInVersion <= 4)
        {
          if (packet.getDataPosRead() + 4 >
                              (packet.getDataStart() + packet.getDataSize()))
          {
            theSequence = (signed short)packet.UnpackUnsignedShort();
          }
          else
            packet >> theSequence;
        }
        else {
          packet >> fore >> back;
          if(fore == back) {
            fore = 0x000000;
            back = 0xFFFFFF;
          }
        }
        packet >> licqChar >> licqVersion;
        nMask |= licqVersion;
        if (licqChar == 'L')
            gLog.info(tr("%sContact list from %s (%s) [Licq %s].\n"), L_TCPxSTR,
                u->getAlias().c_str(), userId.toString().c_str(),
                Licq::UserEvent::licqVersionToString(licqVersion).c_str());
          else
            gLog.info(tr("%sContact list from %s (%s).\n"), L_TCPxSTR,
                u->GetAlias(), userId.toString().c_str());

          Licq::EventContactList* e = Licq::EventContactList::Parse(message, ICQ_CMDxTCP_START, Licq::EventContactList::TimeNow, nMask);
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
          if (nOwnerStatus == ICQ_STATUS_OCCUPIED || nOwnerStatus == ICQ_STATUS_DND)
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
        if (nInVersion <= 4)
        {
          if (packet.getDataPosRead() + 4 >
                              (packet.getDataStart() + packet.getDataSize()))
          {
            theSequence = (signed short)packet.UnpackUnsignedShort();
          }
          else
            packet >> theSequence;
        }
        packet >> licqChar >> licqVersion;

        if (licqChar == 'L')
            gLog.info(tr("%sChat request from %s (%s) [Licq %s].\n"), L_TCPxSTR,
                u->getAlias().c_str(), userId.toString().c_str(),
                Licq::UserEvent::licqVersionToString(licqVersion).c_str());
          else
            gLog.info(tr("%sChat request from %s (%s).\n"), L_TCPxSTR,
                u->GetAlias(), userId.toString().c_str());

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
        if (nInVersion <= 4)
        {
          if (packet.getDataPosRead() + 4 >
                              (packet.getDataStart() + packet.getDataSize()))
          {
            theSequence = (signed short)packet.UnpackUnsignedShort();
          }
          else
            packet >> theSequence;
        }
        packet >> licqChar >> licqVersion;

        if (licqChar == 'L')
            gLog.info(tr("%sFile transfer request from %s (%s) [Licq %s].\n"),
                L_TCPxSTR, u->getAlias().c_str(), userId.toString().c_str(),
                Licq::UserEvent::licqVersionToString(licqVersion).c_str());
          else
            gLog.info(tr("%sFile transfer request from %s (%s).\n"), L_TCPxSTR,
                u->GetAlias(), userId.toString().c_str());

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
            gLog.info(tr("%sUnknown ICBM plugin type: %s\n"), L_TCPxSTR, plugin.c_str());
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

              gLog.info(tr("%sFile transfer request from %s (%s).\n"),
                  L_TCPxSTR, u->GetAlias(), userId.toString().c_str());

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

              gLog.info(tr("%sChat request from %s (%s).\n"), L_TCPxSTR,
                  u->GetAlias(), userId.toString().c_str());

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
              gLog.info(tr("%sURL from %s (%s).\n"), L_TCPxSTR, u->GetAlias(), userId.toString().c_str());
                Licq::EventUrl* e = Licq::EventUrl::Parse(szMessage, ICQ_CMDxTCP_START,
                    Licq::EventUrl::TimeNow, nMask);
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
              gLog.info(tr("%sContact list from %s (%s).\n"), L_TCPxSTR,
                  u->GetAlias(), userId.toString().c_str());
                Licq::EventContactList* e = Licq::EventContactList::Parse(szMessage,
                    ICQ_CMDxTCP_START, Licq::EventContactList::TimeNow, nMask);
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
          gLog.info(tr("%sReceived old-style key request from %s (%s) but we do not support it.\n"),
              L_TCPxSTR, u->GetAlias(), userId.toString().c_str());
        // Send the nack back
          CPT_AckOldSecureChannel p(theSequence, *u);
        AckTCP(p, pSock);
        break;
      }

      // Secure channel request
      case ICQ_CMDxSUB_SECURExOPEN:
      {
#ifdef USE_OPENSSL
        if (nInVersion <= 4)
        {
          if (packet.getDataPosRead() + 4 >
                              (packet.getDataStart() + packet.getDataSize()))
          {
            theSequence = (signed short)packet.UnpackUnsignedShort();
          }
          else
            packet >> theSequence;
        }
        packet >> licqChar >> licqVersion;

        if (licqChar == 'L')
            gLog.info(tr("%sSecure channel request from %s (%s) [Licq %s].\n"),
                L_TCPxSTR, u->getAlias().c_str(), userId.toString().c_str(),
                Licq::UserEvent::licqVersionToString(licqVersion).c_str());
          else
            gLog.info(tr("%sSecure channel request from %s (%s).\n"), L_TCPxSTR,
                u->GetAlias(), userId.toString().c_str());

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
          gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
              Licq::PluginSignal::UserSecurity, u->id(), 1));

          gLog.info(tr("Secure channel established with %s (%s)"),
              u->GetAlias(), userId.toString().c_str());

        break;

#else // We do not support OpenSSL
          gLog.info(tr("%sReceived secure channel request from %s (%s) but we do not support OpenSSL.\n"),
              L_TCPxSTR, u->GetAlias(), userId.toString().c_str());
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
        if (nInVersion <= 4)
        {
          if (packet.getDataPosRead() + 4 >
                              (packet.getDataStart() + packet.getDataSize()))
          {
            theSequence = (signed short)packet.UnpackUnsignedShort();
          }
          else
            packet >> theSequence;
        }
        packet >> licqChar >> licqVersion;

        if (licqChar == 'L')
            gLog.info(tr("%sSecure channel closed by %s (%s) [Licq %s].\n"),
                L_TCPxSTR, u->getAlias().c_str(), userId.toString().c_str(),
                Licq::UserEvent::licqVersionToString(licqVersion).c_str());
          else
            gLog.info(tr("%sSecure channel closed by %s (%s).\n"), L_TCPxSTR,
                u->GetAlias(), userId.toString().c_str());

        // send ack
          CPT_AckCloseSecureChannel p(theSequence, *u);
        AckTCP(p, pSock);

        pSock->SecureStop();
        u->SetSecure(false);
          gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
              Licq::PluginSignal::UserSecurity, u->id(), 0));
          break;

#else // We do not support OpenSSL
          gLog.info(tr("%sReceived secure channel close from %s (%s) but we do not support OpenSSL.\n"),
              L_TCPxSTR, u->GetAlias(), userId.toString().c_str());
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
        if (nInVersion <= 4)
        {
          if (packet.getDataPosRead() + 4 >
                              (packet.getDataStart() + packet.getDataSize()))
          {
            theSequence = (signed short)packet.UnpackUnsignedShort();
          }
          else
            packet >> theSequence;
        }
        else
          packet >> junkLong >> junkLong;
        packet >> licqChar >> licqVersion;
        break;

      case ICQ_CMDxSUB_CHAT:
      {
        char ul[1024];
        packet.UnpackString(ul, sizeof(ul));
        packet >> nPortReversed   // port backwards
               >> nPort;    // port to connect to for chat
        if (nInVersion <= 4)
        {
          if (packet.getDataPosRead() + 4 >
                              (packet.getDataStart() + packet.getDataSize()))
          {
            theSequence = (signed short)packet.UnpackUnsignedShort();
          }
          else
            packet >> theSequence;
        }
        packet >> licqChar >> licqVersion;

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
         if (nInVersion <= 4)
         {
           if (packet.getDataPosRead() + 4 >
                              (packet.getDataStart() + packet.getDataSize()))
           {
             theSequence = (signed short)packet.UnpackUnsignedShort();
           }
           else
             packet >> theSequence;
         }
         packet >> licqChar >> licqVersion;

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
            gLog.info(tr("%sUnknown direct ack ICBM plugin type: %s\n"), L_TCPxSTR, plugin.c_str());

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
        if (nInVersion <= 4)
        {
          if (packet.getDataPosRead() + 4 >
                              (packet.getDataStart() + packet.getDataSize()))
          {
            theSequence = (signed short)packet.UnpackUnsignedShort();
          }
          else
            packet >> theSequence;
        }
        packet >> licqChar >> licqVersion;

        char l[32] = "";
          if (licqChar == 'L')
            sprintf(l, " [Licq %s]", Licq::UserEvent::licqVersionToString(licqVersion).c_str());
          gLog.info(tr("%sSecure channel response from %s (%s)%s.\n"), L_TCPxSTR,
              u->GetAlias(), userId.toString().c_str(), l);

          Licq::Event* e = NULL;

        // Check if the response is ok
        if (message[0] == '\0')
        {
            gLog.info(tr("%s%s (%s) does not support OpenSSL.\n"), L_TCPxSTR,
                u->GetAlias(), userId.toString().c_str());
          u->SetSecure(false);
            gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
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
              gLog.warning(tr("%sSecure channel response from %s (%s) when no request in progress.\n"),
                  L_WARNxSTR, u->GetAlias(), userId.toString().c_str());
            // Close the connection as we are in trouble
            u->SetSecure(false);
              u.unlock();
              if (bNewUser)
                Licq::gUserManager.removeUser(userId, false);
              gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
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
                  u->GetAlias(), userId.toString().c_str());
            u->SetSecure(true);
              gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
                  Licq::PluginSignal::UserSecurity, u->id(), 1));
            }
          }

        // finish up
        e->m_nSubResult = ICQ_TCPxACK_ACCEPT;
          u.unlock();
          if (bNewUser)
            Licq::gUserManager.removeUser(userId, false);
        ProcessDoneEvent(e);

        // get out of here now as we don't want standard ack processing
        return !errorOccured;
      }


      case ICQ_CMDxSUB_SECURExCLOSE:
      {
        if (nInVersion <= 4)
        {
          if (packet.getDataPosRead() + 4 >
                              (packet.getDataStart() + packet.getDataSize()))
          {
            theSequence = (signed short)packet.UnpackUnsignedShort();
          }
          else
            packet >> theSequence;
        }
        packet >> licqChar >> licqVersion;

        char l[32] = "";
          if (licqChar == 'L')
            sprintf(l, " [Licq %s]", Licq::UserEvent::licqVersionToString(licqVersion).c_str());
          gLog.info(tr("%sSecure channel with %s (%s) closed %s.\n"), L_TCPxSTR,
              u->GetAlias(), userId.toString().c_str(), l);

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
          gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
              Licq::PluginSignal::UserSecurity, u->id(), 0));

          u.unlock();
          if (bNewUser)
            Licq::gUserManager.removeUser(userId, false);

        // finish up
        e->m_nSubResult = ICQ_TCPxACK_ACCEPT;
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
    int nSubResult;
    if (ackFlags == ICQ_TCPxACK_REFUSE)
    {
      gLog.info(tr("%sRefusal from %s (#%hu)%s.\n"), L_TCPxSTR, u->GetAlias(), -theSequence, l);
      nSubResult = ICQ_TCPxACK_REFUSE;
    }
    else
    {
      // Update the away message if it's changed
      if (u->autoResponse() != message)
      {
          u->setAutoResponse(message);
        u->SetShowAwayMsg(*message);
        gLog.info(tr("%sAuto response from %s (#%hu)%s.\n"), L_TCPxSTR,
                  u->GetAlias(), -theSequence, l);
      }

      switch(ackFlags)
      {
        case ICQ_TCPxACK_ONLINE:
          gLog.info(tr("%sAck from %s (#%hu)%s.\n"), L_TCPxSTR, u->GetAlias(), -theSequence, l);
            if (pExtendedAck && !pExtendedAck->accepted())
            nSubResult = ICQ_TCPxACK_RETURN;
          else
            nSubResult = ICQ_TCPxACK_ACCEPT;
          break;
        case ICQ_TCPxACK_AWAY:
        case ICQ_TCPxACK_NA:
        case ICQ_TCPxACK_OCCUPIEDx2: //auto decline due to occupied mode
          gLog.info(tr("%sAck from %s (#%hu)%s.\n"), L_TCPxSTR, u->GetAlias(), -theSequence, l);
          nSubResult = ICQ_TCPxACK_REFUSE;
          break;
        case ICQ_TCPxACK_OCCUPIED:
        case ICQ_TCPxACK_DND:
          gLog.info(tr("%sReturned from %s (#%hu)%s.\n"), L_TCPxSTR, u->GetAlias(), -theSequence, l);
          nSubResult = ICQ_TCPxACK_RETURN;
          break;
        case ICQ_TCPxACK_OCCUPIEDxCAR:
        case ICQ_TCPxACK_DNDxCAR:
          gLog.info(tr("%sCustom %s response from %s (#%hu)%s.\n"), L_TCPxSTR,
                    (ackFlags == ICQ_TCPxACK_DNDxCAR ? tr("DnD") : tr("Occupied")), u->GetAlias(),
                    -theSequence, l);
          nSubResult = ICQ_TCPxACK_ACCEPT; // FIXME: or should this be ACK_RETURN ?
          break;
        default:
          gLog.unknown("Unknown ack flag from %s (#%hu): %04x %s",
                       u->GetAlias(), -theSequence, ackFlags, l);
          nSubResult = ICQ_TCPxACK_ACCEPT;
      }
    }

      Licq::Event *e = DoneEvent(sockfd, theSequence, Licq::Event::ResultAcked);
    if (e != NULL)
    {
      e->m_pExtendedAck = pExtendedAck;
      e->m_nSubResult = nSubResult;

        u.unlock();
        if (bNewUser)
          Licq::gUserManager.removeUser(userId, false);

      ProcessDoneEvent(e);
      return true;
    }
    else
    {
      gLog.warning(tr("%sAck for unknown event.\n"), L_TCPxSTR);
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
          gLog.info(tr("%sChat request from %s (%s) cancelled.\n"), L_TCPxSTR,
              u->GetAlias(), userId.toString().c_str());
        if (nInVersion <= 4)
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
          gLog.info(tr("%sFile transfer request from %s (%s) cancelled.\n"),
              L_TCPxSTR, u->GetAlias(), userId.toString().c_str());
        if (nInVersion <= 4)
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

bool IcqProtocol::ProcessPluginMessage(CBuffer &packet, Licq::User* u,
                                      unsigned char nChannel,
                                      bool bIsAck,
                                      unsigned long nMsgID1,
                                      unsigned long nMsgID2,
                                      unsigned short nSequence,
    Licq::TCPSocket* pSock)
{
  bool errorOccured = false;
  const char *szInfo = pSock ? L_TCPxSTR : L_SRVxSTR;

  switch (nChannel)
  {
  case ICQ_CHNxINFO:
  {
    packet.incDataPosRead(2);
    char error_level = packet.UnpackChar();

    if (!bIsAck)
    {
      if (error_level != ICQ_PLUGIN_REQUEST)
      {
        gLog.warning("%sInfo plugin request with unknown level %u from %s.\n",
                  L_WARNxSTR, error_level, u->GetAlias());
        errorOccured = true;
        break;
      }
      char GUID[GUID_LENGTH];
      for (int i = 0 ; i < GUID_LENGTH; i ++)
        packet >> GUID[i];

      if (memcmp(GUID, PLUGIN_QUERYxINFO, GUID_LENGTH) == 0)
      {
        gLog.info("%sInfo plugin list request from %s.\n", szInfo,
                                                          u->GetAlias());
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
        gLog.info("%sPhone Book request from %s.\n", szInfo, u->GetAlias());
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
        gLog.info("%sPicture request from %s.\n", szInfo, u->GetAlias());

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
        gLog.warning("%sUnknown info request from %s.\n", L_WARNxSTR,
                                                       u->GetAlias());
        if (pSock)
        {
          CPT_PluginError p(u, nSequence, ICQ_CHNxINFO);
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
            gLog.warning("%sAck for unknown event from %s.\n", L_WARNxSTR,
                                                            u->GetAlias());
            return true;
          }

          const char *GUID;
          if (e->SNAC() ==
                    MAKESNAC(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SENDxSERVER) &&
              e->ExtraInfo() == ServerInfoPluginRequest)
          {
            GUID = ((CPU_InfoPluginReq *)e->m_pPacket)->RequestGUID();
          }
          else if (e->Channel() == ICQ_CHNxINFO &&
                   e->ExtraInfo() == DirectInfoPluginRequest)
          {
            GUID = ((CPT_InfoPluginReq *)e->m_pPacket)->RequestGUID();
          }
          else
          {
            gLog.warning("%sAck for the wrong event from %s.\n", L_WARNxSTR,
                                                              u->GetAlias());
            delete e;
            return true;
          }

          if (memcmp(GUID, PLUGIN_PICTURE, GUID_LENGTH) == 0)
          {
            gLog.info("%s%s has no picture.\n", szInfo, u->GetAlias());

                if (remove(u->pictureFileName().c_str()) != 0 && errno != ENOENT)
                {
              gLog.error("%sUnable to delete %s's picture file (%s):\n%s%s.\n",
                      L_ERRORxSTR, u->GetAlias(), u->pictureFileName().c_str(), L_BLANKxSTR, strerror(errno));
                }

            u->SetEnableSave(false);
            u->SetPicturePresent(false);
            u->SetEnableSave(true);
            u->SavePictureInfo();

              gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
                  Licq::PluginSignal::UserPicture, u->id()));
            }
          else if (memcmp(GUID, PLUGIN_QUERYxINFO, GUID_LENGTH) == 0)
          {
            gLog.info("%s%s has no info plugins.\n", szInfo, u->GetAlias());
          }
          else
          {
            gLog.unknown("%sUnknown info response with no data from %s.\n",
                                                        szInfo, u->GetAlias());
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

                    gLog.info("%s%s has %s (%s).\n", szInfo, u->GetAlias(), name.c_str(), fullName.c_str());
                  }
                  break;
                }

          case ICQ_PLUGIN_RESP_PHONExBOOK:
          {
            gLog.info("%sPhone Book reply from %s.\n", szInfo, u->GetAlias());
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
            u->SavePhoneBookInfo();
                  delete [] pb;

                gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
                    Licq::PluginSignal::UserInfo, u->id()));
                break;
              }

          case ICQ_PLUGIN_RESP_PICTURE:
          {
            gLog.info("%sPicture reply from %s.\n", szInfo, u->GetAlias());
            packet.incDataPosRead(nEntries); // filename, don't care
            unsigned long nLen = packet.UnpackUnsignedLong();
            if (nLen == 0)	// do not create empty .pic files
              break;

                  int nFD = open(u->pictureFileName().c_str(), O_WRONLY | O_CREAT | O_TRUNC, 00664);
            if (nFD == -1)
            {
              gLog.error("%sUnable to open picture file (%s):\n%s%s.\n",
                        L_ERRORxSTR, u->pictureFileName().c_str(), L_BLANKxSTR, strerror(errno));
                    break;
                  }

                  string data = packet.unpackRawString(nLen);
                  write(nFD, data.c_str(), nLen);
            close(nFD);

            u->SetEnableSave(false);
            u->SetPicturePresent(true);
            u->SetEnableSave(true);
            u->SavePictureInfo();

                gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
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
        gLog.warning("%sInfo plugin not available from %s.\n", L_WARNxSTR,
                                                            u->GetAlias());
            result = Licq::Event::ResultError;
            break;
          }
      case ICQ_PLUGIN_REJECTED:
      {
        gLog.info("%s%s refused our request.\n", szInfo, u->GetAlias());
            result = Licq::Event::ResultFailed;
            break;
          }
      case ICQ_PLUGIN_AWAY:
      {
        gLog.info("%sOur request was refused because %s is away.\n", szInfo,
                  u->GetAlias());
            result = Licq::Event::ResultFailed;
            break;
          }
      default:
      {
        gLog.warning("Unknown reply level %u from %s",
                  error_level, u->GetAlias());
        errorOccured = true;
            result = Licq::Event::ResultError;
            break;
          }
        }

        Licq::Event* e = pSock ? DoneEvent(pSock->Descriptor(), nSequence,
                                      result) :
                            DoneServerEvent(nMsgID2, result);
      if (e == NULL)
        gLog.warning("%sAck for unknown event from %s.\n", L_WARNxSTR,
                                                        u->GetAlias());
      else
        ProcessDoneEvent(e);

    }

    break;
  }
  case ICQ_CHNxSTATUS:
  {
    packet.incDataPosRead(2);
    char error_level = packet.UnpackChar();

    if (!bIsAck)
    {
      if (error_level != ICQ_PLUGIN_REQUEST)
      {
        gLog.warning("%sUnknown status plugin request level %u from %s.\n",
                  L_WARNxSTR, error_level, u->GetAlias());
        errorOccured = true;
        break;
      }

      char GUID[GUID_LENGTH];
      for (int i = 0 ; i < GUID_LENGTH; i ++)
        packet >> GUID[i];

      if (memcmp(GUID, PLUGIN_QUERYxSTATUS, GUID_LENGTH) == 0)
      {
        gLog.info("%sStatus plugin list request from %s.\n", szInfo,
                                                             u->GetAlias());
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
        gLog.info("%sFile server status request from %s.\n", szInfo,
                                                              u->GetAlias());
        unsigned long nStatus;
        {
          Licq::OwnerReadGuard o(LICQ_PPID);
          nStatus = o->SharedFilesStatus();
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
        gLog.info("%sICQphone status request from %s.\n", szInfo,
                                                              u->GetAlias());
        unsigned long nStatus;
        {
          Licq::OwnerReadGuard o(LICQ_PPID);
          nStatus = o->ICQphoneStatus();
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
        gLog.info("%sPhone \"Follow Me\" status request from %s.\n", szInfo,
                                                              u->GetAlias());
        unsigned long nStatus;
        {
          Licq::OwnerReadGuard o(LICQ_PPID);
          nStatus = o->PhoneFollowMeStatus();
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
        gLog.warning("%sUnknown status request from %s.\n", L_WARNxSTR,
                                                         u->GetAlias());
        if (pSock)
        {
          CPT_PluginError p(u, nSequence, ICQ_CHNxSTATUS);
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
          gLog.info("%s%s has no status plugins.\n", szInfo, u->GetAlias());
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

                gLog.info("%s%s has %s (%s).\n", szInfo, u->GetAlias(), name.c_str(), fullName.c_str());
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
          gLog.warning("%sAck for unknown event from %s.\n", L_WARNxSTR,
                                                          u->GetAlias());
          return true;
        }

        const char *GUID;
        if (e->SNAC() ==
                  MAKESNAC(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SENDxSERVER) &&
            e->ExtraInfo() == ServerStatusPluginRequest)
        {
          GUID = ((CPU_StatusPluginReq *)e->m_pPacket)->RequestGUID();
        }
        else if (e->Channel() == ICQ_CHNxINFO &&
                 e->ExtraInfo() == DirectStatusPluginRequest)
        {
          GUID = ((CPT_StatusPluginReq *)e->m_pPacket)->RequestGUID();
        }
        else
        {
          gLog.warning("%sAck for the wrong event from %s.\n", L_WARNxSTR,
                                                            u->GetAlias());
          delete e;
          return true;
        }

        packet.incDataPosRead(4); // Unknown
        unsigned long nState = packet.UnpackUnsignedLong();
        unsigned long nTime = packet.UnpackUnsignedLong();

        if (nTime == u->ClientStatusTimestamp())
          u->SetOurClientStatusTimestamp(nTime);

        const char* szState;
        switch (nState)
        {
          case ICQ_PLUGIN_STATUSxINACTIVE: szState = "inactive"; break;
          case ICQ_PLUGIN_STATUSxACTIVE:   szState = "active";   break;
          case ICQ_PLUGIN_STATUSxBUSY:     szState = "busy";     break;
          default:                         szState = "unknown";  break;
        }

        if (memcmp(GUID, PLUGIN_FILExSERVER, GUID_LENGTH) == 0)
        {
          gLog.info("%s%s's Shared Files Directory is %s.\n", szInfo,
                                                     u->GetAlias(), szState);
          u->SetSharedFilesStatus(nState);
        }
        else if (memcmp(GUID, PLUGIN_FOLLOWxME, GUID_LENGTH) == 0)
        {
          gLog.info("%s%s's Phone \"Follow Me\" is %s.\n", szInfo,
                                                     u->GetAlias(), szState);
          u->SetPhoneFollowMeStatus(nState);
        }
        else if (memcmp(GUID, PLUGIN_ICQxPHONE, GUID_LENGTH) == 0)
        {
          gLog.info("%s%s's ICQphone is %s.\n", szInfo, u->GetAlias(),
                                                                    szState);
          u->SetICQphoneStatus(nState);
        }

        // Which plugin?
            gDaemon.pushPluginSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
                Licq::PluginSignal::UserPluginStatus, u->id(), 0));

        ProcessDoneEvent(e);
        return false;
      }
      case ICQ_PLUGIN_ERROR:
      {
        gLog.warning("%sStatus plugin not available from %s.\n", L_WARNxSTR,
                                                            u->GetAlias());
            result = Licq::Event::ResultError;
            break;
          }
      case ICQ_PLUGIN_REJECTED:
      {
        gLog.info("%s%s refused our request.\n", szInfo, u->GetAlias());
            result = Licq::Event::ResultFailed;
            break;
          }
      case ICQ_PLUGIN_AWAY:
      {
        gLog.info("%sOur request was refused because %s is away.\n", szInfo,
                  u->GetAlias());
            result = Licq::Event::ResultFailed;
            break;
          }
      default:
      {
        gLog.warning("Unknown reply level %u from %s",
                  error_level, u->GetAlias());
        errorOccured = true;
            result = Licq::Event::ResultError;
            break;
          }
        }

        Licq::Event* e = pSock ? DoneEvent(pSock->Descriptor(), nSequence,
                                      result) :
                            DoneServerEvent(nMsgID2, result);
      if (e == NULL)
        gLog.warning("%sAck for unknown event from %s.\n", L_WARNxSTR,
                                                        u->GetAlias());
      else
        ProcessDoneEvent(e);

    }

    break;
  }
  default:
  {
    gLog.warning("%sUnknown channel %u from %s\n", L_WARNxSTR, nChannel,
                                                u->GetAlias());
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
          gLog.warning("%sConnection from unknown user.\n", L_WARNxSTR);
          return false;
        }
        nCookie = u.isLocked() ? u->Cookie() : 0;
      }

      if (nCookie != p_in.SessionId())
      {
        gLog.warning("%sSpoofed connection from %s as uin %s.\n", L_WARNxSTR,
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
        gLog.warning("%sHandshake ack not the right size.\n", L_WARNxSTR);
        return false;
      }

      unsigned long nOk = s->RecvBuffer().UnpackUnsignedLong();
      if (nOk != 1)
      {
        gLog.warning(tr("%sBad handshake ack: %ld.\n"), L_WARNxSTR, nOk);
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
              gLog.warning("%sReverse connection with unknown id (%lu)",
                L_WARNxSTR, p_in.Id());
              pthread_mutex_unlock(&mutex_reverseconnect);
              return false;
            }
            if ((*iter)->nId == p_in.Id() && (*iter)->myIdString == id)
            {
              s->SetChannel((*iter)->nData);
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
          gLog.warning("%sConnection from unknown user.\n", L_WARNxSTR);
          return false;
        }
        nCookie = u->Cookie();
      }

      if (nCookie != p_in.SessionId())
      {
        gLog.warning("%sSpoofed connection from %s as uin %s.\n", L_WARNxSTR,
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
        gLog.warning(tr("%sBad handshake ack: %ld.\n"), L_WARNxSTR, nOk);
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
          gLog.warning("%sConnection from unknown user.\n", L_WARNxSTR);
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
        gLog.warning("%sConnection from %s as %s possible spoof.\n", L_WARNxSTR,
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
          gLog.warning("%sConnection from unknown user.\n", L_WARNxSTR);
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
        gLog.warning("%sConnection from %s as %s possible spoof.\n", L_WARNxSTR,
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
    gLog.warning(tr("%sHandshake error, remote side closed connection.\n"), L_WARNxSTR);
  else
    gLog.warning(tr("%sHandshake socket error:\n%s%s.\n"), L_WARNxSTR, L_BLANKxSTR, s->errorStr().c_str());
  return false;
}

bool IcqProtocol::Handshake_SendConfirm_v7(Licq::TCPSocket* s)
{
  // Send handshake accepted
  CPacketTcp_Handshake_Confirm p_confirm(s->Channel(), 0);
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
      gLog.warning("%sHandshake confirm not the right size.\n", L_WARNxSTR);
      return false;
    }
    unsigned char c = b.UnpackChar();
    unsigned long l = b.UnpackUnsignedLong();
    if (c != 0x03 || l != 0x0000000A)
    {
      gLog.warning("%sUnknown handshake response %2X,%8lX.\n", L_WARNxSTR, c, l);
      return false;
    }
    b.Reset();
    CPacketTcp_Handshake_Confirm p_confirm_in(&b);
    if (p_confirm_in.Channel() != ICQ_CHNxUNKNOWN)
      s->SetChannel(p_confirm_in.Channel());
    else
    {
      gLog.warning("%sUnknown channel in ack packet.\n", L_WARNxSTR);
      return false;
    }

    s->ClearRecvBuffer();

    CPacketTcp_Handshake_Confirm p_confirm_out(p_confirm_in.Channel(),
                                                       p_confirm_in.Id());

    if (s->SendPacket(p_confirm_out.getBuffer()))
      return true;
  }
 
 sock_error:
  if (s->Error() == 0)
    gLog.warning(tr("%sHandshake error, remote side closed connection.\n"), L_WARNxSTR);
  else
    gLog.warning(tr("%sHandshake socket error:\n%s%s.\n"), L_WARNxSTR, L_BLANKxSTR, s->errorStr().c_str());
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
    gLog.info(tr("%sConnection from %s (%s) [v%ld].\n"), L_TCPxSTR,
        u->GetAlias(), userId.toString().c_str(), s->Version());
    if (u->SocketDesc(s->Channel()) != s->Descriptor())
    {
      if (u->SocketDesc(s->Channel()) != -1)
      {
        gLog.warning(tr("%sUser %s (%s) already has an associated socket.\n"),
            L_WARNxSTR, u->GetAlias(), userId.toString().c_str());
        return true;
/*        gSocketManager.CloseSocket(u->SocketDesc(s->Channel()), false);
        u->ClearSocketDesc(s->Channel());*/
      }
      u->SetSocketDesc(s);
    }
  }
  else
  {
    gLog.info(tr("%sConnection from new user (%s) [v%ld].\n"), L_TCPxSTR,
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


