#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mledit.h"
#include <iostream.h>


MLEditWrap::MLEditWrap (bool wordWrap, QWidget *parent, const char *name)
  : QMultiLineEditNew(parent, name)
{
  if (wordWrap)
  {
    setWordWrap(WidgetWidth);
    setWrapPolicy(AtWhiteSpace);
  }
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

#include "mledit.moc"
