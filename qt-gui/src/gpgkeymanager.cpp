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

#include <qheader.h>
#include <qdragobject.h>
#include <qgroupbox.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qcursor.h>

#include "gpgkeymanager.h"

struct luser
{
  QString szId;
  unsigned long nPPID;
  const char *alias;
};

class gkm_UserList : public QPtrList<luser>
{
public:
  gkm_UserList() {};
protected:
  int compareItems( QPtrCollection::Item item1, QPtrCollection::Item item2 )
  { return strcasecmp( ((luser*)item1)->alias, ((luser*)item2)->alias ); };
};

GPGKeyManager::GPGKeyManager( QWidget *_parent )
  : LicqDialog( _parent )
{
  QVBoxLayout *lay_main = new QVBoxLayout( this, 11, 6 );
  
  setCaption( tr("Licq GPG Key Manager") );
  setWFlags( WDestructiveClose );
  
  // GPG Passphrase box
  QGroupBox *box_passphrase = new QGroupBox( tr("GPG Passphrase"), this, "Box Passphrase");
  lay_main->addWidget( box_passphrase );
  
  box_passphrase->setColumnLayout( 1, Qt::Vertical );
  box_passphrase->setInsideSpacing( 6 );
  box_passphrase->setInsideMargin( 11 );
  
  lbl_passphrase = new QLabel( tr("No passphrase set"), box_passphrase );
  QPushButton *btn_setPass = new QPushButton( tr("&Set GPG Passphrase"), box_passphrase);
  connect(btn_setPass, SIGNAL(clicked()), this, SLOT(slot_setPassphrase()));
  
  // GPG User/Key list
  QHBoxLayout *lay_hbox = new QHBoxLayout( lay_main, -1, "Keylist/buttons layout");
  QGroupBox *box_keys = new QGroupBox( tr("User Keys"), this, "Box Keys" );
  box_keys->setColumnLayout( 1, Qt::Vertical );
  box_keys->setInsideSpacing( 6 );
  box_keys->setInsideMargin( 11 );
  
  lay_hbox->addWidget( box_keys );
  QVBoxLayout *lay_buttonBox = new QVBoxLayout( lay_hbox, -1, "Button layout");
  QPushButton *btn_add = new QPushButton( tr("&Add"), this);
  QPushButton *btn_edit = new QPushButton( tr("&Edit"), this);
  QPushButton *btn_remove = new QPushButton( tr("&Remove"), this);
  connect(btn_add,    SIGNAL(clicked()), this, SLOT(slot_add()));
  connect(btn_edit,   SIGNAL(clicked()), this, SLOT(slot_edit()));
  connect(btn_remove, SIGNAL(clicked()), this, SLOT(slot_remove()));
  lbl_dragndrop = new QLabel( tr("<qt>Drag&Drop user to add to list.</qt>"), this);
  
  // create the keylist
  lst_keyList = new KeyList( box_keys );
  lst_keyList->setAllColumnsShowFocus(true);
  lst_keyList->addColumn( tr("User") );
  lst_keyList->addColumn( tr("Active") );
  lst_keyList->addColumn( tr("Key ID") );
  connect( lst_keyList, SIGNAL(doubleClicked( QListViewItem *, const QPoint &, int)),
           this, SLOT(slot_doubleClicked(QListViewItem *, const QPoint &, int )));

  lay_buttonBox->addWidget(btn_add);
  lay_buttonBox->addWidget(btn_edit);
  lay_buttonBox->addWidget(btn_remove);
  lay_buttonBox->addWidget(lbl_dragndrop);
  lay_buttonBox->addStretch();
  
  
  // Close
  QHBoxLayout *lay_btnClose = new QHBoxLayout( lay_main, -1, "Close button layout");
  QPushButton *btn_close = new QPushButton( tr("&Close"), this);
  btn_close->setAutoDefault(true);
  lay_btnClose->addStretch();
  lay_btnClose->addWidget( btn_close );
  connect( btn_close, SIGNAL(clicked()), this, SLOT(slot_close()));
  
  initKeyList();
}

GPGKeyManager::~GPGKeyManager()
{}

void GPGKeyManager::slot_setPassphrase()
{
  QMessageBox::information( this, tr("Set Passphrase"), tr("Not yet implemented. Use licq_gpg.conf.") );
}

void GPGKeyManager::slot_close()
{
  close();
}

void GPGKeyManager::slot_edit()
{
  QListViewItem *item = lst_keyList->currentItem();
  if (item )
    ( (KeyListItem*)item )->edit();
}

void GPGKeyManager::slot_doubleClicked( QListViewItem *item, const QPoint& /* point */, int /* i */)
{
  if (item )
    ( (KeyListItem*)item )->edit();
}


