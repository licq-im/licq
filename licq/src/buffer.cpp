// -*- c-basic-offset: 2 -*-
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <ctype.h>

#include "licq_buffer.h"
#include "licq_log.h"

//=====Utilities=================================================================

// Takes an ip from the buffer class and converts it to network byte order:
unsigned long PacketIpToNetworkIp(unsigned long l)
{
  return htonl((l << 24) | ((l & 0xff00) << 8) | ((l & 0xff0000) >> 8) | (l >> 24));
}

//  Takes an ip in network order and converts it to the packet class format
unsigned long NetworkIpToPacketIp(unsigned long l)
{
	l = ntohl(l);
	return (l << 24) | ((l & 0xff00) << 8) | ((l & 0xff0000) >> 8) | (l >> 24);
}

// Endianness utility routines: Unlike Real Internet Protocols, this
// heap of dung uses little-endian byte sex.  With the new v7 and above
// this heap of SHIT uses little-endian and big-endian byte sex.

unsigned short get_be_short(char *p)
{
  unsigned char *q = (unsigned char *)p;
  return (q[0] << 8) + q[1];
}

unsigned int get_be_int(char *p)
{
  unsigned char *q = (unsigned char *)p;
  return (q[0] << 24) + (q[1] << 16) + (q[2] << 8) + q[3];
}

unsigned long get_be_long(char *p)
{
  unsigned char *q = (unsigned char *)p;
  return (((unsigned long)(q[0]) << 24) +
	  ((unsigned long)(q[1]) << 16) +
	   ((unsigned long)(q[2]) << 8) +
	    ((unsigned long)q[3]));
}

void put_be_short(char *p, unsigned short x)
{
  unsigned char *q = (unsigned char *)p;
  q[0] = ((x & 0xff00) >> 8);
  q[1] = (x & 0x00ff);
}

void put_be_int(char *p, unsigned int x)
{
  unsigned char *q = (unsigned char *)p;
  q[0] = ((x & 0xff000000) >> 24);
  q[1] = ((x & 0x00ff0000) >> 16);
  q[2] = ((x & 0x0000ff00) >> 8);
  q[3] = ((x & 0x000000ff));
}

void put_be_long(char *p, unsigned long x)
{
  unsigned char *q = (unsigned char *)p;
  q[0] = ((x & 0xff000000) >> 24);
  q[1] = ((x & 0x00ff0000) >> 16);
  q[2] = ((x & 0x0000ff00) >> 8);
  q[3] = ((x & 0x000000ff));
}

// return short (16-bit) stored in little-endian format, possibly unaligned
unsigned short get_le_short(char *p)
{
   unsigned char *q = (unsigned char *)p;
   return q[0] + (q[1] << 8);
}

// return int (32-bit) stored in little-endian format, possibly unaligned
unsigned int get_le_int(char *p)
{
   unsigned char *q = (unsigned char *)p;
   return q[0] + (q[1] << 8) + (q[2] << 16) + (q[3] << 24);
}

// return long (32-bit) stored in little-endian format, possibly unaligned
unsigned long get_le_long(char *p)
{
   unsigned char *q = (unsigned char *)p;
   // $C6.1 Promotions: unsigned char gets converted to int by default
   return ((unsigned long)q[0]) +
       (((unsigned long)q[1]) << 8) +
       (((unsigned long)q[2]) << 16) +
       (((unsigned long)q[3]) << 24);

}

// store 16-bit short in little-endian format, possibly unaligned
void put_le_short(char *p, unsigned short x)
{
   unsigned char *q = (unsigned char*)p;
   q[0] = x & 0xff;
   q[1] = (x >> 8) & 0xff;
}

// store 32-bit int in little-endian format, possibly unaligned
void put_le_int( char *p, unsigned int x)
{
   unsigned char *q = (unsigned char*)p;
   q[0] = x & 0xff;
   q[1] = (x >> 8) & 0xff;
   q[2] = (x >> 16) & 0xff;
   q[3] = (x >> 24) & 0xff;
}

