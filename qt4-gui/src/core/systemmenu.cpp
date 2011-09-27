/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2011 Licq developers
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

#include "systemmenu.h"

#include "config.h"

#include <boost/foreach.hpp>

#include <QList>

#include <licq/logging/log.h>
#include <licq/contactlist/group.h>
#include <licq/contactlist/owner.h>
#include <licq/contactlist/usermanager.h>
#include <licq/daemon.h>
#include <licq/icq/icq.h>
#include <licq/plugin/pluginmanager.h>

#include "config/contactlist.h"
#include "config/general.h"
#include "config/iconmanager.h"
#include "config/shortcuts.h"

#include "contactlist/contactlist.h"

#include "dialogs/addgroupdlg.h"
#include "dialogs/adduserdlg.h"
#include "dialogs/authuserdlg.h"
#include "dialogs/awaymsgdlg.h"
#include "dialogs/editgrpdlg.h"
#include "dialogs/historydlg.h"
#include "dialogs/logwindow.h"
#include "dialogs/ownermanagerdlg.h"
#include "dialogs/plugindlg.h"
#include "dialogs/randomchatdlg.h"
#include "dialogs/reqauthdlg.h"
#include "dialogs/searchuserdlg.h"
#include "dialogs/securitydlg.h"
#include "dialogs/gpgkeymanager.h"

#include "settings/settingsdlg.h"

#include "licqgui.h"
#include "mainwin.h"

const int LOG_SET_ALL = -1;
const int LOG_CLEAR_ALL = -2;
const int LOG_PACKETS = -3;

using Licq::User;
using namespace LicqQtGui;
using namespace LicqQtGui::SystemMenuPrivate;
/* TRANSLATOR LicqQtGui::SystemMenu */
/* TRANSLATOR LicqQtGui::SystemMenuPrivate::OwnerData */

