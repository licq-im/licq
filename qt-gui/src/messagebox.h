#ifndef MSGBOXLIST_H
#define MSGBOXLIST_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qlistview.h>
#include <qpopmenu.h>

class CUserEvent;

//=====UserViewItem================================================================================
class MsgViewItem : public QListViewItem
{
public:
   MsgViewItem (CUserEvent *, unsigned short, QListView *);
   ~MsgViewItem(void);
   short index;
   CUserEvent *msg;

protected:
   virtual void paintCell ( QPainter *, const QColorGroup &, int column, int width, int align);

friend class MsgView;
};


//=====UserList====================================================================================
class MsgView : public QListView
{
public:
   MsgView (QWidget *parent = 0, const char *name = 0);
   CUserEvent *currentMsg(void);
   void markRead(short);
protected:
   virtual void resizeEvent(QResizeEvent *e);
   void mouseReleaseEvent(QMouseEvent *e);
};


#endif
