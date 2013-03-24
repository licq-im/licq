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

#include "systemmenu.h"

#include "config.h"

#include <boost/foreach.hpp>

#include <QList>

#include <licq/contactlist/group.h>
#include <licq/contactlist/owner.h>
#include <licq/contactlist/usermanager.h>
#include <licq/daemon.h>
#include <licq/icq/icq.h>
#include <licq/icq/owner.h>
#include <licq/plugin/pluginmanager.h>

#include "config/contactlist.h"
#include "config/general.h"
#include "config/iconmanager.h"
#include "config/shortcuts.h"

#include "contactlist/contactlist.h"

#include "dialogs/addgroupdlg.h"
#include "dialogs/adduserdlg.h"
#include "dialogs/authdlg.h"
#include "dialogs/awaymsgdlg.h"
#include "dialogs/editgrpdlg.h"
#include "dialogs/historydlg.h"
#include "dialogs/logwindow.h"
#include "dialogs/ownermanagerdlg.h"
#include "dialogs/randomchatdlg.h"
#include "dialogs/searchuserdlg.h"
#include "dialogs/gpgkeymanager.h"

#include "settings/settingsdlg.h"

#include "userdlg/userdlg.h"

#include "licqgui.h"
#include "mainwin.h"
#include "signalmanager.h"

using Licq::User;
using namespace LicqQtGui;
using namespace LicqQtGui::SystemMenuPrivate;
/* TRANSLATOR LicqQtGui::SystemMenu */
/* TRANSLATOR LicqQtGui::SystemMenuPrivate::OwnerData */

SystemMenu::SystemMenu(QWidget* parent)
  : QMenu(parent)
{
  QAction* a;

  // Sub menu System Functions
  myOwnerAdmMenu = new QMenu(tr("S&ystem Functions"), this);
  myOwnerAdmMenu->addAction(tr("&View System Messages..."), gLicqGui, SLOT(showAllOwnerEvents()));
  myOwnerAdmMenu->addSeparator();

  // Sub menu User Functions
  myUserAdmMenu = new QMenu(tr("User &Functions"), this);
  myUserAdmMenu->addAction(tr("&Add User..."), this, SLOT(showAddUserDlg()));
  myAddGroupAction = myUserAdmMenu->addAction(tr("A&dd Group..."), this, SLOT(showAddGroupDlg()));
  myUserAutorizeAction = myUserAdmMenu->addAction(tr("A&uthorize User..."), this, SLOT(showAuthUserDlg()));
  myUserReqAutorizeAction = myUserAdmMenu->addAction(tr("Re&quest Authorization..."), this, SLOT(showReqAuthDlg()));
  myUserAdmActionSeparator = myUserAdmMenu->addSeparator();
  myUserPopupAllAction = myUserAdmMenu->addAction(tr("&Popup All Messages..."), gLicqGui, SLOT(showAllEvents()));
  myEditGroupsAction = myUserAdmMenu->addAction(tr("Edit &Groups..."), this, SLOT(showEditGrpDlg()));
  myUserAdmMenu->addSeparator();
  myUserAdmMenu->addAction(tr("Update All Users"), this, SLOT(updateAllUsers()));
  myUserAdmMenu->addAction(tr("Update Current Group"), this, SLOT(updateAllUsersInGroup()));
  myRedrawContactListAction = myUserAdmMenu->addAction(tr("&Redraw User Window"), gGuiContactList, SLOT(reloadAll()));
  myUserAdmMenu->addAction(tr("&Save All Users"), this, SLOT(saveAllUsers()));

  // Sub menu Status
  myStatusMenu = new QMenu(tr("&Status"), this);
  myStatusActions = new QActionGroup(this);
  connect(myStatusActions, SIGNAL(triggered(QAction*)), SLOT(setMainStatus(QAction*)));
  myStatusSeparator = myStatusMenu->addSeparator();
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
  myGroupMenu = new QMenu(tr("&Group"), this);
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
  myHelpMenu = new QMenu(tr("&Help"), this);
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
  myAccountManagerAction = addAction(tr("&Accounts..."), this, SLOT(showOwnerManagerDlg()));
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

  // Sub menus are hidden until we got at least two owners
  myStatusSeparator->setVisible(false);

  connect(gGuiSignalManager, SIGNAL(ownerAdded(const Licq::UserId&)),
      SLOT(addOwner(const Licq::UserId&)));
  connect(gGuiSignalManager, SIGNAL(ownerRemoved(const Licq::UserId&)),
      SLOT(removeOwner(const Licq::UserId&)));

  {
    Licq::OwnerListGuard ownerList;
    BOOST_FOREACH(const Licq::Owner* owner, **ownerList)
      addOwner(owner->id());
  }
}

SystemMenu::~SystemMenu()
{
}

