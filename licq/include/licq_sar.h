#ifndef SAR_H
#define SAR_H

#include <vector>
#include <pthread.h>

#include "licq_file.h"

#define SAR_AWAY      0
#define SAR_NA        1
#define SAR_OCCUPIED  2
#define SAR_DND       3
#define SAR_FFC       4

#define SAR_NUM_SECTIONS 5

#define SAR_SECTIONS { "Away", "NA", "Occupied", "DND", "FFC" }

/*---------------------------------------------------------------------------
 * class CSavedAutoResponse
 *-------------------------------------------------------------------------*/
class CSavedAutoResponse
{
public:
  CSavedAutoResponse(const char *_szName, const char *_szAutoResponse);
  ~CSavedAutoResponse();
  const char *Name() { return m_szName; }
  const char *AutoResponse()  { return m_szAutoResponse; }
protected:
  char *m_szName;
  char *m_szAutoResponse;
};

typedef std::vector<CSavedAutoResponse *> SARList;
typedef std::vector<CSavedAutoResponse *>::iterator SARListIter;


/*---------------------------------------------------------------------------
 * class CSARManager
 *-------------------------------------------------------------------------*/
class CSARManager
{
public:
  CSARManager();
  ~CSARManager();
  bool Load();
  void Save();
  SARList &Fetch(unsigned short n);
  void Drop();
protected:
  CIniFile m_fConf;
  pthread_mutex_t mutex;
  SARList m_lSAR[SAR_NUM_SECTIONS];
};

extern CSARManager gSARManager;

#endif
