// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2006 Licq developers
 *
 * Licq is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Licq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Licq; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qimage.h>
#include <qtextcodec.h>
#include <qtabwidget.h>
#include <qtabbar.h>
#include <qcursor.h>
#include <qevent.h>
#ifdef USE_KDE
#include <kapp.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <qmessagebox.h>
#else
#include <qmessagebox.h>
#endif
#include <qapplication.h>
#include <qstyle.h>
#include <qdatetime.h>
#include <qlayout.h>
#include <qheader.h>
#include <qstringlist.h>
#include <ctype.h>
#include <algorithm>

#include "licq_history.h"
#include "licq_events.h"
#include "licq_user.h"
#include "mainwin.h"
#include "eventdesc.h"
#include "ewidgets.h"
#include "usercodec.h"
#include "usereventdlg.h"

using namespace std;

/*! \brief Dialog with configurable yes/no buttons
 *
 * In it's easiest form, this dialog displays szQuery and the buttons szBtn1 and szBtn2. 
 * szBtn1 means true/yes, 
 * szBtn2 means false/no. 
 * 
 * When bConfirmYes is true, then a second dialog asks the user to confirm his positive 
 * decision, the displayed confirmation message is passed using the QString szConfirmYes.
 *
 * When bConfirmNo is true, then a second dialog asks the user to confirm his negative 
 * decision, the displayed confirmation message is passed using the QString szConfirmNo.
 */
bool QueryUser(QWidget *q, QString szQuery, QString szBtn1, QString szBtn2, bool bConfirmYes, QString szConfirmYes, bool bConfirmNo, QString szConfirmNo)
{
  bool result;

#ifdef USE_KDE
  result = ( KMessageBox::questionYesNo(q, szQuery, QMessageBox::tr("Licq Question"), szBtn1, szBtn2, QString::null, false) == KMessageBox::Yes);
  // The user must confirm his decision!
  if(result == true && bConfirmYes && szConfirmYes)
    result = ( KMessageBox::questionYesNo(q, szConfirmYes, QMessageBox::tr("Licq Question"), QMessageBox::tr("Yes"), QMessageBox::tr("No"), QString::null, false) == KMessageBox::Yes);
  else if(result == false && bConfirmNo && szConfirmNo)
    result = ( KMessageBox::questionYesNo(q, szConfirmNo, QMessageBox::tr("Licq Question"), QMessageBox::tr("Yes"), QMessageBox::tr("No"), QString::null, false) == KMessageBox::Yes);
#else
  result = ( QMessageBox::question(q, QMessageBox::tr("Licq Question"), szQuery, szBtn1, szBtn2) == 0);
  // The user must confirm his decision!
  if(result == true && bConfirmYes && szConfirmYes)
    result = ( QMessageBox::question(q, QMessageBox::tr("Licq Question"), szConfirmYes, QMessageBox::tr("Yes"), QMessageBox::tr("No")) == 0);
  else if(result == false && bConfirmNo && szConfirmNo)
    result = ( QMessageBox::question(q, QMessageBox::tr("Licq Question"), szConfirmNo, QMessageBox::tr("Yes"), QMessageBox::tr("No")) == 0);
#endif
  
  return result;
}


int QueryUser(QWidget *q, QString szQuery, QString szBtn1, QString szBtn2, QString szBtn3)
{
  return ( QMessageBox::question(q, QMessageBox::tr("Licq Question"), szQuery, szBtn1, szBtn2, szBtn3));
}


void InformUser(QWidget *q, QString sz)
{
  CLicqMessageManager::Instance()->addMessage(QMessageBox::Information, sz, q);
}

void WarnUser(QWidget *q, QString sz)
{
  CLicqMessageManager::Instance()->addMessage(QMessageBox::Warning, sz, q);
}

void CriticalUser(QWidget *q, QString sz)
{
  CLicqMessageManager::Instance()->addMessage(QMessageBox::Critical, sz, q);
}

