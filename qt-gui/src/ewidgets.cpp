// -*- c-basic-offset: 2 -*-

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qimage.h>
#include <qtextcodec.h>
#ifdef USE_KDE
#include <kapp.h>
#include <kmessagebox.h>
#include <qmessagebox.h>
#else
#include <qmessagebox.h>
#endif
#include <qapplication.h>
#include <qstyle.h>
#include <qdatetime.h>
#include "licq_history.h"
#include "licq_events.h"
#include "mainwin.h"
#include "eventdesc.h"
#include "ewidgets.h"
#include "usercodec.h"
#include "usereventdlg.h"

using namespace std;

bool QueryUser(QWidget *q, QString szQuery, QString szBtn1, QString szBtn2)
{
#ifdef USE_KDE
#if KDE_VERSION >= 290
  return ( KMessageBox::questionYesNo(q, szQuery, QMessageBox::tr("Licq Question"), szBtn1, szBtn2, QString::null, false) == KMessageBox::Yes);
#else
  return ( KMessageBox::questionYesNo(q, szQuery, QMessageBox::tr("Licq Question"), szBtn1, szBtn2, false) == KMessageBox::Yes);
#endif
#else
  return ( QMessageBox::information(q, QMessageBox::tr("Licq Question"), szQuery, szBtn1, szBtn2) == 0);
#endif
}


int QueryUser(QWidget *q, QString szQuery, QString szBtn1, QString szBtn2, QString szBtn3)
{
  return ( QMessageBox::information(q, QMessageBox::tr("Licq Question"), szQuery, szBtn1, szBtn2, szBtn3));
}


void InformUser(QWidget *q, QString sz)
{
  //(void) new CLicqMessageBox(szInfo, QMessageBox::Information, q);
#ifdef USE_KDE
  KMessageBox::information(q, sz, QMessageBox::tr("Licq Information"), QString::null, false);
#else
  QMessageBox::information(q, QMessageBox::tr("Licq Information"), sz, QMessageBox::Ok | QMessageBox::Default);
#endif
}

void WarnUser(QWidget *q, QString sz)
{
  //(void) new CLicqMessageBox(szInfo, QMessageBox::Warning, q);
#ifdef USE_KDE
  KMessageBox::sorry(q, sz, QMessageBox::tr("Licq Warning"), false);
#else
  QMessageBox::warning(q, QMessageBox::tr("Licq Warning"), sz, QMessageBox::Ok | QMessageBox::Default, 0);
#endif
}

void CriticalUser(QWidget *q, QString sz)
{
  //(void) new CLicqMessageBox(szInfo, QMessageBox::Critical, q);
#ifdef USE_KDE
  KMessageBox::error(q, sz, QMessageBox::tr("Licq Error"), false);
#else
  QMessageBox::warning(q, QMessageBox::tr("Licq Error"), sz, QMessageBox::Ok | QMessageBox::Default, 0);
#endif
}

//-----CELabel------------------------------------------------------------------
CELabel::CELabel(bool _bTransparent, QPopupMenu *m, QWidget *parent, char *name)
  : QLabel(parent, name)
{
  mnuPopUp = m;
  m_bTransparent = _bTransparent;
  if (_bTransparent)
  {
#if QT_VERSION < 300
    setAutoMask(true);
#endif
  }
}


void CELabel::polish()
{
  if (!testWState(WState_Polished))
  {
    setWState(WState_Polished);
    if (extraData() != NULL && extraData()->style != NULL)
      extraData()->style->polish(this);
    else
      qApp->polish(this);
  }
}

void CELabel::setPrependPixmap(const QPixmap& p)
{
  if (!addPix.isNull()) clearPrependPixmap();

  addPix = p;
  addIndent = indent();
  setIndent(indent() + p.width() + 2);
  update();
}

void CELabel::clearPrependPixmap()
{
  if (addPix.isNull()) return;

  setIndent(addIndent);
  addPix = QPixmap();
  update();
}

void CELabel::setBold(bool isBold)
{
   QFont newFont(font());
   newFont.setBold(isBold);
   setFont(newFont);
}

