#ifndef LICQ_CONTACTLIST_OWNER_H
#define LICQ_CONTACTLIST_OWNER_H

#include "user.h"

namespace Licq
{

/**
 * A protocol account including all user information for that account
 *
 * Inherits LicqUser to hold all user information associated with the account.
 */
class Owner : public User
{
public:
  /**
   * Constructor
   *
   * @param accountId User account id
   * @param ppid Protocol instance id
   */
  Owner(const std::string& accountId, unsigned long ppid);

  virtual ~Owner();
  bool Exception() const                        { return m_bException; }

  // Owner specific functions
  const char* Password() const                  { return(m_szPassword); }
  void SetPassword(const char *s) { SetString(&m_szPassword, s); SaveLicqInfo(); }
  void SetWebAware(bool b)     {  m_bWebAware = b; SaveLicqInfo(); }
  virtual void SetWebAwareStatus(char c) { SetWebAware(c); }
  void SetHideIp(bool b)       {  m_bHideIp = b; SaveLicqInfo(); }
  void SetSavePassword(bool b) {  m_bSavePassword = b; SaveLicqInfo(); }
  void SetRandomChatGroup(unsigned long n)  { m_nRandomChatGroup = n; SaveLicqInfo(); }
  bool WebAware() const                         { return m_bWebAware; }
  bool HideIp() const                           { return m_bHideIp; }
  bool SavePassword() const                     { return m_bSavePassword; }
  unsigned long RandomChatGroup() const         { return m_nRandomChatGroup; }
  unsigned long AddStatusFlags(unsigned long nStatus) const;

  // Server Side List functions
  time_t GetSSTime() const                      { return m_nSSTime; }
  void SetSSTime(time_t t)            { m_nSSTime = t; }
  unsigned short GetSSCount() const             { return m_nSSCount; }
  void SetSSCount(unsigned short n)   { m_nSSCount = n; }
  unsigned short GetPDINFO() const              { return m_nPDINFO; }
  void SetPDINFO(unsigned short n)    { m_nPDINFO = n; SaveLicqInfo(); }

  void SetPicture(const char *f);

  // Virtual overloaded functions
  virtual void SaveLicqInfo();
  virtual void SetStatusOffline();

  virtual bool isUser() const                   { return false; }
protected:
  char *m_szPassword;
  bool m_bException,
       m_bWebAware,
       m_bHideIp,
       m_bSavePassword;
  unsigned long  m_nRandomChatGroup;
  unsigned short m_nSSCount;
  time_t m_nSSTime;
  unsigned short m_nPDINFO;
};

} // namespace Licq

#endif // LICQ_CONTACTLIST_OWNER_H
