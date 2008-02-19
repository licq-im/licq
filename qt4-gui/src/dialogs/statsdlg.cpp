// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007 Licq developers
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

#include "statsdlg.h"

#include "config.h"

#include <QDateTime>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include <licq_icqd.h>

#include "core/messagebox.h"

#include "helpers/support.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::StatsDlg */

StatsDlg::StatsDlg(QWidget* parent)
  : QDialog(parent)
{
  Support::setWidgetProps(this, "StatisticsDialog");
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(tr("Licq - Statistics"));

  QVBoxLayout* lay = new QVBoxLayout(this);

  stats = new QLabel();

  lay->addWidget(stats);

  lay->addSpacing(20);

  QDialogButtonBox* buttons = new QDialogButtonBox(
      QDialogButtonBox::Ok);
  connect(buttons, SIGNAL(accepted()), SLOT(close()));

  QPushButton* button = buttons->addButton(QDialogButtonBox::Reset);
  connect(button, SIGNAL(clicked()), SLOT(reset()));
  button->setAutoDefault(false);

  lay->addWidget(buttons);

  prepare();

  show();
}

void StatsDlg::prepare()
{
  QDateTime utime, reset;
  utime.setTime_t(gLicqDaemon->StartTime());
  reset.setTime_t(gLicqDaemon->ResetTime());

  QString text = QString(
      "<table width=100%>"
      "<tr><th colspan=2>%1</th></tr>"
      "<tr><td>%2</td><td align=right>%3</td></tr>"
      "<tr><td>%4</td><td align=right>%5</td></tr>"
      "<tr><td>%6</td><td align=right>%7</td></tr>"
      "</table>"
      "<hr>"
      "<table width=100%>"
      "<tr><th colspan=4>%8</th></tr>"
      "<tr><td></td><td align=right><small>%9</small></td>"
      "<td align=center>/</td><td><small>%10</small></td></tr>")
    .arg(tr("Daemon Statistics"))
    .arg(tr("Up since"))
    .arg(utime.toString())
    .arg(tr("Last reset"))
    .arg(reset.toString())
    .arg(tr("Number of users"))
    .arg(gUserManager.NumUsers())
    .arg(tr("Event Statistics"))
    .arg(tr("Today"))
    .arg(tr("Total"));

  DaemonStatsList::iterator iter;

  for (iter = gLicqDaemon->AllStats().begin();
      iter != gLicqDaemon->AllStats().end();
      iter++)
  {
    text += QString(
        "<tr>"
        "<td>%1</td>"
        "<td align=right>%2</td>"
        "<td align=center>/</td>"
        "<td align=left>%3</td>"
        "</tr>")
      .arg(iter->Name())
      .arg(iter->Today())
      .arg(iter->Total());
  }

  text += "</table>";

  stats->setText(text);
}

void StatsDlg::reset()
{
  if (QueryYesNo(this, tr("Do you really want to\nreset your statistics?")))
  {
    gLicqDaemon->ResetStats();
    prepare();
  }
}
