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
  virtual ~CMSNBuffer() { }
  CMSNBuffer(CMSNBuffer &);
  CMSNBuffer(CBuffer &);
  
  bool ParseHeaders();
  std::string GetValue(std::string);
  bool HasHeader(std::string);
  
  void SkipParameter();
  std::string GetParameter();
  void SkipPacket();
  
private:
  std::list<SHeader *> m_lHeader;  
};

#endif // __MSNBUFFER_H

