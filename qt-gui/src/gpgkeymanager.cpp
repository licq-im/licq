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

#include "gpgkeymanager.moc"

#include <licq_user.h>
#include <licq_events.h>

#include <qvariant.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qheader.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qhgroupbox.h>
#include <qdragobject.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>

#include "mainwin.h"
#include "gpgkeyselect.h"

GPGKeyManager::GPGKeyManager( QWidget* parent )
    : QDialog( parent )
{
	GPGKeyManagerLayout = new QVBoxLayout( this, 11, 6 ); 
    
	setCaption( tr( "Licq GPG Key Manager" ) );
	setWFlags( WDestructiveClose );

	// GPG Passphrase
	QGroupBox *groupBox = new QGroupBox( tr("GPG Passphrase"), this );
	groupBox1Layout = new QHBoxLayout( groupBox, 11, 6 );

	textLabel5 = new QLabel( tr( "No passphrase set" ), groupBox );
	
	groupBox1Layout->addWidget( textLabel5 );
	groupBox1Layout->addStretch( 2 );

	pushButton7 = new QPushButton( tr( "Set GPG Passphrase" ), groupBox );
	connect( pushButton7, SIGNAL(clicked()), this, SLOT(slot_setPassphrase()) );
	groupBox1Layout->addWidget( pushButton7 );

	GPGKeyManagerLayout->addWidget( groupBox );

	// User Keys
	groupBox = new QGroupBox( "User Keys", this );
	groupBox2Layout = new QHBoxLayout( groupBox, 11, 6 );

	listView2 = new KeyList( groupBox );
	listView2->setAllColumnsShowFocus( true );
	listView2->addColumn( tr( "User" ) );
	listView2->addColumn( tr( "Active" ) );
	listView2->addColumn( tr( "Key ID" ) );
	listView2->setColumnAlignment( 2, Qt::AlignRight );
	connect( listView2, SIGNAL(doubleClicked ( QListViewItem *, const QPoint &, int )), this, SLOT(slot_doubleClicked( QListViewItem *, const QPoint &, int )));
	groupBox2Layout->addWidget( listView2 );

	layout10 = new QVBoxLayout( groupBox2Layout );
	int bw=0;

	pushButton8 = new QPushButton( tr( "Add" ), groupBox );
	connect( pushButton8, SIGNAL(clicked()), this, SLOT(slot_add()) );
	layout10->addWidget( pushButton8 );
	bw = QMAX( bw, pushButton8->sizeHint().width() );

	pushButton9 = new QPushButton( tr( "Edit" ), groupBox );
	connect( pushButton9, SIGNAL(clicked()), this, SLOT(slot_edit()) );
	layout10->addWidget( pushButton9 );
	bw = QMAX( bw, pushButton9->sizeHint().width() );

	pushButton10 = new QPushButton( tr( "Remove" ), groupBox );
	connect( pushButton10, SIGNAL(clicked()), this, SLOT(slot_remove()) );
	bw = QMAX( bw, pushButton10->sizeHint().width() );

	pushButton8->setFixedWidth( bw );
	pushButton9->setFixedWidth( bw );
	pushButton10->setFixedWidth( bw );
	layout10->addWidget( pushButton10 );
	QLabel *label = new QLabel( tr("Drag&Drop\nuser to add\nto list."), groupBox);
	label->setMaximumWidth( bw );
	layout10->addWidget(  label);

	layout10->addStretch( 2 );

	GPGKeyManagerLayout->addWidget( groupBox );

	// Close
	layout9 = new QHBoxLayout( GPGKeyManagerLayout ); 
	layout9->addStretch( 2 );

	pushButton5 = new QPushButton( tr( "Close" ), this );
	connect( pushButton5, SIGNAL(clicked()), this, SLOT(slot_close()) );
	pushButton5->setAutoDefault( TRUE );
	pushButton5->setFixedWidth( pushButton5->sizeHint().width() );
	layout9->addWidget( pushButton5 );

	resize( QSize(439, 541).expandedTo(minimumSizeHint()) );

	initKeyList();
}
 
void GPGKeyManager::slot_setPassphrase()
{
	QMessageBox::information( this, "Set Passphrase", "Not yet implemented. Use licq_gpg.conf." );
}

void GPGKeyManager::slot_edit()
{
	QListViewItem *item = listView2->currentItem();
	if ( item )
		( (KeyListItem*)item )->edit();
}

void GPGKeyManager::slot_doubleClicked ( QListViewItem *item, const QPoint &point, int i )
{
	if ( item )
		( (KeyListItem*)item )->edit();
}

void GPGKeyManager::slot_remove()
{
	KeyListItem *item = (KeyListItem*)listView2->currentItem();
	if ( item )
	{
		if ( QMessageBox::question( this, tr("Remove GPG key"), tr("Do you want to remove the GPG key? The key isn't deleted from your keyring."), QMessageBox::Yes, QMessageBox::No )==QMessageBox::Yes )
		{
			item->unsetKey();
			delete item;
		};
	}
}

struct luser
{
	char *szId;
	unsigned long nPPID;
	const char *alias;
};

class gkm_UserList : public QPtrList<luser>
{
public:
	gkm_UserList() {};
protected:
//	virtual int compareItems( luser item1, luser item2 )
	int compareItems( QPtrCollection::Item item1, QPtrCollection::Item item2 )
	{ return strcasecmp( ((luser*)item1)->alias, ((luser*)item2)->alias ); };
};


void GPGKeyManager::slot_add()
{
	QPopupMenu popupMenu;
	gkm_UserList list;
	list.setAutoDelete( true );
	int i = 0;
	FOR_EACH_USER_START(LOCK_R)
	{
		if ( strcmp( pUser->GPGKey(), "" )== 0 )
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

	for ( i=0; i<list.count(); i++ )
		popupMenu.insertItem( list.at(i)->alias, i );

	int res = popupMenu.exec(QCursor::pos());
	if ( res<0 ) return;
	luser *tmp = list.at( res );
	if ( !tmp ) return;

	ICQUser *u = gUserManager.FetchUser(tmp->szId, tmp->nPPID, LOCK_R);
	if ( u )
	{
		( new KeyListItem( listView2, u ) )->edit();
		gUserManager.DropUser(u);
	}
}

void GPGKeyManager::slot_close()
{
	close();
}

GPGKeyManager::~GPGKeyManager()
{
}

void GPGKeyManager::initKeyList()
{ 
	FOR_EACH_USER_START(LOCK_R)
	{
		if ( strcmp( pUser->GPGKey(), "" )!= 0 )
		{
			new KeyListItem( listView2, pUser );
		}
	}
	FOR_EACH_USER_END
}

KeyList::KeyList( QWidget *parent ) : QListView( parent )
{
	setAcceptDrops( TRUE );
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
	setText( 0, u->GetAlias() );
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
		{
			updateText( u );
			gUserManager.DropUser(u);
		};
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
