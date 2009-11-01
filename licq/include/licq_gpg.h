
#ifndef _LICQ_GPG_H_INCLUDED_
#define _LICQ_GPG_H_INCLUDED_

#include <list>
#include <pthread.h>
#include <string>

#include "licq_file.h"

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

class CGPGHelper
{
public:
  static const char pgpSig[];
  CGPGHelper();
  ~CGPGHelper();
  char *Decrypt(const char *);
  char *Encrypt(const char *, const char *, unsigned long);
  void Start();

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
  std::list<GpgKey>* getKeyList() const;

protected:
  CIniFile mKeysIni;
};

class CGPGMEMutex
{
public:
  ~CGPGMEMutex();
  CGPGMEMutex();
  bool Lock();

protected:
  static pthread_mutex_t mutex;
};


extern CGPGHelper gGPGHelper;

#endif //_LICQ_GPG_H_INCLUDED_
