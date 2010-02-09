#ifndef LICQ_GPGHELPER_H
#define LICQ_GPGHELPER_H

#include <boost/noncopyable.hpp>
#include <list>
#include <string>

#include "types.h"

namespace Licq
{

// Structure for holding a user identity for a key
struct GpgUid
{
  std::string name;
  std::string email;
};

// Structure for holding information about a key
struct GpgKey
{
  // First uid in list is the primary uid
  std::list<GpgUid> uids;
  std::string keyid;
};

class GpgHelper : private boost::noncopyable
{
public:
  static const char pgpSig[];
  virtual char* Decrypt(const char* cipher) = 0;
  virtual char* Encrypt(const char* plain, const Licq::UserId& userId) = 0;

  /**
   * Get a list of keys
   * This function allows plugins to present a list of keys for user to select
   * from without making the plugin itself dependant on gpgme.
   *
   * This function only fetches the bare minimum information currently used by
   * the plugins, extended it and the structs above if additional fields are
   * needed.
   *
   * @return A list of keys, must be deleted by caller
   */
  virtual std::list<GpgKey>*  getKeyList() const = 0;

protected:
  virtual ~GpgHelper()
  { /* Empty */ }
};

extern GpgHelper& gGpgHelper;

} // namespace Licq

#endif
