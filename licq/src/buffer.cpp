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

#include "config.h"

#include <cstdlib>
#include <cstdio>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <cstring>

#include <licq/buffer.h>
#include <licq/byteorder.h>
#include <licq/logging/log.h>

#include "gettext.h"

using namespace std;
using Licq::Buffer;
using Licq::OscarTlv;
using Licq::TlvList;
using Licq::TlvPtr;

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


Buffer::Buffer()
{
  m_pDataStart = m_pDataPosRead = m_pDataPosWrite = NULL;
  m_nDataSize = 0;
}


Buffer::Buffer(unsigned long _nDataSize)
{
  m_nDataSize = _nDataSize;
  if (_nDataSize)
    m_pDataStart = new char[m_nDataSize];
  else
    m_pDataStart = NULL;
  m_pDataPosRead = m_pDataPosWrite = m_pDataStart;
}

Buffer::Buffer(const Buffer& b)
{
  m_nDataSize = b.getDataMaxSize();
  if (m_nDataSize)
  {
    m_pDataStart = new char[m_nDataSize];
    memcpy(m_pDataStart, b.getDataStart(), m_nDataSize);
  }
  else
  {
    m_pDataStart = NULL;
  }
  m_pDataPosRead = m_pDataStart + (b.getDataPosRead() - b.getDataStart());
  m_pDataPosWrite = m_pDataStart + (b.getDataPosWrite() - b.getDataStart());
}

#if 0
Buffer::Buffer(Buffer* b)
{
  if (b == NULL)
  {
     m_pDataStart = m_pDataPosRead = m_pDataPosWrite = NULL;
     m_nDataSize = 0;
  }
  else
  {
    m_nDataSize = b->getDataMaxSize();
    m_pDataStart = new char[m_nDataSize];
    memcpy(m_pDataStart, b->getDataStart(), m_nDataSize);
    m_pDataPosRead = m_pDataStart + (b->getDataPosRead() - b->getDataStart());
    m_pDataPosWrite = m_pDataStart + (b->getDataPosWrite() - b->getDataStart());
  }
}
#endif

Buffer& Buffer::operator=(Buffer& b)
{
   if (m_pDataStart != NULL) delete [] m_pDataStart;
   m_nDataSize = b.getDataSize();
   if (m_nDataSize)
   {
     m_pDataStart = new char[m_nDataSize];
     memcpy(m_pDataStart, b.getDataStart(), m_nDataSize);
   }
   else
     m_pDataStart = NULL;
   m_pDataPosRead = m_pDataStart + (b.getDataPosRead() - b.getDataStart());
   m_pDataPosWrite = m_pDataStart + (b.getDataPosWrite() - b.getDataStart());
   myTLVs = b.myTLVs;

   return (*this);
}

Buffer Licq::operator+(Buffer& b0, Buffer& b1)
{
   unsigned long nB0Size = b0.getDataPosWrite() - b0.getDataStart();
   unsigned long nB1Size = b1.getDataPosWrite() - b1.getDataStart();
  Buffer bCat(b0.getDataSize() + b1.getDataSize());

   memcpy(bCat.getDataPosWrite(), b0.getDataStart(), nB0Size);
   bCat.incDataPosWrite(nB0Size);
   memcpy(bCat.getDataPosWrite(), b1.getDataStart(), nB1Size);
   bCat.incDataPosWrite(nB1Size);

   return bCat;
}

Buffer& Buffer::operator+=(Buffer& b)
{
  Buffer buf = *this + b;
  *this = buf;
  return *this;
}

//-----create-------------------------------------------------------------------
void Buffer::Create(unsigned long _nDataSize)
{
   if (m_pDataStart != NULL) delete[] m_pDataStart;
   if (_nDataSize != 0) m_nDataSize = _nDataSize;
   m_pDataStart = new char[m_nDataSize];
   m_pDataPosRead = m_pDataPosWrite = m_pDataStart;
}

//----->>-----------------------------------------------------------------------
Buffer& Buffer::operator>>(char& in)
{
  in = unpackInt8();
  return *this;
}

Buffer& Buffer::operator>>(unsigned char& in)
{
  in = unpackUInt8();
  return *this;
}

Buffer& Buffer::operator>>(unsigned short& in)
{
  in = unpackUInt16LE();
  return *this;
}

Buffer& Buffer::operator>>(unsigned long& in)
{
  in = unpackUInt32LE();
  return *this;
}

string Buffer::unpackRawString(size_t size)
{
  if (remainingDataToRead() < size)
    size = remainingDataToRead();
  string ret(getDataPosRead(), size);
  incDataPosRead(size);
  return ret;
}

string Buffer::unpackByteString()
{
  uint8_t len = unpackUInt8();
  return unpackRawString(len);
}

string Buffer::unpackShortStringLE()
{
  uint16_t len = unpackUInt16LE();
  return unpackRawString(len);
}

string Buffer::unpackShortStringBE()
{
  uint16_t len = unpackUInt16BE();
  return unpackRawString(len);
}

string Buffer::unpackLongStringLE()
{
  uint32_t len = unpackUInt32LE();
  return unpackRawString(len);
}

string Buffer::unpackLongStringBE()
{
  uint32_t len = unpackUInt32BE();
  return unpackRawString(len);
}

uint32_t Buffer::unpackUInt32LE()
{
  if (remainingDataToRead() < 4)
    return 0;

  uint32_t n = LE_32(*(uint32_t*)getDataPosRead());
  incDataPosRead(4);
  return n;
}

