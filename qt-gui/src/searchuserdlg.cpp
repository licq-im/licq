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
   addColumn(_("Alias"), 110);
   addColumn(_("UIN"), 70);
   setColumnAlignment(1, AlignRight);
   addColumn(_("Name"), 150);
   addColumn(_("Email"), 200);
   setAllColumnsShowFocus (true); 
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
    QTabWidget* search_tab = new QTabWidget(this);
    
    //-- first tab: search by email
    
    QWidget* email_tab  = new QWidget(this);
    QBoxLayout* lay2 = new QHBoxLayout(email_tab, 20);
    
    lblEmail = new QLabel(_("Email Address:"), email_tab);
    lay2->addWidget(lblEmail);
    
    edtEmail = new QLineEdit(email_tab);
    lay2->addWidget(edtEmail);
    
    search_tab->addTab(email_tab, _("&Email"));
    
    //-- second tab: search by Alias/name
    
    QWidget* alias_tab = new QWidget(this);
    
    QGridLayout* grid_lay = new QGridLayout(alias_tab, 10, 10);
    grid_lay->addColSpacing(0, 10);grid_lay->addRowSpacing(0, 10);
    grid_lay->addColSpacing(2, 10);grid_lay->addRowSpacing(2, 10);
    grid_lay->addColSpacing(4, 10);grid_lay->addRowSpacing(4, 10);
    grid_lay->addColSpacing(6, 10);grid_lay->addRowSpacing(6, 10);
    
    lblFirst = new QLabel(_("First Name:"), alias_tab);
    grid_lay->addWidget(lblFirst, 1, 1);
    
    edtFirst = new QLineEdit(alias_tab);
    grid_lay->addWidget(edtFirst, 1, 3);
    
    lblLast = new QLabel(_("Last Name:"), alias_tab);
    grid_lay->addWidget(lblLast, 3, 1);
    
    edtLast = new QLineEdit(alias_tab);
    grid_lay->addWidget(edtLast, 3, 3);
    
    lblNick = new QLabel(_("Alias:"), alias_tab);
    grid_lay->addWidget(lblNick, 5, 1);
    
    edtNick = new QLineEdit(alias_tab);
    grid_lay->addWidget(edtNick, 5, 3);
    
    search_tab->addTab(alias_tab, _("&Name"));

    //-- third tab: search by UIN

    QWidget* uin_tab = new QWidget(this);
    lay2 = new QHBoxLayout(uin_tab, 20);
    
    lblUin = new QLabel(_("UIN#:"), uin_tab);
    edtUin = new QLineEdit(uin_tab);
    
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
/*   connect (sigman, SIGNAL(signal_searchFound(unsigned short,  struct UserBasicInfo *)),
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
    lay->addStretch(1);

    btnAdd = new QPushButton(_("&Add User"), this);
    btnAdd->setEnabled(false);
    lay->addWidget(btnAdd);
    
    btnDone = new QPushButton(_("&Done"), this);
    lay->addWidget(btnDone);
    
    //foundView->setStyle(WindowsStyle);
    foundView->setFrameStyle(QFrame::Box | QFrame::Raised);
    
    connect (btnDone, SIGNAL(clicked()), this, SLOT(accept()));
    connect (foundView, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(addUser()));
    connect (btnAdd, SIGNAL(clicked()), this, SLOT(addUser()));
    connect (btnSearchAgain, SIGNAL(clicked()), this, SLOT(resetSearch()));
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
   btnCancel->setEnabled(false);
   btnSearch->setEnabled(false);
   btnAdd->setEnabled(false);
   lblSearch->setText(_("Searching (this can take awhile)..."));
   resize(360, 460);
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
   edtEmail->setText("");
   edtLast->setText("");
   edtFirst->setText("");
   edtNick->setText("");
   btnCancel->setEnabled(true);
   btnSearch->setEnabled(true);
   btnAdd->setEnabled(false);
   btnSearchAgain->setEnabled(false);
   foundView->clear();
   resize(360, 160);
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
   foundView->clear();
}

#include "moc/moc_searchuserdlg.h"
