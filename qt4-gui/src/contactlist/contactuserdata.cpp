// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007 Licq developers
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

#include "contactuserdata.h"

// Standard
#include <climits>
#include <cstring>

// Qt
#include <QDateTime>
#include <QImage>
#include <QTextCodec>

// Licq
#include <licq_events.h>
#include <licq_socket.h>
#include <licq_user.h>

// Qt-gui
#include "config/contactlist.h"

#include "core/gui-defines.h"

#include "helpers/licqstrings.h"
#include "helpers/usercodec.h"

#include "contactgroup.h"
#include "contactuser.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::ContactUserData */

#define FLASH_TIME 500

// Can't initialize timers here in static context so set to zero and let first object take care of initialization
QTimer* ContactUserData::myRefreshTimer = NULL;
QTimer* ContactUserData::myAnimateTimer = NULL;

int ContactUserData::myAnimatorCount = 0;


ContactUserData::ContactUserData(ICQUser* licqUser, QObject* parent)
  : myStatus(ICQ_STATUS_OFFLINE),
    myEvents(0),
    myFlash(false),
    mySubGroup(ContactListModel::OfflineSubGroup),
    myOnlCounter(0),
    myCarCounter(0),
    myAnimating(false),
    myUserIcon(NULL)
{
  myPpid = licqUser->PPID();

  if (licqUser->IdString() != NULL)
  {
    char* szRealId = NULL;
    ICQUser::MakeRealId(licqUser->IdString(), licqUser->PPID(), szRealId);
    myId = szRealId;
    delete [] szRealId;
  }

  if (myRefreshTimer == NULL)
  {
    // Create the static timer used to update dynamic contents
    myRefreshTimer = new QTimer(parent);
    myRefreshTimer->start(60 * 1000);
  }
  connect(myRefreshTimer, SIGNAL(timeout()), SLOT(refresh()));

  // Create the static timer used for animations
  if (myAnimateTimer == NULL)
  {
    myAnimateTimer = new QTimer(parent);
    myAnimateTimer->setInterval(FLASH_TIME);
  }

  updateAll(licqUser);
}

ContactUserData::~ContactUserData()
{
  // Free up animation timer resource if we were using it
  myFlash = false;
  myOnlCounter = 0;
  myCarCounter = 0;
  stopAnimation();

  // Remove this user from all groups
  while (!myUserInstances.isEmpty())
    delete myUserInstances.takeFirst();

  if (myUserIcon != NULL)
    delete myUserIcon;
}

void ContactUserData::update(CICQSignal* sig)
{
  switch (sig->SubSignal())
  {
    case USER_EVENTS:
      if (sig->Argument() == 0)
      {
        // User came online
        myCarCounter = ((5*1000/FLASH_TIME)+1)&(-2);
        startAnimation();
        return;
      }
      break;

    case USER_STATUS:
      if (sig->Argument() == 1)
      {
        // User fetched our auto respons message
        myOnlCounter = 5*1000/FLASH_TIME; // run about 5 seconds
        startAnimation();
        // Fall trough to actually update status
      }
      break;

//    case USER_BASIC:
//    case USER_GENERAL:
//    case USER_EXT:
//    case USER_SECURITY:
//    case USER_TYPING:
  }

  // TODO: Add better handling of subsignals so we don't have to update everything so often


  ICQUser* u = gUserManager.FetchUser(sig->Id(), sig->PPID(), LOCK_R);
  if (u != NULL)
  {
    // No specific handling for this signal so reread everything from the daemon
    updateAll(u);

    // Group membership is handled by ContactList so send it a signal to update
    emit updateUserGroups(this, u);

    gUserManager.DropUser(u);
  }
}

