#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <qlistview.h>
#include <qtooltip.h>

class CUserEvent;
class MsgView;

//=====CMsgViewTips===============================================================================

class CMsgViewTips : public QToolTip
{
public:
  CMsgViewTips(MsgView * parent);
  virtual ~CMsgViewTips() {};

protected:
  virtual void maybeTip(const QPoint&);
};



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

friend class CMsgViewTips;
friend class MsgView;
};


//=====UserList====================================================================================
class MsgView : public QListView
{
public:
  MsgView (QWidget *parent = 0, const char *name = 0);
  CUserEvent *currentMsg(void);
  void markRead(short);
  QSize sizeHint() const;
protected:
  CMsgViewTips *tips;

  virtual void resizeEvent(QResizeEvent *e);
};


#endif
