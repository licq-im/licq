#ifndef SAR_H
#define SAR_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <vector.h>
#include "file.h"

#define SAR_AWAY      0
#define SAR_NA        1
#define SAR_OCCUPIED  2
#define SAR_DND       3
#define SAR_FFF       4

#define SAR_NUM_SECTIONS 5

#define SAR_SECTIONS { "Away", "NA", "Occupied", "DND", "FFC" }

/*---------------------------------------------------------------------------
 * class CSavedAutoResponse
 *-------------------------------------------------------------------------*/
class CSavedAutoResponse
{
public:
  CSavedAutoResponse(const char *_szName, const char *_szAutoResponse);
  ~CSavedAutoResponse(void);
  const char *Name(void) { return m_szName; }
  const char *AutoResponse(void)  { return m_szAutoResponse; }
protected:
  char *m_szName;
  char *m_szAutoResponse;
};

typedef vector<CSavedAutoResponse *> SARList;
typedef vector<CSavedAutoResponse *>::iterator SARListIter;


/*---------------------------------------------------------------------------
 * class CSARManager
 *-------------------------------------------------------------------------*/
class CSARManager
{
public:
  CSARManager(void);
  ~CSARManager(void);
  bool Load(void);
  void Save(void);
  SARList &SavedAutoResponses(unsigned short n);
protected:
  CIniFile m_fConf;
  SARList m_lSAR[SAR_NUM_SECTIONS];
};

extern CSARManager gSARManager;

#endif
