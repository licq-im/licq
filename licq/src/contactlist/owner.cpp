#include "owner.h"

#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <licq/icqdefines.h>

#include "gettext.h"
#include "licq_log.h"

using std::string;
using Licq::UserId;
using namespace LicqDaemon;


Owner::Owner(const UserId& id)
  : User(id, true)
{
  // Pretend to be temporary to LicqUser constructior so it doesn't setup myConf
  // Restore NotInList flag to proper value when we get here
  m_bNotInList = false;
  m_bOnContactList = true;

  m_bException = false;
  m_bSavePassword = true;
  myPassword = "";
  myPDINFO = 0;

  // Get data from the config file
  char p[5];
  Licq::protocolId_toStr(p, myId.protocolId());
  string filename = BASE_DIR;
  filename += "/owner.";
  filename += p;
  myConf.setFilename(filename);
  myConf.loadFile();
  myConf.setSection("user");
  myConf.writeFile();

  // Make sure config file is mode 0600
  if (chmod(filename.c_str(), S_IRUSR | S_IWUSR) == -1)
  {
    gLog.Warn(tr("%sUnable to set %s to mode 0600. Your password is vulnerable if stored locally.\n"),
        L_WARNxSTR, filename.c_str());
  }

  // And finally our favorite function
  LoadInfo();
  // Owner encoding fixup to be UTF-8 by default
  if (myEncoding.empty())
    myEncoding = "UTF-8";
  myConf.get("Password", myPassword, "");
  myConf.get("WebPresence", m_bWebAware, false);
  myConf.get("HideIP", m_bHideIp, false);
  myConf.get("RCG", m_nRandomChatGroup, ICQ_RANDOMxCHATxGROUP_NONE);
  myConf.get("AutoResponse", myAutoResponse, "");
  unsigned long sstime;
  myConf.get("SSTime", sstime, 0);
  m_nSSTime = sstime;
  myConf.get("SSCount", mySsCount, 0);
  myConf.get("PDINFO", myPDINFO, 0);

  gLog.Info(tr("%sOwner configuration for %s.\n"), L_INITxSTR, myId.toString().c_str());

  filename = BASE_DIR;
  filename += "/";
  filename += HISTORY_DIR;
  filename += "/owner.";
  filename += myId.accountId();
  filename += ".";
  filename += p;
  filename += ".history";
  SetHistoryFile(filename.c_str());

  if (m_nTimezone != SystemTimezone() && m_nTimezone != Licq::TIMEZONE_UNKNOWN)
  {
    gLog.Warn(tr("%sCurrent Licq GMT offset (%d) does not match system GMT offset (%d).\n"
              "%sUpdate general info on server to fix.\n"),
       L_WARNxSTR, m_nTimezone, SystemTimezone(), L_BLANKxSTR);
  }
  SetEnableSave(true);
}

Owner::~Owner()
{
  // Save the current auto response
  if (!myConf.loadFile())
  {
     gLog.Error("%sError opening '%s' for reading.\n%sSee log for details.\n",
        L_ERRORxSTR, myConf.filename().c_str(), L_BLANKxSTR);
     return;
  }
  myConf.setSection("user");
  myConf.set("AutoResponse", myAutoResponse);
  myConf.set("SSTime", (unsigned long)m_nSSTime);
  myConf.set("SSCount", mySsCount);
  myConf.set("PDINFO", myPDINFO);
  if (!myConf.writeFile())
  {
    gLog.Error("%sError opening '%s' for writing.\n%sSee log for details.\n",
        L_ERRORxSTR, myConf.filename().c_str(), L_BLANKxSTR);
    return;
  }
}

unsigned long Licq::Owner::AddStatusFlags(unsigned long s) const
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

  if (!myConf.loadFile())
  {
     gLog.Error("%sError opening '%s' for reading.\n%sSee log for details.\n",
        L_ERRORxSTR, myConf.filename().c_str(), L_BLANKxSTR);
     return;
  }
  myConf.setSection("user");
  myConf.set("Uin", accountId());
  myConf.set("WebPresence", WebAware());
  myConf.set("HideIP", HideIp());
  myConf.set("Authorization", GetAuthorization());
  myConf.set("RCG", RandomChatGroup());
  myConf.set("SSTime", (unsigned long)m_nSSTime);
  myConf.set("SSCount", mySsCount);
  myConf.set("PDINFO", myPDINFO);

  if (m_bSavePassword)
    myConf.set("Password", myPassword);
  else
    myConf.set("Password", "");

  if (!myConf.writeFile())
  {
    gLog.Error("%sError opening '%s' for writing.\n%sSee log for details.\n",
        L_ERRORxSTR, myConf.filename().c_str(), L_BLANKxSTR);
    return;
  }
}

void Licq::Owner::SetStatusOffline()
{
  SetStatus(m_nStatus | ICQ_STATUS_OFFLINE);
}

void Licq::Owner::SetPicture(const char *f)
{
  string filename = pictureFileName();
  if (f == NULL)
  {
    SetPicturePresent(false);
    if (remove(filename.c_str()) != 0 && errno != ENOENT)
    {
      gLog.Error("%sUnable to delete %s's picture file (%s):\n%s%s.\n",
          L_ERRORxSTR, myAlias.c_str(), filename.c_str(), L_BLANKxSTR,
                         strerror(errno));
    }
  }
  else if (strcmp(f, filename.c_str()) == 0)
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

    int dest = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 00664);
    if (dest == -1)
    {
      gLog.Error("%sUnable to open picture file (%s):\n%s%s.\n", L_ERRORxSTR,
          filename.c_str(), L_BLANKxSTR, strerror(errno));
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

string Licq::Owner::pictureFileName() const
{
  string filename = BASE_DIR;
  filename += "owner.pic";
  return filename;
}
