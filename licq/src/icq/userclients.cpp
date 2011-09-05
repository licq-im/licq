/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2011 Licq developers <licq-dev@googlegroups.com>
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

/*
 * These functions attempt to identify the user client based on published
 * capabilities and signatures
 *
 * Most of this is taken from Miranda IcqOscar protocol 0.9.25
 */

#include "icq.h"

#include <cstring>
#include <sstream>

#include "defines.h"

using std::string;
using std::stringstream;

const int CAP_LENGTH = 16;

const char* findCapability(const char* caps, int capSize, const char* needle, int needleSize = CAP_LENGTH)
{
  while (capSize >= CAP_LENGTH)
  {
    if (memcmp(caps, needle, needleSize) == 0)
      return caps;
    caps += CAP_LENGTH;
    capSize -= CAP_LENGTH;
  }
  return NULL;
}

static void appendVersion(stringstream& buf, int min, const char* ver)
{
  buf << (int)ver[0];
  if (ver[1] != 0 || min >= 2)
    buf << '.' << (int)ver[1];
  if (ver[2] != 0 || min >= 3)
    buf << '.' << (int)ver[2];
  if (ver[3] != 0 || min >= 4)
    buf << '.' << (int)ver[3];
}

static string mirandaMod(stringstream& buf, const char* cap, bool unicode, const char* name)
{
  buf << "Miranda IM ";
  if (cap[4] != 0 || cap[5] != 0 || cap[6] != 0 || cap[7] != 0)
    appendVersion(buf, 3, cap+4);
  if (unicode)
    buf << " Unicode";
  buf << " (" << name << " v";
  appendVersion(buf, 3, cap+8);
  buf << ')';

  if (memcmp(cap+12, "\x5A\xFE\xC0\xDE", 4) == 0)
    buf << " + SecureIM";

  return buf.str();
}

