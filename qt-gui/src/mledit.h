#ifndef MLE_H
#define MLE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qstring.h>
#include <stdlib.h>

#include "qmultilineeditnew.h"

class MLEditWrap : public QMultiLineEditNew
{
  Q_OBJECT
public:
  MLEditWrap (bool wordWrap, QWidget *parent=NULL, const char *name=NULL);
  void appendNNL(const char *);
  void appendChar(char);
  void goToEnd(void);
  virtual void backspace(void) { QMultiLineEditNew::backspace(); }

protected:
  void keyPressEvent (QKeyEvent *);

signals:
  void keyPressed(QKeyEvent *);
  void signal_CtrlEnterPressed();
};

#endif