void CELabel::setItalic(bool isItalic)
{
   QFont newFont(font());
   newFont.setItalic(isItalic);
   setFont(newFont);
}


void CELabel::setNamedFgColor(char *theColor)
{
   if (theColor == NULL) return;
   QColor c(theColor);
   if (!c.isValid()) return;

   QPalette pal(palette());
#if QT_VERSION >= 210
   pal.setColor(QPalette::Active, QColorGroup::Foreground, c);
   pal.setColor(QPalette::Inactive, QColorGroup::Foreground, c);
#else
   pal.setColor(QPalette::Active, QColorGroup::Text, c);
   pal.setColor(QPalette::Normal, QColorGroup::Text, c);
#endif

   setPalette(pal);
}


void CELabel::setNamedBgColor(char *theColor)
{
   if (theColor == NULL) return;
   QColor c(theColor);
   if (!c.isValid()) return;

   QPalette pal(palette());
// Since Qt sucks we have to use this cheap hack instead of the documented
// and correct way to set the color
#if QT_VERSION < 300
   QColorGroup normal(pal.normal());
#else
   QColorGroup normal(pal.active()); 
#endif
   QColorGroup newNormal(normal.foreground(), c, normal.light(), normal.dark(),
                         normal.mid(), normal.text(), normal.base());
   pal = QPalette(newNormal, newNormal, newNormal);
#if 0
#if QT_VERSION >= 210
   pal.setColor(QPalette::Active, QColorGroup::Background, c);
   pal.setColor(QPalette::Inactive, QColorGroup::Background, c);
#else
   pal.setColor(QPalette::Active, QColorGroup::Background, c);
   pal.setColor(QPalette::Normal, QColorGroup::Background, c);
#endif
#endif

   setPalette(pal);
}

void CELabel::drawContents(QPainter* p)
{
  if(!addPix.isNull())
    p->drawPixmap(addIndent, height() / 2 - addPix.height() / 2, addPix);

  QLabel::drawContents(p);
}

void CELabel::resizeEvent (QResizeEvent *)
{
#if QT_VERSION >= 300
  if (paletteBackgroundPixmap() != NULL)
  {
    // Scale it if it is not transparent
    if (!m_bTransparent)
    {
      QImage im = (paletteBackgroundPixmap()->convertToImage()).smoothScale(width(), height());
      QPixmap pm;
      pm.convertFromImage(im);
      setPaletteBackgroundPixmap(pm);
    }
  }
#else 
  // Resize the background pixmap properly
  if (autoMask()) updateMask();

  if (backgroundPixmap() != NULL)
  {
    QImage im = (backgroundPixmap()->convertToImage()).smoothScale(width(), height());
    QPixmap pm;
    pm.convertFromImage(im);
    setBackgroundPixmap(pm);
  }
#endif
}

void CELabel::mousePressEvent(QMouseEvent* e)
{
  if(e->button() == MidButton)
    emit doubleClicked();
  else if (e->button() == RightButton)
  {
    if (mnuPopUp != NULL)
    {
      QPoint clickPoint(e->x(), e->y());
      mnuPopUp->popup(mapToGlobal(clickPoint));
    }
  }
  else
    QLabel::mousePressEvent(e);
}


void CELabel::mouseDoubleClickEvent(QMouseEvent *)
{
   emit doubleClicked();
}

//-----CEButton-----------------------------------------------------------------
CEButton::CEButton(QPixmap *p1, QPixmap *p2, QPixmap *p3, QWidget *parent, char *name)
   : QPushButton(parent, name)
{
   pmUpFocus = p1;
   pmUpNoFocus = p2;
   pmDown = p3;
   pmCurrent = pmUpNoFocus;
   whenPressed = NoButton;
}

CEButton::CEButton(QString label, QWidget *parent, char *name)
  : QPushButton(label, parent, name)
{
   pmCurrent = pmUpFocus = pmUpNoFocus = pmDown = NULL;
   whenPressed = NoButton;
}

CEButton::CEButton(QWidget *parent, char *name)
  : QPushButton(parent, name)
{
   pmCurrent = pmUpFocus = pmUpNoFocus = pmDown = NULL;
   whenPressed = NoButton;
}