static string detectFromCapSign(const char* caps, int capSize, unsigned ts1, unsigned ts2, unsigned ts3)
{
  const char* cap = caps;
  int capsLeft = capSize / CAP_LENGTH;

  stringstream buf;

  while (capsLeft > 0)
  {
    if (memcmp(cap, ICQ_CAPABILITY_LICQxVER, 12) == 0)
    {
      buf << "Licq " << (int)cap[12] << '.' << (int)cap[13] << '.' << (int)cap[14];
      if (cap[15] == 1)
        buf << "/SSL";
      return buf.str();
    }
    else if (memcmp(cap, "MirandaM", 8) == 0)
    {
      buf << "Miranda IM ";
      if (cap[8] != 0 || cap[9] != 0 || cap[10] != 0 || cap[11] != 0)
        appendVersion(buf, 3, cap+8);
      if (ts1 == 0x7FFFFFFF)
        buf << " Unicode";
      buf << " (ICQ v";
      appendVersion(buf, 3, cap+12);
      buf << ')';

      if (findCapability(caps, capSize, "icqj", 4))
      {
        buf << " (s7 & sss)";
        if (findCapability(caps, capSize, "icqj Secure IM", 14))
          buf << " + SecureIM";
      }
      else if ((ts1 & 0x7FFFFFFF) == 0x7FFFFFFF)
      {
        if (findCapability(caps, capSize, "MirandaMobile\0\0\0"))
          buf << " (Mobile)";
        if (ts3 == 0x5AFEC0DE)
          buf << " + SecureIM";
      }
      return buf.str();
    }
    else if (memcmp(cap, "MirandaA", 8) == 0)
    {
      buf << "Miranda IM ";
      if (cap[8] != 0 || cap[9] != 0 || cap[10] != 0 || cap[11] != 0)
        appendVersion(buf, 3, cap+8);
      buf << " (AimOscar v";
      appendVersion(buf, 3, cap+12);
      buf << ')';
      return buf.str();
    }
    else if (memcmp(cap, "icqj", 4) == 0)
      return mirandaMod(buf, cap, ts3==0x80000000, "ICQ S7 & SSS");

    else if (memcmp(cap, "sinj", 4) == 0)
      return mirandaMod(buf, cap, ts3==0x80000000, "ICQ S!N");

    else if (memcmp(cap, "icqp", 4) == 0)
      return mirandaMod(buf, cap, ts3==0x80000000, "ICQ Plus");

    else if (memcmp(cap, "J2ME m@agent", 12) == 0)
    {
      buf << "Mail.ru Agent (Java) v";
      appendVersion(buf, 2, cap+13);
      return buf.str();
    }
    else if (memcmp(cap, "\x97\xB1\x27\x51\x24\x3C\x43\x34\xAD\x22\xD6\xAB\xF7\x3F\x14\x09", 16) == 0 ||
        memcmp(cap, "\xF2\xE7\xC7\xF4\xFE\xAD\x4D\xFB\xB2\x35\x36\x79\x8B\xDF\0\0", 16) == 0)
    {
      if (findCapability(caps, capSize, ICQ_CAPABILITY_FILE))
        return "Trillian Astra";
      else if (findCapability(caps, capSize, ICQ_CAPABILITY_RTFxMSGS))
        return "Trillian v3";
      else if (findCapability(caps, capSize, "\x01\x38\xCA\x7B\x76\x9A\x49\x15\x88\xF2\x13\xFC\x00\x97\x9E\xA8"))
        return "Trillian Astra";
      else
        return "Trillian";
    }
    else if (memcmp(cap, "\x97\xB1\x27\x51\x24\x3C\x43\x34\xAD\x22\xD6\xAB\xF7\x3F\x14", 15) == 0 &&
        (unsigned char)cap[15] != 0x92 && ((unsigned char)cap[15] >= 0x20 || cap[15] == 0))
    {
      if ((cap[15] & 0xC0) == 0 || cap[15] == 0x40)
        return "Kopete";

      buf << "SIM " << ((((unsigned)cap[15])>>6)-1) << '.' << (cap[15]&0x1F);
      return buf.str();
    }
    else if (memcmp(cap, "SIM client  ", 12) == 0)
    {
      buf << "SIM " << (int)cap[12] << '.' << (int)cap[13] << '.' << (int)cap[14] << '.' << (cap[15]&0x0F);
      if (cap[15] & 0x80)
        buf << " (Win32)";
      else if (cap[15] & 0x40)
        buf << " (MacOS X)";
      return buf.str();
    }
    else if (memcmp(cap, "Kopete ICQ  ", 12) == 0)
    {
      buf << "Kopete " << (int)cap[12] << '.' << (int)cap[13] << '.' << (cap[14]*100 + cap[15]);
      return buf.str();
    }
    else if (memcmp(cap, "climm\xA9 R.K. ", 12) == 0)
    {
      buf << "climm " << (cap[12]&0x7F) << '.' << (int)cap[13] << '.' << (int)cap[14] << '.' << (int)cap[15];
      if (cap[12] & 0x80)
        buf << " alpha";
      if (ts3 == 0x02000020)
        buf << " (Win32)";
      else if (ts3 == 0x03000800)
        buf << " (MacOS X)";
      return buf.str();
    }
    else if (memcmp(cap, "mICQ \xA9 R.K. ", 12) == 0)
    {
      buf << "mICQ " << (cap[12]&0x7F) << '.' << (int)cap[13] << '.' << (int)cap[14] << '.' << (int)cap[15];
      if (cap[12] & 0x80)
        buf << " alpha";
      return buf.str();
    }
    else if (memcmp(cap, "\x74\xED\xC3\x36\x44\xDF\x48\x5B\x8B\x1C\x67\x1A\x1F\x86\x09\x9F", 16) == 0)
      return "IM2";

    else if (memcmp(cap, "&RQinside", 9) == 0 ||
        memcmp(cap, "R&Qinside", 9) == 0)
    {
      buf << cap[0] << cap[1] << cap[2] << ' ' << (int)cap[12] << '.' << (int)cap[11] << '.' << (int)cap[10] << '.' << (int)cap[9];
      return buf.str();
    }
    else if (memcmp(cap, "IMadering Client", 16) == 0)
      return "IMadering";

    else if (memcmp(cap, "V?\xC8\x09\x0BoAQIP     !", 16) == 0)
      return "QIP PDA (Windows)";

    else if (memcmp(cap, "\x51\xAD\xD1\x90\x72\x04\x47\x3D\xA1\xA1\x49\xF4\xA3\x97\xA4\x1F", 16) == 0)
      return "QIP PDA (Symbian)";

    else if (memcmp(cap, "\x60\xDE\x5C\x8A\xDF\x8C\x4E\x1D\xA4\xC8\xBC\x3B\xD9\x79\x4D\xD8", 16) == 0)
      return "QIP Mobile (IPhone)";

    else if (memcmp(cap, "\xB0\x82\x62\xF6\x7F\x7C\x45\x61\xAD\xC1\x1C\x6D\x75\x70\x5E\xC5", 16) == 0)
      return "QIP Mobile (Java)";

    else if (memcmp(cap, "\x7C\x73\x75\x02\xC3\xBE\x4F\x3E\xA6\x9F\x01\x53\x13\x43\x1E\x1A", 16) == 0)
    {
      buf << "QIP Infium";
      if (ts1 != 0)
        buf << " (" << ts1 << ')';
      if (ts2 == 11)
        buf << " Beta";
      return buf.str();
    }
    else if (memcmp(cap, "\x7A\x7B\x7C\x7D\x7E\x7F\x0A\x03\x0B\x04\x01\x53", 12) == 0)
    {
      buf << "QIP 2010";
      if (ts1 != 0)
        buf << " (" << ts1 << ')';
      return buf.str();
    }
    else if (memcmp(cap, "V?\xC8\x09\x0BoAQIP 2005a", 14) == 0)
    {
      buf << "QIP ";
      if (ts3 == 15)
        buf << "2005";
      else
        buf << cap[11] << cap[12] << cap[13] << cap[14] << cap[15];
      if (ts1 != 0 && ts2 == 14)
        buf << ' ' << (int)(ts1>>24) << (int)((ts1>>16)&0xFF) << (int)((ts1>>8)&0xFF) << (int)(ts1&0xFF);
      return buf.str();
    }
    else if (memcmp(cap, "mChat icq ", 10) == 0)
      return "mChat " + string(cap+10, 6);

    else if (memcmp(cap, "Jimm ", 5) == 0)
      return "Jimm " + string(cap+5, 11);

    else if (memcmp(cap, "CORE Pager", 10) == 0)
    {
      buf << "CORE Pager";
      if (ts2 == 0x0FFFF0011 && ts3 == 0x1100FFFF && (ts1 & 0xFF000000) != 0)
      {
        buf << ' ' << (ts1>>24) << '.' << ((ts1>>16)&0xFF);
        if ((ts1 & 0xFF) == 0x0B)
          buf << " Beta";
      }
      return buf.str();
    }
    else if (memcmp(cap, "D[i]Chat ", 9) == 0)
      return "D[i]Chat " + string(cap+9, 7);

    else if (memcmp(cap, "\xDD\x16\xF2\x02\x84\xE6\x11\xD4\x90\xDB\x00\x10\x4B\x9B\x4B\x7D", 16) == 0)
      return "ICQ for Mac";

    else if (memcmp(cap, "\xA7\xE4\x0A\x96\xB3\xA0\x47\x9A\xB8\x45\xC9\xE4\x67\xC5\x6B\x1F", 16) == 0)
      return "uIM";

    else if (memcmp(cap, "\x44\xE5\xBF\xCE\xB0\x96\xE5\x47\xBD\x65\xEF\xD6\xA3\x7E\x36\x02", 16) == 0)
      return "Anastasia";

    else if (memcmp(cap, "JICQ\0\0\0\0\0\0\0\0", 12) == 0)
    {
      buf << "JICQ ";
      appendVersion(buf, 2, cap+12);
      return buf.str();
    }
    else if (memcmp(cap, "MIP ", 4) == 0)
    {
      if (memcmp(cap+4, "Client v", 8) == 0)
      {
        if ((unsigned char)cap[12] < 0x30)
        {
          buf << "MIP ";
          appendVersion(buf, 2, cap+12);
          return buf.str();
        }
        else
          return "MIP " + string(cap+11, 5);
      }
      else
      {
        return "MIP " + string(cap+4, 12);
      }
    }
    else if (memcmp(cap, "VmICQ ", 6) == 0)
      return "VmICQ " + string(cap+6, 10);

    else if (memcmp(cap, "Smaper ", 7) == 0)
      return "SmapeR " + string(cap+7, 9);

    else if (memcmp(cap, "Yapp", 4) == 0)
      return "Yapp! v" + string(cap+8, 5);

    else if (memcmp(cap, "digsby", 6) == 0 ||
        memcmp(cap, "\x09\x46\x01\x05\x4c\x7f\x11\xd1\x82\x22\x44\x45\x45\x53\x54\x00", 16) == 0)
      return "Digsby";

    else if (memcmp(cap, "japp\xA9 by Sergo\0\0", 16) == 0)
      return "japp";

    else if (memcmp(cap, "PIGEON!", 7) == 0)
      return "PIGEON";

    else if (memcmp(cap, "qutim", 5) == 0)
    {
      buf << "qutIM ";
      if (cap[6] == '.')
        buf << cap[5] << '.' << cap[7];
      else
      {
        buf << (int)cap[6] << '.' << (int)cap[7] << '.' << (int)cap[8] << '.' << ((cap[9]<<8) | cap[10]);
        switch (cap[5])
        {
          case 'l': buf << " (Linux)"; break;
          case 'w': buf << " (Win32)"; break;
          case 'm': buf << " (MacOS X)"; break;
        }
      }
      return buf.str();
    }
    else if (memcmp(cap, "bayanICQ", 8) == 0)
      return "bayanICQ " + string(cap+8, 5);

    else if (memcmp(cap, "JIT ", 4) == 0)
      return "Jabber ICQ Transport";

    else if (memcmp(cap, "IcqKid2", 7) == 0)
    {
      buf << "IcqKid2 v";
      appendVersion(buf, 2, cap+7);
      return buf.str();
    }
    else if (memcmp(cap, "WebIcqPro ", 10) == 0)
      return "WebIcqPro";

    else if (memcmp(cap, "\x09\x19\x19\x82\xDE\xAD\xBE\xEF\xCA\xFE\x44\x45\x53\x54\x00\x00", 16) == 0)
      return "Citron IM";

    else if (memcmp(cap, "\xFF\xFF\xFF\xFFnaim", 8) == 0)
      return "naim";

    else if (memcmp(cap, "NCICQ", 5) == 0)
      return "NCICQ " + string(cap+6, 10);

    else if (memcmp(cap, "LocID ", 6) == 0)
      return "LocID";

    --capsLeft;
    cap += CAP_LENGTH;
  }

  return string();
}

