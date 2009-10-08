// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2009 Licq developers
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

#ifndef SELECTEMOTICON_H
#define SELECTEMOTICON_H

#include <QFrame>

class QGridLayout;


namespace LicqQtGui
{
class EmoticonLabel;

class SelectEmoticon : public QFrame
{
  Q_OBJECT
public:
  SelectEmoticon(QWidget* parent);

signals:
  void selected(const QString& value);

private:
  QGridLayout* grid;

private slots:
  void emoticonClicked(const QString& value);
  void moveFrom(EmoticonLabel* item, int key);
};

} // namespace LicqQtGui

#endif
