#ifndef PROTOPLUGIN_H
#define PROTOPLUGIN_H

class CICQDaemon;

const unsigned long PP_SEND_MSG      = 0x00000001;
const unsigned long PP_SEND_URL      = 0x00000002;
const unsigned long PP_SEND_FILE     = 0x00000004;
const unsigned long PP_SEND_CHAT     = 0x00000008;
const unsigned long PP_SEND_CONTACT  = 0x00000010;
const unsigned long PP_SEND_AUTH     = 0x00000020;
const unsigned long PP_SEND_AUTHxREQ = 0x00000040;
const unsigned long PP_SEND_SMS      = 0x00000080;
const unsigned long PP_SEND_SECURE   = 0x00000100;

#ifdef __cplusplus
extern "C" {
#endif

char *LProto_Name();

char *LProto_Version();

char *LProto_PPID();

bool LProto_Init();

unsigned long LProto_SendFuncs();

int LProto_Main(CICQDaemon *);

/*--------INTERNAL USE ONLY------------*/

extern pthread_cond_t LP_IdSignal;
extern pthread_mutex_t LP_IdMutex;
extern std::list<unsigned short> LP_Ids;
unsigned short LP_Id;

void LProto_Exit(int _nResult)
{
  int *p = (int *)malloc(sizeof(int));
  *p = _nResult;
  pthread_mutex_lock(&LP_IdMutex);
  LP_Ids.push_back(LP_Id);
  pthread_mutex_unlock(&LP_IdMutex);
  pthread_cond_signal(&LP_IdSignal);
  pthread_exit(p);
}

void *LProto_Main_tep(void *p)
{
  LProto_Exit(LProto_Main((CICQDaemon *)p));
  return 0;
}

#ifdef __cplusplus
}
#endif

#endif // PROTOPLUGIN_H
