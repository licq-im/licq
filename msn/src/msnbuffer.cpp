#include "msnbuffer.h"

CMSNBuffer::CMSNBuffer(CMSNBuffer &b)
{
  m_nDataSize = b.getDataMaxSize();
  if (m_nDataSize)
  {
    m_pDataStart = new char[m_nDataSize];
    memcpy(m_pDataStart, b.getDataStart(), m_nDataSize);
  }
  else
  {
    m_pDataStart = NULL;
  }
  m_pDataPosRead = m_pDataStart + (b.getDataPosRead() - b.getDataStart());
  m_pDataPosWrite = m_pDataStart + (b.getDataPosWrite() - b.getDataStart());
}

CMSNBuffer::CMSNBuffer(CBuffer &b)
{
  m_nDataSize = b.getDataMaxSize();
  if (m_nDataSize)
  {
    m_pDataStart = new char[m_nDataSize];
    memcpy(m_pDataStart, b.getDataStart(), m_nDataSize);
  }
  else
  {
    m_pDataStart = NULL;
  }
  m_pDataPosRead = m_pDataStart + (b.getDataPosRead() - b.getDataStart());
  m_pDataPosWrite = m_pDataStart + (b.getDataPosWrite() - b.getDataStart());
}

bool CMSNBuffer::ParseHeaders()
{
  char ctmp = 0;
  std::string stmp = "", strHeader, strData;
  if (m_lHeader.size() > 0) return false;
  
  struct SHeader *pHeader = 0;
  
  while (!End())
  {
    *this >> ctmp;
    
    // Get header
    while (ctmp != ':' && ctmp != 0)
    {
      stmp += ctmp;
      *this >> ctmp;
    }
    
    *this >> ctmp; // skip ':'
    
    strHeader = stmp;
    
    // Skip whitespace
    while (ctmp == ' ' && ctmp != 0) *this >> ctmp;
    
    stmp = "";
    
    // Get data
    while (ctmp != '\r' && ctmp != 0)
    {
      stmp += ctmp;
      *this >> ctmp;
    }
    
    // Finish the \n
    *this >> ctmp;
    
    strData = stmp;
    pHeader = new SHeader;
    
    if (!pHeader) return false;
    pHeader->strHeader = strHeader;
    pHeader->strValue = strData;
    m_lHeader.push_back(pHeader);
    
    stmp = "";
  }
  
  return true;
}

std::string CMSNBuffer::GetValue(std::string strKey)
{
  std::string strReturn = "";
  std::list<SHeader *>::iterator it;
  for (it = m_lHeader.begin(); it != m_lHeader.end(); it++)
  {
    if ((*it)->strHeader == strKey)
    {
      strReturn = (*it)->strValue;
    }
  }
  
  return strReturn;
}

bool CMSNBuffer::HasHeader(std::string strKey)
{
  std::list<SHeader *>::iterator it;
  for (it = m_lHeader.begin(); it != m_lHeader.end(); it++)
  {
    if ((*it)->strHeader == strKey)
    {
      return true;
    }
  }
  
  return false;
}

void CMSNBuffer::SkipParameter()
{
  char cCheck;
  *this >> cCheck;
  
  if (cCheck == ' ')
  {
    // Leading space to next paramater
    while (cCheck == ' ' && !End())
      *this >> cCheck;
  }
  
  // Now skip the paramater
  while (cCheck != ' ' && !End())
    *this>> cCheck;
}

std::string CMSNBuffer::GetParameter()
{
  char cCheck;
  std::string strParam;
  *this >> cCheck;
  
  if (cCheck == ' ')
  {
    while (cCheck == ' ' && !End())
      *this >> cCheck;   
  }
  
  setDataPosRead(getDataPosRead() - 1);
  
  while (cCheck != ' ' && !End())
  {
    *this >> cCheck;
    if (cCheck != ' ' && cCheck != '\r' && cCheck != '\n')
      strParam += cCheck;
  }
  
  return strParam;
}

void CMSNBuffer::SkipPacket()
{
  char cCheck = 0;
  
  while (cCheck != '\n' && !End())
    *this >> cCheck;
}
