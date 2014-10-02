/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2012 Licq developers <licq-dev@googlegroups.com>
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

#include "events.h"

#include "config.h"

#include <boost/foreach.hpp>

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <licq/contactlist/owner.h>
#include <licq/daemon.h>
#include <licq/filter.h>
#include <licq/plugin/pluginmanager.h>
#include <licq/oneventmanager.h>
#include <licq/userevents.h>

#include "config/chat.h"
#include "config/contactlist.h"
#include "config/general.h"
#include "config/shortcuts.h"
#include "dialogs/filterruledlg.h"
#include "dialogs/hintsdlg.h"
#include "widgets/shortcutedit.h"

#include "oneventbox.h"
#include "settingsdlg.h"


using Licq::OnEventData;
using Licq::gOnEventManager;
using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::Settings::Events */

Settings::Events::Events(SettingsDlg* parent)
  : QObject(parent)
{
  parent->addPage(SettingsDlg::OnEventPage, createPageOnEvent(parent),
      tr("Events"));
  parent->addPage(SettingsDlg::SoundsPage, createPageSounds(parent),
      tr("Sounds"), SettingsDlg::OnEventPage);
  parent->addPage(SettingsDlg::FilterPage, createPageFilter(parent),
      tr("Filter"), SettingsDlg::OnEventPage);

  load();
}

QWidget* Settings::Events::createPageOnEvent(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  myPageOnEventLayout = new QVBoxLayout(w);
  myPageOnEventLayout->setContentsMargins(0, 0, 0, 0);

  myNewMsgActionsBox = new QGroupBox(tr("Actions on Incoming Messages"));
  myMsgActionsLayout = new QGridLayout(myNewMsgActionsBox);

  myBoldOnMsgCheck = new QCheckBox(tr("Bold message label"));
  myBoldOnMsgCheck->setToolTip(tr("Show the message info label in bold font if there are incoming messages"));
  myMsgActionsLayout->addWidget(myBoldOnMsgCheck, 0, 0);

  myAutoFocusCheck = new QCheckBox(tr("Auto-focus message"));
  myAutoFocusCheck->setToolTip(tr("Automatically focus opened message windows."));
  myMsgActionsLayout->addWidget(myAutoFocusCheck, 1, 0);

  myAutoRaiseCheck = new QCheckBox(tr("Auto-raise main window"));
  myAutoRaiseCheck->setToolTip(tr("Raise the main window on incoming messages"));
  myMsgActionsLayout->addWidget(myAutoRaiseCheck, 2, 0);

  QHBoxLayout* autoPopupLayout = new QHBoxLayout();
  QLabel* autoPopupLabel = new QLabel(tr("Auto-popup message:"));
  autoPopupLayout->addWidget(autoPopupLabel);
  myAutoPopupCombo = new QComboBox();
  myAutoPopupCombo->addItem(tr("Never"));
  myAutoPopupCombo->addItem(tr("Only when online"));
  myAutoPopupCombo->addItem(tr("When online or away"));
  myAutoPopupCombo->addItem(tr("When online, away or N/A"));
  myAutoPopupCombo->addItem(tr("Always except DND"));
  myAutoPopupCombo->addItem(tr("Always"));
  myAutoPopupCombo->setToolTip(tr("Select for which statuses incoming messages should "
      "open automatically.\nOnline also includes Free for chat."));
  autoPopupLabel->setBuddy(myAutoPopupCombo);
  autoPopupLayout->addWidget(myAutoPopupCombo);
  myMsgActionsLayout->addLayout(autoPopupLayout, 3, 0);

  myAutoUrgentCheck = new QCheckBox(tr("Auto-popup urgent only"));
  myAutoUrgentCheck->setToolTip(tr("Only auto-popup urgent messages."));
  myMsgActionsLayout->addWidget(myAutoUrgentCheck, 4, 0);

  myFlashTaskbarCheck = new QCheckBox(tr("Flash taskbar"));
  myFlashTaskbarCheck->setToolTip(tr("Flash the taskbar on incoming messages"));
  myMsgActionsLayout->addWidget(myFlashTaskbarCheck, 0, 1);

  myFlashAllCheck = new QCheckBox(tr("Blink all events"));
  myFlashAllCheck->setToolTip(tr("All incoming events will blink"));
  myMsgActionsLayout->addWidget(myFlashAllCheck, 1, 1);

  myFlashUrgentCheck = new QCheckBox(tr("Blink urgent events"));
  myFlashUrgentCheck->setToolTip(tr("Only urgent events will blink"));
  myMsgActionsLayout->addWidget(myFlashUrgentCheck, 2, 1);

#ifdef Q_WS_X11
  QHBoxLayout* hotKeyLayout = new QHBoxLayout();
  myHotKeyLabel = new QLabel(tr("Hot key:"));
  hotKeyLayout->addWidget(myHotKeyLabel);
  hotKeyLayout->addStretch();
  myHotKeyLabel->setToolTip(tr("Hotkey to pop up the next pending message.\n"
      "Enter the hotkey literally, like \"shift+f10\", or \"none\" for disabling."));
  myHotKeyEdit = new ShortcutEdit();
  myHotKeyEdit->setToolTip(myHotKeyLabel->toolTip());
  myHotKeyLabel->setBuddy(myHotKeyEdit);
  hotKeyLayout->addWidget(myHotKeyEdit);
  myMsgActionsLayout->addLayout(hotKeyLayout, 3, 1);
#endif

  // Make the columns evenly wide
  myMsgActionsLayout->setColumnStretch(0, 1);
  myMsgActionsLayout->setColumnStretch(1, 1);

  myParanoiaBox = new QGroupBox(tr("Paranoia"));
  myParanoiaLayout = new QVBoxLayout(myParanoiaBox);

  myIgnoreNewUsersCheck = new QCheckBox(tr("Ignore new users"));
  myIgnoreNewUsersCheck->setToolTip(tr("Determines if new users are automatically added to your list or must first request authorization."));
  myParanoiaLayout->addWidget(myIgnoreNewUsersCheck);

  myIgnoreMassMsgCheck = new QCheckBox(tr("Ignore mass messages"));
  myIgnoreMassMsgCheck->setToolTip(tr("Determines if mass messages are ignored or not."));
  myParanoiaLayout->addWidget(myIgnoreMassMsgCheck);

  myIgnoreWebPanelCheck = new QCheckBox(tr("Ignore web panel"));
  myIgnoreWebPanelCheck->setToolTip(tr("Determines if web panel messages are ignored or not."));
  myParanoiaLayout->addWidget(myIgnoreWebPanelCheck);

  myIgnoreEmailPagerCheck = new QCheckBox(tr("Ignore email pager"));
  myIgnoreEmailPagerCheck->setToolTip(tr("Determines if email pager messages are ignored or not."));
  myParanoiaLayout->addWidget(myIgnoreEmailPagerCheck);


  myPageOnEventLayout->addWidget(myNewMsgActionsBox);
  myPageOnEventLayout->addWidget(myParanoiaBox);
  myPageOnEventLayout->addStretch(1);

  return w;
}

