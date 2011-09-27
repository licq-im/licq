/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2010 Licq developers
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

#include "aboutdlg.h"

#include "config.h"
#include "pluginversion.h"

#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>

#include <licq/daemon.h>

#include "core/gui-defines.h"

#include "helpers/support.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::AboutDlg */

AboutDlg::AboutDlg(QWidget* parent)
  : QDialog(parent)
{
  Support::setWidgetProps(this, "AboutDialog");
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(tr("Licq - About"));

  QVBoxLayout* lay = new QVBoxLayout(this);

  QString text = QString(
      "<table width=100%>"
      "<tr><th colspan=2>%1</th></tr>"
      "<tr><td>Licq</td><td align=right>%2%3</td></tr>"
      "<tr><td>" DISPLAY_PLUGIN_NAME "</td><td align=right>%4%5</td></tr>"
      "<tr><td>%6</td><td align=right>%7</td></tr>"
      "</table>"
      "<hr>"
      "<table width=100%>"
      "<tr><th colspan=2>%8</th></tr>"
      "<tr><td>%9</td><td align=right>Jon Keating</td></tr>"
      "<tr><td>%10</td><td align=right>Dirk A. Mueller</td></tr>"
      "<tr><td>%11</td><td align=right>Graham Roff</td></tr>"
      "</table>"
      "<hr>"
      "<table width=100%>"
      "<tr><th colspan=2>%12</th></tr>"
      "<tr><td>WWW</td><td align=right>http://www.licq.org</td></tr>"
      "<tr><td>IRC</td><td align=right>irc://irc.freenode.net/licq</td></tr>"
      "</table>"
    )
    .arg(tr("Version"))
    .arg(Licq::gDaemon.Version())
    .arg(Licq::gDaemon.haveCryptoSupport() ? "/SSL" : "")
    .arg(PLUGIN_VERSION_STRING)
#ifdef USE_KDE
    .arg("/KDE")
#else
    .arg("")
#endif
    .arg(tr("Compiled on"))
    .arg(__DATE__)
    .arg(tr("Credits"))
    .arg(tr("Maintainer"))
    .arg(tr("Contributions"))
    .arg(tr("Original author"))
    .arg(tr("Contact us"));

  QLabel* label = new QLabel(text);

  lay->addWidget(label);

  lay->addSpacing(20);

  QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok);
  buttons->setCenterButtons(true);
  connect(buttons, SIGNAL(accepted()), SLOT(close()));

  lay->addWidget(buttons);

  show();
}
