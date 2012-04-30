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

#ifndef HISTORYDLG_H
#define HISTORYDLG_H

#include "config.h"

#include <QDialog>

#include <licq/contactlist/user.h>
#include <licq/userid.h>

class QCalendarWidget;
class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRegExp;

namespace Licq
{
class Event;
}

namespace LicqQtGui
{
class Calendar;
class HistoryView;

/**
 * Dialog that displays history for a contact with search functionallity
 */
class HistoryDlg : public QDialog
{
  Q_OBJECT

public:
  /**
   * Constructor
   *
   * @param userId Contact id
   * @param parent Parent widget
   */
  HistoryDlg(const Licq::UserId& userId, QWidget* parent = 0);

  /**
   * Desstructor
   */
  ~HistoryDlg();

private slots:
  /**
   * Update history view to show the date marked in the calendar
   */
  void calenderClicked();

  /**
   * Search forwards
   */
  void findNext();

  /**
   * Search backwards
   */
  void findPrevious();

  /**
   * Find the next occurence of the word in the search box
   *
   * @param backwards Search backwards in history
   */
  void find(bool backwards);

  /**
   * Search field has changed
   *
   * @param text Contents of text field
   */
  void searchTextChanged(const QString& text);

  /**
   * Popup user menu from menu button
   */
  void showUserMenu();

  /**
   * Go to next date with activity
   */
  void nextDate();

  /**
   * Go to previous date with activity
   */
  void previousDate();

private slots:
  /**
   * A user was updated. Add to history if it was a message recieved for this user
   *
   * @param userId Id for affected user
   * @param subSignal Sub signal telling what the change was
   * @param argument Additional data, usage depend on sub signal type
   */
  void updatedUser(const Licq::UserId& userId, unsigned long subSignal, int argument);

  /**
   * A message was sent. Add to history if it was for the current user
   *
   * @param event Event object for message
   */
  void eventSent(const Licq::Event* event);

private:
  /**
   * Add an event to the current history
   *
   * @param event Event to add
   */
  void addMsg(const Licq::UserEvent* event);

  /**
   * Build a regular expression from the input fields
   *
   * @return A regular expression
   */
  QRegExp getRegExp() const;

  /**
   * Populate history view with entries
   */
  void showHistory();

  /**
   * Update window title
   *
   * @param @user The user object
   */
  void setTitle(const Licq::User* user);

  Licq::UserId myUserId;
  bool myIsOwner;
  QString myContactName;
  QString myOwnerName;
  bool myUseHtml;
  bool myPatternChanged;

  Licq::HistoryList myHistoryList;
  Licq::HistoryList::iterator mySearchPos;

  Calendar* myCalendar;
  HistoryView* myHistoryView;
  QLabel* myStatusLabel;
  QLineEdit* myPatternEdit;
  QCheckBox* myMatchCaseCheck;
  QCheckBox* myRegExpSearchCheck;
  QPushButton* myFindPrevButton;
  QPushButton* myFindNextButton;
};

} // namespace LicqQtGui

#endif
