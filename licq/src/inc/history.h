#ifndef USERHISTORY_H
#define USERHISTORY_H

class CUserHistory
{
public:
  CUserHistory(void);
  ~CUserHistory(void);
  void SetFile(const char *, unsigned long);
  void Append(const char *);
  void Load(char *&);
  void Save(const char *);
  const char *Description(void)  { return m_szDescription; }
  const char *FileName(void)     { return m_szFileName; }  
protected:
  char *m_szFileName;
  char *m_szDescription;
};

#endif



