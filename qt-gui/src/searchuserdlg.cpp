// -*- c-basic-offset: 2 -*-
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
#include <qpainter.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qtabwidget.h>
#include <qvbuttongroup.h>
#include <qvalidator.h>
#include <qcombobox.h>
#include <qspinbox.h>

#include "searchuserdlg.h"
#include "sigman.h"

#include "licq_user.h"
#include "licq_icqd.h"
#include "licq_languagecodes.h"
#include "licq_countrycodes.h"


// -----------------------------------------------------------------------------

SearchUserView::SearchUserView(QWidget *parent)
  : QListView(parent, "SearchUserDialog")
{
  addColumn(tr("Alias"), 105);
  addColumn(tr("UIN"), 70);
  setColumnAlignment(1, AlignRight);
  addColumn(tr("Name"), 120);
  addColumn(tr("Email"), 120);
  addColumn(tr("Status"), 70);

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
  char szStatus[8];
  uinVal = s->Uin();
  setText(0, QString::fromLocal8Bit(s->Alias()));
  setText(1, QString::number(s->Uin()));
  setText(2, QString::fromLocal8Bit(s->FirstName()) + QString(" ") + QString::fromLocal8Bit(s->LastName()));
  setText(3, s->Email());

  switch (s->Status())
  {
    case 0:
      strcpy(szStatus, "Offline");
      break;
   case 1:
      strcpy(szStatus, "Online");
      break;
   case 2:
   default:
      strcpy(szStatus, "Unknown");
  }
  setText(4, QString::fromLocal8Bit(szStatus));
}


unsigned long SearchItem::uin()
{
  return (uinVal);
}


// -----------------------------------------------------------------------------