SystemMenu::SystemMenu(QWidget* parent)
  : QMenu(parent)
{
  QAction* a;

  // Sub menu Debug
  myDebugMenu = new QMenu(tr("Debug Level"));
  connect(myDebugMenu, SIGNAL(triggered(QAction*)), SLOT(changeDebug(QAction*)));
  connect(myDebugMenu, SIGNAL(aboutToShow()), SLOT(aboutToShowDebugMenu()));
#define ADD_DEBUG(text, data, checkable) \
    a = myDebugMenu->addAction(text); \
    a->setCheckable(checkable); \
    a->setData(data);
  ADD_DEBUG(tr("Status Info"), Licq::Log::Info, true)
  ADD_DEBUG(tr("Unknown Packets"), Licq::Log::Unknown, true)
  ADD_DEBUG(tr("Errors"), Licq::Log::Error, true)
  ADD_DEBUG(tr("Warnings"), Licq::Log::Warning, true)
  ADD_DEBUG(tr("Debug"), Licq::Log::Debug, true)
  ADD_DEBUG(tr("Raw Packets"), LOG_PACKETS, true)
  myDebugMenu->addSeparator();
  ADD_DEBUG(tr("Set All"), LOG_SET_ALL, false)
  ADD_DEBUG(tr("Clear All"), LOG_CLEAR_ALL, false)
#undef ADD_DEBUG

  // Sub menu System Functions
  myOwnerAdmMenu = new QMenu(tr("S&ystem Functions"));
  myOwnerAdmMenu->addAction(tr("&View System Messages..."), gLicqGui, SLOT(showAllOwnerEvents()));
  myOwnerAdmMenu->addSeparator();
  myOwnerAdmSeparator = myOwnerAdmMenu->addSeparator();
  myAccountManagerAction = myOwnerAdmMenu->addAction(tr("&Account Manager..."), this, SLOT(showOwnerManagerDlg()));
  myOwnerAdmMenu->addSeparator();
  myOwnerAdmMenu->addMenu(myDebugMenu);

  // Sub menu User Functions
  myUserAdmMenu = new QMenu(tr("User &Functions"));
  myUserAdmMenu->addAction(tr("&Add User..."), this, SLOT(showAddUserDlg()));
  myAddGroupAction = myUserAdmMenu->addAction(tr("A&dd Group..."), this, SLOT(showAddGroupDlg()));
  myUserSearchAction = myUserAdmMenu->addAction(tr("S&earch for User..."), this, SLOT(showSearchUserDlg()));
  myUserAutorizeAction = myUserAdmMenu->addAction(tr("A&uthorize User..."), this, SLOT(showAuthUserDlg()));
  myUserReqAutorizeAction = myUserAdmMenu->addAction(tr("Re&quest Authorization..."), this, SLOT(showReqAuthDlg()));
  myIcqRandomChatAction = myUserAdmMenu->addAction(tr("ICQ Ra&ndom Chat..."), this, SLOT(showRandomChatSearchDlg()));
  myUserAdmMenu->addSeparator();
  myUserPopupAllAction = myUserAdmMenu->addAction(tr("&Popup All Messages..."), gLicqGui, SLOT(showAllEvents()));
  myEditGroupsAction = myUserAdmMenu->addAction(tr("Edit &Groups..."), this, SLOT(showEditGrpDlg()));
  myUserAdmMenu->addSeparator();
  myUserAdmMenu->addAction(tr("Update All Users"), this, SLOT(updateAllUsers()));
  myUserAdmMenu->addAction(tr("Update Current Group"), this, SLOT(updateAllUsersInGroup()));
  myRedrawContactListAction = myUserAdmMenu->addAction(tr("&Redraw User Window"), gGuiContactList, SLOT(reloadAll()));
  myUserAdmMenu->addAction(tr("&Save All Users"), this, SLOT(saveAllUsers()));

  // Sub menu Follow Me
  myFollowMeMenu = new QMenu(tr("ICQ Phone \"Follow Me\""));
  myFollowMeActions = new QActionGroup(this);
  connect(myFollowMeActions, SIGNAL(triggered(QAction*)), SLOT(setFollowMeStatus(QAction*)));
#define ADD_PFM(text, data) \
    a = myFollowMeActions->addAction(text); \
    a->setCheckable(true); \
    a->setData(static_cast<unsigned int>(data)); \
    myFollowMeMenu->addAction(a);
  ADD_PFM(tr("Don't Show"), CICQDaemon::IcqPluginInactive);
  ADD_PFM(tr("Available"), CICQDaemon::IcqPluginActive);
  ADD_PFM(tr("Busy"), CICQDaemon::IcqPluginBusy);
#undef ADD_PFM

  // Sub menu Status
  myStatusMenu = new QMenu(tr("&Status"));
  myStatusActions = new QActionGroup(this);
  connect(myStatusActions, SIGNAL(triggered(QAction*)), SLOT(setMainStatus(QAction*)));
  myStatusSeparator = myStatusMenu->addSeparator();
  myIcqFollowMeAction = myStatusMenu->addMenu(myFollowMeMenu);
  myIcqFollowMeSeparator = myStatusMenu->addSeparator();
#define ADD_MAINSTATUS(var, status) \
    var = myStatusActions->addAction(User::statusToString(status).c_str()); \
    var->setData(status); \
    myStatusMenu->addAction(var);
  ADD_MAINSTATUS(myStatusOnlineAction, User::OnlineStatus);
  ADD_MAINSTATUS(myStatusAwayAction, User::AwayStatus);
  ADD_MAINSTATUS(myStatusNotAvailableAction, User::NotAvailableStatus);
  ADD_MAINSTATUS(myStatusOccupiedAction, User::OccupiedStatus);
  ADD_MAINSTATUS(myStatusDoNotDisturbAction, User::DoNotDisturbStatus);
  ADD_MAINSTATUS(myStatusFreeForChatAction, User::FreeForChatStatus);
  ADD_MAINSTATUS(myStatusOfflineAction, User::OfflineStatus);
#undef ADD_MAINSTATUS
  myStatusMenu->addSeparator();
  myStatusInvisibleAction = myStatusMenu->addAction(
      User::statusToString(User::InvisibleStatus, true, false).c_str(),
      this, SLOT(toggleMainInvisibleStatus()));
  myStatusInvisibleAction->setCheckable(true);

  // Sub menu Group
  myGroupMenu = new QMenu(tr("&Group"));
  myUserGroupActions = new QActionGroup(this);
  connect(myGroupMenu, SIGNAL(aboutToShow()), SLOT(aboutToShowGroupMenu()));
  connect(myUserGroupActions, SIGNAL(triggered(QAction*)), SLOT(setCurrentGroup(QAction*)));
#define ADD_SYSTEMGROUP(group) \
    a = myUserGroupActions->addAction(ContactListModel::systemGroupName(group)); \
    a->setData(group); \
    a->setCheckable(true); \
    myGroupMenu->addAction(a);
  ADD_SYSTEMGROUP(ContactListModel::AllGroupsGroupId);
  ADD_SYSTEMGROUP(ContactListModel::MostUsersGroupId);
  myGroupMenu->addSeparator();
  myGroupSeparator = myGroupMenu->addSeparator();
  for (int i = ContactListModel::SystemGroupOffset; i <= ContactListModel::LastSystemGroup; ++i)
  {
    ADD_SYSTEMGROUP(i);
  }
#undef ADD_SYSTEMGROUP

  // Sub menu Help
  myHelpMenu = new QMenu(tr("&Help"));
  myHelpMenu->addAction(tr("&Hints..."), gMainWindow, SLOT(showHints()));
  myHelpMenu->addAction(tr("&About..."), gMainWindow, SLOT(showAboutBox()));
  myHelpMenu->addAction(tr("&Statistics..."), gMainWindow, SLOT(showStats()));

  // System menu
  addMenu(myOwnerAdmMenu);
  addMenu(myUserAdmMenu);
  addMenu(myStatusMenu);
  addMenu(myGroupMenu);
  mySetArAction = addAction(tr("Set &Auto Response..."), gMainWindow, SLOT(showAwayMsgDlg()));
  addSeparator();
  myLogWinAction = addAction(tr("&Network Window..."), gLicqGui->logWindow(), SLOT(show()));
  myMiniModeAction = addAction(tr("&Mini Mode"), Config::General::instance(), SLOT(setMiniMode(bool)));
  myMiniModeAction->setCheckable(true);
  myShowOfflineAction = addAction(tr("Show Offline &Users"), Config::ContactList::instance(), SLOT(setShowOffline(bool)));
  myShowOfflineAction->setCheckable(true);
  myShowEmptyGroupsAction = addAction(tr("Sh&ow Empty Groups"), Config::ContactList::instance(), SLOT(setShowEmptyGroups(bool)));
  myShowEmptyGroupsAction->setCheckable(true);
  myOptionsAction = addAction(tr("S&ettings..."), this, SLOT(showSettingsDlg()));
  myPluginManagerAction = addAction(tr("&Plugin Manager..."), this, SLOT(showPluginDlg()));
  myKeyManagerAction = addAction(tr("GPG &Key Manager..."), this, SLOT(showGPGKeyManager()));
  if (!Licq::gDaemon.haveGpgSupport())
    myKeyManagerAction->setVisible(false);
  addSeparator();
  mySaveOptionsAction = addAction(tr("Sa&ve Settings"), gLicqGui, SLOT(saveConfig()));
  addMenu(myHelpMenu);
  myShutdownAction = addAction(tr("E&xit"), gMainWindow, SLOT(slot_shutdown()));

  // The following shortcuts aren't shown in the menu but were
  // placed here to be groupped with other system actions.
  myPopupMessageAction = new QAction("Popup Next Message", gMainWindow);
  gMainWindow->addAction(myPopupMessageAction);
  connect(myPopupMessageAction, SIGNAL(triggered()), gLicqGui, SLOT(showNextEvent()));
  myHideMainwinAction = new QAction("Hide Mainwindow", gMainWindow);
  gMainWindow->addAction(myHideMainwinAction);
  connect(myHideMainwinAction, SIGNAL(triggered()), gMainWindow, SLOT(hide()));
  myShowHeaderAction = new QAction("Show Column Headers", gMainWindow);
  gMainWindow->addAction(myShowHeaderAction);
  connect(myShowHeaderAction, SIGNAL(triggered()), Config::ContactList::instance(), SLOT(toggleShowHeader()));

  updateGroups();
  updateIcons();
  updateShortcuts();

  connect(IconManager::instance(), SIGNAL(iconsChanged()), SLOT(updateIcons()));
  connect(Config::Shortcuts::instance(), SIGNAL(shortcutsChanged()), SLOT(updateShortcuts()));

  connect(this, SIGNAL(aboutToShow()), SLOT(aboutToShowMenu()));
  connect(myFollowMeMenu, SIGNAL(aboutToShow()), SLOT(aboutToShowFollowMeMenu()));

  // Sub menus are hidden until we got at least two owners
  myStatusSeparator->setVisible(false);

  // Hide ICQ specific menus until we actually get an ICQ owner
  setIcqEntriesVisible(false);
}