// store 32-bit int in little-endian format, possibly unaligned
void put_le_long(char *p, unsigned long x)
{
   unsigned char *q = (unsigned char*)p;
   q[0] = x & 0xff;
   q[1] = (x >> 8) & 0xff;
   q[2] = (x >> 16) & 0xff;
   q[3] = (x >> 24) & 0xff;
}

//---- Reverse Endianism Functions -------------

void rev_e_short(unsigned short &x)
{
  unsigned short tmp = 0;

  tmp = ((x & 0x00ff) << 8);
  tmp |= ((x & 0xff00) >> 8);

  x = tmp;
}

void rev_e_long(unsigned long &x)
{
  unsigned long tmp = 0;

  tmp = ((x & 0x000000ff) << 24);
  tmp |= ((x & 0x0000ff00) << 8);
  tmp |= ((x & 0x00ff0000) >> 8);
  tmp |= ((x & 0xff000000) >> 24);

  x = tmp;
}

//=====Buffer================================================================

CBuffer::CBuffer()
{
  m_pDataStart = m_pDataPosRead = m_pDataPosWrite = NULL;
  m_pTLV = NULL;
  m_nDataSize = 0;
}


CBuffer::CBuffer(unsigned long _nDataSize)
{
  m_pTLV = NULL;
  m_nDataSize = _nDataSize;
  m_pDataStart = new char[m_nDataSize];
  m_pDataPosRead = m_pDataPosWrite = m_pDataStart;
}

CBuffer::CBuffer(const CBuffer &b)
{
  m_nDataSize = b.getDataMaxSize();
  m_pDataStart = new char[m_nDataSize];
  memcpy(m_pDataStart, b.getDataStart(), m_nDataSize);
  m_pDataPosRead = m_pDataStart + (b.getDataPosRead() - b.getDataStart());
  m_pDataPosWrite = m_pDataStart + (b.getDataPosWrite() - b.getDataStart());
  // DAW FIXME
  m_pTLV = NULL;
}

#if 0
CBuffer::CBuffer(CBuffer *b)
{
  if (b == NULL)
  {
     m_pDataStart = m_pDataPosRead = m_pDataPosWrite = NULL;
     m_pTLV = NULL;
     m_nDataSize = 0;
  }
  else
  {
    m_pTLV = NULL;
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
   m_pDataStart = new char[m_nDataSize];
   memcpy(m_pDataStart, b.getDataStart(), m_nDataSize);
   m_pDataPosRead = m_pDataStart + (b.getDataPosRead() - b.getDataStart());
   m_pDataPosWrite = m_pDataStart + (b.getDataPosWrite() - b.getDataStart());
   m_pTLV = NULL;

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
   m_pTLV = NULL;
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
      in = get_le_short(getDataPosRead());
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
    in = get_le_long(getDataPosRead());
    incDataPosRead(4);
  }
  return(*this);
}

char *CBuffer::UnpackStringBE(char* sz)
{
  unsigned char nLen;
  sz[0] = '\0';
  *this >> nLen;
  for (unsigned short i = 0; i < nLen; i++) *this >> sz[i];
  sz[nLen] = '\0';
  return sz;
}

char *CBuffer::UnpackString(char *sz)
{
  unsigned short nLen;
  sz[0] = '\0';
  *this >> nLen;
  for (unsigned short i = 0; i < nLen; i++) *this >> sz[i];
  sz[nLen] = '\0';
  return sz;
}

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

unsigned long CBuffer::UnpackUnsignedLong()
{
  unsigned long n;
  *this >> n;
  return n;
}

unsigned long CBuffer::UnpackUinString()
{
  unsigned char len;

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
    n = get_be_long(getDataPosRead());
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
    n = get_be_short(getDataPosRead());
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
  if (m_pTLV) cleanupTLV();
  m_pTLV = NULL;
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
  if (m_pTLV) cleanupTLV();
}

