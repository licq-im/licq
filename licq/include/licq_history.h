#ifndef USERHISTORY_H
#define USERHISTORY_H

#include <list>

class CUserEvent;

typedef std::list<CUserEvent *> HistoryList;
typedef std::list<CUserEvent *>::iterator HistoryListIter;

class CUserHistory
{
public:
  CUserHistory();
  ~CUserHistory();
  void SetFile(const char *, unsigned long);
  void Append(const char *);
  bool Load(HistoryList &);
  static void Clear(HistoryList &);
  void Save(const char *);
  const char *Description()  { return m_szDescription; }
  const char *FileName()     { return m_szFileName; }
protected:
  char *m_szFileName;
  char *m_szDescription;
};

#endif



