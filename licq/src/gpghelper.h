#ifndef LICQDAEMON_GPGHELPER_H
#define LICQDAEMON_GPGHELPER_H

#include <licq/gpghelper.h>

#include "config.h"

#ifdef HAVE_LIBGPGME
#include <gpgme.h>
#endif

#include <licq/inifile.h>
#include <licq/thread/mutex.h>

namespace LicqDaemon
{

class GpgHelper : public Licq::GpgHelper
{
public:
  GpgHelper();
  ~GpgHelper();
  void Start();

  // From Licq::GpgHelper
  char* Decrypt(const char* cipher);
  char* Encrypt(const char* plain, const Licq::UserId& userId);
  std::list<Licq::GpgKey>* getKeyList() const;

private:
#ifdef HAVE_LIBGPGME
  static gpgme_error_t PassphraseCallback(void* helperPtr, const char *, const char*, int, int);

  gpgme_ctx_t mCtx;
#endif
  std::string myGpgPassphrase;
  Licq::IniFile myKeysIni;
  mutable Licq::Mutex myMutex;
};

extern GpgHelper gGpgHelper;

} // namespace LicqDaemon

#endif