void ContactUserData::updateAll(ICQUser* u)
{
  myStatus = u->Status();
  myStatusFull = u->StatusFull();
  myStatusInvisible = u->StatusInvisible();
  myStatusTyping = u->GetTyping() == ICQ_TYPING_ACTIVE;
  myPhoneFollowMeStatus = u->PhoneFollowMeStatus();
  myIcqPhoneStatus = u->ICQphoneStatus();
  mySharedFilesStatus = u->SharedFilesStatus();
  myCustomAR = u->CustomAutoResponse()[0] != '\0';
  mySecure = u->Secure();
  myUrgent = false;
  myBirthday =  (u->Birthday() == 0);
  myPhone = u->GetPhoneNumber()[0] != '\0';
  myCellular = u->GetCellularNumber()[0] != '\0';
  myGPGKey = (u->GPGKey() != 0) && (strcmp(u->GPGKey(), "") != 0);
  myGPGKeyEnabled = u->UseGPG();

  myNotInList = u->NotInList();
  myNewUser = u->NewUser();
  myAwaitingAuth = u->GetAwaitingAuth();
  myInIgnoreList = u->IgnoreList();
  myInOnlineNotify = u->OnlineNotify();
  myInInvisibleList = u->InvisibleList();
  myInVisibleList = u->VisibleList();
  myTouched = u->Touched();
  myNewMessages = u->NewMessages();

  updateExtendedStatus();

  if (myEvents != u->NewMessages())
  {
    foreach (ContactUser* user, myUserInstances)
      user->group()->updateNumEvents(u->NewMessages() - myEvents, mySubGroup);

    myEvents = u->NewMessages();
  }

  updateText(u);

  if (myUserIcon != NULL)
  {
    delete myUserIcon;
    myUserIcon = NULL;
  }

  if (u->GetPicturePresent())
  {
    myUserIcon = new QImage(QString::fromLocal8Bit(BASE_DIR) + USER_DIR + "/" +
        myId + ".pic");
    if (myUserIcon->isNull())
    {
      delete myUserIcon;
      myUserIcon = NULL;
    }
  }

  myEventSubCommand = 0;

  if (u->NewMessages() > 0)
  {
    for (unsigned short i = 0; i < u->NewMessages(); i++)
    {
      switch (u->EventPeek(i)->SubCommand())
      {
        case ICQ_CMDxSUB_FILE:
          myEventSubCommand = ICQ_CMDxSUB_FILE;
          break;
        case ICQ_CMDxSUB_CHAT:
          if (myEventSubCommand != ICQ_CMDxSUB_FILE)
            myEventSubCommand = ICQ_CMDxSUB_CHAT;
          break;
        case ICQ_CMDxSUB_URL:
          if (myEventSubCommand != ICQ_CMDxSUB_FILE && myEventSubCommand != ICQ_CMDxSUB_CHAT)
            myEventSubCommand = ICQ_CMDxSUB_URL;
          break;
        case ICQ_CMDxSUB_CONTACTxLIST:
          if(myEventSubCommand != ICQ_CMDxSUB_FILE && myEventSubCommand != ICQ_CMDxSUB_CHAT && myEventSubCommand != ICQ_CMDxSUB_URL)
            myEventSubCommand = ICQ_CMDxSUB_CONTACTxLIST;
        case ICQ_CMDxSUB_MSG:
        default:
          if (myEventSubCommand == 0)
            myEventSubCommand = ICQ_CMDxSUB_MSG;
          break;
      }
      if (u->EventPeek(i)->IsUrgent())
        myUrgent = true;
    }
  }
  Config::ContactList::FlashMode flash = Config::ContactList::instance()->flash();
  bool shouldFlash = ((u->NewMessages() > 0 &&  flash == Config::ContactList::FlashAll) ||
      (myUrgent && flash == Config::ContactList::FlashUrgent));

  if (shouldFlash != myFlash)
  {
    myFlash = shouldFlash;

    if (myFlash)
    {
      myFlashCounter = false;
      startAnimation();
    }
  }

  // Set sub group to put user in
  ContactListModel::SubGroupType newSubGroup = ContactListModel::OnlineSubGroup;

  if (u->NotInList())
    newSubGroup = ContactListModel::NotInListSubGroup;

  else if (myStatus == ICQ_STATUS_OFFLINE)
    newSubGroup = ContactListModel::OfflineSubGroup;

  // If status has changed update the sub groups of all groups
  if (newSubGroup != mySubGroup)
  {
    foreach (ContactUser* user, myUserInstances)
      user->group()->updateSubGroup(mySubGroup, newSubGroup, myEvents);
    mySubGroup = newSubGroup;
  }

  updateSorting();
  updateVisibility();

  emit dataChanged(this);
}

