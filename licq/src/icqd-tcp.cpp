// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998 - 2003 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <langinfo.h>

// Localization
#include "gettext.h"

#include "time-fix.h"

#include "licq_icqd.h"
#include "licq_translate.h"
#include "licq_packets.h"
#include "licq_socket.h"
#include "licq_user.h"
#include "licq_events.h"
#include "licq_log.h"
#include "licq_chat.h"
#include "licq_filetransfer.h"
#include "licq_gpg.h"
#include "support.h"
#include "licq_protoplugind.h"

//-----ICQ::sendMessage--------------------------------------------------------
unsigned long CICQDaemon::ProtoSendMessage(const char *_szId, unsigned long _nPPID,
   const char *m, bool online, unsigned short nLevel, bool bMultipleRecipients,
   CICQColor *pColor, unsigned long nCID)
{
  unsigned long nRet = 0;

  
  if (_nPPID == LICQ_PPID)
    nRet = icqSendMessage(_szId, m, online, nLevel, bMultipleRecipients, pColor);
  else
    PushProtoSignal(new CSendMessageSignal(_szId, m, nCID), _nPPID);

  
  return nRet;
}

unsigned long CICQDaemon::icqSendMessage(const char *szId, const char *m,
   bool online, unsigned short nLevel, bool bMultipleRecipients,
   CICQColor *pColor)
{
  if (m == NULL) return 0;

  ICQEvent *result = NULL;
  char *mDos = NULL;
  char *szMessage = NULL;
  bool bUTF16 = false;
  bool bUserOffline = true;
  if (m != NULL)
  {
    mDos = gTranslator.NToRN(m);
    gTranslator.ClientToServer(mDos);
  }
  CEventMsg *e = NULL;

  unsigned long f = INT_VERSION;

  ICQUser *u;
  char *cipher = NULL;
  u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_R);
  if (u)
  {
    bUserOffline = u->StatusOffline();
    bool useGpg = u->UseGPG();
    gUserManager.DropUser(u);
    if (useGpg && !bUserOffline)
      cipher = gGPGHelper.Encrypt(mDos, szId, LICQ_PPID);
  }

  if (cipher) f |= E_ENCRYPTED;
  if (online) f |= E_DIRECT;
  if (nLevel == ICQ_TCPxMSG_URGENT) f |= E_URGENT;
  if (bMultipleRecipients) f |= E_MULTIxREC;

  // What kinda encoding do we have here?
  unsigned short nCharset = CHARSET_ASCII;
  size_t nUTFLen = 0;
  char *szFromEncoding = 0;
  
  szMessage =  cipher ? cipher : mDos;
     
  if (!online) // send offline
  { 
    if (!bUserOffline && cipher == 0)
    {
      if (!gTranslator.isAscii(mDos))
      {
        u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_R);
        if (u && u->UserEncoding())
          szFromEncoding = strdup(u->UserEncoding());

        if (u && isdigit(u->IdString()[0]))
        {
          // ICQ Users can send a flag that says UTF8/16 is ok
          if (u->SupportsUTF8())
            nCharset = CHARSET_UNICODE;
          else if (u->UserEncoding())
            nCharset = CHARSET_CUSTOM;
        }
        else if(u && !(isdigit(u->IdString()[0])))
        {
          // AIM users support UTF8/16
          nCharset = CHARSET_UNICODE;
        }

        gUserManager.DropUser(u);
      }

      if (nCharset == CHARSET_UNICODE)
      {
        bUTF16 = true;
        if (!szFromEncoding || strlen(szFromEncoding) == 0)
        {
          free(szFromEncoding);
          szFromEncoding = strdup(nl_langinfo(CODESET));
        }
        szMessage = gTranslator.ToUTF16(mDos, szFromEncoding, nUTFLen);
        free(szFromEncoding);
      }
    }

     e = new CEventMsg(m, ICQ_CMDxSND_THRUxSERVER, TIME_NOW, f);
     unsigned short nMaxSize = bUserOffline ? MAX_OFFLINE_MESSAGE_SIZE : MAX_MESSAGE_SIZE;
     if (strlen(szMessage) > nMaxSize)
     {
       gLog.Warn(tr("%sTruncating message to %d characters to send through server.\n"),
                 L_WARNxSTR, nMaxSize);
       szMessage[nMaxSize] = '\0';
     }
     result = icqSendThroughServer(szId, ICQ_CMDxSUB_MSG | (bMultipleRecipients ? ICQ_CMDxSUB_FxMULTIREC : 0),
                                   cipher ? cipher : szMessage, e, nCharset, nUTFLen);
     u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
  }
  else        // send direct
  {
    u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
    if (u == NULL) return 0;
    if (u->Secure()) f |= E_ENCRYPTED;
    e = new CEventMsg(m, ICQ_CMDxTCP_START, TIME_NOW, f);
    if (pColor != NULL) e->SetColor(pColor);
    CPT_Message *p = new CPT_Message(cipher ? cipher : szMessage, nLevel, bMultipleRecipients, pColor, u, nUTFLen);
    gLog.Info(tr("%sSending %smessage to %s (#%hu).\n"), L_TCPxSTR,
       nLevel == ICQ_TCPxMSG_URGENT ? tr("urgent ") : "",
       u->GetAlias(), -p->Sequence());
    result = SendExpectEvent_Client(u, p, e);
  }

  
  
  if (u != NULL)
  {
    u->SetSendServer(!online);
    u->SetSendLevel(nLevel);
    gUserManager.DropUser(u);
  }

  if (pColor != NULL) CICQColor::SetDefaultColors(pColor);

  if (bUTF16 && szMessage)
    delete [] szMessage;
  if (cipher)
    free(cipher);
  if (mDos)
    delete [] mDos;
    
  if (result != NULL)
    return result->EventId();
  else
    return 0;
}

unsigned long CICQDaemon::icqSendMessage(unsigned long _nUin, const char *m,
   bool online, unsigned short nLevel, bool bMultipleRecipients,
   CICQColor *pColor)
{
  char szId[13];
  snprintf(szId, 12, "%lu", _nUin);
  szId[12] = 0;
  return icqSendMessage(szId, m, online, nLevel, bMultipleRecipients, pColor);
}


//-----CICQDaemon::icqFetchAutoResponse (deprecated!)---------------------------
unsigned long CICQDaemon::icqFetchAutoResponse(unsigned long /* nUin */, bool /* bServer */)
{
  return icqFetchAutoResponse(gUserManager.OwnerId(LICQ_PPID).c_str(), LICQ_PPID);
}

//-----CICQDaemon::icqFetchAutoResponse-----------------------------------------
unsigned long CICQDaemon::icqFetchAutoResponse(const char *_szId, unsigned long _nPPID, bool bServer)
{
  if (_szId == gUserManager.OwnerId(LICQ_PPID))
    return 0;

  if (isalpha(_szId[0]))
    return icqFetchAutoResponseServer(_szId);

  ICQEvent *result;
  ICQUser *u = gUserManager.FetchUser(_szId, _nPPID, LOCK_W);

  if (bServer)
  {
    // Generic read, gets changed in constructor
    CSrvPacketTcp *s = new CPU_AdvancedMessage(u,
                        ICQ_CMDxTCP_READxAWAYxMSG, 0, false, 0, 0, 0);
    gLog.Info(tr("%sRequesting auto response from %s.\n"), L_SRVxSTR,
              u->GetAlias());
    result = SendExpectEvent_Server(_szId, _nPPID, s, NULL);
  }
  else
  {
    CPT_ReadAwayMessage *p = new CPT_ReadAwayMessage(u);
    gLog.Info(tr("%sRequesting auto response from %s (#%hu).\n"), L_TCPxSTR,
	      u->GetAlias(), -p->Sequence());
    result = SendExpectEvent_Client(u, p, NULL);
  }

  gUserManager.DropUser(u);
  if (result != NULL)
    return result->EventId();
  return 0;
}


//-----CICQDaemon::sendUrl-----------------------------------------------------
unsigned long CICQDaemon::ProtoSendUrl(const char *_szId, unsigned long _nPPID,
   const char *url, const char *description, bool online, unsigned short nLevel,
   bool bMultipleRecipients, CICQColor *pColor)
{
  unsigned long nRet = 0;

  if (_nPPID == LICQ_PPID)
    nRet = icqSendUrl(_szId, url, description, online,
      nLevel, bMultipleRecipients, pColor);
  else
  {

  }

  return nRet;
}

unsigned long CICQDaemon::icqSendUrl(const char *_szId, const char *url,
   const char *description, bool online, unsigned short nLevel,
   bool bMultipleRecipients, CICQColor *pColor)
{
  const ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
  if (o != NULL)
  {
    bool isOwner = false;
    if (strcmp(o->IdString(), _szId) == 0)
      isOwner = true;
    gUserManager.DropOwner(o);
    if (isOwner)
      return 0;
  }

  // make the URL info string
  char *szDescDos = NULL;
  CEventUrl *e = NULL;
  szDescDos = gTranslator.NToRN(description);
  gTranslator.ClientToServer(szDescDos);
  int n = strlen_safe(url) + strlen_safe(szDescDos) + 2;
  char m[n];
  if (!online && n > MAX_MESSAGE_SIZE && szDescDos != NULL)
    szDescDos[MAX_MESSAGE_SIZE - strlen_safe(url) - 2] = '\0';
  sprintf(m, "%s%c%s", szDescDos == NULL ? "" : szDescDos, char(0xFE), url == NULL ? "" : url);

  ICQEvent *result = NULL;

  unsigned long f = INT_VERSION;
  if (online) f |= E_DIRECT;
  if (nLevel == ICQ_TCPxMSG_URGENT) f |= E_URGENT;
  if (bMultipleRecipients) f |= E_MULTIxREC;

  ICQUser *u;
  if (!online) // send offline
  {
    unsigned short nCharset = 0;
    u = gUserManager.FetchUser(_szId, LICQ_PPID, LOCK_R);
    if (u && u->UserEncoding())
      nCharset = 3;
    gUserManager.DropUser(u);

    e = new CEventUrl(url, description, ICQ_CMDxSND_THRUxSERVER, TIME_NOW, f);
    result = icqSendThroughServer(_szId, ICQ_CMDxSUB_URL | (bMultipleRecipients ? ICQ_CMDxSUB_FxMULTIREC : 0), m, e, nCharset);
    u = gUserManager.FetchUser(_szId, LICQ_PPID, LOCK_W);
  }
  else
  {
    u = gUserManager.FetchUser(_szId, LICQ_PPID, LOCK_W);
    if (u == NULL) return 0;
    if (u->Secure()) f |= E_ENCRYPTED;
    e = new CEventUrl(url, description, ICQ_CMDxTCP_START, TIME_NOW, f);
    if (pColor != NULL) e->SetColor(pColor);
    CPT_Url *p = new CPT_Url(m, nLevel, bMultipleRecipients, pColor, u);
    gLog.Info(tr("%sSending %sURL to %s (#%hu).\n"), L_TCPxSTR,
       nLevel == ICQ_TCPxMSG_URGENT ? tr("urgent ") : "",
       u->GetAlias(), -p->Sequence());
    result = SendExpectEvent_Client(u, p, e);
  }
  if (u != NULL)
  {
    u->SetSendServer(!online);
    u->SetSendLevel(nLevel);
    gUserManager.DropUser(u);
  }

  if (pColor != NULL) CICQColor::SetDefaultColors(pColor);

  if (szDescDos)
    delete [] szDescDos;

  if (result != NULL)
    return result->EventId();
  return 0;
}


//-----CICQDaemon::sendFile---------------------------------------------------
unsigned long CICQDaemon::ProtoFileTransfer(const char *szId, unsigned long nPPID,
  const char *szFilename, const char *szDescription, ConstFileList &lFileList,
  unsigned short nLevel, bool bServer)
{
  unsigned long nRet = 0;
  if (nPPID == LICQ_PPID)
    nRet = icqFileTransfer(szId, szFilename, szDescription, lFileList, nLevel,
      bServer);
  else
    PushProtoSignal(new CSendFileSignal(szId, szFilename, szDescription,
      lFileList), nPPID);

  return nRet;
}

