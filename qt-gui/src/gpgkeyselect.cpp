/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2005-2009 Licq developers
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gpgkeyselect.moc"

#include <cstring>
#include <memory>

#include "mainwin.h"
#include "userbox.h"

#include <licq_gpg.h>
#include <licq_user.h>
#include <licq_events.h>

#include <qheader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>

using namespace std;


GPGKeySelect::GPGKeySelect(const UserId& userId, QWidget *parent) : QDialog( parent )
{
  if (!USERID_ISVALID(userId))
    return;

  setWFlags( WDestructiveClose );

  myUserId = userId;
  const LicqUser* u = gUserManager.fetchUser(myUserId);
  if ( !u ) return;

  setCaption( tr("Select GPG Key for user %1").arg(QString::fromUtf8(u->GetAlias())) );

  QBoxLayout* top_lay = new QVBoxLayout(this, 11, 6);
  
  top_lay->addWidget( new QLabel( tr("Select a GPG key for user %1.").arg(QString::fromUtf8(u->GetAlias())), this ) );
  if ( strcmp( u->GPGKey(), "" )==0 )
    top_lay->addWidget( new QLabel( tr("Current key: No key selected"), this ) );
  else
    top_lay->addWidget( new QLabel( tr("Current key: %1").arg(QString::fromLocal8Bit(u->GPGKey())), this ) );

  useGPG = new QCheckBox( tr("Use GPG Encryption"), this );
  useGPG->setChecked( u->UseGPG() || strcmp(u->GPGKey(),"")==0 );
  top_lay->addWidget( useGPG );

  // Filter
  QHBoxLayout *filterLayout = new QHBoxLayout( top_lay );
  filterLayout->addWidget( new QLabel( tr("Filter:"), this ) );
  QLineEdit *filterText = new QLineEdit( this );
  filterText->setFocus();
  connect( filterText, SIGNAL(textChanged ( const QString & )), this, SLOT(filterTextChanged( const QString & )) );
  filterLayout->addWidget( filterText );

  gUserManager.DropUser(u);

  // public keys
  keySelect = new KeyView(this, myUserId);
  top_lay->addWidget( keySelect );
  connect( keySelect, SIGNAL(doubleClicked ( QListViewItem *, const QPoint &, int )), this, SLOT(slot_doubleClicked( QListViewItem *, const QPoint &, int )));

  QBoxLayout *lay = new QHBoxLayout(top_lay);
  int bw = 0;

  QPushButton *btnOk = new QPushButton(tr("&OK"), this);
  connect (btnOk, SIGNAL(clicked()), this, SLOT(slot_ok()));
  bw = QMAX( bw, btnOk->sizeHint().width() );

  QPushButton *btnNoKey = new QPushButton(tr("&No Key"), this);
  connect (btnNoKey, SIGNAL(clicked()), this, SLOT(slotNoKey()));
  bw = QMAX( bw, btnNoKey->sizeHint().width() );

  QPushButton *btnCancel = new QPushButton(tr("&Cancel"), this);
  connect (btnCancel, SIGNAL(clicked()), this, SLOT(slotCancel()));
  bw = QMAX( bw, btnCancel->sizeHint().width() );

  lay->addStretch(2);
  btnOk->setFixedWidth(bw);
  lay->addWidget(btnOk);
  lay->addSpacing(6);
  
  btnNoKey->setFixedWidth(bw);
  lay->addWidget(btnNoKey);
  lay->addSpacing(6);
  
  btnCancel->setFixedWidth(bw);
  lay->addWidget(btnCancel);

  show();
};

GPGKeySelect::~GPGKeySelect()
{
  emit signal_done();
};

void GPGKeySelect::filterTextChanged ( const QString &str )
{
  QListViewItemIterator it( keySelect );
  while ( it.current() )
  {
    QListViewItem *item = it.current();
    item->setVisible( item->text(0).contains( str, false ) || item->text(1).contains( str, false ) || item->text(2).contains( str, false ) );
    ++it;
  }
};

