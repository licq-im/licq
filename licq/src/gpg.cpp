
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>

#include "licq_file.h"
#include "licq_gpg.h"
#include "licq_log.h"

CGPGHelper gGPGHelper;

const char CGPGHelper::pgpSig[] = "-----BEGIN PGP MESSAGE-----";


CGPGHelper::CGPGHelper()
  : mKeysIni(INI_FxALLOWxCREATE)
{
  mCtx = 0;
  mGPGPassphrase = 0;
}


CGPGHelper::~CGPGHelper()
{
#ifdef HAVE_LIBGPGME
  if (mCtx) gpgme_release(mCtx);
#endif
  if (mGPGPassphrase) free(mGPGPassphrase);
  mKeysIni.CloseFile();
}


char *CGPGHelper::Decrypt(const char *szCipher)
{
#ifdef HAVE_LIBGPGME
  if (!mCtx) return 0;

  size_t nRead = 0;
  GpgmeData cipher, plain;

  CGPGMEMutex mutex;
  if (!mutex.Lock()) return 0;
  if (gpgme_data_new(&cipher) != GPGME_No_Error) return 0;
  char *buf = strdup(szCipher);
  GpgmeError err;
  gpgme_data_write(cipher, buf, strlen(buf));
  free(buf);

  if (gpgme_data_new(&plain) != GPGME_No_Error)
  {
    gpgme_data_release(cipher);
    return 0;
  }

  if ((err = gpgme_op_decrypt(mCtx, cipher, plain)) != GPGME_No_Error)
    gLog.Warn("%sgpgme message decryption failed: %s\n", L_WARNxSTR, gpgme_strerror(err));
  
  gpgme_data_release(cipher);
  buf = gpgme_data_release_and_get_mem(plain, &nRead);
  if (!buf) return 0;
  buf = (char *)realloc(buf, nRead+1);
  buf[nRead] = 0;
  return buf;
#else
  return 0;
#endif
}


char *CGPGHelper::Encrypt(const char *szPlain, const char *szId,
                          unsigned long nPPID)
{
#ifdef HAVE_LIBGPGME
  if (!mCtx) return 0;
  if (!szPlain) return 0;

  char szUser[MAX_LINE_LEN], buf[MAX_LINE_LEN];
  sprintf(szUser, "%s.%lu", szId, nPPID);
  mKeysIni.SetSection("keys");
  if (!mKeysIni.ReadStr(szUser, buf)) return 0;
	
  gLog.Info("[GPG] Encrypting message to %s.\n", szId);

  CGPGMEMutex mutex;
  if (!mutex.Lock()) return 0;
  GpgmeRecipients rcps;
  GpgmeData plain = 0, cipher = 0;
  GpgmeError err;
  char *szCipher = 0;

  if (gpgme_recipients_new(&rcps) != GPGME_No_Error) return 0;
  if (gpgme_recipients_add_name_with_validity(rcps, buf, GPGME_VALIDITY_FULL) != GPGME_No_Error)
    gLog.Error("%sCouldn't use gpgme recipient: %s\n", L_ERRORxSTR, buf);
  else
  {
    if (gpgme_data_new_from_mem(&plain, szPlain, strlen(szPlain), 1) == GPGME_No_Error &&
        gpgme_data_new(&cipher) == GPGME_No_Error)
    {
      if ((err = gpgme_op_encrypt(mCtx, rcps, plain, cipher)) == GPGME_No_Error) 
      {
        size_t nRead;
        if (gpgme_data_read(cipher, 0, 0, &nRead) == GPGME_No_Error)
        {
          szCipher = (char *)malloc(nRead+1);
          memset(szCipher, 0, nRead+1);
          gpgme_data_read(cipher, szCipher, nRead, &nRead);
        }
      }
      else
        gLog.Error("%sEncryption failed: %s\n", L_ERRORxSTR, gpgme_strerror(err));
    }

    if (cipher) gpgme_data_release(cipher);
    if (plain) gpgme_data_release(plain);
  }

  gpgme_recipients_release(rcps);
  return szCipher;
#else
  return 0;
#endif
}


void CGPGHelper::Start()
{
#ifdef HAVE_LIBGPGME
  char buf[MAX_LINE_LEN];
  snprintf(buf, MAX_LINE_LEN, "%s/licq_gpg.conf", BASE_DIR);
  mKeysIni.LoadFile(buf);

  mKeysIni.SetSection("gpg");
  mKeysIni.ReadStr("passphrase", buf); mGPGPassphrase = strdup(buf);

  const char *gpgme_ver = gpgme_check_version(0);
  gLog.Info("%sgpgme library found: %s\n", L_INITxSTR, gpgme_ver);
	
  if (gpgme_engine_check_version(GPGME_PROTOCOL_OpenPGP) != GPGME_No_Error)
    gLog.Error("%sgpgme engine OpenPGP not found!\n", L_ERRORxSTR);

  gpgme_new(&mCtx);
  gpgme_set_protocol(mCtx, GPGME_PROTOCOL_OpenPGP);
  gpgme_set_armor(mCtx, 1);
  gpgme_set_passphrase_cb(mCtx, PassphraseCallback, 0);
#endif
}


const char *CGPGHelper::PassphraseCallback(void *, const char *, void **)
{
  return gGPGHelper.mGPGPassphrase;
}



/*** GPGME lock for thread safety ***/

pthread_mutex_t CGPGMEMutex::mutex;

CGPGMEMutex::~CGPGMEMutex()
{
  pthread_mutex_unlock(&mutex);
}

CGPGMEMutex::CGPGMEMutex()
{
  pthread_mutex_init(&mutex, 0);
}

bool CGPGMEMutex::Lock()
{
  return pthread_mutex_lock(&mutex) == 0;
}