void ContactUserData::updateExtendedStatus()
{
  // Make a bitmask of everything the delegate needs to display extended icons
  myExtendedStatus = 0;

  if (myStatusInvisible)
    myExtendedStatus |= ContactListModel::InvisibleStatus;

  if (myStatusTyping && myPpid == LICQ_PPID)
    myExtendedStatus |= ContactListModel::TypingStatus;

  if (myPhoneFollowMeStatus == ICQ_PLUGIN_STATUSxACTIVE)
    myExtendedStatus |= ContactListModel::PhoneFollowMeActiveStatus;
  else if (myPhoneFollowMeStatus == ICQ_PLUGIN_STATUSxBUSY)
    myExtendedStatus |= ContactListModel::PhoneFollowMeBusyStatus;

  if (myIcqPhoneStatus == ICQ_PLUGIN_STATUSxACTIVE)
    myExtendedStatus |= ContactListModel::IcqPhoneActiveStatus;
  else if (myIcqPhoneStatus == ICQ_PLUGIN_STATUSxBUSY)
    myExtendedStatus |= ContactListModel::IcqPhoneBusyStatus;

  if (mySharedFilesStatus == ICQ_PLUGIN_STATUSxACTIVE)
    myExtendedStatus |= ContactListModel::SharedFilesStatus;

  if (myCustomAR)
    myExtendedStatus |= ContactListModel::CustomArStatus;

  if (mySecure)
    myExtendedStatus |= ContactListModel::SecureStatus;

  if (myBirthday)
    myExtendedStatus |= ContactListModel::BirthdayStatus;

  if (myPhone)
    myExtendedStatus |= ContactListModel::PhoneStatus;

  if (myCellular)
    myExtendedStatus |= ContactListModel::CellularStatus;

  if (myGPGKey)
    myExtendedStatus |= ContactListModel::GpgKeyStatus;

  if (myGPGKeyEnabled)
    myExtendedStatus |= ContactListModel::GpgKeyEnabledStatus;

  if (myInIgnoreList)
    myExtendedStatus |= ContactListModel::IgnoreStatus;

  if (myInOnlineNotify)
    myExtendedStatus |= ContactListModel::OnlineNotifyStatus;

  if (myNotInList)
    myExtendedStatus |= ContactListModel::NotInListStatus;

  if (myInInvisibleList)
    myExtendedStatus |= ContactListModel::InvisibleListStatus;

  if (myInVisibleList)
    myExtendedStatus |= ContactListModel::VisibleListStatus;

  if (myNewUser)
    myExtendedStatus |= ContactListModel::NewUserStatus;

  if (myAwaitingAuth)
    myExtendedStatus |= ContactListModel::AwaitingAuthStatus;
}

void ContactUserData::updateSorting()
{
  // Set status sort order
  int sort = 9;
  switch (myStatus)
  {
    case ICQ_STATUS_FREEFORCHAT:
    case ICQ_STATUS_ONLINE:
      sort = 0;
      break;
    case ICQ_STATUS_OCCUPIED:
      sort = 1;
      break;
    case ICQ_STATUS_DND:
      sort = 2;
      break;
    case ICQ_STATUS_AWAY:
      sort = 3;
      break;
    case ICQ_STATUS_NA:
      sort = 4;
      break;
    case ICQ_STATUS_OFFLINE:
      sort = 5;
      break;
  }
  // Set sorting
  mySortKey = "";
  switch (Config::ContactList::instance()->sortByStatus())
  {
    case 0:  // no sorting
      break;
    case 1:  // sort by status
      mySortKey.sprintf("%1x", sort);
      break;
    case 2:  // sort by status and last event
      mySortKey.sprintf("%1x%016lx", sort, ULONG_MAX - myTouched);
      break;
    case 3:  // sort by status and number of new messages
      mySortKey.sprintf("%1x%016lx", sort, ULONG_MAX - myNewMessages);
      break;
  }
  mySortKey += myText[0];
}

