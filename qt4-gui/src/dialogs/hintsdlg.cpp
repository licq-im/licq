// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2006 Licq developers
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

#include "hintsdlg.h"

#include <QVBoxLayout>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QPushButton>

#include "helpers/support.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::HintsDlg */

HintsDlg::HintsDlg(QString& text, QWidget* parent)
  : QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
  Support::setWidgetProps(this, "HintsDlg");
  setAttribute(Qt::WA_DeleteOnClose, true);

  setWindowTitle(tr("Licq - Hints"));

  QVBoxLayout* lay = new QVBoxLayout(this);

  viewPane = new QTextEdit();
  viewPane->setReadOnly(true);
  viewPane->setMinimumSize(400, 450);
  viewPane->setText(text);
  lay->addWidget(viewPane);

  buttons = new QDialogButtonBox();
  closeButton = buttons->addButton(QDialogButtonBox::Close);
  connect(closeButton, SIGNAL(clicked()), SLOT(close()));
  lay->addWidget(buttons);

  show();
}
