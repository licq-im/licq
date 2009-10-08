/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2009 Licq developers
 *
 * Licq is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Licq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Licq; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef EWIDGETS_H
#define EWIDGETS_H

#include <list>

#include <qlabel.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qtabbar.h>
#include <qtabwidget.h>
#include <qmap.h>
#include <qmessagebox.h>
#include <qsize.h>
#include <qlistview.h>
#include <qstringlist.h>
#include <qspinbox.h>

#include <licq_message.h>
#include <licq_types.h>

#include "mlview.h"

class CUserEvent;
class LicqEvent;
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
  std::list<QPixmap> m_lPixmaps;
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

signals:
  void middleClick(int t);

protected:
  virtual void paintLabel(QPainter *, const QRect &, QTab *, bool) const;
  virtual void wheelEvent(QWheelEvent *e);
  virtual void mousePressEvent(QMouseEvent* e);
  virtual void mouseReleaseEvent(QMouseEvent* e);

private:
  QMap<int, QColor> mTabColors;
  int clickedTab;
};

class CETabWidget : public QTabWidget
{
  Q_OBJECT
public:
  CETabWidget(QWidget * = 0, const char * = 0, WFlags = 0);
  void setTabColor(QWidget *, const QColor &);

  void setPreviousPage();
  void setNextPage();

signals:
  void middleClick(QWidget *p);

protected:
  virtual void wheelEvent(QWheelEvent *e);

protected slots:
  void slot_middleClick(int t);
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
  Q_OBJECT
public:
  CInfoField(QWidget *parent, bool readonly);
  void setData(const char *data);
  void setData(QString data);
  void setData(unsigned long data);
  void setDateTime(uint timestamp);
  void SetReadOnly(bool);
protected:
  virtual void keyPressEvent(QKeyEvent *e);
  QColor baseRO, baseRW;
};

/* ----------------------------------------------------------------------------- */

/**
 * Input field for timezones.
 */
class CTimeZoneField : public QSpinBox
{
  Q_OBJECT
public:
  CTimeZoneField(QWidget *parent);
  void setData(char data);
  char data();
protected:
  QString mapValueToText(int v);
  int mapTextToValue(bool* ok);

  static const int undefinedValue = -24;
};

/* ----------------------------------------------------------------------------- */

class CMessageViewWidget : public MLView
{
  Q_OBJECT
private:
  CMainWindow *mainwin;
public:
  static QStringList getStyleNames(bool includeHistoryStyles = false);

  CMessageViewWidget(const UserId& userId,
    CMainWindow *m, QWidget *parent = 0, const char *name = 0, bool historyMode = false);
  virtual ~CMessageViewWidget();

  void setOwner(const UserId& userId);
  void updateContent();
  void clear();
  void addMsg(direction dir, bool fromHistory, QString eventDescription, QDateTime date, 
    bool isDirect, bool isMultiRec, bool isUrgent, bool isEncrypted, 
    QString contactName, QString messageText);
  void addNotice(QDateTime dateTime, QString messageText);

  unsigned short m_nMsgStyle;
  QString m_nDateFormat;
  bool m_extraSpacing;
  bool m_appendLineBreak;
  bool m_useBuffer;
  bool m_showNotices;
  QColor m_colorRcvHistory;
  QColor m_colorSntHistory;
  QColor m_colorRcv;
  QColor m_colorSnt;
  QColor m_colorNotice;
  
public slots:
  virtual void addMsg(const CUserEvent* e, const UserId& userId = USERID_NONE);
  void addMsg(LicqEvent*);

private:
  void internalAddMsg(QString s);
  QString m_buffer;
  UserId myUserId;
};

/* ----------------------------------------------------------------------------- */

class CLicqMessageBoxItem : public QListViewItem
{
public:
  CLicqMessageBoxItem(QListView *, QListViewItem *);
  void paintCell(QPainter *, const QColorGroup &, int, int, int);

  void setMessage(const QString &s) { m_msg = s; }
  void setFullIcon(const QPixmap &p) { m_fullIcon = p; }
  void setUnread(bool b) { m_unread = b; }
  void setType(QMessageBox::Icon t) { m_type = t; }

  QString getMessage() const { return m_msg; }
  QPixmap getFullIcon() const { return m_fullIcon; }
  bool isUnread() const { return m_unread; }
  QMessageBox::Icon getType() const { return m_type; }

private:
  QString m_msg;
  QPixmap m_fullIcon;
  bool m_unread;
  QMessageBox::Icon m_type;
};

class CLicqMessageBox : public QDialog
{
  Q_OBJECT
public:
  CLicqMessageBox(QWidget *parent = 0);
  void addMessage(QMessageBox::Icon type, const QString &msg);

public slots:
  void slot_toggleMore();
  void slot_clickNext();
  void slot_clickClear();
  void slot_listChanged(QListViewItem *);

private:
  QPixmap getMessageIcon(QMessageBox::Icon);
  void updateCaption(CLicqMessageBoxItem *);

  int m_nUnreadNum;
  QLabel *m_lblIcon,
         *m_lblMessage;
  QPushButton *m_btnNext,
              *m_btnMore,
              *m_btnClear;
  QListView *m_lstMsg;
  QFrame *m_frmList;
  QSize m_Size;
};

class CLicqMessageManager
{
public:
  ~CLicqMessageManager();

  static CLicqMessageManager *Instance();

  void addMessage(QMessageBox::Icon type, const QString &msg, QWidget *p);

private:
  CLicqMessageManager();
  CLicqMessageManager(const CLicqMessageManager &);
  CLicqMessageManager &operator=(const CLicqMessageManager &);

  static CLicqMessageManager *m_pInstance;
  CLicqMessageBox *m_pMsgDlg;
};

#endif
