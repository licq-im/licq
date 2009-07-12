#ifndef LICQ_BUFFER_H
#define LICQ_BUFFER_H

#include <cstring>
#include <map>
#include <string>

#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>


//=====COscarTLV================================================================
class COscarTLV
{
public:
  COscarTLV(unsigned short type = 0, unsigned short length = 0, const char* data = NULL) : myType(type), myLen(length)
  {
    if (myLen > 0)
    {
      myData = boost::shared_array<unsigned char>(new  unsigned char[myLen]);
      memcpy(myData.get(), data, myLen);
    }
  }

  COscarTLV(const COscarTLV &c)
  {
    myType = c.myType;
    myLen = c.myLen;
    myData = boost::shared_array<unsigned char>(new unsigned char[c.myLen]);
    memcpy(myData.get(), c.myData.get(), c.myLen);
  }

  unsigned short getType() const                     { return myType; }
  unsigned short getLength() const                   { return myLen; }
  boost::shared_array<unsigned char> getData() const { return myData; }

  void setType(unsigned short type) { myType = type; }
  void setData(unsigned char *data, unsigned short length)
  {
    if (length > 0)
    {
      myLen = length;
      myData = boost::shared_array<unsigned char>(new unsigned char[length]);
      memcpy(myData.get(), data, length);
    }
  }

private:
  unsigned short myType;
  unsigned short myLen;
  boost::shared_array<unsigned char> myData;

friend class CBuffer;
};

typedef boost::shared_ptr<COscarTLV> TLVPtr;
typedef std::map<unsigned short, TLVPtr> TLVList;
typedef TLVList::iterator TLVListIter;

//=====CBuffer==================================================================
class CBuffer
{
public:
   CBuffer();
   CBuffer(unsigned long _nSize);
   CBuffer(const CBuffer &);
   ~CBuffer();

   CBuffer& operator=(CBuffer &);
   CBuffer& operator+=(CBuffer &);
   friend CBuffer operator+(CBuffer &, CBuffer &);

   //-- Big Endian --
   char *PackBE(CBuffer *);
   char *PackBE(const char *data, int size);
   char *PackStringBE(const char *data, unsigned short max = 0);
   char *PackUnsignedShortBE(unsigned short data);
   char *PackUnsignedLongBE(unsigned long data);

   char *Pack(CBuffer *);
   char *Pack(const char *data, int size);
   char *PackLNTS(const char *);
   char *PackString(const char *data, unsigned short max = 0);
   char *PackUnsignedShort(unsigned short data);
   char *PackUnsignedLong(unsigned long data);
   char *PackChar(char data);
   void Copy(CBuffer *);
   char *print(char *&);
   void Clear();
   void Reset();
   bool Empty();
   bool Full();
   bool End()  { return ( getDataPosRead() >= (getDataStart() + getDataSize()) ); }
   void Create(unsigned long _nDataSize = 0);

   //-- Big Endian --
   unsigned long UnpackUnsignedLongBE();
   unsigned short UnpackUnsignedShortBE();

   CBuffer& operator >> (char &in);
   CBuffer& operator >> (unsigned char &in);
   CBuffer& operator >> (unsigned short &in);
   CBuffer& operator >> (unsigned long &in);

  /**
   * Get several bytes from the buffer
   *
   * @param size Number of bytes to read
   * @return A string containing the requested bytes
   */
  std::string unpackRawString(size_t size);

   char *UnpackRaw(char *, unsigned short);
   char *UnpackBinBlock(char *, unsigned short);
   char *UnpackString(char *, unsigned short);
   char *UnpackString();                // Need to delete[] returned string
   char *UnpackStringBE(char *, unsigned short);
   char *UnpackStringBE();              // Need to delete[] returned string
   char *UnpackUserString();            // Need to delete[] returned string
   unsigned long UnpackUnsignedLong();
   unsigned long UnpackUinString();
   unsigned short UnpackUnsignedShort();
   char UnpackChar();

   char *getDataStart() const           { return m_pDataStart; };
   char *getDataPosRead() const         { return m_pDataPosRead; };
   char *getDataPosWrite() const        { return m_pDataPosWrite; };
   unsigned long getDataSize() const    { return m_pDataPosWrite - m_pDataStart; };
   unsigned long getDataMaxSize() const { return m_nDataSize; };

   void setDataSize(unsigned long _nDataSize)  { m_nDataSize = _nDataSize; };
   void setDataPosWrite(char *_pDataPosWrite)  { m_pDataPosWrite = _pDataPosWrite; };
   void setDataPosRead(char *_pDataPosRead)  { m_pDataPosRead = _pDataPosRead; };
   void incDataPosWrite(unsigned long c)  { m_pDataPosWrite += c; };
   void incDataPosRead(unsigned long c)  { m_pDataPosRead += c; };

   //--- OSCAR Related Functions ------

   bool readTLV(int count = -1, int bytes = -1); // This should be called automatically if m_pTLV == 0

   void PackTLV(unsigned short, unsigned short, const char *);
   void PackTLV(unsigned short, unsigned short, CBuffer *);
   void PackTLV(const TLVPtr&);

   unsigned short getTLVLen(unsigned short);
   bool hasTLV(unsigned short);

   unsigned long UnpackUnsignedLongTLV(unsigned short);
   unsigned short UnpackUnsignedShortTLV(unsigned short);
   unsigned char UnpackCharTLV(unsigned short);
   char *UnpackStringTLV(unsigned short); // Need to delete[] returned string
   //std::string UnpackStringTLV(unsigned short);
   CBuffer UnpackTLV(unsigned short);

   TLVList getTLVList();
   TLVPtr getTLV(unsigned short _nType);

private:
   CBuffer& operator=(const CBuffer&);

protected:

   char *m_pDataStart,
        *m_pDataPosWrite,
        *m_pDataPosRead;
   unsigned long m_nDataSize;
   TLVList myTLVs;
};


#endif
