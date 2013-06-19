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

#include "historydlg.h"

#include "config.h"

#include <cstdio>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QPushButton>
#include <QRegExp>
#include <QShortcut>
#include <QVBoxLayout>

#include <licq/contactlist/owner.h>
#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>
#include <licq/event.h>
#include <licq/pluginsignal.h>
#include <licq/userevents.h>

#include "config/chat.h"
#include "core/licqgui.h"
#include "core/signalmanager.h"
#include "core/usermenu.h"
#include "helpers/support.h"
#include "widgets/calendar.h"
#include "widgets/historyview.h"


using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::HistoryDlg */

HistoryDlg::HistoryDlg(const Licq::UserId& userId, QWidget* parent)
  : QDialog(parent),
    myUserId(userId)
{
  Support::setWidgetProps(this, "UserHistoryDialog");
  setAttribute(Qt::WA_DeleteOnClose, true);

  QVBoxLayout* topLayout = new QVBoxLayout(this);

  // Main content (everything except dialog buttons)
  QHBoxLayout* mainLayout = new QHBoxLayout();
  topLayout->addLayout(mainLayout);

  // Left sidebar with calendar and search controls
  QVBoxLayout* sidebarLayout = new QVBoxLayout();
  mainLayout->addLayout(sidebarLayout);

  // Calendar for navigating
  myCalendar = new Calendar();
  connect(myCalendar, SIGNAL(clicked(const QDate&)), SLOT(calenderClicked()));
  sidebarLayout->addWidget(myCalendar);

  // Buttons to go to previos/next day with activity
  QHBoxLayout* navigateLayout = new QHBoxLayout();
  sidebarLayout->addLayout(navigateLayout);
  QPushButton* previousDateButton = new QPushButton(tr("&Previous day"));
  connect(previousDateButton, SIGNAL(clicked()), SLOT(previousDate()));
  navigateLayout->addWidget(previousDateButton);
  navigateLayout->addStretch(1);
  QPushButton* nextDateButton = new QPushButton(tr("&Next day"));
  connect(nextDateButton, SIGNAL(clicked()), SLOT(nextDate()));
  navigateLayout->addWidget(nextDateButton);

  // Status label for showing various messages
  myStatusLabel = new QLabel();
  sidebarLayout->addWidget(myStatusLabel);

  sidebarLayout->addStretch(1);

  // Controls for searching history
  QGroupBox* searchGroup = new QGroupBox(tr("Search"));
  sidebarLayout->addWidget(searchGroup);
  QVBoxLayout* searchLayout = new QVBoxLayout(searchGroup);

  // Input field for searching
  QHBoxLayout* patternLayout = new QHBoxLayout();
  searchLayout->addLayout(patternLayout);
  QLabel* patternLabel = new QLabel(tr("Find:"));
  patternLayout->addWidget(patternLabel);
  myPatternEdit = new QLineEdit();
  patternLayout->addWidget(myPatternEdit);
  patternLabel->setBuddy(myPatternEdit);

  // Set focus to search box if user presses slash
  QShortcut* patternShortcut = new QShortcut(Qt::Key_Slash, this);
  connect(patternShortcut, SIGNAL(activated()), myPatternEdit, SLOT(setFocus()));

  // Options to control searching
  myMatchCaseCheck = new QCheckBox(tr("Match &case"));
  searchLayout->addWidget(myMatchCaseCheck);
  myRegExpSearchCheck = new QCheckBox(tr("&Regular expression"));
  searchLayout->addWidget(myRegExpSearchCheck);

  // Find button
  QHBoxLayout* findLayout = new QHBoxLayout();
  myFindPrevButton = new QPushButton(tr("F&ind previous"));
  myFindPrevButton->setEnabled(false);
  connect(myFindPrevButton, SIGNAL(clicked()), SLOT(findPrevious()));
  findLayout->addWidget(myFindPrevButton);
  findLayout->addStretch(1);
  myFindNextButton = new QPushButton(tr("&Find next"));
  myFindNextButton->setDefault(true);
  myFindNextButton->setEnabled(false);
  connect(myFindNextButton, SIGNAL(clicked()), SLOT(findNext()));
  findLayout->addWidget(myFindNextButton);
  searchLayout->addLayout(findLayout);
  connect(myPatternEdit, SIGNAL(textChanged(const QString&)), SLOT(searchTextChanged(const QString&)));
  myPatternChanged = true;

  // Shortcuts for searching
  QShortcut* findPrevShortcut = new QShortcut(Qt::SHIFT + Qt::Key_F3, this);
  connect(findPrevShortcut, SIGNAL(activated()), SLOT(findPrevious()));
  QShortcut* findNextShortcut = new QShortcut(Qt::Key_F3, this);
  connect(findNextShortcut, SIGNAL(activated()), SLOT(findNext()));

  // Widget to show history entries
  myHistoryView = new HistoryView(true, myUserId);
  mainLayout->addWidget(myHistoryView, 1);

  // Dialog buttons
  QHBoxLayout* buttonsLayout = new QHBoxLayout();
  topLayout->addLayout(buttonsLayout);
  if (!myUserId.isOwner())
  {
    QPushButton* menuButton = new QPushButton(tr("&Menu"));
    connect(menuButton, SIGNAL(pressed()), SLOT(showUserMenu()));
    menuButton->setMenu(gUserMenu);
    buttonsLayout->addWidget(menuButton);
  }
  QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Close);
  connect(buttons, SIGNAL(rejected()), SLOT(close()));
  buttonsLayout->addWidget(buttons);

  show();

  {
    Licq::UserReadGuard u(myUserId);

    setTitle(*u);

    bool validHistory = false;

    if (!u.isLocked())
    {
      myStatusLabel->setText(tr("Invalid user requested"));
    }
    // Fetch list of all history entries
    else if (!u->GetHistory(myHistoryList))
    {
      myStatusLabel->setText(tr("Error loading history file"));
    }
    // No point in doing anything more if history is empty
    else if (myHistoryList.empty())
    {
      myStatusLabel->setText(tr("History is empty"));
    }
    else
    {
      // No problems that should stop us from continuing
      validHistory = true;
    }

    if (!validHistory)
    {
      myCalendar->setEnabled(false);
      previousDateButton->setEnabled(false);
      nextDateButton->setEnabled(false);
      myPatternEdit->setEnabled(false);
      myFindPrevButton->setEnabled(false);
      myFindNextButton->setEnabled(false);
      return;
    }

    myContactName = tr("server");
    myUseHtml = false;

    if (!myUserId.isOwner())
      myContactName = QString::fromUtf8(u->getAlias().c_str());
    if (u->protocolId() == ICQ_PPID)
    {
      QString myId = u->accountId().c_str();
      for (int x = 0; x < myId.length(); x++)
      {
        if (!myId[x].isDigit())
        {
          myUseHtml = true;
          break;
        }
      }
    }
  }

  {
    Licq::OwnerReadGuard o(myUserId.ownerId());
    if (o.isLocked())
      myOwnerName = QString::fromUtf8(o->getAlias().c_str());
  }

  // Mark all dates with activity so they are easier to find
  Licq::HistoryList::iterator item;
  for (item = myHistoryList.begin(); item != myHistoryList.end(); ++item)
  {
    QDate date = QDateTime::fromTime_t((*item)->Time()).date();
    myCalendar->markDate(date);
  }

  // Limit calendar to dates where we have history entries
  myCalendar->setMinimumDate(QDateTime::fromTime_t((*myHistoryList.begin())->Time()).date());
  QDate lastDate = QDateTime::fromTime_t((*(--myHistoryList.end()))->Time()).date();
  myCalendar->setMaximumDate(lastDate);
  myCalendar->setSelectedDate(lastDate);
  calenderClicked();

  // Catch sent messages and add them to history
  connect(gLicqGui, SIGNAL(eventSent(const Licq::Event*)),
      SLOT(eventSent(const Licq::Event*)));

  // Catch received messages so we can add them to history
  connect(gGuiSignalManager,
      SIGNAL(updatedUser(const Licq::UserId&, unsigned long, int, unsigned long)),
      SLOT(updatedUser(const Licq::UserId&, unsigned long, int)));
}

