// -*- c-basic-offset: 2 -*-
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

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
#include "support.h"

//-----ICQ::sendMessage--------------------------------------------------------
unsigned long CICQDaemon::icqSendMessage(unsigned long _nUin, const char *m,
   bool online, unsigned short nLevel, bool bMultipleRecipients,
   CICQColor *pColor)
{
  if (_nUin == gUserManager.OwnerUin() || m == NULL) return 0;

  ICQEvent *result = NULL;
  char *mDos = NULL;
  if (m != NULL)
  {
    mDos = gTranslator.NToRN(m);
    gTranslator.ClientToServer(mDos);
  }
  CEventMsg *e = NULL;

  unsigned long f = INT_VERSION;
  if (online) f |= E_DIRECT;
  if (nLevel == ICQ_TCPxMSG_URGENT) f |= E_URGENT;
  if (bMultipleRecipients) f |= E_MULTIxREC;

	ICQUser *u = gUserManager.FetchUser(_nUin, LOCK_W);
  if (!online) // send offline
  {
     e = new CEventMsg(m, ICQ_CMDxSND_THRUxSERVER, TIME_NOW, f);
     if (strlen(mDos) > MAX_MESSAGE_SIZE)
     {
       gLog.Warn("%sTruncating message to %d characters to send through server.\n",
                 L_WARNxSTR, MAX_MESSAGE_SIZE);
       mDos[MAX_MESSAGE_SIZE] = '\0';
     }
     result = icqSendThroughServer(_nUin, ICQ_CMDxSUB_MSG | (bMultipleRecipients ? ICQ_CMDxSUB_FxMULTIREC : 0),
                                   mDos, e);
  }
  else        // send direct
  {
    if (u == NULL) return 0;
    if (u->Secure()) f |= E_ENCRYPTED;
    e = new CEventMsg(m, ICQ_CMDxTCP_START, TIME_NOW, f);
    if (pColor != NULL) e->SetColor(pColor);
    CPT_Message *p = new CPT_Message(mDos, nLevel, bMultipleRecipients, pColor, u);
    gLog.Info("%sSending %smessage to %s (#%ld).\n", L_TCPxSTR,
       nLevel == ICQ_TCPxMSG_URGENT ? "urgent " : "",
       u->GetAlias(), -p->Sequence());
    result = SendExpectEvent_Client(u, p, e);
  }

  if (u != NULL)
  {
    u->SetSendServer(!online);
    u->SetSendLevel(nLevel);
  }
  gUserManager.DropUser(u);
  if (pColor != NULL) CICQColor::SetDefaultColors(pColor);

  if (mDos)
    delete [] mDos;

  return result->EventId();
}


//-----CICQDaemon::sendReadAwayMsg---------------------------------------------
unsigned long CICQDaemon::icqFetchAutoResponse(unsigned long nUin)
{
  if (nUin == gUserManager.OwnerUin()) return 0;

  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
  CPT_ReadAwayMessage *p = new CPT_ReadAwayMessage(u);
  gLog.Info("%sRequesting auto response from %s (#%ld).\n", L_TCPxSTR,
            u->GetAlias(), -p->Sequence());
  ICQEvent *result = SendExpectEvent_Client(u, p, NULL);
  gUserManager.DropUser(u);

  return result->EventId();
}


//-----CICQDaemon::sendUrl-----------------------------------------------------
unsigned long CICQDaemon::icqSendUrl(unsigned long _nUin, const char *url,
   const char *description, bool online, unsigned short nLevel,
   bool bMultipleRecipients, CICQColor *pColor)
{
  if (_nUin == gUserManager.OwnerUin()) return 0;

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

  ICQUser *u = gUserManager.FetchUser(_nUin, LOCK_W);
  if (!online) // send offline
  {
    e = new CEventUrl(url, description, ICQ_CMDxSND_THRUxSERVER, TIME_NOW, f);
    result = icqSendThroughServer(_nUin, ICQ_CMDxSUB_URL | (bMultipleRecipients ? ICQ_CMDxSUB_FxMULTIREC : 0), m, e);
  }
  else
  {
    if (u == NULL) return 0;
    if (u->Secure()) f |= E_ENCRYPTED;
    e = new CEventUrl(url, description, ICQ_CMDxTCP_START, TIME_NOW, f);
    if (pColor != NULL) e->SetColor(pColor);
    CPT_Url *p = new CPT_Url(m, nLevel, bMultipleRecipients, pColor, u);
    gLog.Info("%sSending %sURL to %s (#%ld).\n", L_TCPxSTR,
       nLevel == ICQ_TCPxMSG_URGENT ? "urgent " : "",
       u->GetAlias(), -p->Sequence());
    result = SendExpectEvent_Client(u, p, e);
  }
  if (u != NULL)
  {
    u->SetSendServer(!online);
    u->SetSendLevel(nLevel);
  }
  gUserManager.DropUser(u);
  if (pColor != NULL) CICQColor::SetDefaultColors(pColor);

  if (szDescDos)
    delete [] szDescDos;

  return result->EventId();
}


