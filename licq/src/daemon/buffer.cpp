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
   return q[0] + (q[1] << 8) + (q[2] << 16) + (q[3] << 24);
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

CBuffer::CBuffer(void)
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
void CBuffer::Create(unsigned long _nDataSize = 0)
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
   if(getDataPosRead() + sizeof(unsigned short) > (getDataStart() + getDataSize())) 
      in = 0;
   else
   {
      in = get_le_short(getDataPosRead());
      incDataPosRead(sizeof(unsigned short));
   }
   return(*this);
}

CBuffer& CBuffer::operator>>(unsigned long &in)
{
  if(getDataPosRead() + sizeof(unsigned long) > (getDataStart() + getDataSize())) 
    in = 0;
  else
  {
    in = get_le_long(getDataPosRead());
    incDataPosRead(sizeof(unsigned long));
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

unsigned long CBuffer::UnpackUnsignedLong(void)
{
  unsigned long n;
  *this >> n;
  return n;
}

unsigned short CBuffer::UnpackUnsignedShort(void)
{
  unsigned short n;
  *this >> n;
  return n;
}

char CBuffer::UnpackChar(void)
{
  char n;
  *this >> n;
  return n;
}



//-----clear--------------------------------------------------------------------
void CBuffer::Clear(void)
{
  if (m_pDataStart != NULL) delete[] m_pDataStart;
  m_pDataStart = m_pDataPosRead = m_pDataPosWrite = NULL;
  m_nDataSize = 0;
}


//-----reset--------------------------------------------------------------------
void CBuffer::Reset(void)
{
  setDataPosRead(getDataStart());
  setDataPosWrite(getDataStart());
}

//-----Empty--------------------------------------------------------------------
bool CBuffer::Empty(void)
{
  return (m_pDataStart == NULL);
}

//-----Full---------------------------------------------------------------------
bool CBuffer::Full(void)
{
  return (!Empty() && getDataPosWrite() >= (getDataStart() + getDataMaxSize()));
}


CBuffer::~CBuffer(void)
{
  if (m_pDataStart != NULL) delete[] m_pDataStart;
}

//-----add----------------------------------------------------------------------
unsigned short CBuffer::add(const unsigned short &data)
{
  put_le_short(getDataPosWrite(), data);
  incDataPosWrite(sizeof(unsigned short));
  return sizeof(unsigned short);
}

unsigned short CBuffer::add(const unsigned long &data)
{
  put_le_long(getDataPosWrite(), data);
  incDataPosWrite(sizeof(unsigned long));
  return sizeof(unsigned long);
}

unsigned short CBuffer::add(const char &data)
{
  *getDataPosWrite() = data;
  incDataPosWrite(sizeof(char));
  return sizeof(char);
}

unsigned short CBuffer::add(const char *data, int size)
{
   memcpy(getDataPosWrite(), data, size);
   incDataPosWrite(size);
   return size;
}

unsigned short CBuffer::PackString(const char *data)
{
   unsigned short n = strlen(data) + 1;
   add(n);
   add(data, n);
   // We just added n+2 bytes to the packet
   return n + 2;
}


//-----print--------------------------------------------------------------------
char *CBuffer::print(char *&p)
{
   static const unsigned short BYTES_PER_LINE = 24;
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
   }   
   return(p);
}