HistoryDlg::~HistoryDlg()
{
  Licq::User::ClearHistory(myHistoryList);
}

void HistoryDlg::updatedUser(const Licq::UserId& userId, unsigned long subSignal, int argument)
{
  if (userId != myUserId)
    return;

  if (subSignal == Licq::PluginSignal::UserEvents)
  {
    const Licq::UserEvent* event;
    {
      Licq::UserReadGuard u(myUserId);
      if (!u.isLocked())
        return;

      event = u->EventPeekId(argument);
    }

    if (event != NULL && argument > 0 && argument > (*(--myHistoryList.end()))->Id())
      addMsg(event);
  }
  else if (subSignal == Licq::PluginSignal::UserBasic)
  {
    Licq::UserReadGuard u(myUserId);
    setTitle(*u);
  }
}

void HistoryDlg::setTitle(const Licq::User* user)
{
  QString name;
  if (user == NULL)
  {
    name = tr("INVALID USER");
  }
  else
  {
    name = QString::fromUtf8(user->getFullName().c_str());
    if (!name.isEmpty())
      name = " (" + name + ")";
    name.prepend(QString::fromUtf8(user->getAlias().c_str()));
  }

  setWindowTitle(tr("Licq - History ") + name);
}

void HistoryDlg::eventSent(const Licq::Event* event)
{
  if (event->userId() == myUserId && event->userEvent() != NULL)
    addMsg(event->userEvent());
}

