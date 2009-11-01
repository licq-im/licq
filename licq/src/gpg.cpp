
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_LIBGPGME
#include <gpgme.h>
#endif
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "licq_file.h"
#include "licq_gpg.h"
#include "licq_log.h"
#include <licq_user.h>

using namespace std;

CGPGHelper gGPGHelper;

const char CGPGHelper::pgpSig[] = "-----BEGIN PGP MESSAGE-----";

// These variables are local to the GPGHelper but are not in the header file
// to reduce dependancies for other classes and plugins
#ifdef HAVE_LIBGPGME
static gpgme_ctx_t mCtx;
static gpgme_error_t PassphraseCallback(void *, const char *, const char*, int, int);
#endif
static char* mGPGPassphrase;


CGPGHelper::CGPGHelper()
  : mKeysIni(INI_FxALLOWxCREATE)
{
#ifdef HAVE_LIBGPGME
  mCtx = 0;
#endif
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
  gpgme_data_t cipher, plain;

  CGPGMEMutex mutex;
  if (!mutex.Lock()) return 0;
  if (gpgme_data_new(&cipher) != GPG_ERR_NO_ERROR) return 0;
  char *buf = strdup(szCipher);
  gpgme_error_t err;
  gpgme_data_write(cipher, buf, strlen(buf));
  free(buf);

  if (gpgme_data_new(&plain) != GPG_ERR_NO_ERROR)
  {
    gpgme_data_release(cipher);
    return 0;
  }

  gpgme_data_seek(cipher, 0, SEEK_SET);
  if ((err = gpgme_op_decrypt(mCtx, cipher, plain)) != GPG_ERR_NO_ERROR)
    gLog.Warn("%s[GPG] gpgme message decryption failed: %s\n", L_WARNxSTR, gpgme_strerror(err));
  
  gpgme_data_release(cipher);
  buf = gpgme_data_release_and_get_mem(plain, &nRead);
  if (!buf) return 0;
  buf = (char *)realloc(buf, nRead+1);
  buf[nRead] = 0;
  return buf;
#else
  (void)szCipher;
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
  buf[0] = '\0';
  sprintf(szUser, "%s.%lu", szId, nPPID);
  mKeysIni.SetSection("keys");

  const ICQUser* u = gUserManager.FetchUser(szId, nPPID, LOCK_R);
  if (u != NULL)
  {
    const char *tmp = u->GPGKey();
    if ( tmp && tmp[0]!='\0' )
      strncpy( buf, tmp, MAX_LINE_LEN-1 );
    gUserManager.DropUser( u );
  }
  if ( !buf[0] && !mKeysIni.ReadStr(szUser, buf) ) return 0;
	
  gLog.Info("[GPG] Encrypting message to %s.\n", szId);

  CGPGMEMutex mutex;
  if (!mutex.Lock()) return 0;
  gpgme_key_t rcps[2];
  gpgme_data_t plain = 0, cipher = 0;
  gpgme_error_t err;
  char *szCipher = 0;
  size_t nRead = 0;

  rcps[1] = 0;
  // Still use the old method, gpgme_get_key requires the fingerprint, which
  // actually isn't very helpful.
  if (gpgme_op_keylist_start (mCtx, buf, 0) != GPG_ERR_NO_ERROR)
    gLog.Error("%s[GPG] Couldn't use gpgme recipient: %s\n", L_ERRORxSTR, buf);
  else
  {
    if (gpgme_op_keylist_next(mCtx, rcps) != GPG_ERR_NO_ERROR)
      gLog.Error("%s[GPG] Couldn't get key: %s\n", L_ERRORxSTR, buf);
    else
    {
      if (gpgme_data_new_from_mem(&plain, szPlain, strlen(szPlain), 0) == GPG_ERR_NO_ERROR &&
          gpgme_data_new(&cipher) == GPG_ERR_NO_ERROR)
      {
        if ((err = gpgme_op_encrypt(mCtx, rcps, GPGME_ENCRYPT_ALWAYS_TRUST, plain, cipher)) == GPG_ERR_NO_ERROR)
        {
          szCipher = gpgme_data_release_and_get_mem(cipher, &nRead);
          cipher = 0;
          szCipher = (char *)realloc(szCipher, nRead + 1);
          szCipher[nRead] = 0;
        }
        else
          gLog.Error("%s[GPG] Encryption failed: %s\n", L_ERRORxSTR, gpgme_strerror(err));
      }
    }

    if (cipher) gpgme_data_release(cipher);
    if (plain) gpgme_data_release(plain);
  }

  gpgme_key_unref(rcps[0]);
  return szCipher;
#else
  (void)szPlain;
  (void)szId;
  (void)nPPID;
  return 0;
#endif
}

list<GpgKey>* CGPGHelper::getKeyList() const
{
  list<GpgKey>* keyList = new list<GpgKey>();
#ifdef HAVE_LIBGPGME
  CGPGMEMutex mutex;
  if (!mutex.Lock()) return keyList;

  int err = gpgme_op_keylist_start(mCtx, NULL, 0);

  while (err == 0)
  {
    gpgme_key_t key;
    err = gpgme_op_keylist_next(mCtx, &key);
    if (err)
      break;
    gpgme_user_id_t uid = key->uids;
    if (uid && key->can_encrypt && key->subkeys)
    {
      GpgKey keyData;
      keyData.keyid = key->subkeys->keyid;

      while (uid != NULL)
      {
        if (uid->name != NULL && uid->email != NULL)
        {
          GpgUid uidData;
          uidData.name = uid->name;
          uidData.email = uid->email;
          keyData.uids.push_back(uidData);
        }
        uid = uid->next;
      }
      keyList->push_back(keyData);
    }
    gpgme_key_release(key);
  }
#endif
  return keyList;
}

void CGPGHelper::Start()
{
#ifdef HAVE_LIBGPGME
  char buf[MAX_LINE_LEN];
  snprintf(buf, MAX_LINE_LEN, "%s/licq_gpg.conf", BASE_DIR);
  mKeysIni.LoadFile(buf);

  mKeysIni.SetSection("gpg");
  mKeysIni.ReadStr("passphrase", buf, ""); mGPGPassphrase = strdup(buf);

  const char *gpgme_ver = gpgme_check_version(0);
  gLog.Info("%s[GPG] gpgme library found: %s\n", L_INITxSTR, gpgme_ver);
	
  if (gpgme_engine_check_version(GPGME_PROTOCOL_OpenPGP) != GPG_ERR_NO_ERROR)
    gLog.Error("%s[GPG] gpgme engine OpenPGP not found!\n", L_ERRORxSTR);

  gpgme_new(&mCtx);
  gpgme_set_protocol(mCtx, GPGME_PROTOCOL_OpenPGP);
  gpgme_set_armor(mCtx, 1);
  gpgme_set_passphrase_cb(mCtx, PassphraseCallback, 0);
#endif
}

#ifdef HAVE_LIBGPGME
gpgme_error_t PassphraseCallback(void *, const char *, const char *, int, int fd)
{
  const char nl = '\n';
  const char* const pf = mGPGPassphrase;
  if (pf == 0)
  {
    write(fd, &nl, 1);
    return GPG_ERR_CANCELED;
  }
  write(fd, pf, strlen(pf));
  write(fd, &nl, 1);
  return GPG_ERR_NO_ERROR;
}
#endif



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

