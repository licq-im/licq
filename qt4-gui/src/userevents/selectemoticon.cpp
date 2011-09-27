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

#include "selectemoticon.h"

#include <math.h>

#include <QGridLayout>
#include <QMap>

#include "config/emoticons.h"

#include "helpers/support.h"

#include "emoticonlabel.h"

using namespace LicqQtGui;

SelectEmoticon::SelectEmoticon(QWidget* parent)
  : QFrame(parent, Qt::Popup)
{
  Support::setWidgetProps(this, "SelectEmoticon");
  setAttribute(Qt::WA_DeleteOnClose, true);

  setFrameShape(StyledPanel);

  QMap<QString, QString> map = Emoticons::self()->emoticonsKeys();
  QMap<QString, QString>::iterator iter;

  int nCols = static_cast<int>(sqrt(map.size()));

  grid = new QGridLayout(this);
  grid->setContentsMargins(0, 0, 0, 0);
  grid->setSpacing(0);

  int x = 0, y = 0;
  for (iter = map.begin(); iter != map.end(); ++iter)
  {
    EmoticonLabel* lbl = new EmoticonLabel(iter.key(), iter.value(), this);

    connect(lbl, SIGNAL(clicked(const QString&)), SLOT(emoticonClicked(const QString&)));
    connect(lbl, SIGNAL(move(EmoticonLabel*, int)), SLOT(moveFrom(EmoticonLabel*, int)));

    grid->addWidget(lbl, y, x++);
    grid->setAlignment(lbl, Qt::AlignCenter);

    // Set the focus to the first item so we can use the keyboard to navigate
    if (y == 0 && x == 1)
      lbl->setFocus();

    if (x == nCols)
    {
      x = 0;
      y++;
    }
  }
}

void SelectEmoticon::emoticonClicked(const QString& value)
{
  emit selected(value);
  close();
}

void SelectEmoticon::moveFrom(EmoticonLabel* item, int key)
{
  if (item == 0)
    return;

  int index = grid->indexOf(item);

  switch (key)
  {
    case Qt::Key_Up:
      index -= grid->columnCount();
      if (index < 0)
        index += grid->columnCount() * grid->rowCount();
      while (grid->itemAt(index) == 0)
        index -= grid->columnCount();
      break;

    case Qt::Key_Down:
      index += grid->columnCount();
      while (grid->itemAt(index) == 0)
        if (index >= grid->columnCount() * grid->rowCount())
          index -= grid->columnCount() * grid->rowCount();
        else
          index += grid->columnCount();
      break;

    default:
      return;
  }

  grid->itemAt(index)->widget()->setFocus();
}
