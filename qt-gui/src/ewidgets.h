#ifndef EWIDGETS_H
#define EWIDGETS_H

#include <qlabel.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qvalidator.h>

#include "mledit.h"
#include "mlview.h"

class CUserEvent;
class ICQEvent;
class CMainWindow;

bool QueryUser(QWidget *, QString, QString, QString, bool bConfirmYes=false, QString szConfirm=NULL, bool bConfirmNo=false, QString szConfirmNo=NULL);
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
  void setPrependPixmap(const QPixmap&);
  void clearPrependPixmap();
public slots:
  void polish();
protected:
  void resizeEvent (QResizeEvent *);
  virtual void mouseDoubleClickEvent(QMouseEvent *);
  virtual void mousePressEvent(QMouseEvent *);
  virtual void drawContents(QPainter*);
  bool m_bTransparent;
  QPopupMenu *mnuPopUp;
  QPixmap addPix;
  int addIndent;
signals:
  void doubleClicked();
};


class CEButton : public QPushButton
{
  Q_OBJECT
public:
  CEButton(QPixmap *, QPixmap *, QPixmap *, QWidget *parent = 0, char *name = 0);
  CEButton(QString, QWidget *parent = 0, char *name = 0);
  CEButton(QWidget *parent = 0, char *name = 0);
  void setNamedFgColor(char *);
  void setNamedBgColor(char *);
  ButtonState stateWhenPressed()
  {
    ButtonState b = whenPressed;
    whenPressed = NoButton;
    return b;
  }
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
public:
  CInfoField(QWidget *parent, bool readonly);
  void setData(const char *data);
  void setData(QString data);
  void setData(const unsigned long data);
  void SetReadOnly(bool);
protected:
  QColor baseRO, baseRW;
};


/* ----------------------------------------------------------------------------- */

class CHistoryWidget : public MLView
{
  Q_OBJECT
public:
  CHistoryWidget(QWidget* parent = 0, const char* name = 0);
  virtual ~CHistoryWidget() {};
      
protected:
#if QT_VERSION < 300
  virtual void paintCell(QPainter* p, int row, int col);
#endif
};

class CMessageViewWidget : public CHistoryWidget
{
  Q_OBJECT
private:
  char *m_szId;
  unsigned long m_nPPID;
  unsigned long m_nUin;
  CMainWindow *mainwin;
public:
  CMessageViewWidget(const char *szId, unsigned long nPPID,
    CMainWindow *m, QWidget *parent = 0, const char *name = 0);
  CMessageViewWidget(unsigned long _nUin, CMainWindow *m,
		     QWidget* parent=0, const char * name =0);
  virtual ~CMessageViewWidget();
public slots:
  virtual void addMsg(CUserEvent *);
  void addMsg(ICQEvent *);
};


#endif