//-----CELabel------------------------------------------------------------------
CELabel::CELabel(bool _bTransparent, QPopupMenu *m, QWidget *parent, char *name)
  : QLabel(parent, name)
{
  mnuPopUp = m;
  m_bTransparent = _bTransparent;
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

void CELabel::addPixmap(const QPixmap &p)
{
  m_lPixmaps.push_back(p);
  if (m_lPixmaps.size() == 1)
    startingIndent = indent();
  update();
}

void CELabel::clearPixmaps()
{
  if (m_lPixmaps.size() == 0) return;
  m_lPixmaps.clear();
  setIndent(startingIndent);
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
   pal.setColor(QPalette::Active, QColorGroup::Foreground, c);
   pal.setColor(QPalette::Inactive, QColorGroup::Foreground, c);

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
   QColorGroup normal(pal.active());
   QColorGroup newNormal(normal.foreground(), c, normal.light(), normal.dark(),
                         normal.mid(), normal.text(), normal.base());
   pal = QPalette(newNormal, newNormal, newNormal);
#if 0
   pal.setColor(QPalette::Active, QColorGroup::Background, c);
   pal.setColor(QPalette::Inactive, QColorGroup::Background, c);
#endif

   setPalette(pal);
}

void CELabel::drawContents(QPainter* p)
{
  if(!addPix.isNull())
    p->drawPixmap(addIndent, height() / 2 - addPix.height() / 2, addPix);

  if (m_lPixmaps.size())
  {
    list<QPixmap>::iterator it;
    int i = indent();
    for (it = m_lPixmaps.begin(); it != m_lPixmaps.end(); it++)
    {
      p->drawPixmap(i, height() / 2 - it->height() / 2, *it);
      i += it->width() + 2;
    }
  }

  QLabel::drawContents(p);
}

void CELabel::resizeEvent (QResizeEvent *)
{
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
   QColorGroup normal(pal.active());
   QColorGroup newNormal(normal.foreground(), normal.background(), normal.light(), normal.dark(),
                         normal.mid(), QColor(theColor), normal.base());
   setPalette(QPalette(newNormal, pal.disabled(), newNormal));
}

void CEButton::setNamedBgColor(char *theColor)
{
   if (theColor == NULL) return;

   QPalette pal(palette());
   QColorGroup normal(pal.active());
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
      setPalette(QPalette(pal.active(), pal.active(), pal.active()));
   }
}


void CEComboBox::setNamedFgColor(char *theColor)
{
   if (theColor == NULL) return;

   QPalette pal(palette());
   QColorGroup normal(pal.active());
   QColorGroup newNormal(normal.foreground(), normal.background(), normal.light(), normal.dark(),
                         normal.mid(), QColor(theColor), normal.base());
   setPalette(QPalette(newNormal, pal.disabled(), newNormal));
}


void CEComboBox::setNamedBgColor(char *theColor)
{
   if (theColor == NULL) return;

   QPalette pal(palette());
   QColorGroup normal(pal.active());
   QColorGroup newNormal(normal.foreground(), normal.background(), normal.light(), normal.dark(),
                         normal.mid(), normal.text(), QColor(theColor));
   setPalette(QPalette(newNormal, pal.disabled(), newNormal));
}

//CETabBar
void CETabBar::setTabColor(int id, const QColor &color)
{
  QTab *t = tab(id);
  if (t)
  {
    mTabColors.insert(id, color);
    repaint(t->rect(), false);
  }
}

const QColor &CETabBar::tabColor(int id) const
{
  if (mTabColors.contains(id))
    return mTabColors[id];
    
  return colorGroup().foreground();
}
void CETabBar::removeTab(QTab *t)
{
  mTabColors.remove(t->identifier());
  QTabBar::removeTab(t);
}

void CETabBar::setPreviousTab()
{
  if (currentTab() == -1)
    return; // No current tab

  int tab = indexOf(currentTab()) - 1;
  if (tab < 0)
    tab = count() - 1;

  setCurrentTab(tabAt(tab));
}

void CETabBar::setNextTab()
{
  if (currentTab() == -1)
    return; // No current tab

  int tab = indexOf(currentTab()) + 1;
  if (tab >= count())
    tab = 0;

  setCurrentTab(tabAt(tab));
}

void CETabBar::paintLabel(QPainter* p, const QRect &br,
                          QTab* t, bool has_focus) const
{
  QRect r = br;
  bool selected = currentTab() == t->identifier();
  if (t->iconSet())
  {
    // the tab has an iconset, draw it in the right mode
    QIconSet::Mode mode = (t->isEnabled() && isEnabled())
         ? QIconSet::Normal : QIconSet::Disabled;
    if (mode == QIconSet::Normal && has_focus)
      mode = QIconSet::Active;
    
    QPixmap pixmap = t->iconSet()->pixmap(QIconSet::Small, mode);
    int pixw = pixmap.width();
    int pixh = pixmap.height();
    r.setLeft(r.left() + pixw + 4);
    r.setRight(r.right() + 2);

#if QT_VERSION >= 0x030200 
    int xoff = 0, yoff = 0;
    if (!selected)
    {
      xoff = style().pixelMetric(QStyle::PM_TabBarTabShiftHorizontal, this);
      yoff = style().pixelMetric(QStyle::PM_TabBarTabShiftVertical, this);
    }
    
    p->drawPixmap(br.left() + 2 + xoff, br.center().y()-pixh/2 + yoff, pixmap);
#else
    p->drawPixmap(br.left() + 2 + ((selected == true) ? 0 : 2),
                  br.center().y()-pixh/2 + ((selected == true) ? 0 : 2),
                  pixmap);
#endif
  }
    
  QStyle::SFlags flags = QStyle::Style_Default;

  if (isEnabled() && t->isEnabled())
    flags |= QStyle::Style_Enabled;
  if (has_focus)
    flags |= QStyle::Style_HasFocus;
  if (selected)
    flags |= QStyle::Style_Selected;
  //else if (t == d->pressed)
  //  flags |= QStyle::Style_Sunken;
  
  if (t->rect().contains(mapFromGlobal(QCursor::pos())))
    flags |= QStyle::Style_MouseOver;
      
  QColorGroup cg(colorGroup());
  if (mTabColors.contains(t->identifier()))
     cg.setColor(QColorGroup::Foreground, mTabColors[t->identifier()]);
  
  style().drawControl(QStyle::CE_TabBarLabel, p, this, r,
                      t->isEnabled() ? cg: palette().disabled(),
                      flags, QStyleOption(t));
}