SystemMenu::~SystemMenu()
{
}

void SystemMenu::setIcqEntriesVisible(bool visible)
{
  myHasIcqOwner = visible;
  myIcqFollowMeAction->setVisible(visible);
  myIcqFollowMeSeparator->setVisible(visible);
  myIcqRandomChatAction->setVisible(visible);
}

void SystemMenu::updateIcons()
{
  IconManager* iconman = IconManager::instance();

  myKeyManagerAction->setIcon(iconman->getIcon(IconManager::GpgKeyIcon));

  myUserSearchAction->setIcon(iconman->getIcon(IconManager::SearchIcon));
  myUserAutorizeAction->setIcon(iconman->getIcon(IconManager::AuthorizeMessageIcon));
  myUserReqAutorizeAction->setIcon(iconman->getIcon(IconManager::ReqAuthorizeMessageIcon));

  myStatusOnlineAction->setIcon(iconman->iconForStatus(User::OnlineStatus));
  myStatusAwayAction->setIcon(iconman->iconForStatus(User::AwayStatus));
  myStatusNotAvailableAction->setIcon(iconman->iconForStatus(User::NotAvailableStatus));
  myStatusOccupiedAction->setIcon(iconman->iconForStatus(User::OccupiedStatus));
  myStatusDoNotDisturbAction->setIcon(iconman->iconForStatus(User::DoNotDisturbStatus));
  myStatusFreeForChatAction->setIcon(iconman->iconForStatus(User::FreeForChatStatus));
  myStatusOfflineAction->setIcon(iconman->iconForStatus(User::OfflineStatus));
  myStatusInvisibleAction->setIcon(iconman->iconForStatus(User::InvisibleStatus, Licq::UserId(), true));

  foreach (OwnerData* data, myOwnerData.values())
    data->updateIcons();
}