unsigned long CICQDaemon::icqFileTransfer(const char *szId, const char *szFilename,
                        const char *szDescription, ConstFileList &lFileList,
                        unsigned short nLevel, bool bServer)
{
  if (gUserManager.FindOwner(szId, LICQ_PPID)) return 0;

  ICQEvent *result = NULL;
  char *szDosDesc = NULL;
  if (szDescription != NULL)
  {
    szDosDesc = gTranslator.NToRN(szDescription);
    gTranslator.ClientToServer(szDosDesc);
  }
  CEventFile *e = NULL;

  ICQUser *u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
  if (u == NULL) return 0;

  if (bServer)
  {
    unsigned long f = INT_VERSION;
    //flags through server are a little different
    if (nLevel == ICQ_TCPxMSG_NORMAL)
      nLevel = ICQ_TCPxMSG_NORMAL2;
    else if (nLevel == ICQ_TCPxMSG_URGENT)
    {
      f |= E_URGENT;
      nLevel = ICQ_TCPxMSG_URGENT2;
    }
    else if (nLevel == ICQ_TCPxMSG_LIST)
      nLevel = ICQ_TCPxMSG_LIST2;

    CPU_FileTransfer *p = new CPU_FileTransfer(u, lFileList, szFilename,
      szDosDesc, nLevel, (u->Version() > 7));

    if (!p->IsValid())
    {
      delete p;
      result = NULL;
    }
    else
    {
      e = new CEventFile(szFilename, p->GetDescription(), p->GetFileSize(),
                lFileList, p->Sequence(), TIME_NOW, f);
      gLog.Info(tr("%sSending file transfer to %s (#%hu).\n"), L_SRVxSTR, 
                u->GetAlias(), -p->Sequence());

      result = SendExpectEvent_Server(szId, LICQ_PPID, p, e);
    }
  }
  else
  {
    CPT_FileTransfer *p = new CPT_FileTransfer(lFileList, szFilename, szDosDesc, 
      nLevel, u);

    if (!p->IsValid())
    {
      delete p;
      result = NULL;
    }
    else
    {
      unsigned long f = E_DIRECT | INT_VERSION;
      if (nLevel == ICQ_TCPxMSG_URGENT) f |= E_URGENT;
      if (u->Secure()) f |= E_ENCRYPTED;

      e = new CEventFile(szFilename, p->GetDescription(), p->GetFileSize(),
                         lFileList, p->Sequence(), TIME_NOW, f);
      gLog.Info(tr("%sSending %sfile transfer to %s (#%hu).\n"), L_TCPxSTR,
                nLevel == ICQ_TCPxMSG_URGENT ? tr("urgent ") : "", 
                u->GetAlias(), -p->Sequence());

      result = SendExpectEvent_Client(u, p, e);
    }
  }

  u->SetSendServer(bServer);
  u->SetSendLevel(nLevel);
  gUserManager.DropUser(u);

  if (szDosDesc)
    delete [] szDosDesc;

  if (result)
    return result->EventId();
  else
    return 0;
}

unsigned long CICQDaemon::icqFileTransfer(unsigned long nUin, const char *szFilename,
                        const char *szDescription, ConstFileList &lFileList,
                        unsigned short nLevel, bool bServer)
{
  char szUin[13];
  snprintf(szUin, 12, "%lu", nUin);
  szUin[12] = '\0';
  
  return icqFileTransfer(szUin, szFilename, szDescription, lFileList,
    nLevel, bServer);
}

//-----CICQDaemon::sendContactList-------------------------------------------
unsigned long CICQDaemon::icqSendContactList(const char *szId,
   const StringList& users, bool online, unsigned short nLevel,
   bool bMultipleRecipients, CICQColor *pColor)
{
  if (gUserManager.FindOwner(szId, LICQ_PPID) != NULL) return 0;

  char *m = new char[3 + users.size() * 80];
  int p = sprintf(m, "%d%c", int(users.size()), char(0xFE));
  ContactList vc;

  StringList::const_iterator iter;
  for (iter = users.begin(); iter != users.end(); ++iter)
  {
    const ICQUser* u = gUserManager.FetchUser(iter->c_str(), LICQ_PPID, LOCK_R);
    p += sprintf(&m[p], "%s%c%s%c", iter->c_str(), char(0xFE),
       u == NULL ? "" : u->GetAlias(), char(0xFE));
    vc.push_back(new CContact(iter->c_str(), LICQ_PPID, u == NULL ? "" : u->GetAlias()));
    gUserManager.DropUser(u);
  }

  if (!online && p > MAX_MESSAGE_SIZE)
  {
    gLog.Warn(tr("%sContact list too large to send through server.\n"), L_WARNxSTR);
    delete []m;
    return 0;
  }

  CEventContactList *e = NULL;
  ICQEvent *result = NULL;

  unsigned long f = INT_VERSION;
  if (online) f |= E_DIRECT;
  if (nLevel == ICQ_TCPxMSG_URGENT) f |= E_URGENT;
  if (bMultipleRecipients) f |= E_MULTIxREC;

  ICQUser* u;
  if (!online) // send offline
  {
    e = new CEventContactList(vc, false, ICQ_CMDxSND_THRUxSERVER, TIME_NOW, f);
    result = icqSendThroughServer(szId,
      ICQ_CMDxSUB_CONTACTxLIST | (bMultipleRecipients ? ICQ_CMDxSUB_FxMULTIREC : 0),
      m, e);
    u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
  }
  else
  {
    u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
    if (u == NULL) return 0;
    if (u->Secure()) f |= E_ENCRYPTED;
    e = new CEventContactList(vc, false, ICQ_CMDxTCP_START, TIME_NOW, f);
    if (pColor != NULL) e->SetColor(pColor);
    CPT_ContactList *p = new CPT_ContactList(m, nLevel, bMultipleRecipients, pColor, u);
    gLog.Info(tr("%sSending %scontact list to %s (#%hu).\n"), L_TCPxSTR,
       nLevel == ICQ_TCPxMSG_URGENT ? tr("urgent ") : "",
       u->GetAlias(), -p->Sequence());
    result = SendExpectEvent_Client(u, p, e);
  }
  if (u != NULL)
  {
    u->SetSendServer(!online);
    u->SetSendLevel(nLevel);
    gUserManager.DropUser(u);
  }

  if (pColor != NULL) CICQColor::SetDefaultColors(pColor);

  delete []m;
  if (result != NULL)
    return result->EventId();
  return 0;
}

unsigned long CICQDaemon::icqSendContactList(unsigned long nUin,
   UinList &uins, bool online, unsigned short nLevel, bool bMultipleRecipients,
   CICQColor *pColor)
{
  StringList users;
  char szUin[24];

  UinList::iterator it;
  for (it = uins.begin(); it != uins.end(); ++it)
  {
    sprintf(szUin, "%lu", *it);
    users.push_back(szUin);
  }

  sprintf(szUin, "%lu", nUin);
  return icqSendContactList(szUin, users,online, nLevel, bMultipleRecipients,
    pColor);
}

