#ifndef EVENT_DATA_H
#define EVENT_DATA_H

// Data structures for input routines
class CData
{
public:
  CData (unsigned long n)
    { nUin = n; nPos = 0; szQuery[0] = '\0'; }
  unsigned long nUin;
  unsigned short nPos;
  char szQuery[32];
};


class DataMsg : public CData
{
public:
  DataMsg(unsigned long n) : CData(n)
    { szMsg[0] = '\0'; bUrgent = false; bServer = false; }
  char szMsg[1024];
  bool bUrgent;
  bool bServer;
};


class DataSendFile : public CData
{
public:
  DataSendFile(unsigned long n) : CData(n)
    { szFileName[0] = '\0'; szDescription[0] = '\0'; bUrgent = false; }
  char szFileName[512];
  char szDescription[512];
  bool bUrgent;
};


class DataAutoResponse : public CData
{
public:
  DataAutoResponse() : CData(0)
    { szRsp[0] = '\0'; }
  char szRsp[1024];
};


class DataUrl : public CData
{
public:
  DataUrl(unsigned long n) : CData(n)
    { szUrl[0] = '\0'; szDesc[0] = '\0'; bUrgent = false; bServer = false; }
  char szUrl[1024];
  char szDesc[1024];
  bool bUrgent;
  bool bServer;
};


#endif
