#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qimage.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <qlistbox.h>

#include "ewidgets.h"
#include "outputwin.h"

bool QueryUser(QWidget *q, QString szQuery, QString szBtn1, QString szBtn2)
{
  return ( QMessageBox::information(q, "Licq", szQuery, szBtn1, szBtn2) == 0);
}


int QueryUser(QWidget *q, QString szQuery, QString szBtn1, QString szBtn2, QString szBtn3)
{
  return ( QMessageBox::information(q, "Licq", szQuery, szBtn1, szBtn2, szBtn3));
}


void InformUser(QWidget *q, QString szInfo)
{
  (void) new CLicqMessageBox(szInfo, QMessageBox::Information, q);
}

void WarnUser(QWidget *q, QString szInfo)
{
  (void) new CLicqMessageBox(szInfo, QMessageBox::Warning, q);
}

//-----CELabel------------------------------------------------------------------
CELabel::CELabel(bool _bTransparent, QPopupMenu *m, QWidget *parent, char *name)
  : QLabel(parent, name)
{
   mnuPopUp = m;
   m_bTransparent = _bTransparent;
   if (_bTransparent) setAutoMask(true);
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
   QColorGroup normal(pal.normal());
   QColorGroup newNormal(normal.foreground(), c, normal.light(), normal.dark(),
                         normal.mid(), normal.text(), normal.base());
   pal = QPalette(newNormal, newNormal, newNormal);
#if 0
#if QT_VESION >= 210
   pal.setColor(QPalette::Active, QColorGroup::Background, c);
   pal.setColor(QPalette::Inactive, QColorGroup::Background, c);
#else
   pal.setColor(QPalette::Active, QColorGroup::Background, c);
   pal.setColor(QPalette::Normal, QColorGroup::Background, c);
#endif
#endif

   setPalette(pal);
}

void CELabel::resizeEvent (QResizeEvent *)
{
  // Resize the background pixmap properly
  if (autoMask()) updateMask();

  if (backgroundPixmap() != NULL)
  {
    QImage im = (backgroundPixmap()->convertToImage()).smoothScale(width(), height());
    QPixmap pm;
    pm.convertFromImage(im);
    setBackgroundPixmap(pm);
  }
}


void CELabel::mouseDoubleClickEvent(QMouseEvent *)
{
   emit doubleClicked();
}


void CELabel::mouseReleaseEvent(QMouseEvent *e)
{
   QLabel::mouseReleaseEvent(e);
   if (mnuPopUp == NULL) return;
   if (e->button() == RightButton)
   {
      QPoint clickPoint(e->x(), e->y());
      mnuPopUp->popup(mapToGlobal(clickPoint));
   }
}


//-----CEButton-----------------------------------------------------------------
CEButton::CEButton(QPixmap *p1, QPixmap *p2, QPixmap *p3, QWidget *parent, char *name)
   : QPushButton(parent, name)
{
   pmUpFocus = p1;
   pmUpNoFocus = p2;
   pmDown = p3;
   pmCurrent = pmUpNoFocus;
   mouseEvent = NULL;
}

CEButton::CEButton(QString label, QWidget *parent, char *name) : QPushButton(label, parent, name)
{
   pmCurrent = pmUpFocus = pmUpNoFocus = pmDown = NULL;
}

CEButton::~CEButton()
{
  if (pmUpFocus != NULL) delete pmUpFocus;
  if (pmUpNoFocus != NULL) delete pmUpNoFocus;
  if (pmDown != NULL) delete pmDown;
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
   mouseEvent = e;
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
   QColorGroup normal(pal.normal());
   QColorGroup newNormal(normal.foreground(), normal.background(), normal.light(), normal.dark(),
                         normal.mid(), QColor(theColor), normal.base());
   setPalette(QPalette(newNormal, pal.disabled(), newNormal));
}

void CEButton::setNamedBgColor(char *theColor)
{
   if (theColor == NULL) return;

   QPalette pal(palette());
   QColorGroup normal(pal.normal());
   QColorGroup newNormal(normal.foreground(), QColor(theColor), normal.light(), normal.dark(),
                         normal.mid(), normal.text(), normal.base());
   setPalette(QPalette(newNormal, pal.disabled(), newNormal));
}


//-----CEComboBox---------------------------------------------------------------
CEComboBox::CEComboBox(bool _bAppearEnabledAlways, QWidget *parent, char *name)
   : QComboBox(parent, name)
{
   m_bAppearEnabledAlways = _bAppearEnabledAlways;
   if (m_bAppearEnabledAlways)
   {
      QPalette pal(palette());
      setPalette(QPalette(pal.normal(), pal.normal(), pal.normal()));
   }
}


void CEComboBox::setNamedFgColor(char *theColor)
{
   if (theColor == NULL) return;

   QPalette pal(palette());
   QColorGroup normal(pal.normal());
   QColorGroup newNormal(normal.foreground(), normal.background(), normal.light(), normal.dark(),
                         normal.mid(), QColor(theColor), normal.base());
   setPalette(QPalette(newNormal, pal.disabled(), newNormal));
}


void CEComboBox::setNamedBgColor(char *theColor)
{
   if (theColor == NULL) return;

   QPalette pal(palette());
   QColorGroup normal(pal.normal());
   QColorGroup newNormal(normal.foreground(), normal.background(), normal.light(), normal.dark(),
                         normal.mid(), normal.text(), QColor(theColor));
   setPalette(QPalette(newNormal, pal.disabled(), newNormal));
}