CEButton::~CEButton()
{
  delete pmUpFocus;
  delete pmUpNoFocus;
  delete pmDown;
}


void CEButton::polish()
{
  if (!testWState(WState_Polished))
  {
    setWState(WState_Polished);
    if (extraData() != NULL && extraData()->style != NULL)
      extraData()->style->polish(this);
    else
      qApp->polish(this);
  }
}

void CEButton::enterEvent (QEvent *)
{
   pmCurrent = pmUpFocus;
   if (pmCurrent != NULL) repaint();
}

void CEButton::leaveEvent (QEvent *)
{
   pmCurrent = pmUpNoFocus;
   if (pmCurrent != NULL) repaint();
}

void CEButton::mouseReleaseEvent(QMouseEvent *e)
{
   pmCurrent = pmUpNoFocus;
   if (pmCurrent != NULL) repaint();
   QPushButton::mouseReleaseEvent(e);
}

void CEButton::mousePressEvent(QMouseEvent *e)
{
   pmCurrent = pmDown;
   if (pmCurrent != NULL) repaint();
   whenPressed = e->state();
   QPushButton::mousePressEvent(e);
}


void CEButton::drawButton(QPainter *p)
{
   if (pmCurrent == NULL)
   {
      QPushButton::drawButton(p);
      return;
   }

   p->drawPixmap(0, 0, *pmCurrent);
}

void CEButton::setNamedFgColor(char *theColor)
{
   if (theColor == NULL) return;

   QPalette pal(palette());
#if QT_VERSION < 300
   QColorGroup normal(pal.normal());
#else
   QColorGroup normal(pal.active());
#endif
   QColorGroup newNormal(normal.foreground(), normal.background(), normal.light(), normal.dark(),
                         normal.mid(), QColor(theColor), normal.base());
   setPalette(QPalette(newNormal, pal.disabled(), newNormal));
}

void CEButton::setNamedBgColor(char *theColor)
{
   if (theColor == NULL) return;

   QPalette pal(palette());
#if QT_VERSION < 300
   QColorGroup normal(pal.normal());
#else
   QColorGroup normal(pal.active());
#endif
   QColorGroup newNormal(normal.foreground(), QColor(theColor), normal.light(), normal.dark(),
                         normal.mid(), normal.text(), normal.base());
   setPalette(QPalette(newNormal, pal.disabled(), newNormal));
}


//-----CEComboBox---------------------------------------------------------------
CEComboBox::CEComboBox(bool _bAppearEnabledAlways, QWidget *parent, char *name)
   : QComboBox(false, parent, name)
{
   m_bAppearEnabledAlways = _bAppearEnabledAlways;
   if (m_bAppearEnabledAlways)
   {
      QPalette pal(palette());
#if QT_VERSION < 300
      setPalette(QPalette(pal.normal(), pal.normal(), pal.normal()));
#else
      setPalette(QPalette(pal.active(), pal.active(), pal.active()));
#endif
   }
}


void CEComboBox::setNamedFgColor(char *theColor)
{
   if (theColor == NULL) return;

   QPalette pal(palette());
#if QT_VERSION < 300
   QColorGroup normal(pal.normal());
#else
   QColorGroup normal(pal.active());
#endif
   QColorGroup newNormal(normal.foreground(), normal.background(), normal.light(), normal.dark(),
                         normal.mid(), QColor(theColor), normal.base());
   setPalette(QPalette(newNormal, pal.disabled(), newNormal));
}


void CEComboBox::setNamedBgColor(char *theColor)
{
   if (theColor == NULL) return;

   QPalette pal(palette());
#if QT_VERSION < 300
   QColorGroup normal(pal.normal());
#else
   QColorGroup normal(pal.active());
#endif
   QColorGroup newNormal(normal.foreground(), normal.background(), normal.light(), normal.dark(),
                         normal.mid(), normal.text(), QColor(theColor));
   setPalette(QPalette(newNormal, pal.disabled(), newNormal));
}


