#ifndef EVENT_DATA_H
#define EVENT_DATA_H

// Data structures for input routines
class CData
{
public:
  unsigned long nUin;
  unsigned short nPos;
};


class DataMsg : public CData
{
public:
  DataMsg(unsigned long n)
    { nUin = n; nPos = 0; szMsg[0] = '\0'; bUrgent = false; bServer = false; }
  char szMsg[1024];
  bool bUrgent;
  bool bServer;
};


class DataUrl : public CData
{
public:
  DataUrl(unsigned long n)
    { nUin = n; nPos = 0; szUrl[0] = '\0'; szDesc[0] = '\0'; bUrgent = false; bServer = false; }
  char szUrl[1024];
  char szDesc[1024];
  bool bUrgent;
  bool bServer;
};


#endif
