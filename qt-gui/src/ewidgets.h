#ifndef EWIDGETS_H
#define EWIDGETS_H

#include <qlabel.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qvalidator.h>

#include "mledit.h"

bool QueryUser(QWidget *, QString, QString, QString);
int QueryUser(QWidget *, QString, QString, QString, QString);
void InformUser(QWidget *q, QString);
void WarnUser(QWidget *q, QString szInfo);
void CriticalUser(QWidget *q, QString szInfo);


class CELabel : public QLabel
{
  Q_OBJECT
public:
  CELabel(bool _bTransparent, QPopupMenu *m = NULL, QWidget *parent = 0, char *name = 0);
  void setBold(bool);
  void setItalic(bool);
  void setNamedFgColor(char *);
  void setNamedBgColor(char *);
public slots:
  void polish();
protected:
  void resizeEvent (QResizeEvent *);
  virtual void mouseDoubleClickEvent(QMouseEvent *);
  virtual void mouseReleaseEvent(QMouseEvent *e);
  bool m_bTransparent;
  QPopupMenu *mnuPopUp;
signals:
  void doubleClicked();
};


class CEButton : public QPushButton
{
  Q_OBJECT
public:
  CEButton(QPixmap *, QPixmap *, QPixmap *, QWidget *parent = 0, char *name = 0);
  CEButton(QString, QWidget *parent = 0, char *name = 0);
  void setNamedFgColor(char *);
  void setNamedBgColor(char *);
  ButtonState stateWhenPressed() { return whenPressed; }
  ~CEButton();
public slots:
  void polish();
protected:
  QPixmap *pmUpFocus, *pmUpNoFocus, *pmDown,
          *pmCurrent;

  ButtonState whenPressed;
  // overloaded drawButton to remove typical button 3D effect
  virtual void drawButton(QPainter *);
  virtual void mouseReleaseEvent(QMouseEvent *e);
  virtual void mousePressEvent(QMouseEvent *e);
  virtual void enterEvent (QEvent *);
  virtual void leaveEvent (QEvent *);
};


/* ----------------------------------------------------------------------------- */

class CEComboBox : public QComboBox
{
//   Q_OBJECT
public:
   CEComboBox (bool _bAppearEnabledAlways, QWidget *parent = 0, char *name = 0);
   void setNamedFgColor(char *);
   void setNamedBgColor(char *);
protected:
   bool m_bAppearEnabledAlways;
};


/* ----------------------------------------------------------------------------- */

class CInfoField : public QLineEdit
{
  Q_OBJECT
public:
  CInfoField(QWidget *parent, bool readonly);
  void setData(const char *data);
  void setData(QString data);
  void setData(const unsigned long data);
  void SetReadOnly(bool);
  bool ReadOnly() { return m_bReadOnly; }
public slots:
  void setEnabled(bool);
protected:
  QColor baseRO, baseRW;
  bool m_bReadOnly;
  void keyPressEvent( QKeyEvent *e );
  void mouseReleaseEvent(QMouseEvent *e);
};


/* ----------------------------------------------------------------------------- */

class CHistoryWidget : public MLEditWrap
{
public:
  CHistoryWidget(QWidget* parent = 0, const char* name = 0);
  virtual ~CHistoryWidget() {};

protected:
  virtual void paintCell(QPainter* p, int row, int col);
};


/* ----------------------------------------------------------------------------- */

class CLogWidget : public MLEditWrap
{
public:
  CLogWidget(QWidget* parent = 0, const char* name = 0);
  virtual ~CLogWidget() {};

protected:
  virtual void paintCell(QPainter* p, int row, int col);
};


/* ----------------------------------------------------------------------------- */
// If Troll wasn't so fucking lame we could implement a proper font dialog
// but all the necessary data structures are private
#if 0
// MOC_SKIP_BEGIN
class CFontDialog : public QFontDialog
{
Q_OBJECT
public:
  CFontDialog(QWidget *p, const char *n, bool b) : QFontDialog(p, n, b)
  { updateFamilies(); }

  static QFont GetFontFromFullSet(bool *ok, const QFont &def, QWidget *p = 0, const char *n = 0);

protected:
  QFontDatabase fdb;
  QStringList familyNames;
  virtual void updateFamilies();
  virtual void updateScripts();

protected slots:
  virtual void familyHighlighted(int);
};
// MOC_SKIP_END
#endif
/* ----------------------------------------------------------------------------- */

#endif
