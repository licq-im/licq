#ifndef MLE_H
#define MLE_H

#include "qmultilineeditnew.h"

class MLEditWrap : public QMultiLineEditNew
{
  Q_OBJECT
public:
  MLEditWrap (bool wordWrap, QWidget *parent=0, const char *name=0);
  void appendNNL(QString s);
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