//-----add----------------------------------------------------------------------
char *CBuffer::PackUnsignedLong(unsigned long data)
{
  put_le_long(getDataPosWrite(), data);
  incDataPosWrite(4);
  return getDataPosWrite() - 4;
}

char *CBuffer::PackUnsignedLongBE(unsigned long data)
{
  put_be_long(getDataPosWrite(), data);
  incDataPosWrite(4);
  return getDataPosWrite() - 4;
}

char *CBuffer::PackChar(char data)
{
  *getDataPosWrite() = data;
  incDataPosWrite(1);
  return getDataPosWrite() - 1;
}

char *CBuffer::Pack(const char *data, int size)
{
  if (!size) return getDataPosWrite();
  memcpy(getDataPosWrite(), data, size);
  incDataPosWrite(size);
  return getDataPosWrite() - size;
}

char *CBuffer::Pack(CBuffer *buf)
{
  memcpy(getDataPosWrite(), buf->getDataStart(), buf->getDataSize());
  incDataPosWrite(buf->getDataSize());
  return getDataPosWrite() - buf->getDataSize();
}

char *CBuffer::PackString(const char *data, unsigned short max)
{
  unsigned short n = (data == NULL ? 0 : strlen(data));
  if (max > 0 && n > max) n = max;
  put_le_short(getDataPosWrite(), n + 1);
  incDataPosWrite(2);
  memcpy(getDataPosWrite(), data, n);
  incDataPosWrite(n);
  *getDataPosWrite() = '\0';
  incDataPosWrite(1);
  return getDataPosWrite() - n - 1;
}

char *CBuffer::PackUnsignedShort(unsigned short data)
{
  put_le_short(getDataPosWrite(), data);
  incDataPosWrite(2);
  return getDataPosWrite() - 2;
}

char *CBuffer::PackUnsignedShortBE(unsigned short data)
{
  put_be_short(getDataPosWrite(), data);
  incDataPosWrite(2);
  return getDataPosWrite() - 2;
}

//-----TLV----------------------------------------------------------------------

bool CBuffer::readTLV(int count)
{
  if (m_pTLV || !count) return false; // already have one

  int num = 0;
  while(getDataPosRead() + 4 < (getDataStart() + getDataSize())) {
    SOscarTLV_Chain *now = new SOscarTLV_Chain;
    now->pTLV = new SOscarTLV;

    *this >> now->pTLV->nType;
    *this >> now->pTLV->nLen;


    rev_e_short(now->pTLV->nType);
    rev_e_short(now->pTLV->nLen);

    //gLog.Info("got type %d, len: %d\n", now->pTLV->nType, now->pTLV->nLen);

    if(getDataPosRead() + now->pTLV->nLen > (getDataStart() + getDataSize()) ||
       now->pTLV->nLen < 1) {
      now->pTLV->nLen = 0;
      now->pTLV->pData = 0;
    }
    else {
      now->pTLV->pData = new unsigned char[now->pTLV->nLen];
      memcpy(now->pTLV->pData, m_pDataPosRead, now->pTLV->nLen);
      m_pDataPosRead += now->pTLV->nLen;
    }

    now->pNext = m_pTLV;
    m_pTLV = now;

    ++num;
    if (count > 0 && num == count)
      return true;
  }

  return true;
}

void CBuffer::cleanupTLV()
{
  if (!m_pTLV) return;

  SOscarTLV_Chain *now = m_pTLV;

  while (now) {
    SOscarTLV_Chain *tmp;

    if (now->pTLV)
    {
      if (now->pTLV->pData)
	delete [] now->pTLV->pData;

      delete now->pTLV;
    }

    tmp = now->pNext;
    delete now;
    now = tmp;
  }

  m_pTLV = NULL;
}

