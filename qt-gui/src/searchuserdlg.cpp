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
// modified by Graham Roff
// enhanced by Dirk A. Mueller <dmuell@gmx.net>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qcheckbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qtabwidget.h>
#include <qvbuttongroup.h>
#include <qvalidator.h>

#include "searchuserdlg.h"
#include "sigman.h"

#include "licq_user.h"
#include "licq_icqd.h"


// -----------------------------------------------------------------------------

SearchUserView::SearchUserView(QWidget *parent, char *name)
  : QListView(parent, name)
{
  addColumn(tr("Alias"), 105);
  addColumn(tr("UIN"), 70);
  setColumnAlignment(1, AlignRight);
  addColumn(tr("Name"), 120);
  addColumn(tr("Email"), 120);
  setAllColumnsShowFocus (true);
  setMinimumHeight(150);
  setMinimumWidth(440);
#if QT_VERSION >= 210
  setSelectionMode(QListView::Extended);
  setShowSortIndicator(true);
#endif
}


SearchItem::SearchItem(CSearchAck *s, QListView *parent) : QListViewItem(parent)
{
  uinVal = s->Uin();
  setText(0, QString::fromLocal8Bit(s->Alias()));
  setText(1, QString::number(s->Uin()));
  setText(2, QString::fromLocal8Bit(s->FirstName()) + QString(" ") + QString::fromLocal8Bit(s->LastName()));
  setText(3, s->Email());
}


unsigned long SearchItem::uin()
{
  return (uinVal);
}


// -----------------------------------------------------------------------------