static string detectFromTimestamps(int tcpVersion, unsigned ts1, unsigned ts2,
    unsigned ts3, time_t onlineSince, int webPort, int capSize)
{
  stringstream buf;

  if ((ts1 & 0xFF7F0000) == 0x7D000000)
  {
    int v = ts1 & 0xFFFF;
    buf << "Licq v" << (v/1000) << '.' << ((v/10)%100) << '.' << (v%10);
    if (ts1 & 0x00800000)
      buf << "/SSL";
    return buf.str();
  }
  else if (ts1 == 0xFFFFFFFF && ts2 == 0xFFFFFFFF)
    return "Gaim";

  else if (ts1 == 0xFFFFFFFF && ts2 == 0 && tcpVersion == 7)
    return "WebICQ";

  else if (ts1 == 0xFFFFFFFF && ts2 == 0 && ts3 == 0x3B7248ED)
    return "Spam Bot";

  else if ((ts1 & 0x7FFFFFFF) == 0x7FFFFFFF && ts2 != 0)
  {
    buf << "Miranda IM ";
    if (ts1 & 0x80000000)
      buf << "Unicode ";
    if (ts2 == 1)
      buf << "0.1.2 alpha";
    else
      buf << (ts2>>24) << '.' << ((ts2>>16)&0xFF) << '.' << ((ts2>>8)&0xFF) << '.' << (ts2&0xFF);
    return buf.str();
  }
  else if (ts1 == 0xFFFFFF8F)
    return"StrICQ";

  else if (ts1 == 0xFFFFFF42)
    return "mICQ";

  else if (ts1 == 0xFFFFFFBE)
  {
    buf << "Alicq " << (ts2>>24) << '.' << ((ts2>>16)&0xFF) << '.' << ((ts2>>8)&0xFF);
    return buf.str();
  }
  else if (ts1 == 0xFFFFFF7F)
    return "&RQ";

  else if (ts1 == 0xFFFFFFAB)
    return "YSM";

  else if (ts1 == 0x04031980)
    return "vICQ";

  else if (ts1 == 0x3B75AC09)
    return "Trillian";

  else if (ts1 == 0x3BA8DBAF && tcpVersion == 2)
    return "stICQ";

  else if (ts1 == 0xFFFFFFFE && ts3 == 0xFFFFFFFE)
    return "Jimm";

  else if (ts1 == 0x3FF19BEB && ts3 == 0x3FF19BEB)
    return "IM2";

  else if (ts1 == 0xDDDDEEFF && ts2 == 0 && ts3 == 0)
    return "SmartICQ";

  else if ((ts1 & 0xFFFFFFF0) == 0x494D2B00 && ts2 == 0 && ts3 == 0)
  {
    switch (ts1 & 0xFF)
    {
      case 3: return "IM+ (SmartPhone, Pocket PC)";
      case 5: return "IM+ (Win32)";
      default: return "IM+";
    }
  }
  else if (ts1 == 0x3B4C4C0C && ts2 == 0 && ts3 == 0x3B7248ed)
    return "KXicq2";

  else if (ts1 == 0xFFFFF666 && ts3 == 0)
  {
    buf << "R&Q " << ts2;
    return buf.str();
  }
  else if (ts1 == 0x66666666 && ts3 == 0x66666666)
  {
    buf << "D[i]Chat v.";
    if (ts2 == 0x10000)
      buf << "0.1a";
    else
    {
      buf << ((ts2>>8)&0xF) << '.' << ((ts2>>4)&0xF);
      switch (ts2 & 0x0F)
      {
        case 1: buf << " alpha"; break;
        case 2: buf << " beta"; break;
        case 3: buf << " final"; break;
      }
    }
    return buf.str();
  }
  else if (ts1 == 0xFFFF8615 && ts3 == 0xFFFF8615)
    return "NCICQ";

  else if (ts1 == 0x48151623 && ts3 == 0x48151623)
  {
    buf << "LocID v" << ((ts2>>8)&0xF) << '.' << ((ts2>>4)&0xF) << (ts2&0xF) << " p" << ((ts2>>16)&0xFF);
    return buf.str();
  }
  else if (ts1 == ts2 && ts2 == ts3 && tcpVersion == 8
           && static_cast<time_t>(ts1) < onlineSince + 3600
           && static_cast<time_t>(ts1) > onlineSince - 3600)
    return "Spam Bot";

  else if (ts1 == 0 && ts2 == 0 && ts3 == 0 && tcpVersion == 0 && capSize == 0 && webPort == 0x75BB)
    return "Spam Bot";

  else if (ts1 == 0x44F523B0 && ts2 == 0x44F523A6 && ts3 == 0x44F523A6 && tcpVersion == 8)
    return "Virus";

  return string();
}