void CBuffer::PackTLV(unsigned short nType, unsigned short nSize,
		       const char *data)
{
  PackUnsignedShortBE(nType);
  PackUnsignedShortBE(nSize);
  Pack(data, nSize);
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

unsigned short CBuffer::getTLVLen(unsigned short nType, unsigned short nIndex)
{
  unsigned short nLen = 0;

  SOscarTLV *pTLV = getTLV(nType, nIndex);
  if (pTLV)
  {
    nLen = pTLV->nLen;
  }

  return nLen;
}

unsigned long CBuffer::UnpackUnsignedLongTLV(unsigned short nType,
					     unsigned short nIndex)
{
  unsigned long nRet = 0;

  SOscarTLV *pTLV = getTLV(nType, nIndex);
  if (pTLV && pTLV->nLen > 3)
  {
    nRet |= (*((pTLV->pData)+0) << 24);
    nRet |= (*((pTLV->pData)+1) << 16);
    nRet |= (*((pTLV->pData)+2) << 8);
    nRet |= (*((pTLV->pData)+3));
  }

  return nRet;
}

unsigned short CBuffer::UnpackUnsignedShortTLV(unsigned short nType,
					       unsigned short nIndex)
{
  unsigned short nRet = 0;

  SOscarTLV *pTLV = getTLV(nType, nIndex);
  if (pTLV && pTLV->nLen > 1)
  {
    nRet |= (*((pTLV->pData)+0) << 8);
    nRet |= (*((pTLV->pData)+1));
  }

  return nRet;
}

unsigned char CBuffer::UnpackCharTLV(unsigned short nType, unsigned short nIndex)
{
  unsigned char nRet = 0;

  SOscarTLV *pTLV = getTLV(nType, nIndex);
  if (pTLV && pTLV->nLen > 0)
    nRet = *(pTLV->pData);

  return nRet;
}

char *CBuffer::UnpackStringTLV(unsigned short nType, unsigned short nIndex)
{
  char *str = 0;

  SOscarTLV *pTLV = getTLV(nType, nIndex);
  if (pTLV)
  {
    str = new char[pTLV->nLen+1];
    if (!str) return 0;

    memcpy(str, pTLV->pData, pTLV->nLen);
    *(str+pTLV->nLen) = '\0';
  }

  return str;
}

CBuffer CBuffer::UnpackTLV(unsigned short nType, unsigned short nIndex)
{
  SOscarTLV *pTLV = getTLV(nType, nIndex);
  if (pTLV)
  {
    CBuffer cbuf(pTLV->nLen);
    cbuf.Pack((const char*)pTLV->pData, pTLV->nLen);
    cbuf.Reset();

    return cbuf;
  }
  else
    return CBuffer(0);
}

SOscarTLV *CBuffer::getTLV(unsigned short nType, unsigned short nIndex)
{
  if (!m_pTLV) return 0;

  SOscarTLV_Chain *now;
  int i = 0;

  for (now = m_pTLV; now; now = now->pNext)
  {
    if (now && now->pTLV)
    {
      if (now->pTLV->nType == nType)
	i++;

      if (i >= nIndex) {

	return now->pTLV;
      }
    }
  }

  return 0;
}

//-----print--------------------------------------------------------------------
char *CBuffer::print(char *&p)
{
   static const unsigned long MAX_DATA_SIZE = 8 * 1024;
   static const char BUFFER_BLANKS[] = "     ";
   static const unsigned long SPACE_PER_LINE =
    strlen(BUFFER_BLANKS) + strlen("0000: ") + 16 * 3 + 18 + 4;

   unsigned long nBytesToPrint = getDataSize();
   char szAscii[16 + 1];
   szAscii[16] = '\0';

   if (nBytesToPrint > MAX_DATA_SIZE)
   {
     nBytesToPrint = MAX_DATA_SIZE;
   }

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

     if (i >= nBytesToPrint) break;

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












