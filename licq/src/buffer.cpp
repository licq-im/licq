/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998-2011 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
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
   if(getDataPosRead() + sizeof(char) > (getDataStart() + getDataSize()))
      in = 0;
   else
   {
      in = *((char *)getDataPosRead());
      incDataPosRead(sizeof(char));
   }
   return(*this);
}

Buffer& Buffer::operator>>(unsigned char& in)
{
   if(getDataPosRead() + sizeof(unsigned char) > (getDataStart() + getDataSize()))
      in = 0;
   else
   {
      in = *((unsigned char *)getDataPosRead());
      incDataPosRead(sizeof(unsigned char));
   }
   return(*this);
}

Buffer& Buffer::operator>>(unsigned short& in)
{
   if(getDataPosRead() + 2 > (getDataStart() + getDataSize()))
      in = 0;
   else
   {
      in = LE_16(*(uint16_t*)getDataPosRead());
      incDataPosRead(2);
   }
   return(*this);
}

Buffer& Buffer::operator>>(unsigned long& in)
{
  if(getDataPosRead() + 4 > (getDataStart() + getDataSize()))
    in = 0;
  else
  {
    in = LE_32(*(uint32_t*)getDataPosRead());
    incDataPosRead(4);
  }
  return(*this);
}

string Buffer::unpackRawString(size_t size)
{
  char* c = new char[size];
  for (size_t i = 0; i < size; ++i)
    *this >> c[i];
  string ret(c, size);
  delete [] c;
  return ret;
}

char* Buffer::UnpackRaw(char* sz, unsigned short _nSize)
{
  for (unsigned short i = 0; i < _nSize; i++) *this >> sz[i];
  sz[_nSize] = '\0';
  return sz;
}

char* Buffer::UnpackBinBlock(char* sz, unsigned short _nSize)
{
  for (unsigned short i = 0; i < _nSize; i++) *this >> sz[i];
  return sz;
}

char* Buffer::UnpackStringBE(char* sz, unsigned short _usiSize)
{
  unsigned short nLen;
  sz[0] = '\0';
  *this >> nLen;
  nLen = BSWAP_16(nLen);
  nLen = nLen < _usiSize ? nLen : _usiSize - 1;
  for (unsigned short i = 0; i < nLen; i++) *this >> sz[i];
  sz[nLen] = '\0';
  return sz;
}

// Need to delete[] returned string
char* Buffer::UnpackStringBE()
{
  unsigned short nLen;
  *this >> nLen;
  nLen = BSWAP_16(nLen);
  char *sz = new char[nLen+1];
  sz[0] = '\0';
  for (unsigned short i = 0; i < nLen; i++) *this >> sz[i];
  sz[nLen] = '\0';
  return sz;
}

char* Buffer::UnpackString(char* sz, unsigned short _usiSize)
{
  unsigned short nLen;
  sz[0] = '\0';
  *this >> nLen;
  nLen = nLen < _usiSize ? nLen : _usiSize - 1;
  for (unsigned short i = 0; i < nLen; i++) *this >> sz[i];
  sz[nLen] = '\0';
  return sz;
}

string Buffer::unpackString()
{
  unsigned short nLen;
  *this >> nLen;
  char* sz = new char[nLen+1];
  for (unsigned short i = 0; i < nLen; i++) *this >> sz[i];
  string ret(sz, nLen);
  delete[] sz;
  return ret;
}

// Need to delete[] returned string
char* Buffer::UnpackString()
{
  unsigned short nLen;
  *this >> nLen;
  char* sz = new char[nLen+1];
  sz[0] = '\0';
  for (unsigned short i = 0; i < nLen; i++) *this >> sz[i];
  sz[nLen] = '\0';
  return sz;
}

// Need to dlete[] returned string
char* Buffer::UnpackUserString()
{
  unsigned char nLen;
  *this >> nLen;
  char *sz = new char[nLen+1];
  sz[0] = '\0';
  for (unsigned char i = 0; i < nLen; i++) *this >> sz[i];
  sz[nLen] = '\0';
  return sz;
}

unsigned long Buffer::UnpackUnsignedLong()
{
  unsigned long n;
  *this >> n;
  return n;
}

unsigned long Buffer::UnpackUinString()
{
  unsigned char nUinLen;
  char uin[20];
  *this >> nUinLen;
  if (nUinLen > 15) return 0;

  char* ptr = uin;
  while (nUinLen--)
    *ptr++ = UnpackChar();
  *ptr = '\0';

  return atoi(uin);
}

