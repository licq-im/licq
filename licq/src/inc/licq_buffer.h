#ifndef LICQ_BUFFER_H
#define LICQ_BUFFER_H

/*------------------------------------------------------------------------------
 * PacketIpToNetworkIp
 *
 * Takes an ip from the buffer class and converts it to network byte order:
 * Little endian machine:
 *  Packet returns ip in big-endian -> reverse digits -> call htonl
 * Big endian machine:
 *  Packet returns ip in little-endian -> reverse digits -> call htonl (does nothing)
 *----------------------------------------------------------------------------*/
extern unsigned long PacketIpToNetworkIp(unsigned long l);

/*------------------------------------------------------------------------------
 * NetworkIpToPacketIp
 *
 * Takes an ip in network order and converts it to the packet class format
 * Little endian machine:
 *  Packet returns ip in big-endian -> reverse digits -> call htonl
 * Big endian machine:
 *  Packet returns ip in little-endian -> reverse digits -> call htonl (does nothing)
 *----------------------------------------------------------------------------*/
extern unsigned long NetworkIpToPacketIp(unsigned long l);

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