//-----CInfoField::constructor--------------------------------------------------
CInfoField::CInfoField(QWidget *parent, bool readonly)
  : QLineEdit(parent)
{
  baseRO = palette().disabled().base();
  baseRW = palette().normal().base();

  // Set colors
  SetReadOnly(readonly);
}

void CInfoField::SetReadOnly(bool b)
{
  m_bReadOnly = b;
  QColorGroup cg(palette().normal().foreground(),
                 palette().normal().background(),
                 palette().normal().light(),
                 palette().normal().dark(),
                 palette().normal().mid(),
                 palette().normal().text(),
                 m_bReadOnly ? baseRO : baseRW);

  setPalette(QPalette(cg, palette().disabled(), cg));
}

void CInfoField::keyPressEvent( QKeyEvent *e )
{
  if (m_bReadOnly)
  {
    if ( e->ascii() >= 32 ||
         e->key() == Key_Delete || e->key() == Key_Backspace ||
        ( (e->state() & ControlButton) && (e->key() == Key_D ||
                                           e->key() == Key_H ||
                                           e->key() == Key_K ||
                                           e->key() == Key_V ||
                                           e->key() == Key_X)
        )
       )
    {
      e->ignore();
      return;
    }
  }
  QLineEdit::keyPressEvent(e);
}


void CInfoField::mouseReleaseEvent(QMouseEvent *e)
{
  if (m_bReadOnly && e->button() == MidButton)
    return;
  QLineEdit::mouseReleaseEvent(e);
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
  sprintf(t, "%ld", data);
  setData(t);
}


void CInfoField::setEnabled(bool _b)
{
  QLineEdit::setEnabled(_b);
}


// -----------------------------------------------------------------------------

CHistoryWidget::CHistoryWidget(QWidget* parent, const char* name)
  : MLEditWrap(true, parent, true, name)
{
  setReadOnly(true);
};


// -----------------------------------------------------------------------------

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

  MLEditWrap::paintCell(p, row, col);
}


// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------

CLogWidget::CLogWidget(QWidget* parent, const char* name)
  : MLEditWrap(false, parent, true, name)
{
  setReadOnly(true);
}


// -----------------------------------------------------------------------------

void CLogWidget::paintCell(QPainter* p, int row, int col)
{
  QPalette& pal = const_cast<QPalette&>(palette());

  pal.setColor(QColorGroup::Text, Qt::black);

  /*if (col < 9)
  {
    pal.setColor(QColorGroup::Text, Qt::darkGreen);
  }
  else*/
  {
    QString s;
    int i = row;
    while ( i >= 0 && (s = stringShown(i).mid(11, 3)) == "   ") i--;

    if (s == "WRN")
      pal.setColor(QColorGroup::Text, Qt::darkYellow);
    else if (s == "ERR")
      pal.setColor(QColorGroup::Text, Qt::darkRed);
    else if (s == "PKT")
      pal.setColor(QColorGroup::Text, Qt::darkBlue);
    else if (s == "???")
      pal.setColor(QColorGroup::Text, Qt::magenta);
  }

  MLEditWrap::paintCell(p, row, col);
}


// -----------------------------------------------------------------------------
#if 0
QFont CFontDialog::GetFontFromFullSet(bool *ok, const QFont &def,
   QWidget *p = 0, const char *n = 0)
{
// Code almost straight from Qt 2.1 beta 1 source tree
    QFont result;
    result = def;

    CFontDialog *dlg = new CFontDialog( p, n, TRUE );
    //dlg->setFont( def );
    dlg->setCaption( tr("Select Font") );
    if ( dlg->exec() == QDialog::Accepted )
    {
      result = dlg->font();
      if (ok) *ok = TRUE;
    }
    else
    {
      if (ok) *ok = FALSE;
    }
    delete dlg;
    return result;
}


void CFontDialog::updateFamilies()
{
// Code almost straight from Qt 2.1 beta 1 source tree
printf("fuck off\n");
    familyNames = fdb.families(false);
    QStringList newList;
    QString s;
    QStringList::Iterator it = familyNames.begin();
    for( ; it != familyNames.end() ; it++ ) {
        s = *it;
        if ( s.contains('-') ) {
            int i = s.find('-');
            s = s.right( s.length() - i - 1 ) + " [" + s.left( i ) + "]";
        }
        s[0] = s[0].upper();
        newList.append( s );
    }
    familyListBox()->insertStringList( newList );
}

void CFontDialog::familyHighlighted( int i )
{
  QString s = familyNames[i];
  QFontDialog::familyHighlighted( s );
}


void CFontDialog::updateScripts()
{
    scriptCombo()->clear();

    charSetNames = fdb.charSets( d->family );

    if ( charSetNames.isEmpty() ) {
        qWarning( "QFontDialog::updateFamilies: Internal error, "
                  "no character sets for family \"%s\"",
                  (const char *) d->family );
        return;
    }

    QStringList::Iterator it = charSetNames.begin();
    for ( ; it != charSetNames.end() ; ++it )
        d->scriptCombo->insertItem( fdb.verboseCharSetName(*it) );
}

#endif
#include "ewidgets.moc"
