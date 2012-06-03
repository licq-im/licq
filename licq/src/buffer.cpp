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

#include <licq/buffer.h>

#include "config.h"

#include <cstdlib>
#include <cstdio>
#include <ctype.h>
#include <cstring>

#include <licq/byteorder.h>
#include <licq/logging/log.h>

#include "gettext.h"

using namespace std;
using Licq::Buffer;


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

Buffer& Buffer::operator=(const Buffer& b)
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

   return (*this);
}

Buffer Licq::operator+(const Buffer& b0, const Buffer& b1)
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

Buffer& Buffer::operator+=(const Buffer& b)
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
  incDataPosWrite(data.size()+1);
}

void Buffer::packString32LE(const char* data, size_t length)
{
  assert(remainingDataToWrite() >= length + 4);

  *(uint32_t*)getDataPosWrite() = LE_32(length);
  incDataPosWrite(4);
  if (length > 0)
  {
    memcpy(getDataPosWrite(), data, length);
    incDataPosWrite(length);
  }
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