bool ContactUserData::updateText(ICQUser* licqUser)
{
  bool hasChanged = false;

  for (unsigned short i = 0; i < Config::ContactList::instance()->columnCount(); i++)
  {
    QString format = Config::ContactList::instance()->columnFormat(i);
    format.replace("%a", "@_USER_ALIAS_@");

    QTextCodec* codec = UserCodec::codecForICQUser(licqUser);
    char* temp = licqUser->usprintf(codec->fromUnicode(format));
    QString newStr = codec->toUnicode(temp);
    free(temp);

    temp = licqUser->usprintf("%a");
    QString alias = QString::fromUtf8(temp);
    free(temp);

    newStr.replace("@_USER_ALIAS_@", alias);

    if (newStr != myText[i])
    {
      myText[i] = newStr;
      hasChanged = true;
    }
  }
  return hasChanged;
}

void ContactUserData::configUpdated()
{
  ICQUser* u = gUserManager.FetchUser(myId.toLatin1(), myPpid, LOCK_R);
  if (u == NULL)
    return;

  updateText(u);
  updateSorting();
  updateVisibility();

  gUserManager.DropUser(u);

  emit dataChanged(this);
}

void ContactUserData::updateVisibility()
{
  bool visibility = false;

  // Only hide contacts who are offline
  if (myStatus != ContactListModel::OfflineStatus)
    visibility = true;

  // Don't hide contacts with unread events
  if (myEvents > 0)
    visibility = true;

  // ... or the contact is in online notify list and option "Always show online notify users" is active
  if (Config::ContactList::instance()->alwaysShowONU() &&
      ((myExtendedStatus & ContactListModel::OnlineNotifyStatus) != 0))
    visibility = true;

  // ... or the contact is not added to the list
  if ((myExtendedStatus & ContactListModel::NotInListStatus) != 0)
    visibility = true;

  if (visibility == myVisibility)
    return;

  // Update groups
  foreach (ContactUser* user, myUserInstances)
    user->group()->updateVisibility(visibility);

  myVisibility = visibility;
}

void ContactUserData::refresh()
{
  // Here we update any content that may be dynamic, for example timestamps

  ICQUser* u = gUserManager.FetchUser(myId.toLatin1(), myPpid, LOCK_R);
  if (u == NULL)
    return;

  bool hasChanged = updateText(u);
  gUserManager.DropUser(u);

  // To reduce performance impact on refreshs, keep track whether anything changed so we don't force unnecessary updates
  if (hasChanged)
  {
    updateSorting();
    emit dataChanged(this);
  }
}

void ContactUserData::startAnimation()
{
  // Start common timer if not already running
  if (myAnimatorCount == 0)
    myAnimateTimer->start();

  // Attach to signal if we are not already animating something else
  if (!myAnimating)
  {
    myAnimatorCount++;
    connect(myAnimateTimer, SIGNAL(timeout()), SLOT(animate()));
    myAnimating = true;
  }
}

void ContactUserData::stopAnimation()
{
  myAnimating = myFlash || myOnlCounter || myCarCounter;

  // If this was the only or last animation disconnect from timer
  if (!myAnimating)
  {
    disconnect(myAnimateTimer, SIGNAL(timeout()), this, SLOT(animate()));
    myAnimatorCount--;

    // Stop animation timer if noone is using it anymore
    if (myAnimatorCount == 0)
      myAnimateTimer->stop();
  }
}

void ContactUserData::animate()
{
  // Animation for incoming event
  if (myFlash)
    myFlashCounter = !myFlashCounter;

  // Animation for going online
  if (myOnlCounter > 0)
    myOnlCounter--;

  // Animation for auto response read
  if (myCarCounter > 0)
    myCarCounter--;

  // Release timer if this was last animation
  stopAnimation();

  // data() will check the counter value to determine which icon to show so nothing to do here except triggering an update
  emit dataChanged(this);
}

void ContactUserData::addGroup(ContactUser* user)
{
  myUserInstances.append(user);
}

void ContactUserData::removeGroup(ContactUser* user)
{
  myUserInstances.removeAll(user);
}