QWidget* Settings::Events::createPageSounds(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  myPageSoundsLayout = new QVBoxLayout(w);
  myPageSoundsLayout->setContentsMargins(0, 0, 0, 0);

  myOnEventBox = new OnEventBox(true);
  myPageSoundsLayout->addWidget(myOnEventBox);

  QGridLayout* soundsLayout = dynamic_cast<QGridLayout*>(myOnEventBox->layout());
  int soundsRows = soundsLayout->rowCount();

  myNoSoundInActiveChatCheck = new QCheckBox(tr("Disable sound for active window"));
  myNoSoundInActiveChatCheck->setToolTip(tr("Don't perform OnEvent command if chat window for user is currently active."));
  soundsLayout->addWidget(myNoSoundInActiveChatCheck, soundsRows, 1, 1, 3);

  myPageSoundsLayout->addStretch(1);

  return w;
}

QWidget* Settings::Events::createPageFilter(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  QVBoxLayout* pageFilterLayout = new QVBoxLayout(w);
  pageFilterLayout->setContentsMargins(0, 0, 0, 0);

  QGroupBox* filterRulesBox = new QGroupBox(tr("Rules for Incoming Events"));
  pageFilterLayout->addWidget(filterRulesBox);
  QVBoxLayout* filterRulesLayout = new QVBoxLayout(filterRulesBox);

  myRulesList = new QTreeWidget();
  QStringList headers;
  headers << tr("Enabled") << tr("Action") << tr("Protocol") << tr("Event Type") << tr("Expression");
  myRulesList->setHeaderLabels(headers);
  myRulesList->setIndentation(0);
  myRulesList->setAllColumnsShowFocus(true);
  filterRulesLayout->addWidget(myRulesList);

  QHBoxLayout* buttons = new QHBoxLayout();

  QPushButton* ruleHintsButton = new QPushButton(tr("Hints"));
  buttons->addWidget(ruleHintsButton);

  QPushButton* rulesResetButton = new QPushButton(tr("Reset"));
  buttons->addWidget(rulesResetButton);

  myRuleAddButton = new QPushButton(tr("Add"));
  buttons->addWidget(myRuleAddButton);

  myRuleRemoveButton = new QPushButton(tr("Remove"));
  buttons->addWidget(myRuleRemoveButton);

  myRuleEditButton = new QPushButton(tr("Modify"));
  buttons->addWidget(myRuleEditButton);

  myRuleUpButton = new QPushButton(tr("Move Up"));
  buttons->addWidget(myRuleUpButton);

  myRuleDownButton = new QPushButton(tr("Move Down"));
  buttons->addWidget(myRuleDownButton);

  filterRulesLayout->addLayout(buttons);

  myRuleEditor = NULL;

  connect(myRulesList, SIGNAL(itemSelectionChanged()), SLOT(updateRuleButtons()));
  connect(myRulesList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
      SLOT(editRule(QTreeWidgetItem*,int)));
  connect(ruleHintsButton, SIGNAL(clicked()), SLOT(showRuleHints()));
  connect(rulesResetButton, SIGNAL(clicked()), SLOT(resetRules()));
  connect(myRuleAddButton, SIGNAL(clicked()), SLOT(insertRule()));
  connect(myRuleRemoveButton, SIGNAL(clicked()), SLOT(removeRule()));
  connect(myRuleEditButton, SIGNAL(clicked()), SLOT(editRule()));
  connect(myRuleUpButton, SIGNAL(clicked()), SLOT(moveRuleUp()));
  connect(myRuleDownButton, SIGNAL(clicked()), SLOT(moveRuleDown()));

  return w;
}

