#ifndef MLVIEW3_H
#define MLVIEW3_H

#include <qtextbrowser.h>

class CICQDaemon;

class MLView : public QTextBrowser
{
  Q_OBJECT
public:
  MLView (QWidget* parent=0, const char *name=0);
  virtual ~MLView() {}

  void appendNoNewLine(const QString& s);
  void append(const QString& s);
  void GotoEnd();

  bool hasMarkedText() const;
  QString markedText() const;

  void setBackground(const QColor&);
  void setForeground(const QColor&);
  void setHandleLinks(bool enable);

  static QString toRichText(const QString& s, bool highlightURLs = false, bool useHTML = false);
public slots:
  virtual void setSource(const QString& name);
private:
  bool m_handleLinks;
signals:
  void viewurl(QWidget*, QString);
};

#endif
