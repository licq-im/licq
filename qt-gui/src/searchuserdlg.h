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

#include <qdialog.h>
#include <qlistview.h>

class QTabWidget;
class QCheckBox;
class QLabel;
class QLineEdit;

class CICQDaemon;
class CSignalManager;
class ICQEvent;
class ICQUser;
class CSearchAck;

class SearchUserView : public QListView
{
   Q_OBJECT
public:
   SearchUserView(QWidget *parent = NULL, char *name = NULL);
   unsigned long currentUin(void);
};


class SearchItem : public QListViewItem
{
public:
   SearchItem(CSearchAck *, QListView *parent);
   unsigned long uin();
protected:
   unsigned long uinVal;
};



class SearchUserDlg : public QDialog
{
   Q_OBJECT
public:
   SearchUserDlg (CICQDaemon *s, CSignalManager *theSigMan,
                  QWidget *parent = NULL, const char *name = NULL);

   ~SearchUserDlg();

private:
   bool uin_search;

protected:

   CICQDaemon *server;
   CSignalManager *sigman;
   QPushButton *btnSearch, *btnCancel;
   QCheckBox* qcbAlertUser;
   QTabWidget* search_tab;

   QLabel *lblEmail, *lblFirst, *lblLast, *lblNick, *lblUin;
   QLineEdit *edtEmail, *edtFirst, *edtLast, *edtNick, *edtUin;
   QWidget *alias_tab, *email_tab, *uin_tab;

   unsigned short searchSequence;
   QPushButton *btnDone, *btnAdd, *btnSearchAgain;
   SearchUserView *foundView;
   QLabel *lblSearch;

   void searchFound(CSearchAck *);
   void searchDone(char);
   void searchFailed(void);

public slots:
   virtual void show();
   virtual void hide();
   void startSearch();
   void addUser();
   void resetSearch();
   void slot_searchResult(ICQEvent *);
};


#endif