void Settings::Events::load()
{
  Config::Chat* chatConfig = Config::Chat::instance();
  Config::ContactList* contactListConfig = Config::ContactList::instance();
  Config::General* generalConfig = Config::General::instance();
  Config::Shortcuts* shortcutConfig = Config::Shortcuts::instance();

  myAutoRaiseCheck->setChecked(generalConfig->autoRaiseMainwin());
  myBoldOnMsgCheck->setChecked(generalConfig->boldOnMsg());

#ifdef Q_WS_X11
  myHotKeyEdit->setKeySequence(shortcutConfig->getShortcut(Config::Shortcuts::GlobalPopupMessage));
#endif

  Config::ContactList::FlashMode flash = contactListConfig->flash();
  myFlashUrgentCheck->setChecked(flash == Config::ContactList::FlashUrgent || flash == Config::ContactList::FlashAll);
  myFlashAllCheck->setChecked(flash == Config::ContactList::FlashAll);

  myAutoPopupCombo->setCurrentIndex(chatConfig->autoPopup());
  myAutoUrgentCheck->setChecked(chatConfig->autoPopupUrgentOnly());
  myAutoFocusCheck->setChecked(chatConfig->autoFocus());
  myFlashTaskbarCheck->setChecked(chatConfig->flashTaskbar());
  myNoSoundInActiveChatCheck->setChecked(chatConfig->noSoundInActiveChat());

  myIgnoreNewUsersCheck->setChecked(Licq::gDaemon.ignoreType(Licq::Daemon::IgnoreNewUsers));
  myIgnoreMassMsgCheck->setChecked(Licq::gDaemon.ignoreType(Licq::Daemon::IgnoreMassMsg));
  myIgnoreWebPanelCheck->setChecked(Licq::gDaemon.ignoreType(Licq::Daemon::IgnoreWebPanel));
  myIgnoreEmailPagerCheck->setChecked(Licq::gDaemon.ignoreType(Licq::Daemon::IgnoreEmailPager));

  const OnEventData* eventData = gOnEventManager.lockGlobal();
  myOnEventBox->load(eventData, NULL);
  gOnEventManager.unlock(eventData);

  Licq::gFilterManager.getRules(myFilterRules);
  updateRulesList();
}

