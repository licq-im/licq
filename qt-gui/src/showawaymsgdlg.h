#ifndef SHOWAWAYMSGDLG_H
#define SHOWAWAYMSGDLG_H

/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include <qdialog.h>

//#include "user.h"

class MLEditWrap;
class CSignalManager;
class CICQDaemon;
class ICQEvent;

class QPushButton;
class QCheckBox;


class ShowAwayMsgDlg : public QDialog
{
  Q_OBJECT
public:
  ShowAwayMsgDlg(CICQDaemon *_server, CSignalManager* _sigman,
                 unsigned long _nUin, QWidget *parent = 0, const char *name = 0);
  virtual ~ShowAwayMsgDlg();

protected:
  unsigned long m_nUin;
  MLEditWrap *mleAwayMsg;
  QCheckBox *chkShowAgain;
  QPushButton *btnOk;

  CSignalManager *sigman;
  CICQDaemon *server;
  ICQEvent *icqEvent;

protected slots:
  virtual void accept();
  void doneEvent(ICQEvent *);
};


#endif
