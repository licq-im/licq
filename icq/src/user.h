/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2012-2013 Licq developers <licq-dev@googlegroups.com>
 *
 * Licq is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Licq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Licq; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef LICQICQ_USER_H
#define LICQICQ_USER_H

#include <licq/icq/user.h>

#include "buffer.h"

namespace LicqIcq
{
class DcSocket;

/**
 * An ICQ protocol contact
 */
class User : public virtual Licq::IcqUser
{
public:
  /// Constructor
  User(const Licq::UserId& id, bool temporary = false);

  /// Destructor
  virtual ~User();

  /// Inherited from Licq::User to save local additions
  virtual void saveLicqInfo();
  virtual void saveUserInfo();
  virtual void savePictureInfo();

  // Inherited from Licq::User to update TLV entry when alias changes
  virtual void setAlias(const std::string& alias);

  // Picture Info
  unsigned buddyIconType() const                { return myBuddyIconType; }
  char buddyIconHashType() const                { return myBuddyIconHashType; }
  const std::string& buddyIconHash() const      { return myBuddyIconHash; }
  const std::string& ourBuddyIconHash() const   { return myOurBuddyIconHash; }
  void setBuddyIconType(unsigned s)     { myBuddyIconType = s; }
  void setBuddyIconHashType(char s)     { myBuddyIconHashType = s; }
  void setBuddyIconHash(const std::string& s) { myBuddyIconHash = s; }
  void setOurBuddyIconHash(const std::string& s) { myOurBuddyIconHash = s; }

  unsigned short GetSID() const                 { return myNormalSid; }
  unsigned short GetInvisibleSID() const        { return myInvisibleSid; }
  unsigned short GetVisibleSID() const          { return myVisibleSid; }
  unsigned short GetGSID() const                { return myGroupSid; }
  void SetSID(unsigned short s)                 { myNormalSid = s; }
  void SetInvisibleSID(unsigned short s)        { myInvisibleSid = s; }
  void SetVisibleSID(unsigned short s)          { myVisibleSid = s; }
  void SetGSID(unsigned short s);

  //!True if they have sent the UTF8 Cap
  bool SupportsUTF8() const                     { return mySupportsUtf8; }
  void SetSupportsUTF8(bool b)                  { mySupportsUtf8 = b; }

  unsigned short SendLevel() const              { return mySendLevel; }
  void SetSendLevel(unsigned short s)           { mySendLevel = s; }

  unsigned short Sequence(bool = false);
  void setDirectMode(bool direct)               { myDirectMode = direct; }

  bool directMode() const                       { return myDirectMode; }

  /// Overridden to check for existing connections
  bool canSendDirect() const;

  unsigned long ClientTimestamp() const         { return myClientTimestamp; }
  unsigned long OurClientTimestamp() const      { return myOurClientTimestamp; }
  unsigned long ClientInfoTimestamp() const     { return myClientInfoTimestamp; }
  unsigned long OurClientInfoTimestamp() const  { return myOurClientInfoTimestamp; }
  unsigned long ClientStatusTimestamp() const   { return myClientStatusTimestamp; }
  unsigned long OurClientStatusTimestamp() const { return myOurClientStatusTimestamp; }
  void SetClientTimestamp(unsigned long s) { myClientTimestamp = s; }
  void SetOurClientTimestamp(unsigned long s) { myOurClientTimestamp = s; }
  void SetClientInfoTimestamp(unsigned long s) { myClientInfoTimestamp = s; }
  void SetOurClientInfoTimestamp(unsigned long s) { myOurClientInfoTimestamp = s; }
  void SetClientStatusTimestamp(unsigned long s) { myClientStatusTimestamp = s; }
  void SetOurClientStatusTimestamp(unsigned long s) { myOurClientStatusTimestamp = s; }

  void setPhoneFollowMeStatus(unsigned n)       { myPhoneFollowMeStatus = n; save(SaveLicqInfo); }
  void setIcqPhoneStatus(unsigned n)            { myIcqPhoneStatus = n; }
  void setSharedFilesStatus(unsigned n)         { mySharedFilesStatus = n; }

  // Convert between ICQ timezones and minute based
  char GetTimezone() const { return myTimezone==TimezoneUnknown ? -100 : myTimezone/-1800; }
  void SetTimezone(char tz) { setTimezone(tz==-100 ? TimezoneUnknown : ((int)tz)*-1800); }

  enum DirectFlags
  {
    DirectDisabled      = 0,    // Direct contact not possible
    DirectAnyone        = 1,    // Direct contact with anyone
    DirectListed        = 2,    // Direct contact only with contacts in list
    DirectAuth          = 3,    // Direct contact only with authorized contacts
  };
  unsigned directFlag() const                   { return myDirectFlag; }
  void setDirectFlag(unsigned n)                { myDirectFlag = n; }

