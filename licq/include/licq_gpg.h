
#ifndef _LICQ_GPG_H_INCLUDED_
#define _LICQ_GPG_H_INCLUDED_

#ifdef HAVE_LIBGPGME
#include <gpgme.h>
#endif
#include <pthread.h>

class CGPGHelper
{
public:
  static const char pgpSig[];
  CGPGHelper();
  ~CGPGHelper();
  char *Decrypt(const char *);
  char *Encrypt(const char *, const char *, unsigned long);
  void Start();

protected:
  CIniFile mKeysIni;
#ifdef HAVE_LIBGPGME
  GpgmeCtx mCtx;
#endif
  char *mGPGPassphrase;

  void gpgmeLock();
  void gpgmeUnlock();
  static const char *PassphraseCallback(void *, const char *, void **);
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
