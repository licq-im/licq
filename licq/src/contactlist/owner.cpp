#include "owner.h"

#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <licq_icq.h>

#include "gettext.h"
#include "licq_log.h"

using std::string;
using Licq::Owner;
using Licq::UserId;


LicqDaemon::Owner::Owner(const UserId& id)
  : User(id, true)
{
  // Pretend to be temporary to LicqUser constructior so it doesn't setup m_fConf
  // Restore NotInList flag to proper value when we get here
  m_bNotInList = false;
  m_bOnContactList = true;

  char szTemp[MAX_LINE_LEN];
  char filename[MAX_FILENAME_LEN];
  m_bException = false;
  m_bSavePassword = true;
  m_szPassword = NULL;
  m_nPDINFO = 0;

  // Get data from the config file
  char p[5];
  Licq::protocolId_toStr(p, myId.protocolId());
  snprintf(filename, MAX_FILENAME_LEN - 1, "%s/owner.%s", BASE_DIR, p);
  filename[MAX_FILENAME_LEN - 1] = '\0';

  m_fConf.SetFileName(filename);
  m_fConf.SetFlags(INI_FxWARN | INI_FxALLOWxCREATE);
  m_fConf.ReloadFile();
  m_fConf.CreateSection("user");
  m_fConf.FlushFile();
  m_fConf.SetFlags(0);

  // Make sure config file is mode 0600
  if (chmod(filename, S_IRUSR | S_IWUSR) == -1)
  {
    gLog.Warn(tr("%sUnable to set %s to mode 0600. Your password is vulnerable if stored locally.\n"),
                 L_WARNxSTR, filename);
  }

  // And finally our favorite function
  LoadInfo();
  // Owner encoding fixup to be UTF-8 by default
  if (strcmp(m_szEncoding, "") == 0)
    SetString(&m_szEncoding, "UTF-8");
  m_fConf.ReadStr("Password", szTemp, "", false);
  SetPassword(&szTemp[1]); // skip leading space since we didn't trim
  m_fConf.ReadBool("WebPresence", m_bWebAware, false);
  m_fConf.ReadBool("HideIP", m_bHideIp, false);
  m_fConf.ReadNum("RCG", m_nRandomChatGroup, ICQ_RANDOMxCHATxGROUP_NONE);
  m_fConf.ReadStr("AutoResponse", szTemp, "");
  m_fConf.ReadNum("SSTime", m_nSSTime, 0L);
  m_fConf.ReadNum("SSCount", m_nSSCount, 0);
  m_fConf.ReadNum("PDINFO", m_nPDINFO, 0);

  SetAutoResponse(szTemp);

  m_fConf.CloseFile();

  gLog.Info(tr("%sOwner configuration for %s.\n"), L_INITxSTR, myId.toString().c_str());

  snprintf(filename, MAX_FILENAME_LEN - 1, "%s/%s/owner.%s.%s.history", BASE_DIR, HISTORY_DIR,
      myId.accountId().c_str(), p);
    SetHistoryFile(filename);

  if (m_nTimezone != SystemTimezone() && m_nTimezone != Licq::TIMEZONE_UNKNOWN)
  {
    gLog.Warn(tr("%sCurrent Licq GMT offset (%d) does not match system GMT offset (%d).\n"
              "%sUpdate general info on server to fix.\n"),
       L_WARNxSTR, m_nTimezone, SystemTimezone(), L_BLANKxSTR);
  }
  SetEnableSave(true);
}

LicqDaemon::Owner::~Owner()
{
  // Save the current auto response
  if (!m_fConf.ReloadFile())
  {
     gLog.Error("%sError opening '%s' for reading.\n%sSee log for details.\n",
                L_ERRORxSTR, m_fConf.FileName(), L_BLANKxSTR);
     return;
  }
  m_fConf.SetSection("user");
  m_fConf.WriteStr("AutoResponse", AutoResponse());
  m_fConf.WriteNum("SSTime", (unsigned long)m_nSSTime);
  m_fConf.WriteNum("SSCount", m_nSSCount);
  m_fConf.WriteNum("PDINFO", m_nPDINFO);
  if (!m_fConf.FlushFile())
  {
    gLog.Error("%sError opening '%s' for writing.\n%sSee log for details.\n",
               L_ERRORxSTR, m_fConf.FileName(), L_BLANKxSTR);
    return;
  }
  m_fConf.CloseFile();

  if ( m_szPassword )
    free( m_szPassword );
}

