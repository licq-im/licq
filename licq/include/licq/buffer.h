/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq developers
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

#ifndef LICQ_BUFFER_H
#define LICQ_BUFFER_H

#include "logging/log.h"

#include <map>
#include <string>

#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>


namespace Licq
{

class OscarTlv
{
public:
  OscarTlv(unsigned short type = 0, unsigned short length = 0, const char* data = NULL);

  OscarTlv(const OscarTlv& c);

  unsigned short getType() const                     { return myType; }
  unsigned short getLength() const                   { return myLen; }
  boost::shared_array<unsigned char> getData() const { return myData; }

  void setType(unsigned short type) { myType = type; }
  void setData(unsigned char* data, unsigned short length);

private:
  unsigned short myType;
  unsigned short myLen;
  boost::shared_array<unsigned char> myData;

  friend class Buffer;
};

typedef boost::shared_ptr<OscarTlv> TlvPtr;
typedef std::map<unsigned short, TlvPtr> TlvList;


class Buffer
{
public:
  Buffer();
  Buffer(unsigned long _nSize);
  Buffer(const Buffer&);
  ~Buffer();

  Buffer& operator=(Buffer&);
  Buffer& operator+=(Buffer&);
  friend Buffer operator+(Buffer&, Buffer&);

   //-- Big Endian --
  char* PackBE(Buffer*);
   char *PackBE(const char *data, int size);
   char *PackStringBE(const char *data, unsigned short max = 0);
   char *PackUnsignedShortBE(unsigned short data);
   char *PackUnsignedLongBE(unsigned long data);

  char* Pack(Buffer*);
   char *Pack(const char *data, int size);

  char* pack(const std::string& data)
  { return Pack(data.c_str(), data.size()); }

   char *PackLNTS(const char *);
   char *PackString(const char *data, unsigned short max = 0);
  char* packString(const std::string& data, unsigned short max = 0)
  { return PackString(data.c_str(), max); }
   char *PackUnsignedShort(unsigned short data);
   char *PackUnsignedLong(unsigned long data);
   char *PackChar(char data);
  void Copy(Buffer*);

   /**
    * Log the packet with the given message.
    */
  void log(Log::Level level, const char* format, ...)
#ifdef __GNUC__
      __attribute__((format (printf, 3, 4)))
#endif
   ;

   void Clear();
   void Reset();
   bool Empty() const;
   bool Full() const;
   bool End()  { return ( getDataPosRead() >= (getDataStart() + getDataSize()) ); }
   void Create(unsigned long _nDataSize = 0);

   //-- Big Endian --
   unsigned long UnpackUnsignedLongBE();
   unsigned short UnpackUnsignedShortBE();

  Buffer& operator>>(char &in);
  Buffer& operator>>(unsigned char &in);
  Buffer& operator>>(unsigned short &in);
  Buffer& operator>>(unsigned long &in);

  /**
   * Get several bytes from the buffer
   *
   * @param size Number of bytes to read
   * @return A string containing the requested bytes
   */
  std::string unpackRawString(size_t size);

  /**
   * Get a string from the buffer
   * Length of string is a 16 bit word at beginning of data
   *
   * @return String read
   */
  std::string unpackString();

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
  void PackTLV(unsigned short, unsigned short, Buffer*);
  void PackTLV(const TlvPtr&);

   unsigned short getTLVLen(unsigned short);
   bool hasTLV(unsigned short);

   unsigned long UnpackUnsignedLongTLV(unsigned short);
   unsigned short UnpackUnsignedShortTLV(unsigned short);
   unsigned char UnpackCharTLV(unsigned short);
   char *UnpackStringTLV(unsigned short); // Need to delete[] returned string
   //std::string UnpackStringTLV(unsigned short);
  Buffer UnpackTLV(unsigned short);

  TlvList getTlvList();
  TlvPtr getTLV(unsigned short _nType);

private:
  Buffer& operator=(const Buffer&);

protected:

   char *m_pDataStart,
        *m_pDataPosWrite,
        *m_pDataPosRead;
   unsigned long m_nDataSize;
  TlvList myTLVs;
};

Buffer operator+(Buffer& b0, Buffer& b1);

} // namespace Licq

#endif