void SystemMenu::updateGroups()
{
  QAction* a;

  // Clear old groups but leave system groups as they never change
  foreach (a, myUserGroupActions->actions())
    if (a->data().toInt() < ContactListModel::SystemGroupOffset)
      delete a;

  Licq::GroupListGuard groupList;
  BOOST_FOREACH(const Licq::Group* group, **groupList)
  {
    Licq::GroupReadGuard pGroup(group);

    QString name = QString::fromLocal8Bit(pGroup->name().c_str());
    name.replace("&", "&&");

    a = myUserGroupActions->addAction(name);
    a->setData(pGroup->id());
    a->setCheckable(true);

    myGroupMenu->insertAction(myGroupSeparator, a);
  }
}

void SystemMenu::updateShortcuts()
{
  Config::Shortcuts* shortcuts = Config::Shortcuts::instance();

  mySetArAction->setShortcut(shortcuts->getShortcut(Config::Shortcuts::MainwinSetAutoResponse));
  myLogWinAction->setShortcut(shortcuts->getShortcut(Config::Shortcuts::MainwinNetworkLog));
  myMiniModeAction->setShortcut(shortcuts->getShortcut(Config::Shortcuts::MainwinToggleMiniMode));
  myShowOfflineAction->setShortcut(shortcuts->getShortcut(Config::Shortcuts::MainwinToggleShowOffline));
  myShowEmptyGroupsAction->setShortcut(shortcuts->getShortcut(Config::Shortcuts::MainwinToggleEmptyGroups));
  myOptionsAction->setShortcut(shortcuts->getShortcut(Config::Shortcuts::MainwinSettings));
  myShutdownAction->setShortcut(shortcuts->getShortcut(Config::Shortcuts::MainwinExit));
  myHideMainwinAction->setShortcut(shortcuts->getShortcut(Config::Shortcuts::MainwinHide));

  myStatusOnlineAction->setShortcut(shortcuts->getShortcut(Config::Shortcuts::MainwinStatusOnline));
  myStatusAwayAction->setShortcut(shortcuts->getShortcut(Config::Shortcuts::MainwinStatusAway));
  myStatusNotAvailableAction->setShortcut(shortcuts->getShortcut(Config::Shortcuts::MainwinStatusNotAvailable));
  myStatusOccupiedAction->setShortcut(shortcuts->getShortcut(Config::Shortcuts::MainwinStatusOccupied));
  myStatusDoNotDisturbAction->setShortcut(shortcuts->getShortcut(Config::Shortcuts::MainwinStatusDoNotDisturb));
  myStatusFreeForChatAction->setShortcut(shortcuts->getShortcut(Config::Shortcuts::MainwinStatusFreeForChat));
  myStatusOfflineAction->setShortcut(shortcuts->getShortcut(Config::Shortcuts::MainwinStatusOffline));
  myStatusInvisibleAction->setShortcut(shortcuts->getShortcut(Config::Shortcuts::MainwinStatusInvisible));

  myAccountManagerAction->setShortcut(shortcuts->getShortcut(Config::Shortcuts::MainwinAccountManager));
  myAddGroupAction->setShortcut(shortcuts->getShortcut(Config::Shortcuts::MainwinAddGroup));
  myPopupMessageAction->setShortcut(shortcuts->getShortcut(Config::Shortcuts::MainwinPopupMessage));
  myUserPopupAllAction->setShortcut(shortcuts->getShortcut(Config::Shortcuts::MainwinPopupAllMessages));
  myEditGroupsAction->setShortcut(shortcuts->getShortcut(Config::Shortcuts::MainwinEditGroups));
  myRedrawContactListAction->setShortcut(shortcuts->getShortcut(Config::Shortcuts::MainwinRedrawContactList));
  myShowHeaderAction->setShortcut(shortcuts->getShortcut(Config::Shortcuts::MainwinToggleShowHeader));
}

