#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qfont.h>
#include <qpainter.h>
#include "mledit.h"


QFont *MLEditWrap::editFont = NULL;

MLEditWrap::MLEditWrap (bool wordWrap, QWidget* parent, bool doQuotes, const char *name)
  : QMultiLineEdit(parent, name)
{
  m_bDoQuotes = doQuotes;

#if QT_VERSION >= 210
  if (wordWrap)
  {
    setWordWrap(WidgetWidth);
    setWrapPolicy(AtWhiteSpace);
  }
  else
  {
    setWordWrap(NoWrap);
  }
#else
  if (wordWrap)
    fprintf(stderr, "Word wrap not supported in Qt < 2.10\n");
#endif

  if (editFont) QWidget::setFont(*editFont, true);
}


void MLEditWrap::appendNoNewLine(QString s)
{
  if (!atEnd()) GotoEnd();
  QMultiLineEdit::insert(s);
}


void MLEditWrap::GotoEnd(void)
{
   setCursorPosition(numLines() - 1, lineLength(numLines() - 1) - 1);
}

void MLEditWrap::keyPressEvent (QKeyEvent *e)
{
   if ((e->state() & ControlButton) && (e->key() == Key_Return || e->key() == Key_Enter))
   {
     emit signal_CtrlEnterPressed();
     return;
   }
   QMultiLineEdit::keyPressEvent(e);
}


void MLEditWrap::clear()
{
  setText("");
}


// -----------------------------------------------------------------------------

void MLEditWrap::paintCell(QPainter* p, int row, int col)
{

#if QT_VERSION >= 210
  if (m_bDoQuotes)
  {
    QString s = stringShown(row);
    int i = (s[0] == ' ');
    bool italic = (s[i] == '>' && (s[i+1] == ' ' || s[i+1] == '>'));

    if (italic ^ p->font().italic())
    {
      QFont f(p->font());
      f.setItalic(italic);
      p->setFont(f);
    }
  }
#endif

  QMultiLineEdit::paintCell(p, row, col);
}

#include "mledit.moc"