//-----CInfoField::constructor--------------------------------------------------
CInfoField::CInfoField(QWidget *parent, bool readonly)
  : QLineEdit(parent)
{
  baseRO = palette().disabled().base();
#if QT_VERSION < 300
  baseRW = palette().normal().base();
#else
  baseRW = palette().active().base();
#endif

  // Set colors
  SetReadOnly(readonly);
}

void CInfoField::SetReadOnly(bool b)
{
#if QT_VERSION < 300
  QColorGroup cg(palette().normal().foreground(),
                 palette().normal().background(),
                 palette().normal().light(),
                 palette().normal().dark(),
                 palette().normal().mid(),
                 palette().normal().text(),
                 b ? baseRO : baseRW);
#else
  QColorGroup cg(palette().active().foreground(),
                 palette().active().background(),
                 palette().active().light(),
                 palette().active().dark(),
                 palette().active().mid(),
                 palette().active().text(),
                 b ? baseRO : baseRW);
#endif

  setPalette(QPalette(cg, palette().disabled(), cg));
  setReadOnly(b);
}



void CInfoField::setData(const char *data)
{
  setText(QString::fromLocal8Bit(data));
}


void CInfoField::setData(QString data)
{
  setText(data);
}


void CInfoField::setData(const unsigned long data)
{
  char t[32];
  sprintf(t, "%lu", data);
  setData(t);
}



// -----------------------------------------------------------------------------

CHistoryWidget::CHistoryWidget(QWidget* parent, const char* name)
  : MLView(parent, name)
{
#if QT_VERSION >= 300
  setTextFormat(RichText);
#endif
};

// -----------------------------------------------------------------------------

#if QT_VERSION < 300
void CHistoryWidget::paintCell(QPainter* p, int row, int col)
{
  QPalette& pal = const_cast<QPalette&>(palette());

  QString s = stringShown(row);
  bool bold = (s[0] == '\001' || s[0] == '\002');
  if(bold ^ p->font().bold()) {
    QFont f(p->font());
    f.setBold(bold);
    p->setFont(f);
  }

  int i= row;
  pal.setColor(QColorGroup::Text, Qt::blue);
  while(i >= 0)
  {
    QString s2 = stringShown(i--);
    if (s2[0] == '\002')  break;
    if(s2[0] == '\001')
    {
      pal.setColor(QColorGroup::Text, Qt::red);
      break;
    }
  }

  MLView::paintCell(p, row, col);
}
#endif

//- Message View Widget ---------------------------------------------------------
CMessageViewWidget::CMessageViewWidget(const char *szId, unsigned long nPPID,
  CMainWindow *m, QWidget* parent, const char * name)
  : CHistoryWidget(parent, name)
{
  m_szId = szId ? strdup(szId) : 0;
  m_nPPID = nPPID;
  mainwin = m;

  // add all unread messages.
  vector<CUserEvent*> newEventList;
  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_W);
  if (u != NULL && u->NewMessages() > 0)
  {
    for (unsigned short i = 0; i < u->NewMessages(); i++)
    {
      CUserEvent *e = u->EventPeek(i);
      if (e->Direction() == D_RECEIVER && e->SubCommand() == ICQ_CMDxSUB_MSG)
        newEventList.push_back(e);
    }
  }
  gUserManager.DropUser(u);
  for (unsigned short i = 0; i < newEventList.size(); i++)
    addMsg(newEventList[i]);
}

CMessageViewWidget::CMessageViewWidget(unsigned long _nUin, CMainWindow *m, QWidget* parent, const char * name)
:CHistoryWidget(parent,name)
{
  m_nUin= _nUin;
  mainwin = m;
/*
  // add all unread messages.
  vector<CUserEvent*> newEventList;
  ICQUser *u = gUserManager.FetchUser(_nUin, LOCK_W);
  if (u != NULL && u->NewMessages() > 0)
  {
    for (unsigned short i = 0; i < u->NewMessages(); i++)
    {
      CUserEvent *e = u->EventPeek(i);
      if (e->Direction() == D_RECEIVER && e->SubCommand() == ICQ_CMDxSUB_MSG)
	newEventList.push_back(e);
    }
  }
  gUserManager.DropUser(u);
  for (unsigned short i = 0; i < newEventList.size(); i++)
    addMsg(newEventList[i]); */
}