void SystemMenu::addOwner(const Licq::UserId& userId)
{
  if (myOwnerData.count(userId) > 0)
    return;

  // Make we actually have a plugin protocol loaded for the owner,
  //   otherwise there is no point in including it in the menus.
  unsigned long ppid = userId.protocolId();
  Licq::ProtocolPlugin::Ptr protocol = Licq::gPluginManager.getProtocolPlugin(ppid);
  if (protocol.get() == NULL)
    return;

  OwnerData* newOwner = new OwnerData(ppid, protocol->name().c_str(),
      protocol->capabilities(), this);
  QMenu* ownerAdmin = newOwner->getOwnerAdmMenu();
  QMenu* ownerStatus = newOwner->getStatusMenu();
  myOwnerAdmMenu->insertMenu(myOwnerAdmSeparator, ownerAdmin);
  myStatusMenu->insertMenu(myStatusSeparator, ownerStatus);

  if (myOwnerData.size() < 1)
  {
    // There are no other owners, hide the sub menus
    ownerStatus->menuAction()->setVisible(false);
    ownerAdmin->menuAction()->setVisible(false);

    foreach (QAction* a, ownerAdmin->actions())
      myOwnerAdmMenu->insertAction(myOwnerAdmSeparator, a);
  }

  if (myOwnerData.size() == 1)
  {
    // Adding the second owner, show the sub menus for the first owner as well
    OwnerData* firstOwner = myOwnerData.begin().value();
    firstOwner->getStatusMenu()->menuAction()->setVisible(true);
    myStatusSeparator->setVisible(true);

    QMenu* firstOwnerAdm = firstOwner->getOwnerAdmMenu();
    firstOwnerAdm->menuAction()->setVisible(true);
    foreach (QAction* a, firstOwnerAdm->actions())
      myOwnerAdmMenu->removeAction(a);
  }

  if (ppid == LICQ_PPID)
    setIcqEntriesVisible(true);

  myOwnerData.insert(userId, newOwner);
}

