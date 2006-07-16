#ifndef EWIDGETS_H
#define EWIDGETS_H

#include <qlabel.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qtabbar.h>
#include <qtabwidget.h>
#include <qmap.h>

#include "mlview.h"
#include "licq_message.h"

#include <list>

using std::list;

class CUserEvent;
class ICQEvent;
class CMainWindow;
class ICQUser;

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
  void addPixmap(const QPixmap&);
  void clearPixmaps();
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
  list<QPixmap> m_lPixmaps;
  int addIndent,
      startingIndent;
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

class CETabBar : public QTabBar
{
  Q_OBJECT
public:
  CETabBar(QWidget *p, const char *n)
    : QTabBar(p, n) { }
    
  const QColor &tabColor(int) const;
  void setTabColor(int, const QColor &);
  virtual void removeTab(QTab *);

  void setPreviousTab();
  void setNextTab();
  
protected:
  virtual void paintLabel(QPainter *, const QRect &, QTab *, bool) const;
  virtual void wheelEvent(QWheelEvent *e);
  
private:
  QMap<int, QColor> mTabColors;
};

class CETabWidget : public QTabWidget
{
  Q_OBJECT
public:
  CETabWidget(QWidget * = 0, const char * = 0, WFlags = 0);
  void setTabColor(QWidget *, const QColor &);

  void setPreviousPage();
  void setNextPage();

protected:
  virtual void wheelEvent(QWheelEvent *e);
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
  virtual void keyPressEvent(QKeyEvent *e);
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

  void setOwner(const char *szId);
  void addMsg(direction dir, bool fromHistory, QString eventDescription, QDateTime date, 
    bool isDirect, bool isMultiRec, bool isUrgent, bool isEncrypted, 
    QString contactName, QString messageText);
  void addNotice(QString dateTime, QString messageText);

  unsigned short m_nMsgStyle;
  bool m_bAppendLineBreak;
  QColor m_colorRcvHistory;
  QColor m_colorSntHistory;
  QColor m_colorRcv;
  QColor m_colorSnt;
  
public slots:
  virtual void addMsg(CUserEvent *, const char * = 0, unsigned long = 0);
  void addMsg(ICQEvent *);
};


#endif
