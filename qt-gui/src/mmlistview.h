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
  char *Id()  { return m_szId; }
  unsigned long PPID()  { return m_nPPID; }

protected:
  unsigned long m_nUin;
  char *m_szId;
  unsigned long m_nPPID;

  friend class CMMUserView;
};


//=====MMUserView===============================================================
class CMMUserView : public QListView
{
  Q_OBJECT
public:
  CMMUserView (ColumnInfos &_colInfo, bool, unsigned long, CMainWindow *,
     QWidget *parent = 0);
  CMMUserView(ColumnInfos &_colInfo, bool, char *, unsigned long,
     CMainWindow *, QWidget *parent = 0);
  virtual ~CMMUserView();

  void AddUser(unsigned long);
  void AddUser(const char *, unsigned long);

protected:
  QPopupMenu *mnuMM;
  ColumnInfos colInfo;
  unsigned long m_nUin;
  char *m_szId;
  unsigned long m_nPPID;
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
