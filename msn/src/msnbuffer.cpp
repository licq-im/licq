/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2004-2010 Licq developers
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

#include <cstdlib>
#include <cstring>
#include "msnbuffer.h"

using namespace std;

CMSNBuffer::CMSNBuffer(CMSNBuffer &b)
  : Licq::Buffer(b)
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

CMSNBuffer::CMSNBuffer(Licq::Buffer& b)
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
  int counter = 0;
  string stmp = "", strHeader, strData;
  struct SHeader *pHeader = 0;
  
  if (m_lHeader.size() > 0)
    ClearHeaders();

  while (!End())
  {
    *this >> ctmp;
   
    // Get header
    while (ctmp != ':' && ctmp != '\r' && ctmp != 0)
    {
      stmp += ctmp;
      *this >> ctmp;
    }
    
    // Check for the end of the headers list
    if (ctmp == '\r')
    {
      while (ctmp == '\r' || ctmp == '\n')
      {
        counter++;
        *this >> ctmp;
        
        if (counter == 2)
        {
          setDataPosRead(getDataPosRead() - 1);
          return true;
        }
      }
      
      counter = 0;
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

string CMSNBuffer::GetValue(const string& strKey)
{
  string strReturn = "";
  list<SHeader *>::iterator it;
  for (it = m_lHeader.begin(); it != m_lHeader.end(); it++)
  {
    if ((*it)->strHeader == strKey)
    {
      strReturn = (*it)->strValue;
    }
  }
  
  return strReturn;
}

bool CMSNBuffer::HasHeader(const string& strKey)
{
  list<SHeader *>::iterator it;
  for (it = m_lHeader.begin(); it != m_lHeader.end(); it++)
  {
    if ((*it)->strHeader == strKey)
    {
      return true;
    }
  }
  
  return false;
}

void CMSNBuffer::ClearHeaders()
{
  list<SHeader *>::iterator it;
  for (it = m_lHeader.begin(); it != m_lHeader.end(); ++it)
  {
    if (*it)
    { 
      delete *it;
     *it = 0;
    }
  }
  m_lHeader.clear();
}

void CMSNBuffer::SkipParameter()
{
  char cCheck;
  *this >> cCheck;
  
  if (isspace(cCheck))
  {
    // Leading space to next paramater
    while (isspace(cCheck) && !End())
      *this >> cCheck;
  }
  
  // Now skip the paramater
  while (!isspace(cCheck) && !End())
    *this>> cCheck;
}

void CMSNBuffer::SkipRN()
{
  char cCheck;
  *this >> cCheck;
  
  while ((cCheck == '\r' || cCheck == '\n') && !End())
    *this >> cCheck;

  setDataPosRead(getDataPosRead() - 1);
}

string CMSNBuffer::GetParameter()
{
  char cCheck;
  string strParam;
  *this >> cCheck;
  
  if (cCheck == ' ')
  {
    while (cCheck == ' ' && !End())
      *this >> cCheck;   
  }
  
  setDataPosRead(getDataPosRead() - 1);
  
  while (cCheck != ' ' && cCheck != '\r' && !End())
  {
    *this >> cCheck;
    if (cCheck != ' ' && cCheck != '\r' && cCheck != '\n')
      strParam += cCheck;
  }
  
  return strParam;
}

unsigned short CMSNBuffer::GetParameterUnsignedShort()
{
  string strValue = GetParameter();
  unsigned short nValue = (unsigned short)atoi(strValue.c_str());
  return nValue;
}

unsigned long CMSNBuffer::GetParameterUnsignedLong()
{
  string strValue = GetParameter();
  unsigned long nValue = strtoul(strValue.c_str(), (char **)NULL, 10);
  return nValue;
}
void CMSNBuffer::SkipPacket()
{
  char cCheck = 0;
  
  while (cCheck != '\n' && !End())
    *this >> cCheck;
}

void CMSNBuffer::Skip(unsigned long _nSize)
{
  incDataPosRead(_nSize);
}

