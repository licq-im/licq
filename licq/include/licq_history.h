#ifndef USERHISTORY_H
#define USERHISTORY_H

#include <list>

class CUserEvent;

typedef std::list<CUserEvent *> HistoryList;
typedef std::list<CUserEvent *>::iterator HistoryListIter;
typedef std::list<CUserEvent *>::reverse_iterator HistoryListRIter;

class CUserHistory
{
public:
  CUserHistory();
  ~CUserHistory();
  void SetFile(const char *, unsigned long);
  void SetFile(const char *, const char *, unsigned long);
  void Append(const char* buf) { Write(buf, true); }
  bool Load(HistoryList& history) const;
  static void Clear(HistoryList &);
  void Save(const char* buf) { Write(buf, false); }
  const char* Description() const       { return m_szDescription; }
  const char* FileName() const          { return m_szFileName; }

  /**
   * Write to the history file, creating it if necessary
   *
   * @param buf String with data to write
   * @param append True to append data or false to overwrite file
   */
  void Write(const char* buf, bool append);

protected:
  char *m_szFileName;
  char *m_szDescription;
};

#endif