void HistoryDlg::addMsg(const Licq::UserEvent* event)
{
  Licq::UserEvent* eventCopy = event->Copy();
  myHistoryList.push_back(eventCopy);
  QDate date = QDateTime::fromTime_t(event->Time()).date();
  myCalendar->markDate(date);
  myCalendar->setMaximumDate(date);
}

QRegExp HistoryDlg::getRegExp() const
{
  // Since QRegExp has a FixedString mode we can use it for normal search also
  return QRegExp(
      myPatternEdit->text(),
      (myMatchCaseCheck->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive),
      (myRegExpSearchCheck->isChecked() ? QRegExp::RegExp2 : QRegExp::FixedString));
}

void HistoryDlg::showHistory()
{
  if (myHistoryList.empty())
    return;

  myHistoryView->clear();
  myHistoryView->setReverse(Config::Chat::instance()->reverseHistory());

  QDateTime date;

  // Go through all entries in the list
  Licq::HistoryList::iterator item;
  for (item = myHistoryList.begin(); item != myHistoryList.end(); ++item)
  {
    date.setTime_t((*item)->Time());

    // Skip those that aren't from the selected date
    if (date.date() != myCalendar->selectedDate())
      continue;

    QString messageText = QString::fromUtf8((*item)->text().c_str());
    QString name = (*item)->isReceiver() ? myContactName : myOwnerName;

    QRegExp highlight;

    // Check if this is the entry we've searched for
    if (item == mySearchPos)
    {
      highlight = getRegExp();
      highlight.setMinimal(true);
    }
    messageText = HistoryView::toRichText(messageText, true, myUseHtml, highlight);

    // Add entry to history view
    myHistoryView->addMsg((*item)->isReceiver(), false,
        ((*item)->eventType() == Licq::UserEvent::TypeMessage ? "" : ((*item)->description() + " ").c_str()),
        date,
        (*item)->IsDirect(),
        (*item)->IsMultiRec(),
        (*item)->IsUrgent(),
        (*item)->IsEncrypted(),
        name,
        messageText,
        (item == mySearchPos ? "SearchHit" : QString()));
  }

  // Tell history view to update in case it is buffered
  myHistoryView->updateContent();
}

void HistoryDlg::calenderClicked()
{
  // Clear search position
  mySearchPos = myHistoryList.end();

  myStatusLabel->setText(QString());
  showHistory();
}

void HistoryDlg::findNext()
{
  find(false);
}

void HistoryDlg::findPrevious()
{
  find(true);
}

