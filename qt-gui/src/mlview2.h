#ifndef MLVIEW2_H
#define MLVIEW2_H

#include <qmultilineedit.h>

class MLViewQt2 : public QMultiLineEdit
{
  Q_OBJECT
public:
  MLViewQt2 (QWidget* parent=0, const char *name=0);
  virtual ~MLViewQt2() {}

  void appendNoNewLine(const QString& s);
  void append(const QString& s);
  void GotoEnd();

  using QMultiLineEdit::hasMarkedText;
  using QMultiLineEdit::markedText;

  void setBackground(const QColor&);
  void setForeground(const QColor&);

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
