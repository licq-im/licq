#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>

#include "buffer.h"
#include "log.h"


//=====Utilities=================================================================

// Endianness utility routines: Unlike Real Internet Protocols, this
// heap of dung uses little-endian byte sex.

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


//=====Buffer================================================================

CBuffer::CBuffer()
{
  m_pDataStart = m_pDataPosRead = m_pDataPosWrite = NULL;
  m_nDataSize = 0;
}


CBuffer::CBuffer(unsigned long _nDataSize)
{
  m_nDataSize = _nDataSize;
  m_pDataStart = new char[m_nDataSize];
  m_pDataPosRead = m_pDataPosWrite = m_pDataStart;
}

CBuffer::CBuffer(CBuffer &b)
{
  m_nDataSize = b.getDataMaxSize();
  m_pDataStart = new char[m_nDataSize];
  memcpy(m_pDataStart, b.getDataStart(), m_nDataSize);
  m_pDataPosRead = m_pDataStart + (b.getDataPosRead() - b.getDataStart());
  m_pDataPosWrite = m_pDataStart + (b.getDataPosWrite() - b.getDataStart());

}

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


char *CBuffer::UnpackString(char *sz)
{
  unsigned short nLen;
  sz[0] = '\0';
  *this >> nLen;
  for (unsigned short i = 0; i < nLen; i++) *this >> sz[i];
  return sz;
}

unsigned long CBuffer::UnpackUnsignedLong()
{
  unsigned long n;
  *this >> n;
  return n;
}

unsigned short CBuffer::UnpackUnsignedShort()
{
  unsigned short n;
  *this >> n;
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
  m_pDataStart = m_pDataPosRead = m_pDataPosWrite = NULL;
  m_nDataSize = 0;
}


//-----reset--------------------------------------------------------------------
void CBuffer::Reset()
{
  setDataPosRead(getDataStart());
  setDataPosWrite(getDataStart());
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


CBuffer::~CBuffer()
{
  if (m_pDataStart != NULL) delete[] m_pDataStart;
}

//-----add----------------------------------------------------------------------
char *CBuffer::PackUnsignedLong(unsigned long data)
{
  put_le_long(getDataPosWrite(), data);
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
  memcpy(getDataPosWrite(), data, size);
  incDataPosWrite(size);
  return getDataPosWrite() - size;
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


//-----print--------------------------------------------------------------------
char *CBuffer::print(char *&p)
{
   static const unsigned short BYTES_PER_LINE = 20;
   static const unsigned long MAX_DATA_SIZE = 1024 * 1024;

   if (getDataSize() > MAX_DATA_SIZE)
   {
     gLog.Error("%sInternal error: CBuffer::print(): Packet is suspiciously large (%lu bytes).\n%sAborting print.\n",
                L_ERRORxSTR, L_BLANKxSTR, getDataSize());
     p = new char[32];
     strcpy(p, "- E R R O R -");
     return (p);
   }

   unsigned short nLenBlank = strlen(L_BLANKxSTR);
   unsigned short nLenBuf = nLenBlank + getDataSize() * 3
     + (int)(getDataSize() / BYTES_PER_LINE) * (1 + nLenBlank)
     + (int)(getDataSize() / 4) * 2
     + 4;
   p = new char[nLenBuf];
   char *pPos = p;
   sprintf(pPos, "%s", L_BLANKxSTR);
   pPos += nLenBlank;
   for(unsigned long i = 0; i < getDataSize(); i++)
   {
      sprintf(pPos, "%02X ", (unsigned char)getDataStart()[i]);
      pPos += 3;

      if((i + 1) % BYTES_PER_LINE == 0)
      {
        sprintf(pPos, "\n%s", L_BLANKxSTR);
        pPos += 1 + nLenBlank;
      }
      /*else if((i + 1) % 4 == 0)
        pPos += sprintf(pPos, "- ");*/
   }
   return(p);
}

