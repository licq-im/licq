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
class Owner : public virtual User
{
public:
  bool Exception() const                        { return m_bException; }

  // Owner specific functions
  const std::string& password() const           { return myPassword; }
  void setPassword(const std::string& s)        { myPassword = s; SaveLicqInfo(); }
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
  unsigned short GetSSCount() const             { return mySsCount; }
  void SetSSCount(unsigned short n)             { mySsCount = n; }
  unsigned short GetPDINFO() const              { return myPDINFO; }
  void SetPDINFO(unsigned short n)              { myPDINFO = n; SaveLicqInfo(); }

  void SetPicture(const char *f);

  /**
   * Get path for file with owner picture
   * Note: This function will not check if file actually (should) exist
   *
   * @return Path to user picture
   */
  std::string pictureFileName() const;

  // Virtual overloaded functions
  virtual void SaveLicqInfo() = 0;
  virtual void SetStatusOffline();

  virtual bool isUser() const                   { return false; }
protected:
  virtual ~Owner() { /* Empty */ }

  std::string myPassword;
  bool m_bException,
       m_bWebAware,
       m_bHideIp,
       m_bSavePassword;
  unsigned long  m_nRandomChatGroup;
  unsigned mySsCount;
  time_t m_nSSTime;
  unsigned myPDINFO;
};

/**
 * Read mutex guard for Licq::Owner
 */
class OwnerReadGuard : public ReadMutexGuard<Owner>
{
public:
  /**
   * Constructor, will fetch and lock an owner based on user id
   * Note: Always check that the owner was actually fetched before using
   *
   * @param userId Id of owner to fetch
   */
  OwnerReadGuard(const UserId& userId);

  /**
   * Constructor, will fetch and lock an owner based on protocolId
   * Note: Always check that the owner was actually fetched before using
   *
   * @param userId Id of owner to fetch
   */
  OwnerReadGuard(unsigned long protocolId);

  // Derived constructors
  OwnerReadGuard(const Owner* owner, bool locked = false)
    : ReadMutexGuard<Owner>(owner, locked)
  { }
  OwnerReadGuard(ReadMutexGuard<Owner>* guard)
    : ReadMutexGuard<Owner>(guard)
  { }
};

/**
 * Write mutex guard for Licq::Owner
 */
class OwnerWriteGuard : public WriteMutexGuard<Owner>
{
public:
  /**
   * Constructor, will fetch and lock an owner based on user id
   * Note: Always check that the owner was actually fetched before using
   *
   * @param userId Id of owner to fetch
   */
  OwnerWriteGuard(const UserId& userId);

  /**
   * Constructor, will fetch and lock an owner based on protocolId
   * Note: Always check that the owner was actually fetched before using
   *
   * @param userId Id of owner to fetch
   */
  OwnerWriteGuard(unsigned long protocolId);

  // Derived constructors
  OwnerWriteGuard(Owner* owner, bool locked = false)
    : WriteMutexGuard<Owner>(owner, locked)
  { }
  OwnerWriteGuard(WriteMutexGuard<Owner>* guard)
    : WriteMutexGuard<Owner>(guard)
  { }
};

} // namespace Licq

#endif // LICQ_CONTACTLIST_OWNER_H
