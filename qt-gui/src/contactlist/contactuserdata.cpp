/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2013 Licq developers <licq-dev@googlegroups.com>
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

// Licq
#include <licq/contactlist/user.h>
#include <licq/icq/user.h>
#include <licq/plugin/pluginmanager.h>
#include <licq/pluginsignal.h>
#include <licq/socket.h>
#include <licq/userevents.h>

// Qt-gui
#include "config/contactlist.h"

#include "core/gui-defines.h"

#include "contactgroup.h"
#include "contactuser.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::ContactUserData */

using std::string;
using Licq::User;

#define FLASH_TIME 500

// Can't initialize timers here in static context so set to zero and let first object take care of initialization
QTimer* ContactUserData::myRefreshTimer = NULL;
QTimer* ContactUserData::myAnimateTimer = NULL;

int ContactUserData::myAnimatorCount = 0;


ContactUserData::ContactUserData(const Licq::User* licqUser, QObject* parent)
  : myStatus(User::OfflineStatus),
    myEvents(0),
    myFlash(false),
    mySubGroup(ContactListModel::OfflineSubGroup),
    myVisibility(false),
    myOnlCounter(0),
    myCarCounter(0),
    myAnimating(false),
    myUserIcon(NULL)
{
  myUserId = licqUser->id();

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

  update(licqUser, 0);
}

ContactUserData::~ContactUserData()
{
  // Free up animation timer resource if we were using it
  if (myFlash || myOnlCounter > 0 || myCarCounter > 0)
    stopAnimation();

  // Remove this user from all groups
  while (!myUserInstances.isEmpty())
    delete myUserInstances.takeFirst();

  if (myUserIcon != NULL)
    delete myUserIcon;
}

void ContactUserData::update(unsigned long subSignal, int argument)
{
  if (subSignal == Licq::PluginSignal::UserEvents && argument == 0)
  {
    // User fetched our auto response message
    myCarCounter = ((5*1000/FLASH_TIME)+1)&(-2);
    startAnimation();
    return;
  }

  if (subSignal == Licq::PluginSignal::UserStatus && argument == 1)
  {
    // User came online
    myOnlCounter = 5*1000/FLASH_TIME; // run about 5 seconds
    startAnimation();
    // Fall trough to actually update status
  }

  Licq::UserReadGuard u(myUserId);
  if (!u.isLocked())
    return;

  update(*u, subSignal);
}

