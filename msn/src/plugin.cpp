#include "licq_icqd.h"
#include "licq_user.h"
#include "licq_protoplugin.h"

#include "msn.h"

char *LProto_Name()
{
  static char szName[] = "MSN";
  return szName;
}

char *LProto_Version()
{
  static char szVersion[] = "0.1";
  return szVersion;
}

const char *LProto_Description()
{
  static char szDesc[] = "MSN Protocol Plugin";
  return szDesc;
}

unsigned long LProto_Capabilities()
{
  return 0;
}

void LProto_Main(CICQDaemon *_pDaemon)
{
  int nPipe = _pDaemon->RegisterProtoPlugin();  

  CMSN *pMSN = new CMSN(_pDaemon, nPipe);
  pMSN->Run();
}

char *LProto_Id()
{
  static char szId[] = "MSN_";
  return szId;
}

bool LProto_Init()
{
  return true;
}
