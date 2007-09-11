/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2005-2006 Licq developers
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

#ifdef HAVE_LIBGPGME

#include "gpgkeyselect.moc"

#include "mainwin.h"
#include "userbox.h"

#include <licq_user.h>
#include <licq_events.h>

#include <gpgme.h>

#include <qheader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>

GPGKeySelect::GPGKeySelect ( const char *szId, unsigned long nPPID, QWidget *parent) : QDialog( parent )
{
  if ( !szId || !nPPID ) return;

  setWFlags( WDestructiveClose );

  ICQUser *u = gUserManager.FetchUser( szId, nPPID, LOCK_R );
  if ( !u ) return;

  setCaption( tr("Select GPG Key for user %1").arg(QString::fromUtf8(u->GetAlias())) );

  this->szId = strdup( szId );
  this->nPPID = nPPID;

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
  keySelect = new KeyView( this, szId, nPPID );
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
  if ( szId )
    free( szId );

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
    ICQUser *u = gUserManager.FetchUser( szId, nPPID, LOCK_W );
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
  CICQSignal s(SIGNAL_UPDATExUSER, USER_GENERAL, szId, nPPID);
  gMainWindow->slot_updatedUser(&s);
  return;
}


void GPGKeySelect::slotNoKey()
{
  ICQUser *u = gUserManager.FetchUser( szId, nPPID, LOCK_W );
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

gpgme_ctx_t mCtx;
gpgme_key_t key;


KeyView::KeyView( QWidget *parent, const char *szId, unsigned long nPPID ) : QListView( parent )
{
  header()->setClickEnabled(false);
  addColumn( tr("Name") );
  addColumn( "EMail" );
  addColumn( "ID" );
    
  setAllColumnsShowFocus(true);

  this->szId = szId;
  this->nPPID = nPPID;

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

void KeyView::testViewItem( QListViewItem *item, ICQUser *u )
{
  int val = 0;
  for ( int i = 0; i<2; i++ )
  {
    if ( item->text(i).contains( u->GetFirstName(), false ) ) val++;
    if ( item->text(i).contains( u->GetLastName(), false ) ) val++;
    if ( item->text(i).contains( u->GetAlias(), false ) ) val++;
    if ( item->text(i).contains( u->GetEmailPrimary(), false ) ) val++;
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
  gpgme_new( &mCtx );

  ICQUser *u = gUserManager.FetchUser( szId, nPPID, LOCK_R );
  maxItemVal = -1;
  maxItem = NULL;

  int err = gpgme_op_keylist_start( mCtx, NULL, 0);

  while ( !err)
  {
    err = gpgme_op_keylist_next( mCtx, &key );
    if ( err ) break;
    gpgme_user_id_t uid = key->uids;
    if ( uid && key->can_encrypt && key->subkeys )
    {
      QListViewItem *f = new QListViewItem( this, QString::fromUtf8(uid->name), QString::fromUtf8(uid->email), QString( key->subkeys->keyid ).right(8) );
      if ( u ) testViewItem( f, u );
      uid = uid->next;
      while ( uid )
      {
        QListViewItem *g = new QListViewItem( f, QString::fromUtf8(uid->name), QString::fromUtf8(uid->email) );
        if ( u ) testViewItem( g, u );
        uid = uid->next;
      }
    }
    gpgme_key_release (key);
  }

  if ( u )
    gUserManager.DropUser( u );
  
  gpgme_release( mCtx );
  if ( maxItem )
    setCurrentItem( maxItem );
}

#endif
