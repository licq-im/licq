#ifndef EVENT_DATA_H
#define EVENT_DATA_H

// Data structures for input routines
class CData
{
public:
  CData (const char *i, unsigned long n)
    { szId = i; nPPID = n, nPos = 0; szQuery[0] = '\0'; }
  const char *szId;
  unsigned long nPPID;
  unsigned short nPos;
  char szQuery[80];
};


class DataMsg : public CData
{
public:
  DataMsg(const char *i, unsigned long n) : CData(i, n)
    { szMsg[0] = '\0'; bUrgent = false; bServer = false; }
  char szMsg[1024];
  bool bUrgent;
  bool bServer;
};


class DataSendFile : public CData
{
public:
  DataSendFile(const char *i, unsigned long n) : CData(i, n)
    { szFileName[0] = '\0'; szDescription[0] = '\0'; bUrgent = false; }
  char szFileName[512];
  char szDescription[512];
  bool bUrgent;
};


class DataAutoResponse : public CData
{
public:
  DataAutoResponse() : CData(NULL, 0)
    { szRsp[0] = '\0'; }
  char szRsp[1024];
};


class DataUrl : public CData
{
public:
  DataUrl(const char *i, unsigned long n) : CData(i, n)
    { szUrl[0] = '\0'; szDesc[0] = '\0'; bUrgent = false; bServer = false; }
  char szUrl[1024];
  char szDesc[1024];
  bool bUrgent;
  bool bServer;
};

class DataSms : public CData
{
public:
  DataSms(const char *i, unsigned long n) : CData(i, n)
    { szMsg[0] = '\0'; }
  char szMsg[1024];
};
class DataRegWizard : public CData
{
public:
  DataRegWizard(const char *i = NULL, unsigned long n = 0) : CData(i, n)
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
  DataUserSelect(const char *i, unsigned long n) : CData(i, n)
    {  szPassword[0] = '\0'; }
  char szPassword[80];
};

class DataSearch : public CData
{
public:
  DataSearch() : CData(NULL, 0)
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
  DataFileChatOffer(CEventFile *_f, const char *i, unsigned long n) : CData(i, n)
    { szReason[0] = '\0'; f = _f; }
  CEventFile *f;
  char szReason[256];
};


#endif