unsigned long Buffer::UnpackUnsignedLongBE()
{
  unsigned long n;
  if(getDataPosRead() + 4 > (getDataStart() + getDataSize()))
    n = 0;
  else
  {
    n = BE_32(*(uint32_t*)getDataPosRead());
    incDataPosRead(4);
  }
  return n;
}

unsigned short Buffer::UnpackUnsignedShort()
{
  unsigned short n;
  *this >> n;
  return n;
}

unsigned short Buffer::UnpackUnsignedShortBE()
{
  unsigned short n;
  if (getDataPosRead() + 2 > (getDataStart() + getDataSize()))
    n = 0;
  else
  {
    n = BE_16(*(uint16_t*)getDataPosRead());
    incDataPosRead(2);
  }
  return n;
}

char Buffer::UnpackChar()
{
  char n;
  *this >> n;
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
char* Buffer::PackUnsignedLong(unsigned long data)
{
  if ( getDataSize() + 4 > getDataMaxSize() )
  {
    gLog.warning(tr("PackUnsignedLong(): Trying to pack more data than "
        "Licq::Buffer can hold!"));
    return getDataPosWrite();
  }
  *(uint32_t*)getDataPosWrite() = LE_32(data);
  incDataPosWrite(4);
  return getDataPosWrite() - 4;
}

char* Buffer::PackUnsignedLongBE(unsigned long data)
{
  if (getDataSize() + 4 > getDataMaxSize() )
  {
    gLog.warning(tr("PackUnsignedLongBE(): Trying to pack more data than "
        "Licq::Buffer can hold!"));
    return getDataPosWrite();
  }
  *(uint32_t*)getDataPosWrite() = BE_32(data);
  incDataPosWrite(4);
  return getDataPosWrite() - 4;
}

char* Buffer::PackChar(char data)
{
  if (getDataSize() + 1 > getDataMaxSize())
  {
    gLog.warning(tr("PackChar(): Trying to pack more data than "
        "Licq::Buffer can hold!"));
    return getDataPosWrite();
  }
  *getDataPosWrite() = data;
  incDataPosWrite(1);
  return getDataPosWrite() - 1;
}

char* Buffer::Pack(const char* data, int size)
{
  if ( getDataSize() + size > getDataMaxSize() )
  {
    gLog.warning(tr("Pack(): Trying to pack more data than "
        "Licq::Buffer can hold!"));
    return getDataPosWrite();
  }
  if (!size) return getDataPosWrite();
  memcpy(getDataPosWrite(), data, size);
  incDataPosWrite(size);
  return getDataPosWrite() - size;
}

char* Buffer::Pack(Buffer* buf)
{
  if ( getDataSize() + buf->getDataSize() > getDataMaxSize() )
  {
    gLog.warning(tr("Pack(): Trying to pack more data than "
        "Licq::Buffer can hold!"));
    return getDataPosWrite();
  }
  memcpy(getDataPosWrite(), buf->getDataStart(), buf->getDataSize());
  incDataPosWrite(buf->getDataSize());
  return getDataPosWrite() - buf->getDataSize();
}

char* Buffer::PackLNTS(const char* data)
{
  int size = (data == NULL ? 1 : strlen(data) + 1);
  PackUnsignedShort(size);
  if (data != NULL)
    Pack(data, size);
  else
    PackChar('\0');
  return getDataPosWrite() - size;
}

char* Buffer::PackString(const char* data, unsigned short max)
{
  unsigned short n = (data == NULL ? 0 : strlen(data));
  if (max > 0 && n > max) n = max;
  if ( getDataSize()  + n + 1 > getDataMaxSize() )
  {
    gLog.warning(tr("PackString(): Trying to pack more data than "
        "Licq::Buffer can hold!"));
    return getDataPosWrite();
  }
  *(uint16_t*)getDataPosWrite() = LE_16(n + 1);
  incDataPosWrite(2);
  if (n)
  {
    memcpy(getDataPosWrite(), data, n);
    incDataPosWrite(n);
  }
  *getDataPosWrite() = '\0';
  incDataPosWrite(1);
  return getDataPosWrite() - 2 - n - 1;
}

char* Buffer::PackUnsignedShort(unsigned short data)
{
  if ( getDataSize() + 2 > getDataMaxSize() )
  {
    gLog.warning(tr("PackUnsignedShort(): Trying to pack more data than "
        "Licq::Buffer can hold!"));
    return getDataPosWrite();
  }
  *(uint16_t*)getDataPosWrite() = LE_16(data);
  incDataPosWrite(2);
  return getDataPosWrite() - 2;
}

char* Buffer::PackUnsignedShortBE(unsigned short data)
{
  if ( getDataSize() + 2 > getDataMaxSize() )
  {
    gLog.warning(tr("PackUnsignedShortBE(): Trying to pack more data than "
        "Licq::Buffer can hold!"));
    return getDataPosWrite();
  }
  *(uint16_t*)getDataPosWrite() = BE_16(data);
  incDataPosWrite(2);
  return getDataPosWrite() - 2;
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
      UnpackChar();
  }

  return true;
}