void SystemMenu::updateIcons()
{
  IconManager* iconman = IconManager::instance();

  myKeyManagerAction->setIcon(iconman->getIcon(IconManager::GpgKeyIcon));

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

  OwnerData* newOwner = new OwnerData(userId, protocol->name().c_str(),
      protocol->capabilities(), this);
  QMenu* ownerAdmin = newOwner->getOwnerAdmMenu();
  QMenu* ownerStatus = newOwner->getStatusMenu();
  myOwnerAdmMenu->addMenu(ownerAdmin);
  myStatusMenu->insertMenu(myStatusSeparator, ownerStatus);
  if (ppid == ICQ_PPID)
  {
    myStatusMenu->insertMenu(myIcqFollowMeSeparator, newOwner->getIcqFollowMeMenu());
    myUserAdmMenu->insertAction(myUserAdmActionSeparator, newOwner->getIcqUserSearchAction());
    myUserAdmMenu->insertAction(myUserAdmActionSeparator, newOwner->getIcqRandomChatAction());
  }

  if (myOwnerData.size() < 1)
  {
    // There are no other owners, hide the sub menus
    ownerStatus->menuAction()->setVisible(false);
    ownerAdmin->menuAction()->setVisible(false);

    foreach (QAction* a, ownerAdmin->actions())
      myOwnerAdmMenu->addAction(a);
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

  myOwnerData.insert(userId, newOwner);
}

void SystemMenu::removeOwner(const Licq::UserId& userId)
{
  OwnerData* data = myOwnerData.take(userId);
  if (data == NULL)
    return;

  delete data;

  if (myOwnerData.size() == 1)
  {
    // Only one owner left, hide the sub menus
    OwnerData* lastOwner = myOwnerData.begin().value();
    lastOwner->getStatusMenu()->menuAction()->setVisible(false);
    myStatusSeparator->setVisible(false);

    QMenu* lastOwnerAdm = lastOwner->getOwnerAdmMenu();
    lastOwnerAdm->menuAction()->setVisible(false);
    foreach (QAction* a, lastOwnerAdm->actions())
      myOwnerAdmMenu->addAction(a);
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

void SystemMenu::aboutToShowGroupMenu()
{
  int gid = Config::ContactList::instance()->groupId();

  foreach (QAction* a, myUserGroupActions->actions())
    if (a->data().toInt() == gid)
      a->setChecked(true);
}

void SystemMenu::setCurrentGroup(QAction* action)
{
  int id = action->data().toInt();

  Config::ContactList::instance()->setGroup(id);
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
    AwayMsgDlg::showAwayMsgDlg(status, true);
  else
    gLicqGui->changeStatus(status, invisible);
}

void SystemMenu::toggleMainInvisibleStatus()
{
  gLicqGui->changeStatus(User::InvisibleStatus, myStatusInvisibleAction->isChecked());
}

void SystemMenu::updateAllUsers()
{
  Licq::OwnerListGuard ownerList;
  BOOST_FOREACH(const Licq::Owner* owner, **ownerList)
  {
    const Licq::UserId& ownerId(owner->id());

    if (ownerId.protocolId() == ICQ_PPID)
    {
      Licq::IcqProtocol::Ptr icq = plugin_internal_cast<Licq::IcqProtocol>(
          Licq::gPluginManager.getProtocolInstance(ownerId));
      if (icq != NULL)
        icq->updateAllUsersInGroup(ownerId, 0);
    }
  }
}

void SystemMenu::updateAllUsersInGroup()
{
  int groupId = Config::ContactList::instance()->groupId();

  // TODO: Not implemented for system groups
  if (groupId >= ContactListModel::SystemGroupOffset)
    return;

  Licq::OwnerListGuard ownerList;
  BOOST_FOREACH(const Licq::Owner* owner, **ownerList)
  {
    const Licq::UserId& ownerId(owner->id());

    if (ownerId.protocolId() == ICQ_PPID)
    {
      Licq::IcqProtocol::Ptr icq = plugin_internal_cast<Licq::IcqProtocol>(
          Licq::gPluginManager.getProtocolInstance(ownerId));
      if (icq != NULL)
        icq->updateAllUsersInGroup(ownerId, groupId);
    }
  }
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

void SystemMenu::showAuthUserDlg()
{
  new AuthDlg(AuthDlg::GrantAuth);
}

void SystemMenu::showReqAuthDlg()
{
  new AuthDlg(AuthDlg::RequestAuth);
}

void SystemMenu::showEditGrpDlg()
{
  new EditGrpDlg();
}

void SystemMenu::showSettingsDlg()
{
  SettingsDlg::show();
}

void SystemMenu::showGPGKeyManager()
{
  new GPGKeyManager();
}


OwnerData::OwnerData(const Licq::UserId& userId, const QString& protoName,
    unsigned long sendFunctions, SystemMenu* parent)
  : QObject(parent),
    myUserId(userId)
{
  unsigned long myPpid = userId.protocolId();
  myUseAwayMessage = ((sendFunctions & Licq::ProtocolPlugin::CanHoldStatusMsg) != 0);

  QString ownerCaption = QString("%1 (%2)").arg(protoName).arg(userId.accountId().c_str());

  // System sub menu
  myOwnerAdmMenu = new QMenu(ownerCaption);
  myOwnerAdmInfoAction = myOwnerAdmMenu->addAction(tr("&Info..."), this, SLOT(viewInfo()));
  myOwnerAdmHistoryAction = myOwnerAdmMenu->addAction(tr("View &History..."), this, SLOT(viewHistory()));
  myOwnerAdmSettingsAction = myOwnerAdmMenu->addAction(tr("&Settings..."), this, SLOT(showSettingsDlg()));

  // Status sub menu
  myStatusMenu = new QMenu(ownerCaption);
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

  if (myUserId.protocolId() == ICQ_PPID)
  {
    // Sub menu Follow Me
    myIcqFollowMeMenu = new QMenu(tr("ICQ Phone \"Follow Me\""));
    myIcqFollowMeActions = new QActionGroup(this);
    connect(myIcqFollowMeActions, SIGNAL(triggered(QAction*)), SLOT(setIcqFollowMeStatus(QAction*)));
    QAction* a;
#define ADD_PFM(text, data) \
    a = myIcqFollowMeActions->addAction(text); \
    a->setCheckable(true); \
    a->setData(static_cast<unsigned int>(data)); \
    myIcqFollowMeMenu->addAction(a);
    ADD_PFM(tr("Don't Show"), Licq::IcqPluginInactive);
    ADD_PFM(tr("Available"), Licq::IcqPluginActive);
    ADD_PFM(tr("Busy"), Licq::IcqPluginBusy);
#undef ADD_PFM

    connect(myIcqFollowMeMenu, SIGNAL(aboutToShow()), SLOT(aboutToShowIcqFollowMeMenu()));

    myIcqUserSearchAction = new QAction(tr("ICQ User S&earch..."), this);
    connect(myIcqUserSearchAction, SIGNAL(triggered()), SLOT(showIcqUserSearchDlg()));

    myIcqRandomChatAction = new QAction(tr("ICQ Ra&ndom Chat..."), this);
    connect(myIcqRandomChatAction, SIGNAL(triggered()), SLOT(showIcqRandomChatSearchDlg()));
  }
  else
  {
    myIcqFollowMeMenu = NULL;
    myIcqFollowMeActions = NULL;
    myIcqUserSearchAction = NULL;
    myIcqRandomChatAction = NULL;
  }

  connect(myStatusMenu, SIGNAL(aboutToShow()), SLOT(aboutToShowStatusMenu()));

  updateIcons();
}

OwnerData::~OwnerData()
{
  if (myIcqFollowMeMenu != NULL)
    delete myIcqFollowMeMenu;
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

  if (myIcqUserSearchAction != NULL)
    myIcqUserSearchAction->setIcon(iconman->getIcon(IconManager::SearchIcon));
}

void OwnerData::aboutToShowStatusMenu()
{
  Licq::OwnerReadGuard o(myUserId);
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

void OwnerData::aboutToShowIcqFollowMeMenu()
{
  Licq::IcqOwnerReadGuard o(myUserId);
  if (!o.isLocked())
    return;

  int status = o->phoneFollowMeStatus();

  foreach (QAction* a, myIcqFollowMeActions->actions())
    if (a->data().toInt() == status)
      a->setChecked(true);
}

void OwnerData::viewInfo()
{
  UserDlg::showDialog(myUserId, UserDlg::GeneralPage);
}

void OwnerData::showSettingsDlg()
{
  UserDlg::showDialog(myUserId, UserDlg::OwnerPage);
}

void OwnerData::viewHistory()
{
  new HistoryDlg(myUserId);
}

void OwnerData::setStatus(QAction* action)
{
  unsigned status = action->data().toUInt();
  bool withMsg = (myUseAwayMessage && status & User::MessageStatuses);
  bool invisible = (myStatusInvisibleAction != NULL && myStatusInvisibleAction->isChecked());

  if (invisible)
    status |= User::InvisibleStatus;

  if (withMsg)
    AwayMsgDlg::showAwayMsgDlg(status, true, myUserId);
  else
    gLicqGui->changeStatus(status, myUserId, invisible);
}

void OwnerData::toggleInvisibleStatus()
{
  gLicqGui->changeStatus(User::InvisibleStatus, myUserId, myStatusInvisibleAction->isChecked());
}

void OwnerData::setIcqFollowMeStatus(QAction* action)
{
  int id = action->data().toUInt();

  Licq::IcqProtocol::Ptr icq = plugin_internal_cast<Licq::IcqProtocol>(
      Licq::gPluginManager.getProtocolInstance(myUserId));
  if (!icq)
    return;
  icq->icqSetPhoneFollowMeStatus(myUserId, id);
}

void OwnerData::showIcqUserSearchDlg()
{
  new SearchUserDlg(myUserId);
}

void OwnerData::showIcqRandomChatSearchDlg()
{
  new RandomChatDlg(myUserId);
}
