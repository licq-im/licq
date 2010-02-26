// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998 - 2009 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cstdlib>
#include <cstdio>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <cstring>

// Localization
#include "gettext.h"

#include "licq_buffer.h"
#include "licq_byteorder.h"
#include "licq_log.h"
#include "support.h"

using namespace std;

// Endianness utility routines: Unlike Real Internet Protocols, this
// heap of dung uses little-endian byte sex.  With the new v7 and above
// this heap of SHIT uses little-endian and big-endian byte sex.


//=====Buffer================================================================

CBuffer::CBuffer()
{
  m_pDataStart = m_pDataPosRead = m_pDataPosWrite = NULL;
  m_nDataSize = 0;
}


CBuffer::CBuffer(unsigned long _nDataSize)
{
  m_nDataSize = _nDataSize;
  if (_nDataSize)
    m_pDataStart = new char[m_nDataSize];
  else
    m_pDataStart = NULL;
  m_pDataPosRead = m_pDataPosWrite = m_pDataStart;
}

CBuffer::CBuffer(const CBuffer &b)
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
CBuffer::CBuffer(CBuffer *b)
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

CBuffer& CBuffer::operator=(CBuffer &b)
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

CBuffer operator+(CBuffer &b0, CBuffer &b1)
{
   unsigned long nB0Size = b0.getDataPosWrite() - b0.getDataStart();
   unsigned long nB1Size = b1.getDataPosWrite() - b1.getDataStart();
   CBuffer bCat(b0.getDataSize() + b1.getDataSize());

   memcpy(bCat.getDataPosWrite(), b0.getDataStart(), nB0Size);
   bCat.incDataPosWrite(nB0Size);
   memcpy(bCat.getDataPosWrite(), b1.getDataStart(), nB1Size);
   bCat.incDataPosWrite(nB1Size);

   return bCat;
}

CBuffer& CBuffer::operator+=(CBuffer &b)
{
  CBuffer buf = *this + b;
  *this = buf;
  return *this;
}

//-----create-------------------------------------------------------------------
void CBuffer::Create(unsigned long _nDataSize)
{
   if (m_pDataStart != NULL) delete[] m_pDataStart;
   if (_nDataSize != 0) m_nDataSize = _nDataSize;
   m_pDataStart = new char[m_nDataSize];
   m_pDataPosRead = m_pDataPosWrite = m_pDataStart;
}

//----->>-----------------------------------------------------------------------
CBuffer& CBuffer::operator>>(char &in)
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

CBuffer& CBuffer::operator>>(unsigned char &in)
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

CBuffer& CBuffer::operator>>(unsigned short &in)
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

CBuffer& CBuffer::operator>>(unsigned long &in)
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

string CBuffer::unpackRawString(size_t size)
{
  char* c = new char[size];
  for (size_t i = 0; i < size; ++i)
    *this >> c[i];
  string ret(c);
  delete [] c;
  return ret;
}

char *CBuffer::UnpackRaw(char *sz, unsigned short _nSize)
{
  for (unsigned short i = 0; i < _nSize; i++) *this >> sz[i];
  sz[_nSize] = '\0';
  return sz;
}

char *CBuffer::UnpackBinBlock(char *sz, unsigned short _nSize)
{
  for (unsigned short i = 0; i < _nSize; i++) *this >> sz[i];
  return sz;
}

char *CBuffer::UnpackStringBE(char* sz, unsigned short _usiSize)
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
char *CBuffer::UnpackStringBE()
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

char *CBuffer::UnpackString(char *sz, unsigned short _usiSize)
{
  unsigned short nLen;
  sz[0] = '\0';
  *this >> nLen;
  nLen = nLen < _usiSize ? nLen : _usiSize - 1;
  for (unsigned short i = 0; i < nLen; i++) *this >> sz[i];
  sz[nLen] = '\0';
  return sz;
}

// Need to delete[] returned string
char *CBuffer::UnpackString()
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
char *CBuffer::UnpackUserString()
{
  unsigned char nLen;
  *this >> nLen;
  char *sz = new char[nLen+1];
  sz[0] = '\0';
  for (unsigned char i = 0; i < nLen; i++) *this >> sz[i];
  sz[nLen] = '\0';
  return sz;
}

