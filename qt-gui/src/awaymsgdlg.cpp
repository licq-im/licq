#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <qarray.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmultilineedit.h>
#include <qpopupmenu.h>
#include <qstringlist.h>
#include <qvaluelist.h>

#include "awaymsgdlg.h"
#include "licq-locale.h"

int AwayMsgDlg::s_nX = 100;
int AwayMsgDlg::s_nY = 100;

AwayMsgDlg::AwayMsgDlg(QStringList& _respHeader, QStringList& _respText, QWidget *parent = 0, const char *name = 0)
    : QDialog(parent, name),
      responseHeader(_respHeader),
      responseText(_respText)
{
  QBoxLayout* top_lay = new QVBoxLayout(this, 10);

  mleAwayMsg = new MLEditWrap(true, this);
  // ICQ99b allows 37 chars per line, so we do the same
  mleAwayMsg->setWordWrap(QMultiLineEdit::FixedColumnWrap);
  mleAwayMsg->setWrapColumnOrWidth(37);
  connect(mleAwayMsg, SIGNAL(signal_CtrlEnterPressed()), this, SLOT(ok()));
  top_lay->addWidget(mleAwayMsg);

  QBoxLayout* l = new QHBoxLayout(top_lay, 10);

  btnSelect = new QPushButton(_("&Select"), this);
  // this doesn't work yet (Qt bug)
  //btnSelect->setIsMenuButton(true);
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

void AwayMsgDlg::selectAutoResponse(unsigned short _status)
{
  char s[32];

  status = _status;

  // Yeah, I know this is plain ugly, it's just a hack
  // but getStatusStr() can't be used because it returns
  // the wrong (old status) value and it can't be called
  // with an arbitary status. 

  // Needs to be moved to somewhere else (Dirk)
  switch(status) {
    case ICQ_STATUS_FREEFORCHAT: strcpy(s, _("Free For Chat")); break;
    case ICQ_STATUS_OCCUPIED:    strcpy(s, _("Occupied"));      break;
    case ICQ_STATUS_DND:         strcpy(s, _("DND"));           break;
    case ICQ_STATUS_NA:          strcpy(s, _("N/A"));           break;
    case ICQ_STATUS_AWAY:        strcpy(s, _("Away"));          break;
    default:                     strcpy(s, _("default"));       break;
  }

  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  
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

void AwayMsgDlg::show()
{
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  unsigned short status = o->getStatus();
  gUserManager.DropOwner();
  
  selectAutoResponse(status);
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

  int result = 0;
  int offset = 0;

  // Select auto response group
  switch(status) {
    case ICQ_STATUS_FREEFORCHAT: offset = 32; break;
    case ICQ_STATUS_OCCUPIED:    offset = 16; break;
    case ICQ_STATUS_DND:         offset = 24; break;
    case ICQ_STATUS_NA:          offset =  8; break;
    case ICQ_STATUS_AWAY:
    default:                     offset =  0; break;
  }

  for(int i = 0; i<8; i++)
      menu->insertItem(responseHeader[i+offset], i);

  menu->insertSeparator();
  // as this is not yet implemented, give user feedback
  menu->setItemEnabled(menu->insertItem(_("&Edit Items"), -2), false);
  
  result = menu->exec(btnSelect->mapToGlobal(QPoint(0,btnSelect->height())));

  if(result == -2) {
    // TODO: open options menu
  }
  else {
    // (unsigned) -1 > 8 !
    if ((unsigned) result < 8)
      mleAwayMsg->setText(responseText[result+offset]);
  }

  delete menu;
}


#include "moc/moc_awaymsgdlg.h"