uint32_t Buffer::unpackUInt32BE()
{
  if (remainingDataToRead() < 4)
    return 0;

  uint32_t n = BE_32(*(uint32_t*)getDataPosRead());
  incDataPosRead(4);
  return n;
}

uint16_t Buffer::unpackUInt16LE()
{
  if (remainingDataToRead() < 2)
    return 0;

  uint16_t n = LE_16(*(uint16_t*)getDataPosRead());
  incDataPosRead(2);
  return n;
}

uint16_t Buffer::unpackUInt16BE()
{
  if (remainingDataToRead() < 2)
    return 0;

  uint16_t n = BE_16(*(uint16_t*)getDataPosRead());
  incDataPosRead(2);
  return n;
}

uint8_t Buffer::unpackUInt8()
{
  if (remainingDataToRead() < 1)
    return 0;

  uint8_t n = *((uint8_t*)getDataPosRead());
  incDataPosRead(1);
  return n;
}

int8_t Buffer::unpackInt8()
{
  if (remainingDataToRead() < 1)
    return 0;

  int8_t n = *((int8_t*)getDataPosRead());
  incDataPosRead(1);
  return n;
}


//-----clear--------------------------------------------------------------------
void Buffer::Clear()
{
  if (m_pDataStart != NULL) delete[] m_pDataStart;
  myTLVs.clear();
  m_pDataStart = m_pDataPosRead = m_pDataPosWrite = NULL;
  m_nDataSize = 0;
}


//-----reset--------------------------------------------------------------------
void Buffer::Reset()
{
  m_pDataPosRead = m_pDataStart;
}

//-----Empty--------------------------------------------------------------------
bool Buffer::Empty() const
{
  return (m_pDataStart == NULL);
}

//-----Full---------------------------------------------------------------------
bool Buffer::Full() const
{
  return (!Empty() && getDataPosWrite() >= (getDataStart() + getDataMaxSize()));
}


//-----Copy---------------------------------------------------------------------
void Buffer::Copy(Buffer* b)
{
  Create(b->getDataSize());
  Pack(b);
}


Buffer::~Buffer()
{
  if (m_pDataStart != NULL) delete[] m_pDataStart;
}

//-----add----------------------------------------------------------------------
void Buffer::packUInt32LE(uint32_t data)
{
  if (remainingDataToWrite() < 4)
  {
    gLog.warning(tr("Trying to pack more data than Licq::Buffer can hold!"));
    return;
  }
  *(uint32_t*)getDataPosWrite() = LE_32(data);
  incDataPosWrite(4);
}

void Buffer::packUInt32BE(uint32_t data)
{
  if (remainingDataToWrite() < 4)
  {
    gLog.warning(tr("Trying to pack more data than Licq::Buffer can hold!"));
    return;
  }
  *(uint32_t*)getDataPosWrite() = BE_32(data);
  incDataPosWrite(4);
}

void Buffer::packUInt16LE(uint16_t data)
{
  if (remainingDataToWrite() < 2)
  {
    gLog.warning(tr("Trying to pack more data than Licq::Buffer can hold!"));
    return;
  }
  *(uint16_t*)getDataPosWrite() = LE_16(data);
  incDataPosWrite(2);
}

void Buffer::packUInt16BE(uint16_t data)
{
  if (remainingDataToWrite() < 2)
  {
    gLog.warning(tr("Trying to pack more data than Licq::Buffer can hold!"));
    return;
  }
  *(uint16_t*)getDataPosWrite() = BE_16(data);
  incDataPosWrite(2);
}

void Buffer::packUInt8(uint8_t data)
{
  if (remainingDataToWrite() < 1)
  {
    gLog.warning(tr("Trying to pack more data than Licq::Buffer can hold!"));
    return;
  }
  *(uint8_t*)getDataPosWrite() = data;
  incDataPosWrite(1);
}

void Buffer::packInt8(int8_t data)
{
  if (remainingDataToWrite() < 1)
  {
    gLog.warning(tr("Trying to pack more data than Licq::Buffer can hold!"));
    return;
  }
  *(int8_t*)getDataPosWrite() = data;
  incDataPosWrite(1);
}

void Buffer::packRaw(const void* data, size_t length)
{
  if (data == NULL)
    return;
  if (remainingDataToWrite() < length)
  {
    gLog.warning(tr("Trying to pack more data than Licq::Buffer can hold!"));
    return;
  }
  memcpy(getDataPosWrite(), data, length);
  incDataPosWrite(length);
}

void Buffer::packShortNullStringLE(const string& data)
{
  if (remainingDataToWrite() < data.size() + 3)
  {
    gLog.warning(tr("Trying to pack more data than Licq::Buffer can hold!"));
    return;
  }

  *(uint16_t*)getDataPosWrite() = LE_16(data.size()+1);
  incDataPosWrite(2);
  memcpy(getDataPosWrite(), data.c_str(), data.size()+1);
  incDataPosWrite(data.size());
}

void Buffer::Pack(Buffer* buf)
{
  if ( getDataSize() + buf->getDataSize() > getDataMaxSize() )
  {
    gLog.warning(tr("Pack(): Trying to pack more data than "
        "Licq::Buffer can hold!"));
    return;
  }
  memcpy(getDataPosWrite(), buf->getDataStart(), buf->getDataSize());
  incDataPosWrite(buf->getDataSize());
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

void Buffer::log(Log::Level level, const char* format, ...)
{
  va_list args;
  va_start(args, format);

  gLog.packet(level,
              reinterpret_cast<const uint8_t*>(getDataStart()),
              getDataSize(),
              format, args);

  va_end(args);
}
