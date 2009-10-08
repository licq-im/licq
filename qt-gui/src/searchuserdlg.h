/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2009 Licq developers
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

#include <qwidget.h>
#include <qlistview.h>

#include <licq_types.h>

#include "mainwin.h"

class QTabWidget;
class QCheckBox;
class QLabel;
class QLineEdit;
class QComboBox;
class QSpinBox;
class QCheckBox;

class CICQDaemon;
class CSignalManager;
class LicqEvent;
class CSearchAck;

//TODO for protocol plugin support

class SearchUserView : public QListView
{
  Q_OBJECT

public:
   SearchUserView(QWidget *parent = 0);
};


class SearchItem : public QListViewItem
{
public:
  SearchItem(const CSearchAck* s, const QString& encoding, QListView* parent);

  const UserId& userId() const { return myUserId; }

protected:
  UserId myUserId;
};



class SearchUserDlg : public QWidget
{
   Q_OBJECT
public:
   SearchUserDlg (CMainWindow *mainwin, CICQDaemon *s, CSignalManager *theSigMan, const QString &);
   ~SearchUserDlg();

private:
   bool uin_search;

protected:

   CMainWindow *mainwin;
   CICQDaemon *server;
   CSignalManager *sigman;
   QPushButton *btnSearch, *btnReset;
   QCheckBox* qcbAlertUser;
   QTabWidget* search_tab;

   QLabel *lblSearch;
   QLineEdit *edtEmail, *edtKeyword, *edtFirst, *edtLast, *edtNick, *edtUin,
     *edtCity, *edtState, *edtCoName, *edtCoDept, *edtCoPos;
   QComboBox *cmbAge, *cmbCountry, *cmbGender, *cmbLanguage;
   
   QWidget *whitepages_tab, *uin_tab;
   QCheckBox *chkOnlineOnly;

   unsigned long searchTag;
   QPushButton *btnDone, *btnInfo, *btnAdd;
   SearchUserView *foundView;
   QString m_Encoding;

  void searchFound(const CSearchAck* s);
  void searchDone(const CSearchAck* sa);
   void searchFailed();

public slots:
   void startSearch();
   void viewInfo();
   void addUser();
   void resetSearch();
   void searchResult(LicqEvent*);
   void selectionChanged();
};


#endif
