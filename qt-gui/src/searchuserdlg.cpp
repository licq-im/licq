// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2006 Licq developers
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
#include <qvbuttongroup.h>
#include <qvalidator.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qtextcodec.h>

#include "ewidgets.h"
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
  addColumn(tr("Sex & Age"), 70);
  addColumn(tr("Authorize"), 65);

  setAllColumnsShowFocus (true);
  setMinimumHeight(150);
  setMinimumWidth(600);
  setSelectionMode(QListView::Extended);
  setShowSortIndicator(true);
}


SearchItem::SearchItem(const CSearchAck* s, const QString& encoding, QListView* parent)
  : QListViewItem(parent)
{
  QString qsStatus;
  QString qsGender;
  QString qsAge;
  QString qsAuth;

  myId = s->Id();
  myPpid = s->PPID();
  QTextCodec *codec = QTextCodec::codecForName(encoding);
  if (codec == 0)
    codec = QTextCodec::codecForLocale();
  setText(0, codec->toUnicode(s->Alias()));
  setText(1, s->Id());
  setText(2, codec->toUnicode(s->FirstName()) + QString(" ") + codec->toUnicode(s->LastName()));
  setText(3, s->Email());

  switch (s->Status())
  {
    case 0:
      qsStatus = SearchUserView::tr("Offline");
      break;
   case 1:
      qsStatus = SearchUserView::tr("Online");
      break;
   case 2:
   default:
      qsStatus = SearchUserView::tr("Unknown");
  }
  setText(4, qsStatus);
  
  switch (s->Gender())
  {
    case 1:
      qsGender = SearchUserView::tr("F");
      break;
    case 2:
      qsGender = SearchUserView::tr("M");
      break;
    default:			// unknown
      qsGender = SearchUserView::tr("?");
  }
  qsAge = (s->Age()) ? QString::number(s->Age()) : QChar('?');
  setText(5, qsGender + '/' + qsAge);
  
  qsAuth =(s->Auth() ? SearchUserView::tr("No") : SearchUserView::tr("Yes"));
  setText(6, qsAuth);
}

// -----------------------------------------------------------------------------