void Settings::Events::apply()
{
  Config::Chat* chatConfig = Config::Chat::instance();
  Config::ContactList* contactListConfig = Config::ContactList::instance();
  Config::General* generalConfig = Config::General::instance();
  Config::Shortcuts* shortcutConfig = Config::Shortcuts::instance();
  chatConfig->blockUpdates(true);
  contactListConfig->blockUpdates(true);
  generalConfig->blockUpdates(true);

  generalConfig->setAutoRaiseMainwin(myAutoRaiseCheck->isChecked());
  generalConfig->setBoldOnMsg(myBoldOnMsgCheck->isChecked());

#ifdef Q_WS_X11
  shortcutConfig->setShortcut(Config::Shortcuts::GlobalPopupMessage, myHotKeyEdit->keySequence());
#endif

  if (myFlashAllCheck->isChecked())
    contactListConfig->setFlash(Config::ContactList::FlashAll);
  else if(myFlashUrgentCheck->isChecked())
    contactListConfig->setFlash(Config::ContactList::FlashUrgent);
  else
    contactListConfig->setFlash(Config::ContactList::FlashNone);

  chatConfig->setAutoPopup(myAutoPopupCombo->currentIndex());
  chatConfig->setAutoPopupUrgentOnly(myAutoUrgentCheck->isChecked());
  chatConfig->setAutoFocus(myAutoFocusCheck->isChecked());
  chatConfig->setFlashTaskbar(myFlashTaskbarCheck->isChecked());
  chatConfig->setNoSoundInActiveChat(myNoSoundInActiveChatCheck->isChecked());

  Licq::gDaemon.setIgnoreType(Licq::Daemon::IgnoreNewUsers, myIgnoreNewUsersCheck->isChecked());
  Licq::gDaemon.setIgnoreType(Licq::Daemon::IgnoreMassMsg, myIgnoreMassMsgCheck->isChecked());
  Licq::gDaemon.setIgnoreType(Licq::Daemon::IgnoreWebPanel, myIgnoreWebPanelCheck->isChecked());
  Licq::gDaemon.setIgnoreType(Licq::Daemon::IgnoreEmailPager, myIgnoreEmailPagerCheck->isChecked());

  OnEventData* eventData = gOnEventManager.lockGlobal();
  myOnEventBox->apply(eventData);
  gOnEventManager.unlock(eventData, true);

  Licq::gFilterManager.setRules(myFilterRules);

  chatConfig->blockUpdates(false);
  contactListConfig->blockUpdates(false);
  generalConfig->blockUpdates(false);
}

void Settings::Events::updateRulesList()
{
  myRulesList->clear();
  BOOST_FOREACH(const Licq::FilterRule& rule, myFilterRules)
  {
    QTreeWidgetItem* item = new QTreeWidgetItem(myRulesList);

    item->setText(0, (rule.isEnabled ? tr("Yes") : tr("No")));

    QString actionStr;
    switch (rule.action)
    {
      case Licq::FilterRule::ActionAccept: actionStr = tr("Accept"); break;
      case Licq::FilterRule::ActionSilent: actionStr = tr("Silent"); break;
      case Licq::FilterRule::ActionIgnore: actionStr = tr("Ignore"); break;
    }
    item->setText(1, actionStr);

    if (rule.protocolId == 0)
    {
      item->setText(2, tr("Any"));
    }
    else
    {
      Licq::ProtocolPlugin::Ptr proto = Licq::gPluginManager.getProtocolPlugin(rule.protocolId);
      if (proto.get() != 0)
        item->setText(2, QString::fromLocal8Bit(proto->name().c_str()));
    }

    QString eventName;
    for (int i = 0; i < 32; ++i)
    {
      if ((rule.eventMask & (1<<i)) == 0)
        continue;
      if (eventName.isEmpty())
      {
        eventName = QString::fromLocal8Bit(Licq::UserEvent::eventName(i).c_str());
      }
      else
      {
        eventName = tr("(Multiple)");
        break;
      }
    }
    item->setText(3, eventName);

    item->setText(4, QString::fromUtf8(rule.expression.c_str()));
  }

  myRulesList->resizeColumnToContents(0);
  myRulesList->resizeColumnToContents(1);
  myRulesList->resizeColumnToContents(2);
  myRulesList->resizeColumnToContents(3);
  myRulesList->resizeColumnToContents(4);
  updateRuleButtons();
}

void Settings::Events::updateRuleButtons()
{
  QTreeWidgetItem* item = myRulesList->currentItem();
  int pos = (item == NULL ? -1 : myRulesList->indexOfTopLevelItem(item));
  bool editing = (myRuleEditor != NULL);
  myRuleAddButton->setEnabled(!editing);
  myRuleRemoveButton->setEnabled(item != NULL && !editing);
  myRuleEditButton->setEnabled(item != NULL && !editing);
  myRuleUpButton->setEnabled(item != NULL && !editing && pos > 0);
  myRuleDownButton->setEnabled(item != NULL && !editing &&
      pos < myRulesList->topLevelItemCount() - 1);
}

