#ifndef PROTOPLUGIN_H
#define PROTOPLUGIN_H

class CICQDaemon;

#ifdef __cplusplus
extern "C" {
#endif

char *LProto_Name();

char *LProto_Version();

char *LProto_Id();

bool LProto_Init();

void LProto_Main(CICQDaemon *);

void *LProto_Main_tep(void *p)
{
  LProto_Main((CICQDaemon *)p);
  return 0;
}

#ifdef __cplusplus
}
#endif

#endif // PROTOPLUGIN_H