static string detectFromCaps(const char* caps, int capSize, int userClass,
    int tcpVersion, unsigned ts1, unsigned ts2, unsigned ts3, int webPort)
{
  // Check which capablities we've got
  bool hasRtf = false;
  bool hasUtf8 = false;
  bool hasRelay = false;
  bool hasXtraz = false;
  bool hasTyping = false;
  bool hasFile = false;
  bool hasChat = false;
  bool hasContacts = false;
  bool hasTzers = false;
  bool hasIcqDirect = false;
  bool hasAimIcon = false;
  bool hasAimDirect = false;
  bool hasAimFileShare = false;
  bool hasIcqDevils = false;
  bool hasAimSmartCaps = false;
  bool hasAimLiveVideo = false;
  bool hasAimLiveAudio = false;
  bool hasStatusTextAware = false;
  bool hasComm20012 = false;
  bool hasRambler = false;
  bool hasAbv = false;
  bool hasNetvigator = false;
  bool hasIMSecKey = false;
  bool hasFakeHtml = false;
  bool hasXtrazVideo = false;
  bool is2001 = false;
  bool is2002 = false;
  bool isIcqLite = false;

  const char* cap = caps;
  int capsLeft = capSize / CAP_LENGTH;
  while (capsLeft > 0)
  {
    if (memcmp(cap+4, "\x4C\x7F\x11\xD1\x82\x22\x44\x45\x53\x54\x00\x00", 12) == 0)
    {
      if (cap[0] == 0x09 && cap[1] == 0x46)
      {
        if (cap[2] == 0x13)
        {
          switch (cap[3])
          {
            case 0x43: hasFile = true; break;
            case 0x44: hasIcqDirect = true; break;
            case 0x45: hasAimDirect = true; break;
            case 0x46: hasAimIcon = true; break;
            case 0x48: hasAimFileShare = true; break;
            case 0x49: hasRelay = true; break;
            case 0x4B: hasContacts = true; break;
            case 0x4C: hasIcqDevils = true; break;
            case 0x4E: hasUtf8 = true; break;
          }
        }
        else if (cap[2] == 0x01)
        {
          switch ((unsigned)cap[3])
          {
            case 0xFF: hasAimSmartCaps = true; break;
            case 0x01: hasAimLiveVideo = true; break;
            case 0x04: hasAimLiveAudio = true; break;
            case 0x0A: hasStatusTextAware = true; break;
          }
        }
      }
      else if (memcmp(cap, "\xA0\xE9\x3F\x37", 4) == 0)
        hasComm20012 = true;
      else if (memcmp(cap, "\x10\xCF\x40\xD1", 4) == 0)
        is2002 = true;
    }
    else if (memcmp(cap, ICQ_CAPABILITY_RTFxMSGS, CAP_LENGTH) == 0)
      hasRtf = true;
    else if (memcmp(cap, ICQ_CAPABILITY_XTRAZ, CAP_LENGTH) == 0)
      hasXtraz = true;
    else if (memcmp(cap, ICQ_CAPABILITY_TYPING, CAP_LENGTH) == 0)
      hasTyping = true;
    else if (memcmp(cap, "\xB2\xEC\x8F\x16\x7C\x6F\x45\x1B\xBD\x79\xDC\x58\x49\x78\x88\xB9", CAP_LENGTH) == 0)
      hasTzers = true;
    else if (memcmp(cap, "\x2E\x7A\x64\x75\xFA\xDF\x4D\xC8\x88\x6F\xEA\x35\x95\xFD\xB6\xDF", CAP_LENGTH) == 0)
      is2001 = true;
    else if (memcmp(cap, "\xC8\x95\x3A\x9F\x21\xF1\x4F\xAA\xB0\xB2\x6D\xE6\x63\xAB\xF5\xB7", CAP_LENGTH) == 0)
      isIcqLite = true;
    else if (memcmp(cap, "\x7E\x11\xB7\x78\xA3\x53\x49\x26\xA8\x02\x44\x73\x52\x08\xC4\x2A", CAP_LENGTH) == 0)
      hasRambler = true;
    else if (memcmp(cap, "\x00\xE7\xE0\xDF\xA9\xD0\x4F\xE1\x91\x62\xC8\x90\x9A\x13\x2A\x1B", CAP_LENGTH) == 0)
      hasAbv = true;
    else if (memcmp(cap, "\x4C\x6B\x90\xA3\x3D\x2D\x48\x0E\x89\xD6\x2E\x4B\x2C\x10\xD9\x9F", CAP_LENGTH) == 0)
      hasNetvigator = true;
    else if (memcmp(cap, "\x01\x01\x01\x01\x01\x01", 6) == 0 || memcmp(cap, "\x02\x02\x02\x02\x02\x02", 6) == 0)
      hasIMSecKey = true;
    else if (memcmp(cap, "\x01\x38\xCA\x7B\x76\x9A\x49\x15\x88\xF2\x13\xFC\x00\x97\x9E\xA8", CAP_LENGTH) == 0)
      hasFakeHtml = true;
    else if (memcmp(cap, "\x17\x8C\x2D\x9B\xDA\xA5\x45\xBB\x8D\xDB\xF3\xBD\xBD\x53\xA1\x0A", CAP_LENGTH) == 0)
      hasXtrazVideo = true;
    else if (memcmp(cap, "\x74\x8F\x24\x20\x62\x87\x11\xD1\x82\x22\x44\x45\x53\x54\x00\x00", CAP_LENGTH) == 0)
      hasChat = true;

    cap += CAP_LENGTH;
    --capsLeft;
  }

  if (ts1 == 0x3AA773EE && ts2 == 0x3AA66380)
  {
    // libicq2000, try to determine which client is behind it
    if (hasRtf)
      // centericq added rtf capability to libicq2000
      return "Centericq";
    else if (hasUtf8)
      // IcyJuice added unicode capability to libicq2000
      return "libicq2000 (Unicode)";
    else
      // others - like jabber transport uses unmodified library, thus cannot be detected
      return "libicq2000";
  }

  // AIM clients
  if ((userClass & 0x0040) == 0)
  {
    if (hasAimIcon && hasChat && hasUtf8 && hasTyping && capSize == 0x40)
      return "Meebo";

    else if (hasRelay && hasUtf8 && hasTyping && hasXtraz && hasChat && hasAimIcon && hasAimDirect && hasFakeHtml && capSize == 0x90)
      return "libpurple";

    else if (hasRelay && hasUtf8 && hasTyping && hasXtraz && hasChat & hasAimIcon && hasFakeHtml && capSize == 0x70)
      return "Meebo";

    else
      return "AIM";
  }

  if (tcpVersion == 8 && hasXtraz && hasIMSecKey)
    tcpVersion = 9;

  switch (tcpVersion)
  {
    case 8:
      if (hasComm20012 || hasRelay)
      {
        if (is2001)
        {
          if (ts1 == 0 && ts2 == 0 && ts3 == 0)
          {
            if (hasRtf)
              return "TICQClient"; // possibly also older GnomeICU
            else
              return "ICQ for Pocket PC";
          }
          else
            return "ICQ 2001";
        }
        else if (is2002)
          return "ICQ 2002";

        else if (hasRelay && hasUtf8 && hasRtf)
        {
          if (ts1 == 0 && ts2 == 0 && ts3 == 0)
          {
            if (webPort == 0)
              return "GnomeICU 0.99.5+";
            else
              return "IC@";
          }
          else
            return "ICQ 2002/2003a";
        }
        else if (hasRelay && hasUtf8 && hasTyping && hasXtraz && hasChat && hasAimIcon && hasFakeHtml)
        {
          if (hasAimDirect)
            return "libpurple";
          else
            return "Meebo";
        }
        else if (hasRelay && hasUtf8 && hasTyping && ts1 == 0 && ts2 == 0 && ts3 == 0)
          return "PreludeICQ";
      }
      else if (hasUtf8 && hasTyping && hasAimIcon && hasAimDirect)
        return "imo.im";

      break;

    case 9:
      if (hasXtraz)
      {
        if (hasFile)
        {
          string base;
          if (hasTzers)
          {
            if (isIcqLite && hasStatusTextAware && hasAimLiveVideo && hasAimLiveAudio)
              base = "ICQ 7";

            else if (hasFakeHtml)
            {
              if (hasAimLiveVideo && hasAimLiveAudio)
                base = "ICQ 6";

              else if (hasRtf && hasContacts && hasIcqDevils)
                base = "Qnext v4";
            }
            else
              base = "icq5.1";
          }
          else
          {
            base = "icq5";
          }

          if (hasRambler)
            return base + " (Rambler)";
          else if (hasAbv)
            return base + " (Abv)";
          else if (hasNetvigator)
            return base + " (Netvigator)";
          else
            return base;
        }
        else if (hasIcqDirect)
        {
          if (hasRtf)
            return "Qnext";
          else if (hasTyping && hasTzers && hasFakeHtml)
          {
            if (hasRelay && hasUtf8 && hasAimLiveAudio)
              return "Mail.ru Agent (PC)";
            else
              return "Fring";
          }
          else
            return "pyICQ";
        }
        else
          return "ICQ Lite v4";
      }
      else if (isIcqLite)
        return "ICQ Lite";

      else if (!hasIcqDirect)
      {
        if (hasFakeHtml && hasChat && hasAimSmartCaps)
          return "Trillian Astra";
        else
          return "pyICQ";
      }
      break;

    case 7:
      if (hasRtf)
        return "GnomeICU";
      else if (hasRelay)
      {
        if (ts1 == 0 && ts2 == 0 && ts3 == 0)
          return "&RQ";
        else
          return "ICQ 2000";
      }
      else if (hasUtf8)
      {
        if (hasTyping)
          return "Icq2Go! (Java)";
        else if (userClass & 0x0080)
          return "Pocket Web 1&1";
        else
          return "Icq2Go!";
      }
      break;

    case 10:
      if (!hasRtf && !hasUtf8)
        return "Qnext";
      else if (hasRtf && hasUtf8 && ts1 == 0 && ts2 == 0 && ts3 == 0)
        return "NanoICQ";

      break;

    case 11:
      if (hasXtraz && hasRelay && hasTyping && hasUtf8 && hasIcqDevils)
        return "Mail.ru Agent (Symbian)";

      break;

    case 0:
      if (ts1 == 0 && ts2 == 0 && ts3 == 0 && webPort == 0)
      {
        if (hasTyping && is2001 &&is2002 && hasComm20012)
          return "Spam Bot";

        else if (hasAimIcon && hasAimDirect && hasFile && hasUtf8)
        {
          if (hasRelay)
            return "Adium X";
          else if (hasTyping)
            return "libpurple";
          else
            return "libgaim";
        }
        else if (hasAimIcon && hasAimDirect && hasChat && hasFile && capSize == 0x40)
          return "libgaim";

        else if (hasFile && hasChat && capSize == 0x20)
          return "Easy Message";

        else if (hasUtf8 && hasTyping && hasAimIcon && hasChat && capSize == 0x40)
          return "Meebo";

        else if (hasUtf8 && hasAimIcon && capSize == 0x20)
          return "PyICQ-t Jabber Transport";

        else if (hasUtf8 && hasXtraz && hasAimIcon && hasXtrazVideo)
          return "PyICQ-t Jabber Transport";

        else if (hasUtf8 && hasRelay && hasIcqDirect && hasTyping && capSize == 0x40)
          return "Agile Messenger"; // Smartphone 2002

        else if (hasUtf8 && hasRelay && hasIcqDirect && hasFile && hasAimFileShare)
          return "Slick";

        else if (hasUtf8 && hasRelay && hasFile && hasContacts && hasAimFileShare && hasAimIcon)
          return "Digsby";

        else if (hasUtf8 && hasRelay && hasContacts && hasAimIcon && hasFakeHtml)
          return "mundu IM";

        else if (hasUtf8 && hasFile && hasChat)
          return "eBuddy";

        else if (hasContacts && hasFile && hasAimIcon && hasAimDirect && hasChat)
          return "IloveIM";
      }
      break;
  }

  return string();
}

