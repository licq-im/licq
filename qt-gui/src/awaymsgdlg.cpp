#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <qarray.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qpopupmenu.h>
#include <qstringlist.h>
#include <qvaluelist.h>

#include "awaymsgdlg.h"
#include "licq-locale.h"

int AwayMsgDlg::s_nX = 100;
int AwayMsgDlg::s_nY = 100;

AwayMsgDlg::AwayMsgDlg(QWidget *parent = 0, const char *name = 0)
  : QDialog(parent, name)
{
  QBoxLayout* top_lay = new QVBoxLayout(this, 10);

#if QT_VERSION >= 210
  mleAwayMsg = new QMultiLineEdit(this);
  // ICQ99b allows 37 chars per line, so we do the same
  mleAwayMsg->setWordWrap(QMultiLineEdit::FixedColumnWrap);
  mleAwayMsg->setWrapColumnOrWidth(37);
#else  
  mleAwayMsg = new MLEditWrap(true, this);
  connect(mleAwayMsg, SIGNAL(signal_CtrlEnterPressed()), this, SLOT(ok()));
#endif  
  top_lay->addWidget(mleAwayMsg);

  QBoxLayout* l = new QHBoxLayout(top_lay, 10);

  btnSelect = new QPushButton(_("&Select"), this);
  btnSelect->setIsMenuButton(true);
  connect(btnSelect, SIGNAL(clicked()), SLOT(selectMessage()));
  l->addWidget(btnSelect);
  
  l->addStretch(1);
  l->addSpacing(30);
  
  QPushButton *btnOk, *cancel;
  btnOk = new QPushButton(_("&Ok"), this );
  btnOk->setDefault(true);
  connect( btnOk, SIGNAL(clicked()), SLOT(ok()) );
  l->addWidget(btnOk);

  cancel = new QPushButton(_("&Cancel"), this );
  connect( cancel, SIGNAL(clicked()), SLOT(reject()) );
  l->addWidget(cancel);
}


void AwayMsgDlg::show()
{
  char s[32];
  
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  o->getStatusStr(s);
  
  setCaption(QString(_("Set %1 Response for %2"))
             .arg(s).arg(QString::fromLocal8Bit(o->getAlias())));
  if (*o->getAwayMessage())
    mleAwayMsg->setText(QString::fromLocal8Bit(o->getAwayMessage()));
  else
    mleAwayMsg->setText(_("I am currently %1.\nYou can leave me a message.")
                        .arg(s));
  gUserManager.DropOwner();
  move(s_nX, s_nY);
  mleAwayMsg->setFocus();
  mleAwayMsg->selectAll();
  QDialog::show();
}

void AwayMsgDlg::hide()
{
  s_nX = x();
  s_nY = y();
  QDialog::hide();
}


void AwayMsgDlg::ok()
{
  ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
  o->setAwayMessage(mleAwayMsg->text().local8Bit());
  gUserManager.DropOwner();
  accept();
}

void AwayMsgDlg::selectMessage()
{

  QPopupMenu* menu = new QPopupMenu(this);

  QStringList lst;
  int result = 0;

  lst << "bin gleich wieder da" << "Uni" << "Spocht" << "Physickern";

  for(QStringList::Iterator it = lst.begin(); it != lst.end(); ++it)
      menu->insertItem(*it, result++);

  menu->insertSeparator();
  menu->insertItem(_("&Edit Items"), -2);
  
  result = menu->exec(btnSelect->mapToGlobal(QPoint(0,btnSelect->height())));

  if(result == -2) {
    // TODO: open options menu
  }
  else {
    // (unsigned) -1 >= lst.count() !
    if ((unsigned) result < lst.count())
      mleAwayMsg->setText(lst[result]);
  }

  delete menu;
}


#include "moc/moc_awaymsgdlg.h"
