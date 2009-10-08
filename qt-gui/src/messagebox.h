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

#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <qlistview.h>
#include <qtooltip.h>

#include "licq_constants.h"

class CUserEvent;
class MsgView;

//=====UserViewItem================================================================================
class MsgViewItem : public QListViewItem
{
public:
  MsgViewItem(const CUserEvent* theMsg, QTextCodec *codec, QListView *);
  ~MsgViewItem(void);
  void MarkRead();

  CUserEvent *msg;
  QTextCodec *m_codec;
  int m_nEventId;

protected:
  virtual void paintCell(QPainter *, const QColorGroup &, int column, int width, int align);
  virtual void paintFocus(QPainter *, const QColorGroup &, const QRect &) {}

  void SetEventLine();

friend class MsgView;
};


//=====UserList====================================================================================
class MsgView : public QListView, public QToolTip
{
  Q_OBJECT
public:
  MsgView (QWidget *parent = 0);
  CUserEvent *currentMsg(void);
  QSize sizeHint() const;

signals:
  void sizeChange(int, int, int);

protected:

  virtual void resizeEvent(QResizeEvent *e);
  virtual void maybeTip(const QPoint&);
  void SetEventLines();
};


#endif
