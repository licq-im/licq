#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <qlistview.h>

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