CMessageViewWidget::~CMessageViewWidget()
{
  if (m_szId)
    free(m_szId);
}

void CMessageViewWidget::addMsg(ICQEvent * _e)
{
  if (strcmp(_e->Id(), m_szId) == 0 && _e->PPID() == m_nPPID &&
    _e->UserEvent())
    addMsg( _e->UserEvent() );
}

void CMessageViewWidget::addMsg(CUserEvent* e )
{
  QDateTime date;
  date.setTime_t(e->Time());
  QString sd = date.time().toString();

  QString contactName;
  QTextCodec *codec = QTextCodec::codecForLocale();

  {

    ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
    if (u != NULL)
    {
      codec = UserCodec::codecForICQUser(u);
      if (e->Direction() == D_RECEIVER)
        contactName = codec->toUnicode(u->GetAlias());
      gUserManager.DropUser(u);
    }
  }

  if (e->Direction() != D_RECEIVER)
  {
    ICQOwner *o = gUserManager.FetchOwner(m_nPPID, LOCK_R);
    if (o != NULL)
    {
      // Don't use this codec to decode our conversation with the contact
      // since we're using the contact's encoding, not ours.
      QTextCodec *ownerCodec = UserCodec::codecForICQUser(o);
      contactName = ownerCodec->toUnicode(o->GetAlias());
      gUserManager.DropOwner();
    }
  }

  QString s;

#if QT_VERSION >= 300
  QString messageText;
  if (e->SubCommand() == ICQ_CMDxSUB_SMS)
     messageText = QString::fromUtf8(e->Text());
  else
     messageText = codec->toUnicode(e->Text());

  const char *color = (e->Direction() == D_RECEIVER) ? "red" : "blue";

  // QTextEdit::append adds a paragraph break so we don't have to.
  s.sprintf("<html><body><font color=\"%s\"><b>%s%s [%c%c%c%c] %s:</b></font><br>"
            "<font color=\"%s\">%s</font></body></html>",
            color,
            e->SubCommand() == ICQ_CMDxSUB_MSG ? "" :
              (EventDescription(e) + " ").utf8().data(),
            sd.utf8().data(),
            e->IsDirect() ? 'D' : '-',
            e->IsMultiRec() ? 'M' : '-',
            e->IsUrgent() ? 'U' : '-',
            e->IsEncrypted() ? 'E' : '-',
            contactName.utf8().data(),
            color,
            MLView::toRichText(messageText, true).utf8().data()
           );
  append(s);
#else
  QString messageText = codec->toUnicode(e->Text());
  s.sprintf("%c%s%s [%c%c%c%c] %s:\n%s",
            (e->Direction() == D_RECEIVER) ? '\001' : '\002',
            e->SubCommand() == ICQ_CMDxSUB_MSG ? "" :
              (EventDescription(e) + " ").utf8().data(),
            sd.utf8().data(),
            e->IsDirect() ? 'D' : '-',
            e->IsMultiRec() ? 'M' : '-',
            e->IsUrgent() ? 'U' : '-',
            e->IsEncrypted() ? 'E' : '-',
            contactName.utf8().data(),
            messageText.utf8().data()
           );
  append(s);
#endif
#if QT_VERSION < 300
  repaint(false);
#endif
  GotoEnd();

  QWidget *parent = NULL;
  if (parentWidget() &&
      parentWidget()->parentWidget() &&
      parentWidget()->parentWidget()->parentWidget())
    parent = parentWidget()->parentWidget()->parentWidget();
  if (
#if QT_VERSION >= 300
      parent && parent->isActiveWindow() &&
      (!mainwin->m_bTabbedChatting || (mainwin->m_bTabbedChatting &&
       mainwin->userEventTabDlg->tabIsSelected(parent))) &&
#endif
      e->Direction() == D_RECEIVER && e->SubCommand() == ICQ_CMDxSUB_MSG)
  {
    UserSendCommon *s = static_cast<UserSendCommon*>(parent);
    QTimer::singleShot(s->clearDelay, s, SLOT(slot_ClearNewEvents()));
  }
}

#include "ewidgets.moc"
