/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1998-2012 Licq developers <licq-dev@googlegroups.com>
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

#include "buffer.h"

#include <cstring>

#include <licq/byteorder.h>
#include <licq/logging/log.h>

#include "../gettext.h"

using namespace LicqIcq;
using Licq::gLog;
using std::string;

OscarTlv::OscarTlv(unsigned short type, unsigned short length, const char* data)
  : myType(type), myLen(length)
{
  if (myLen > 0)
  {
    myData = boost::shared_array<unsigned char>(new  unsigned char[myLen]);
    memcpy(myData.get(), data, myLen);
  }
}

OscarTlv::OscarTlv(const OscarTlv& c)
{
  myType = c.myType;
  myLen = c.myLen;
  myData = boost::shared_array<unsigned char>(new unsigned char[c.myLen]);
  memcpy(myData.get(), c.myData.get(), c.myLen);
}

void OscarTlv::setData(unsigned char* data, unsigned short length)
{
  if (length > 0)
  {
    myLen = length;
    myData = boost::shared_array<unsigned char>(new unsigned char[length]);
    memcpy(myData.get(), data, length);
  }
}

Buffer::Buffer(const Licq::Buffer& b)
  : Licq::Buffer(b)
{
  const Buffer* buf = dynamic_cast<const Buffer*>(&b);
  if (buf != NULL)
    myTLVs = buf->myTLVs;
}

Licq::Buffer& Buffer::operator=(const Licq::Buffer& b)
{
  Licq::Buffer::operator=(b);
  const Buffer* buf = dynamic_cast<const Buffer*>(&b);
  if (buf != NULL)
    myTLVs = buf->myTLVs;
  else
    myTLVs.clear();
  return *this;
}

void Buffer::Clear()
{
  Licq::Buffer::Clear();
  myTLVs.clear();
}

//-----TLV----------------------------------------------------------------------

bool Buffer::readTLV(int nCount, int nBytes)
{
  if (!nCount) return false;

  // Clear the list if we already have some TLVs
  if (myTLVs.size() > 0)
    myTLVs.clear();

  int num = 0;
  int nCurBytes = 0;

  // Keep reading until it is impossible for any TLV headers to be found
  while(getDataPosRead() + 4 <= (getDataStart() + getDataSize())) {
    TlvPtr tlv(new OscarTlv);

    *this >> tlv->myType;
    *this >> tlv->myLen;

    tlv->myType = BSWAP_16(tlv->myType);
    tlv->myLen = BSWAP_16(tlv->myLen);

    nCurBytes += 4 + tlv->myLen;

    if(getDataPosRead() + tlv->myLen > (getDataStart() + getDataSize()) ||
       tlv->myLen < 1) {
      tlv->myLen = 0;
    }
    else {
      tlv->myData.reset(new unsigned char[tlv->myLen]);
      memcpy(tlv->myData.get(), m_pDataPosRead, tlv->myLen);
      m_pDataPosRead += tlv->myLen;
    }

    // Save it in the map
    myTLVs[tlv->myType] = tlv;

    ++num;
    if ((nCount > 0 && num == nCount) ||
        (nBytes > 0 && nCurBytes == nBytes) )
      return true;

    if (nBytes > 0 && nCurBytes > nBytes)
    {
      gLog.warning(tr("Read too much TLV data!"));
      return true;
    }
  }

  // Finish off the number of bytes we wanted
  if (nCurBytes < nBytes)
  {
    gLog.warning(tr("Unable to read requested amount of TLV data!"));
    for (; nCurBytes < nBytes; nCurBytes++)
      unpackInt8();
  }

  return true;
}

void Buffer::PackTLV(unsigned short nType, unsigned short nSize,
		       const char *data)
{
  packUInt16BE(nType);
  packUInt16BE(nSize);
  packRaw(data, nSize);
}

void Buffer::PackTLV(unsigned short nType, unsigned short nSize, Buffer* b)
{
  packUInt16BE(nType);
  packUInt16BE(nSize);
  Pack(b);
}

void Buffer::PackTLV(const TlvPtr& tlv)
{
  packUInt16BE(tlv->myType);
  packUInt16BE(tlv->myLen);
  packRaw(tlv->myData.get(), tlv->myLen);
}

#if 0
void Buffer::PackFNACHeader(unsigned short nFamily, unsigned short nSubtype,
			     char nFlag1, char nFlag2, unsigned long nSeq)
{
  packUInt16BE(nFamily);
  packUInt16BE(nSubtype);
  packInt8(nFlag1);
  packInt8(nFlag2);
  packUInt32BE(nSeq);
}
#endif

unsigned short Buffer::getTLVLen(unsigned short nType)
{
  unsigned short len = 0;
  TlvList::iterator iter = myTLVs.find(nType);
  if (iter != myTLVs.end())
    len = iter->second->myLen;
  return len;
}

bool Buffer::hasTLV(unsigned short nType)
{
  TlvList::iterator iter = myTLVs.find(nType);
  bool found = (iter != myTLVs.end());
  return found;
}

uint32_t Buffer::unpackTlvUInt32(int type)
{
  try
  {
    TlvPtr tlv = getTLV(type);
    if (tlv->myLen >= 4)
      return BE_32(*(uint32_t*)(tlv->myData.get()));
  }
  catch (...)
  {
    // TODO Throw an exception
  }
  return 0;
}

uint16_t Buffer::unpackTlvUInt16(int type)
{
  try
  {
    TlvPtr tlv = getTLV(type);
    if (tlv->myLen >= 2)
      return BE_16(*(uint16_t*)(tlv->myData.get()));
  }
  catch (...)
  {
    // TODO Throw an exception
  }
  return 0;
}

uint8_t Buffer::unpackTlvUInt8(int type)
{
  try
  {
    TlvPtr tlv = getTLV(type);
    if (tlv->myLen >= 1)
      return *(uint8_t*)(tlv->myData.get());
  }
  catch (...)
  {
    // TODO Throw an exception
  }
  return 0;
}

string Buffer::unpackTlvString(int type)
{
  try
  {
    TlvPtr tlv = getTLV(type);
    return string((const char*)(tlv->myData.get()), tlv->myLen);
  }
  catch (...)
  {
    // TODO Throw an exception
  }
  return "";
}

Buffer Buffer::UnpackTLV(unsigned short nType)
{
  try
  {
    TlvPtr tlv = getTLV(nType);
    Buffer cbuf(tlv->myLen);
    cbuf.packRaw(tlv->myData.get(), tlv->myLen);
    cbuf.Reset();

    return cbuf;
  }
  catch (...)
  {
    return Buffer(0);
  }
}

TlvPtr Buffer::getTLV(unsigned short nType)
{
  if (myTLVs.empty())
    throw std::exception();

  TlvList::iterator iter = myTLVs.find(nType);
  if (iter == myTLVs.end())
    throw std::exception();

  return iter->second;
}

TlvList Buffer::getTlvList()
{
  return myTLVs;
}