void SystemMenu::removeOwner(const Licq::UserId& userId)
{
  OwnerData* data = myOwnerData.take(userId);
  if (data == NULL)
    return;

  delete data;

  unsigned long ppid = userId.protocolId();
  if (ppid == LICQ_PPID)
    setIcqEntriesVisible(false);

  if (myOwnerData.size() == 1)
  {
    // Only one owner left, hide the sub menus
    OwnerData* lastOwner = myOwnerData.begin().value();
    lastOwner->getStatusMenu()->menuAction()->setVisible(false);
    myStatusSeparator->setVisible(false);

    QMenu* lastOwnerAdm = lastOwner->getOwnerAdmMenu();
    lastOwnerAdm->menuAction()->setVisible(false);
    foreach (QAction* a, lastOwnerAdm->actions())
      myOwnerAdmMenu->insertAction(myOwnerAdmSeparator, a);
  }
}

bool SystemMenu::getInvisibleStatus(const Licq::UserId& userId) const
{
  OwnerData* data = myOwnerData.value(userId);
  if (data == NULL)
    return getInvisibleStatus();

  return data->getInvisibleStatus();
}

void SystemMenu::aboutToShowMenu()
{
  myMiniModeAction->setChecked(Config::General::instance()->miniMode());
  myShowOfflineAction->setChecked(Config::ContactList::instance()->showOffline());
  myShowEmptyGroupsAction->setChecked(Config::ContactList::instance()->showEmptyGroups());
}

void SystemMenu::aboutToShowFollowMeMenu()
{
  Licq::OwnerReadGuard o(LICQ_PPID);
  if (!o.isLocked())
    return;

  int status = o->phoneFollowMeStatus();

  foreach (QAction* a, myFollowMeActions->actions())
    if (a->data().toInt() == status)
      a->setChecked(true);
}

void SystemMenu::aboutToShowGroupMenu()
{
  int gid = Config::ContactList::instance()->groupId();

  foreach (QAction* a, myUserGroupActions->actions())
    if (a->data().toInt() == gid)
      a->setChecked(true);
}

void SystemMenu::aboutToShowDebugMenu()
{
  using Licq::Log;

  Licq::PluginLogSink::Ptr sink = gLicqGui->logWindow()->pluginLogSink();

  foreach (QAction* action, myDebugMenu->actions())
  {
    if (action->isCheckable())
    {
      if (action->data().toInt() == LOG_PACKETS)
        action->setChecked(sink->isLoggingPackets());
      else
      {
        Log::Level level = static_cast<Log::Level>(action->data().toInt());
        action->setChecked(sink->isLogging(level));
      }
    }
  }
}

void SystemMenu::changeDebug(QAction* action)
{
  Licq::PluginLogSink::Ptr sink = gLicqGui->logWindow()->pluginLogSink();

  const int data = action->data().toInt();
  if (data == LOG_SET_ALL || data == LOG_CLEAR_ALL)
  {
    const bool enable = data == LOG_SET_ALL;
    sink->setAllLogLevels(enable);
    sink->setLogPackets(enable);
  }
  else if (data == LOG_PACKETS)
  {
    sink->setLogPackets(action->isChecked());
  }
  else
  {
    Licq::Log::Level level = static_cast<Licq::Log::Level>(data);
    sink->setLogLevel(level, action->isChecked());
  }
}

void SystemMenu::setCurrentGroup(QAction* action)
{
  int id = action->data().toInt();

  Config::ContactList::instance()->setGroup(id);
}

void SystemMenu::setFollowMeStatus(QAction* action)
{
  int id = action->data().toUInt();

  gLicqDaemon->icqSetPhoneFollowMeStatus(id);
}

