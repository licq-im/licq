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

#ifndef LICQMSN_MSNPACKET_H
#define LICQMSN_MSNPACKET_H

#include <cstdlib>
#include <string>

#include <licq/packet.h>

#include "msnbuffer.h"

namespace LicqMsn
{

static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

class CMSNPacket : public Licq::Packet
{
public:
  CMSNPacket(bool = false);
  virtual ~CMSNPacket() { if (m_pBuffer) delete m_pBuffer; if (m_szCommand) free(m_szCommand); }
  Licq::Buffer* getBuffer() { return m_pBuffer; }

  unsigned short Command() { return 0; }
  unsigned short SubSequence() { return 0; }
  unsigned short SubCommand() { return 0; }
  unsigned short Sequence() { return m_nSequence; }
  const char* MSNCommand() { return m_szCommand; }
  
  virtual void InitBuffer();

  char *CreateGUID();

protected:
  
  CMSNBuffer *m_pBuffer;
  char *m_szCommand;
  unsigned short m_nSequence;
  unsigned long m_nSize;
  bool m_bPing;
  
  // Statics
  static unsigned short s_nSequence;
};

class CMSNPayloadPacket : public CMSNPacket
{
public:
  CMSNPayloadPacket(char msgType);
  virtual ~CMSNPayloadPacket() { }
  
  virtual void InitBuffer();
  
protected:
  unsigned long m_nPayloadSize;
  char myMsgType;
};

class CMSNP2PPacket : public CMSNPayloadPacket
{
public:
  CMSNP2PPacket(const std::string& toEmail,unsigned long = 0, unsigned long = 0, unsigned long = 0,
		unsigned long = 0, unsigned long = 0, unsigned long = 0,
		unsigned long = 0, unsigned long = 0, unsigned long = 0,
		unsigned long = 0, unsigned long = 0, unsigned long = 0);
  virtual ~CMSNP2PPacket();

  void InitBuffer();

  unsigned long SessionId() { return m_nSessionId; }
  unsigned long BaseId() { return m_nBaseId; }
  char *CallGUID() { return m_szCallGUID; }
  // unsigned long DataSize() {}
  //unsigned long Offset() {}

protected:
  std::string myToEmail;
  char *m_szCallGUID;
  unsigned long m_nSessionId,
    m_nBaseId,
    m_nDataSizeLO,
    m_nDataSizeHI,
    m_nDataOffsetLO,
    m_nDataOffsetHI,
    m_nLen,
    m_nFlag,
    m_nAckId,
    m_nAckUniqueId,
    m_nAckDataLO,
    m_nAckDataHI;
};

class CPS_MSNVersion : public CMSNPacket
{
public:
  CPS_MSNVersion();
};

class CPS_MSNClientVersion : public CMSNPacket
{
public:
  CPS_MSNClientVersion(const std::string& username);
};

class CPS_MSNUser : public CMSNPacket
{
public:
  CPS_MSNUser(const std::string& username);
};

class CPS_MSNSendTicket : public CMSNPacket
{
public:
  CPS_MSNSendTicket(const std::string& ticket);
};

class CPS_MSNChangeStatus : public CMSNPacket
{
public:
  CPS_MSNChangeStatus(std::string& status);
};

class CPS_MSNLogoff : public CMSNPacket
{
public:
  CPS_MSNLogoff();
};

class CPS_MSNSync : public CMSNPacket
{
public:
  CPS_MSNSync(unsigned long);
};

class CPS_MSNChallenge : public CMSNPacket
{
public:
  CPS_MSNChallenge(const std::string& hash);
};

class CPS_MSNSetPrivacy : public CMSNPacket
{
public:
  CPS_MSNSetPrivacy();
};

class CPS_MSNAddUser : public CMSNPacket
{
public:
  CPS_MSNAddUser(const std::string& username, const char* list);
};

class CPS_MSNRemoveUser : public CMSNPacket
{
public:
  CPS_MSNRemoveUser(const std::string& username, const char* list);
};

class CPS_MSNRenameUser : public CMSNPacket
{
public:
  CPS_MSNRenameUser(const std::string& username, const std::string& newNick);
};

class CPS_MSN_SBStart : public CMSNPacket
{
public:
  CPS_MSN_SBStart(const std::string& cookie, const std::string& username);
};

class CPS_MSN_SBAnswer : public CMSNPacket
{
public:
  CPS_MSN_SBAnswer(const std::string& session, const std::string& cookie, const std::string& username);
};

class CPS_MSNMessage : public CMSNPayloadPacket
{
public:
  CPS_MSNMessage(const char *);
  virtual ~CPS_MSNMessage() { if (m_szMsg) free(m_szMsg); }
  
protected:
  char *m_szMsg;
};

class CPS_MsnClientCaps : public CMSNPayloadPacket
{
public:
  CPS_MsnClientCaps();
};

class CPS_MSNPing : public CMSNPacket
{
public:
  CPS_MSNPing();
};

class CPS_MSNXfr : public CMSNPacket
{
public:
  CPS_MSNXfr();
};

class CPS_MSNCall : public CMSNPacket
{
public:
  CPS_MSNCall(const std::string& username);
};

class CPS_MSNTypingNotification : public CMSNPayloadPacket
{
public:
  CPS_MSNTypingNotification(const std::string& email);
};

class CPS_MSNCancelInvite : public CMSNPayloadPacket
{
public:
  CPS_MSNCancelInvite(const std::string& cookie, const std::string& code);
};

class CPS_MSNInvitation : public CMSNP2PPacket
{
public:
  CPS_MSNInvitation(const std::string& toEmail, const std::string& fromEmail,
      const std::string& msnObject);
};

class CPS_MSNP2PBye : public CMSNP2PPacket
{
public:
  CPS_MSNP2PBye(const std::string& toEmail, const std::string& fromEmail, const std::string& szCallId,
                unsigned long nBaseId, unsigned long nAckId,
		unsigned long nDataSizeHI, unsigned long nDataSizeLO);
};

class CPS_MSNP2PAck : public CMSNP2PPacket
{
public:
  CPS_MSNP2PAck(const std::string& toEmail, unsigned long nSessionId,
		unsigned long nBaseId, unsigned long nAckId,
		unsigned long nAckBaseId, unsigned long nDataSizeHI,
		unsigned long nDataSizeLO);
};

} // namespace LicqMsn

#endif
