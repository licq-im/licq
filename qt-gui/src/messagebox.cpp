#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qpainter.h>
#include <qdatetime.h>
#include <qstring.h>
#include <qheader.h>

#include "message.h"
#include "messagebox.h"
#include "eventdesc.h"

MsgViewItem::MsgViewItem(CUserEvent *theMsg, unsigned short theIndex, QListView *parent) : QListViewItem(parent)
{
  index = (unsigned short)theIndex;
  msg = theMsg->Copy();
  QDateTime d;
  d.setTime_t(msg->Time());
  QString sd = d.toString();
  sd.truncate(sd.length() - 5);

  setText(0, "*");
  setText(1, EventDescription(msg));
  setText(2, sd);
  /*char szFlags[4];
  szFlags[0] = msg->IsDirect() ? 'D' : ' ';
  szFlags[1] = msg->IsUrgent() ? 'U' : ' ';
  szFlags[2] = msg->IsMultiRec() ? 'M' : ' ';
  szFlags[3] = '\0';
  setText(3, szFlags);
  setText(4, msg->IsLicq() ? msg->LicqVersionStr() : "");*/
}

MsgViewItem::~MsgViewItem(void)
{
  delete msg;
}


void MsgViewItem::paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int align )
{
  QFont f(p->font());
  if (index != -1)
  {
    f.setItalic(true);
    p->setFont(f);
    QListViewItem::paintCell(p, QColorGroup(cg.foreground(), cg.background(), cg.light(), cg.dark(), cg.mid(), QColor("blue"), cg.base()), column, width, align);
  }
  else
  {
    f.setItalic(false);
    p->setFont(f);
    QListViewItem::paintCell(p, cg, column, width, align);
  }

  // add line to bottom and right side
  p->setPen(cg.mid());
  p->drawLine(0, height() - 1, width - 1, height() - 1);
  p->drawLine(width - 1, 0, width - 1, height() - 1);
}


//-----MsgView::constructor------------------------------------------------------------------------
MsgView::MsgView (QWidget *parent, const char *name)
  : QListView(parent, name)
{
  addColumn(tr("N"), 20);
  //addColumn(tr("Event Type"), 115);
  addColumn(tr("Event Type"), 215);
  addColumn(tr("Time Received"), 115);
  //addColumn(tr("Flags"), 50);
  //addColumn(tr("Licq"), 50);
  setColumnAlignment(0, AlignCenter);
  //setColumnAlignment(3, AlignCenter);
  //setColumnAlignment(4, AlignRight);
  setAllColumnsShowFocus (true);
  setVScrollBarMode(AlwaysOn);

  header()->hide();

  QPalette pal(palette());
  QColorGroup normal(pal.normal());
  QColorGroup newNormal(normal.foreground(), normal.background(), normal.light(), normal.dark(),
                        normal.mid(), normal.text(), QColor(192, 192, 192));
  setPalette(QPalette(newNormal, pal.disabled(), newNormal));
  setFrameStyle(QFrame::Panel | QFrame::Sunken);
  setMinimumHeight(40);

  tips = new CMsgViewTips(this);
}

CUserEvent *MsgView::currentMsg(void)
{
   if (currentItem() == NULL) return (NULL);
   return (((MsgViewItem *)currentItem())->msg);
}


QSize MsgView::sizeHint() const
{
  QSize s = QListView::sizeHint();
  s.setHeight(minimumHeight());

  return s;
}


void MsgView::resizeEvent(QResizeEvent *e)
{
  QListView::resizeEvent(e);
  QScrollBar *s = verticalScrollBar();
  setColumnWidth(2, width() - 240 - s->width());
}


//-----MsgView::markRead---------------------------------------------------------------------------
void MsgView::markRead(short index)
{
   MsgViewItem *e = (MsgViewItem *)firstChild();
   if (e == NULL) return;

   if (e->index == index)
   {
      e->index = -1;
      e->setText(0, "");
   }
   else if (e->index > index)
      e->index--;

   while ((e = (MsgViewItem *)e->nextSibling()) != NULL)
   {
      if (e->index == index)
      {
         e->index = -1;
         e->setText(0, "");
      }
      else if (e->index > index)
         e->index--;
   }
}


//=====CMsgItemTips===============================================================================

CMsgViewTips::CMsgViewTips(MsgView* parent)
  : QToolTip(parent)
{
  // nothing to do
}

void CMsgViewTips::maybeTip(const QPoint& c)
{
  QPoint p(c);
  QListView *w = (QListView *)parentWidget();
  if (w->header()->isVisible())
    p.setY(p.y()-w->header()->height());

  MsgViewItem *item = (MsgViewItem*) w->itemAt(p);

  if (item == NULL) return;

  QRect r(w->itemRect(item));
  if (w->header()->isVisible())
    r.moveBy(0, w->header()->height());

  QString s(item->msg->IsDirect() ? w->tr("Direct") : w->tr("Server"));
  if (item->msg->IsUrgent())
    s += QString(" / ") + w->tr("Urgent");
  if (item->msg->IsMultiRec())
    s += QString(" / ") + w->tr("Multiple Recipients");
  if (item->msg->IsLicq())
    s += QString(" / Licq ") + QString::fromLocal8Bit(item->msg->LicqVersionStr());

  tip(r, s);
}