void SystemMenu::setMainStatus(QAction* action)
{
  unsigned status = action->data().toUInt();
  bool withMsg = false;
  if (status & User::MessageStatuses)
  {
    // Only popup away message dialog if we have at least one owner with away message support
    foreach (OwnerData* data, myOwnerData.values())
      if (data->useAwayMessage())
        withMsg = true;
  }
  bool invisible = (myStatusInvisibleAction != NULL && myStatusInvisibleAction->isChecked());

  if (invisible)
    status |= User::InvisibleStatus;

  if (withMsg)
    AwayMsgDlg::showAwayMsgDlg(status, true, 0);
  else
    gLicqGui->changeStatus(status, invisible);
}

void SystemMenu::toggleMainInvisibleStatus()
{
  gLicqGui->changeStatus(User::InvisibleStatus, myStatusInvisibleAction->isChecked());
}

void SystemMenu::updateAllUsers()
{
  gLicqDaemon->UpdateAllUsers();
}

void SystemMenu::updateAllUsersInGroup()
{
  int groupId = Config::ContactList::instance()->groupId();

  if (groupId < ContactListModel::SystemGroupOffset)
    gLicqDaemon->updateAllUsersInGroup(groupId);
  // TODO: Not implemented for system groups
}

void SystemMenu::saveAllUsers()
{
   Licq::gUserManager.SaveAllUsers();
}

void SystemMenu::showOwnerManagerDlg()
{
  OwnerManagerDlg::showOwnerManagerDlg();
}

void SystemMenu::showAddUserDlg()
{
  new AddUserDlg();
}

void SystemMenu::showAddGroupDlg()
{
  new AddGroupDlg();
}

void SystemMenu::showSearchUserDlg()
{
  new SearchUserDlg();
}

void SystemMenu::showAuthUserDlg()
{
  new AuthUserDlg(Licq::UserId(), true);
}

void SystemMenu::showReqAuthDlg()
{
  new ReqAuthDlg();
}

void SystemMenu::showEditGrpDlg()
{
  new EditGrpDlg();
}

void SystemMenu::showRandomChatSearchDlg()
{
  new RandomChatDlg();
}

void SystemMenu::showSettingsDlg()
{
  SettingsDlg::show();
}

void SystemMenu::showPluginDlg()
{
  PluginDlg::showPluginDlg();
}

void SystemMenu::showGPGKeyManager()
{
  new GPGKeyManager();
}


OwnerData::OwnerData(unsigned long ppid, const QString& protoName,
    unsigned long sendFunctions, SystemMenu* parent)
  : QObject(parent),
    myPpid(ppid)
{
  myUserId = Licq::gUserManager.ownerUserId(ppid);
  myUseAwayMessage = ((sendFunctions & Licq::ProtocolPlugin::CanHoldStatusMsg) != 0);

  // System sub menu
  myOwnerAdmMenu = new QMenu(protoName);
  myOwnerAdmInfoAction = myOwnerAdmMenu->addAction(tr("&Info..."), this, SLOT(viewInfo()));
  myOwnerAdmHistoryAction = myOwnerAdmMenu->addAction(tr("View &History..."), this, SLOT(viewHistory()));
  if (ppid == LICQ_PPID)
  {
    myOwnerAdmMenu->addAction(tr("&Security Options..."), this, SLOT(showSecurityDlg()));
    myOwnerAdmMenu->addAction(tr("&Random Chat Group..."), this, SLOT(showRandomChatGroupDlg()));
  }

  // Status sub menu
  myStatusMenu = new QMenu(protoName);
  myStatusActions = new QActionGroup(this);
  connect(myStatusActions, SIGNAL(triggered(QAction*)), SLOT(setStatus(QAction*)));
 #define ADD_STATUS(var, status, cond) \
    if (cond) \
    { \
      var = myStatusActions->addAction(User::statusToString(status).c_str()); \
      var->setData(status); \
      var->setCheckable(true); \
      myStatusMenu->addAction(var); \
    } \
    else \
    { \
      var = NULL; \
    }
  ADD_STATUS(myStatusOnlineAction, User::OnlineStatus, true);
  ADD_STATUS(myStatusAwayAction, User::AwayStatus, true);
  ADD_STATUS(myStatusNotAvailableAction, User::NotAvailableStatus, myPpid != MSN_PPID);
  ADD_STATUS(myStatusOccupiedAction, User::OccupiedStatus, myPpid != JABBER_PPID);
  ADD_STATUS(myStatusDoNotDisturbAction, User::DoNotDisturbStatus, myPpid != MSN_PPID);
  ADD_STATUS(myStatusFreeForChatAction, User::FreeForChatStatus, myPpid != MSN_PPID);
  ADD_STATUS(myStatusOfflineAction, User::OfflineStatus, true);
  if (myPpid != JABBER_PPID)
  {
    myStatusMenu->addSeparator();
    myStatusInvisibleAction = myStatusMenu->addAction(
        User::statusToString(User::InvisibleStatus, true, false).c_str(),
        this, SLOT(toggleInvisibleStatus()));
    myStatusInvisibleAction->setCheckable(true);
  }
  else
  {
    myStatusInvisibleAction = NULL;
  }
#undef ADD_STATUS

  connect(myStatusMenu, SIGNAL(aboutToShow()), SLOT(aboutToShowStatusMenu()));

  updateIcons();
}

