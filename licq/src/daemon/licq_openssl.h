#ifndef __L_OPENSSL_H__
#define __L_OPENSSL_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//#define USE_OPENSSL

#ifdef USE_OPENSSL

#include <openssl/bn.h>

class CBuffer;

// Cypto Status
enum CRYPTO_STATUS
{
  CRYPTO_NONE,  // no crypto
  CRYPTO_HALF,  // p, a, g set
  CRYPTO_FULL   // gab set, ready to go
};


class CDHKey
{
public:
  CDHKey();
  void SetP(BIGNUM *p)     { BN_copy (m_p, p); }
  void SetA(BIGNUM *a)     { BN_copy (m_a, a); }
  void SetG(BIGNUM *g)     { BN_copy (m_g, g); }
  void SetGAB(BIGNUM *gab) { BN_copy (m_gab, gab); }
  void SetCryptoStatus (CRYPTO_STATUS s) { m_nCryptoStatus = s; }
  CRYPTO_STATUS CryptoStatus () { return (m_nCryptoStatus); }
  BIGNUM *P() { return (m_p); }
  BIGNUM *A() { return (m_a); }
  BIGNUM *G() { return (m_g); }
  BIGNUM *GAB() { return (m_gab); }

  CBuffer *DesXEncrypt(CBuffer *);
  CBuffer *DesXDecrypt(CBuffer *);

protected:
  BIGNUM *m_p, *m_a, *m_g, *m_gab;
  CRYPTO_STATUS m_nCryptoStatus;
};

#else

#define CDHKey void

#endif

#endif




