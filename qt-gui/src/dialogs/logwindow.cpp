/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2012 Licq developers <licq-dev@googlegroups.com>
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
#include <QHBoxLayout>
#include <QMenu>
#include <QPushButton>
#include <QScrollBar>
#include <QShowEvent>
#include <QSocketNotifier>
#include <QTextStream>
#include <QVBoxLayout>

#ifdef USE_KDE
#include <KDE/KFileDialog>
#else
#include <QFileDialog>
#endif

#include <licq/logging/log.h>
#include <licq/logging/logservice.h>
#include <licq/logging/logutils.h>

#include "core/messagebox.h"

#include "helpers/support.h"

#include "widgets/mlview.h"

#undef connect

const int LOG_SET_ALL = -1;
const int LOG_CLEAR_ALL = -2;
const int LOG_PACKETS = -3;

using Licq::PluginLogSink;
using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::LogWindow */

LogWindow::LogWindow(QWidget* parent)
  : QDialog(parent)
{
  Support::setWidgetProps(this, "NetworkLog");
  setWindowTitle(tr("Licq - Network Log"));

  QVBoxLayout* top_lay = new QVBoxLayout(this);

  outputBox = new MLView(this);
  outputBox->setSizeHintLines(16);
  outputBox->setMinimumWidth(outputBox->sizeHint().height() * 2);

  QTextDocument* doc = outputBox->document();
  // hardcoded limit, maybe should be user configurable?
  doc->setMaximumBlockCount(500);
  outputBox->setDocument(doc);

  top_lay->addWidget(outputBox);

  QHBoxLayout* buttonsLayout = new QHBoxLayout();

  // Sub menu Debug
  myDebugMenu = new QMenu(this);
  connect(myDebugMenu, SIGNAL(triggered(QAction*)), SLOT(changeDebug(QAction*)));
  connect(myDebugMenu, SIGNAL(aboutToShow()), SLOT(aboutToShowDebugMenu()));
  QAction* a;
#define ADD_DEBUG(text, data, checkable) \
    a = myDebugMenu->addAction(text); \
    a->setCheckable(checkable); \
    a->setData(data);
  ADD_DEBUG(tr("Status Info"), Licq::Log::Info, true)
  ADD_DEBUG(tr("Unknown Packets"), Licq::Log::Unknown, true)
  ADD_DEBUG(tr("Errors"), Licq::Log::Error, true)
  ADD_DEBUG(tr("Warnings"), Licq::Log::Warning, true)
  ADD_DEBUG(tr("Debug"), Licq::Log::Debug, true)
  ADD_DEBUG(tr("Raw Packets"), LOG_PACKETS, true)
  myDebugMenu->addSeparator();
  ADD_DEBUG(tr("Set All"), LOG_SET_ALL, false)
  ADD_DEBUG(tr("Clear All"), LOG_CLEAR_ALL, false)
#undef ADD_DEBUG

  QPushButton* debugLevelButton = new QPushButton(tr("Log Level"));
  debugLevelButton->setMenu(myDebugMenu);
  buttonsLayout->addWidget(debugLevelButton);

  QDialogButtonBox* buttons = new QDialogButtonBox(
      QDialogButtonBox::Close);

  connect(buttons, SIGNAL(rejected()), SLOT(hide()));
  buttons->button(QDialogButtonBox::Close)->setDefault(true);

  QPushButton* btnSave = buttons->addButton(tr("Save..."),
      QDialogButtonBox::ActionRole);
  btnSave->setAutoDefault(false);
  connect(btnSave, SIGNAL(clicked()), SLOT(save()));

  QPushButton* btnClear = buttons->addButton(tr("Clear"),
      QDialogButtonBox::ResetRole);
  btnClear->setAutoDefault(false);
  connect(btnClear, SIGNAL(clicked()), outputBox, SLOT(clear()));

  buttonsLayout->addWidget(buttons);
  top_lay->addLayout(buttonsLayout);

  myLogSink.reset(new PluginLogSink());
  Licq::gLogService.registerLogSink(myLogSink);
  myLogSink->setLogLevel(Licq::Log::Unknown, true);
  myLogSink->setLogLevel(Licq::Log::Info, true);
  myLogSink->setLogLevel(Licq::Log::Warning, true);
  myLogSink->setLogLevel(Licq::Log::Error, true);

  sn = new QSocketNotifier(myLogSink->getReadPipe(), QSocketNotifier::Read, this);
  connect(sn, SIGNAL(activated(int)), SLOT(log(int)));
}

LogWindow::~LogWindow()
{
  Licq::gLogService.unregisterLogSink(myLogSink);
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

  outputBox->append(str, false);

  if (message->level == Licq::Log::Error)
    CriticalUser(NULL, str);
}

void LogWindow::save()
{
  QString fn;

#ifdef USE_KDE
  KUrl u = KFileDialog::getSaveUrl(QString(QDir::homePath() + "/licq.log"),
      QString::null, this, tr("Licq - Save Network Log"));
  fn = u.path();
#else
  fn = QFileDialog::getSaveFileName(this, tr("Licq - Save Network Log"),
      QDir::homePath() + "/licq.log");
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

void LogWindow::aboutToShowDebugMenu()
{
  foreach (QAction* action, myDebugMenu->actions())
  {
    if (action->isCheckable())
    {
      if (action->data().toInt() == LOG_PACKETS)
        action->setChecked(myLogSink->isLoggingPackets());
      else
      {
        Licq::Log::Level level = static_cast<Licq::Log::Level>(action->data().toInt());
        action->setChecked(myLogSink->isLogging(level));
      }
    }
  }
}

void LogWindow::changeDebug(QAction* action)
{
  const int data = action->data().toInt();
  if (data == LOG_SET_ALL || data == LOG_CLEAR_ALL)
  {
    const bool enable = data == LOG_SET_ALL;
    myLogSink->setAllLogLevels(enable);
    myLogSink->setLogPackets(enable);
  }
  else if (data == LOG_PACKETS)
  {
    myLogSink->setLogPackets(action->isChecked());
  }
  else
  {
    Licq::Log::Level level = static_cast<Licq::Log::Level>(data);
    myLogSink->setLogLevel(level, action->isChecked());
  }
}