unsigned long CBuffer::UnpackUnsignedLong()
{
  unsigned long n;
  *this >> n;
  return n;
}

unsigned long CBuffer::UnpackUinString()
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

unsigned long CBuffer::UnpackUnsignedLongBE()
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

unsigned short CBuffer::UnpackUnsignedShort()
{
  unsigned short n;
  *this >> n;
  return n;
}

unsigned short CBuffer::UnpackUnsignedShortBE()
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

char CBuffer::UnpackChar()
{
  char n;
  *this >> n;
  return n;
}



//-----clear--------------------------------------------------------------------
void CBuffer::Clear()
{
  if (m_pDataStart != NULL) delete[] m_pDataStart;
  myTLVs.clear();
  m_pDataStart = m_pDataPosRead = m_pDataPosWrite = NULL;
  m_nDataSize = 0;
}


//-----reset--------------------------------------------------------------------
void CBuffer::Reset()
{
  m_pDataPosRead = m_pDataStart;
}

//-----Empty--------------------------------------------------------------------
bool CBuffer::Empty()
{
  return (m_pDataStart == NULL);
}

//-----Full---------------------------------------------------------------------
bool CBuffer::Full()
{
  return (!Empty() && getDataPosWrite() >= (getDataStart() + getDataMaxSize()));
}


//-----Copy---------------------------------------------------------------------
void CBuffer::Copy(CBuffer *b)
{
  Create(b->getDataSize());
  Pack(b);
}


CBuffer::~CBuffer()
{
  if (m_pDataStart != NULL) delete[] m_pDataStart;
}

//-----add----------------------------------------------------------------------
char *CBuffer::PackUnsignedLong(unsigned long data)
{
  if ( getDataSize() + 4 > getDataMaxSize() )
  {
    gLog.Warn(tr("%sPackUnsignedLong(): Trying to pack more data than "
                 "CBuffer can hold!\n"), L_WARNxSTR);
    return getDataPosWrite();
  }
  *(uint32_t*)getDataPosWrite() = LE_32(data);
  incDataPosWrite(4);
  return getDataPosWrite() - 4;
}

char *CBuffer::PackUnsignedLongBE(unsigned long data)
{
  if (getDataSize() + 4 > getDataMaxSize() )
  {
    gLog.Warn(tr("%sPackUnsignedLongBE(): Trying to pack more data than "
                 "CBuffer can hold!\n"), L_WARNxSTR);
    return getDataPosWrite();
  }
  *(uint32_t*)getDataPosWrite() = BE_32(data);
  incDataPosWrite(4);
  return getDataPosWrite() - 4;
}

char *CBuffer::PackChar(char data)
{
  if (getDataSize() + 1 > getDataMaxSize())
  {
    gLog.Warn(tr("%sPackChar(): Trying to pack more data than "
                 "CBuffer can hold!\n"), L_WARNxSTR);
    return getDataPosWrite();
  }
  *getDataPosWrite() = data;
  incDataPosWrite(1);
  return getDataPosWrite() - 1;
}

char *CBuffer::Pack(const char *data, int size)
{
  if ( getDataSize() + size > getDataMaxSize() )
  {
    gLog.Warn(tr("%sPack(): Trying to pack more data than "
                 "CBuffer can hold!\n"), L_WARNxSTR);
    return getDataPosWrite();
  }
  if (!size) return getDataPosWrite();
  memcpy(getDataPosWrite(), data, size);
  incDataPosWrite(size);
  return getDataPosWrite() - size;
}

char *CBuffer::Pack(CBuffer *buf)
{
  if ( getDataSize() + buf->getDataSize() > getDataMaxSize() )
  {
    gLog.Warn(tr("%sPack(): Trying to pack more data than "
                 "CBuffer can hold!\n"), L_WARNxSTR);
    return getDataPosWrite();
  }
  memcpy(getDataPosWrite(), buf->getDataStart(), buf->getDataSize());
  incDataPosWrite(buf->getDataSize());
  return getDataPosWrite() - buf->getDataSize();
}