void CETabBar::wheelEvent(QWheelEvent *e)
{
  if (count() <= 1 || !hasMouse())
  {
    e->ignore();
    return;
  }

  if (e->delta() > 0)
    setPreviousTab();
  else
    setNextTab();
}

void CETabBar::mousePressEvent(QMouseEvent* e)
{
  if ((e->button() & Qt::MouseButtonMask) == Qt::MidButton)
  {
    QTab* tab = selectTab(e->pos());
    if (tab != 0)
      clickedTab = indexOf(tab->identifier());
  }

  QTabBar::mousePressEvent(e);
}

void CETabBar::mouseReleaseEvent(QMouseEvent* e)
{
  if ((e->button() & Qt::MouseButtonMask) == Qt::MidButton)
  {
    QTab* tab = selectTab(e->pos());
    if (tab != 0)
    {
      int t = indexOf(tab->identifier());
      if (t == clickedTab)
        emit middleClick(t);
    }
  }

  clickedTab = -1;
  QTabBar::mouseReleaseEvent(e);
}

//CETabWidget
CETabWidget::CETabWidget(QWidget *parent, const char *name, WFlags f)
  : QTabWidget(parent, name, f)
{
  CETabBar* tb = new CETabBar(this, "tabbar");
  setTabBar(tb);
  connect(tb, SIGNAL(middleClick(int)), this, SLOT(slot_middleClick(int)));
}

void CETabWidget::setTabColor(QWidget *w, const QColor &color)
{
  QTab *t = tabBar()->tabAt(indexOf(w));
  if (t)
    static_cast<CETabBar *>(tabBar())->setTabColor(t->identifier(), color);
}

void CETabWidget::setPreviousPage()
{
  static_cast<CETabBar *>(tabBar())->setPreviousTab();
}

void CETabWidget::setNextPage()
{
  static_cast<CETabBar *>(tabBar())->setNextTab();
}

void CETabWidget::wheelEvent(QWheelEvent *e)
{
  if (count() <= 1)
  {
    e->ignore();
    return;
  }

  QTabBar *tabs = tabBar();
  const bool cursorAboveTabBar = (e->y() < tabs->y());
  const bool cursorBelowTabBar = (e->y() > (tabs->y() + tabs->height()));
  if (cursorAboveTabBar || cursorBelowTabBar)
  {
    e->ignore();
    return;
  }

  if (e->delta() > 0)
    setPreviousPage();
  else
    setNextPage();
}

void CETabWidget::slot_middleClick(int t)
{
  QWidget* p = page(t);
  if (p)
    emit middleClick(p);
}

//-----CInfoField::constructor--------------------------------------------------
CInfoField::CInfoField(QWidget *parent, bool readonly)
  : QLineEdit(parent)
{
  baseRO = palette().disabled().base();
  baseRW = palette().active().base();

  // Set colors
  SetReadOnly(readonly);
}