static string detectGeneric(int tcpVersion)
{
  switch (tcpVersion)
  {
    case 6: return "ICQ99";
    case 7: return "ICQ 2000/ICQ2Go";
    case 8: return "ICQ 2001-2003a";
    case 9: return "ICQ Lite";
    case 10: return "ICQ 2003b";
  }
  return string();
}

static void detectExtras(string& client, const char* caps, int capSize)
{
  if (client.substr(0, 10) == "Miranda IM")
  {
    const char* m = findCapability(caps, capSize, "MIM/", 4);
    if (m != NULL)
      client += " [" + string(m+4, 12) + "]";
  }

  if (findCapability(caps, capSize, "SIMPSIMPSIMPSIMP"))
    client += " + SimpLite";
  else if (findCapability(caps, capSize, "SIMP_PROSIMP_PRO"))
    client += " + SimpPro";
  else if (findCapability(caps, capSize, "IMsecureCphr\0\0\x06\x01") ||
      findCapability(caps, capSize, "\x01\x01\x01\x01\x01\x01", 6) || findCapability(caps, capSize, "\x02\x02\x02\x02\x02\x02", 6))
    client += " + IMsecure";
}

string IcqProtocol::detectUserClient(const char* caps, int capSize, int userClass,
    int tcpVersion, unsigned ts1, unsigned ts2, unsigned ts3, time_t onlineSince, int webPort)
{
  string client;

  if (capSize > 0)
  {
    client = detectFromCapSign(caps, capSize, ts1, ts2, ts3);

    // Make sure we didin't get any null characters by mistake
    string::size_type pos = client.find('\x0');
    if (pos != string::npos)
      client.erase(pos);
  }

  if (client.empty())
    client = detectFromTimestamps(tcpVersion, ts1, ts2, ts3, onlineSince, webPort, capSize);

  if (client.empty())
    client = detectFromCaps(caps, capSize, userClass, tcpVersion, ts1, ts2, ts3, webPort);

  if (client.empty())
    client = detectGeneric(tcpVersion);

  if (!client.empty())
    detectExtras(client, caps, capSize);

  return client;
}
