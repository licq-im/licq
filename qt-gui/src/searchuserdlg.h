#ifndef SEARCHUSERDLG_H
#define SEARCHUSERDLG_H

/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

// Search user code base written by Alan Penner (apenner@andrew.cmu.edu)
// modified by Graham Roff && Dirk A. Mueller <dmuell@gmx.net>

#include <qwidget.h>
#include <qlistview.h>

class QTabWidget;
class QCheckBox;
class QLabel;
class QLineEdit;
class QComboBox;
class QSpinBox;
class QCheckBox;

class CICQDaemon;
class CSignalManager;
class ICQEvent;
class ICQUser;
class CSearchAck;

class SearchUserView : public QListView
{
public:
   SearchUserView(QWidget *parent = 0);
};


class SearchItem : public QListViewItem
{
public:
   SearchItem(CSearchAck *, QListView *parent);
   unsigned long uin();
protected:
   unsigned long uinVal;
};



class SearchUserDlg : public QWidget
{
   Q_OBJECT
public:
   SearchUserDlg (CICQDaemon *s, CSignalManager *theSigMan,
                  QWidget *parent =0);
   ~SearchUserDlg();

private:
   bool uin_search;

protected:

   CICQDaemon *server;
   CSignalManager *sigman;
   QPushButton *btnSearch, *btnReset;
   QCheckBox* qcbAlertUser;
   QTabWidget* search_tab;

   QLabel *lblSearch;
   QLineEdit *edtEmail, *edtFirst, *edtLast, *edtNick, *edtUin,
     *edtCity, *edtState, *edtCoName, *edtCoDept, *edtCoPos;
   QComboBox *cmbAge, *cmbCountry, *cmbGender, *cmbLanguage;
   
   QWidget *alias_tab, *email_tab, *uin_tab;
   QCheckBox *chkOnlineOnly;

   unsigned long searchTag;
   QPushButton *btnDone, *btnAdd;
   SearchUserView *foundView;

   void searchFound(CSearchAck *);
   void searchDone(CSearchAck *);
   void searchFailed();

public slots:
   void startSearch();
   void addUser();
   void resetSearch();
   void searchResult(ICQEvent *);
   void selectionChanged();
};


#endif