void CInfoField::SetReadOnly(bool b)
{
  QColorGroup cg(palette().active().foreground(),
                 palette().active().background(),
                 palette().active().light(),
                 palette().active().dark(),
                 palette().active().mid(),
                 palette().active().text(),
                 b ? baseRO : baseRW);

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

void CInfoField::setData(unsigned long data)
{
  setText(QString::number(data));
}

void CInfoField::setDateTime(uint timestamp)
{
  if (timestamp == 0)
    setText(tr("Unknown"));
  else
  {
    QDateTime t;
    t.setTime_t(timestamp);
    setText(t.toString());
  }
}

void CInfoField::keyPressEvent(QKeyEvent *e)
{
  const bool isShift   = e->state() & ShiftButton;
  const bool isControl = e->state() & ControlButton;

  if (isShift && e->key() == Key_Insert)
    return paste();

  if (isShift && e->key() == Key_Delete)
    return cut();

  if (isControl && e->key() == Key_Insert)
    return copy();

  QLineEdit::keyPressEvent(e);
}

// -----------------------------------------------------------------------------

CTimeZoneField::CTimeZoneField(QWidget *parent)
    : QSpinBox(-24, 24, 1, parent)
{
  // The world is round so let timezones wrap
  setWrapping(true);

  // Plus and minus seems more fitting than up and down
  setButtonSymbols(QSpinBox::PlusMinus);

  // Force the input to be in format GMT+500, GMT-1030, etc...
  setPrefix("GMT");
  setValidator(new QRegExpValidator(QRegExp("^[\\+\\-](1[012]|\\d)[03]0$|^Unknown$"), this));

  // Allow the value to be undefined as well. This will replace the lowest value (-24)
  setSpecialValueText(tr("Unknown"));
}

void CTimeZoneField::setData(char data)
{
  // The spinbox uses the lowest value to mark the undefined state but the constant is some other value so we need to change it
  // For all defined values, the sign is inverted
  setValue(data == TIMEZONE_UNKNOWN ? undefinedValue : static_cast<int>(-data));
}

char CTimeZoneField::data()
{
  int v = value();
  if (v == undefinedValue)
    return TIMEZONE_UNKNOWN;
  return static_cast<char>(-v);
}

QString CTimeZoneField::mapValueToText(int v)
{
  // The internal value in the spinbox is 30min intervals so convert it to something more readable
  return QString("%1%2%3").arg(v < 0 ? "-" : "+").arg(abs(v) / 2).arg(v % 2 ? "30" : "00");
}

int CTimeZoneField::mapTextToValue(bool *ok)
{
  // The user entered something so now we must try and convert it back to the internal int
  QRegExp rx("^(\\+|-)(\\d+)(0|3)0$");
  if (rx.search(cleanText()) == -1)
  {
    *ok = false;
    return 0;
  }
  int ret = rx.cap(2).toInt() * 2;
  if (rx.cap(3) == "3")
    ret++;
  if (rx.cap(1) == "-")
    ret = -ret;
  *ok = true;
  return ret;
}


//- Message View Widget ---------------------------------------------------------

QStringList CMessageViewWidget::getStyleNames(bool includeHistoryStyles)
{
  static const char *const styleNames[] = {
    QT_TR_NOOP("Default"),
    QT_TR_NOOP("Compact"),
    QT_TR_NOOP("Tiny"),
    QT_TR_NOOP("Table"),
    QT_TR_NOOP("Long"),
    QT_TR_NOOP("Wide")
  };

  int listLength;

  // Style 5 (Wide) is currently only supported in buffered mode which is not used for chat
  if(includeHistoryStyles)
    listLength = 6;
  else
    listLength = 5;

  QStringList styleList;
  for(int i = 0; i < listLength; ++i)
    styleList.append(tr(styleNames[i]));

  return styleList;
}

CMessageViewWidget::CMessageViewWidget(const char *szId, unsigned long nPPID,
  CMainWindow *m, QWidget* parent, const char *name, bool historyMode)
  : MLView(parent, name)
{
  setTextFormat(RichText);
  m_szId = szId ? strdup(szId) : 0;
  m_nPPID = nPPID;
  if (historyMode)
  {
    m_useBuffer = true;
    m_nMsgStyle = m->m_histMsgStyle;
    m_nDateFormat = m->m_histDateFormat;
    m_extraSpacing = m->m_histVertSpacing;
    m_appendLineBreak = false;
  }
  else
  {
    m_useBuffer = false;
    m_nMsgStyle = m->m_chatMsgStyle;
    m_nDateFormat = m->m_chatDateFormat;
    m_extraSpacing = m->m_chatVertSpacing;
    m_appendLineBreak = m->m_chatAppendLineBreak;
  }
  m_showNotices = m->m_showNotices;
  m_colorRcv = m->m_colorRcv;
  m_colorSnt = m->m_colorSnt;
  m_colorRcvHistory = m->m_colorRcvHistory;
  m_colorSntHistory = m->m_colorSntHistory;
  m_colorNotice = m->m_colorNotice;
  setPaletteBackgroundColor(m->m_colorChatBkg);
  mainwin = m;

  clear();

/*
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
*/
}

CMessageViewWidget::~CMessageViewWidget()
{
  if (m_szId)
    free(m_szId);
}

void CMessageViewWidget::setOwner(const char *_szId)
{
  if (!_szId) return;

  if (m_szId)
    free(m_szId);
  m_szId = strdup(_szId);
}

void CMessageViewWidget::clear()
{
  MLView::clear();

  m_buffer = "";

  switch (m_nMsgStyle)
  {
    case 5:
      // table doesn't work when appending so must buffer when using this style
      m_buffer.append("<table border=\"0\">");
      m_useBuffer = true;
      break;
  }
  if (m_useBuffer)
    m_buffer.prepend("<html><body>");
}

void CMessageViewWidget::updateContent()
{
  if (m_useBuffer)
    setText(m_buffer);
}

void CMessageViewWidget::internalAddMsg(QString s)
{
  if (m_extraSpacing)
  {
    if (m_nMsgStyle != 5)
    {
      if (m_useBuffer)
      {
        s.prepend("<p>");
        s.append("</p>");
      }
      else
      {
        s.append("<br>");
      }
    }
    else
    {
      s.append("<tr><td colspan=\"3\"></td></tr>");
    }
  }

  if (m_useBuffer)
  {
    if (!m_extraSpacing && m_nMsgStyle != 5)
      s.append("<br>");

    m_buffer.append(s);
    if (m_appendLineBreak)
      m_buffer.append("<hr>");
  }
  else
  {
    append(s);
    if (m_appendLineBreak)
      append("<hr>");
  }
}

void CMessageViewWidget::addMsg(ICQEvent * _e)
{
  if (strcmp(_e->Id(), m_szId) == 0 && _e->PPID() == m_nPPID &&
    _e->UserEvent())
    addMsg( _e->UserEvent() );
}

void CMessageViewWidget::addMsg(direction dir, bool fromHistory, QString eventDescription, QDateTime date, 
  bool isDirect, bool isMultiRec, bool isUrgent, bool isEncrypted, 
  QString contactName, QString messageText) 
{
  QString s;
  QString color;

  if (fromHistory)
  {
    if (dir == D_RECEIVER) 
      color = m_colorRcvHistory.name();
    else 
      color = m_colorSntHistory.name();
  }
  else
  {
    if (dir == D_RECEIVER) 
      color = m_colorRcv.name();
    else 
      color = m_colorSnt.name();
  }

  // Remove trailing line breaks.
  for (int i = messageText.length(); i > 0; i--)
  {
    if (messageText.at(i - 1) != '\n' && messageText.at(i - 1) != '\r')
    {
      messageText.truncate(i);
      break;
    }
  }

  // Extract everything inside <body>...</body>
  // Leaving <html> and <body> messes with our message display
  QRegExp body("<body[^>]*>(.*)</body>");
  if (body.search(messageText) != -1)
    messageText = body.cap(1);

  // Remove all font tags
  messageText.replace(QRegExp("</?font[^>]*>"), "");

  QString my_date = date.toString( m_nDateFormat );

  switch (m_nMsgStyle) {
    case 0:
      s = QString("<font color=\"%1\"><b>%2%3 [%4%5%6%7] %8:</b></font><br>")
                  .arg(color)
                  .arg(eventDescription)
                  .arg(my_date)
                  .arg(isDirect ? 'D' : '-')
                  .arg(isMultiRec ? 'M' : '-')
                  .arg(isUrgent ? 'U' : '-')
                  .arg(isEncrypted ? 'E' : '-')
                  .arg(contactName);
      s.append(QString("<font color=\"%1\">%2</font>")
                      .arg(color)
                      .arg(messageText));
      break;
    case 1:
      s = QString("<font color=\"%1\"><b>(%2%3) [%4%5%6%7] %8: </b></font>")
                  .arg(color)
                  .arg(eventDescription)
                  .arg(my_date)
                  .arg(isDirect ? 'D' : '-')
                  .arg(isMultiRec ? 'M' : '-')
                  .arg(isUrgent ? 'U' : '-')
                  .arg(isEncrypted ? 'E' : '-')
                  .arg(contactName);
      s.append(QString("<font color=\"%1\">%2</font>")
                      .arg(color)
                      .arg(messageText));
      break;
    case 2:
      s = QString("<font color=\"%1\"><b>%2%3 - %4: </b></font>")
                  .arg(color)
                  .arg(eventDescription)
                  .arg(my_date)
                  .arg(contactName);
      s.append(QString("<font color=\"%1\">%2</font>")
                      .arg(color)
                      .arg(messageText));
      break;  
    case 3:
      s = QString("<table border=\"1\"><tr><td><b><font color=\"%1\">%2%3</font><b><td><b><font color=\"%4\">%5</font></b></font></td>")
                  .arg(color)
                  .arg(eventDescription)
                  .arg(my_date)
                  .arg(color)
                  .arg(contactName);
      s.append(QString("<td><font color=\"%1\">%2</font></td></tr></table>")
                      .arg(color)
                      .arg(messageText));
      break; 
    case 4:
      s = QString("<font color=\"%1\"><b>%2<br>%3 [%4%5%6%7]</b></font><br><br>")
                  .arg(color)
                  .arg((dir == D_RECEIVER ? tr("%1 from %2") : tr("%1 to %2")))
                  .arg(eventDescription)
                  .arg(contactName)
                  .arg(my_date)
                  .arg(isDirect ? 'D' : '-')
                  .arg(isMultiRec ? 'M' : '-')
                  .arg(isUrgent ? 'U' : '-')
                  .arg(isEncrypted ? 'E' : '-');

      // We break the paragraph here, since the history text
      // could be in a different BiDi directionality than the
      // header and timestamp text.
      s.append(QString("<font color=\"%1\">%2</font><br><br>")
                  .arg(color)
                  .arg(messageText));
      break;
    case 5:
      // Mode 5 is a table so it cannot be displayed in paragraphs
      s = QString("<tr><td><nobr><b><font color=\"%1\">%2</font><b></nobr></td>")
                  .arg(color)
                  .arg(my_date);
      s.append(QString("<td><b><font color=\"%3\">%4</font></b></font></td>")
                       .arg(color)
                       .arg(contactName));
      s.append(QString("<td><font color=\"%1\">%2</font></td></tr>")
                      .arg(color)
                      .arg(messageText));
      break;
  }

  internalAddMsg(s);
}

void CMessageViewWidget::addMsg(const CUserEvent* e, const char* _szId, unsigned long _nPPID)
{
  QDateTime date;
  date.setTime_t(e->Time());
  QString sd = date.time().toString( m_nDateFormat );
  bool bUseHTML = false;

  QString contactName;
  QTextCodec *codec = QTextCodec::codecForLocale();

  {

    ICQUser *u = _szId ? gUserManager.FetchUser(_szId, _nPPID, LOCK_R) :
                         gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
    if (u)
    {
      codec = UserCodec::codecForICQUser(u);
      if (e->Direction() == D_RECEIVER)
        contactName = QString::fromUtf8(u->GetAlias());
      for (unsigned int x = 0; x < strlen(m_szId); x++)
      {
        if (!isdigit(m_szId[x]) && m_nPPID == LICQ_PPID && e->Direction() == D_RECEIVER)
        {
          bUseHTML = true;
          break;
        }
      }
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
      gUserManager.DropOwner(o);
    }
  }

  QString messageText;
  if (e->SubCommand() == ICQ_CMDxSUB_SMS)
     messageText = QString::fromUtf8(e->Text());
  else
     messageText = codec->toUnicode(e->Text());

  addMsg(e->Direction(), false,
         (e->SubCommand() == ICQ_CMDxSUB_MSG ? QString("") : (EventDescription(e) + " ")),
         date,
         e->IsDirect(),
         e->IsMultiRec(),
         e->IsUrgent(),
         e->IsEncrypted(),
         contactName,
         MLView::toRichText(messageText, true, bUseHTML));
  GotoEnd();

  QWidget *parent = NULL;
  if (parentWidget() &&
      parentWidget()->parentWidget() &&
      parentWidget()->parentWidget()->parentWidget())
    parent = parentWidget()->parentWidget()->parentWidget();
  if (parent && parent->isActiveWindow() &&
      (!mainwin->m_bTabbedChatting || (mainwin->m_bTabbedChatting &&
       mainwin->userEventTabDlg->tabIsSelected(parent))) &&
      e->Direction() == D_RECEIVER && e->SubCommand() == ICQ_CMDxSUB_MSG)
  {
    UserSendCommon *s = static_cast<UserSendCommon*>(parent);
    QTimer::singleShot(s->clearDelay, s, SLOT(slot_ClearNewEvents()));
  }
}

void CMessageViewWidget::addNotice(QDateTime dt, QString messageText)
{
  if (!m_showNotices)
    return;

  QString color = m_colorNotice.name();
  QString s = "";
  const QString dateTime = dt.toString( m_nDateFormat );

  // Remove trailing line breaks.
  for (int i = messageText.length(); i > 0; i--)
  {
    if (messageText.at(i - 1) != '\n' && messageText.at(i - 1) != '\r')
    {
      messageText.truncate(i);
      break;
    }
  }

  switch (m_nMsgStyle)
  {
    case 1:
      s = QString("<font color=\"%1\"><b>[%2] %3</b></font>")
                  .arg(color)
                  .arg(dateTime)
                  .arg(messageText);
      break;
    case 2:
      s = QString("<font color=\"%1\"><b>[%2] %3</b></font>")
                  .arg(color)
                  .arg(dateTime)
                  .arg(messageText);
      break;  
    case 3:
      s = QString("<table border=\"1\"><tr><td><b><font color=\"%1\">%2</font><b><td><b><font color=\"%3\">%4</font></b></font></td></tr></table>")
                  .arg(color)
                  .arg(dateTime)
                  .arg(color)
                  .arg(messageText);
      break; 

    case 5:
      s = QString("<tr><td><b><font color=\"%1\">%2</font><b></td><td colspan=\"2\"><b><font color=\"%3\">%4</font></b></font></td></tr>")
                  .arg(color)
                  .arg(dateTime)
                  .arg(color)
                  .arg(messageText);
      break;

    case 0:
    default:
      s = QString("<font color=\"%1\"><b>[%2] %3</b></font><br>")
                  .arg(color)
                  .arg(dateTime)
                  .arg(messageText);
      break;    
  }

  internalAddMsg(s);
}

CLicqMessageBox::CLicqMessageBox(QWidget *parent)
  : QDialog(parent, "LicqInfo", false, Qt::WType_Dialog|Qt::WShowModal), m_nUnreadNum(0)
{
  setCaption(tr("Licq"));

  // Start with no message
  QString msg = "";
  QMessageBox::Icon type = QMessageBox::Information;

  QVBoxLayout *topLay = new QVBoxLayout(this, 11, 6);

  // Make the first horizontal layout for the icon and message
  QFrame *frmMessage = new QFrame(this);
  QHBoxLayout *lay = new QHBoxLayout(frmMessage, 5, 6);
  m_lblIcon = new QLabel(frmMessage);
  QPixmap icon = getMessageIcon(type);
  m_lblIcon->setPixmap(icon);
  m_lblMessage = new QLabel(msg, frmMessage);

  lay->addWidget(m_lblIcon, 0, Qt::AlignCenter);
  lay->addWidget(m_lblMessage);

  // Make the list box of all the pending messages, starts out hidden
  m_frmList = new QFrame(this);
  QHBoxLayout *layList = new QHBoxLayout(m_frmList);
  m_lstMsg = new QListView(m_frmList);
  m_lstMsg->addColumn(""); // Only use one column
  m_lstMsg->setFixedHeight(100); // This seems to be a good height
  QHeader *hdr = m_lstMsg->header();
  hdr->hide(); // Don't need to show the list's headers
  layList->addWidget(m_lstMsg);
  // Add this listbox as an extension to the dialog, and make it shown at the
  // bottom part of the dialog.
  setOrientation(Qt::Vertical);
  setExtension(m_frmList);

  // Make the second horizontal layout for the buttons
  QFrame *frmButtons = new QFrame(this);
  QHBoxLayout *lay2 = new QHBoxLayout(frmButtons, 0, 15);
  m_btnMore = new QPushButton(tr("&List"), frmButtons);
  //m_btnMore->setDisabled(true);
  m_btnNext = new QPushButton(tr("&Next"), frmButtons);
  m_btnNext->setDisabled(true);
  m_btnClear = new QPushButton(tr("&Ok"), frmButtons);
  m_btnClear->setDefault(true);

  lay2->addWidget(m_btnMore);
  lay2->addWidget(m_btnNext);
  lay2->addWidget(m_btnClear);

  topLay->addWidget(frmMessage, 0, Qt::AlignCenter);
  topLay->addWidget(frmButtons, 0, Qt::AlignCenter);

  m_Size = sizeHint();
  setFixedSize(m_Size);

  // Connect all the signals here
  connect(m_btnMore, SIGNAL(clicked()), this, SLOT(slot_toggleMore()));
  connect(m_btnNext, SIGNAL(clicked()), this, SLOT(slot_clickNext()));
  connect(m_btnClear, SIGNAL(clicked()), this, SLOT(slot_clickClear()));
  connect(m_lstMsg, SIGNAL(selectionChanged(QListViewItem *)),
      this, SLOT(slot_listChanged(QListViewItem *)));

  // Make the column take up the entire width
  m_lstMsg->setColumnWidth(0, m_Size.width());

  show();
}

void CLicqMessageBox::addMessage(QMessageBox::Icon type, const QString &msg)
{
  bool unread = false;

  // The icons we will show
  QPixmap pix = getMessageIcon(type);
  QImage img;

  // If we have only one message in queue, show that one, otherwise update
  // the number of pending messages.
  if (m_lstMsg->childCount() == 0)
  { 
    m_lblIcon->setPixmap(pix);
    m_lblMessage->setText(msg);
    m_btnNext->setText(tr("&Next"));
    m_btnNext->setEnabled(false);
    m_btnMore->setEnabled(false);
    m_btnNext->hide();
    m_btnMore->hide();
    m_btnClear->setText(tr("&Ok"));
    showExtension(false); // We are opening the window, so default to not showing this
  }
  else
  {
    m_nUnreadNum++;
    unread = true; // It is unread
    m_btnClear->setText(tr("&Clear All"));
    QString nextStr = QString(tr("&Next (%1)")).arg(m_nUnreadNum);
    m_btnNext->setText(nextStr);
    if (!m_btnNext->isEnabled())
    {
      m_btnNext->setEnabled(true);
      m_btnNext->show();
    }
    if (!m_btnMore->isEnabled())
    {
      m_btnMore->setEnabled(true);
      m_btnMore->show();
    }
  }

  // Add it to the list
  CLicqMessageBoxItem *pEntry = new CLicqMessageBoxItem(m_lstMsg,
      m_lstMsg->firstChild());
  // Resize the icon
  img = pix;
  QPixmap scaledPix(img.scale(16, 16));
  // Add the columns now
  pEntry->setPixmap(0, scaledPix);
  pEntry->setText(0, msg.left(std::min(50, msg.find('\n')))); // Put this in setMessage()
  // Set the special data
  pEntry->setMessage(msg);
  pEntry->setFullIcon(pix);
  pEntry->setUnread(unread);
  pEntry->setType(type);

  // Set the caption if we set the text and icon here
  if (m_nUnreadNum == 0)
    updateCaption(pEntry);
}

/// ////////////////////////////////////////////////////////
/// @brief Toggline the detailed list view
///
/// When the more button is clicked, toggle showing the list view of
/// pending messages. When we hide the list view, we need to show the
/// oldest unread message.
/// ////////////////////////////////////////////////////////
void CLicqMessageBox::slot_toggleMore()
{
  showExtension(m_frmList->isHidden());
}

/// ////////////////////////////////////////////////////////
/// @brief Show the next pending message
///
/// When the next button is clicked, we will show the next pending 
/// unread message. The text and icon will both be updated.
/// ////////////////////////////////////////////////////////
void CLicqMessageBox::slot_clickNext()
{
  // Find the next unread message
  bool bFound = false;
  QListViewItem *i = m_lstMsg->selectedItem();
  CLicqMessageBoxItem *item = 0;
  while (i != 0)
  {
    item = dynamic_cast<CLicqMessageBoxItem *>(i);
    if (item->isUnread())
    {
      bFound = true;
      break;
    }
    i = i->itemAbove();
  }

  // If no unread item was found, search from the bottom
  if (!bFound)
  {
    i = m_lstMsg->lastItem();
    while (i != 0)
    {
      item = dynamic_cast<CLicqMessageBoxItem *>(i);
      if (item->isUnread())
      {
        bFound = true;
        break;
      }
      i = i->itemAbove();
    }
  }

  // Only change the item if there something to change it to
  if (bFound)
    m_lstMsg->setCurrentItem(item);
}

/// ////////////////////////////////////////////////////////
/// @brief Clear all pending messages and close the message box
///
/// When the clear all button is clicked, we will close the dialog and
/// remove all messages from the queue.
/// ////////////////////////////////////////////////////////
void CLicqMessageBox::slot_clickClear()
{
  // Hide the window first
  hide();

  // Remove all items that have been read
  QListViewItemIterator it(m_lstMsg);
  while (it.current())
    delete it.current();
  m_nUnreadNum = 0;
}

/// ////////////////////////////////////////////////////////
/// @brief The user changed the selected list item
/// 
/// When the item changes, the message box's icon and message that
/// it shows, must be changed to match what the user selected.
///
/// @param item The new QListViewItem that has been selected
/// ////////////////////////////////////////////////////////
void CLicqMessageBox::slot_listChanged(QListViewItem *i)
{
  // Change the icon, message and caption
  CLicqMessageBoxItem *item = dynamic_cast<CLicqMessageBoxItem *>(i);
  if (item != NULL)
  {
    m_lblIcon->setPixmap(item->getFullIcon());
    m_lblMessage->setText(item->getMessage());
    updateCaption(item);

    // Mark it as read
    if (item->isUnread())
    {
      m_nUnreadNum--;
      item->setUnread(false);
    }
  }

  // Update the next button
  QString nextStr;
  if (m_nUnreadNum > 0)
    nextStr = QString(tr("&Next (%1)")).arg(m_nUnreadNum);
  else
  {
    // No more unread messages
    nextStr = QString(tr("&Next"));
    m_btnNext->setEnabled(false);
    m_nUnreadNum = 0;
  }
  m_btnNext->setText(nextStr);
}

/// ////////////////////////////////////////////////////////
/// @brief Change the caption of the message box
/// 
/// @param item The CLicqMessageBoxItem that will be shown
/// ////////////////////////////////////////////////////////
void CLicqMessageBox::updateCaption(CLicqMessageBoxItem *item)
{
  if (!item)
    return;

  QString strCaption;
  switch (item->getType())
  {
    case QMessageBox::Information:
      strCaption = tr("Licq Information");
      break;

    case QMessageBox::Warning:
      strCaption = tr("Licq Warning");
      break;

    case QMessageBox::Critical:
      strCaption = tr("Licq Critical");
      break;

    case QMessageBox::NoIcon:
    case QMessageBox::Question:
    default:
      strCaption = tr("Licq");
      break;
  }
  setCaption(strCaption);
}

QPixmap CLicqMessageBox::getMessageIcon(QMessageBox::Icon type)
{
#ifdef USE_KDE
  QString iconName;
  switch (type)
  {
    case QMessageBox::Information:
      iconName = "messagebox_info";
      break;
    case QMessageBox::Warning:
      iconName = "messagebox_warning";
      break;
    case QMessageBox::Critical:
      iconName = "messagebox_critical";
      break;
    case QMessageBox::NoIcon:
    case QMessageBox::Question:
    default:
      return QPixmap();
  }

  QPixmap icon = KApplication::kApplication()->iconLoader()->loadIcon(iconName,
    KIcon::NoGroup, KIcon::SizeMedium, KIcon::DefaultState, 0, true);
  if (icon.isNull())
    icon = QMessageBox::standardIcon(type);
#else
  QPixmap icon = QMessageBox::standardIcon(type);
#endif

  return icon;
}

CLicqMessageManager *CLicqMessageManager::m_pInstance = 0;

CLicqMessageManager::CLicqMessageManager()
  : m_pMsgDlg(0)
{
}

CLicqMessageManager::~CLicqMessageManager()
{
  if (m_pMsgDlg)
    delete m_pMsgDlg;
}

CLicqMessageManager *CLicqMessageManager::Instance()
{
  if (m_pInstance == 0)
    m_pInstance = new CLicqMessageManager;

  return m_pInstance;
}

void CLicqMessageManager::addMessage(QMessageBox::Icon type, const QString &msg,
  QWidget *parent)
{
  // We should pass parent to it, but it causes a crash after the parent closes
  // and we try to show another message box. I tried to reparent m_pMsgDlg, but
  // that didnt help much.
  // So for now.. we do this:
  parent = 0; // XXX See comment above
  if (m_pMsgDlg == 0)
    m_pMsgDlg = new CLicqMessageBox(parent); 

  m_pMsgDlg->addMessage(type, msg);
  m_pMsgDlg->show();
}

CLicqMessageBoxItem::CLicqMessageBoxItem(QListView *parent, QListViewItem *after)
  : QListViewItem(parent, after)
{
  m_unread = false;
}

void CLicqMessageBoxItem::paintCell(QPainter *p, const QColorGroup &cg,
    int column, int width, int alignment)
{
  QColorGroup _cg(cg);
  QColor c = _cg.text(); // Save it to revert it when we are done

  if (isUnread())
    _cg.setColor(QColorGroup::Text, Qt::red);

  QListViewItem::paintCell(p, _cg, column, width, alignment);
  _cg.setColor(QColorGroup::Text, c);
}

#include "ewidgets.moc"
