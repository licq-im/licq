/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2004-2014 Licq developers <licq-dev@googlegroups.com>
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

#include "gpghelper.h"
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>
#include <licq/thread/mutexlocker.h>
#include <licq/logging/log.h>

#include "gettext.h"

using namespace LicqDaemon;
using Licq::gLog;
using Licq::GpgKey;
using Licq::GpgUid;
using Licq::MutexLocker;
using std::list;
using std::string;

// Declare our global GpgHelper (internal for deamon)
LicqDaemon::GpgHelper LicqDaemon::gGpgHelper;

// Initialize global Licq::GpgHelper to refer to the internal GpgHelper
Licq::GpgHelper& Licq::gGpgHelper(LicqDaemon::gGpgHelper);

const char Licq::GpgHelper::pgpSig[] = "-----BEGIN PGP MESSAGE-----";


GpgHelper::GpgHelper()
  : myKeysIni("licq_gpg.conf")
{
#ifdef HAVE_LIBGPGME
  mCtx = 0;
#endif
}


GpgHelper::~GpgHelper()
{
#ifdef HAVE_LIBGPGME
  if (mCtx) gpgme_release(mCtx);
#endif
}


char* GpgHelper::Decrypt(const char *szCipher)
{
#ifdef HAVE_LIBGPGME
  if (!mCtx) return 0;

  size_t nRead = 0;
  gpgme_data_t cipher, plain;

  MutexLocker lock(myMutex);
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
    gLog.warning(tr("[GPG] gpgme message decryption failed: %s"), gpgme_strerror(err));
  
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

char* GpgHelper::Encrypt(const char *szPlain, const Licq::UserId& userId)
{
#ifdef HAVE_LIBGPGME
  if (!mCtx) return 0;
  if (!szPlain) return 0;

  char iniKey[4096];
  string key;
  sprintf(iniKey, "%s.%lu", userId.accountId().c_str(), userId.protocolId());
  myKeysIni.setSection("keys");

  {
    Licq::UserReadGuard u(userId);
    if (u.isLocked())
      key = u->gpgKey();
  }

  if (key.empty() && !myKeysIni.get(iniKey, key))
    return 0;

  gLog.info(tr("[GPG] Encrypting message to %s."), userId.toString().c_str());

  MutexLocker lock(myMutex);
  gpgme_key_t rcps[2];
  gpgme_data_t plain = 0, cipher = 0;
  gpgme_error_t err;
  char *szCipher = 0;
  size_t nRead = 0;

  rcps[1] = 0;
  // Still use the old method, gpgme_get_key requires the fingerprint, which
  // actually isn't very helpful.
  if (gpgme_op_keylist_start (mCtx, key.c_str(), 0) != GPG_ERR_NO_ERROR)
    gLog.error(tr("[GPG] Couldn't use gpgme recipient: %s"), key.c_str());
  else
  {
    if (gpgme_op_keylist_next(mCtx, rcps) != GPG_ERR_NO_ERROR)
      gLog.error(tr("[GPG] Couldn't get key: %s"), key.c_str());
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
          gLog.error(tr("[GPG] Encryption failed: %s"), gpgme_strerror(err));
      }
    }

    if (cipher) gpgme_data_release(cipher);
    if (plain) gpgme_data_release(plain);
  }

  gpgme_key_unref(rcps[0]);
  return szCipher;
#else
  (void)szPlain;
  (void)userId;
  return 0;
#endif
}

list<GpgKey>* GpgHelper::getKeyList() const
{
  list<GpgKey>* keyList = new list<GpgKey>();
#ifdef HAVE_LIBGPGME
  MutexLocker lock(myMutex);

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

void GpgHelper::Start()
{
#ifdef HAVE_LIBGPGME
  myKeysIni.loadFile();

  myKeysIni.setSection("gpg");
  myKeysIni.get("passphrase", myGpgPassphrase, "");

  const char *gpgme_ver = gpgme_check_version(0);
  gLog.info(tr("[GPG] gpgme library found: %s"), gpgme_ver);

  if (gpgme_engine_check_version(GPGME_PROTOCOL_OpenPGP) != GPG_ERR_NO_ERROR)
    gLog.error(tr("[GPG] gpgme engine OpenPGP not found"));

  gpgme_new(&mCtx);
  gpgme_set_protocol(mCtx, GPGME_PROTOCOL_OpenPGP);
  gpgme_set_armor(mCtx, 1);
  gpgme_set_passphrase_cb(mCtx, PassphraseCallback, this);
#endif
}

#ifdef HAVE_LIBGPGME
gpgme_error_t GpgHelper::PassphraseCallback(void* helperPtr, const char *,
    const char *, int prev_was_bad, int fd)
{
  GpgHelper* helper = static_cast<GpgHelper*>(helperPtr);
  const char nl = '\n';
  const string& pf = helper->myGpgPassphrase;
  if (prev_was_bad || pf.empty())
  {
    write(fd, &nl, 1);
    return GPG_ERR_CANCELED;
  }
  write(fd, pf.c_str(), pf.size());
  write(fd, &nl, 1);
  return GPG_ERR_NO_ERROR;
}
#endif

