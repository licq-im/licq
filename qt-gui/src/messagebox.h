#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <qlistview.h>
#include <qtooltip.h>

#include "licq_constants.h"

class CUserEvent;
class MsgView;
class CMsgViewTips;

//=====UserViewItem================================================================================
class MsgViewItem : public QListViewItem
{
public:
  MsgViewItem (CUserEvent *, QTextCodec *codec, QListView *);
  ~MsgViewItem(void);
  void MarkRead();

  CUserEvent *msg;
  QTextCodec *m_codec;
  int m_nEventId;

protected:
  virtual void paintCell(QPainter *, const QColorGroup &, int column, int width, int align);
  virtual void paintFocus(QPainter *, const QColorGroup &cg, const QRect &r) {}

  void SetEventLine();

friend class CMsgViewTips;
friend class MsgView;
};


//=====UserList====================================================================================
class MsgView : public QListView, public QToolTip
{
  Q_OBJECT
public:
  MsgView (QWidget *parent = 0);
  CUserEvent *currentMsg(void);
  QSize sizeHint() const;

signals:
  void sizeChange(int, int, int);

protected:
  CMsgViewTips *tips;

  virtual void resizeEvent(QResizeEvent *e);
  virtual void maybeTip(const QPoint&);
  void SetEventLines();
};


#endif