OwnerData::~OwnerData()
{
  delete myOwnerAdmMenu;
  delete myStatusMenu;
}

void OwnerData::updateIcons()
{
  IconManager* iconman = IconManager::instance();

  myOwnerAdmInfoAction->setIcon(iconman->getIcon(IconManager::InfoIcon));
  myOwnerAdmHistoryAction->setIcon(iconman->getIcon(IconManager::HistoryIcon));

#define SET_ICON(action, status) \
  if (action != NULL) \
    action->setIcon(iconman->iconForStatus(status, myUserId, true))

  SET_ICON(myStatusOnlineAction, User::OnlineStatus);
  SET_ICON(myStatusAwayAction, User::AwayStatus);
  SET_ICON(myStatusNotAvailableAction, User::NotAvailableStatus);
  SET_ICON(myStatusOccupiedAction, User::OccupiedStatus);
  SET_ICON(myStatusDoNotDisturbAction, User::DoNotDisturbStatus);
  SET_ICON(myStatusFreeForChatAction, User::FreeForChatStatus);
  SET_ICON(myStatusOfflineAction, User::OfflineStatus);
  SET_ICON(myStatusInvisibleAction, User::InvisibleStatus);
#undef SET_ICON
}

void OwnerData::aboutToShowStatusMenu()
{
  Licq::OwnerReadGuard o(myPpid);
  if (!o.isLocked())
    return;

  unsigned status = o->status();

  // Update protocol status
  foreach (QAction* a, myStatusActions->actions())
  {
    unsigned s = a->data().toUInt();
    if (status == s || status & s)
      a->setChecked(true);
  }

  if (myStatusInvisibleAction != NULL && status != User::OfflineStatus)
    myStatusInvisibleAction->setChecked(o->isInvisible());
}

void OwnerData::viewInfo()
{
  gLicqGui->showInfoDialog(mnuUserGeneral, myUserId);
}

void OwnerData::viewHistory()
{
  new HistoryDlg(myUserId);
}

void OwnerData::showSecurityDlg()
{
  new SecurityDlg();
}

void OwnerData::showRandomChatGroupDlg()
{
  new SetRandomChatGroupDlg();
}

void OwnerData::setStatus(QAction* action)
{
  unsigned status = action->data().toUInt();
  bool withMsg = (myUseAwayMessage && status & User::MessageStatuses);
  bool invisible = (myStatusInvisibleAction != NULL && myStatusInvisibleAction->isChecked());

  if (invisible)
    status |= User::InvisibleStatus;

  if (withMsg)
    AwayMsgDlg::showAwayMsgDlg(status, true, myPpid);
  else
    gLicqGui->changeStatus(status, myUserId, invisible);
}

void OwnerData::toggleInvisibleStatus()
{
  gLicqGui->changeStatus(User::InvisibleStatus, myUserId, myStatusInvisibleAction->isChecked());
}
