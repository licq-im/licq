#ifndef USERHISTORY_H
#define USERHISTORY_H

#include <list.h>

class CEventHistory;

typedef list<CEventHistory *> HistoryList;
typedef list<CEventHistory *>::iterator HistoryListIter;

class CUserHistory
{
public:
  CUserHistory(void);
  ~CUserHistory(void);
  void SetFile(const char *, unsigned long);
  void Append(const char *);
  bool Load(HistoryList &);
  void Clear(void);
  void Save(const char *);
  const char *Description(void)  { return m_szDescription; }
  const char *FileName(void)     { return m_szFileName; }
protected:
  char *m_szFileName;
  char *m_szDescription;
};

#endif