void GPGKeySelect::slot_doubleClicked ( QListViewItem *item, const QPoint& /* pt */, int /* i */)
{
  if ( item )
    slot_ok();
};

void GPGKeySelect::slot_ok()
{
  QListViewItem *curItem = keySelect->QListView::currentItem();
  if ( curItem->parent() ) curItem=curItem->parent();
  if ( curItem )
  {
    LicqUser* u = gUserManager.fetchUser(myUserId, LOCK_W);
    if ( u )
    {
      u->SetGPGKey( curItem->text(2).ascii() );
      u->SetUseGPG( useGPG->isChecked() );
      gUserManager.DropUser( u );
      updateIcon();
    }
  }
  close();
}

void GPGKeySelect::updateIcon()
{
  gMainWindow->slot_updatedUser(myUserId, USER_GENERAL);
  return;
}


void GPGKeySelect::slotNoKey()
{
  LicqUser* u = gUserManager.fetchUser(myUserId, LOCK_W);
  if ( u )
  {
    u->SetGPGKey( "" );
    gUserManager.DropUser( u );
    updateIcon();
  }

  close();
};

void GPGKeySelect::slotCancel()
{
  close();
};


KeyView::KeyView(QWidget *parent, const UserId& userId)
  : QListView( parent )
{
  header()->setClickEnabled(false);
  addColumn( tr("Name") );
  addColumn( "EMail" );
  addColumn( "ID" );
    
  setAllColumnsShowFocus(true);

  myUserId = userId;

  initKeyList();

  setRootIsDecorated(true);
}

void KeyView::resizeEvent(QResizeEvent *e)
{
  QListView::resizeEvent(e);

  unsigned short totalWidth = 0;
  unsigned short nNumCols = header()->count();
  for (unsigned short i = 0; i < nNumCols - 1; i++)
    totalWidth += columnWidth(i);

  int newWidth = width() - totalWidth - 2;
  if (newWidth <= 0)
  {
    setHScrollBarMode(Auto);
    setColumnWidth(nNumCols - 1, 2);
  }
  else
  {
    setHScrollBarMode(AlwaysOff);
    setColumnWidth(nNumCols - 1, newWidth);
  }
}

void KeyView::testViewItem(QListViewItem *item, const LicqUser* u)
{
  int val = 0;
  for ( int i = 0; i<2; i++ )
  {
    if (item->text(i).contains(u->getFirstName().c_str(), false)) val++;
    if (item->text(i).contains(u->getLastName().c_str(), false)) val++;
    if ( item->text(i).contains( u->GetAlias(), false ) ) val++;
    if (item->text(i).contains(u->getEmail().c_str(), false )) val++;
  }
  
  if ( item->text(2).contains( u->GPGKey(), false ) ) val += 10;

  if ( val>maxItemVal )
  {
    maxItemVal = val;
    maxItem = item;
  }
}


void KeyView::initKeyList()
{
  LicqUserReadGuard u(myUserId);
  maxItemVal = -1;
  maxItem = NULL;

  auto_ptr<list<GpgKey> > keyList(gGPGHelper.getKeyList());
  list<GpgKey>::const_iterator i;
  for (i = keyList->begin(); i != keyList->end(); ++i)
  {
    // There shouldn't be any key without a user id in list, but make just in case
    if (i->uids.empty())
      continue;

    // First user id is primary uid
    list<GpgUid>::const_iterator uid = i->uids.begin();

    QListViewItem* keyItem = new QListViewItem( this, QString::fromUtf8(uid->name.c_str()),
        QString::fromUtf8(uid->email.c_str()), QString(i->keyid.c_str()).right(8) );
    if (u.isLocked())
      testViewItem(keyItem, *u);

    ++uid;
    for ( ;uid != i->uids.end(); ++uid)
    {
      QListViewItem* uidItem = new QListViewItem(keyItem, QString::fromUtf8(uid->name.c_str()),
          QString::fromUtf8(uid->email.c_str()));
      if (u.isLocked())
        testViewItem(uidItem, *u);
    }
  }

  if ( maxItem )
    setCurrentItem( maxItem );
}
