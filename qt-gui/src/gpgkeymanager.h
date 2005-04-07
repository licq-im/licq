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

    QLabel* textLabel5;
    QPushButton* pushButton7;
    QListView* listView2;
    QPushButton* pushButton8;
    QPushButton* pushButton9;
    QPushButton* pushButton10;
    QPushButton* pushButton5;

protected:
    QVBoxLayout* GPGKeyManagerLayout;
    QHBoxLayout* groupBox1Layout;
    QHBoxLayout* groupBox2Layout;
    QVBoxLayout* layout10;
    QSpacerItem* spacer6;
    QHBoxLayout* layout9;

	void initKeyList();

protected slots:
	void slot_edit();
	void slot_close();
	void slot_remove();
	void slot_add();
	void slot_setPassphrase();
	void slot_doubleClicked ( QListViewItem *, const QPoint &, int );
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

protected:
	char *szId;
	unsigned long nPPID;
	GPGKeySelect *keySelect;
	void updateText( ICQUser *u );

protected slots:
	void slot_done();
};

#endif // GPGKEYMANAGER_H
