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

#ifndef GPGKEYMANAGER_H
#define GPGKEYMANAGER_H

#include <qvariant.h>
#include <qdialog.h>
#include <qlistview.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QLabel;
class QPushButton;
class QListView;
class QListViewItem;

class ICQUser;
class GPGKeySelect;

class GPGKeyManager : public QDialog
{
    Q_OBJECT

public:
    GPGKeyManager( QWidget* parent = 0 );
    ~GPGKeyManager();

    QListView* listView2;
    QLabel *passPhraseLabel;

protected:

  void initKeyList();
  void editUser( ICQUser *u );

protected slots:
  void slot_edit();
  void slot_close();
  void slot_remove();
  void slot_add();
  void slot_setPassphrase();
  void slot_doubleClicked ( QListViewItem *item, const QPoint &pt, int i );
};

class KeyList : public QListView
{
public:
  KeyList( QWidget *parent );
  ~KeyList();

protected:
  void dragEnterEvent(QDragEnterEvent* event);
  void dropEvent(QDropEvent * de);
  virtual void resizeEvent(QResizeEvent *e);
};

class KeyListItem : public QObject, public QListViewItem
{
  Q_OBJECT
public:
  KeyListItem( QListView *parent, ICQUser *u );
  ~KeyListItem();

  void edit();
  void unsetKey();

  const char* getszId() { return szId; };
  unsigned long getnPPID() { return nPPID; };

protected:
  char *szId;
  unsigned long nPPID;
  GPGKeySelect *keySelect;
  void updateText( ICQUser *u );

protected slots:
  void slot_done();
};

#endif // GPGKEYMANAGER_H
