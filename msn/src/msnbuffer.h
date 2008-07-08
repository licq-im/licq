/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// written by Jon Keating <jon@licq.org>

#ifndef __MSNBUFFER_H
#define __MSNBUFFER_H

#include "licq_buffer.h"

#include <string>
#include <list>

struct SHeader
{
  std::string strHeader;
  std::string strValue;
};


class CMSNBuffer : public CBuffer
{
public:
  CMSNBuffer() : CBuffer() { }
  CMSNBuffer(unsigned long n) : CBuffer(n) { }
  virtual ~CMSNBuffer() { ClearHeaders(); }
  CMSNBuffer(CMSNBuffer &);
  CMSNBuffer(CBuffer &);

  bool ParseHeaders();
  std::string GetValue(const std::string& key);
  bool HasHeader(const std::string& key);
  void ClearHeaders();

  void SkipParameter();
  void SkipRN();
  std::string GetParameter();
  unsigned short GetParameterUnsignedShort();
  unsigned long GetParameterUnsignedLong();
  void SkipPacket();
  void Skip(unsigned long);

private:
  std::list<SHeader*> m_lHeader;
};

#endif // __MSNBUFFER_H