SearchUserDlg::SearchUserDlg(CMainWindow *m, CICQDaemon *s, CSignalManager *theSigMan, const QString &encoding)
  : QWidget(NULL, "SearchUserDialog", WDestructiveClose)
{
  mainwin = m;
  server = s;
  sigman = theSigMan;
  m_Encoding = encoding;
  setCaption(tr("Licq - User Search"));
  searchTag = 0;

  QBoxLayout* top_lay = new QVBoxLayout(this, 6);
  QBoxLayout* lay = new QHBoxLayout(top_lay, 10);

  // pre-search widgets
  search_tab = new CETabWidget(this);

  //-- first tab: search by Alias/name
  whitepages_tab = new QWidget(this);
  QGridLayout* grid_lay = new QGridLayout(whitepages_tab, 17, 7, 10, 5);
  grid_lay->addColSpacing(0, 10);
  grid_lay->addColSpacing(2, 10);
  grid_lay->addColSpacing(4, 10);

  unsigned short CR = 0;

  grid_lay->addWidget(new QLabel(tr("Alias:"), whitepages_tab), ++CR, 1);
  edtNick = new QLineEdit(whitepages_tab);
  grid_lay->addWidget(edtNick, CR, 3);
  grid_lay->addWidget(new QLabel(tr("First Name:"), whitepages_tab), ++CR, 1);
  edtFirst = new QLineEdit(whitepages_tab);
  grid_lay->addWidget(edtFirst, CR, 3);
  grid_lay->addWidget(new QLabel(tr("Last Name:"), whitepages_tab), ++CR, 1);
  edtLast = new QLineEdit(whitepages_tab);
  grid_lay->addWidget(edtLast, CR, 3);
  grid_lay->addWidget(new QLabel(tr("Age Range:"), whitepages_tab), ++CR, 1);
  cmbAge = new QComboBox(false, whitepages_tab);
  cmbAge->insertItem(tr("Unspecified"), 0);
  cmbAge->insertItem(tr("18 - 22"), 1);
  cmbAge->insertItem(tr("23 - 29"), 2);
  cmbAge->insertItem(tr("30 - 39"), 3);
  cmbAge->insertItem(tr("40 - 49"), 4);
  cmbAge->insertItem(tr("50 - 59"), 5);
  cmbAge->insertItem(tr("60+"), 6);
  cmbAge->setFixedWidth(cmbAge->sizeHint().width());
  grid_lay->addWidget(cmbAge, CR, 3);
  grid_lay->addWidget(new QLabel(tr("Gender:"), whitepages_tab), ++CR, 1);
  cmbGender = new QComboBox(false, whitepages_tab);
  cmbGender->insertItem(tr("Unspecified"), GENDER_UNSPECIFIED);
  cmbGender->insertItem(tr("Female"), GENDER_FEMALE);
  cmbGender->insertItem(tr("Male"), GENDER_MALE);
  cmbGender->setFixedWidth(cmbGender->sizeHint().width());
  grid_lay->addWidget(cmbGender, CR, 3);

  grid_lay->addWidget(new QLabel(tr("Language:"), whitepages_tab), ++CR, 1);
  cmbLanguage = new QComboBox(false, whitepages_tab);
  cmbLanguage->setFixedWidth(cmbGender->sizeHint().width());
  for (unsigned short i = 0; i < NUM_LANGUAGES; i++)
    cmbLanguage->insertItem(GetLanguageByIndex(i)->szName);
  grid_lay->addWidget(cmbLanguage, CR, 3);

  CR = 0;

  grid_lay->addWidget(new QLabel(tr("City:"), whitepages_tab), ++CR, 5);
  edtCity = new QLineEdit(whitepages_tab);
  grid_lay->addWidget(edtCity, CR, 7);
  grid_lay->addWidget(new QLabel(tr("State:"), whitepages_tab), ++CR, 5);
  edtState = new QLineEdit(whitepages_tab);
  grid_lay->addWidget(edtState, CR, 7);

  grid_lay->addWidget(new QLabel(tr("Country:"), whitepages_tab), ++CR, 5);
  cmbCountry = new QComboBox(false, whitepages_tab);
  cmbCountry->setFixedWidth(cmbGender->sizeHint().width());
  for (unsigned short i = 0; i < NUM_COUNTRIES; i++)
    cmbCountry->insertItem(GetCountryByIndex(i)->szName);
  grid_lay->addWidget(cmbCountry, CR, 7);

  grid_lay->addWidget(new QLabel(tr("Company Name:"), whitepages_tab), ++CR, 5);
  edtCoName = new QLineEdit(whitepages_tab);
  grid_lay->addWidget(edtCoName, CR, 7);
  grid_lay->addWidget(new QLabel(tr("Company Department:"), whitepages_tab), ++CR, 5);
  edtCoDept = new QLineEdit(whitepages_tab);
  grid_lay->addWidget(edtCoDept, CR, 7);
  grid_lay->addWidget(new QLabel(tr("Company Position:"), whitepages_tab), ++CR, 5);
  edtCoPos = new QLineEdit(whitepages_tab);
  grid_lay->addWidget(edtCoPos, CR, 7);

  grid_lay->addWidget(new QLabel(tr("Email Address:"), whitepages_tab), ++CR, 1);
  edtEmail = new QLineEdit(whitepages_tab);
  grid_lay->addMultiCellWidget(edtEmail, CR, CR, 3, 7);
  CR++;
  grid_lay->addWidget(new QLabel(tr("Keyword:"), whitepages_tab), ++CR, 1);
  edtKeyword = new QLineEdit(whitepages_tab);
  grid_lay->addMultiCellWidget(edtKeyword, CR, CR, 3, 7);
  CR++;
  
  chkOnlineOnly = new QCheckBox(tr("Return Online Users Only"), whitepages_tab);
  grid_lay->addMultiCellWidget(chkOnlineOnly, CR, CR, 1, 7);

  search_tab->addTab(whitepages_tab, tr("&Whitepages"));

  uin_tab = new QWidget(this);
  QBoxLayout *misc_lay = new QHBoxLayout(uin_tab, 10);
  misc_lay->addWidget(new QLabel(tr("UIN#:"), uin_tab));
  edtUin = new QLineEdit(uin_tab);
  edtUin->setValidator(new QIntValidator(10000,2147483647, edtUin));
  misc_lay->addWidget(edtUin);

  search_tab->addTab(uin_tab, tr("&UIN#"));

  lay->addWidget(search_tab, 1);
  misc_lay = new QVBoxLayout(lay, 10);
  misc_lay->addStretch(1);
  btnSearch = new QPushButton(tr("&Search"), this);
  btnSearch->setDefault(true);
  misc_lay->addWidget(btnSearch);

  btnReset = new QPushButton(tr("Reset Search"), this);
  btnReset->setEnabled(false);
  btnReset->setFixedSize(btnReset->sizeHint());
  misc_lay->addWidget(btnReset);

  btnDone = new QPushButton(tr("&Done"), this);
  misc_lay->addWidget(btnDone);

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

  btnInfo = new QPushButton(tr("View &Info"), this);
  btnInfo->setEnabled(false);
  lay->addWidget(btnInfo);

  btnAdd = new QPushButton(tr("&Add User"), this);
  btnAdd->setEnabled(false);
  lay->addWidget(btnAdd);

  connect(btnDone, SIGNAL(clicked()), this, SLOT(close()));
  connect(foundView, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
  connect(btnInfo, SIGNAL(clicked()), this, SLOT(viewInfo()));
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
  
  edtNick->setEnabled(false);
  edtFirst->setEnabled(false);
  edtLast->setEnabled(false);
  cmbAge->setEnabled(false);
  cmbGender->setEnabled(false);
  cmbLanguage->setEnabled(false);
  edtCity->setEnabled(false);
  edtState->setEnabled(false);
  cmbCountry->setEnabled(false);
  edtCoName->setEnabled(false);
  edtCoDept->setEnabled(false);
  edtCoPos->setEnabled(false);
  edtEmail->setEnabled(false);
  edtKeyword->setEnabled(false);
  chkOnlineOnly->setEnabled(false);
  
  edtUin->setEnabled(false);
  
  btnSearch->setEnabled(false);
  btnReset->setEnabled(true);
  btnReset->setText(tr("Cancel"));
  btnDone->setEnabled(false);
  btnInfo->setEnabled(false);
  btnAdd->setEnabled(false);

  if (search_tab->currentPage() == uin_tab)
  {
    searchTag = server->icqSearchByUin(edtUin->text().toULong());
  }
  else
  {
     QTextCodec *codec = QTextCodec::codecForName(m_Encoding);
     if (codec == 0)
      codec = QTextCodec::codecForLocale();
     searchTag = server->icqSearchWhitePages(
     codec->fromUnicode(edtFirst->text()),
     codec->fromUnicode(edtLast->text()),
     codec->fromUnicode(edtNick->text()),
     edtEmail->text().local8Bit().data(),
     mins[cmbAge->currentItem()],
     maxs[cmbAge->currentItem()],
     cmbGender->currentItem(),
     GetLanguageByIndex(cmbLanguage->currentItem())->nCode,
     codec->fromUnicode(edtCity->text()),
     codec->fromUnicode(edtState->text()),
     GetCountryByIndex(cmbCountry->currentItem())->nCode,
     codec->fromUnicode(edtCoName->text()),
     codec->fromUnicode(edtCoDept->text()),
     codec->fromUnicode(edtCoPos->text()),
     codec->fromUnicode(edtKeyword->text()),
     chkOnlineOnly->isChecked());
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
    edtNick->clear();
    edtFirst->clear();
    edtLast->clear();
    cmbAge->setCurrentItem(0);
    cmbGender->setCurrentItem(0);
    cmbLanguage->setCurrentItem(0);
    edtCity->clear();
    edtState->clear();
    cmbCountry->setCurrentItem(0);
    edtCoName->clear();
    edtCoDept->clear();
    edtCoPos->clear();
    edtEmail->clear();
    edtKeyword->clear();
    chkOnlineOnly->setChecked(false);

    edtUin->clear();

    foundView->clear();
    btnReset->setEnabled(false);
  }
  edtNick->setEnabled(true);
  edtFirst->setEnabled(true);
  edtLast->setEnabled(true);
  cmbAge->setEnabled(true);
  cmbGender->setEnabled(true);
  cmbLanguage->setEnabled(true);
  edtCity->setEnabled(true);
  edtState->setEnabled(true);
  cmbCountry->setEnabled(true);
  edtCoName->setEnabled(true);
  edtCoDept->setEnabled(true);
  edtCoPos->setEnabled(true);
  edtEmail->setEnabled(true);
  edtKeyword->setEnabled(true);
  chkOnlineOnly->setEnabled(true);

  edtUin->setEnabled(true);

  btnDone->setEnabled(true);
  btnSearch->setEnabled(true);
  btnInfo->setEnabled(false);
  btnAdd->setEnabled(false);
  lblSearch->setText(tr("Enter search parameters and select 'Search'"));
}


void SearchUserDlg::searchResult(ICQEvent *e)
{
  if (!e->Equals(searchTag)) return;

  edtNick->setEnabled(true);
  edtFirst->setEnabled(true);
  edtLast->setEnabled(true);
  cmbAge->setEnabled(true);
  cmbGender->setEnabled(true);
  cmbLanguage->setEnabled(true);
  edtCity->setEnabled(true);
  edtState->setEnabled(true);
  cmbCountry->setEnabled(true);
  edtCoName->setEnabled(true);
  edtCoDept->setEnabled(true);
  edtCoPos->setEnabled(true);
  edtEmail->setEnabled(true);
  edtKeyword->setEnabled(true);
  chkOnlineOnly->setEnabled(true);

  edtUin->setEnabled(true);

  btnSearch->setEnabled(true);
  btnDone->setEnabled(true);

  if (e->SearchAck() != NULL && e->SearchAck()->Uin() != 0)
    searchFound(e->SearchAck());

  if (e->Result() == EVENT_SUCCESS)
    searchDone(e->SearchAck());
  else if (e->Result() != EVENT_ACKED)
    searchFailed();
}

void SearchUserDlg::searchFound(const CSearchAck* s)
{
  (void) new SearchItem(s, m_Encoding, foundView);
}

void SearchUserDlg::searchDone(const CSearchAck* sa)
{
  if (sa == NULL || sa->More() == 0)
    lblSearch->setText(tr("Search complete."));
  else if (sa->More() == (unsigned long)(-1))
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

  btnInfo->setEnabled(true);
  btnAdd->setEnabled(true);
  switch(count) {
  case 0:
    btnInfo->setEnabled(false);
    btnAdd->setEnabled(false);
    // fall through
  case 1:
    btnAdd->setText(tr("&Add User"));
    break;
  default:
    btnAdd->setText(tr("&Add %1 Users").arg(count));
  }
}

void SearchUserDlg::viewInfo()
{
  SearchItem *current = static_cast<SearchItem *>(foundView->firstChild());

  while (current)
  {
    if (current->isSelected())
    {
      ICQUser* u = gUserManager.FetchUser(current->id().latin1(), current->ppid(), LOCK_R);
      if (!u)
        server->AddUserToList(current->id().latin1(), current->ppid(), false, true);
      else
        gUserManager.DropUser(u);

      mainwin->callInfoTab(mnuUserGeneral, current->id().latin1(), current->ppid(), false, true);
      break;
    }
    current = static_cast<SearchItem*>(current->nextSibling());
  }
}

void SearchUserDlg::addUser()
{
  SearchItem* current = static_cast<SearchItem*>(foundView->firstChild());

  while (current)
  {
    if (current->isSelected())
    {
      ICQUser* user = gUserManager.FetchUser(current->id().latin1(), current->ppid(), LOCK_R);

      if (user)
      {
        bool tempUser = user->NotInList();
        gUserManager.DropUser(user);
        if (tempUser)
          gUserManager.RemoveUser(current->id().latin1(), current->ppid());
      }

      if (server->AddUserToList(current->id().latin1(), current->ppid()) &&
          qcbAlertUser->isChecked()) // alert the user they were added
        server->icqAlertUser(current->id().latin1(), current->ppid());
    }
    current = static_cast<SearchItem*>(current->nextSibling());
  }

  foundView->triggerUpdate();
  selectionChanged();
}

#include "searchuserdlg.moc"
