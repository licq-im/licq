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

#include <string>
#include <qhbox.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qtabwidget.h>
#include <qvbuttongroup.h>
#include <iostream.h>

#include "searchuserdlg.h"


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
}


unsigned long SearchUserView::currentUin(void)
{
  if (!selectedItem()) return (0);
  return (((SearchItem *)currentItem())->uin());
}


SearchItem::SearchItem(struct UserBasicInfo *us, QListView *parent) : QListViewItem(parent)
{
  uinVal = (unsigned long)atol(us->uin);
  setText(0, us->alias);
  setText(1, us->uin);
  setText(2, us->name);
  setText(3, us->email);
}


unsigned long SearchItem::uin(void)
{
  return (uinVal);
}


SearchUserDlg::SearchUserDlg(CICQDaemon *s, CSignalManager *theSigMan,
                             QWidget *parent, const char *name)
  : QDialog(parent, name)
{
  server = s;
  sigman = theSigMan;
  setCaption("Licq User Search");
  
  QBoxLayout* top_lay = new QVBoxLayout(this, 10);
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
  
  lay2->addWidget(edtUin);
  
  search_tab->addTab(uin_tab, tr("&Uin#"));
  
  lay->addWidget(search_tab, 1);
  
  lay2 = new QVBoxLayout(lay, 10);
  
  lay2->addStretch(1);
  
  btnSearch = new QPushButton(tr("&Search"), this);
  lay2->addWidget(btnSearch);
  
  btnSearchAgain = new QPushButton(tr("Search Again"), this);
  btnSearchAgain->setEnabled(false);
  lay2->addWidget(btnSearchAgain);
  
  btnCancel = new QPushButton(tr("&Cancel"), this);
  lay2->addWidget(btnCancel);
  
  connect (btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect (btnSearch, SIGNAL(clicked()), this, SLOT(startSearch()));
  connect (sigman, SIGNAL(signal_searchResult(ICQEvent *)),
           this, SLOT(slot_searchResult(ICQEvent *)));
  
  // pseudo Status Bar
  lblSearch = new QLabel(tr("Enter search parameters and select 'Search'"), this);
  lblSearch->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
  
  // post-search widgets
  
  top_lay->addSpacing(10);
  
  foundView = new SearchUserView(this);
  top_lay->addWidget(foundView);
  
  lay = new QHBoxLayout(top_lay, 20);
  
  qcbAlertUser = new QCheckBox(tr("A&lert User"), this);
  qcbAlertUser->setChecked(true);
  
  lay->addWidget(qcbAlertUser);
  
  lay->addStretch(1);
  
  btnAdd = new QPushButton(tr("&Add User"), this);
  btnAdd->setEnabled(false);
  lay->addWidget(btnAdd);
  
  btnDone = new QPushButton(tr("&Done"), this);
  lay->addWidget(btnDone);
  
  connect (btnDone, SIGNAL(clicked()), this, SLOT(accept()));
  connect (foundView, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(addUser()));
  connect (btnAdd, SIGNAL(clicked()), this, SLOT(addUser()));
  connect (btnSearchAgain, SIGNAL(clicked()), this, SLOT(resetSearch()));
  
  top_lay->addWidget(lblSearch);
}

SearchUserDlg::~SearchUserDlg()
{
  delete btnSearch; btnSearch = 0;
  delete btnCancel; btnCancel = 0;
  delete qcbAlertUser; qcbAlertUser = 0;
  delete edtEmail; edtEmail = 0;
  delete edtFirst; edtFirst = 0;
  delete edtLast; edtLast = 0;
  delete edtNick; edtNick = 0;
  delete edtUin; edtUin = 0;
  delete btnDone; btnDone = 0;
  delete btnAdd; btnAdd = 0;
  delete btnSearchAgain; btnSearchAgain =0;
  delete foundView; foundView = 0;
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
  btnCancel->setEnabled(false);
  btnSearch->setEnabled(false);
  btnAdd->setEnabled(false);
  
  if(uin_search && (uin >= 1000 && uin <= 999999999)) {
    // we should do a uin search here
    
    UserBasicInfo us;
    memset(&us, 0, sizeof(UserBasicInfo));
    
    sprintf(us.uin, "%ld", uin);
    
    searchFound(&us);
    searchDone(0);
  }
  else {
    searchSequence = server->icqStartSearch(edtNick->text(), edtFirst->text(),
                                            edtLast->text(), edtEmail->text());
    lblSearch->setText(tr("Searching (this can take awhile)..."));
  }
}

void SearchUserDlg::show()
{
  resetSearch();
  QDialog::show();
}


void SearchUserDlg::hide()
{
  QDialog::hide();
  delete this;
}


void SearchUserDlg::resetSearch()
{
  edtNick->setEnabled(true);
  edtFirst->setEnabled(true);
  edtLast->setEnabled(true);
  edtEmail->setEnabled(true);
  edtUin->setEnabled(true);
  edtEmail->setText("");
  edtLast->setText("");
  edtFirst->setText("");
  edtNick->setText("");
  edtUin->setText("");
  btnCancel->setEnabled(true);
  btnSearch->setEnabled(true);
  btnAdd->setEnabled(false);
  btnSearchAgain->setEnabled(false);
  foundView->clear();
  lblSearch->setText(tr("Enter search parameters and select 'Search'"));
}


void SearchUserDlg::slot_searchResult(ICQEvent *e)
{
  if (e->m_nSubSequence != searchSequence) return;
  
  if (e->m_eResult == EVENT_SUCCESS)
    searchDone(e->m_sSearchAck->cMore);
  else if (e->m_eResult == EVENT_ACKED)
    searchFound(e->m_sSearchAck->sBasicInfo);
  else
    searchFailed();
}

void SearchUserDlg::searchFound(struct UserBasicInfo *us)
{
  (void) new SearchItem(us, foundView);
  if (!btnAdd->isEnabled()) btnAdd->setEnabled(true);
}


void SearchUserDlg::searchDone(char more)
{
  if (more == (char)1)
    lblSearch->setText(tr("More users found. Narrow search."));
  else
    lblSearch->setText("Search complete.");
  btnSearchAgain->setEnabled(true);
}


void SearchUserDlg::searchFailed(void)
{
  lblSearch->setText(tr("Search failed."));
  btnSearchAgain->setEnabled(true);
}

void SearchUserDlg::addUser()
{
  unsigned long uin = foundView->currentUin();
  ICQUser* user = 0;
  
  // no user selected --> return
  if (uin == 0) return;
  
  // user already there
  if((user = gUserManager.FetchUser(uin, LOCK_N))) {
    QString msg = QString(tr("Sorry, but this user is already\non your "
                            "contact list as\n'%1'\n\nYou can't add a user twice.")).arg(user->getAlias());
    
    QMessageBox::warning(this, "Licq - Warning", msg, tr("&OK"));
    return;
  }
  
  // Ok, let's add him
  server->AddUserToList(uin);
  if (qcbAlertUser->isChecked()) // alert the user they were added
    server->icqAlertUser(uin);
  resetSearch();
}

#include "moc/moc_searchuserdlg.h"
