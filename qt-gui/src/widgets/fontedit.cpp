/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2008-2009 Licq developers
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

#include "config.h"

#include "fontedit.h"

#ifndef USE_KDE
#include <QFontDialog>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QToolButton>
#endif

#include "config/general.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::FontEdit */

FontEdit::FontEdit(QWidget* parent)
  : FONTEDIT_BASE(parent)
{
#ifndef USE_KDE
  QHBoxLayout* lay = new QHBoxLayout(this);
  lay->setContentsMargins(0, 0, 0, 0);

  // Input field
  myEditor = new QLineEdit();
  lay->addWidget(myEditor);

  // Button to open file dialog
  QToolButton* browseButton = new QToolButton();
  // TODO: Use a font icon instead of text for browseButton
  browseButton->setText(tr("Choose..."));
  browseButton->setToolTip(tr("Select a font from the system list."));
  connect(browseButton, SIGNAL(clicked()), SLOT(browse()));
  lay->addWidget(browseButton);
#endif
}

void FontEdit::setFont(const QFont& font, bool /* onlyFixed */)
{
  QString s;
  if (font == Config::General::instance()->defaultFont())
    s = tr("default (%1)").arg(font.toString());
#ifndef USE_KDE
  else
    s = font.toString();

  myEditor->setFont(font);
  myEditor->setText(s);
  myEditor->setCursorPosition(0);
#else
  KFontRequester::setFont(font);
  setSampleText(s);
#endif
}

#ifndef USE_KDE
QFont FontEdit::font() const
{
  return myEditor->font();
}

void FontEdit::browse()
{
  bool fontOk;
  QFont f = QFontDialog::getFont(&fontOk, myEditor->font(), this);

  if (fontOk)
  {
    setFont(f);
    emit fontSelected(f);
  }
}
#endif