//-----CICQDaemon::sendInfoPluginReq--------------------------------------------
unsigned long CICQDaemon::icqRequestInfoPlugin(ICQUser *u, bool bServer,
                                               const char *GUID)
{
  ICQEvent *result = NULL;
  if (bServer)
  {
    CPU_InfoPluginReq *p = new CPU_InfoPluginReq(u, GUID, 0);
    result = SendExpectEvent_Server(u->IdString(), u->PPID(), p, NULL);
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
unsigned long CICQDaemon::icqRequestInfoPluginList(const char *szId,
                                                   bool bServer)
{
  if (gUserManager.FindOwner(szId, LICQ_PPID)) return 0;

  ICQUser *u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
  if (u == NULL) return 0;

  if (bServer)
    gLog.Info("%sRequesting info plugin list from %s through server.\n",
              L_SRVxSTR, u->GetAlias());
  else
    gLog.Info("%sRequesting info plugin list from %s.\n", L_TCPxSTR,
              u->GetAlias());

  unsigned long result = icqRequestInfoPlugin(u, bServer, PLUGIN_QUERYxINFO);

  gUserManager.DropUser(u);

  return result;
}

//-----CICQDaemon::sendPhoneBookReq--------------------------------------------
unsigned long CICQDaemon::icqRequestPhoneBook(const char *szId,
                                              bool bServer)
{
  if (gUserManager.FindOwner(szId, LICQ_PPID)) return 0;

  ICQUser *u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
  if (u == NULL) return 0;

  if (bServer)
    gLog.Info("%sRequesting Phone Book from %s through server.\n",
              L_SRVxSTR, u->GetAlias());
  else
    gLog.Info("%sRequesting Phone Book from %s.\n", L_TCPxSTR, u->GetAlias());

  unsigned long result = icqRequestInfoPlugin(u, bServer, PLUGIN_PHONExBOOK);

  gUserManager.DropUser(u);

  return result;
}

//-----CICQDaemon::sendPictureReq-----------------------------------------------
unsigned long CICQDaemon::icqRequestPicture(const char *szId, bool bServer)
{
  if (gUserManager.FindOwner(szId, LICQ_PPID)) return 0;

  ICQUser *u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
  if (u == NULL) return 0;

  if (bServer)
    gLog.Info("%sRequesting Picture from %s through server.\n",
              L_SRVxSTR, u->GetAlias());
  else
    gLog.Info("%sRequesting Picture from %s.\n", L_TCPxSTR, u->GetAlias());

  unsigned long result = icqRequestInfoPlugin(u, bServer, PLUGIN_PICTURE);

  gUserManager.DropUser(u);

  return result;
}

//-----CICQDaemon::sendStatusPluginReq------------------------------------------
unsigned long CICQDaemon::icqRequestStatusPlugin(ICQUser *u, bool bServer,
                                                 const char *GUID)
{
  ICQEvent *result = NULL;
  if (bServer)
  {
    CPU_StatusPluginReq *p = new CPU_StatusPluginReq(u, GUID, 0);
    result = SendExpectEvent_Server(u->IdString(), u->PPID(), p, NULL);
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
unsigned long CICQDaemon::icqRequestStatusPluginList(const char *szId,
                                                     bool bServer)
{
  if (gUserManager.FindOwner(szId, LICQ_PPID)) return 0;

  ICQUser *u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
  if (u == NULL) return 0;

  if (bServer)
    gLog.Info("%sRequesting status plugin list from %s through server.\n",
              L_SRVxSTR, u->GetAlias());
  else
    gLog.Info("%sRequesting status plugin list from %s.\n", L_TCPxSTR,
              u->GetAlias());

  unsigned long result = icqRequestStatusPlugin(u, bServer,
                                                PLUGIN_QUERYxSTATUS);

  gUserManager.DropUser(u);

  return result;
}

//-----CICQDaemon::sendSharedFilesReq--------------------------------------
unsigned long CICQDaemon::icqRequestSharedFiles(const char *szId,
                                                bool bServer)
{
  if (gUserManager.FindOwner(szId, LICQ_PPID)) return 0;

  ICQUser *u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
  if (u == NULL) return 0;

  if (bServer)
    gLog.Info("%sRequesting file server status from %s through server.\n",
              L_SRVxSTR, u->GetAlias());
  else
    gLog.Info("%sRequesting file server status from %s.\n", L_TCPxSTR,
              u->GetAlias());

  unsigned long result = icqRequestStatusPlugin(u, bServer, PLUGIN_FILExSERVER);

  gUserManager.DropUser(u);

  return result;
}

//-----CICQDaemon::sendPhoneFollowMeReq--------------------------------------
unsigned long CICQDaemon::icqRequestPhoneFollowMe(const char *szId,
                                                  bool bServer)
{
  if (gUserManager.FindOwner(szId, LICQ_PPID)) return 0;

  ICQUser *u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
  if (u == NULL) return 0;

  if (bServer)
    gLog.Info("%sRequesting Phone \"Follow Me\" status from %s through"
              " server.\n", L_SRVxSTR, u->GetAlias());
  else
    gLog.Info("%sRequesting Phone \"Follow Me\" status from %s.\n", L_TCPxSTR,
              u->GetAlias());

  unsigned long result = icqRequestStatusPlugin(u, bServer, PLUGIN_FOLLOWxME);

  gUserManager.DropUser(u);

  return result;
}

//-----CICQDaemon::sendICQphoneReq--------------------------------------
unsigned long CICQDaemon::icqRequestICQphone(const char *szId,
                                             bool bServer)
{
  if (gUserManager.FindOwner(szId, LICQ_PPID)) return 0;

  ICQUser *u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
  if (u == NULL) return 0;

  if (bServer)
    gLog.Info("%sRequesting ICQphone status from %s through server.\n",
              L_SRVxSTR, u->GetAlias());
  else
    gLog.Info("%sRequesting ICQphone status from %s.\n", L_TCPxSTR,
              u->GetAlias());

  unsigned long result = icqRequestStatusPlugin(u, bServer, PLUGIN_FILExSERVER);

  gUserManager.DropUser(u);

  return result;
}

//-----CICQDaemon::fileCancel-------------------------------------------------------------------------
void CICQDaemon::ProtoFileTransferCancel(const char *szId, unsigned long nPPID,
  unsigned long nFlag)
{
  if (nPPID == LICQ_PPID)
    icqFileTransferCancel(szId, (unsigned short)nFlag);
  else
    PushProtoSignal(new CCancelEventSignal(szId, nFlag), nPPID);
}

void CICQDaemon::icqFileTransferCancel(const char *szId, unsigned short nSequence)
{
  // add to history ??
  ICQUser* u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
  if (u == NULL) return;
  gLog.Info(tr("%sCancelling file transfer to %s (#%hu).\n"), L_TCPxSTR, 
            u->GetAlias(), -nSequence);
  CPT_CancelFile p(nSequence, u);
  AckTCP(p, u->SocketDesc(ICQ_CHNxNONE));
  gUserManager.DropUser(u);
}

void CICQDaemon::icqFileTransferCancel(unsigned long nUin, unsigned short nSequence)
{
  char szUin[13];
  snprintf(szUin, 12, "%lu", nUin);
  szUin[12] = '\0'; 

  icqFileTransferCancel(szUin, nSequence);
}


//-----CICQDaemon::fileAccept-----------------------------------------------------------------------------
void CICQDaemon::ProtoFileTransferAccept(const char *szId, unsigned long nPPID,
   unsigned short nPort, unsigned long nSequence, unsigned long nFlag1,
   unsigned long nFlag2, const char *szDesc, const char *szFile,
   unsigned long nFileSize, bool bDirect)
{
  if (nPPID == LICQ_PPID)
  {
    unsigned long nMsgId[] = { nFlag1, nFlag2 };
    icqFileTransferAccept(szId, nPort, (unsigned short)nSequence,
      nMsgId, bDirect, szDesc, szFile, nFileSize);
  }
  else
    PushProtoSignal(new CSendEventReplySignal(szId, 0, true, nPort, nSequence,
      nFlag1, nFlag2, bDirect), nPPID);
}

void CICQDaemon::icqFileTransferAccept(const char *szId, unsigned short nPort,
    unsigned short nSequence, const unsigned long nMsgID[2], bool bDirect,
   const char *szDesc, const char *szFile, unsigned long nFileSize)
{
   // basically a fancy tcp ack packet which is sent late
  ICQUser* u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
  if (u == NULL) return;
  gLog.Info(tr("%sAccepting file transfer from %s (#%hu).\n"),
    bDirect ? L_TCPxSTR : L_SRVxSTR, u->GetAlias(), -nSequence);
  if (bDirect)
  {
    CPT_AckFileAccept p(nPort, nSequence, u);
    AckTCP(p, u->SocketDesc(ICQ_CHNxNONE));
  }
  else
  {
    CPU_AckFileAccept *p = new CPU_AckFileAccept(u, nMsgID,
      nSequence, nPort, szDesc, szFile, nFileSize);
    SendEvent_Server(p);
  }

  gUserManager.DropUser(u);
}
  
void CICQDaemon::icqFileTransferAccept(unsigned long nUin, unsigned short nPort,
    unsigned short nSequence, const unsigned long nMsgID[2], bool bDirect,
   const char *szDesc, const char *szFile, unsigned long nFileSize)
{
  char szUin[13];
  snprintf(szUin, 12, "%lu", nUin);
  szUin[12] = '\0'; 
  
  icqFileTransferAccept(szUin, nPort, nSequence, nMsgID, bDirect,
    szDesc, szFile, nFileSize);
}



//-----CICQDaemon::fileRefuse-----------------------------------------------------------------------------
void CICQDaemon::ProtoFileTransferRefuse(const char *szId, unsigned long nPPID,
     const char *szReason, unsigned long nSequence, unsigned long nFlag1,
     unsigned long nFlag2, bool bDirect)
{
  if (nPPID == LICQ_PPID)
  {
    unsigned long nMsgId[] = { nFlag1, nFlag2 };
    icqFileTransferRefuse(szId, szReason, (unsigned short)nSequence,
      nMsgId, bDirect);
  }
  else
    PushProtoSignal(new CSendEventReplySignal(szId, szReason, false, nSequence,
      nFlag1, nFlag2, bDirect), nPPID);
}

void CICQDaemon::icqFileTransferRefuse(const char *szId, const char *szReason,
    unsigned short nSequence, const unsigned long nMsgID[2], bool bDirect)
{
   // add to history ??
  char *szReasonDos = gTranslator.NToRN(szReason);
  gTranslator.ClientToServer(szReasonDos);
  ICQUser* u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
  if (u == NULL) return;
  gLog.Info(tr("%sRefusing file transfer from %s (#%hu).\n"), 
            bDirect ? L_TCPxSTR : L_SRVxSTR, u->GetAlias(), -nSequence);

  if (bDirect)
  {
    CPT_AckFileRefuse p(szReasonDos, nSequence, u);
    AckTCP(p, u->SocketDesc(ICQ_CHNxNONE));
  }
  else
  {
    CPU_AckFileRefuse *p = new CPU_AckFileRefuse(u, nMsgID, nSequence,
      szReasonDos);
    SendEvent_Server(p);
  }

  gUserManager.DropUser(u);

  if (szReasonDos)
    delete [] szReasonDos;
}

void CICQDaemon::icqFileTransferRefuse(unsigned long nUin, const char *szReason,
    unsigned short nSequence, const unsigned long nMsgID[2], bool bDirect)
{
  char szUin[13];
  snprintf(szUin, 12, "%lu", nUin);
  szUin[12] = '\0'; 

  icqFileTransferRefuse(szUin, szReason, nSequence, nMsgID, bDirect);
}


//-----CICQDaemon::sendChat------------------------------------------------------------
unsigned long CICQDaemon::icqChatRequest(unsigned long nUin, const char *szReason,
                                         unsigned short nLevel, bool bServer)
{
  char id[16];
  snprintf(id, 16, "%lu", nUin);
  return icqMultiPartyChatRequest(id, szReason, NULL, 0, nLevel, bServer);
}

unsigned long CICQDaemon::icqChatRequest(const char* id, const char *szReason,
                                         unsigned short nLevel, bool bServer)
{
  return icqMultiPartyChatRequest(id, szReason, NULL, 0, nLevel, bServer);
}


unsigned long CICQDaemon::icqMultiPartyChatRequest(unsigned long nUin,
   const char *reason, const char *szChatUsers, unsigned short nPort,
   unsigned short nLevel, bool bServer)
{
  char id[16];
  snprintf(id, 16, "%lu", nUin);
  return icqMultiPartyChatRequest(id, reason, szChatUsers, nPort, nLevel, bServer);
}

unsigned long CICQDaemon::icqMultiPartyChatRequest(const char* id,
   const char *reason, const char *szChatUsers, unsigned short nPort,
   unsigned short nLevel, bool bServer)
{
  if (gUserManager.FindOwner(id, LICQ_PPID) != NULL)
    return 0;

  ICQUser* u = gUserManager.FetchUser(id, LICQ_PPID, LOCK_W);
  if (u == NULL) return 0;
  char *szReasonDos = gTranslator.NToRN(reason);
  gTranslator.ClientToServer(szReasonDos);

  unsigned long f;
  ICQEvent *result = NULL;
  if (bServer)
  {
    f = INT_VERSION;
    
    //flags through server are a little different
    if (nLevel == ICQ_TCPxMSG_NORMAL)
      nLevel = ICQ_TCPxMSG_NORMAL2;
    else if (nLevel == ICQ_TCPxMSG_URGENT)
    {
      f |= E_URGENT;
      nLevel = ICQ_TCPxMSG_URGENT2;
    }
    else if (nLevel == ICQ_TCPxMSG_LIST)
      nLevel = ICQ_TCPxMSG_LIST2;

		CPU_ChatRequest *p = new CPU_ChatRequest(szReasonDos,
                               szChatUsers, nPort, nLevel, u,
                               (u->Version() > 7));

		CEventChat *e = new CEventChat(reason, szChatUsers, nPort, p->Sequence(),
																	 TIME_NOW, f);
		gLog.Info(tr("%sSending chat request to %s (#%hu).\n"), L_SRVxSTR,
			  u->GetAlias(), -p->Sequence());

		result = SendExpectEvent_Server(u->IdString(), u->PPID(), p, e);
	}
	else
	{
		CPT_ChatRequest *p = new CPT_ChatRequest(szReasonDos, szChatUsers, nPort,
						 nLevel, u, (u->Version() > 7));
		f = E_DIRECT | INT_VERSION;
		if (nLevel == ICQ_TCPxMSG_URGENT) f |= E_URGENT;
		if (u->Secure()) f |= E_ENCRYPTED;
		CEventChat *e = new CEventChat(reason, szChatUsers, nPort, p->Sequence(),
																	 TIME_NOW, f);
		gLog.Info(tr("%sSending %schat request to %s (#%hu).\n"), L_TCPxSTR,
							nLevel == ICQ_TCPxMSG_URGENT ? tr("urgent ") : "",
							u->GetAlias(), -p->Sequence());
		result = SendExpectEvent_Client(u, p, e);
	}
	
	u->SetSendServer(bServer);
  u->SetSendLevel(nLevel);
  gUserManager.DropUser(u);

  if (szReasonDos)
    delete [] szReasonDos;
  if (result != NULL)
    return result->EventId();
  return 0;
}


//-----CICQDaemon::chatCancel----------------------------------------------------------
void CICQDaemon::icqChatRequestCancel(unsigned long nUin, unsigned short nSequence)
{
  char id[16];
  snprintf(id, 16, "%lu", nUin);
  icqChatRequestCancel(id, nSequence);
}

void CICQDaemon::icqChatRequestCancel(const char* id, unsigned short nSequence)
{
  ICQUser* u = gUserManager.FetchUser(id, LICQ_PPID, LOCK_W);
  if (u == NULL) return;
  gLog.Info(tr("%sCancelling chat request with %s (#%hu).\n"), L_TCPxSTR, 
            u->GetAlias(), -nSequence);
  CPT_CancelChat p(nSequence, u);
  AckTCP(p, u->SocketDesc(ICQ_CHNxNONE));
  gUserManager.DropUser(u);
}


//-----CICQDaemon::chatRefuse-----------------------------------------------------------------------------
void CICQDaemon::icqChatRequestRefuse(unsigned long nUin, const char *szReason,
    unsigned short nSequence, const unsigned long nMsgID[2], bool bDirect)
{
  char id[16];
  snprintf(id, 16, "%lu", nUin);
  icqChatRequestRefuse(id, szReason, nSequence, nMsgID, bDirect);
}

void CICQDaemon::icqChatRequestRefuse(const char* id, const char *szReason,
    unsigned short nSequence, const unsigned long nMsgID[2], bool bDirect)
{
  // add to history ??
  ICQUser* u = gUserManager.FetchUser(id, LICQ_PPID, LOCK_W);
  if (u == NULL) return;
  gLog.Info(tr("%sRefusing chat request with %s (#%hu).\n"), 
            bDirect ? L_TCPxSTR : L_SRVxSTR, u->GetAlias(), -nSequence);
  char *szReasonDos = gTranslator.NToRN(szReason);
  gTranslator.ClientToServer(szReasonDos);

	if (bDirect)
	{
		CPT_AckChatRefuse p(szReasonDos, nSequence, u);
		AckTCP(p, u->SocketDesc(ICQ_CHNxNONE));
	}
	else
	{
		CPU_AckChatRefuse *p = new CPU_AckChatRefuse(u, nMsgID, nSequence,
																								 szReasonDos);
		SendEvent_Server(p);
	}

  gUserManager.DropUser(u);

  if (szReasonDos)
    delete [] szReasonDos;
}


//-----CICQDaemon::chatAccept-----------------------------------------------------------------------------
void CICQDaemon::icqChatRequestAccept(unsigned long nUin, unsigned short nPort,
    const char* szClients, unsigned short nSequence,
    const unsigned long nMsgID[2], bool bDirect)
{
  char id[16];
  snprintf(id, 16, "%lu", nUin);
  icqChatRequestAccept(id, nPort, szClients, nSequence, nMsgID, bDirect);
}

void CICQDaemon::icqChatRequestAccept(const char* id, unsigned short nPort,
    const char* szClients, unsigned short nSequence,
    const unsigned long nMsgID[2], bool bDirect)
{
  // basically a fancy tcp ack packet which is sent late
  // add to history ??
  ICQUser* u = gUserManager.FetchUser(id, LICQ_PPID, LOCK_W);
  if (u == NULL) return;
  gLog.Info(tr("%sAccepting chat request with %s (#%hu).\n"), 
            bDirect ? L_TCPxSTR : L_SRVxSTR, u->GetAlias(), -nSequence);

	if (bDirect)
	{
		CPT_AckChatAccept p(nPort, szClients, nSequence, u, u->Version() > 7);
		AckTCP(p, u->SocketDesc(ICQ_CHNxNONE));
	}
	else
	{
		CPU_AckChatAccept *p = new CPU_AckChatAccept(u, szClients, nMsgID, nSequence,
																								 nPort);
		SendEvent_Server(p);
	}

  gUserManager.DropUser(u);
}

/*---------------------------------------------------------------------------
 * OpenSSL stuff
 *-------------------------------------------------------------------------*/

unsigned long CICQDaemon::ProtoOpenSecureChannel(const char *szId,
  unsigned long nPPID)
{
  unsigned long nRet = 0;
  if (nPPID == LICQ_PPID)
    nRet = icqOpenSecureChannel(szId);
  else
    PushProtoSignal(new COpenSecureSignal(szId), nPPID);
   
  return nRet;
}

unsigned long CICQDaemon::icqOpenSecureChannel(const char *szId)
{
  if (gUserManager.FindOwner(szId, LICQ_PPID)) return 0;

#ifdef USE_OPENSSL
  ICQEvent *result = NULL;

  ICQUser *u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
  if (u == NULL)
  {
    gLog.Warn(tr("%sCannot send secure channel request to user not on list (%s).\n"),
       L_WARNxSTR, szId);
    return 0;
  }

  // Check that the user doesn't already have a secure channel
  if (u->Secure())
  {
    gLog.Warn(tr("%s%s (%s) already has a secure channel.\n"), L_WARNxSTR,
       u->GetAlias(), szId);
    gUserManager.DropUser(u);
    return 0;
  }

  CPT_OpenSecureChannel *pkt = new CPT_OpenSecureChannel(u);
  gLog.Info(tr("%sSending request for secure channel to %s (#%hu).\n"), L_TCPxSTR,
            u->GetAlias(), -pkt->Sequence());
  result = SendExpectEvent_Client(u, pkt, NULL);

  u->SetSendServer(false);

  gUserManager.DropUser(u);

  if (result != NULL)
    return result->EventId();
  return 0;

#else // No OpenSSL
  gLog.Warn("%sicqOpenSecureChannel() to %s called when we do not support OpenSSL.\n",
     L_WARNxSTR, szId);
  return 0;

#endif
}

unsigned long CICQDaemon::icqOpenSecureChannel(unsigned long nUin)
{
  char szUin[13];
  snprintf(szUin, 12, "%lu", nUin);
  szUin[12] = '\0';

  return icqOpenSecureChannel(szUin);
}

unsigned long CICQDaemon::ProtoCloseSecureChannel(const char *szId,
  unsigned long nPPID)
{
  unsigned long nRet = 0;
  if (nPPID == LICQ_PPID)
    nRet = icqCloseSecureChannel(szId);
  else
    PushProtoSignal(new CCloseSecureSignal(szId), nPPID);
    
  return nRet;
}


unsigned long CICQDaemon::icqCloseSecureChannel(const char *szId)
{
#ifdef USE_OPENSSL
  ICQEvent *result = NULL;

  ICQUser *u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
  if (u == NULL)
  {
    gLog.Warn(tr("%sCannot send secure channel request to user not on list (%s).\n"),
       L_WARNxSTR, szId);
    return 0;
  }

  // Check that the user doesn't already have a secure channel
  if (!u->Secure())
  {
    gLog.Warn(tr("%s%s (%s) does not have a secure channel.\n"), L_WARNxSTR,
       u->GetAlias(), szId);
    gUserManager.DropUser(u);
    return 0;
  }

  CPT_CloseSecureChannel *pkt = new CPT_CloseSecureChannel(u);
  gLog.Info(tr("%sClosing secure channel with %s (#%hu).\n"), L_TCPxSTR,
            u->GetAlias(), -pkt->Sequence());
  result = SendExpectEvent_Client(u, pkt, NULL);

  u->SetSendServer(false);

  gUserManager.DropUser(u);

  if (result != NULL)
    return result->EventId();
  return 0;

#else // No OpenSSL
  gLog.Warn("%sicqCloseSecureChannel() to %s called when we do not support OpenSSL.\n",
     L_WARNxSTR, szId);
  return 0;

#endif
}

unsigned long CICQDaemon::icqCloseSecureChannel(unsigned long nUin)
{
  char szUin[13];
  snprintf(szUin, 12, "%lu", nUin);
  szUin[12] = '\0';
  
  return icqCloseSecureChannel(szUin);
}

//-----CICQDaemon::keyCancel-------------------------------------------------------------------------
void CICQDaemon::ProtoOpenSecureChannelCancel(const char *szId,
  unsigned long nPPID, unsigned long nSequence)
{
  if (nPPID == LICQ_PPID)
    icqOpenSecureChannelCancel(szId, (unsigned short)nSequence);
  else
    PushProtoSignal(new CCancelEventSignal(szId, nSequence), nPPID);
}


void CICQDaemon::icqOpenSecureChannelCancel(const char *szId,
  unsigned short nSequence)
{
  ICQUser *u = gUserManager.FetchUser(szId, LICQ_PPID, LOCK_W);
  if (u == NULL) return;
  gLog.Info(tr("%sCancelling secure channel request to %s (#%hu).\n"), L_TCPxSTR,
            u->GetAlias(), -nSequence);
  // XXX Tear down tcp connection ??
  gUserManager.DropUser(u);
}

void CICQDaemon::icqOpenSecureChannelCancel(unsigned long nUin,
  unsigned short nSequence)
{
  char szUin[13];
  snprintf(szUin, 12, "%lu", nUin);
  szUin[12] = '\0';
  
  icqOpenSecureChannelCancel(szUin, nSequence);  
}





/*---------------------------------------------------------------------------
 * Handshake
 *
 * Shake hands on the given socket with the given user.
 *-------------------------------------------------------------------------*/
bool CICQDaemon::Handshake_Send(TCPSocket *s, const char* id,
   unsigned short nPort, unsigned short nVersion, bool bConfirm,
   unsigned long nId)
{
  s->SetVersion(nVersion);
  s->SetOwner(id, LICQ_PPID);

  unsigned long nUin = strtoul(id, NULL, 10);

  switch (nVersion)
  {
    case 2:
    case 3:
    {
      CPacketTcp_Handshake_v2 p(s->LocalPort());
      if (!s->SendPacket(p.getBuffer())) goto sock_error;
      break;
    }
    case 4:
    case 5:
    {
      CPacketTcp_Handshake_v4 p(s->LocalPort());
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
        gLog.Warn(tr("%sBad handshake ack: %ld.\n"), L_WARNxSTR, nOk);
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
        gLog.Warn(tr("%sBad handshake session id: received %ld, expecting %ld.\n"),
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
        gLog.Warn(tr("%sBad handshake ack: %ld.\n"), L_WARNxSTR, nOk);
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
        gLog.Warn(tr("%sBad handshake cookie: received %ld, expecting %ld.\n"),
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
      gLog.Error("%sUnknown ICQ TCP version (%d).\n", L_ERRORxSTR, nVersion);
      return false;
  }

  return true;
  
sock_error:
  char buf[128];
  if (s->Error() == 0)
    gLog.Warn(tr("%sHandshake error, remote side closed connection.\n"), L_WARNxSTR);
  else
    gLog.Warn(tr("%sHandshake socket error:\n%s%s.\n"), L_WARNxSTR, L_BLANKxSTR, s->ErrorStr(buf, 128));
  return false;
}


/*------------------------------------------------------------------------------
 * ConnectToUser
 *
 * Creates a new TCPSocket and connects it to a given user.  Adds the socket
 * to the global socket manager and to the user.
 *----------------------------------------------------------------------------*/
int CICQDaemon::ConnectToUser(const char* id, unsigned char nChannel)
{
  ICQUser* u = gUserManager.FetchUser(id, LICQ_PPID, LOCK_W);
  if (u == NULL) return -1;

  int sd = u->SocketDesc(nChannel);

  // Check that we need to connect at all
  if (sd != -1)
  {
    gUserManager.DropUser(u);
    gLog.Warn(tr("%sConnection attempted to already connected user (%s).\n"),
       L_WARNxSTR, id);
    return sd;
  }

  char szAlias[64];
  strncpy(szAlias, u->GetAlias(), sizeof(szAlias));
  szAlias[sizeof(szAlias) - 1] = '\0';
  unsigned short nPort = u->Port();
  unsigned short nVersion = u->ConnectionVersion();

  // Poll if there is a connection in progress already
  while (u->ConnectionInProgress())
  {
    gUserManager.DropUser(u);
    struct timeval tv = { 2, 0 };
    if (select(0, NULL, NULL, NULL, &tv) == -1 && errno == EINTR) return -1;
    u = gUserManager.FetchUser(id, LICQ_PPID, LOCK_W);
    if (u == NULL) return -1;
  }
  sd = u->SocketDesc(ICQ_CHNxNONE);
  if (sd == -1) u->SetConnectionInProgress(true);
  gUserManager.DropUser(u);
  if (sd != -1) return sd;

  TCPSocket* s = new TCPSocket(id, LICQ_PPID);
  if (!OpenConnectionToUser(id, s, nPort))
  {
    u = gUserManager.FetchUser(id, LICQ_PPID, LOCK_W);
    if (u != NULL) u->SetConnectionInProgress(false);
    gUserManager.DropUser(u);
    delete s;
    return -1;
  }
  s->SetChannel(nChannel);

  gLog.Info(tr("%sShaking hands with %s (%s) [v%d].\n"), L_TCPxSTR,
     szAlias, id, nVersion);
  nPort = s->LocalPort();

  if (!Handshake_Send(s, id, 0, nVersion))
  {
    u = gUserManager.FetchUser(id, LICQ_PPID, LOCK_W);
    if (u != NULL) u->SetConnectionInProgress(false);
    gUserManager.DropUser(u);
    delete s;
    return -1;
  }
  s->SetVersion(nVersion);
  int nSD = s->Descriptor();

  // Set the socket descriptor in the user
  u = gUserManager.FetchUser(id, LICQ_PPID, LOCK_W);
  if (u == NULL) return -1;
  u->SetSocketDesc(s);
  u->SetConnectionInProgress(false);
  gUserManager.DropUser(u);

  // Add the new socket to the socket manager
  gSocketManager.AddSocket(s);
  gSocketManager.DropSocket(s);

  // Alert the select thread that there is a new socket
  write(pipe_newsocket[PIPE_WRITE], "S", 1);

  return nSD;
}



/*------------------------------------------------------------------------------
 * OpenConnectionToUser
 *
 * Connects a socket to a given user on a given port.
 *----------------------------------------------------------------------------*/
bool CICQDaemon::OpenConnectionToUser(const char* id, TCPSocket *sock,
                                      unsigned short nPort)
{
  const ICQUser* u = gUserManager.FetchUser(id, LICQ_PPID, LOCK_R);
  if (u == NULL) return false;

  char szAlias[64];
  snprintf(szAlias, sizeof(szAlias), "%s (%s)", u->GetAlias(), u->IdString());
  szAlias[sizeof(szAlias) - 1] = '\0';
  unsigned long ip = u->Ip();
  unsigned long intip = u->IntIp();
  bool bSendIntIp = u->SendIntIp();

  gUserManager.DropUser(u);

  return OpenConnectionToUser(szAlias, ip, intip, sock, nPort, bSendIntIp);
}


bool CICQDaemon::OpenConnectionToUser(const char *szAlias, unsigned long nIp,
   unsigned long nIntIp, TCPSocket *sock, unsigned short nPort, bool bSendIntIp)
{
  char buf[128];

  // Sending to internet ip
  if (!bSendIntIp)
  {
    gLog.Info(tr("%sConnecting to %s at %s:%d.\n"), L_TCPxSTR, szAlias,
      ip_ntoa(nIp, buf), nPort);
    // If we fail to set the remote address, the ip must be 0
    if (!sock->SetRemoteAddr(nIp, nPort)) return false;

    if (!sock->OpenConnection())
    {
      gLog.Warn(tr("%sConnect to %s failed:\n%s%s.\n"), L_WARNxSTR, szAlias,
                L_BLANKxSTR, sock->ErrorStr(buf, 128));

      // Now try the internal ip if it is different from this one and we are behind a firewall
      if (sock->Error() != EINTR && nIntIp != nIp &&
          nIntIp != 0 && CPacket::Firewall())
      {
        gLog.Info(tr("%sConnecting to %s at %s:%d.\n"), L_TCPxSTR, szAlias,
                  ip_ntoa(nIntIp, buf), nPort);
        sock->SetRemoteAddr(nIntIp, nPort);

        if (!sock->OpenConnection())
        {
          char buf[128];
          gLog.Warn(tr("%sConnect to %s real ip failed:\n%s%s.\n"), L_WARNxSTR, szAlias,
                    L_BLANKxSTR, sock->ErrorStr(buf, 128));
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
    gLog.Info(tr("%sConnecting to %s at %s:%d.\n"), L_TCPxSTR, szAlias,
       ip_ntoa(nIntIp, buf), nPort);
    if (!sock->SetRemoteAddr(nIntIp, nPort)) return false;

    if (!sock->OpenConnection())
    {
      gLog.Warn(tr("%sConnect to %s real ip failed:\n%s%s.\n"), L_WARNxSTR, szAlias,
         L_BLANKxSTR, sock->ErrorStr(buf, 128));
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
int CICQDaemon::ReverseConnectToUser(const char* id, unsigned long nIp,
   unsigned short nPort, unsigned short nVersion, unsigned short nFailedPort,
   unsigned long nId, unsigned long nMsgID1, unsigned long nMsgID2)
{
  // Find which socket this is for
  TCPSocket *tcp = (TCPSocket *)gSocketManager.FetchSocket(m_nTCPSocketDesc);
  unsigned short tcpPort = tcp ? tcp->LocalPort() : 0;
  gSocketManager.DropSocket(tcp);

  CFileTransferManager *ftm = CFileTransferManager::FindByPort(nFailedPort);
  CChatManager *cm = CChatManager::FindByPort(nFailedPort);

  if (nFailedPort != tcpPort && nFailedPort != 0 && cm == NULL && ftm == NULL)
  {
    gLog.Warn("%sReverse connection to unknown port (%d).\n", L_WARNxSTR,
                                                              nFailedPort);
    return -1;
  }

  TCPSocket* s = new TCPSocket(id, LICQ_PPID);
  char buf[32];

  gLog.Info(tr("%sReverse connecting to %s at %s:%d.\n"), L_TCPxSTR, id,
            ip_ntoa(nIp, buf), nPort);

  // If we fail to set the remote address, the ip must be 0
  s->SetRemoteAddr(nIp, nPort);

  if (!s->OpenConnection())
  {
    char buf[128];
    gLog.Warn(tr("%sReverse connect to %s failed:\n%s%s.\n"), L_WARNxSTR,
        id, L_BLANKxSTR, s->ErrorStr(buf, 128));

    CPU_ReverseConnectFailed* p = new CPU_ReverseConnectFailed(id, nMsgID1,
        nMsgID2, nPort, nFailedPort, nId);
    SendEvent_Server(p);
    return -1;
  }

  gLog.Info(tr("%sReverse shaking hands with %s.\n"), L_TCPxSTR, id);
  bool bConfirm = ftm == NULL && cm == NULL;

  // Make sure we use the right version
  nVersion = VersionToUse(nVersion);

  if (!Handshake_Send(s, id, 0, nVersion, bConfirm, nId))
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
    ICQUser* u = gUserManager.FetchUser(id, LICQ_PPID, LOCK_W);
    if (u != NULL)
    {
      u->SetSocketDesc(s);
      gUserManager.DropUser(u);
    }

    // Add the new socket to the socket manager, alert the thread
    gSocketManager.AddSocket(s);
    gSocketManager.DropSocket(s);
    write(pipe_newsocket[PIPE_WRITE], "S", 1);
  }

  return nSD;
}


//-----CICQDaemon::ProcessTcpPacket----------------------------------------------------
bool CICQDaemon::ProcessTcpPacket(TCPSocket *pSock)
{
  unsigned long senderIp, localIp,
                senderPort, junkLong, nPort, nPortReversed;
  unsigned short version, command, junkShort, newCommand, messageLen,
                 headerLen, ackFlags, msgFlags, licqVersion, theSequence;
  char licqChar = '\0', junkChar;
  bool errorOccured = false;
  char *message = 0;
  char id[16] = "";

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
      snprintf(id, 15, "%lu", nUin);
      break;
    }
    case 4:
    case 5:
    {
      if (!Decrypt_Client(&packet, 4))
      {
        char *buf;
        gLog.Unknown("%sInvalid TCPv4 encryption:\n%s\n", L_UNKNOWNxSTR, packet.print(buf));
        delete [] buf;
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
      snprintf(id, 15, "%lu", nUin);
      break;
    }
    case 6:
    {
      strncpy(id, pSock->OwnerId(), 15); id[15] = '\0';
      if (!Decrypt_Client(&packet, 6))
      {
        char *buf;
        gLog.Unknown("%sInvalid TCPv6 encryption:\n%s\n", L_UNKNOWNxSTR, packet.print(buf));
        delete [] buf;
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
      strncpy(id, pSock->OwnerId(), 15); id[15] = '\0';
      if (!Decrypt_Client(&packet, nInVersion))
      {
        char *buf;
        gLog.Unknown("%sUnknown TCPv%d packet:\n%s\n", L_UNKNOWNxSTR, nInVersion, packet.print(buf));
        delete [] buf;
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
        const ICQUser* u = gUserManager.FetchUser(id, LICQ_PPID, LOCK_R);
        if (u && (u->LicqVersion() == 0 || u->LicqVersion() >= 1022))
        {
          msgFlags <<= 4;
          msgFlags &= 0x0060;
          if (msgFlags & ICQ_TCPxMSG_URGENT)
            msgFlags = ICQ_TCPxMSG_LIST;
          else if (msgFlags & ICQ_TCPxMSG_LIST)
            msgFlags = ICQ_TCPxMSG_URGENT;
        }
        if (u)
          gUserManager.DropUser(u);
      }
      
      break;
    }
    default:
    {
      gLog.Warn(tr("%sUnknown TCP version %d from socket.\n"), L_WARNxSTR, nInVersion);
      break;
    }
  }

  // Some simple validation of the packet
  if (id[0] == '\0' || command == 0)
  {
    char *buf;
    gLog.Unknown("%sInvalid TCP packet (uin: %s, cmd: %04x):\n%s\n",
        L_UNKNOWNxSTR, id, command, packet.print(buf));
    delete [] buf;
    return false;
  }

  if (gUserManager.FindOwner(id, LICQ_PPID) != NULL || strcmp(id, pSock->OwnerId()) != 0)
  {
    char *buf;
    if (gUserManager.FindOwner(id, LICQ_PPID) != NULL)
      gLog.Warn(tr("%sTCP message from self (probable spoof):\n%s\n"), L_WARNxSTR, packet.print(buf));
    else
      gLog.Warn(tr("%sTCP message from invalid UIN (%s, expect %s):\n%s\n"),
          L_WARNxSTR, id, pSock->OwnerId(), packet.print(buf));
    delete [] buf;
    return false;
  }

  // Store our status for later use
  const ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
  unsigned short nOwnerStatus = o->Status();
  gUserManager.DropOwner(o);

  // find which user was sent
  bool bNewUser = false;
  ICQUser* u = gUserManager.FetchUser(id, LICQ_PPID, LOCK_W);
  if (u == NULL)
  {
    u = new ICQUser(id, LICQ_PPID);
    u->SetSocketDesc(pSock);
    bNewUser = true;
  }

  // Check for spoofing
  if (u->SocketDesc(pSock->Channel()) != sockfd)
  {
    gLog.Warn("%sUser %s (%s) socket (%d) does not match incoming message (%d).\n",
              L_TCPxSTR, u->GetAlias(), u->IdString(),
              u->SocketDesc(pSock->Channel()), sockfd);
  }

  if (pSock->Channel() != ICQ_CHNxNONE)
  {
    errorOccured = ProcessPluginMessage(packet, u, pSock->Channel(),
                                        command == ICQ_CMDxTCP_ACK,
                                        0, 0, theSequence, pSock);
  }
  else
  {
  
  // read in the message minus any stupid DOS \r's
  char messageTmp[messageLen + 1];
  unsigned short j = 0;
  for (unsigned short i = 0; i < messageLen; i++)
  {
    packet >> junkChar;
    if (junkChar != 0x0D) messageTmp[j++] = junkChar;
  }
  messageTmp[j] = '\0';

  message = parseRTF(messageTmp);

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
    senderIp = PacketIpToNetworkIp(senderIp);
    localIp = PacketIpToNetworkIp(localIp);
  }

  unsigned long nMask = E_DIRECT
                        | ((newCommand & ICQ_CMDxSUB_FxMULTIREC) ? E_MULTIxREC : 0)
                        | ((msgFlags & ICQ_TCPxMSG_URGENT) ? E_URGENT : 0)
                        | (pSock->Secure() ? E_ENCRYPTED : 0);
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
        gLog.Warn(tr("%sUnknown TCP status: %04X\n"), L_WARNxSTR, msgFlags);
        break;
    }
    //fprintf(stderr, "%08lX\n", (u->StatusFull() & ICQ_STATUS_FxFLAGS) | ns);
    /*if (!bNewUser && ns != ICQ_STATUS_OFFLINE &&
        !((ns & ICQ_STATUS_FxPRIVATE) && u->StatusOffline()))*/
    if (!bNewUser && ns != ICQ_STATUS_OFFLINE &&
        !(ns == ICQ_STATUS_ONLINE && u->Status() == ICQ_STATUS_FREEFORCHAT) &&
        ns != (u->Status() | (u->StatusInvisible() ? ICQ_STATUS_FxPRIVATE : 0)))
    {
      bool r = u->OfflineOnDisconnect() || u->StatusOffline();
      ChangeUserStatus(u, (u->StatusFull() & ICQ_STATUS_FxFLAGS) | ns);
      gLog.Info(tr("%s%s (%s) is %s to us.\n"), L_TCPxSTR, u->GetAlias(),
         u->IdString(), u->StatusStr());
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
            gLog.Info(tr("%sMessage from %s (%s) [Licq %s].\n"), L_TCPxSTR,
                u->GetAlias(), id, CUserEvent::LicqVersionToString(licqVersion));
	  else
            gLog.Info(tr("%sMessage from %s (%s).\n"), L_TCPxSTR, u->GetAlias(), id);

        CPT_AckGeneral p(newCommand, theSequence, true, bAccept, u);
        AckTCP(p, pSock);

        CEventMsg *e = CEventMsg::Parse(message, ICQ_CMDxTCP_START, TIME_NOW, nMask);
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
          if (Ignore(IGNORE_NEWUSERS))
          {
            // FIXME should log a message here or in reject event
            // FIXME should either refuse the event or have a special auto response
            // for rejected events instead of pretending to accept the user
              RejectEvent(id, e);
            break;
          }
          AddUserToList(u->IdString(), u->PPID(), false, true);
          bNewUser = false;
        }

        if (!AddUserEvent(u, e)) break;
        m_xOnEventManager.Do(ON_EVENT_MSG, u);
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
            gLog.Info(tr("%s%s (%s) requested auto response [Licq %s].\n"), L_TCPxSTR,
                u->GetAlias(), id, CUserEvent::LicqVersionToString(licqVersion));
          else
            gLog.Info(tr("%s%s (%s) requested auto response.\n"), L_TCPxSTR, u->GetAlias(), id);

        CPT_AckGeneral p(newCommand, theSequence, true, false, u);
        AckTCP(p, pSock);

        m_sStats[STATS_AutoResponseChecked].Inc();
        u->SetLastCheckedAutoResponse();

          PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_EVENTS, id, LICQ_PPID));
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
            gLog.Info(tr("%sURL from %s (%s) [Licq %s].\n"), L_TCPxSTR, u->GetAlias(),
                id, CUserEvent::LicqVersionToString(licqVersion));
          else
            gLog.Info(tr("%sURL from %s (%s).\n"), L_TCPxSTR, u->GetAlias(), id);

        CEventUrl *e = CEventUrl::Parse(message, ICQ_CMDxTCP_START, TIME_NOW, nMask);
        if (e == NULL)
        {
          char *buf;
          gLog.Warn(tr("%sInvalid URL message:\n%s\n"), L_WARNxSTR, packet.print(buf));
          delete []buf;
          errorOccured = true;
          break;
        }
        e->SetColor(fore, back);

        CPT_AckGeneral p(newCommand, theSequence, true, bAccept, u);
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
          if (Ignore(IGNORE_NEWUSERS))
          {
              RejectEvent(id, e);
            break;
          }
          AddUserToList(u->IdString(), u->PPID(), false, true);
          bNewUser = false;
        }

        if (!AddUserEvent(u, e)) break;
        m_xOnEventManager.Do(ON_EVENT_URL, u);
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
            gLog.Info(tr("%sContact list from %s (%s) [Licq %s].\n"), L_TCPxSTR,
               u->GetAlias(), id, CUserEvent::LicqVersionToString(licqVersion));
          else
            gLog.Info(tr("%sContact list from %s (%s).\n"), L_TCPxSTR,
                u->GetAlias(), id);

        CEventContactList *e = CEventContactList::Parse(message, ICQ_CMDxTCP_START, TIME_NOW, nMask);
        if (e == NULL)
        {
          char *buf;
          gLog.Warn(tr("%sInvalid contact list message:\n%s\n"), L_WARNxSTR, packet.print(buf));
          delete []buf;
          errorOccured = true;
          break;
        }
        e->SetColor(fore, back);

        CPT_AckGeneral p(newCommand, theSequence, true, bAccept, u);
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
          if (Ignore(IGNORE_NEWUSERS))
          {
              RejectEvent(id, e);
            break;
          }
          AddUserToList(u->IdString(), u->PPID(), false, true);
          bNewUser = false;
        }

        if (!AddUserEvent(u, e)) break;
        m_xOnEventManager.Do(ON_EVENT_MSG, u);

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
            gLog.Info(tr("%sChat request from %s (%s) [Licq %s].\n"), L_TCPxSTR,
               u->GetAlias(), id, CUserEvent::LicqVersionToString(licqVersion));
          else
            gLog.Info(tr("%sChat request from %s (%s).\n"), L_TCPxSTR,
                u->GetAlias(), id);

        // translating string with translation table
        gTranslator.ServerToClient (message);
        CEventChat *e = new CEventChat(message, szChatClients, nPort, theSequence,
           TIME_NOW, nMask | licqVersion);

        // Add the user to our list if they are new
        if (bNewUser)
        {
          if (Ignore(IGNORE_NEWUSERS))
          {
              RejectEvent(id, e);
            break;
          }
          AddUserToList(u->IdString(), u->PPID(), false, true);
          bNewUser = false;
        }

        if (!AddUserEvent(u, e)) break;
        m_xOnEventManager.Do(ON_EVENT_CHAT, u);
        break;
      }

      // File transfer
      case ICQ_CMDxSUB_FILE:
      {
        unsigned short nLenFilename;
        unsigned long nFileLength;
        packet >> junkLong
               >> nLenFilename;
        char szFilename[nLenFilename+1];
        for (unsigned short i = 0; i < nLenFilename; i++)
           packet >> szFilename[i];
        szFilename[nLenFilename] = '\0';
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
            gLog.Info(tr("%sFile transfer request from %s (%s) [Licq %s].\n"),
                L_TCPxSTR, u->GetAlias(), id, CUserEvent::LicqVersionToString(licqVersion));
          else
            gLog.Info(tr("%sFile transfer request from %s (%s).\n"), L_TCPxSTR,
                u->GetAlias(), id);

        ConstFileList filelist;
        filelist.push_back(szFilename);

        // translating string with translation table
        gTranslator.ServerToClient (message);
        CEventFile *e = new CEventFile(szFilename, message, nFileLength,
                                       filelist, theSequence, TIME_NOW,
                                       nMask | licqVersion);
        // Add the user to our list if they are new
        if (bNewUser)
        {
          if (Ignore(IGNORE_NEWUSERS))
          {
              RejectEvent(id, e);
            break;
          }
          AddUserToList(u->IdString(), u->PPID(), false, true);
          bNewUser = false;
        }

        if (!AddUserEvent(u, e)) break;
        m_xOnEventManager.Do(ON_EVENT_FILE, u);
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

				char szPlugin[nLongLen+1];
				for (unsigned long i = 0; i < nLongLen; i++)
					packet >> szPlugin[i];
				szPlugin[nLongLen] = '\0';

				packet.incDataPosRead(nLen - 22 - nLongLen);
				packet.incDataPosRead(4); // bytes left in packet
				packet >> nLongLen; // message len

				int nICBMCommand = 0;
				if (strstr(szPlugin, "File"))
					nICBMCommand = ICQ_CMDxSUB_FILE;
				else if (strstr(szPlugin, "URL"))
					nICBMCommand = ICQ_CMDxSUB_URL;
				else if (strstr(szPlugin, "Chat"))
					nICBMCommand = ICQ_CMDxSUB_CHAT;
				else if (strstr(szPlugin, "Contacts"))
					nICBMCommand = ICQ_CMDxSUB_CONTACTxLIST;
				else
				{
					gLog.Info(tr("%sUnknown ICBM plugin type: %s\n"), L_TCPxSTR, szPlugin);
					break;
				}

				char szMessage[nLongLen+1];
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
					char szFilename[nLen+1];
					for (unsigned short i = 0; i < nLen; i++)
						packet >> szFilename[i];
					szFilename[nLen] = '\0';
					packet >> nFileSize;
					packet.incDataPosRead(2); // reversed port (BE)

              gLog.Info(tr("%sFile transfer request from %s (%s).\n"),
                  L_TCPxSTR, u->GetAlias(), id);

					ConstFileList filelist;
					filelist.push_back(szFilename);

					// translating string with translation table
					gTranslator.ServerToClient(szMessage);
					CEventFile *e = new CEventFile(szFilename, szMessage, nFileSize,
																				 filelist, theSequence, TIME_NOW, nMask);
					if (bNewUser)
					{
						if (Ignore(IGNORE_NEWUSERS))
						{
                  RejectEvent(id, e);
							break;
						}
						AddUserToList(u->IdString(), u->PPID(), false, true);
						bNewUser = false;
					}

					if (!AddUserEvent(u, e)) break;
					m_xOnEventManager.Do(ON_EVENT_FILE, u);
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

              gLog.Info(tr("%sChat request from %s (%s).\n"), L_TCPxSTR,
                  u->GetAlias(), id);

					// translating string with translation table
					gTranslator.ServerToClient(szMessage);
					CEventChat *e = new CEventChat(szMessage, szChatClients, nPort,
																				 theSequence, TIME_NOW, nMask);
					if (bNewUser)
					{
						if (Ignore(IGNORE_NEWUSERS))
						{
                  RejectEvent(id, e);
							break;
						}
						AddUserToList(u->IdString(), u->PPID(), false, true);
						bNewUser = false;
					}

					if (!AddUserEvent(u, e)) break;
					m_xOnEventManager.Do(ON_EVENT_CHAT, u);
					break;
				}
				case ICQ_CMDxSUB_URL:
				{
              gLog.Info(tr("%sURL from %s (%s).\n"), L_TCPxSTR, u->GetAlias(), id);
					CEventUrl *e = CEventUrl::Parse(szMessage, ICQ_CMDxTCP_START,
																					TIME_NOW, nMask);
					if (e == NULL)
					{
						char *buf;
						gLog.Warn(tr("%sInvalid URL message:\n%s\n"), L_WARNxSTR,
											packet.print(buf));
						delete [] buf;
						errorOccured = true;
						break;
					}

					if (bNewUser)
					{
						if (Ignore(IGNORE_NEWUSERS))
						{
                  RejectEvent(id, e);
							break;
						}
						AddUserToList(u->IdString(), u->PPID(), false, true);
						bNewUser = false;
					}

					if (!AddUserEvent(u, e)) break;
					m_xOnEventManager.Do(ON_EVENT_URL, u);
					break;
				}
				case ICQ_CMDxSUB_CONTACTxLIST:
				{
              gLog.Info(tr("%sContact list from %s (%s).\n"), L_TCPxSTR,
                  u->GetAlias(), id);
					CEventContactList *e = CEventContactList::Parse(szMessage,
																													ICQ_CMDxTCP_START,
																													TIME_NOW, nMask);
					if (e == NULL)
					{
						char *buf;
						gLog.Warn(tr("%sInvalid contact list message:\n%s\n"), L_TCPxSTR,
											packet.print(buf));
						delete [] buf;
						errorOccured = true;
						break;
					}

					if (bNewUser)
					{
						if (Ignore(IGNORE_NEWUSERS))
						{
                  RejectEvent(id, e);
							break;
						}
						AddUserToList(u->IdString(), u->PPID(), false, true);
						bNewUser = false;
					}

					if (!AddUserEvent(u, e)) break;
					m_xOnEventManager.Do(ON_EVENT_MSG, u);					
					break;
				}
				} // switch nICBMCommand

				break;
			}

      // Old-style encryption request:
      case ICQ_CMDxSUB_SECURExOLD:
      {
          gLog.Info(tr("%sReceived old-style key request from %s (%s) but we do not support it.\n"),
              L_TCPxSTR, u->GetAlias(), id);
        // Send the nack back
        CPT_AckOldSecureChannel p(theSequence, u);
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
            gLog.Info(tr("%sSecure channel request from %s (%s) [Licq %s].\n"),
                L_TCPxSTR, u->GetAlias(), id, CUserEvent::LicqVersionToString(licqVersion));
          else
            gLog.Info(tr("%sSecure channel request from %s (%s).\n"), L_TCPxSTR,
                u->GetAlias(), id);

        CPT_AckOpenSecureChannel p(theSequence, true, u);
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
          if (Ignore(IGNORE_NEWUSERS))
            break;
          AddUserToList(u->IdString(), u->PPID(), false, true);
          bNewUser = false;
        }

        u->SetSendServer(false);
          PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_SECURITY, id, LICQ_PPID, 1));

          gLog.Info(tr("%sSecure channel established with %s (%s).\n"),
              L_SSLxSTR, u->GetAlias(), id);

        break;

#else // We do not support OpenSSL
          gLog.Info(tr("%sReceived secure channel request from %s (%s) but we do not support OpenSSL.\n"),
              L_TCPxSTR, u->GetAlias(), id);
        // Send the nack back
        CPT_AckOpenSecureChannel p(theSequence, false, u);
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
            gLog.Info(tr("%sSecure channel closed by %s (%s) [Licq %s].\n"),
                L_TCPxSTR, u->GetAlias(), id, CUserEvent::LicqVersionToString(licqVersion));
          else
            gLog.Info(tr("%sSecure channel closed by %s (%s).\n"), L_TCPxSTR,
                u->GetAlias(), id);

        // send ack
        CPT_AckCloseSecureChannel p(theSequence, u);
        AckTCP(p, pSock);

        pSock->SecureStop();
        u->SetSecure(false);
          PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_SECURITY, id, LICQ_PPID, 0));
        break;

#else // We do not support OpenSSL
          gLog.Info(tr("%sReceived secure channel close from %s (%s) but we do not support OpenSSL.\n"),
              L_TCPxSTR, u->GetAlias(), id);
        // Send the nack back
        CPT_AckCloseSecureChannel p(theSequence, u);
        AckTCP(p, pSock);
        break;
#endif
      }


      default:
      {
        char *buf;
        gLog.Unknown("%sUnknown TCP message type (%04x):\n%s\n", L_UNKNOWNxSTR,
          newCommand, packet.print(buf));
        errorOccured = true;
        delete []buf;
        break;
      }
    }
    break;
  }

  //-----ACK--------------------------------------------------------------------
  case ICQ_CMDxTCP_ACK:  // message received packet
  {
    // If this is not from a user on our list then ignore it
    if (bNewUser) break;

    CExtendedAck *pExtendedAck = NULL;

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

        pExtendedAck = new CExtendedAck (nPort != 0, nPort, message);
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

         pExtendedAck = new CExtendedAck(nPort != 0, nPort, message);
         break;
      }

		  case ICQ_CMDxSUB_ICBM:
			{
				unsigned short nLen;
				unsigned long nLongLen;

				packet >> nLen;
				packet.incDataPosRead(18); // eh?
				packet >> nLongLen; // Plugin name len

				char szPlugin[nLongLen+1];
				for (unsigned long i = 0; i < nLongLen; i++)
					packet >> szPlugin[i];
				szPlugin[nLongLen] = '\0';
				
				packet.incDataPosRead(nLen - 22 - nLongLen);
				packet.incDataPosRead(4); // left in packet

				int nICBMCommand = 0;
				if (strstr(szPlugin, "File"))
					nICBMCommand = ICQ_CMDxSUB_FILE;
				else if (strstr(szPlugin, "Chat"))
					nICBMCommand = ICQ_CMDxSUB_CHAT;
				else if (strstr(szPlugin, "URL"))
					nICBMCommand = ICQ_CMDxSUB_URL;
				else if (strstr(szPlugin, "Contacts"))
					nICBMCommand = ICQ_CMDxSUB_CONTACTxLIST;
				else
				{
					gLog.Info(tr("%sUnknown direct ack ICBM plugin type: %s\n"), L_TCPxSTR,
										szPlugin);
					gUserManager.DropUser(u);
					return true;
				}

				packet >> nLongLen;
				char szMessage[nLongLen+1];
				for (unsigned short i = 0; i < nLongLen; i++)
					packet >> szMessage[i];
				szMessage[nLongLen] = '\0';

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

					pExtendedAck = new CExtendedAck(nPort != 0, nPort,
                                message[0] != '\0' ? message : szMessage);
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
					pExtendedAck = new CExtendedAck(bAccepted, nPort,
                                message[0] != '\0' ? message : szMessage);
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
        if (licqChar == 'L') sprintf(l, " [Licq %s]",
         CUserEvent::LicqVersionToString(licqVersion));
          gLog.Info(tr("%sSecure channel response from %s (%s)%s.\n"), L_TCPxSTR,
              u->GetAlias(), id, l);

        ICQEvent *e = NULL;

        // Check if the response is ok
        if (message[0] == '\0')
        {
            gLog.Info(tr("%s%s (%s) does not support OpenSSL.\n"), L_TCPxSTR,
                u->GetAlias(), id);
          u->SetSecure(false);
            PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_SECURITY, id, LICQ_PPID, 0));
          // find the event, fail it
          e = DoneEvent(sockfd, theSequence, EVENT_FAILED);
        }
        else
        {
          // Find the event, succeed it
          e = DoneEvent(sockfd, theSequence, EVENT_SUCCESS);

          // Check that a request was in progress...should always be ok
          if (e == NULL)
          {
              gLog.Warn(tr("%sSecure channel response from %s (%s) when no request in progress.\n"),
                  L_WARNxSTR, u->GetAlias(), id);
            // Close the connection as we are in trouble
            u->SetSecure(false);
            gUserManager.DropUser(u);
              PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_SECURITY, id, LICQ_PPID, 0));
            return false;
          }

          if (!pSock->SecureConnect())
          {
            errorOccured = true;
            e->m_eResult = EVENT_FAILED;
          }
          else
          {
              gLog.Info(tr("%sSecure channel established with %s (%s).\n"),
                  L_SSLxSTR, u->GetAlias(), id);
            u->SetSecure(true);
              PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_SECURITY, id, LICQ_PPID, 1));
          }
        }

        // finish up
        e->m_nSubResult = ICQ_TCPxACK_ACCEPT;
        gUserManager.DropUser(u);
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
        if (licqChar == 'L') sprintf(l, " [Licq %s]", CUserEvent::LicqVersionToString(licqVersion));
          gLog.Info(tr("%sSecure channel with %s (%s) closed %s.\n"), L_TCPxSTR,
              u->GetAlias(), id, l);

        // Find the event, succeed it
        ICQEvent *e = DoneEvent(sockfd, theSequence, EVENT_SUCCESS);

        // Check that a request was in progress...should always be ok
        if (e == NULL)
        {
          // Close the connection as we are in trouble
          gUserManager.DropUser(u);
          delete e;
          return false;
        }

        pSock->SecureStop();
        u->SetSecure(false);
        gUserManager.DropUser(u);
          PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_SECURITY, id, LICQ_PPID, 0));

        // finish up
        e->m_nSubResult = ICQ_TCPxACK_ACCEPT;
        ProcessDoneEvent(e);

        // get out of here now as we don't want standard ack processing
        return true;
      }
