/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2012 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQICQ_BUFFER_H
#define LICQICQ_BUFFER_H

#include <licq/buffer.h>

#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>
#include <map>

namespace LicqIcq
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

class Buffer : public Licq::Buffer
{
public:
  Buffer()
    : Licq::Buffer()
  { }
  Buffer(unsigned long n)
    : Licq::Buffer(n)
  { }
  Buffer(const Licq::Buffer& b);

  /// Destructor
  virtual ~Buffer()
  { }

  virtual Licq::Buffer& operator=(const Licq::Buffer& b);
  virtual void Clear();

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
  unsigned long UnpackUnsignedLongTLV(unsigned short type)
  { return unpackTlvUInt32(type); }
  unsigned short UnpackUnsignedShortTLV(unsigned short type)
  { return unpackTlvUInt16(type); }
  unsigned char UnpackCharTLV(unsigned short type)
  { return unpackTlvUInt8(type); }

private:
  TlvList myTLVs;
};

} // namespace LicqIcq

#endif