void HistoryDlg::find(bool backwards)
{
  if (myPatternEdit->text().isEmpty())
    return;

  QRegExp regExp(getRegExp());

  // An expression that can match zero characters is no better than an empty search text
  if (regExp.indexIn("") != -1)
    return;

  // If search pattern has changed, find all matching dates and mark them in the calendar
  if (myPatternChanged)
  {
    myCalendar->clearMatches();

    Licq::HistoryList::iterator i;
    for (i = myHistoryList.begin(); i != myHistoryList.end(); ++i)
    {
      QString messageText = QString::fromUtf8((*i)->text().c_str());
      if (messageText.contains(regExp))
      {
        QDate date = QDateTime::fromTime_t((*i)->Time()).date();
        myCalendar->addMatch(date);
      }
    }

    // No need to do this again next time
    myPatternChanged = false;
  }

  myStatusLabel->setText(QString());

  // If this is first search we need to find an entry to start searching from
  if (mySearchPos == myHistoryList.end())
  {
    for (mySearchPos = myHistoryList.begin(); mySearchPos != myHistoryList.end(); ++mySearchPos)
    {
      QDate date = QDateTime::fromTime_t((*mySearchPos)->Time()).date();

      // When searching backwards, set start to first entry after current day
      if (date > myCalendar->selectedDate())
        break;

      // When searching forwards, set start to last entry before current day
      if (!backwards && date >= myCalendar->selectedDate())
        break;
    }

    // Back one step to actually get entry before current day
    if (!backwards)
      --mySearchPos;
  }

  // Remember where we started so we can stop after checking all entries once
  Licq::HistoryList::iterator startPos = mySearchPos;

  while (true)
  {
    if (backwards)
      --mySearchPos;
    else
      ++mySearchPos;

    // end is outside list so don't try to match it
    if (mySearchPos != myHistoryList.end())
    {
      QString messageText = QString::fromUtf8((*mySearchPos)->text().c_str());
      if (messageText.contains(regExp))
        // We have a match
        break;
    }

    if (mySearchPos == startPos)
    {
      myStatusLabel->setText(tr("Search returned no matches"));
      myPatternEdit->setStyleSheet("background: red");
      return;
    }

    if (mySearchPos == myHistoryList.end())
    {
      myStatusLabel->setText(tr("Search wrapped around"));

      // Iterator wraps around between begin and end so no extra handling needed
      continue;
    }
  }

  QDate date = QDateTime::fromTime_t((*mySearchPos)->Time()).date();
  myCalendar->setSelectedDate(date);
  showHistory();
  myHistoryView->scrollToAnchor("SearchHit");
}

void HistoryDlg::searchTextChanged(const QString& text)
{
  // Disable search buttons if there is no text in the search box
  myFindNextButton->setEnabled(!text.isEmpty());
  myFindPrevButton->setEnabled(!text.isEmpty());

  // Clear failed status from previous search
  myPatternEdit->setStyleSheet("");

  // Mark that pattern has changed since previous search
  myPatternChanged = true;

  // Search field is cleared so clear status message and matching dates
  if (text.isEmpty())
  {
    myStatusLabel->setText(QString());
    myCalendar->clearMatches();
  }
}

void HistoryDlg::showUserMenu()
{
  gUserMenu->setUser(myUserId);
}

void HistoryDlg::nextDate()
{
  QDateTime date;
  Licq::HistoryList::iterator item;

  // Find first entry in next date
  for (item = myHistoryList.begin(); item != myHistoryList.end(); ++item)
  {
    date.setTime_t((*item)->Time());

    // Stop when we find an entry with date later then current
    if (date.date() > myCalendar->selectedDate())
      break;
  }

  // No later date found so go to oldest entry
  if (item == myHistoryList.end())
    date.setTime_t((*myHistoryList.begin())->Time());

  myCalendar->setSelectedDate(date.date());
  calenderClicked();
}

void HistoryDlg::previousDate()
{
  QDateTime date;
  Licq::HistoryList::iterator item;

  // Find first entry in next date
  for (item = myHistoryList.begin(); item != myHistoryList.end(); ++item)
  {
    date.setTime_t((*item)->Time());

    // Stop when we find an entry with date later then current
    if (date.date() >= myCalendar->selectedDate())
      break;
  }

  // Go back to last entry of previous day
  --item;

  // No earlier date, go to last
  if (item == myHistoryList.end())
    --item;

  date.setTime_t((*item)->Time());

  myCalendar->setSelectedDate(date.date());
  calenderClicked();
}