SearchUserDlg::SearchUserDlg(CICQDaemon *s, CSignalManager *theSigMan,
                             QWidget *parent, const char *name)
  : QDialog(parent, name, false, WDestructiveClose)
{
  server = s;
  sigman = theSigMan;
  setCaption(tr("Licq - User Search"));
  searchTag = NULL;

  QBoxLayout* top_lay = new QVBoxLayout(this, 6);
  QBoxLayout* lay = new QHBoxLayout(top_lay, 10);

  // pre-search widgets
  search_tab = new QTabWidget(this);

  //-- first tab: search by Alias/name
  alias_tab = new QWidget(this);
  QGridLayout* grid_lay = new QGridLayout(alias_tab, 7, 7);
  grid_lay->addColSpacing(0, 10);grid_lay->addRowSpacing(0, 10);
  grid_lay->addColSpacing(2, 10);grid_lay->addRowSpacing(2, 10);
  grid_lay->addColSpacing(4, 10);grid_lay->addRowSpacing(4, 10);
  grid_lay->addRowSpacing(6, 10);

  lblNick = new QLabel(tr("Alias:"), alias_tab);
  grid_lay->addWidget(lblNick, 1, 1);
  edtNick = new QLineEdit(alias_tab);
  grid_lay->addWidget(edtNick, 1, 3);
  lblFirst = new QLabel(tr("First Name:"), alias_tab);
  grid_lay->addWidget(lblFirst, 3, 1);
  edtFirst = new QLineEdit(alias_tab);
  grid_lay->addWidget(edtFirst, 3, 3);
  lblLast = new QLabel(tr("Last Name:"), alias_tab);
  grid_lay->addWidget(lblLast, 5, 1);
  edtLast = new QLineEdit(alias_tab);
  grid_lay->addWidget(edtLast, 5, 3);

  search_tab->addTab(alias_tab, tr("&Name"));

  //-- second tab: search by email

  email_tab  = new QWidget(this);
  QBoxLayout* lay2 = new QHBoxLayout(email_tab, 10);
  lblEmail = new QLabel(tr("Email Address:"), email_tab);
  lay2->addWidget(lblEmail);
  edtEmail = new QLineEdit(email_tab);
  lay2->addWidget(edtEmail);

  search_tab->addTab(email_tab, tr("&Email"));

  //-- third tab: search by UIN
  uin_tab = new QWidget(this);
  lay2 = new QHBoxLayout(uin_tab, 10);
  lblUin = new QLabel(tr("UIN#:"), uin_tab);
  lay2->addWidget(lblUin);
  edtUin = new QLineEdit(uin_tab);
  edtUin->setValidator(new QIntValidator(10000,2000000000, this));
  lay2->addWidget(edtUin);

  search_tab->addTab(uin_tab, tr("&Uin#"));

  lay->addWidget(search_tab, 1);
  lay2 = new QVBoxLayout(lay, 10);
  lay2->addStretch(1);
  btnSearch = new QPushButton(tr("&Search"), this);
  btnSearch->setDefault(true);
  lay2->addWidget(btnSearch);

  btnReset = new QPushButton(tr("Reset Search"), this);
  btnReset->setEnabled(false);
  lay2->addWidget(btnReset);

  btnDone = new QPushButton(tr("&Done"), this);
  lay2->addWidget(btnDone);

  connect (btnReset, SIGNAL(clicked()), this, SLOT(resetSearch()));
  connect (btnSearch, SIGNAL(clicked()), this, SLOT(startSearch()));
  connect (sigman, SIGNAL(signal_searchResult(ICQEvent *)),
           this, SLOT(searchResult(ICQEvent *)));

   // pseudo Status Bar
  lblSearch = new QLabel(tr("Enter search parameters and select 'Search'"), this);
  lblSearch->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

  // post-search widgets
  top_lay->addSpacing(10);

  foundView = new SearchUserView(this);
  top_lay->addWidget(foundView, 2);

  lay = new QHBoxLayout(top_lay, 20);

  qcbAlertUser = new QCheckBox(tr("A&lert User"), this);
  qcbAlertUser->setChecked(true);

  lay->addWidget(qcbAlertUser);

  lay->addStretch(1);

  btnAdd = new QPushButton(tr("&Add User"), this);
  btnAdd->setEnabled(false);
  lay->addWidget(btnAdd);

  connect(btnDone, SIGNAL(clicked()), this, SLOT(close()));
  connect(foundView, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
  connect (btnAdd, SIGNAL(clicked()), this, SLOT(addUser()));

  top_lay->addWidget(lblSearch);

  resetSearch();
}

SearchUserDlg::~SearchUserDlg()
{
  delete searchTag;
}


void SearchUserDlg::startSearch()
{
  unsigned long uin = edtUin->text().toULong(&uin_search);

  foundView->clear();
  edtNick->setEnabled(false);
  edtFirst->setEnabled(false);
  edtLast->setEnabled(false);
  edtEmail->setEnabled(false);
  edtUin->setEnabled(false);
  btnSearch->setEnabled(false);
  btnReset->setEnabled(true);
  btnDone->setEnabled(false);
  btnAdd->setEnabled(false);

  if (uin_search && uin)
  {
    searchTag = server->icqSearchByUin(uin);
  }
  else
  {
    searchTag = server->icqSearchByInfo(edtNick->text().local8Bit(),edtFirst->text().local8Bit(),
       edtLast->text().local8Bit(), edtEmail->text().local8Bit());
  }
  lblSearch->setText(tr("Searching (this can take awhile)..."));
}

void SearchUserDlg::resetSearch()
{
  edtEmail->setText("");
  edtLast->setText("");
  edtFirst->setText("");
  edtNick->setText("");
  edtUin->setText("");
  btnSearch->setEnabled(true);
  btnAdd->setEnabled(false);
  btnReset->setEnabled(false);
  foundView->clear();
  delete searchTag;
  searchTag = NULL;
  lblSearch->setText(tr("Enter search parameters and select 'Search'"));
}


void SearchUserDlg::searchResult(ICQEvent *e)
{
  if (!searchTag->Equals(e)) return;

  btnSearch->setEnabled(true);
  btnDone->setEnabled(true);
  edtNick->setEnabled(true);
  edtFirst->setEnabled(true);
  edtLast->setEnabled(true);
  edtEmail->setEnabled(true);
  edtUin->setEnabled(true);

  if (e->Result() == EVENT_SUCCESS)
  {
    searchDone(e->SearchAck()->More());
    delete searchTag;
    searchTag = NULL;
  }
  else if (e->Result() == EVENT_ACKED)
    searchFound(e->SearchAck());
  else
  {
    searchFailed();
    delete searchTag;
    searchTag = NULL;
  }

}

void SearchUserDlg::searchFound(CSearchAck *s)
{
  (void) new SearchItem(s, foundView);
}


void SearchUserDlg::searchDone(bool more)
{
  if (more)
    lblSearch->setText(tr("More users found. Narrow search."));
  else
    lblSearch->setText("Search complete.");
}


void SearchUserDlg::searchFailed()
{
  lblSearch->setText(tr("Search failed."));
}

void SearchUserDlg::selectionChanged()
{
  QListViewItem* current = foundView->firstChild();
  int count = 0;

  while(current) {
    if(current->isSelected())
      count++;
    current = current->nextSibling();
  }

  btnAdd->setEnabled(true);
  switch(count) {
  case 0:
    btnAdd->setEnabled(false);
    // fall through
  case 1:
    btnAdd->setText(tr("&Add User"));
    break;
  default:
    btnAdd->setText(tr("&Add %1 Users").arg(count));
  }
}


void SearchUserDlg::addUser()
{
  SearchItem* current = static_cast<SearchItem*>(foundView->firstChild());

  while(current) {
    if(current->isSelected()) {
      ICQUser* user = gUserManager.FetchUser(current->uin(), LOCK_N);

      if(user)
        gUserManager.DropUser(user);
      else {
        server->AddUserToList(current->uin());
        if (qcbAlertUser->isChecked()) // alert the user they were added
            server->icqAlertUser(current->uin());
      }
//      current->setSelected(false);
    }
    current = static_cast<SearchItem*>(current->nextSibling());
  }
  foundView->triggerUpdate();
  selectionChanged();
}

#include "searchuserdlg.moc"