QVariant ContactUserData::data(int column, int role) const
{
  switch (role)
  {
    case Qt::DisplayRole:
      if (column >= 0 && column < MAX_COLUMNCOUNT)
        return myText[column];
      break;

    case Qt::ToolTipRole:
      return tooltip();

    case ContactListModel::UserIdRole:
      return myId;

    case ContactListModel::PpidRole:
      return static_cast<unsigned int>(myPpid);

    case ContactListModel::ItemTypeRole:
      return ContactListModel::UserItem;

    case ContactListModel::SortPrefixRole:
      // Primary sorting by sub group, make room for the seprator bars between each sub group
      return 2 * mySubGroup + 1;

    case ContactListModel::SortRole:
      return mySortKey;

    case ContactListModel::UnreadEventsRole:
      return myEvents;

    case ContactListModel::SubGroupRole:
      return mySubGroup;

    case ContactListModel::StatusRole:
      return myStatus;

    case ContactListModel::FullStatusRole:
      return static_cast<unsigned int>(myStatusFull);

    case ContactListModel::ExtendedStatusRole:
      return myExtendedStatus;

    case ContactListModel::UserIconRole:
      if (myUserIcon != NULL)
        return *myUserIcon;
      break;

    case ContactListModel::EventSubCommandRole:
      return myEventSubCommand;

    case ContactListModel::CarAnimationRole:
      if (myCarCounter > 0)
        return myCarCounter & 1;
      break;

    case ContactListModel::OnlineAnimationRole:
      if (myOnlCounter > 0)
        return myOnlCounter & 1;
      break;

    case ContactListModel::EventAnimationRole:
      if (myFlash)
        return myFlashCounter;
      else if (myNewMessages > 0)
        // No flashing but we have unread events so show a static event icon
        return 1;
      break;

    case ContactListModel::VisibilityRole:
      return myVisibility;
  }

  return QVariant();
}

