#ifndef MLEDIT_H
#define MLEDIT_H

#include <qmultilineedit.h>

class MLEditWrap : public QMultiLineEdit
{
  Q_OBJECT
public:
  MLEditWrap (bool wordWrap, QWidget* parent=0, bool handlequotes = false, const char *name=0);
  virtual ~MLEditWrap() {}

  void appendNoNewLine(QString s);
  void GotoEnd();

  // Why the fuck does Qt make these protected?
  bool hasMarkedText() const { return QMultiLineEdit::hasMarkedText(); }
  QString markedText() const { return QMultiLineEdit::markedText(); }

  // Why does Qt not provide this?
  void setBackground(const QColor&);
  void setForeground(const QColor&);

  static QFont *editFont;

protected:
  bool m_bDoQuotes;
  virtual void paintCell(QPainter *p, int row, int col);
  virtual void setCellWidth ( int );

signals:
  void signal_CtrlEnterPressed();
};

#endif