//-----CICQDaemon::sendFile---------------------------------------------------
unsigned long CICQDaemon::icqFileTransfer(unsigned long nUin, const char *szFilename,
                        const char *szDescription, unsigned short nLevel, bool bServer)
{
  if (nUin == gUserManager.OwnerUin()) return 0;

  ICQEvent *result = NULL;
  char *szDosDesc = NULL;
  if (szDescription != NULL)
  {
    szDosDesc = gTranslator.NToRN(szDescription);
    gTranslator.ClientToServer(szDosDesc);
  }
  CEventFile *e = NULL;

  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
  if (u == NULL) return 0;

	if (bServer)
	{
		CPU_FileTransfer *p = new CPU_FileTransfer(u, szFilename, szDosDesc,
                                (u->Version() > 7));

		if (!p->IsValid())
		{
			delete p;
			result = NULL;
		}
		else
		{
			e = new CEventFile(szFilename, p->GetDescription(), p->GetFileSize(),
								p->Sequence(), TIME_NOW, INT_VERSION);
			gLog.Info("%sSending file transfer to %s (#%ld).\n", L_SRVxSTR,
				u->GetAlias(), -p->Sequence());

			result = SendExpectEvent_Server(u->Uin(), p, e);
		}
	}
	else
	{
		CPT_FileTransfer *p = new CPT_FileTransfer(szFilename, szDosDesc, nLevel, u);

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
												 p->Sequence(), TIME_NOW, f);
			gLog.Info("%sSending %sfile transfer to %s (#%ld).\n", L_TCPxSTR,
				nLevel == ICQ_TCPxMSG_URGENT ? "urgent " : "",
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


//-----CICQDaemon::sendContactList-------------------------------------------
unsigned long CICQDaemon::icqSendContactList(unsigned long nUin,
   UinList &uins, bool online, unsigned short nLevel, bool bMultipleRecipients,
   CICQColor *pColor)
{
  if (nUin == gUserManager.OwnerUin()) return 0;

  char *m = new char[3 + uins.size() * 80];
  int p = sprintf(m, "%d%c", uins.size(), char(0xFE));
  ContactList vc;

  ICQUser *u = NULL;
  UinList::iterator iter;
  for (iter = uins.begin(); iter != uins.end(); iter++)
  {
    u = gUserManager.FetchUser(*iter, LOCK_R);
    p += sprintf(&m[p], "%ld%c%s%c", *iter, char(0xFE),
       u == NULL ? "" : u->GetAlias(), char(0xFE));
    vc.push_back(new CContact(*iter, u == NULL ? "" : u->GetAlias()));
    gUserManager.DropUser(u);
  }

  if (!online && p > MAX_MESSAGE_SIZE)
  {
    gLog.Warn("%sContact list too large to send through server.\n", L_WARNxSTR);
    delete []m;
    return 0;
  }

  CEventContactList *e = NULL;
  ICQEvent *result = NULL;

  unsigned long f = INT_VERSION;
  if (online) f |= E_DIRECT;
  if (nLevel == ICQ_TCPxMSG_URGENT) f |= E_URGENT;
  if (bMultipleRecipients) f |= E_MULTIxREC;

  u = gUserManager.FetchUser(nUin, LOCK_W);
  if (!online) // send offline
  {
    e = new CEventContactList(vc, false, ICQ_CMDxSND_THRUxSERVER, TIME_NOW, f);
    result = icqSendThroughServer(nUin, ICQ_CMDxSUB_CONTACTxLIST | (bMultipleRecipients ? ICQ_CMDxSUB_FxMULTIREC : 0), m, e);
  }
  else
  {
    if (u == NULL) return 0;
    if (u->Secure()) f |= E_ENCRYPTED;
    e = new CEventContactList(vc, false, ICQ_CMDxTCP_START, TIME_NOW, f);
    if (pColor != NULL) e->SetColor(pColor);
    CPT_ContactList *p = new CPT_ContactList(m, nLevel, bMultipleRecipients, pColor, u);
    gLog.Info("%sSending %scontact list to %s (#%ld).\n", L_TCPxSTR,
       nLevel == ICQ_TCPxMSG_URGENT ? "urgent " : "",
       u->GetAlias(), -p->Sequence());
    result = SendExpectEvent_Client(u, p, e);
  }
  if (u != NULL)
  {
    u->SetSendServer(!online);
    u->SetSendLevel(nLevel);
  }
  gUserManager.DropUser(u);
  if (pColor != NULL) CICQColor::SetDefaultColors(pColor);

  delete []m;
  return result->EventId();
}



//-----CICQDaemon::fileCancel-------------------------------------------------------------------------
void CICQDaemon::icqFileTransferCancel(unsigned long nUin, unsigned long nSequence)
{
  // add to history ??
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  if (u == NULL) return;
  gLog.Info("%sCancelling file transfer to %s (#%ld).\n", L_TCPxSTR,
     u->GetAlias(), -nSequence);
  CPT_CancelFile p(nSequence, u);
  AckTCP(p, u->SocketDesc());
  gUserManager.DropUser(u);
}


//-----CICQDaemon::fileAccept-----------------------------------------------------------------------------
void CICQDaemon::icqFileTransferAccept(unsigned long nUin, unsigned short nPort,
   unsigned long nSequence, unsigned long nMsgID[2], bool bDirect)
{
   // basically a fancy tcp ack packet which is sent late
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  if (u == NULL) return;
	gLog.Info("%sAccepting file transfer from %s (#%ld).\n",
						bDirect ? L_TCPxSTR : L_SRVxSTR, u->GetAlias(), -nSequence);
	if (bDirect)
	{
		CPT_AckFileAccept p(nPort, nSequence, u);
		AckTCP(p, u->SocketDesc());
	}
	else
	{
		CPU_AckFileAccept *p = new CPU_AckFileAccept(u, nMsgID, nSequence, nPort);
		SendEvent_Server(p);
	}

  gUserManager.DropUser(u);
}



//-----CICQDaemon::fileRefuse-----------------------------------------------------------------------------
void CICQDaemon::icqFileTransferRefuse(unsigned long nUin, const char *szReason,
   unsigned long nSequence, unsigned long nMsgID[2], bool bDirect)
{
   // add to history ??
  char *szReasonDos = gTranslator.NToRN(szReason);
  gTranslator.ClientToServer(szReasonDos);
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  if (u == NULL) return;
  gLog.Info("%sRefusing file transfer from %s (#%ld).\n",
						bDirect ? L_TCPxSTR : L_SRVxSTR, u->GetAlias(), -nSequence);

	if (bDirect)
	{
		CPT_AckFileRefuse p(szReasonDos, nSequence, u);
		AckTCP(p, u->SocketDesc());
  }
	else
	{
		CPU_AckFileRefuse *p = new CPU_AckFileRefuse(nUin, nMsgID, nSequence,
																								 szReasonDos);
		SendEvent_Server(p);
	}

	gUserManager.DropUser(u);

  if (szReasonDos)
    delete [] szReasonDos;
}


//-----CICQDaemon::sendChat------------------------------------------------------------
unsigned long CICQDaemon::icqChatRequest(unsigned long nUin, const char *szReason,
																				 unsigned short nLevel, bool bServer)
{
  return icqMultiPartyChatRequest(nUin, szReason, NULL, 0, nLevel, bServer);
}


unsigned long CICQDaemon::icqMultiPartyChatRequest(unsigned long nUin,
   const char *reason, const char *szChatUsers, unsigned short nPort,
   unsigned short nLevel, bool bServer)
{
  if (nUin == gUserManager.OwnerUin()) return 0;

  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
  if (u == NULL) return 0;
  char *szReasonDos = gTranslator.NToRN(reason);
  gTranslator.ClientToServer(szReasonDos);

	unsigned long f;
	ICQEvent *result = NULL;
	if (bServer)
	{
		CPU_ChatRequest *p = new CPU_ChatRequest(szReasonDos, szChatUsers, u,
                               (u->Version() > 7));
		f = INT_VERSION;
		CEventChat *e = new CEventChat(reason, szChatUsers, nPort, p->Sequence(),
																	 TIME_NOW, f);
		gLog.Info("%sSending chat request to %s (#%ld).\n", L_SRVxSTR,
							u->GetAlias(), -p->Sequence());

		result = SendExpectEvent_Server(u->Uin(), p, e);
	}
	else
	{
		CPT_ChatRequest *p = new CPT_ChatRequest(szReasonDos, szChatUsers, nPort,
																						 nLevel, u);
		f = E_DIRECT | INT_VERSION;
		if (nLevel == ICQ_TCPxMSG_URGENT) f |= E_URGENT;
		if (u->Secure()) f |= E_ENCRYPTED;
		CEventChat *e = new CEventChat(reason, szChatUsers, nPort, p->Sequence(),
																	 TIME_NOW, f);
		gLog.Info("%sSending %schat request to %s (#%ld).\n", L_TCPxSTR,
							nLevel == ICQ_TCPxMSG_URGENT ? "urgent " : "",
							u->GetAlias(), -p->Sequence());
		result = SendExpectEvent_Client(u, p, e);
	}
	
	u->SetSendServer(bServer);
  u->SetSendLevel(nLevel);
  gUserManager.DropUser(u);

  if (szReasonDos)
    delete [] szReasonDos;
  return result->EventId();
}


//-----CICQDaemon::chatCancel----------------------------------------------------------
void CICQDaemon::icqChatRequestCancel(unsigned long nUin, unsigned long nSequence)
{
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  if (u == NULL) return;
  gLog.Info("%sCancelling chat request with %s (#%ld).\n", L_TCPxSTR,
     u->GetAlias(), -nSequence);
  CPT_CancelChat p(nSequence, u);
  AckTCP(p, u->SocketDesc());
  gUserManager.DropUser(u);
}


//-----CICQDaemon::chatRefuse-----------------------------------------------------------------------------
void CICQDaemon::icqChatRequestRefuse(unsigned long nUin, const char *szReason,
   unsigned long nSequence, unsigned long nMsgID[2], bool bDirect)
{
  // add to history ??
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  if (u == NULL) return;
  gLog.Info("%sRefusing chat request with %s (#%ld).\n",
						bDirect ? L_TCPxSTR : L_SRVxSTR, u->GetAlias(), -nSequence);
  char *szReasonDos = gTranslator.NToRN(szReason);
  gTranslator.ClientToServer(szReasonDos);

	if (bDirect)
	{
		CPT_AckChatRefuse p(szReasonDos, nSequence, u);
		AckTCP(p, u->SocketDesc());
	}
	else
	{
		CPU_AckChatRefuse *p = new CPU_AckChatRefuse(u->Uin(), nMsgID, nSequence,
																								 szReasonDos);
		SendEvent_Server(p);
	}

  gUserManager.DropUser(u);

  if (szReasonDos)
    delete [] szReasonDos;
}


//-----CICQDaemon::chatAccept-----------------------------------------------------------------------------
void CICQDaemon::icqChatRequestAccept(unsigned long nUin, unsigned short nPort,
   unsigned long nSequence, unsigned long nMsgID[2], bool bDirect)
{
  // basically a fancy tcp ack packet which is sent late
  // add to history ??
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  if (u == NULL) return;
  gLog.Info("%sAccepting chat request with %s (#%ld).\n",
						bDirect ? L_TCPxSTR : L_SRVxSTR, u->GetAlias(), -nSequence);

	if (bDirect)
	{
		CPT_AckChatAccept p(nPort, nSequence, u);
		AckTCP(p, u->SocketDesc());
	}
	else
	{
		CPU_AckChatAccept *p = new CPU_AckChatAccept(nUin, nMsgID, nSequence,
																								 nPort);
		SendEvent_Server(p);
	}

  gUserManager.DropUser(u);
}

/*---------------------------------------------------------------------------
 * OpenSSL stuff
 *-------------------------------------------------------------------------*/

unsigned long CICQDaemon::icqOpenSecureChannel(unsigned long nUin)
{
  if (nUin == gUserManager.OwnerUin()) return 0;

#ifdef USE_OPENSSL
  ICQEvent *result = NULL;

  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
  if (u == NULL)
  {
    gLog.Warn("%sCannot send secure channel request to user not on list (%ld).\n",
       L_WARNxSTR, nUin);
    return 0;
  }

  // Check that the user doesn't already have a secure channel
  if (u->Secure())
  {
    gLog.Warn("%s%s (%ld) already has a secure channel.\n", L_WARNxSTR,
       u->GetAlias(), nUin);
    gUserManager.DropUser(u);
    return 0;
  }

  CPT_OpenSecureChannel *pkt = new CPT_OpenSecureChannel(u);
  gLog.Info("%sSending request for secure channel to %s (#%ld).\n", L_TCPxSTR,
     u->GetAlias(), -pkt->Sequence());
  result = SendExpectEvent_Client(u, pkt, NULL);

  u->SetSendServer(false);

  gUserManager.DropUser(u);

  return result->EventId();

#else // No OpenSSL
  gLog.Warn("%sicqOpenSecureChannel() to %ld called when we do not support OpenSSL.\n",
     L_WARNxSTR, nUin);
  return 0;

#endif
}


unsigned long CICQDaemon::icqCloseSecureChannel(unsigned long nUin)
{
#ifdef USE_OPENSSL
  ICQEvent *result = NULL;

  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
  if (u == NULL)
  {
    gLog.Warn("%sCannot send secure channel request to user not on list (%ld).\n",
       L_WARNxSTR, nUin);
    return 0;
  }

  // Check that the user doesn't already have a secure channel
  if (!u->Secure())
  {
    gLog.Warn("%s%s (%ld) does not have a secure channel.\n", L_WARNxSTR,
       u->GetAlias(), nUin);
    gUserManager.DropUser(u);
    return 0;
  }

  CPT_CloseSecureChannel *pkt = new CPT_CloseSecureChannel(u);
  gLog.Info("%sClosing secure channel with %s (#%ld).\n", L_TCPxSTR,
     u->GetAlias(), -pkt->Sequence());
  result = SendExpectEvent_Client(u, pkt, NULL);

  u->SetSendServer(false);

  gUserManager.DropUser(u);

  return result->EventId();

#else // No OpenSSL
  gLog.Warn("%sicqCloseSecureChannel() to %ld called when we do not support OpenSSL.\n",
     L_WARNxSTR, nUin);
  return 0;

#endif
}


//-----CICQDaemon::keyCancel-------------------------------------------------------------------------
void CICQDaemon::icqOpenSecureChannelCancel(unsigned long nUin, unsigned long nSequence)
{
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
  if (u == NULL) return;
  gLog.Info("%sCancelling secure channel request to %s (#%ld).\n", L_TCPxSTR,
     u->GetAlias(), -nSequence);
  // XXX Tear down tcp connection ??
  gUserManager.DropUser(u);
}






/*---------------------------------------------------------------------------
 * Handshake
 *
 * Shake hands on the given socket with the given user.
 *-------------------------------------------------------------------------*/
bool CICQDaemon::Handshake_Send(TCPSocket *s, unsigned long nUin,
   unsigned short nPort, unsigned short nVersion, bool bConfirm)
{
  s->SetVersion(nVersion);
  s->SetOwner(nUin);

  switch (nVersion)
  {
    case 2:
    case 3:
    {
      CPacketTcp_Handshake_v2 p(s->LocalPort());
      if (!s->SendPacket(p.getBuffer())) goto sock_error;
      return true;
    }
    case 4:
    {
      CPacketTcp_Handshake_v4 p(s->LocalPort());
      if (!s->SendPacket(p.getBuffer())) goto sock_error;
      return true;
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
        gLog.Warn("%sBad handshake ack: %ld.\n", L_WARNxSTR, nOk);
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
        gLog.Warn("%sBad handshake session id: received %ld, expecting %ld.\n",
           L_WARNxSTR, p_in.SessionId(), p.SessionId());
        return false;
      }

      // Send the hanshake ack
      CPacketTcp_Handshake_Ack p_ack;
      if (!s->SendPacket(p_ack.getBuffer())) goto sock_error;

      return true;
    }

    case 7:
    case 8:
    {
      // Send the hanshake
      CPacketTcp_Handshake_v7 p(nUin, 0, nPort);
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
        gLog.Warn("%sBad handshake ack: %ld.\n", L_WARNxSTR, nOk);
        return false;
      }

      // Wait for the reverse handshake
      do
      {
        if (!s->RecvPacket()) goto sock_error;
      } while (!s->RecvBufferFull());
      CPacketTcp_Handshake_v7 p_in(&s->RecvBuffer());
      s->ClearRecvBuffer();
      if (p.SessionId() != p_in.SessionId())
      {
        gLog.Warn("%sBad handshake cookie: received %ld, expecting %ld.\n",
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
				// Send handshake accepted
				CPacketTcp_Handshake_Confirm p_confirm;
				if (!s->SendPacket(p_confirm.getBuffer())) goto sock_error;

				// Wait for reverse handshake accepted
				do
				{
					if (!s->RecvPacket()) goto sock_error;
				} while (!s->RecvBufferFull());
				s->ClearRecvBuffer();
			}

      return true;
    }

    default:
      // Should never happen
      gLog.Error("%sUnknown ICQ TCP version (%d).\n", L_ERRORxSTR, nVersion);
      return false;
  }

sock_error:
  char buf[128];
  if (s->Error() == 0)
    gLog.Warn("%sHandshake error, remote side closed connection.\n", L_WARNxSTR);
  else
    gLog.Warn("%sHandshake socket error:\n%s%s.\n", L_WARNxSTR, L_BLANKxSTR, s->ErrorStr(buf, 128));
  return false;
}


/*------------------------------------------------------------------------------
 * ConnectToUser
 *
 * Creates a new TCPSocket and connects it to a given user.  Adds the socket
 * to the global socket manager and to the user.
 *----------------------------------------------------------------------------*/
int CICQDaemon::ConnectToUser(unsigned long nUin)
{
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
  if (u == NULL) return -1;

  int sd = u->SocketDesc();

  // Check that we need to connect at all
  if (sd != -1)
  {
    gUserManager.DropUser(u);
    gLog.Warn("%sConnection attempted to already connected user (%ld).\n",
       L_WARNxSTR, nUin);
    return sd;
  }

  char szAlias[64];
  strncpy(szAlias, u->GetAlias(), sizeof(szAlias));
  unsigned short nPort = u->Port();
  unsigned short nVersion = u->ConnectionVersion();

  // Poll if there is a connection in progress already
  while (u->ConnectionInProgress())
  {
    gUserManager.DropUser(u);
    struct timeval tv = { 2, 0 };
    if (select(0, NULL, NULL, NULL, &tv) == -1 && errno == EINTR) return -1;
    u = gUserManager.FetchUser(nUin, LOCK_W);
    if (u == NULL) return -1;
  }
  sd = u->SocketDesc();
  if (sd == -1) u->SetConnectionInProgress(true);
  gUserManager.DropUser(u);
  if (sd != -1) return sd;

  TCPSocket *s = new TCPSocket(nUin);
  if (!OpenConnectionToUser(nUin, s, nPort))
  {
    u = gUserManager.FetchUser(nUin, LOCK_W);
    if (u != NULL) u->SetConnectionInProgress(false);
    gUserManager.DropUser(u);
    delete s;
    return -1;
  }

  gLog.Info("%sShaking hands with %s (%ld) [v%d].\n", L_TCPxSTR,
     szAlias, nUin, nVersion);
  nPort = s->LocalPort();

  if (!Handshake_Send(s, nUin, 0, nVersion))
  {
    u = gUserManager.FetchUser(nUin, LOCK_W);
    if (u != NULL) u->SetConnectionInProgress(false);
    gUserManager.DropUser(u);
    delete s;
    return -1;
  }
  s->SetVersion(nVersion);
  int nSD = s->Descriptor();

  // Set the socket descriptor in the user
  u = gUserManager.FetchUser(nUin, LOCK_W);
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
bool CICQDaemon::OpenConnectionToUser(unsigned long nUin, TCPSocket *sock,
                                      unsigned short nPort)
{
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  if (u == NULL) return false;

  char szAlias[64];
  snprintf(szAlias, sizeof(szAlias), "%s (%ld)", u->GetAlias(), u->Uin());
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
    gLog.Info("%sConnecting to %s at %s:%d.\n", L_TCPxSTR, szAlias,
      ip_ntoa(nIp, buf), nPort);
    // If we fail to set the remote address, the ip must be 0
    if (!sock->SetRemoteAddr(nIp, nPort)) return false;

    if (!sock->OpenConnection())
    {
      gLog.Warn("%sConnect to %s failed:\n%s%s.\n", L_WARNxSTR, szAlias,
                L_BLANKxSTR, sock->ErrorStr(buf, 128));

      // Now try the internal ip if it is different from this one and we are behind a firewall
      if (sock->Error() != EINTR && nIntIp != nIp &&
          nIntIp != 0 && CPacket::Firewall())
      {
        gLog.Info("%sConnecting to %s at %s:%d.\n", L_TCPxSTR, szAlias,
                  ip_ntoa(nIntIp, buf), nPort);
        sock->SetRemoteAddr(nIntIp, nPort);

        if (!sock->OpenConnection())
        {
          char buf[128];
          gLog.Warn("%sConnect to %s real ip failed:\n%s%s.\n", L_WARNxSTR, szAlias,
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
    gLog.Info("%sConnecting to %s at %s:%d.\n", L_TCPxSTR, szAlias,
       ip_ntoa(nIntIp, buf), nPort);
    if (!sock->SetRemoteAddr(nIntIp, nPort)) return false;

    if (!sock->OpenConnection())
    {
      gLog.Warn("%sConnect to %s real ip failed:\n%s%s.\n", L_WARNxSTR, szAlias,
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
int CICQDaemon::ReverseConnectToUser(unsigned long nUin, unsigned long nIp,
   unsigned short nPort, unsigned short nVersion, unsigned short nFailedPort)
{
  TCPSocket *s = new TCPSocket(nUin);
  char buf[32];

  gLog.Info("%sReverse connecting to %ld at %s:%d.\n", L_TCPxSTR, nUin,
            ip_ntoa(nIp, buf), nPort);

  // If we fail to set the remote address, the ip must be 0
  s->SetRemoteAddr(nIp, nPort);

  if (!s->OpenConnection())
  {
    char buf[128];
    gLog.Warn("%sReverse connect to %ld failed:\n%s%s.\n", L_WARNxSTR,
              nUin, L_BLANKxSTR, s->ErrorStr(buf, 128));
    return -1;
  }

  gLog.Info("%sReverse shaking hands with %ld.\n", L_TCPxSTR, nUin);
  if (!Handshake_Send(s, nUin, 0, nVersion))
  {
    delete s;
    return -1;
  }
  s->SetVersion(nVersion);
  int nSD = s->Descriptor();

  // Find which socket this is for
  TCPSocket *tcp = (TCPSocket *)gSocketManager.FetchSocket(m_nTCPSocketDesc);
  unsigned short tcpPort = tcp ? tcp->LocalPort() : 0;
  gSocketManager.DropSocket(tcp);

  CFileTransferManager *ftm = NULL;
  CChatManager *cm = NULL;

  // Check if it's the main port
  if (nFailedPort == tcpPort || nFailedPort == 0)
  {
    // Set the socket descriptor in the user if this user is on our list
    ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
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

  // File transfer port
  else if ( (ftm = CFileTransferManager::FindByPort(nFailedPort)) != NULL)
  {
     ftm->AcceptReverseConnection(s);
     delete s;
  }

  // Chat port
  else if ( (cm = CChatManager::FindByPort(nFailedPort)) != NULL)
  {
     cm->AcceptReverseConnection(s);
     delete s;
  }

  // What the--?
  else
  {
    gLog.Warn("%sReverse connection to unknown port (%d).\n", L_WARNxSTR, nFailedPort);
    delete s;
  }

  return nSD;
}


//-----CICQDaemon::ProcessTcpPacket----------------------------------------------------
bool CICQDaemon::ProcessTcpPacket(TCPSocket *pSock)
{
  unsigned long nUin, theSequence, senderIp, localIp,
                senderPort, junkLong, nPort, nPortReversed;
  unsigned short version, command, junkShort, newCommand, messageLen,
                headerLen, ackFlags, msgFlags, licqVersion;
  char licqChar, junkChar;
  bool errorOccured = false;

  // only used for v7,v8
	headerLen = 0;

  CBuffer &packet = pSock->RecvBuffer();
  int sockfd = pSock->Descriptor();

  // Static variables to keep track of repeating chat/file ack packets
  // THIS IS SHIT AND NEEDS TO BE CHANGED FIXME
  static unsigned long s_nChatUin, s_nChatSequence, s_nFileUin, s_nFileSequence;

  unsigned short nInVersion = pSock->Version();

  switch (nInVersion)
  {
    case 1:
    case 2:
    case 3:
    {
      packet >> nUin
             >> version
             >> command      // main tcp command (start, cancel, ack)
             >> junkShort    // 00 00 to fill in the MSB of the command long int which is read in as a short
             >> nUin
             >> newCommand   // sub command (message/chat/read away message/...)
             >> messageLen   // length of incoming message
      ;
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
      packet >> nUin
             >> version
             >> junkLong     // checksum
             >> command      // main tcp command (start, cancel, ack)
             >> junkShort    // 00 00 to fill in the MSB of the command long int which is read in as a short
             >> nUin
             >> newCommand   // sub command (message/chat/read away message/...)
             >> messageLen   // length of incoming message
      ;
      break;
    }
    case 6:
    {
      nUin = pSock->Owner();
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
      nUin = pSock->Owner();
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
      headerLen = packet.UnpackUnsignedShort(); // 0x000E
      theSequence = (signed short)packet.UnpackUnsignedShort();
      packet.incDataPosRead(headerLen - 2);
      newCommand = packet.UnpackUnsignedShort();
      ackFlags = packet.UnpackUnsignedShort();
      msgFlags = packet.UnpackUnsignedShort();
      packet >> messageLen;
      break;
    }
    default:
    {
      gLog.Warn("%sUnknown TCP version %d from socket.\n", L_WARNxSTR, nInVersion);
      break;
    }
  }

  // Some simple validation of the packet
  if (nUin == 0 || command == 0 || newCommand == 0)
  {
    char *buf;
    gLog.Unknown("%sInvalid TCP packet (uin: %08lx, cmd: %04x, subcmd: %04x):\n%s\n",
                 L_UNKNOWNxSTR, nUin, command, newCommand, packet.print(buf));
    delete [] buf;
    return false;
  }

  if (nUin == gUserManager.OwnerUin() || nUin != pSock->Owner())
  {
    char *buf;
    if(nUin == gUserManager.OwnerUin())
      gLog.Warn("%sTCP message from self (probable spoof):\n%s\n", L_WARNxSTR, packet.print(buf));
    else
      gLog.Warn("%sTCP message from invalid UIN (%ld, expect %ld):\n%s\n",
                L_WARNxSTR, nUin, pSock->Owner(), packet.print(buf));
    delete [] buf;
    return false;
  }

  // Silently leave, this is in v8 clients.  Seems to be saying
  // that you are on their list
  if (headerLen == 0x0012)
    return false;

  // read in the message minus any stupid DOS \r's
  char message[messageLen + 1];
  unsigned short j = 0;
  for (unsigned short i = 0; i < messageLen; i++)
  {
    packet >> junkChar;
    if (junkChar != 0x0D) message[j++] = junkChar;
  }

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


  // Store our status for later use
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  unsigned short nOwnerStatus = o->Status();
  gUserManager.DropOwner();

  // find which user was sent
  bool bNewUser = false;
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
  if (u == NULL)
  {
    u = new ICQUser(nUin);
    u->SetSocketDesc(pSock);
    bNewUser = true;
  }

  // Check for spoofing
  if (u->SocketDesc() != sockfd)
  {
    gLog.Warn("%sUser %s (%ld) socket (%d) does not match incoming message (%d).\n",
              L_TCPxSTR, u->GetAlias(), u->Uin(), u->SocketDesc(), sockfd);
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

  //printf("status: %04X (%04X)  msgtype: %04X\n", ackFlags, u->Status(), msgFlags);

  switch(command)
  {

  //-----START------------------------------------------------------------------
  case ICQ_CMDxTCP_START:
  {
    // Process the status bits
    unsigned short s = msgFlags & 0xFF80, ns = 0;
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
        gLog.Warn("%sUnknown TCP status: %04X\n", L_WARNxSTR, msgFlags);
        break;
    }
    //printf("%08lX\n", (u->StatusFull() & ICQ_STATUS_FxFLAGS) | ns);
    /*if (!bNewUser && ns != ICQ_STATUS_OFFLINE &&
        !((ns & ICQ_STATUS_FxPRIVATE) && u->StatusOffline()))*/
    if (!bNewUser && ns != ICQ_STATUS_OFFLINE &&
        !(ns == ICQ_STATUS_ONLINE && u->Status() == ICQ_STATUS_FREEFORCHAT) &&
        ns != u->Status() | (u->StatusInvisible() ? ICQ_STATUS_FxPRIVATE : 0))
    {
      bool r = u->OfflineOnDisconnect() || u->StatusOffline();
      ChangeUserStatus(u, (u->StatusFull() & ICQ_STATUS_FxFLAGS) | ns);
      gLog.Info("%s%s (%ld) is %s to us.\n", L_TCPxSTR, u->GetAlias(),
         u->Uin(), u->StatusStr());
      if (r) u->SetOfflineOnDisconnect(true);
    }

    // Process the command
    switch(newCommand)
    {
      case ICQ_CMDxSUB_MSG:  // straight message from a user
      {
        unsigned long back = 0xFFFFFF, fore = 0x000000;
        if (nInVersion <= 4)
          packet >> theSequence;
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
					gLog.Info("%sMessage from %s (%ld) [Licq %s].\n", L_TCPxSTR, u->GetAlias(),
             nUin, CUserEvent::LicqVersionToString(licqVersion));
				else
					gLog.Info("%sMessage from %s (%ld).\n", L_TCPxSTR, u->GetAlias(), nUin);

				CEventMsg *e = CEventMsg::Parse(message, ICQ_CMDxTCP_START, TIME_NOW, nMask);
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
            // FIXME should log a message here or in reject event
            // FIXME should either refuse the event or have a special auto response
            // for rejected events instead of pretending to accept the user
            RejectEvent(nUin, e);
            break;
          }
          AddUserToList(u);
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
        if (nInVersion <= 4) packet >> theSequence;
        else packet >> junkLong >> junkLong;
        packet >> licqChar >> licqVersion;
        if (licqChar == 'L')
          gLog.Info("%s%s (%ld) requested auto response [Licq %s].\n", L_TCPxSTR,
             u->GetAlias(), nUin, CUserEvent::LicqVersionToString(licqVersion));
        else
          gLog.Info("%s%s (%ld) requested auto response.\n", L_TCPxSTR, u->GetAlias(), nUin);

        CPT_AckGeneral p(newCommand, theSequence, true, false, u);
        AckTCP(p, pSock);

        m_sStats[STATS_AutoResponseChecked].Inc();
        u->SetLastCheckedAutoResponse();

        PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_EVENTS, nUin));
        break;
      }

      case ICQ_CMDxSUB_URL:  // url sent
      {
        unsigned long back = 0xFFFFFF, fore = 0x000000;
        if (nInVersion <= 4) packet >> theSequence;
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
          gLog.Info("%sURL from %s (%ld) [Licq %s].\n", L_TCPxSTR, u->GetAlias(),
            nUin, CUserEvent::LicqVersionToString(licqVersion));
        else
          gLog.Info("%sURL from %s (%ld).\n", L_TCPxSTR, u->GetAlias(), nUin);

        CEventUrl *e = CEventUrl::Parse(message, ICQ_CMDxTCP_START, TIME_NOW, nMask);
        if (e == NULL)
        {
          char *buf;
          gLog.Warn("%sInvalid URL message:\n%s\n", L_WARNxSTR, packet.print(buf));
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
            RejectEvent(nUin, e);
            break;
          }
          AddUserToList(u);
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
        if (nInVersion <= 4) packet >> theSequence;
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
          gLog.Info("%sContact list from %s (%ld) [Licq %s].\n", L_TCPxSTR,
             u->GetAlias(), nUin, CUserEvent::LicqVersionToString(licqVersion));
        else
          gLog.Info("%sContact list from %s (%ld).\n", L_TCPxSTR, u->GetAlias(),
             nUin);

        CEventContactList *e = CEventContactList::Parse(message, ICQ_CMDxTCP_START, TIME_NOW, nMask);
        if (e == NULL)
        {
          char *buf;
          gLog.Warn("%sInvalid contact list message:\n%s\n", L_WARNxSTR, packet.print(buf));
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
            RejectEvent(nUin, e);
            break;
          }
          AddUserToList(u);
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
        if (nInVersion <= 4) packet >> theSequence;
        packet >> licqChar >> licqVersion;

        if (licqChar == 'L')
          gLog.Info("%sChat request from %s (%ld) [Licq %s].\n", L_TCPxSTR,
             u->GetAlias(), nUin, CUserEvent::LicqVersionToString(licqVersion));
        else
          gLog.Info("%sChat request from %s (%ld).\n", L_TCPxSTR, u->GetAlias(),
             nUin);

        // translating string with translation table
        gTranslator.ServerToClient (message);
        CEventChat *e = new CEventChat(message, szChatClients, nPort, theSequence,
           TIME_NOW, nMask | licqVersion);

        // Add the user to our list if they are new
        if (bNewUser)
        {
          if (Ignore(IGNORE_NEWUSERS))
          {
            RejectEvent(nUin, e);
            break;
          }
          AddUserToList(u);
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
        char szFilename[nLenFilename];
        for (unsigned short i = 0; i < nLenFilename; i++)
           packet >> szFilename[i];
        packet >> nFileLength
               >> junkLong;
        if (nInVersion <= 4) packet >> theSequence;
        packet >> licqChar >> licqVersion;

        if (licqChar == 'L')
          gLog.Info("%sFile transfer request from %s (%ld) [Licq %s].\n",
             L_TCPxSTR, u->GetAlias(), nUin, CUserEvent::LicqVersionToString(licqVersion));
        else
          gLog.Info("%sFile transfer request from %s (%ld).\n", L_TCPxSTR,
             u->GetAlias(), nUin);

        // translating string with translation table
        gTranslator.ServerToClient (message);
        CEventFile *e = new CEventFile(szFilename, message, nFileLength,
                                       theSequence, TIME_NOW,
                                       nMask | licqVersion);
        // Add the user to our list if they are new
        if (bNewUser)
        {
          if (Ignore(IGNORE_NEWUSERS))
          {
            RejectEvent(nUin, e);
            break;
          }
          AddUserToList(u);
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
					gLog.Info("%sUnknown ICBM plugin type: %s\n", L_TCPxSTR, szPlugin);
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
					char szFilename[nLen];
					for (unsigned short i = 0; i < nLen; i++)
						packet >> szFilename[i];
					packet >> nFileSize;
					packet.incDataPosRead(2); // reversed port (BE)

					gLog.Info("%sFile transfer request from %s (%ld).\n", L_TCPxSTR,
										u->GetAlias(), nUin);

					// translating string with translation table
					gTranslator.ServerToClient(szMessage);
					CEventFile *e = new CEventFile(szFilename, szMessage, nFileSize,
																				 theSequence, TIME_NOW, nMask);
					if (bNewUser)
					{
						if (Ignore(IGNORE_NEWUSERS))
						{
							RejectEvent(nUin, e);
							break;
						}
						AddUserToList(u);
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
					nPortReversed = packet.UnpackUnsignedShortBE();
					packet >> nPort;

					if (nPort == 0)
						nPort = nPortReversed;

					gLog.Info("%sChat request from %s (%ld).\n", L_TCPxSTR,
										u->GetAlias(), nUin);
					
					// translating string with translation table
					gTranslator.ServerToClient(szMessage);
					CEventChat *e = new CEventChat(szMessage, szChatClients, nPort,
																				 theSequence, TIME_NOW, nMask);
					if (bNewUser)
					{
						if (Ignore(IGNORE_NEWUSERS))
						{
							RejectEvent(nUin, e);
							break;
						}
						AddUserToList(u);
						bNewUser = false;
					}

					if (!AddUserEvent(u, e)) break;
					m_xOnEventManager.Do(ON_EVENT_CHAT, u);
					break;
				}
				case ICQ_CMDxSUB_URL:
				{
					gLog.Info("%sURL from %s (%ld).\n", L_TCPxSTR, u->GetAlias(), nUin);
					CEventUrl *e = CEventUrl::Parse(szMessage, ICQ_CMDxTCP_START,
																					TIME_NOW, nMask);
					if (e == NULL)
					{
						char *buf;
						gLog.Warn("%sInvalid URL message:\n%s\n", L_WARNxSTR,
											packet.print(buf));
						delete [] buf;
						errorOccured = true;
						break;
					}

					if (bNewUser)
					{
						if (Ignore(IGNORE_NEWUSERS))
						{
							RejectEvent(nUin, e);
							break;
						}
						AddUserToList(u);
						bNewUser = false;
					}

					if (!AddUserEvent(u, e)) break;
					m_xOnEventManager.Do(ON_EVENT_URL, u);
					break;
				}
				case ICQ_CMDxSUB_CONTACTxLIST:
				{
					gLog.Info("%sContact list from %s (%ld).\n", L_TCPxSTR,
										u->GetAlias(), nUin);
					CEventContactList *e = CEventContactList::Parse(szMessage,
																													ICQ_CMDxTCP_START,
																													TIME_NOW, nMask);
					if (e == NULL)
					{
						char *buf;
						gLog.Warn("%sInvalid contact list message:\n%s\n", L_TCPxSTR,
											packet.print(buf));
						delete [] buf;
						errorOccured = true;
						break;
					}

					if (bNewUser)
					{
						if (Ignore(IGNORE_NEWUSERS))
						{
							RejectEvent(nUin, e);
							break;
						}
						AddUserToList(u);
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
        gLog.Info("%sReceived old-style key request from %s (%ld) but we do not support it.\n",
           L_TCPxSTR, u->GetAlias(), nUin);
        // Send the nack back
        CPT_AckOldSecureChannel p(theSequence, u);
        AckTCP(p, pSock);
        break;
      }

      // Secure channel request
      case ICQ_CMDxSUB_SECURExOPEN:
      {
#ifdef USE_OPENSSL
        if (nInVersion <= 4) packet >> theSequence;
        packet >> licqChar >> licqVersion;

        if (licqChar == 'L')
          gLog.Info("%sSecure channel request from %s (%ld) [Licq %s].\n",
           L_TCPxSTR, u->GetAlias(), nUin, CUserEvent::LicqVersionToString(licqVersion));
        else
          gLog.Info("%sSecure channel request from %s (%ld).\n", L_TCPxSTR,
           u->GetAlias(), nUin);

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
          AddUserToList(u);
          bNewUser = false;
        }

        u->SetSendServer(false);
        PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_SECURITY, nUin, 1));

        gLog.Info("%sSecure channel established with %s (%ld).\n",
         L_SSLxSTR, u->GetAlias(), nUin);

        break;

#else // We do not support OpenSSL
       gLog.Info("%sReceived secure channel request from %s (%ld) but we do not support OpenSSL.\n",
           L_TCPxSTR, u->GetAlias(), nUin);
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
        if (nInVersion <= 4) packet >> theSequence;
        packet >> licqChar >> licqVersion;

        if (licqChar == 'L')
          gLog.Info("%sSecure channel closed by %s (%ld) [Licq %s].\n",
           L_TCPxSTR, u->GetAlias(), nUin, CUserEvent::LicqVersionToString(licqVersion));
        else
          gLog.Info("%sSecure channel closed by %s (%ld).\n", L_TCPxSTR,
           u->GetAlias(), nUin);

        // send ack
        CPT_AckCloseSecureChannel p(theSequence, u);
        AckTCP(p, pSock);

        pSock->SecureStop();
        u->SetSecure(false);
        PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_SECURITY, nUin, 0));
        break;

#else // We do not support OpenSSL
        gLog.Info("%sReceived secure channel close from %s (%ld) but we do not support OpenSSL.\n",
           L_TCPxSTR, u->GetAlias(), nUin);
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
        if (nInVersion <= 4) packet >> theSequence;
        else packet >> junkLong >> junkLong;
        packet >> licqChar >> licqVersion;
        break;

      case ICQ_CMDxSUB_CHAT:
      {
        char ul[1024];
        packet.UnpackString(ul, sizeof(ul));
        packet >> nPortReversed   // port backwards
               >> nPort;    // port to connect to for chat
        if (nInVersion <= 4) packet >> theSequence;
        packet >> licqChar >> licqVersion;

        if (nPort == 0) nPort = (nPortReversed >> 8) | ((nPortReversed & 0xFF) << 8);
        // only if this is the first chat ack packet do we do anything
        if (s_nChatSequence == theSequence && s_nChatUin == nUin)
        {
          gUserManager.DropUser(u);
          return true;
        }
        s_nChatSequence = theSequence;
        s_nChatUin = nUin;

        //pExtendedAck = new CExtendedAck (ackFlags != ICQ_TCPxACK_REFUSE, nPort, message);
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
         if (nInVersion <= 4) packet >> theSequence;
         packet >> licqChar >> licqVersion;

         // Some clients only send the first port (reversed)
         if (nPort == 0) nPort = (nPortReversed >> 8) | ((nPortReversed & 0xFF) << 8);
         // only if this is the first file ack packet do we do anything
         if (s_nFileSequence == theSequence && s_nFileUin == nUin)
         {
           gUserManager.DropUser(u);
           return true;
         }
         s_nFileSequence = theSequence;
         s_nFileUin = nUin;

         //pExtendedAck = new CExtendedAck(ackFlags != ICQ_TCPxACK_REFUSE, nPort, message);
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
					gLog.Info("%sUnknown direct ack ICBM plugin type: %s\n", L_TCPxSTR,
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

					pExtendedAck = new CExtendedAck(nPort != 0, nPort, szMessage);
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

					pExtendedAck = new CExtendedAck(nPort != 0, nPort, szMessage);
					break;
				}
				} // switch nICBMCommand

				break;
			}

#ifdef USE_OPENSSL
      case ICQ_CMDxSUB_SECURExOPEN:
      {
        if (nInVersion <= 4) packet >> theSequence;
        packet >> licqChar >> licqVersion;

        char l[32] = "";
        if (licqChar == 'L') sprintf(l, " [Licq %s]",
         CUserEvent::LicqVersionToString(licqVersion));
        gLog.Info("%sSecure channel response from %s (%ld)%s.\n", L_TCPxSTR,
         u->GetAlias(), nUin, l);

        ICQEvent *e = NULL;

        // Check if the response is ok
        if (message[0] == '\0')
        {
          gLog.Info("%s%s (%ld) does not support OpenSSL.\n", L_TCPxSTR,
             u->GetAlias(), nUin);
          u->SetSecure(false);
          PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_SECURITY, nUin, 0));
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
            gLog.Warn("%sSecure channel response from %s (%ld) when no request in progress.\n",
               L_WARNxSTR, u->GetAlias(), nUin);
            // Close the connection as we are in trouble
            u->SetSecure(false);
            gUserManager.DropUser(u);
            PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_SECURITY, nUin, 0));
            return false;
          }

          if (!pSock->SecureConnect())
          {
            errorOccured = true;
            e->m_eResult = EVENT_FAILED;
          }
          else
          {
            gLog.Info("%sSecure channel established with %s (%ld).\n", L_SSLxSTR,
               u->GetAlias(), nUin);
            u->SetSecure(true);
            PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_SECURITY, nUin, 1));
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
        if (nInVersion <= 4) packet >> theSequence;
        packet >> licqChar >> licqVersion;

        char l[32] = "";
        if (licqChar == 'L') sprintf(l, " [Licq %s]", CUserEvent::LicqVersionToString(licqVersion));
        gLog.Info("%sSecure channel with %s (%ld) closed %s.\n", L_TCPxSTR,
         u->GetAlias(), nUin, l);

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
        PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_SECURITY, nUin, 0));

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
      gLog.Info("%sRefusal from %s (#%ld)%s.\n", L_TCPxSTR, u->GetAlias(), -theSequence, l);
      nSubResult = (newCommand == ICQ_CMDxSUB_FILE || newCommand == ICQ_CMDxSUB_CHAT) ? ICQ_TCPxACK_ACCEPT : ICQ_TCPxACK_REFUSE;
    }
    else
    {
      // Update the away message if it's changed
      if (strcmp(u->AutoResponse(), message))
      {
        u->SetAutoResponse(message);
        u->SetShowAwayMsg(*message);
        gLog.Info("%sAuto response from %s (#%ld)%s.\n", L_TCPxSTR,
          u->GetAlias(), -theSequence, l);
      }

      switch(ackFlags)
      {
        case ICQ_TCPxACK_ONLINE:
        case ICQ_TCPxACK_AWAY:
        case ICQ_TCPxACK_NA:
          gLog.Info("%sAck from %s (#%ld)%s.\n", L_TCPxSTR, u->GetAlias(),
            -theSequence, l);
          nSubResult = ICQ_TCPxACK_ACCEPT;
          break;
        case ICQ_TCPxACK_OCCUPIED:
        case ICQ_TCPxACK_DND:
          gLog.Info("%sReturned from %s (#%ld)%s.\n", L_TCPxSTR, u->GetAlias(),
            -theSequence, l);
          nSubResult = ICQ_TCPxACK_RETURN;
          break;
        case ICQ_TCPxACK_OCCUPIEDxCAR:
        case ICQ_TCPxACK_DNDxCAR:
          gLog.Info("%sCustom %s response from %s (#%ld)%s.\n", L_TCPxSTR,
                    (ackFlags == ICQ_TCPxACK_DNDxCAR ? "DnD" : "Occupied"), u->GetAlias(),
                    -theSequence, l);
          nSubResult = ICQ_TCPxACK_ACCEPT; // FIXME: or should this be ACK_RETURN ?
          break;
        default:
          gLog.Unknown("%sUnknown ack flag from %s (#%ld): %04x %s.\n", L_UNKNOWNxSTR,
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
      gLog.Warn("%sAck for unknown event.\n", L_TCPxSTR);
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
        gLog.Info("%sChat request from %s (%ld) cancelled.\n", L_TCPxSTR,
                 u->GetAlias(), nUin);
        if (nInVersion <= 4)
          packet >> junkLong >> junkLong >> junkShort >> junkChar >> theSequence;
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
        gLog.Info("%sFile transfer request from %s (%ld) cancelled.\n",
                 L_TCPxSTR, u->GetAlias(), nUin);
        if (nInVersion <= 4)
          packet >> junkLong >> junkShort >> junkChar >> junkLong >> junkLong >> theSequence;
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

  if (bNewUser)
  {
    delete u;
    return false;
  }
  gUserManager.DropUser(u);
  return !errorOccured;
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
																bool bConfirm)
{
  char cHandshake;
  unsigned short nVersionMajor, nVersionMinor;
  CBuffer &b = s->RecvBuffer();
  b >> cHandshake >> nVersionMajor >> nVersionMinor;

  if ((unsigned char)cHandshake != ICQ_CMDxTCP_HANDSHAKE)
  {
    char *buf;
    gLog.Unknown("%sUnknown TCP handshake packet (command = 0x%02X):\n%s\n",
                 L_UNKNOWNxSTR, cHandshake, b.print(buf));
    delete [] buf;
    return false;
  }

  unsigned long nUin = 0;
  unsigned short nVersion = 0;

  switch (VersionToUse(nVersionMajor))
  {
    case 8:
    case 7:
    {
      b.Reset();
      CPacketTcp_Handshake_v7 p_in(&b);
      nUin = p_in.SourceUin();

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
      unsigned long nOk = s->RecvBuffer().UnpackUnsignedLong();
      if (nOk != 1)
      {
        gLog.Warn("%sBad handshake ack: %ld.\n", L_WARNxSTR, nOk);
        return false;
      }

			if (bConfirm)
			{
				// Get handshake confirmation
				CPacketTcp_Handshake_Confirm p_confirm;
				int nGot = s->RecvBuffer().getDataSize();
				s->ClearRecvBuffer();
      
				if (nGot > 4)
				{
					if (!s->SendPacket(p_confirm.getBuffer())) goto sock_error;
				}
				else
				{
					do
					{
						if (!s->RecvPacket()) goto sock_error;
					} while (!s->RecvBufferFull());

					if (!s->SendPacket(p_confirm.getBuffer())) goto sock_error;
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
        gLog.Warn("%sBad handshake ack: %ld.\n", L_WARNxSTR, nOk);
        return false;
      }
      nVersion = 6;
      break;
    }

    case 5:
    case 4:
    {
      b.UnpackUnsignedLong(); // port number
      nUin = b.UnpackUnsignedLong();
      nVersion = 4;
      break;
    }

    case 3:
    case 2:
    case 1:
    {
      b.UnpackUnsignedLong(); // port number
      nUin = b.UnpackUnsignedLong();
      nVersion = 2;
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

  s->SetOwner(nUin);
  s->SetVersion(nVersion);

  return true;

sock_error:
  char buf[128];
  if (s->Error() == 0)
    gLog.Warn("%sHandshake error, remote side closed connection.\n", L_WARNxSTR);
  else
    gLog.Warn("%sHandshake socket error:\n%s%s.\n", L_WARNxSTR, L_BLANKxSTR, s->ErrorStr(buf, 128));
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
  unsigned long nUin = s->Owner();
  if (nUin == 0) return false;

  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
  if (u != NULL)
  {
    gLog.Info("%sConnection from %s (%ld) [v%ld].\n", L_TCPxSTR,
       u->GetAlias(), nUin, s->Version());
    if (u->SocketDesc() != s->Descriptor())
    {
      if (u->SocketDesc() != -1)
      {
        gLog.Warn("%sUser %s (%ld) already has an associated socket.\n",
                  L_WARNxSTR, u->GetAlias(), nUin);
        gSocketManager.CloseSocket(u->SocketDesc(), false);
        u->ClearSocketDesc();
      }
      u->SetSocketDesc(s);
    }
    gUserManager.DropUser(u);
  }
  else
  {
    gLog.Info("%sConnection from new user (%ld) [v%ld].\n", L_TCPxSTR,
       nUin, s->Version());
  }

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


