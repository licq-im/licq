#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qfont.h>
#include <qpainter.h>
#include "mledit.h"

QFont* MLEditWrap::editFont = 0;

MLEditWrap::MLEditWrap (bool wordWrap, QWidget* parent, bool doQuotes, const char *name)
  : QMultiLineEditNew(parent, name)
{
  m_doQuotes = doQuotes;
  if (wordWrap)
  {
    setWordWrap(WidgetWidth);
    setWrapPolicy(AtWhiteSpace);
  }
  else
  {
    setWordWrap(NoWrap);
  }

  if(editFont)  QWidget::setFont(*editFont, true);
}


void MLEditWrap::appendNNL(QString s)
{
  if (!atEnd()) goToEnd();
  QMultiLineEditNew::insert(s);
}


void MLEditWrap::goToEnd(void)
{
   setCursorPosition(numLines() - 1, lineLength(numLines() - 1) - 1);
}


void MLEditWrap::appendChar(char c)
{
  if (!atEnd()) goToEnd();
  QMultiLineEditNew::insertChar(c);
}


void MLEditWrap::keyPressEvent (QKeyEvent *e)
{
   emit keyPressed(e);
   if ((e->state() & ControlButton) && (e->key() == Key_Return || e->key() == Key_Enter))
   {
     emit signal_CtrlEnterPressed();
     return;
   }

   QMultiLineEditNew::keyPressEvent(e);
}


// -----------------------------------------------------------------------------

void MLEditWrap::paintCell(QPainter* p, int row, int col)
{

#if QT_VERSION < 210
  if(m_doQuotes) {
    QString s = stringShown(row);
    int i = (s[0] == ' ');
    bool italic = (s[i] == '>' && (s[i+1] == ' ' || s[i+1] == '>'));

    if(italic ^ p->font().italic()) {
      QFont f(p->font());
      f.setItalic(italic);
      p->setFont(f);
    }
  }
#endif

  QMultiLineEditNew::paintCell(p, row, col);
}

#include "mledit.moc"