  unsigned long Cookie() const                  { return myCookie; }
  void SetCookie(unsigned long cookie)          { myCookie = cookie; }

  // User TLV List handling
  void AddTLV(TlvPtr);
  void RemoveTLV(unsigned long);
  void SetTLVList(TlvList& tlvs);
  TlvList GetTLVList()                          { return myTLVs; }
  const TlvList GetTLVList() const              { return myTLVs; }

  void SetIpPort(unsigned long nIp, unsigned short nPort);
  std::string internalIpToString() const;

  int socketDesc(int channel) const;
  void setSocketDesc(DcSocket* s);
  void clearSocketDesc(Licq::INetSocket* s);
  void clearAllSocketDesc() { clearSocketDesc(NULL); }
  int normalSocketDesc() const { return myNormalSocketDesc; }
  int infoSocketDesc() const { return myInfoSocketDesc; }

  unsigned long Version() const { return myVersion; }
  unsigned short ConnectionVersion() const;
  void SetVersion(unsigned long s) { myVersion = s; }
  void SetConnectionVersion(unsigned short s) { myConnectionVersion = s; }

private:
  /**
   * Save a category list
   *
   * @param category The category map to save
   * @param key Base name of key in file for entries
   */
  void saveCategory(const Licq::UserCategoryMap& category, const std::string& key);

  /**
   * Load a category list
   *
   * @param category The category map to save
   * @param key Base name of key in file for entries
   */
  void loadCategory(Licq::UserCategoryMap& category, const std::string& key);

  unsigned short mySequence;
  unsigned long myCookie;
  unsigned long myClientTimestamp;
  unsigned long myClientInfoTimestamp;
  unsigned long myClientStatusTimestamp;
  unsigned long myOurClientTimestamp;
  unsigned long myOurClientInfoTimestamp;
  unsigned long myOurClientStatusTimestamp;

  unsigned myDirectFlag;
  bool myDirectMode;
  bool mySupportsUtf8;
  unsigned short mySendLevel;

  unsigned myBuddyIconType;
  unsigned myBuddyIconHashType;
  std::string myBuddyIconHash;
  std::string myOurBuddyIconHash;

  // Server Side ID, Group SID
  unsigned myNormalSid;
  unsigned myInvisibleSid;
  unsigned myVisibleSid;
  unsigned myGroupSid;

  // Extra TLVs attached to this user's SSI info
  // We use a map to allow fast access to the TLV by type, even though the
  // actual type is in SOscarTLV as well. Which should make it obvious
  // that the TLV handling should be fixed in licq_buffer.h/buffer.cpp
  TlvList myTLVs;

  // Socket descriptors for direct connections
  int myNormalSocketDesc;
  int myInfoSocketDesc;
  int myStatusSocketDesc;

  unsigned long myVersion;
  unsigned myConnectionVersion;
};


/**
 * Read mutex guard for LicqIcq::User
 */
class UserReadGuard : public Licq::UserReadGuard
{
public:
  // Derived costructors
  UserReadGuard(const Licq::UserId& userId, bool addUser = false, bool* retWasAdded = NULL)
    : Licq::UserReadGuard(userId, addUser, retWasAdded)
  { }
  UserReadGuard(const User* user, bool locked = false)
    : Licq::UserReadGuard(user, locked)
  { }
  UserReadGuard(UserReadGuard* guard)
    : Licq::UserReadGuard(guard)
  { }

  // Access operators
  const User* operator*() const
  { return dynamic_cast<const User*>(Licq::UserReadGuard::operator*()); }
  const User* operator->() const
  { return dynamic_cast<const User*>(Licq::UserReadGuard::operator->()); }
};

/**
 * Write mutex guard for LicqIcq::User
 */
class UserWriteGuard : public Licq::UserWriteGuard
{
public:
  // Derived costructors
  UserWriteGuard(const Licq::UserId& userId, bool addUser = false, bool* retWasAdded = NULL)
    : Licq::UserWriteGuard(userId, addUser, retWasAdded)
  { }
  UserWriteGuard(User* user, bool locked = false)
    : Licq::UserWriteGuard(user, locked)
  { }
  UserWriteGuard(UserWriteGuard* guard)
    : Licq::UserWriteGuard(guard)
  { }

  // Access operators
  User* operator*() const
  { return dynamic_cast<User*>(Licq::UserWriteGuard::operator*()); }
  User* operator->() const
  { return dynamic_cast<User*>(Licq::UserWriteGuard::operator->()); }
};

} // namespace LicqIcq

#endif
