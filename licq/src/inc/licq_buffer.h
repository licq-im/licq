#ifndef CBUFFER_H
#define CBUFFER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <netinet/in.h>

/*------------------------------------------------------------------------------
 * PacketIpToNetworkIp
 *
 * Takes an ip from the buffer class and converts it to network byte order:
 * Little endian machine:
 *  Packet returns ip in big-endian -> reverse digits -> call htonl
 * Big endian machine:
 *  Packet returns ip in little-endian -> reverse digits -> call htonl (does nothing)
 *----------------------------------------------------------------------------*/
static unsigned long PacketIpToNetworkIp(unsigned long l)
{
	return htonl((l << 24) | ((l & 0xff00) << 8) | ((l & 0xff0000) >> 8) | (l >> 24));
}

/*------------------------------------------------------------------------------
 * NetworkIpToPacketIp
 *
 * Takes an ip in network order and converts it to the packet class format
 * Little endian machine:
 *  Packet returns ip in big-endian -> reverse digits -> call htonl
 * Big endian machine:
 *  Packet returns ip in little-endian -> reverse digits -> call htonl (does nothing)
 *----------------------------------------------------------------------------*/
static unsigned long NetworkIpToPacketIp(unsigned long l)
{
	l = ntohl(l);
	return (l << 24) | ((l & 0xff00) << 8) | ((l & 0xff0000) >> 8) | (l >> 24);
}


//=====CBuffer==================================================================
class CBuffer
{
public:
   CBuffer();
   CBuffer(unsigned long _nSize);
   CBuffer(CBuffer *);
   CBuffer(CBuffer &);
   ~CBuffer();

   char *Pack(const char *data, int size);
   char *PackString(const char *data, unsigned short max = 0);
   char *PackUnsignedShort(unsigned short data);
   char *PackUnsignedLong(unsigned long data);
   char *PackChar(char data);
   char *print(char *&);
   void Clear();
   void Reset();
   bool Empty();
   bool Full();
   bool End()  { return ( getDataPosRead() >= (getDataStart() + getDataSize()) ); }
   void Create(unsigned long _nDataSize = 0);

   CBuffer& operator >> (char &in);
   CBuffer& operator >> (unsigned char &in);
   CBuffer& operator >> (unsigned short &in);
   CBuffer& operator >> (unsigned long &in);
   char *UnpackString(char *);
   unsigned long UnpackUnsignedLong();
   unsigned short UnpackUnsignedShort();
   char UnpackChar();

   char *getDataStart()           { return m_pDataStart; };
   char *getDataPosRead()         { return m_pDataPosRead; };
   char *getDataPosWrite()        { return m_pDataPosWrite; };
   unsigned long getDataSize()    { return m_pDataPosWrite - m_pDataStart; };
   unsigned long getDataMaxSize() { return m_nDataSize; };

   void setDataSize(unsigned long _nDataSize)  { m_nDataSize = _nDataSize; };
   void setDataPosWrite(char *_pDataPosWrite)  { m_pDataPosWrite = _pDataPosWrite; };
   void setDataPosRead(char *_pDataPosRead)  { m_pDataPosRead = _pDataPosRead; };
   void incDataPosWrite(unsigned long c)  { m_pDataPosWrite += c; };
   void incDataPosRead(unsigned long c)  { m_pDataPosRead += c; };

protected:
   char *m_pDataStart,
        *m_pDataPosWrite,
        *m_pDataPosRead;
   unsigned long m_nDataSize;

   void antiwarning() { NetworkIpToPacketIp(PacketIpToNetworkIp(127)); }
};


#endif
