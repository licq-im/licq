#ifndef MLEDIT_H
#define MLEDIT_H

#include "qmultilineeditnew.h"

class MLEditWrap : public QMultiLineEditNew
{
  Q_OBJECT
public:
  MLEditWrap (bool wordWrap, QWidget* parent=0, bool handlequotes = false, const char *name=0);
  void appendNNL(QString s);
  void appendChar(char);
  void goToEnd(void);
  virtual void backspace(void) { QMultiLineEditNew::backspace(); }

  static QFont* editFont;
protected:
  bool m_doQuotes;
  void keyPressEvent (QKeyEvent *);
  void paintCell(QPainter* p, int row, int col);

signals:
  void keyPressed(QKeyEvent *);
  void signal_CtrlEnterPressed();
};

#endif