void Settings::Events::showRuleHints()
{
  QString h = tr(
      "<h2>Hints for Event Filter Rules</h2>"
      "<p>Incoming events are run through the list of rules to decide how to"
      " handle them. The first rule to match decides the action and if no rule"
      " matches the default action is to accept the event. (To override the"
      " default, add a rule last with another action that matches all event"
      " types and has an empty expression.)</p>"
      "<p>Any event from a user already in the contact list is always accepted"
      " (unless they're in the ignore list). The event filter is only applied"
      " to events from unknown users.</p>"
      "<p>The following actions are available:</p><ul>"
      "<li>Accept - the event as handled as normal and on events performed.</li>"
      "<li>Silent - the event is written to history but otherwise ignored.</li>"
      "<li>Ignore - the event is completely ignored.</li>"
      "</ul>"
      "<p>If the expression is empty, it will match any event. Otherwise it is"
      " applied as a regular expression to any message in the event. "
      "The expression must match the entire message text. (To match only part"
      " of a message, enter it as \".*part.*\".)</p>"
      "<p>The filter has a default set of rules that will block some common"
      " spam messages. The defaults can be restored by removing the file"
      " \"~/.licq/filter.conf\" while Licq is NOT running.</p>"
      );
  new HintsDlg(h, dynamic_cast<QWidget*>(parent()));
}

void Settings::Events::resetRules()
{
  Licq::gFilterManager.getDefaultRules(myFilterRules);
  updateRulesList();
}

void Settings::Events::editRule(QTreeWidgetItem* item, int /* index */)
{
  if (myRuleEditor != NULL || item == NULL)
    return;

  myRuleEditIndex = myRulesList->indexOfTopLevelItem(item);
  if (myRuleEditIndex < 0)
    return;

  myRuleEditor = new FilterRuleDlg(&myFilterRules[myRuleEditIndex]);
  connect(myRuleEditor, SIGNAL(finished(int)), SLOT(editRuleDone(int)));

  updateRuleButtons();
}

void Settings::Events::editRule()
{
  editRule(myRulesList->currentItem(), 0);
}

void Settings::Events::insertRule()
{
  myRuleEditIndex = -1;
  myRuleEditor = new FilterRuleDlg();
  connect(myRuleEditor, SIGNAL(finished(int)), SLOT(editRuleDone(int)));

  updateRuleButtons();
}

void Settings::Events::editRuleDone(int dialogCode)
{
  if (dialogCode == QDialog::Rejected)
  {
    // User pressed cancel
    myRuleEditor = NULL;
    updateRuleButtons();
    return;
  }

  if (myRuleEditIndex == -1)
  {
    // Rule is added, make room for it in list
    myRuleEditIndex = myFilterRules.size();
    myFilterRules.resize(myRuleEditIndex + 1);
  }

  myRuleEditor->getFilterRule(myFilterRules[myRuleEditIndex]);
  myRuleEditor = NULL;

  updateRulesList();
}

void Settings::Events::removeRule()
{
  QTreeWidgetItem* item = myRulesList->currentItem();
  if (item == NULL)
    return;

  int pos = myRulesList->indexOfTopLevelItem(item);
  if (pos < 0)
    return;

  for (int i = pos; i < (int)myFilterRules.size() - 1; ++i)
    myFilterRules[i] = myFilterRules[i+1];
  myFilterRules.erase(--myFilterRules.end());

  // Drop the list item instead of reloading entire list
  delete item;
}

void Settings::Events::moveRuleUp()
{
  QTreeWidgetItem* item = myRulesList->currentItem();
  if (item == NULL)
    return;

  int pos = myRulesList->indexOfTopLevelItem(item);
  if (pos <= 0)
    return;

  Licq::FilterRule tempRule = myFilterRules[pos];
  myFilterRules[pos] = myFilterRules[pos - 1];
  myFilterRules[pos - 1] = tempRule;

  // Swap the list items instead of reloading entire list
  myRulesList->takeTopLevelItem(pos);
  myRulesList->insertTopLevelItem(pos-1, item);
  myRulesList->setCurrentItem(item);
}

void Settings::Events::moveRuleDown()
{
  QTreeWidgetItem* item = myRulesList->currentItem();
  if (item == NULL)
    return;

  int pos = myRulesList->indexOfTopLevelItem(item);
  if (pos < 0 || pos >= myRulesList->topLevelItemCount() - 1)
    return;

  Licq::FilterRule tempRule = myFilterRules[pos];
  myFilterRules[pos] = myFilterRules[pos + 1];
  myFilterRules[pos + 1] = tempRule;

  // Swap the list items instead of reloading entire list
  myRulesList->takeTopLevelItem(pos);
  myRulesList->insertTopLevelItem(pos+1, item);
  myRulesList->setCurrentItem(item);
}
