#ifndef MLEDIT_H
#define MLEDIT_H

#include <qmultilineedit.h>

class MLEditWrap : public QMultiLineEdit
{
  Q_OBJECT
public:
  MLEditWrap (bool wordWrap, QWidget* parent=0, bool handlequotes = false, const char *name=0);
  virtual ~MLEditWrap() {}

  void appendNoNewLine(const QString& s);
  void append(const QString& s);
  void GotoEnd();

#if QT_VERSION >= 300
  using QMultiLineEdit::hasMarkedText;
  using QMultiLineEdit::markedText;
#endif

  void setBackground(const QColor&);
  void setForeground(const QColor&);

  static QFont *editFont;

protected:
  bool m_bDoQuotes;
  virtual void paintCell(QPainter *p, int row, int col);
  virtual void keyPressEvent( QKeyEvent * );
  virtual void setCellWidth ( int );
  bool focusNextPrevChild( bool n);

#if QT_VERSION < 300
  using QMultiLineEdit::hasMarkedText;
  using QMultiLineEdit::markedText;
#endif

signals:
  void signal_CtrlEnterPressed();
};

#endif
