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
  char szQuery[80];
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


class DataRegWizard : public CData
{
public:
  DataRegWizard(unsigned long n = 0) : CData(n)
    {  szOption[0] = '\0'; szPassword1[0] = '\0'; szPassword2[0] = '\0'; szUin[0] = '\0'; nState = 0; }
  char szOption[80];
  char szPassword1[80];
  char szPassword2[80];
  char szUin[10];
  int nState;
};

class DataUserSelect : public CData
{
public:
  DataUserSelect(unsigned long n) : CData(n)
    {  szPassword[0] = '\0'; }
  char szPassword[80];
};

class DataSearch : public CData
{
public:
  DataSearch() : CData(0)
      {  szAlias[0] = szFirstName[0] = szLastName[0] = szEmail[0] =
         szCity[0] = szState[0] = szCoName[0] = szCoDept[0] = szCoPos[0] = '\0';
         nState = nMinAge = nMaxAge = nGender = nLanguage = nCountryCode = 0;
         bOnlineOnly = false; }
  char szAlias[80];
  char szFirstName[80];
  char szLastName[80];
  char szEmail[80];
  unsigned short nMinAge;
  unsigned short nMaxAge;
  char nGender;
  char nLanguage;
  char szCity[80];
  char szState[80];
  unsigned short nCountryCode;
  char szCoName[80];
  char szCoDept[80];
  char szCoPos[80];
  bool bOnlineOnly;

  unsigned short nState;
};


class DataFileChatOffer : public CData
{
public:
  DataFileChatOffer(unsigned long _nSeq, unsigned long _nUin) : CData(_nUin)
    { szReason[0] = '\0'; nSequence = _nSeq; }
  char szReason[256];
  unsigned long nSequence;
};


#endif