char *CBuffer::PackLNTS(const char *data)
{
  int size = (data == NULL ? 1 : strlen(data) + 1);
  PackUnsignedShort(size);
  if (data != NULL)
    Pack(data, size);
  return getDataPosWrite() - size;
}

char *CBuffer::PackString(const char *data, unsigned short max)
{
  unsigned short n = (data == NULL ? 0 : strlen(data));
  if (max > 0 && n > max) n = max;
  if ( getDataSize()  + n + 1 > getDataMaxSize() )
  {
    gLog.Warn(tr("%sPackString(): Trying to pack more data than "
                 "CBuffer can hold!\n"), L_WARNxSTR);
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

char *CBuffer::PackUnsignedShort(unsigned short data)
{
  if ( getDataSize() + 2 > getDataMaxSize() )
  {
    gLog.Warn(tr("%sPackUnsignedShort(): Trying to pack more data than "
                 "CBuffer can hold!\n"), L_WARNxSTR);
    return getDataPosWrite();
  }
  *(uint16_t*)getDataPosWrite() = LE_16(data);
  incDataPosWrite(2);
  return getDataPosWrite() - 2;
}

char *CBuffer::PackUnsignedShortBE(unsigned short data)
{
  if ( getDataSize() + 2 > getDataMaxSize() )
  {
    gLog.Warn(tr("%sPackUnsignedShortBE(): Trying to pack more data than "
                 "CBuffer can hold!\n"), L_WARNxSTR);
    return getDataPosWrite();
  }
  *(uint16_t*)getDataPosWrite() = BE_16(data);
  incDataPosWrite(2);
  return getDataPosWrite() - 2;
}

//-----TLV----------------------------------------------------------------------

bool CBuffer::readTLV(int nCount, int nBytes)
{
  if (!nCount) return false;

  // Clear the list if we already have some TLVs
  if (myTLVs.size() > 0)
    myTLVs.clear();

  int num = 0;
  int nCurBytes = 0;

  // Keep reading until it is impossible for any TLV headers to be found
  while(getDataPosRead() + 4 <= (getDataStart() + getDataSize())) {
    TLVPtr tlv(new COscarTLV);

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
      gLog.Warn(tr("%sRead too much TLV data!\n"), L_WARNxSTR);
      return true;
    }
  }

  // Finish off the number of bytes we wanted
  if (nCurBytes < nBytes)
  { 
    gLog.Warn(tr("%sUnable to read requested amount of TLV data!\n"), L_WARNxSTR);
    for (; nCurBytes < nBytes; nCurBytes++)
      UnpackChar();
  }

  return true;
}

void CBuffer::PackTLV(unsigned short nType, unsigned short nSize,
		       const char *data)
{
  PackUnsignedShortBE(nType);
  PackUnsignedShortBE(nSize);
  Pack(data, nSize);
}

void CBuffer::PackTLV(unsigned short nType, unsigned short nSize,
		      CBuffer *b)
{
  PackUnsignedShortBE(nType);
  PackUnsignedShortBE(nSize);
  Pack(b);
}

void CBuffer::PackTLV(const TLVPtr& tlv)
{
  PackUnsignedShortBE(tlv->myType);
  PackUnsignedShortBE(tlv->myLen);
  Pack(reinterpret_cast<const char *>(tlv->myData.get()), tlv->myLen);
}

#if 0
void CBuffer::PackFNACHeader(unsigned short nFamily, unsigned short nSubtype,
			     char nFlag1, char nFlag2, unsigned long nSeq)
{
  PackUnsignedShortBE(nFamily);
  PackUnsignedShortBE(nSubtype);
  PackChar(nFlag1);
  PackChar(nFlag2);
  PackUnsignedLongBE(nSeq);
}
#endif

unsigned short CBuffer::getTLVLen(unsigned short nType)
{
  unsigned short len = 0;
  TLVListIter iter = myTLVs.find(nType);
  if (iter != myTLVs.end())
    len = iter->second->myLen;
  return len;
}

bool CBuffer::hasTLV(unsigned short nType)
{
  TLVListIter iter = myTLVs.find(nType);
  bool found = (iter != myTLVs.end());
  return found;
}

unsigned long CBuffer::UnpackUnsignedLongTLV(unsigned short nType)
{
  unsigned long nRet = 0;

  try
  {
    TLVPtr tlv = getTLV(nType);
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

unsigned short CBuffer::UnpackUnsignedShortTLV(unsigned short nType)
{
  unsigned short nRet = 0;

  try
  {
    TLVPtr tlv = getTLV(nType);
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

unsigned char CBuffer::UnpackCharTLV(unsigned short nType)
{
  unsigned char nRet = 0;

  try
  {
    TLVPtr tlv = getTLV(nType);
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
char *CBuffer::UnpackStringTLV(unsigned short nType)
{
  char *str = 0;

  try
  {
    TLVPtr tlv = getTLV(nType);
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
std::string CBuffer::UnpackStringTLV(unsigned short nType)
{
  std::string str;

  try
  {
    TLVPtr tlv = getTLV(nType);
    str.assign(tlv->myData.get(), tlv->myLen);
  }
  catch (...)
  {
    // TODO Throw an exception
  }

  return str;
}
#endif

CBuffer CBuffer::UnpackTLV(unsigned short nType)
{
  try
  {
    TLVPtr tlv = getTLV(nType);
    CBuffer cbuf(tlv->myLen);
    cbuf.Pack(reinterpret_cast<const char *>(tlv->myData.get()), tlv->myLen);
    cbuf.Reset();

    return cbuf;
  }
  catch (...)
  {
    return CBuffer(0);
  }
}

TLVPtr CBuffer::getTLV(unsigned short nType)
{
  if (myTLVs.size() == 0)
    throw std::exception();

  TLVListIter iter = myTLVs.find(nType);
  if (iter == myTLVs.end())
    throw std::exception();

  return iter->second;
}

TLVList CBuffer::getTLVList()
{
  return myTLVs;
}

//-----print--------------------------------------------------------------------
char *CBuffer::print(char *&p)
{
   static const char BUFFER_BLANKS[] = "     ";
   static const unsigned long SPACE_PER_LINE =
    strlen(BUFFER_BLANKS) + strlen("0000: ") + 16 * 3 + 18 + 4;

   unsigned long nBytesToPrint = getDataSize();
   char szAscii[16 + 1];
   szAscii[16] = '\0';

   unsigned short nLenBuf = ((int)(nBytesToPrint / 16) + 1) *
    SPACE_PER_LINE;
   p = new char[nLenBuf + 1];
   char *pPos = p;
   pPos += sprintf(pPos, "%s0000: ", BUFFER_BLANKS);
   unsigned short i = 0;
   unsigned char c = 0;
   while(true)
   {
     c = (unsigned char)getDataStart()[i];
     szAscii[i % 16] = isprint(c) ? c : '.';
     pPos += sprintf(pPos, "%02X ", c);
     i++;

     if ((i % 16 == 0) && i >= nBytesToPrint)
     {
     	 pPos += sprintf(pPos, "  %s", szAscii);
     	 break;
     }
     else if (i >= nBytesToPrint)  break;

     if (i % 16 == 0)
       pPos += sprintf(pPos, "  %s\n%s%04X: ", szAscii, BUFFER_BLANKS, i);
     else if(i % 8 == 0)
       pPos += sprintf(pPos, " ");
   }

   if (nBytesToPrint != getDataSize())
     pPos += sprintf(pPos, "...");
   else
   {
     if ( (i % 16) && (i % 16) <= 8)
       pPos += sprintf(pPos, " ");

     szAscii[i % 16] = '\0';

     while (i++ % 16 != 0)
       pPos += sprintf(pPos, "   ");

     pPos += sprintf(pPos, "  %s", szAscii);

     *pPos = '\0';
   }

   return(p);
}












