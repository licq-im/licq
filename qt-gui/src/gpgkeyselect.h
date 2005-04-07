#ifndef GPGKEYSELECT_H
#define GPGKEYSELECT_H

#include "licqdialog.h"

#include <qobject.h>
#include <qlistview.h>
#include <qdialog.h>

class CICQDaemon;
class ICQUser;
class QCheckBox;

class KeyView : public QListView
{
    Q_OBJECT
public:
	KeyView( QWidget *parent, const char *szId, unsigned long nPPID );
	~KeyView() {};

protected:
	const char *szId;
	unsigned long nPPID;
	void testViewItem( QListViewItem *item, ICQUser *u );
	int maxItemVal;
	QListViewItem *maxItem;
	void initKeyList();
	virtual void resizeEvent(QResizeEvent *);
};

class GPGKeySelect : public QDialog
{
	Q_OBJECT
public:
	GPGKeySelect ( const char *szId, unsigned long nPPID, QWidget *parent = 0);
	~GPGKeySelect();

protected:
	QListView *keySelect;
	QCheckBox *useGPG;
	char *szId;
	unsigned long nPPID;
	void updateIcon();

protected slots:
	void slot_ok();
	void slotNoKey();
	void slotCancel();

signals:
	void signal_done();
};

#endif