unsigned long Owner::AddStatusFlags(unsigned long s) const
{
  s &= 0x0000FFFF;

  if (WebAware())
    s |= ICQ_STATUS_FxWEBxPRESENCE;
  if (HideIp())
    s |= ICQ_STATUS_FxHIDExIP;
  if (Birthday() == 0)
    s |= ICQ_STATUS_FxBIRTHDAY;
  if (PhoneFollowMeStatus() != ICQ_PLUGIN_STATUSxINACTIVE)
    s |= ICQ_STATUS_FxPFM;
  if (PhoneFollowMeStatus() == ICQ_PLUGIN_STATUSxACTIVE)
    s |= ICQ_STATUS_FxPFMxAVAILABLE;

  return s;
}

void Owner::SaveLicqInfo()
{
  if (!EnableSave()) return;

  User::SaveLicqInfo();

  if (!m_fConf.ReloadFile())
  {
     gLog.Error("%sError opening '%s' for reading.\n%sSee log for details.\n",
                L_ERRORxSTR, m_fConf.FileName(), L_BLANKxSTR);
     return;
  }
  m_fConf.SetSection("user");
  m_fConf.writeString("Uin", accountId());
  m_fConf.WriteBool("WebPresence", WebAware());
  m_fConf.WriteBool("HideIP", HideIp());
  m_fConf.WriteBool("Authorization", GetAuthorization());
  m_fConf.WriteNum("RCG", RandomChatGroup());
  m_fConf.WriteNum("SSTime", (unsigned long)m_nSSTime);
  m_fConf.WriteNum("SSCount", m_nSSCount);
  m_fConf.WriteNum("PDINFO", m_nPDINFO);

  if (m_bSavePassword)
    m_fConf.WriteStr("Password", Password());
  else
    m_fConf.WriteStr("Password", "");

  if (!m_fConf.FlushFile())
  {
    gLog.Error("%sError opening '%s' for writing.\n%sSee log for details.\n",
               L_ERRORxSTR, m_fConf.FileName(), L_BLANKxSTR);
    return;
  }

  m_fConf.CloseFile();
}

void Owner::SetStatusOffline()
{
  SetStatus(m_nStatus | ICQ_STATUS_OFFLINE);
}

void Owner::SetPicture(const char *f)
{
  char szFilename[MAX_FILENAME_LEN];
  szFilename[MAX_FILENAME_LEN - 1] = '\0';
  snprintf(szFilename, MAX_FILENAME_LEN - 1, "%s/owner.pic", BASE_DIR);
  if (f == NULL)
  {
    SetPicturePresent(false);
    if (remove(szFilename) != 0 && errno != ENOENT)
    {
      gLog.Error("%sUnable to delete %s's picture file (%s):\n%s%s.\n",
          L_ERRORxSTR, myAlias.c_str(), szFilename, L_BLANKxSTR,
                         strerror(errno));
    }
  }
  else if (strcmp(f, szFilename) == 0)
  {
    SetPicturePresent(true);
    return;
  }
  else
  {
    int source = open(f, O_RDONLY);
    if (source == -1)
    {
      gLog.Error("%sUnable to open source picture file (%s):\n%s%s.\n",
                       L_ERRORxSTR, f, L_BLANKxSTR, strerror(errno));
      return;
    }

    int dest = open(szFilename, O_WRONLY | O_CREAT | O_TRUNC, 00664);
    if (dest == -1)
    {
      gLog.Error("%sUnable to open picture file (%s):\n%s%s.\n", L_ERRORxSTR,
                                     szFilename, L_BLANKxSTR, strerror(errno));
      close(source);
      return;
    }

    char buf[8192];
    ssize_t s;
    while (1)
    {
      s = read(source, buf, sizeof(buf));
      if (s == 0)
      {
        SetPicturePresent(true);
        break;
      }
      else if (s == -1)
      {
        gLog.Error("%sError reading from %s:\n%s%s.\n", L_ERRORxSTR, f,
                                         L_BLANKxSTR, strerror(errno));
        SetPicturePresent(false);
        break;
      }

      if (write(dest, buf, s) != s)
      {
        gLog.Error("%sError writing to %s:\n%s%s.\n", L_ERRORxSTR, f,
                                         L_BLANKxSTR, strerror(errno));
        SetPicturePresent(false);
        break;
      }
    }

    close(source);
    close(dest);
  }
}