void GPGKeyManager::slot_add()
{
  QPopupMenu popupMenu;
  gkm_UserList list;
  list.setAutoDelete(true);
  
  FOR_EACH_USER_START(LOCK_R)
  {
    if (strcmp( pUser->GPGKey(), "") == 0)
    {
      luser *tmp = new luser;
      tmp->szId = pUser->IdString();
      tmp->nPPID = pUser->PPID();
      tmp->alias = pUser->GetAlias();
      list.append( tmp );
    }
  }
  FOR_EACH_USER_END
  
  list.sort();
  
  for ( unsigned int i=0; i<list.count(); i++ )
  popupMenu.insertItem( QString::fromUtf8(list.at(i)->alias), i );

  int res = popupMenu.exec(QCursor::pos());
  if ( res<0 ) return;
  luser *tmp = list.at( res );
  if ( !tmp ) return;

  ICQUser* u = gUserManager.FetchUser(tmp->szId.latin1(), tmp->nPPID, LOCK_R);
  if ( u )
  {
    editUser( u );
    gUserManager.DropUser(u);
  }
}

void GPGKeyManager::editUser( ICQUser *u )
{
  QListViewItemIterator it( lst_keyList );
  while ( it.current() )
  {
    KeyListItem* item = (KeyListItem*)it.current();
    if ( strcmp( item->getszId(), u->IdString() )==0 && item->getnPPID() == u->PPID() )
    {
      item->edit();
      break;
    }
    ++it;
  }
  if ( ! it.current() )
    ( new KeyListItem( lst_keyList, u ) )->edit();
};

void GPGKeyManager::slot_remove()
{
  KeyListItem *item = (KeyListItem*)lst_keyList->currentItem();
  if ( item )
  {
    if ( QMessageBox::question( this, tr("Remove GPG key"), tr("Do you want to remove the GPG key? The key isn't deleted from your keyring."), QMessageBox::Yes, QMessageBox::No )==QMessageBox::Yes )
    {
      item->unsetKey();
      delete item;
    };
  }
}

void GPGKeyManager::initKeyList()
{ 
  FOR_EACH_USER_START(LOCK_R)
  {
    if ( strcmp( pUser->GPGKey(), "" )!= 0 )
    {
      new KeyListItem( lst_keyList, pUser );
    }
  }
  FOR_EACH_USER_END
}



// THE KEYLIST
KeyList::KeyList( QWidget *_parent )
  : QListView( _parent )
{
  setAcceptDrops(true);
}

KeyList::~KeyList()
{
}

void KeyList::dragEnterEvent(QDragEnterEvent* event)
{
  event->accept( QTextDrag::canDecode(event) );
}

void KeyList::dropEvent(QDropEvent * de)
{
  QString text;
  if ( !QTextDrag::decode(de, text) )
    return;

  char *szId = strdup( text.right(text.length()-4).latin1() );
  unsigned long nPPID = LICQ_PPID; //TODO dropevent needs the ppid

  ICQUser *u = gUserManager.FetchUser(szId, nPPID, LOCK_R);

  if ( u )
  {
    QListViewItemIterator it( this );
    while ( it.current() )
    {
      KeyListItem* item = (KeyListItem*)it.current();
      if ( strcmp( item->getszId(), szId )==0 && item->getnPPID() == nPPID )
      {
        item->edit();
        break;
      }
      ++it;
    }
    if ( ! it.current() )
      ( new KeyListItem( this, u ) )->edit();
    gUserManager.DropUser(u);
  }

  if ( szId )
    free( szId );
}

void KeyList::resizeEvent(QResizeEvent *e)
{
  QListView::resizeEvent(e);

  unsigned short totalWidth = 0;
  unsigned short nNumCols = header()->count();
  for (unsigned short i = 1; i < nNumCols; i++)
    totalWidth += columnWidth(i);

  int newWidth = width() - totalWidth - 2;
  if (newWidth <= 0)
  {
    setHScrollBarMode(Auto);
    setColumnWidth(0, 2);
  }
  else
  {
    setHScrollBarMode(AlwaysOff);
    setColumnWidth(0, newWidth);
  }
}

// KEYLISTITEM
KeyListItem::KeyListItem( QListView *parent, ICQUser *u )
  :QListViewItem( parent )
{
  szId = strdup( u->IdString() );
  nPPID = u->PPID();
  keySelect = NULL;
  updateText( u );
}

KeyListItem::~KeyListItem()
{
  free( szId );
}

void KeyListItem::updateText( ICQUser *u )
{
  setText( 0, QString::fromUtf8(u->GetAlias()) );
  setText( 1, u->UseGPG() ? tr("Yes") : tr("No") );
  setText( 2, u->GPGKey() );
}

void KeyListItem::edit()
{
  if ( !keySelect )
  {
    keySelect = new GPGKeySelect( szId, nPPID );
    connect( keySelect, SIGNAL(signal_done()), this, SLOT(slot_done() ));
  }
}

void KeyListItem::slot_done()
{
  ICQUser *u = gUserManager.FetchUser(szId, nPPID, LOCK_R);
  keySelect = NULL;

  if ( u )
  {
    if ( strcmp(u->GPGKey(),"")==0 )
      delete this;
    else
      updateText( u );
    gUserManager.DropUser(u);
  }
}

void KeyListItem::unsetKey()
{
  ICQUser *u = gUserManager.FetchUser(szId, nPPID, LOCK_W);

  if ( u )
  {
    u->SetUseGPG( false );
    u->SetGPGKey( "" );
    gUserManager.DropUser(u);
    CICQSignal s(SIGNAL_UPDATExUSER, USER_GENERAL, szId, nPPID);
    gMainWindow->slot_updatedUser(&s);
  }
}

#include "gpgkeymanager.moc"

#endif