SearchUserDlg::SearchUserDlg(CICQDaemon *s, CSignalManager *theSigMan,
                             QWidget *parent)
  : QWidget(parent, "SearchUserDialog", WDestructiveClose)
{
  server = s;
  sigman = theSigMan;
  setCaption(tr("Licq - User Search"));
  searchTag = 0;

  QBoxLayout* top_lay = new QVBoxLayout(this, 6);
  QBoxLayout* lay = new QHBoxLayout(top_lay, 10);

  // pre-search widgets
  search_tab = new QTabWidget(this);

  //-- first tab: search by Alias/name
  alias_tab = new QWidget(this);
  QGridLayout* grid_lay = new QGridLayout(alias_tab, 15, 7, 10, 5);
  grid_lay->addColSpacing(0, 10);
  grid_lay->addColSpacing(2, 10);
  grid_lay->addColSpacing(4, 10);

  unsigned short CR = 0;

  grid_lay->addWidget(new QLabel(tr("Alias:"), alias_tab), ++CR, 1);
  edtNick = new QLineEdit(alias_tab);
  grid_lay->addWidget(edtNick, CR, 3);
  grid_lay->addWidget(new QLabel(tr("First Name:"), alias_tab), ++CR, 1);
  edtFirst = new QLineEdit(alias_tab);
  grid_lay->addWidget(edtFirst, CR, 3);
  grid_lay->addWidget(new QLabel(tr("Last Name:"), alias_tab), ++CR, 1);
  edtLast = new QLineEdit(alias_tab);
  grid_lay->addWidget(edtLast, CR, 3);
  grid_lay->addWidget(new QLabel(tr("Age Range:"), alias_tab), ++CR, 1);
  cmbAge = new QComboBox(false, alias_tab);
  cmbAge->insertItem(tr("Unspecified"), 0);
  cmbAge->insertItem(tr("18 - 22"), 1);
  cmbAge->insertItem(tr("23 - 29"), 2);
  cmbAge->insertItem(tr("30 - 39"), 3);
  cmbAge->insertItem(tr("40 - 49"), 4);
  cmbAge->insertItem(tr("50 - 59"), 5);
  cmbAge->insertItem(tr("69+"), 6);
  cmbAge->setFixedWidth(cmbAge->sizeHint().width());
  grid_lay->addWidget(cmbAge, CR, 3);
  grid_lay->addWidget(new QLabel(tr("Gender:"), alias_tab), ++CR, 1);
  cmbGender = new QComboBox(false, alias_tab);
  cmbGender->insertItem(tr("Unspecified"), GENDER_UNSPECIFIED);
  cmbGender->insertItem(tr("Female"), GENDER_FEMALE);
  cmbGender->insertItem(tr("Male"), GENDER_MALE);
  cmbGender->setFixedWidth(cmbGender->sizeHint().width());
  grid_lay->addWidget(cmbGender, CR, 3);

  grid_lay->addWidget(new QLabel(tr("Language:"), alias_tab), ++CR, 1);
  cmbLanguage = new QComboBox(false, alias_tab);
  cmbLanguage->setFixedWidth(cmbGender->sizeHint().width());
  for (unsigned short i = 0; i < NUM_LANGUAGES; i++)
    cmbLanguage->insertItem(gLanguages[i].szName);
  grid_lay->addWidget(cmbLanguage, CR, 3);

  CR = 0;

  grid_lay->addWidget(new QLabel(tr("City:"), alias_tab), ++CR, 5);
  edtCity = new QLineEdit(alias_tab);
  grid_lay->addWidget(edtCity, CR, 7);
  grid_lay->addWidget(new QLabel(tr("State:"), alias_tab), ++CR, 5);
  edtState = new QLineEdit(alias_tab);
  grid_lay->addWidget(edtState, CR, 7);

  grid_lay->addWidget(new QLabel(tr("Country:"), alias_tab), ++CR, 5);
  cmbCountry = new QComboBox(false, alias_tab);
  cmbCountry->setFixedWidth(cmbGender->sizeHint().width());
  for (unsigned short i = 0; i < NUM_COUNTRIES; i++)
    cmbCountry->insertItem(gCountries[i].szName);
  grid_lay->addWidget(cmbCountry, CR, 7);

  grid_lay->addWidget(new QLabel(tr("Company Name:"), alias_tab), ++CR, 5);
  edtCoName = new QLineEdit(alias_tab);
  grid_lay->addWidget(edtCoName, CR, 7);
  grid_lay->addWidget(new QLabel(tr("Company Department:"), alias_tab), ++CR, 5);
  edtCoDept = new QLineEdit(alias_tab);
  grid_lay->addWidget(edtCoDept, CR, 7);
  grid_lay->addWidget(new QLabel(tr("Company Position:"), alias_tab), ++CR, 5);
  edtCoPos = new QLineEdit(alias_tab);
  grid_lay->addWidget(edtCoPos, CR, 7);

  CR++;
  chkOnlineOnly = new QCheckBox(tr("Return Online Users Only"), alias_tab);
  grid_lay->addMultiCellWidget(chkOnlineOnly, CR, CR, 1, 7);

  search_tab->addTab(alias_tab, tr("&Whitepages"));

  //-- second tab: search by email

  email_tab  = new QWidget(this);
  QBoxLayout* lay2 = new QHBoxLayout(email_tab, 10);
  lay2->addWidget(new QLabel(tr("Email Address:"), email_tab));
  edtEmail = new QLineEdit(email_tab);
  lay2->addWidget(edtEmail);

  search_tab->addTab(email_tab, tr("&Email"));

  //-- third tab: search by UIN
  uin_tab = new QWidget(this);
  lay2 = new QHBoxLayout(uin_tab, 10);
  lay2->addWidget(new QLabel(tr("UIN#:"), uin_tab));
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
  btnReset->setFixedSize(btnReset->sizeHint());
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
}


