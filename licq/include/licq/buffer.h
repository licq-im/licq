/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010,2012 Licq developers <licq-dev@googlegroups.com>
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

  /// Add an unsigned 32 bit little endian integer from the buffer
  void packUInt32LE(uint32_t data);

  /// Add an unsigned 32 bit big endian integer from the buffer
  void packUInt32BE(uint32_t data);

  /// Add an unsigned 16 bit little endian integer from the buffer
  void packUInt16LE(uint16_t data);

  /// Add an unsigned 16 bit big endian integer from the buffer
  void packUInt16BE(uint16_t data);

  /// Add an unsigned 8 bit integer from the buffer
  void packUInt8(uint8_t data);

  /// Add an signed 8 bit integer from the buffer
  void packInt8(int8_t data);

  /// Add binary data
  void packRaw(const void* data, size_t length);
  void packRaw(const std::string& data) { packRaw(data.c_str(), data.size()); }

  /// Add a null terminated string preceded by a 16bit little endian length
  void packShortNullStringLE(const std::string& data);

  void Pack(Buffer*);

  void Copy(Buffer*);

  // Deprecated add functions
  void PackUnsignedLong(unsigned long data) { packUInt32LE(data); }
  void PackUnsignedLongBE(unsigned long data) { packUInt32BE(data); }
  void PackUnsignedShort(unsigned short data) { packUInt16LE(data); }
  void PackUnsignedShortBE(unsigned short data) { packUInt16BE(data); }
  void PackChar(char data) { packInt8(data); }
  void Pack(const char* data, int size) { packRaw(data, size); }
  void Pack(const uint8_t* data, int size) { packRaw(data, size); }
  void pack(const std::string& data) { packRaw(data); }
  void PackString(const char *data) { packShortNullStringLE(data); }
  void packString(const std::string& data) { packShortNullStringLE(data); }

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
   * Length of string is a 8 bit byte at beginning of data
   *
   * @return String read
   */
  std::string unpackByteString();

  /**
   * Get a string from the buffer
   * Length of string is a 16 bit little endian word at beginning of data
   *
   * @return String read
   */
  std::string unpackShortStringLE();

  /**
   * Get a string from the buffer
   * Length of string is a 16 bit big endian word at beginning of data
   *
   * @return String read
   */
  std::string unpackShortStringBE();

  /**
   * Get a string from the buffer
   * Length of string is a 32 bit little endian word at beginning of data
   *
   * @return String read
   */
  std::string unpackLongStringLE();

  /**
   * Get a string from the buffer
   * Length of string is a 32 bit big endian word at beginning of data
   *
   * @return String read
   */
  std::string unpackLongStringBE();

  /// Get an unsigned 32 bit little endian integer from the buffer
  uint32_t unpackUInt32LE();

  /// Get an unsigned 32 bit big endian integer from the buffer
  uint32_t unpackUInt32BE();

  /// Get an unsigned 16 bit little endian integer from the buffer
  uint16_t unpackUInt16LE();

  /// Get an unsigned 16 bit big endian integer from the buffer
  uint16_t unpackUInt16BE();

  /// Get an unsigned 8 bit integer from the buffer
  uint8_t unpackUInt8();

  /// Get a signed 8 bit integer from the buffer
  int8_t unpackInt8();

  // Deprecated integer access functions
  unsigned long UnpackUnsignedLongBE() { return unpackUInt32BE(); }
  unsigned short UnpackUnsignedShortBE() { return unpackUInt16BE(); }
  unsigned long UnpackUnsignedLong() { return unpackUInt32LE(); }
  unsigned short UnpackUnsignedShort() { return unpackUInt16LE(); }
  char UnpackChar() { return unpackInt8(); }

   char *getDataStart() const           { return m_pDataStart; };
   char *getDataPosRead() const         { return m_pDataPosRead; };
   char *getDataPosWrite() const        { return m_pDataPosWrite; };
   unsigned long getDataSize() const    { return m_pDataPosWrite - m_pDataStart; };
   unsigned long getDataMaxSize() const { return m_nDataSize; };

  /**
   * Get number of available bytes left to read
   */
  size_t remainingDataToRead() const
  { return m_pDataPosWrite - m_pDataPosRead; }

  /**
   * Get number of available bytes left to write
   */
  size_t remainingDataToWrite() const
  { return m_pDataStart + m_nDataSize - m_pDataPosWrite; }

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

  uint32_t unpackTlvUInt32(int type);
  uint16_t unpackTlvUInt16(int type);
  uint8_t unpackTlvUInt8(int type);
  std::string unpackTlvString(int type);
  Buffer UnpackTLV(unsigned short);

  TlvList getTlvList();
  TlvPtr getTLV(unsigned short _nType);

  // Deprecated TLV access functions
  unsigned long UnpackUnsignedLongTLV(unsigned short type) { return unpackTlvUInt32(type); }
  unsigned short UnpackUnsignedShortTLV(unsigned short type) { return unpackTlvUInt16(type); }
  unsigned char UnpackCharTLV(unsigned short type) { return unpackTlvUInt8(type); }

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
