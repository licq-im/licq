#ifndef MLVIEW3_H
#define MLVIEW3_H

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