void SearchUserDlg::startSearch()
{
  unsigned short mins[7] = {0, 18, 23, 30, 40, 50, 60};
  unsigned short maxs[7] = {0, 22, 29, 39, 49, 59, 120};

  foundView->clear();
  /* FIXME
  edtNick->setEnabled(false);
  edtFirst->setEnabled(false);
  edtLast->setEnabled(false);
  */
  edtEmail->setEnabled(false);
  edtUin->setEnabled(false);
  btnSearch->setEnabled(false);
  btnReset->setEnabled(true);
  btnReset->setText(tr("Cancel"));
  btnDone->setEnabled(false);
  btnAdd->setEnabled(false);

  if (search_tab->currentPage() == uin_tab)
  {
    searchTag = server->icqSearchByUin(edtUin->text().toULong());
  }
  else if (search_tab->currentPage() == email_tab)
  {
    searchTag = server->icqSearchWhitePages(
     edtFirst->text().local8Bit().data(),
     edtLast->text().local8Bit().data(),
     edtNick->text().local8Bit().data(),
     edtEmail->text().local8Bit().data(),
     0, 0, GENDER_UNSPECIFIED, LANGUAGE_UNSPECIFIED,
     "", "", COUNTRY_UNSPECIFIED, "", "", "", false);
  }
  else
  {
     searchTag = server->icqSearchWhitePages(
     edtFirst->text().local8Bit().data(),
     edtLast->text().local8Bit().data(),
     edtNick->text().local8Bit().data(),
     "",
     mins[cmbAge->currentItem()],
     maxs[cmbAge->currentItem()],
     cmbGender->currentItem(),
     GetLanguageByIndex(cmbLanguage->currentItem())->nCode,
     edtCity->text().local8Bit().data(),
     edtState->text().local8Bit().data(),
     GetCountryByIndex(cmbCountry->currentItem())->nCode,
     edtCoName->text().local8Bit().data(),
     edtCoDept->text().local8Bit().data(),
     edtCoPos->text().local8Bit().data(), chkOnlineOnly->isChecked());
  }
  lblSearch->setText(tr("Searching (this can take awhile)..."));
}

void SearchUserDlg::resetSearch()
{
  if (searchTag)
  {
    searchTag = 0;
    btnReset->setText(tr("Reset Search"));
  }
  else
  {
    edtEmail->clear();
    edtLast->clear();
    edtFirst->clear();
    edtNick->clear();
    edtUin->clear();
    foundView->clear();
    btnReset->setEnabled(false);
  }
  btnSearch->setEnabled(true);
  btnDone->setEnabled(true);
  edtNick->setEnabled(true);
  edtFirst->setEnabled(true);
  edtLast->setEnabled(true);
  edtEmail->setEnabled(true);
  edtUin->setEnabled(true);
  btnSearch->setEnabled(true);
  btnAdd->setEnabled(false);
  lblSearch->setText(tr("Enter search parameters and select 'Search'"));
}


void SearchUserDlg::searchResult(ICQEvent *e)
{
  if (!e->Equals(searchTag)) return;

  btnSearch->setEnabled(true);
  btnDone->setEnabled(true);
  edtNick->setEnabled(true);
  edtFirst->setEnabled(true);
  edtLast->setEnabled(true);
  edtEmail->setEnabled(true);
  edtUin->setEnabled(true);

  if (e->SearchAck() != NULL && e->SearchAck()->Uin() != 0)
    searchFound(e->SearchAck());

  if (e->Result() == EVENT_SUCCESS)
    searchDone(e->SearchAck());
  else if (e->Result() != EVENT_ACKED)
    searchFailed();
}

void SearchUserDlg::searchFound(CSearchAck *s)
{
  (void) new SearchItem(s, foundView);
}


void SearchUserDlg::searchDone(CSearchAck *sa)
{
  if (sa == NULL || sa->More() == 0)
    lblSearch->setText("Search complete.");
  else if (sa->More() == -1)
    lblSearch->setText(tr("More users found. Narrow search."));
  else
    lblSearch->setText(tr("%1 more users found. Narrow search.").arg(sa->More()));

  searchTag = 0;
  btnReset->setText(tr("Reset Search"));
}


void SearchUserDlg::searchFailed()
{
  lblSearch->setText(tr("Search failed."));
  searchTag = 0;
  btnReset->setText(tr("Reset Search"));
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
