#ifndef MLVIEW_H
#define MLVIEW_H

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

#if QT_VERSION >= 300
#include <qtextbrowser.h>

class CICQDaemon;

class MLViewQt3 : public QTextBrowser
{
  Q_OBJECT
public:
  MLViewQt3 (QWidget* parent=0, const char *name=0);
  virtual ~MLViewQt3() {}

  void appendNoNewLine(const QString& s);
  void append(const QString& s);
  void GotoEnd();

  bool hasMarkedText();
  QString markedText();

  void setBackground(const QColor&);
  void setForeground(const QColor&);
  void setHandleLinks(bool enable);
  void setICQDaemon(CICQDaemon* licqDaemon);
  
  static QString toRichText(const QString& s, bool highlightURLs = false);
public slots:
  virtual void setSource(const QString& name);
private:
  bool m_handleLinks;
  // This is required for non-KDE version to be able to launch URLs.
  CICQDaemon *m_licqDaemon;
};
#endif

#if QT_VERSION >= 300
typedef MLViewQt3 MLView;
#else
typedef MLViewQt2 MLView;
#endif

#endif
