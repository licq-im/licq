#ifndef MMBOX_H
#define MMBOX_H

#include <vector>
#include <list>

#include <qlistview.h>
#include <qtooltip.h>

#include "userbox.h"

class ICQUser;
class CMainWindow;


//=====UserViewItem================================================================================
class CMMUserViewItem : public QListViewItem
{
public:
  CMMUserViewItem (ICQUser *, QListView *);
  virtual ~CMMUserViewItem();
  unsigned long Uin()  { return m_nUin; }
protected:
  unsigned long m_nUin;

  friend class CMMUserView;
};


//=====MMUserView===============================================================
class CMMUserView : public QListView
{
  Q_OBJECT
public:
  CMMUserView (ColumnInfos &_colInfo, bool, unsigned long, CMainWindow *,
     QWidget *parent = 0);
  virtual ~CMMUserView();

  void AddUser(unsigned long);

protected:
  QPopupMenu *mnuMM;
  ColumnInfos colInfo;
  unsigned long m_nUin;
  CMainWindow *mainwin;

  virtual void viewportMousePressEvent(QMouseEvent *e);
  virtual void keyPressEvent(QKeyEvent *e);
  virtual void dragEnterEvent(QDragEnterEvent * dee);
  virtual void dropEvent(QDropEvent * de);
  virtual void resizeEvent(QResizeEvent *);

  friend class CMMUserViewItem;

protected slots:
  void slot_menu(int);
};

#endif
