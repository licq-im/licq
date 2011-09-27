/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2010 Licq developers
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

#ifndef SEARCHUSERDLG_H
#define SEARCHUSERDLG_H

// Search user code base written by Alan Penner (apenner@andrew.cmu.edu)
// modified by Graham Roff && Dirk A. Mueller <dmuell@gmx.net>

#include <QDialog>

class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTreeWidget;

namespace Licq
{
class Event;
class SearchData;
}

namespace LicqQtGui
{
//TODO for protocol plugin support

class SearchUserDlg : public QDialog
{
  Q_OBJECT

public:
  SearchUserDlg();

public slots:
  void reject();

private:
  unsigned long ppid;
  unsigned long searchTag;

  QLabel* lblSearch;

  QLineEdit* edtEmail;
  QLineEdit* edtKeyword;
  QLineEdit* edtFirst;
  QLineEdit* edtLast;
  QLineEdit* edtNick;
  QLineEdit* edtUin;
  QLineEdit* edtCity;
  QLineEdit* edtState;
  QLineEdit* edtCoName;
  QLineEdit* edtCoDept;
  QLineEdit* edtCoPos;

  QGroupBox* grpParms;
  QGroupBox* grpResult;

  QComboBox* cmbAge;
  QComboBox* cmbCountry;
  QComboBox* cmbGender;
  QComboBox* cmbLanguage;

  QCheckBox* chkOnlineOnly;

  QPushButton* btnSearch;
  QPushButton* btnReset;
  QPushButton* btnDone;
  QPushButton* btnInfo;
  QPushButton* btnAdd;

  QTreeWidget* foundView;

  void searchFound(const Licq::SearchData* sa);
  void searchDone(const Licq::SearchData* sa);
  void searchFailed();

private slots:
  void startSearch();
  void resetSearch();
  void searchResult(const Licq::Event* e);
  void selectionChanged();
  void viewInfo();
  void addUser();
};

} // namespace LicqQtGui

#endif
