#ifndef __KEYREQ_H__
#define __KEYREQ_H__

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

class CSignalManager;
class ICQEvent;
class CICQEventTag;

class QLabel;
class QPushButton;

class KeyRequestDlg : public QDialog
{
  Q_OBJECT
public:
  KeyRequestDlg(CSignalManager *_sigman, unsigned long nUin, QWidget *parent = 0);
  virtual ~KeyRequestDlg();

protected:
  unsigned long m_nUin;
  QPushButton *btnSend, *btnCancel;
  QLabel *lblStatus;
  bool m_bOpen;

  CSignalManager *sigman;
  CICQEventTag *icqEventTag;

protected slots:
  virtual void done(int);
  void startSend();
  void doneEvent(ICQEvent *);
  void openConnection();
  void closeConnection();
};


#endif
