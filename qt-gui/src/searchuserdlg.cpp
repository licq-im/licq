// Search user code base written by Alan Penner (apenner@andrew.cmu.edu)
// modified by Graham Roff

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>
#include <qhbox.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qtabwidget.h>
#include <iostream.h>

#include "searchuserdlg.h"
#include "licq-locale.h"


SearchUserView::SearchUserView(QWidget *parent = NULL, char *name = NULL) : QListView(parent, name)
{
  addColumn(_("Alias"), 105);
  addColumn(_("UIN"), 70);
  setColumnAlignment(1, AlignRight);
  addColumn(_("Name"), 120);
  addColumn(_("Email"), 120);
  setAllColumnsShowFocus (true);
  setMinimumHeight(150);
  setMinimumWidth(440);
}


unsigned long SearchUserView::currentUin(void)
{
   if (currentItem() == NULL) return (0);
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
                             QWidget *parent = 0, const char *name = 0)
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

  lblNick = new QLabel(_("Alias:"), alias_tab);
  grid_lay->addWidget(lblNick, 1, 1);

  edtNick = new QLineEdit(alias_tab);
  grid_lay->addWidget(edtNick, 1, 3);

  lblFirst = new QLabel(_("First Name:"), alias_tab);
  grid_lay->addWidget(lblFirst, 3, 1);

  edtFirst = new QLineEdit(alias_tab);
  grid_lay->addWidget(edtFirst, 3, 3);

  lblLast = new QLabel(_("Last Name:"), alias_tab);
  grid_lay->addWidget(lblLast, 5, 1);

  edtLast = new QLineEdit(alias_tab);
  grid_lay->addWidget(edtLast, 5, 3);

  search_tab->addTab(alias_tab, _("&Name"));

  //-- second tab: search by email

  email_tab  = new QWidget(this);
  QBoxLayout* lay2 = new QHBoxLayout(email_tab, 10);

  lblEmail = new QLabel(_("Email Address:"), email_tab);
  lay2->addWidget(lblEmail);

  edtEmail = new QLineEdit(email_tab);
  lay2->addWidget(edtEmail);

  search_tab->addTab(email_tab, _("&Email"));

  //-- third tab: search by UIN

  uin_tab = new QWidget(this);
  lay2 = new QHBoxLayout(uin_tab, 10);

  lblUin = new QLabel(_("UIN#:"), uin_tab);
  lay2->addWidget(lblUin);

  edtUin = new QLineEdit(uin_tab);
  lay2->addWidget(edtUin);

  search_tab->addTab(uin_tab, _("&Uin#"));

  lay->addWidget(search_tab, 1);

  lay2 = new QVBoxLayout(lay, 10);

  btnSearch = new QPushButton(_("&Search"), this);
  lay2->addWidget(btnSearch);

  btnSearchAgain = new QPushButton(_("Search Again"), this);
  btnSearchAgain->setEnabled(false);
  lay2->addWidget(btnSearchAgain);

  btnCancel = new QPushButton(_("&Cancel"), this);
  lay2->addWidget(btnCancel);

  connect (btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect (btnSearch, SIGNAL(clicked()), this, SLOT(startSearch()));
  /*  connect (sigman, SIGNAL(signal_searchFound(unsigned short, struct UserBasicInfo *)),
      this, SLOT(slot_searchFound(unsigned short, struct UserBasicInfo *)));
      connect (sigman, SIGNAL(signal_doneSearch(unsigned short, char)),
      this, SLOT(slot_searchDone(unsigned short, char)));*/
  connect (sigman, SIGNAL(signal_searchResult(ICQEvent *)),
           this, SLOT(slot_searchResult(ICQEvent *)));

  lblSearch = new QLabel(_("Searching (this can take awhile)..."), this);
  top_lay->addWidget(lblSearch);

  // post-search widgets

  foundView = new SearchUserView(this);
  top_lay->addWidget(foundView);

  lay = new QHBoxLayout(top_lay, 20);

  qcbAlertUser = new QCheckBox(_("A&lert User"), this);
  qcbAlertUser->setChecked(true);

  lay->addWidget(qcbAlertUser);

  lay->addStretch(1);

  btnAdd = new QPushButton(_("&Add User"), this);
  btnAdd->setEnabled(false);
  lay->addWidget(btnAdd);

  btnDone = new QPushButton(_("&Done"), this);
  lay->addWidget(btnDone);

  connect (btnDone, SIGNAL(clicked()), this, SLOT(accept()));
  connect (foundView, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(addUser()));
  connect (btnAdd, SIGNAL(clicked()), this, SLOT(addUser()));
  connect (btnSearchAgain, SIGNAL(clicked()), this, SLOT(resetSearch()));

/*
  server = s;
  sigman = theSigMan;
  setCaption("Licq User Search");

  // pre-search widgets

   lblEmail = new QLabel(_("Email Address:"), this);
   lblEmail->setGeometry(25, 5, 80, 20);
   edtEmail = new QLineEdit(this);
   edtEmail->setGeometry(115, 5, 180, 20);

   OtherBox = new QGroupBox(this);
   OtherBox->setFrameStyle(QFrame::HLine | QFrame::Raised);
   OtherBox->setGeometry(5, 32, 450, 2);

   lblFirst = new QLabel(_("First Name:"), this);
   lblFirst->setGeometry(25, 40, 80, 20);
   edtFirst = new QLineEdit(this);
   edtFirst->setGeometry(115, 40, 180, 20);

   lblLast = new QLabel(_("Last Name:"), this);
   lblLast->setGeometry(25, 65, 80, 20);
   edtLast = new QLineEdit(this);
   edtLast->setGeometry(115, 65, 180, 20);

   lblNick = new QLabel(_("Alias:"), this);
   lblNick->setGeometry(25, 90, 80, 20);
   edtNick = new QLineEdit(this);
   edtNick->setGeometry(115, 90, 180, 20);

   btnSearch = new QPushButton(_("&Search"), this);
   btnSearch->setGeometry(80, 120, 80, 30);
   btnCancel = new QPushButton(_("&Cancel"), this);
   btnCancel->setGeometry(180, 120, 80, 30);

   connect (btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
   connect (btnSearch, SIGNAL(clicked()), this, SLOT(startSearch()));
   connect (sigman, SIGNAL(signal_searchResult(ICQEvent *)),
            this, SLOT(slot_searchResult(ICQEvent *)));


   // post-search widgets

   foundView = new SearchUserView(this);
   foundView->setGeometry(10, 200, 340, 180);

   btnDone = new QPushButton(_("&Done"), this);
   btnDone->setGeometry(220, 400, 80, 30);

   btnAdd = new QPushButton(_("&Add User"), this);
   btnAdd->setGeometry(30, 400, 80, 30);
   btnAdd->setEnabled(false);

   btnSearchAgain = new QPushButton(_("Search Again"), this);
   btnSearchAgain->setGeometry(120, 400, 90, 30);
   btnSearchAgain->setEnabled(false);

   lblSearch = new QLabel(_("Searching (this can take awhile)..."), this);
   lblSearch->setGeometry(10, 170, 215, 20);
   //foundView->setStyle(WindowsStyle);
   foundView->setFrameStyle(QFrame::Box | QFrame::Raised);

   connect (btnDone, SIGNAL(clicked()), this, SLOT(accept()));
   connect (foundView, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(addUser()));
   connect (btnAdd, SIGNAL(clicked()), this, SLOT(addUser()));
   connect (btnSearchAgain, SIGNAL(clicked()), this, SLOT(resetSearch()));*/
}


void SearchUserDlg::startSearch()
{
   searchSequence = server->icqStartSearch(edtNick->text(),
                          edtFirst->text(),
                          edtLast->text(),
                          edtEmail->text());
   foundView->clear();
   edtNick->setEnabled(false);
   edtFirst->setEnabled(false);
   edtLast->setEnabled(false);
   edtEmail->setEnabled(false);
   edtUin->setEnabled(false);
   btnCancel->setEnabled(false);
   btnSearch->setEnabled(false);
   btnAdd->setEnabled(false);
   lblSearch->setText(_("Searching (this can take awhile)..."));
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
    lblSearch->setText(_("More users found. Narrow search."));
  else
    lblSearch->setText("Search complete.");
  btnSearchAgain->setEnabled(true);
}


void SearchUserDlg::searchFailed(void)
{
  lblSearch->setText(_("Search failed."));
  btnSearchAgain->setEnabled(true);
}

void SearchUserDlg::addUser()
{
   unsigned long uin = foundView->currentUin();
   if (uin == 0) return;
   server->AddUserToList(uin);
   if (qcbAlertUser->isChecked()) // alert the user they were added
       server->icqAlertUser(uin);
   foundView->clear();
}

#include "moc/moc_searchuserdlg.h"
