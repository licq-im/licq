#ifndef SEARCHUSER_H
#define SEARCHUSER_H

// Search user code base written by Alan Penner (apenner@andrew.cmu.edu)
// modified by Graham Roff

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qdialog.h>
#include <qpushbt.h>
#include <qlabel.h>
#include <qlined.h>
#include <qchkbox.h>
#include <qlistview.h>

#include "icq.h"
#include "user.h"
#include "sigman.h"


class SearchUserView : public QListView
{
   Q_OBJECT
public:
   SearchUserView(QWidget *parent = NULL, char *name = NULL);
   unsigned long currentUin(void);
};


class SearchItem : public QListViewItem
{
public:
   SearchItem(struct UserBasicInfo *, QListView *parent);
   unsigned long uin();
protected:
   unsigned long uinVal;
};



class SearchUserDlg : public QDialog
{
   Q_OBJECT
public:
   SearchUserDlg (CICQDaemon *s, CSignalManager *theSigMan,
                  QWidget *parent = NULL, const char *name = NULL);

protected:
   CICQDaemon *server;
   CSignalManager *sigman;
   QPushButton *btnSearch, *btnCancel;
   QLabel *lblEmail, *lblFirst, *lblLast, *lblNick, *lblUin;
   QLineEdit *edtEmail, *edtFirst, *edtLast, *edtNick, *edtUin;

   unsigned short searchSequence;
   QPushButton *btnDone, *btnAdd, *btnSearchAgain;
   SearchUserView *foundView;
   QLabel *lblSearch;

   void searchFound(struct UserBasicInfo *);
   void searchDone(char);
   void searchFailed(void);

public slots:
   virtual void show();
   virtual void hide();
   void startSearch();
   void addUser();
   void resetSearch();
   void slot_searchResult(ICQEvent *);
};


#endif
