#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qpainter.h>
#include <qdatetime.h>

#include "message.h"
#include "messagebox.h"
#include "eventdesc.h"

MsgViewItem::MsgViewItem(CUserEvent *theMsg, unsigned short theIndex, QListView *parent) : QListViewItem(parent)
{
   index = (unsigned short)theIndex;
   msg = theMsg->Copy();
   QDateTime d;
   d.setTime_t(msg->Time());

   setText(0, "*");
   setText(1, EventDescription(msg));
   setText(2, d.toString());
   char szFlags[4];
   szFlags[0] = msg->IsDirect() ? 'D' : ' ';
   szFlags[1] = msg->IsUrgent() ? 'U' : ' ';
   szFlags[2] = msg->IsMultiRec() ? 'M' : ' ';
   szFlags[3] = '\0';
   setText(3, szFlags);
   setText(4, msg->IsLicq() ? msg->LicqVersionStr() : "");
}

MsgViewItem::~MsgViewItem(void)
{
   delete msg;
}


void MsgViewItem::paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int align )
{
   if (index != -1)
      QListViewItem::paintCell(p, QColorGroup(cg.foreground(), cg.background(), cg.light(), cg.dark(), cg.mid(), QColor("blue"), cg.base()), column, width, align);
   else
      QListViewItem::paintCell(p, cg, column, width, align);

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
  addColumn(tr("Event Type"), 115);
  addColumn(tr("Time Received"), 115);
  addColumn(tr("Flags"), 50);
  addColumn(tr("Licq"), 50);
  setColumnAlignment(0, AlignCenter);
  setColumnAlignment(3, AlignCenter);
  setColumnAlignment(4, AlignRight);
  setAllColumnsShowFocus (true);
  setVScrollBarMode(AlwaysOn);

  QPalette pal(palette());
  QColorGroup normal(pal.normal());
  QColorGroup newNormal(normal.foreground(), normal.background(), normal.light(), normal.dark(),
                        normal.mid(), normal.text(), QColor(192, 192, 192));
  setPalette(QPalette(newNormal, pal.disabled(), newNormal));
  setFrameStyle(QFrame::Panel | QFrame::Sunken);
  setMinimumHeight(60);
}

CUserEvent *MsgView::currentMsg(void)
{
   if (currentItem() == NULL) return (NULL);
   return (((MsgViewItem *)currentItem())->msg);
}


void MsgView::resizeEvent(QResizeEvent *e)
{
  QListView::resizeEvent(e);
  QScrollBar *s = verticalScrollBar();
  setColumnWidth(1, width() - 240 - s->width());
}


//-----MsgView::mouseReleaseEvent------------------------------------------------------------------
void MsgView::mouseReleaseEvent(QMouseEvent *e)
{
   // does nothing special
   QListView::mouseReleaseEvent(e);
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