void Buffer::PackTLV(unsigned short nType, unsigned short nSize,
		       const char *data)
{
  PackUnsignedShortBE(nType);
  PackUnsignedShortBE(nSize);
  Pack(data, nSize);
}

void Buffer::PackTLV(unsigned short nType, unsigned short nSize, Buffer* b)
{
  PackUnsignedShortBE(nType);
  PackUnsignedShortBE(nSize);
  Pack(b);
}

void Buffer::PackTLV(const TlvPtr& tlv)
{
  PackUnsignedShortBE(tlv->myType);
  PackUnsignedShortBE(tlv->myLen);
  Pack(reinterpret_cast<const char *>(tlv->myData.get()), tlv->myLen);
}

#if 0
void Buffer::PackFNACHeader(unsigned short nFamily, unsigned short nSubtype,
			     char nFlag1, char nFlag2, unsigned long nSeq)
{
  PackUnsignedShortBE(nFamily);
  PackUnsignedShortBE(nSubtype);
  PackChar(nFlag1);
  PackChar(nFlag2);
  PackUnsignedLongBE(nSeq);
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

unsigned long Buffer::UnpackUnsignedLongTLV(unsigned short nType)
{
  unsigned long nRet = 0;

  try
  {
    TlvPtr tlv = getTLV(nType);
    if (tlv->myLen > 3)
    {
      nRet |= (*((tlv->myData.get())+0) << 24);
      nRet |= (*((tlv->myData.get())+1) << 16);
      nRet |= (*((tlv->myData.get())+2) << 8);
      nRet |= (*((tlv->myData.get())+3));
    }
  }
  catch (...)
  {
    // TODO Throw an exception
  }

  return nRet;
}

unsigned short Buffer::UnpackUnsignedShortTLV(unsigned short nType)
{
  unsigned short nRet = 0;

  try
  {
    TlvPtr tlv = getTLV(nType);
    if (tlv->myLen > 1)
    {
      nRet |= (*((tlv->myData.get())+0) << 8);
      nRet |= (*((tlv->myData.get())+1));
    }
  }
  catch (...)
  {
    // TODO Throw an exception
  }

  return nRet;
}

unsigned char Buffer::UnpackCharTLV(unsigned short nType)
{
  unsigned char nRet = 0;

  try
  {
    TlvPtr tlv = getTLV(nType);
    if (tlv->myLen > 0)
      nRet = *(tlv->myData.get());
  }
  catch (...)
  {
    // TODO Throw an exception
  }

  return nRet;
}

// Need to delete[] returned string
char* Buffer::UnpackStringTLV(unsigned short nType)
{
  char *str = 0;

  try
  {
    TlvPtr tlv = getTLV(nType);
    str = new char[tlv->myLen+1];
    memcpy(str, tlv->myData.get(), tlv->myLen);
    *(str+tlv->myLen) = '\0';
  }
  catch (...)
  {
    if (str)
    {
      delete [] str;
      str = 0;
    }
  }

  return str;
}

#if 0
//TODO Add this function and use it everywhere so we don't have to
// constanly remember to call delete[]
std::string Buffer::UnpackStringTLV(unsigned short nType)
{
  std::string str;

  try
  {
    TlvPtr tlv = getTLV(nType);
    str.assign(tlv->myData.get(), tlv->myLen);
  }
  catch (...)
  {
    // TODO Throw an exception
  }

  return str;
}
#endif

Buffer Buffer::UnpackTLV(unsigned short nType)
{
  try
  {
    TlvPtr tlv = getTLV(nType);
    Buffer cbuf(tlv->myLen);
    cbuf.Pack(reinterpret_cast<const char *>(tlv->myData.get()), tlv->myLen);
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