#endif

      default:
      {
        char *buf;
        gLog.Unknown("%sUnknown TCP Ack subcommand (%04x):\n%s\n", L_UNKNOWNxSTR,
                     newCommand, packet.print(buf));
        errorOccured = true;
        delete []buf;
        break;
      }
    }

    char l[32] = "";
    if (licqChar == 'L') sprintf(l, " [Licq %s]", CUserEvent::LicqVersionToString(licqVersion));

    // translating string with translation table
    gTranslator.ServerToClient (message);
    // output the away message if there is one (ie if user status is not online)
    int nSubResult;
    if (ackFlags == ICQ_TCPxACK_REFUSE)
    {
      gLog.Info(tr("%sRefusal from %s (#%hu)%s.\n"), L_TCPxSTR, u->GetAlias(), -theSequence, l);
      nSubResult = ICQ_TCPxACK_REFUSE;
    }
    else
    {
      // Update the away message if it's changed
      if (strcmp(u->AutoResponse(), message))
      {
        u->SetAutoResponse(message);
        u->SetShowAwayMsg(*message);
        gLog.Info(tr("%sAuto response from %s (#%hu)%s.\n"), L_TCPxSTR,
                  u->GetAlias(), -theSequence, l);
      }

      switch(ackFlags)
      {
        case ICQ_TCPxACK_ONLINE:
          gLog.Info(tr("%sAck from %s (#%hu)%s.\n"), L_TCPxSTR, u->GetAlias(), -theSequence, l);
          if (pExtendedAck && !pExtendedAck->Accepted())
            nSubResult = ICQ_TCPxACK_RETURN;
          else
            nSubResult = ICQ_TCPxACK_ACCEPT;
          break;
        case ICQ_TCPxACK_AWAY:
        case ICQ_TCPxACK_NA:
        case ICQ_TCPxACK_OCCUPIEDx2: //auto decline due to occupied mode
          gLog.Info(tr("%sAck from %s (#%hu)%s.\n"), L_TCPxSTR, u->GetAlias(), -theSequence, l);
          nSubResult = ICQ_TCPxACK_REFUSE;
          break;
        case ICQ_TCPxACK_OCCUPIED:
        case ICQ_TCPxACK_DND:
          gLog.Info(tr("%sReturned from %s (#%hu)%s.\n"), L_TCPxSTR, u->GetAlias(), -theSequence, l);
          nSubResult = ICQ_TCPxACK_RETURN;
          break;
        case ICQ_TCPxACK_OCCUPIEDxCAR:
        case ICQ_TCPxACK_DNDxCAR:
          gLog.Info(tr("%sCustom %s response from %s (#%hu)%s.\n"), L_TCPxSTR,
                    (ackFlags == ICQ_TCPxACK_DNDxCAR ? tr("DnD") : tr("Occupied")), u->GetAlias(),
                    -theSequence, l);
          nSubResult = ICQ_TCPxACK_ACCEPT; // FIXME: or should this be ACK_RETURN ?
          break;
        default:
          gLog.Unknown("%sUnknown ack flag from %s (#%hu): %04x %s.\n", L_UNKNOWNxSTR,
                       u->GetAlias(), -theSequence, ackFlags, l);
          nSubResult = ICQ_TCPxACK_ACCEPT;
      }
    }

    ICQEvent *e = DoneEvent(sockfd, theSequence, EVENT_ACKED);
    if (e != NULL)
    {
      e->m_pExtendedAck = pExtendedAck;
      e->m_nSubResult = nSubResult;
      gUserManager.DropUser(u);
      ProcessDoneEvent(e);
      return true;
    }
    else
    {
      gLog.Warn(tr("%sAck for unknown event.\n"), L_TCPxSTR);
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
          gLog.Info(tr("%sChat request from %s (%s) cancelled.\n"), L_TCPxSTR,
              u->GetAlias(), id);
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
            u->EventPeek(i)->Cancel();
            break;
          }
        }
        break;
      }
      case ICQ_CMDxSUB_FILE:
      {
          gLog.Info(tr("%sFile transfer request from %s (%s) cancelled.\n"),
              L_TCPxSTR, u->GetAlias(), id);
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
            u->EventPeek(i)->Cancel();
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
    char *buf;
    gLog.Unknown("%sUnknown TCP packet (command 0x%04x):\n%s\n", L_UNKNOWNxSTR, command, packet.print(buf));
    delete [] buf;
    errorOccured = true;
    break;
  }
  
  }
  if (bNewUser)
  {
    delete u;
    return false;
  }
  gUserManager.DropUser(u);
  if (message)
    delete [] message;
  return !errorOccured;
}