QString ContactUserData::tooltip() const
{
  ICQUser* u = gUserManager.FetchUser(myId.toLatin1(), myPpid, LOCK_R);
  if (u == NULL)
    return "";

  Config::ContactList* config = Config::ContactList::instance();

  QTextCodec* codec = UserCodec::codecForICQUser(u);
  QString s = "<nobr>";
  if (config->popupPicture() && u->GetPicturePresent())
  {
    QString file = QString::fromLocal8Bit(BASE_DIR) + USER_DIR + "/" +
      u->IdString() + ".pic";
    QImage picture = QImage(file);
    if (!picture.isNull())
      s += QString("<center><img src=\"%1\"></center>").arg(file);
  }

  s += LicqStrings::getStatus(myStatus, myStatusInvisible);

  if (config->popupAlias() && *u->GetAlias())
    s += "<br>" + QString::fromUtf8(u->GetAlias());

  if (config->popupName() && (*u->GetFirstName() || *u->GetLastName()))
  {
    s += "<br>";
    if (*u->GetFirstName())
      s += codec->toUnicode(u->GetFirstName());
    if (*u->GetFirstName() && *u->GetLastName())
      s += " ";
    if (*u->GetLastName())
      s += codec->toUnicode(u->GetLastName());
  }

  if (myBirthday)
    s += "<br><b>" + tr("Birthday Today!") + "</b>";

  if (myStatus != ICQ_STATUS_OFFLINE)
  {
    if (myStatusTyping)
      s += "<br>" + tr("Typing a message");
    if (myPhoneFollowMeStatus == ICQ_PLUGIN_STATUSxACTIVE)
      s += "<br>" + tr("Phone &quot;Follow Me&quot;: Available");
    else if (myPhoneFollowMeStatus == ICQ_PLUGIN_STATUSxBUSY)
      s += "<br>" + tr("Phone &quot;Follow Me&quot;: Busy");

    if (myIcqPhoneStatus == ICQ_PLUGIN_STATUSxACTIVE)
      s += "<br>" + tr("ICQphone: Available");
    else if (myIcqPhoneStatus == ICQ_PLUGIN_STATUSxBUSY)
      s += "<br>" + tr("ICQphone: Busy");

    if (mySharedFilesStatus == ICQ_PLUGIN_STATUSxACTIVE)
      s += "<br>" + tr("File Server: Enabled");
  }

  if (mySecure)
    s += "<br>" + tr("Secure connection");

  if (myCustomAR)
    s += "<br>" + tr("Custom Auto Response");

  if (config->popupAuth() && u->GetAwaitingAuth())
    s += "<br>" + tr("Awaiting authorization");

  if (!u->StatusOffline() && u->ClientInfo() && *u->ClientInfo())
    s += "<br>" + codec->toUnicode(u->ClientInfo());

  if (u->AutoResponse() && *u->AutoResponse() &&
      myStatus != ICQ_STATUS_OFFLINE &&
      myStatus != ICQ_STATUS_ONLINE)
    s += "<br><u>" + tr("Auto Response:") + "</u><br>&nbsp;&nbsp;&nbsp;" +
      codec->toUnicode(u->AutoResponse()).trimmed()
      .replace("\n", "<br>&nbsp;&nbsp;&nbsp;");

  if (config->popupEmail() && *u->GetEmailPrimary())
    s += "<br>" + tr("E: ") + codec->toUnicode(u->GetEmailPrimary());

  if (config->popupPhone() && myPhone)
    s += "<br>" + tr("P: ") + codec->toUnicode(u->GetPhoneNumber());

  if (config->popupCellular() && myCellular)
    s += "<br>" + tr("C: ") + codec->toUnicode(u->GetCellularNumber());

  if (config->popupEmail() && *u->GetFaxNumber())
    s += "<br>" + tr("F: ") + codec->toUnicode(u->GetFaxNumber());

  if (config->popupIP() && (u->Ip() || u->IntIp()))
  {
    char buf[32];
    ip_ntoa(u->Ip(), buf);
    s += "<br>" + tr("Ip: ") + QString::fromAscii(buf);
    if (u->IntIp() != 0 && u->IntIp() != u->Ip())
    {
      ip_ntoa(u->IntIp(), buf);
      s += " / " + QString::fromAscii(buf);
    }
  }

  if (config->popupLastOnline() && u->LastOnline() > 0)
  {
    QDateTime t;
    t.setTime_t(u->LastOnline());
    s += "<br>" + tr("O: ") + t.toString();
  }

  if (config->popupOnlineSince() && !u->StatusOffline())
  {
    time_t nLoggedIn = time(0) - u->OnlineSince();
    unsigned long nWeek, nDay, nHour, nMinute;
    nWeek = nLoggedIn / 604800;
    nDay = (nLoggedIn % 604800) / 86400;
    nHour = (nLoggedIn % 86400) / 3600;
    nMinute = (nLoggedIn % 3600) / 60;

    QString ds, temp;
    if (nWeek != 0)
    {
      ds += temp.setNum(nWeek);
      ds += " ";
      ds += (nWeek > 1 ? tr(" weeks") : tr(" week"));
    }
    if (nDay != 0)
    {
      if (nWeek != 0) ds += " ";
      ds += temp.setNum(nDay);
      ds += " ";
      ds += (nDay > 1 ? tr(" days") : tr(" day"));
    }
    if (nHour != 0)
    {
      if (nWeek != 0 || nDay != 0) ds += " ";
      ds += temp.setNum(nHour);
      ds += (nHour > 1 ? tr(" hours") : tr(" hour"));
    }
    if (nMinute != 0)
    {
      if (nWeek != 0 || nDay != 0 || nHour != 0) ds += " ";
      ds += temp.setNum(nMinute);
      ds += (nMinute > 1 ? tr(" minutes") : tr(" minute"));
    }
    if (nWeek == 0 && nDay == 0 && nHour == 0 && nMinute == 0)
      ds += tr("0 minutes");

    s += "<br>" + tr("Logged In: ") + ds;
  }

  if (config->popupIdleTime() && u->IdleSince())
  {
    char* szTemp;
    szTemp = u->usprintf("%I");
    QString temp(szTemp);
    free(szTemp);
    s += "<br>" + tr("Idle: ") + temp;
  }

  if (config->popupLocalTime())
  {
    char* szTemp;
    szTemp = u->usprintf("%F");
    QString temp(szTemp);
    free(szTemp);
    s += "<br>" + tr("Local time: ") + temp;
  }

  if (config->popupID())
  {
    char* szTemp;
    szTemp = u->usprintf("%u");
    QString temp(szTemp);
    free(szTemp);
    s += "<br>" + tr("ID: ") + temp;
  }

  gUserManager.DropUser(u);

  s += "</nobr>";

  return s;
}