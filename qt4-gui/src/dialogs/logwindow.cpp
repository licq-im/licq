// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2010 Licq developers
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

#include "logwindow.h"

#include "config.h"

#include <sstream>
#include <unistd.h>

#include <QDateTime>
#include <QDialogButtonBox>
#include <QFile>
#include <QPushButton>
#include <QShowEvent>
#include <QSocketNotifier>
#include <QTextStream>
#include <QVBoxLayout>

#ifdef USE_KDE
#include <KDE/KFileDialog>
#else
#include <QFileDialog>
#endif

#include <licq/daemon.h>
#include <licq/logging/logservice.h>
#include <licq/logging/logutils.h>

#include "core/messagebox.h"

#include "helpers/support.h"

#include "widgets/mledit.h"

#undef connect

using Licq::PluginLogSink;
using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::LogWindow */

LogWindow::LogWindow(QWidget* parent)
  : QDialog(parent)
{
  Support::setWidgetProps(this, "NetworkLog");
  setWindowTitle(tr("Licq - Network Log"));

  QVBoxLayout* top_lay = new QVBoxLayout(this);

  outputBox = new MLEdit(false, this, true);
  outputBox->setReadOnly(true);
  outputBox->setMinimumHeight(outputBox->frameWidth() * 2
      + 16 * outputBox->fontMetrics().lineSpacing());
  outputBox->setMinimumWidth(outputBox->minimumHeight() * 2);

  QTextDocument* doc = outputBox->document();
  // hardcoded limit, maybe should be user configurable?
  doc->setMaximumBlockCount(500);
  outputBox->setDocument(doc);

  top_lay->addWidget(outputBox);

  QDialogButtonBox* buttons = new QDialogButtonBox(
      QDialogButtonBox::Save |
      QDialogButtonBox::Close);

  connect(buttons, SIGNAL(accepted()), SLOT(save()));
  connect(buttons, SIGNAL(rejected()), SLOT(hide()));

  buttons->button(QDialogButtonBox::Close)->setDefault(true);

  QPushButton* btnClear = buttons->addButton(tr("Clear"),
      QDialogButtonBox::ActionRole);
  btnClear->setAutoDefault(false);
  connect(btnClear, SIGNAL(clicked()), outputBox, SLOT(clear()));

  top_lay->addWidget(buttons);

  adjustSize();

  myLogSink.reset(new PluginLogSink());
  Licq::gDaemon.getLogService().registerLogSink(myLogSink);

  sn = new QSocketNotifier(myLogSink->getReadPipe(), QSocketNotifier::Read, this);
  connect(sn, SIGNAL(activated(int)), SLOT(log(int)));
}

LogWindow::~LogWindow()
{
  Licq::gDaemon.getLogService().unregisterLogSink(myLogSink);
}

void LogWindow::log(int /*fd*/)
{
  using Licq::LogSink;
  using namespace Licq::LogUtils;

  LogSink::Message::Ptr message = myLogSink->popMessage();

  QDateTime dt;
  dt.setTime_t(message->time.sec);
  dt.setTime(dt.time().addMSecs(message->time.msec));

  QString str;
  str += dt.time().toString("hh:mm:ss.zzz");
  str += " [";
  str += QString::fromUtf8(levelToShortString(message->level));
  str += "] ";
  str += QString::fromUtf8(message->sender.c_str());
  str += ": ";
  str += QString::fromUtf8(message->text.c_str());

  if (!str.endsWith('\n'))
    str += '\n';

  if (myLogSink->isLoggingPackets() && !message->packet.empty())
  {
    str += QString::fromUtf8(packetToString(message).c_str()) + '\n';
  }

  outputBox->appendNoNewLine(str);
  outputBox->GotoEnd();

  if (message->level == Licq::Log::Error)
    CriticalUser(NULL, str);
}

void LogWindow::save()
{
  QString fn;

#ifdef USE_KDE
  KUrl u = KFileDialog::getSaveUrl(QString(QDir::homePath() + "/licq.log"),
      QString::null, this);
  fn = u.path();
#else
  fn = QFileDialog::getSaveFileName(this, QString::null, QDir::homePath() + "/licq.log");
#endif

  if (fn.isNull())
    return;

  QFile f(fn);
  if (!f.open(QIODevice::WriteOnly))
  {
    WarnUser(this, tr("Failed to open file:\n%1").arg(fn));
  }
  else
  {
    QTextStream t(&f);
    t << outputBox->toPlainText();
    f.close();
  }
}