bool CICQDaemon::ProcessPluginMessage(CBuffer &packet, ICQUser *u,
                                      unsigned char nChannel,
                                      bool bIsAck,
                                      unsigned long nMsgID1,
                                      unsigned long nMsgID2,
                                      unsigned short nSequence,
                                      TCPSocket *pSock)
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
        gLog.Warn("%sInfo plugin request with unknown level %u from %s.\n",
                  L_WARNxSTR, error_level, u->GetAlias());
        errorOccured = true;
        break;
      }
      char GUID[GUID_LENGTH];
      for (int i = 0 ; i < GUID_LENGTH; i ++)
        packet >> GUID[i];

      if (memcmp(GUID, PLUGIN_QUERYxINFO, GUID_LENGTH) == 0)
      {
        gLog.Info("%sInfo plugin list request from %s.\n", szInfo,
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
        gLog.Info("%sPhone Book request from %s.\n", szInfo, u->GetAlias());
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
        gLog.Info("%sPicture request from %s.\n", szInfo, u->GetAlias());

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
        gLog.Warn("%sUnknown info request from %s.\n", L_WARNxSTR,
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
      EventResult result;

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
          ICQEvent *e = pSock ? DoneEvent(pSock->Descriptor(), nSequence,
                                          EVENT_ACKED) :
                                DoneServerEvent(nMsgID2, EVENT_ACKED);

          if (e == NULL)
          {
            gLog.Warn("%sAck for unknown event from %s.\n", L_WARNxSTR,
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
            gLog.Warn("%sAck for the wrong event from %s.\n", L_WARNxSTR,
                                                              u->GetAlias());
            delete e;
            return true;
          }

          if (memcmp(GUID, PLUGIN_PICTURE, GUID_LENGTH) == 0)
          {
            gLog.Info("%s%s has no picture.\n", szInfo, u->GetAlias());

            char szFilename[MAX_FILENAME_LEN];
            szFilename[MAX_FILENAME_LEN - 1] = '\0';
            snprintf(szFilename, MAX_FILENAME_LEN - 1, "%s/%s/%s.pic",
                                             BASE_DIR, USER_DIR, u->IdString());

            if (remove(szFilename) != 0 && errno != ENOENT)
            {
              gLog.Error("%sUnable to delete %s's picture file (%s):\n%s%s.\n",
                         L_ERRORxSTR, u->GetAlias(), szFilename, L_BLANKxSTR,
                         strerror(errno));
            }

            u->SetEnableSave(false);
            u->SetPicturePresent(false);
            u->SetEnableSave(true);
            u->SavePictureInfo();

            PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_PICTURE,
                u->IdString(), u->PPID()));
          }
          else if (memcmp(GUID, PLUGIN_QUERYxINFO, GUID_LENGTH) == 0)
          {
            gLog.Info("%s%s has no info plugins.\n", szInfo, u->GetAlias());
          }
          else
          {
            gLog.Unknown("%sUnknown info response with no data from %s.\n",
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
              char szName[nLen+1];
              for (unsigned long i = 0; i < nLen; i++)
                packet >> szName[i];
              szName[nLen] = '\0';

              nLen = packet.UnpackUnsignedLong();
              char szFullName[nLen+1];
              for (unsigned long i = 0; i < nLen; i++)
                packet >> szFullName[i];
              szFullName[nLen] = '\0';

              packet.incDataPosRead(4); //Unknown (always 0?)

              gLog.Info("%s%s has %s (%s).\n", szInfo, u->GetAlias(), szName,
                                               szFullName);
            }
            break;
          }

          case ICQ_PLUGIN_RESP_PHONExBOOK:
          {
            gLog.Info("%sPhone Book reply from %s.\n", szInfo, u->GetAlias());
            struct PhoneBookEntry pb[nEntries];
            for (unsigned long i = 0; i < nEntries; i ++)
            {
              unsigned long nLen = packet.UnpackUnsignedLong();
              pb[i].szDescription = new char[nLen + 1];
              for (unsigned long j = 0; j < nLen; j++)
                packet >> pb[i].szDescription[j];
              pb[i].szDescription[nLen] = '\0';
              gTranslator.ServerToClient(pb[i].szDescription);

              nLen = packet.UnpackUnsignedLong();
              pb[i].szAreaCode = new char[nLen + 1];
              for (unsigned long j = 0; j < nLen; j++)
                packet >> pb[i].szAreaCode[j];
              pb[i].szAreaCode[nLen] = '\0';
              gTranslator.ServerToClient(pb[i].szAreaCode);

              nLen = packet.UnpackUnsignedLong();
              pb[i].szPhoneNumber = new char[nLen + 1];
              for (unsigned long j = 0; j < nLen; j++)
                packet >> pb[i].szPhoneNumber[j];
              pb[i].szPhoneNumber[nLen] = '\0';
              gTranslator.ServerToClient(pb[i].szPhoneNumber);

              nLen = packet.UnpackUnsignedLong();
              pb[i].szExtension = new char[nLen + 1];
              for (unsigned long j = 0; j < nLen; j++)
                packet >> pb[i].szExtension[j];
              pb[i].szExtension[nLen] = '\0';
              gTranslator.ServerToClient(pb[i].szExtension);

              nLen = packet.UnpackUnsignedLong();
              pb[i].szCountry = new char[nLen + 1];
              for (unsigned long j = 0; j < nLen; j++)
                packet >> pb[i].szCountry[j];
              pb[i].szCountry[nLen] = '\0';
              gTranslator.ServerToClient(pb[i].szCountry);

              pb[i].nActive = packet.UnpackUnsignedLong();
            }
            for (unsigned long i = 0; i < nEntries; i ++)
            {
              packet.UnpackUnsignedLong(); // entry length

              pb[i].nType = packet.UnpackUnsignedLong();

              unsigned long nLen = packet.UnpackUnsignedLong();
              pb[i].szGateway = new char[nLen + 1];
              for (unsigned long j = 0; j < nLen; j++)
                packet >> pb[i].szGateway[j];
              pb[i].szGateway[nLen] = '\0';
              gTranslator.ServerToClient(pb[i].szGateway);

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
              delete [] pb[i].szDescription;
              delete [] pb[i].szAreaCode;
              delete [] pb[i].szPhoneNumber;
              delete [] pb[i].szExtension;
              delete [] pb[i].szCountry;
              delete [] pb[i].szGateway;
            }
            u->SetEnableSave(true);
            u->SavePhoneBookInfo();

            PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_PHONExBOOK,
                u->IdString(), u->PPID()));

            break;
          }

          case ICQ_PLUGIN_RESP_PICTURE:
          {
            gLog.Info("%sPicture reply from %s.\n", szInfo, u->GetAlias());
            packet.incDataPosRead(nEntries); // filename, don't care
            unsigned long nLen = packet.UnpackUnsignedLong();
            if (nLen == 0)	// do not create empty .pic files
              break;

            char szFilename[MAX_FILENAME_LEN];
            szFilename[MAX_FILENAME_LEN - 1] = '\0';
            snprintf(szFilename, MAX_FILENAME_LEN - 1, "%s/%s/%s.pic",
                                             BASE_DIR, USER_DIR, u->IdString());

            int nFD = open(szFilename, O_WRONLY | O_CREAT | O_TRUNC, 00664);
            if (nFD == -1)
            {
              gLog.Error("%sUnable to open picture file (%s):\n%s%s.\n",
                         L_ERRORxSTR, szFilename, L_BLANKxSTR, strerror(errno));
              break;
            }

            char data[nLen];
            for (unsigned long i = 0; i < nLen; i++)
            {
              packet >> data[i];
            }

            write(nFD, data, nLen);
            close(nFD);

            u->SetEnableSave(false);
            u->SetPicturePresent(true);
            u->SetEnableSave(true);
            u->SavePictureInfo();

            PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_PICTURE,
                u->IdString(), u->PPID()));

            break;
          }
          }
        }

        result = EVENT_ACKED;
        break;
      }
      case ICQ_PLUGIN_ERROR:
      {
        gLog.Warn("%sInfo plugin not available from %s.\n", L_WARNxSTR,
                                                            u->GetAlias());
        result = EVENT_ERROR;
        break;
      }
      case ICQ_PLUGIN_REJECTED:
      {
        gLog.Info("%s%s refused our request.\n", szInfo, u->GetAlias());
        result = EVENT_FAILED;
        break;
      }
      case ICQ_PLUGIN_AWAY:
      {
        gLog.Info("%sOur request was refused because %s is away.\n", szInfo,
                  u->GetAlias());
        result = EVENT_FAILED;
        break;
      }
      default:
      {
        gLog.Warn("%sUnknown reply level %u from %s.\n", L_UNKNOWNxSTR,
                  error_level, u->GetAlias());
        errorOccured = true;
        result = EVENT_ERROR;
        break;
      }
      } 

      ICQEvent *e = pSock ? DoneEvent(pSock->Descriptor(), nSequence,
                                      result) :
                            DoneServerEvent(nMsgID2, result);
      if (e == NULL)
        gLog.Warn("%sAck for unknown event from %s.\n", L_WARNxSTR,
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
        gLog.Warn("%sUnknown status plugin request level %u from %s.\n",
                  L_WARNxSTR, error_level, u->GetAlias());
        errorOccured = true;
        break;
      }

      char GUID[GUID_LENGTH];
      for (int i = 0 ; i < GUID_LENGTH; i ++)
        packet >> GUID[i];

      if (memcmp(GUID, PLUGIN_QUERYxSTATUS, GUID_LENGTH) == 0)
      {
        gLog.Info("%sStatus plugin list request from %s.\n", szInfo,
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
        gLog.Info("%sFile server status request from %s.\n", szInfo,
                                                              u->GetAlias());
        const ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
        unsigned long nStatus = o->SharedFilesStatus();
        gUserManager.DropOwner(o);
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
        gLog.Info("%sICQphone status request from %s.\n", szInfo,
                                                              u->GetAlias());
        const ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
        unsigned long nStatus = o->ICQphoneStatus();
        gUserManager.DropOwner(o);
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
        gLog.Info("%sPhone \"Follow Me\" status request from %s.\n", szInfo,
                                                              u->GetAlias());
        const ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
        unsigned long nStatus = o->PhoneFollowMeStatus();
        gUserManager.DropOwner(o);
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
        gLog.Warn("%sUnknown status request from %s.\n", L_WARNxSTR,
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
      EventResult result;

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
          gLog.Info("%s%s has no status plugins.\n", szInfo, u->GetAlias());
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
            char szName[nLen+1];
            for (unsigned long i = 0; i < nLen; i++)
              packet >> szName[i];
            szName[nLen] = '\0';

            nLen = packet.UnpackUnsignedLong();
            char szFullName[nLen+1];
            for (unsigned long i = 0; i < nLen; i++)
              packet >> szFullName[i];
            szFullName[nLen] = '\0';

            packet.incDataPosRead(4); //Unknown (always 0?)

            gLog.Info("%s%s has %s (%s).\n", szInfo, u->GetAlias(), szName,
                                             szFullName);
          }
        }

        result = EVENT_ACKED;
        break;
      }
      case ICQ_PLUGIN_STATUSxREPLY:
      {
        ICQEvent *e = pSock ? DoneEvent(pSock->Descriptor(), nSequence,
                                        EVENT_ACKED) :
                              DoneServerEvent(nMsgID2, EVENT_ACKED);

        if (e == NULL)
        {
          gLog.Warn("%sAck for unknown event from %s.\n", L_WARNxSTR,
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
          gLog.Warn("%sAck for the wrong event from %s.\n", L_WARNxSTR,
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
          gLog.Info("%s%s's Shared Files Directory is %s.\n", szInfo,
                                                     u->GetAlias(), szState);
          u->SetSharedFilesStatus(nState);
        }
        else if (memcmp(GUID, PLUGIN_FOLLOWxME, GUID_LENGTH) == 0)
        {
          gLog.Info("%s%s's Phone \"Follow Me\" is %s.\n", szInfo,
                                                     u->GetAlias(), szState);
          u->SetPhoneFollowMeStatus(nState);
        }
        else if (memcmp(GUID, PLUGIN_ICQxPHONE, GUID_LENGTH) == 0)
        {
          gLog.Info("%s%s's ICQphone is %s.\n", szInfo, u->GetAlias(),
                                                                    szState);
          u->SetICQphoneStatus(nState);
        }

        // Which plugin?
        PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_PLUGIN_STATUS,
            u->IdString(), u->PPID(), 0));

        ProcessDoneEvent(e);
        return false;
      }
      case ICQ_PLUGIN_ERROR:
      {
        gLog.Warn("%sStatus plugin not available from %s.\n", L_WARNxSTR,
                                                            u->GetAlias());
        result = EVENT_ERROR;
        break;
      }
      case ICQ_PLUGIN_REJECTED:
      {
        gLog.Info("%s%s refused our request.\n", szInfo, u->GetAlias());
        result = EVENT_FAILED;
        break;
      }
      case ICQ_PLUGIN_AWAY:
      {
        gLog.Info("%sOur request was refused because %s is away.\n", szInfo,
                  u->GetAlias());
        result = EVENT_FAILED;
        break;
      }
      default:
      {
        gLog.Warn("%sUnknown reply level %u from %s.\n", L_UNKNOWNxSTR,
                  error_level, u->GetAlias());
        errorOccured = true;
        result = EVENT_ERROR;
        break;
      }
      }

      ICQEvent *e = pSock ? DoneEvent(pSock->Descriptor(), nSequence,
                                      result) :
                            DoneServerEvent(nMsgID2, result);
      if (e == NULL)
        gLog.Warn("%sAck for unknown event from %s.\n", L_WARNxSTR,
                                                        u->GetAlias());
      else
        ProcessDoneEvent(e);

    }

    break;
  }
  default:
  {
    gLog.Warn("%sUnknown channel %u from %s\n", L_WARNxSTR, nChannel,
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
void CICQDaemon::AckTCP(CPacketTcp &p, int nSd)
{
#if ICQ_VERSION_TCP == 3
  TCPSocket *s = (TCPSocket *)gSocketManager.FetchSocket(nSD);
  if (s != NULL)
  {
    s->Send(p.getBuffer());
    gSocketManager.DropSocket(s);
  }
#else
  SendEvent(nSd, p, false);
#endif
}

void CICQDaemon::AckTCP(CPacketTcp &p, TCPSocket *tcp)
{
#if ICQ_VERSION_TCP == 3
  tcp->Send(p.getBuffer());
#else
  SendEvent(tcp, p, false);
#endif
}


bool CICQDaemon::Handshake_Recv(TCPSocket *s, unsigned short nPort,
																bool bConfirm, bool bChat)
{
  char cHandshake;
  unsigned short nVersionMajor, nVersionMinor;
  CBuffer &b = s->RecvBuffer();
  b >> cHandshake >> nVersionMajor >> nVersionMinor;

  unsigned long nUin = 0;
  unsigned short nVersion = 0;
  char id[16];

  switch (VersionToUse(nVersionMajor))
  {
    case 8:
    case 7:
    {
      b.Reset();
      CPacketTcp_Handshake_v7 p_in(&b);
      nUin = p_in.SourceUin();
      snprintf(id, 16, "%lu", nUin);

      const ICQUser* u = gUserManager.FetchUser(id, LICQ_PPID, LOCK_R);
      if (u == NULL && !bChat)
      {
        gLog.Warn("%sConnection from unknown user.\n", L_WARNxSTR);
        return false;
      }
      unsigned long nCookie = u ? u->Cookie() : 0;
      gUserManager.DropUser(u);

      if (nCookie != p_in.SessionId())
      {
        char ipbuf[32];
        gLog.Warn("%sSpoofed connection from %s as uin %s.\n", L_WARNxSTR,
            s->RemoteIpStr(ipbuf), id);
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
        gLog.Warn("%sHandshake ack not the right size.\n", L_WARNxSTR);
        return false;
      }

      unsigned long nOk = s->RecvBuffer().UnpackUnsignedLong();
      if (nOk != 1)
      {
        gLog.Warn(tr("%sBad handshake ack: %ld.\n"), L_WARNxSTR, nOk);
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
              gLog.Warn("%sReverse connection with unknown id (%lu)",
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

      const ICQUser* u = gUserManager.FetchUser(id, LICQ_PPID, LOCK_R);
      if (u == NULL)
      {
        gLog.Warn("%sConnection from unknown user.\n", L_WARNxSTR);
        return false;
      }
      unsigned long nCookie = u->Cookie();
      gUserManager.DropUser(u);

      if (nCookie != p_in.SessionId())
      {
        char ipbuf[32];
        gLog.Warn("%sSpoofed connection from %s as uin %s.\n", L_WARNxSTR,
            s->RemoteIpStr(ipbuf), id);
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
        gLog.Warn(tr("%sBad handshake ack: %ld.\n"), L_WARNxSTR, nOk);
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
      nVersion = VersionToUse(nVersionMajor);

      const ICQUser* u = gUserManager.FetchUser(id, LICQ_PPID, LOCK_R);
      if (u == NULL)
      {
        gLog.Warn("%sConnection from unknown user.\n", L_WARNxSTR);
        return false;
      }
      unsigned long nIntIp = u->IntIp();
      unsigned long nIp = u->Ip();
      gUserManager.DropUser(u);
      /* This might prevent connections from clients behind assymetric
         connections (i.e. direct to ICQ server and through socks to clients)
         but they should be using v6+ anyway */
      if (nIntIp != s->RemoteIp() && nIp != s->RemoteIp())
      {
        char ipbuf[32];
        return false;
        gLog.Warn("%sConnection from %s as %s possible spoof.\n", L_WARNxSTR,
            s->RemoteIpStr(ipbuf), id);
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
      nVersion = 2;

      const ICQUser* u = gUserManager.FetchUser(id, LICQ_PPID, LOCK_R);
      if (u == NULL)
      {
        gLog.Warn("%sConnection from unknown user.\n", L_WARNxSTR);
        return false;
      }
      unsigned long nIntIp = u->IntIp();
      unsigned long nIp = u->Ip();
      gUserManager.DropUser(u);
      /* This might prevent connections from clients behind assymetric
         connections (i.e. direct to ICQ server and through socks to clients)
         but they should be using v6+ anyway */
      if (nIntIp != s->RemoteIp() && nIp != s->RemoteIp())
      {
        char ipbuf[32];
        return false;
        gLog.Warn("%sConnection from %s as %s possible spoof.\n", L_WARNxSTR,
            s->RemoteIpStr(ipbuf), id);
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
    {
      char *buf;
      gLog.Unknown("%sUnknown TCP handshake packet :\n%s\n",
                   L_UNKNOWNxSTR, b.print(buf));
      delete [] buf;
      return false;
    }
  }

  s->SetOwner(id, LICQ_PPID);
  s->SetVersion(nVersion);

  return true;

sock_error:
  char buf[128];
  if (s->Error() == 0)
    gLog.Warn(tr("%sHandshake error, remote side closed connection.\n"), L_WARNxSTR);
  else
    gLog.Warn(tr("%sHandshake socket error:\n%s%s.\n"), L_WARNxSTR, L_BLANKxSTR, s->ErrorStr(buf, 128));
  return false;
}

bool CICQDaemon::Handshake_SendConfirm_v7(TCPSocket *s)
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

bool CICQDaemon::Handshake_RecvConfirm_v7(TCPSocket *s)
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
      gLog.Warn("%sHandshake confirm not the right size.\n", L_WARNxSTR);
      return false;
    }
    unsigned char c = b.UnpackChar();
    unsigned long l = b.UnpackUnsignedLong();
    if (c != 0x03 || l != 0x0000000A)
    {
      gLog.Warn("%sUnknown handshake response %2X,%8lX.\n", L_WARNxSTR, c, l);
      return false;
    }
    b.Reset();
    CPacketTcp_Handshake_Confirm p_confirm_in(&b);
    if (p_confirm_in.Channel() != ICQ_CHNxUNKNOWN)
      s->SetChannel(p_confirm_in.Channel());
    else
    {
      gLog.Warn("%sUnknown channel in ack packet.\n", L_WARNxSTR);
      return false;
    }

    s->ClearRecvBuffer();

    CPacketTcp_Handshake_Confirm p_confirm_out(p_confirm_in.Channel(),
                                                       p_confirm_in.Id());

    if (s->SendPacket(p_confirm_out.getBuffer()))
      return true;
  }
 
 sock_error:
  char buf[128];
  if (s->Error() == 0)
    gLog.Warn(tr("%sHandshake error, remote side closed connection.\n"), L_WARNxSTR);
  else
    gLog.Warn(tr("%sHandshake socket error:\n%s%s.\n"), L_WARNxSTR, L_BLANKxSTR, s->ErrorStr(buf, 128));
  return false;
}

/*------------------------------------------------------------------------------
 * ProcessTcpHandshake
 *
 * Takes the first buffer from a socket and parses it as a icq handshake.
 * Does not check that the given user already has a socket or not.
 *----------------------------------------------------------------------------*/
bool CICQDaemon::ProcessTcpHandshake(TCPSocket *s)
{
  if (!Handshake_Recv(s, 0)) return false;
  const char* id = s->OwnerId();
  unsigned long ppid = s->OwnerPPID();
  if (id == NULL)
    return false;

  ICQUser *u = gUserManager.FetchUser(id, ppid, LOCK_W);
  if (u != NULL)
  {
    gLog.Info(tr("%sConnection from %s (%s) [v%ld].\n"), L_TCPxSTR,
       u->GetAlias(), id, s->Version());
    if (u->SocketDesc(s->Channel()) != s->Descriptor())
    {
      if (u->SocketDesc(s->Channel()) != -1)
      {
        gLog.Warn(tr("%sUser %s (%s) already has an associated socket.\n"),
                  L_WARNxSTR, u->GetAlias(), id);
        gUserManager.DropUser(u);
        return true;
/*        gSocketManager.CloseSocket(u->SocketDesc(s->Channel()), false);
        u->ClearSocketDesc(s->Channel());*/
      }
      u->SetSocketDesc(s);
    }
    gUserManager.DropUser(u);
  }
  else
  {
    gLog.Info(tr("%sConnection from new user (%s) [v%ld].\n"), L_TCPxSTR,
       id, s->Version());
  }

  //awaken waiting threads, maybe unnecessarily, but doesn't hurt
  pthread_cond_broadcast(&cond_reverseconnect_done);

  return true;
}


bool CICQDaemon::CryptoEnabled()
{
#ifdef USE_OPENSSL
  return true;
#else
  return false;
#endif
}


void CICQDaemon::StupidChatLinkageFix()
{
  CChatManager *chatman = new CChatManager(NULL, 0);
  delete chatman;
  CFileTransferManager *ftman = new CFileTransferManager(NULL, 0);
  delete ftman;
}