void ContactUserData::update(const Licq::User* u, unsigned long subSignal)
{
  // Save some old values so we know if we got changes to signal
  ContactListModel::SubGroupType oldSubGroup = mySubGroup;
  bool oldVisibility = myVisibility;

  if (subSignal == 0 || subSignal == Licq::PluginSignal::UserStatus)
  {
    myStatus = u->status();
    myStatusInvisible = u->isInvisible();
    myTouched = u->Touched();
  }

  if (subSignal == 0 || subSignal == Licq::PluginSignal::UserTyping)
    myStatusTyping = u->isTyping();


  if (subSignal == 0 || subSignal == Licq::PluginSignal::UserInfo)
  {
    myBirthday = (u->Birthday() == 0);
    myPhone = !u->getUserInfoString("PhoneNumber").empty();
    myCellular = !u->getCellularNumber().empty();
  }

  if (subSignal == 0 || subSignal == Licq::PluginSignal::UserSecurity)
  {
    mySecure = u->Secure();
    myGPGKey = !u->gpgKey().empty();
    myGPGKeyEnabled = u->UseGPG();
  }

  if (subSignal == 0 || subSignal == Licq::PluginSignal::UserSettings)
  {
    myCustomAR = !u->customAutoResponse().empty();
    myNotInList = u->NotInList();
    myNewUser = u->NewUser();
    myAwaitingAuth = u->GetAwaitingAuth();
    myInIgnoreList = u->IgnoreList();
    myInOnlineNotify = u->OnlineNotify();
    myInInvisibleList = u->InvisibleList();
    myInVisibleList = u->VisibleList();
  }

  if (myUserId.protocolId() == ICQ_PPID)
  {
    const Licq::IcqUser* icquser = dynamic_cast<const Licq::IcqUser*>(u);

    if (subSignal == 0 || subSignal == Licq::PluginSignal::UserPluginStatus)
    {
      myPhoneFollowMeStatus = icquser->phoneFollowMeStatus();
      myIcqPhoneStatus = icquser->icqPhoneStatus();
      mySharedFilesStatus = icquser->sharedFilesStatus();
    }
  }
  else
  {
    myPhoneFollowMeStatus = Licq::IcqPluginInactive;
    myIcqPhoneStatus = Licq::IcqPluginInactive;
    mySharedFilesStatus = Licq::IcqPluginInactive;
  }

  updateExtendedStatus();

  if (subSignal == 0 || subSignal == Licq::PluginSignal::UserEvents)
    updateEvents(u);

  if (subSignal == 0 || subSignal == Licq::PluginSignal::UserPicture)
    updatePicture(u);

  if (subSignal != Licq::PluginSignal::UserGroups &&
      subSignal != Licq::PluginSignal::UserPicture &&
      subSignal != Licq::PluginSignal::UserTyping &&
      subSignal != Licq::PluginSignal::UserSecurity)
  {
    if (myNotInList)
      mySubGroup = ContactListModel::NotInListSubGroup;
    else if (myStatus == User::OfflineStatus)
      mySubGroup = ContactListModel::OfflineSubGroup;
    else
      mySubGroup = ContactListModel::OnlineSubGroup;

    updateText(u);
    updateSorting();
    updateVisibility();
  }

  // Note: When we get called from constructor, noone is connected to our signals
  //       and myUserInstances is empty so below code won't trigger anything strange

  // Signal our own data changes before starting to touch groups and bars
  if (subSignal != Licq::PluginSignal::UserGroups)
    emit dataChanged(this);

  // If status has changed update the sub groups of all groups
  if (mySubGroup != oldSubGroup)
    foreach (ContactUser* user, myUserInstances)
      user->group()->updateSubGroup(oldSubGroup, mySubGroup, myEvents);

  // Update group visibility
  if (myVisibility != oldVisibility)
    foreach (ContactUser* user, myUserInstances)
      user->group()->updateVisibility(myVisibility, mySubGroup);

  // Add/remove us to/from groups
  if (subSignal == 0 || subSignal == Licq::PluginSignal::UserSettings || subSignal == Licq::PluginSignal::UserGroups)
    // Group membership is handled by ContactList so send it a signal to update
    emit updateUserGroups(this, u);
}

void ContactUserData::updatePicture(const Licq::User* u)
{
  if (myUserIcon != NULL)
  {
    delete myUserIcon;
    myUserIcon = NULL;
  }

  if (u->GetPicturePresent())
  {
    myUserIcon = new QImage(QString::fromLocal8Bit(u->pictureFileName().c_str()));
    if (myUserIcon->isNull())
    {
      delete myUserIcon;
      myUserIcon = NULL;
    }
  }
}

