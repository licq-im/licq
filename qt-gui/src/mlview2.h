#ifndef MLVIEW2_H
#define MLVIEW2_H

#include <qmultilineedit.h>

class MLView : public QMultiLineEdit
{
  Q_OBJECT
public:
  MLView (QWidget* parent=0, const char *name=0);
  virtual ~MLView() {}

  void appendNoNewLine(const QString& s);
  void append(const QString& s);
  void GotoEnd();

  void setBackground(const QColor&);
  void setForeground(const QColor&);

  using QMultiLineEdit::hasMarkedText;
  using QMultiLineEdit::markedText;

  // Determines whether "quoted" (begining with ">") lines should be
  // specially formatted.
  void setFormatQuoted(bool enable);

protected:
  virtual void setCellWidth ( int );
  virtual void paintCell(QPainter *p, int row, int col);

private:
  bool m_bFormatQuoted;
};

#endif
