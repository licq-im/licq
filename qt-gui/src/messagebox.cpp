#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qpainter.h>
#include "messagebox.h"
#include "licq-locale.h"

MsgViewItem::MsgViewItem(CUserEvent *theMsg, unsigned short theIndex, QListView *parent) : QListViewItem(parent)
{
   index = (unsigned short)theIndex;
   msg = theMsg->Copy();

   setText(0, "*");
   setText(1, msg->Description());
   setText(2, msg->Time());
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
MsgView::MsgView (QWidget *parent = 0, const char *name = 0) : QListView(parent, name)
{
  addColumn(_("N"), 20);
  addColumn(_("Event Type"), 115);
  addColumn(_("Time Received"), 115);
  addColumn(_("Flags"), 50);
  addColumn(_("Licq"), 50);
  setColumnAlignment(0, AlignCenter);
  setColumnAlignment(3, AlignCenter);
  setColumnAlignment(4, AlignRight);
  setAllColumnsShowFocus (true);

  QPalette pal(palette());
  QColorGroup normal(pal.normal());
  QColorGroup newNormal(normal.foreground(), normal.background(), normal.light(), normal.dark(),
                        normal.mid(), normal.text(), QColor(192, 192, 192));
  setPalette(QPalette(newNormal, pal.disabled(), newNormal));
  //setStyle(WindowsStyle);
  setFrameStyle(QFrame::Panel | QFrame::Sunken);
}

CUserEvent *MsgView::currentMsg(void)
{
   if (currentItem() == NULL) return (NULL);
   return (((MsgViewItem *)currentItem())->msg);
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