void ContactUserData::updateEvents(const Licq::User* u)
{
  myUrgent = false;
  myNewMessages = u->NewMessages();
  if (myEvents != myNewMessages)
  {
    foreach (ContactUser* user, myUserInstances)
      user->group()->updateNumEvents(myNewMessages - myEvents, mySubGroup);

    myEvents = myNewMessages;
  }

  myEventType = 0;

  if (myNewMessages > 0)
  {
    for (unsigned short i = 0; i < myNewMessages; i++)
    {
      switch (u->EventPeek(i)->eventType())
      {
        case Licq::UserEvent::TypeFile:
          myEventType = Licq::UserEvent::TypeFile;
          break;
        case Licq::UserEvent::TypeChat:
          if (myEventType != Licq::UserEvent::TypeFile)
            myEventType = Licq::UserEvent::TypeChat;
          break;
        case Licq::UserEvent::TypeUrl:
          if (myEventType != Licq::UserEvent::TypeFile &&
              myEventType != Licq::UserEvent::TypeChat)
            myEventType = Licq::UserEvent::TypeUrl;
          break;
        case Licq::UserEvent::TypeContactList:
          if(myEventType != Licq::UserEvent::TypeFile &&
              myEventType != Licq::UserEvent::TypeChat &&
              myEventType != Licq::UserEvent::TypeUrl)
            myEventType = Licq::UserEvent::TypeContactList;
        case Licq::UserEvent::TypeMessage:
        default:
          if (myEventType == 0)
            myEventType = Licq::UserEvent::TypeMessage;
          break;
      }
      if (u->EventPeek(i)->IsUrgent())
        myUrgent = true;
    }
  }
  Config::ContactList::FlashMode flash = Config::ContactList::instance()->flash();
  bool shouldFlash = ((myNewMessages > 0 &&  flash == Config::ContactList::FlashAll) ||
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
}

void ContactUserData::updateExtendedStatus()
{
  // Make a bitmask of everything the delegate needs to display extended icons
  myExtendedStatus = 0;

  if (myStatusInvisible)
    myExtendedStatus |= ContactListModel::InvisibleStatus;

  if (myStatusTyping)
    myExtendedStatus |= ContactListModel::TypingStatus;

  if (myPhoneFollowMeStatus == Licq::IcqPluginActive)
    myExtendedStatus |= ContactListModel::PhoneFollowMeActiveStatus;
  else if (myPhoneFollowMeStatus == Licq::IcqPluginBusy)
    myExtendedStatus |= ContactListModel::PhoneFollowMeBusyStatus;

  if (myIcqPhoneStatus == Licq::IcqPluginActive)
    myExtendedStatus |= ContactListModel::IcqPhoneActiveStatus;
  else if (myIcqPhoneStatus == Licq::IcqPluginBusy)
    myExtendedStatus |= ContactListModel::IcqPhoneBusyStatus;

  if (mySharedFilesStatus == Licq::IcqPluginActive)
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
  if (myStatus & User::OccupiedStatus)
    sort = 1;
  else if (myStatus & User::DoNotDisturbStatus)
    sort = 2;
  else if (myStatus & User::AwayStatus)
    sort = 3;
  else if (myStatus & User::NotAvailableStatus)
    sort = 4;
  else if (myStatus == User::OfflineStatus)
    sort = 5;
  else
    sort = 0;

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

bool ContactUserData::updateText(const Licq::User* licqUser)
{
  bool hasChanged = false;

  myAlias = QString::fromUtf8(licqUser->getAlias().c_str());

  for (int i = 0; i < Config::ContactList::instance()->columnCount(); i++)
  {
    QString format = Config::ContactList::instance()->columnFormat(i);
    format.replace("%a", "@_USER_ALIAS_@");
    QString newStr = QString::fromLocal8Bit(licqUser->usprintf(format.toLocal8Bit().constData()).c_str());
    newStr.replace("@_USER_ALIAS_@", myAlias);

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
  bool oldVisibility = myVisibility;

  {
    Licq::UserReadGuard u(myUserId);
    if (!u.isLocked())
      return;

    updateText(*u);
    updateSorting();
    updateVisibility();
  }

  emit dataChanged(this);

  // Update groups
  if (myVisibility != oldVisibility)
    foreach (ContactUser* user, myUserInstances)
      user->group()->updateVisibility(myVisibility, mySubGroup);
}

void ContactUserData::updateVisibility()
{
  myVisibility = false;

  // Only hide contacts who are offline
  if (myStatus != User::OfflineStatus)
    myVisibility = true;

  // Don't hide contacts with unread events
  if (myEvents > 0)
    myVisibility = true;

  // ... or the contact is in online notify list and option "Always show online notify users" is active
  if (Config::ContactList::instance()->alwaysShowONU() &&
      ((myExtendedStatus & ContactListModel::OnlineNotifyStatus) != 0))
    myVisibility = true;

  // ... or the contact is not added to the list
  if ((myExtendedStatus & ContactListModel::NotInListStatus) != 0)
    myVisibility = true;
}

bool ContactUserData::setData(const QVariant& value, int role)
{
  if (role != ContactListModel::NameRole || !value.isValid())
    return false;

  if (value.toString() == myAlias)
    return true;

  {
    Licq::UserWriteGuard u(myUserId);
    if (!u.isLocked())
      return false;

    myAlias = value.toString();
    u->SetKeepAliasOnUpdate(true);
    u->setAlias(myAlias.toUtf8().data());

    Licq::gPluginManager.pushPluginSignal(new Licq::PluginSignal(
        Licq::PluginSignal::SignalUser, Licq::PluginSignal::UserBasic, myUserId));
  }

  return true;
}

void ContactUserData::refresh()
{
  // Here we update any content that may be dynamic, for example timestamps

  bool birthday;
  bool hasChanged;
  {
    Licq::UserReadGuard u(myUserId);
    if (!u.isLocked())
      return;

    // Check if birthday icon should be updated
    birthday = (u->Birthday() == 0);
    hasChanged = updateText(*u);
  }

  if (birthday != myBirthday)
  {
    myBirthday = birthday;
    hasChanged = true;
    if (myBirthday)
      myExtendedStatus |= ContactListModel::BirthdayStatus;
    else
      myExtendedStatus &= ~ContactListModel::BirthdayStatus;
  }

  // To reduce performance impact on refreshes, keep track on
  // whether anything has changed so we don't force unnecessary updates
  if (hasChanged)
  {
    updateSorting();
    emit dataChanged(this);
  }
}

void ContactUserData::startAnimation()
{
  // Start common timer if not already running
  if (!myAnimateTimer->isActive())
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
  // Disconnect from timer and keep track of usage
  disconnect(myAnimateTimer, SIGNAL(timeout()), this, SLOT(animate()));
  myAnimatorCount--;

  // Stop animation timer if noone is using it anymore
  if (myAnimatorCount == 0)
    myAnimateTimer->stop();

  myAnimating = false;
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
  if (!myFlash && myOnlCounter == 0 && myCarCounter == 0)
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

    case ContactListModel::NameRole:
      return myAlias;

    case Qt::ToolTipRole:
      return tooltip();

    case ContactListModel::UserIdRole:
      return QVariant::fromValue(myUserId);

    case ContactListModel::AccountIdRole:
      return myUserId.accountId().c_str();

    case ContactListModel::PpidRole:
      return static_cast<unsigned int>(myUserId.protocolId());

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

    case ContactListModel::ExtendedStatusRole:
      return myExtendedStatus;

    case ContactListModel::UserIconRole:
      if (myUserIcon != NULL)
        return *myUserIcon;
      break;

    case ContactListModel::EventTypeRole:
      return myEventType;

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
  Licq::UserReadGuard u(myUserId);
  if (!u.isLocked())
    return "";

  Config::ContactList* config = Config::ContactList::instance();

  QString s = "<nobr>";
  if (config->popupPicture() && u->GetPicturePresent())
  {
    QString file = QString::fromLocal8Bit(u->pictureFileName().c_str());
    QImage picture = QImage(file);
    if (!picture.isNull())
      s += QString("<center><img src=\"%1\"></center>").arg(file);
  }

  s += User::statusToString(myStatus).c_str();

  if (config->popupAlias() && !u->getAlias().empty())
    s += "<br>" + QString::fromUtf8(u->getAlias().c_str());

  if (config->popupName())
  {
    string fullName = u->getFullName();
    if (!fullName.empty())
      s += "<br>" + QString::fromUtf8(fullName.c_str());
  }

  if (myBirthday)
    s += "<br><b>" + tr("Birthday Today!") + "</b>";

  if (myStatus != User::OfflineStatus)
  {
    if (myStatusTyping)
      s += "<br>" + tr("Typing a message");
    if (myPhoneFollowMeStatus == Licq::IcqPluginActive)
      s += "<br>" + tr("Phone &quot;Follow Me&quot;: Available");
    else if (myPhoneFollowMeStatus == Licq::IcqPluginBusy)
      s += "<br>" + tr("Phone &quot;Follow Me&quot;: Busy");

    if (myIcqPhoneStatus == Licq::IcqPluginActive)
      s += "<br>" + tr("ICQphone: Available");
    else if (myIcqPhoneStatus == Licq::IcqPluginBusy)
      s += "<br>" + tr("ICQphone: Busy");

    if (mySharedFilesStatus == Licq::IcqPluginActive)
      s += "<br>" + tr("File Server: Enabled");
  }

  if (mySecure)
    s += "<br>" + tr("Secure connection");

  if (myCustomAR)
    s += "<br>" + tr("Custom Auto Response");

  if (config->popupAuth() && u->GetAwaitingAuth())
    s += "<br>" + tr("Awaiting authorization");

  if (u->isOnline() && !u->clientInfo().empty())
    s += "<br>" + QString::fromUtf8(u->clientInfo().c_str());

  if (!u->autoResponse().empty() && myStatus & User::MessageStatuses)
    s += "<br><u>" + tr("Auto Response:") + "</u><br>&nbsp;&nbsp;&nbsp;" +
      QString::fromUtf8(u->autoResponse().c_str()).trimmed()
      .replace("\n", "<br>&nbsp;&nbsp;&nbsp;");

  if (config->popupEmail())
  {
    string email = u->getEmail();
    if (!email.empty())
      s += "<br>" + tr("E: ") + QString::fromUtf8(email.c_str());
  }

  if (config->popupPhone() && myPhone)
    s += "<br>" + tr("P: ") + QString::fromUtf8(u->getUserInfoString("PhoneNumber").c_str());

  if (config->popupCellular() && myCellular)
    s += "<br>" + tr("C: ") + QString::fromUtf8(u->getCellularNumber().c_str());

  if (config->popupFax())
  {
    string faxNumber = u->getUserInfoString("FaxNumber");
    if (!faxNumber.empty())
      s += "<br>" + tr("F: ") + QString::fromUtf8(faxNumber.c_str());
  }

  if (config->popupIP() && (u->Ip() || u->IntIp()))
  {
    char buf[32];
    Licq::ip_ntoa(u->Ip(), buf);
    s += "<br>" + tr("Ip: ") + QString::fromAscii(buf);
    if (u->IntIp() != 0 && u->IntIp() != u->Ip())
    {
      Licq::ip_ntoa(u->IntIp(), buf);
      s += " / " + QString::fromAscii(buf);
    }
  }

  if (config->popupLastOnline() && u->LastOnline() > 0)
  {
    QDateTime t;
    t.setTime_t(u->LastOnline());
    s += "<br>" + tr("O: ") + t.toString();
  }

  if (config->popupOnlineSince() && u->isOnline() && u->OnlineSince() > 0 && u->OnlineSince() <= time(0))
    s += "<br>" + tr("Logged In: ") + User::RelativeStrTime(u->OnlineSince()).c_str();

  if (config->popupAwayTime() && (myStatus & User::AwayStatuses) && u->awaySince())
    s += "<br>" + tr("Away: ") + User::RelativeStrTime(u->awaySince()).c_str();

  if (config->popupIdleTime() && u->IdleSince())
    s += "<br>" + tr("Idle: ") + User::RelativeStrTime(u->IdleSince()).c_str();

  if (config->popupLocalTime())
    s += "<br>" + tr("Local time: ") + u->usprintf("%F").c_str();

  if (config->popupID())
    s += "<br>" + tr("ID: ") + u->usprintf("%u").c_str();

  s += "</nobr>";

  return s;
}
