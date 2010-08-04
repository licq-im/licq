// -*- c-basic-offset: 2 -*-
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

#include "general.h"

#include "config.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>

#include <licq/daemon.h>

#include "config/general.h"
#include "core/gui-defines.h"
#include "widgets/fontedit.h"

#include "settingsdlg.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::Settings::General */

Settings::General::General(SettingsDlg* parent)
  : QObject(parent)
{
  parent->addPage(SettingsDlg::DockingPage, createPageDocking(parent),
      tr("Docking"), SettingsDlg::ContactListPage);
  parent->addPage(SettingsDlg::FontsPage, createPageFonts(parent),
      tr("Fonts"), SettingsDlg::ContactListPage);

  load();
}

QWidget* Settings::General::createPageDocking(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  QVBoxLayout* myPageDockingLayout = new QVBoxLayout(w);
  myPageDockingLayout->setContentsMargins(0, 0, 0, 0);

  myDockingBox = new QGroupBox(tr("Docking"));
  myDockingLayout = new QGridLayout(myDockingBox);

  myUseDockCheck = new QCheckBox(tr("Use dock icon"), myDockingBox);
  myUseDockCheck->setToolTip(tr("Controls whether or not the dockable icon should be displayed."));
  myDockingLayout->addWidget(myUseDockCheck, 0, 0);

  myHiddenCheck = new QCheckBox(tr("Start hidden"));
  myHiddenCheck->setToolTip(tr("Start main window hidden. Only the dock icon will be visible."));
  myDockingLayout->addWidget(myHiddenCheck, 0, 1);

  myDockDefaultRadio = new QRadioButton(tr("Default icon"));
  myDockingLayout->addWidget(myDockDefaultRadio, 1, 0);

  myDockFortyEightCheck = new QCheckBox(tr("64 x 48 dock icon"));
  myDockFortyEightCheck->setToolTip(tr("Selects between the standard 64x64 icon used in the WindowMaker/Afterstep wharf\n"
      "and a shorter 64x48 icon for use in the Gnome/KDE panel."));
  myDockingLayout->addWidget(myDockFortyEightCheck, 1, 1);

  myDockThemedRadio = new QRadioButton(tr("Themed icon"));
  myDockingLayout->addWidget(myDockThemedRadio, 2, 0);

  myDockTrayRadio = new QRadioButton(tr("Tray icon"));
  myDockTrayRadio->setToolTip(tr("Uses the freedesktop.org standard to dock a small icon into the system tray.\n"
      "Works with many different window managers."));
  myDockingLayout->addWidget(myDockTrayRadio, 3, 0);

  myDockTrayBlinkCheck = new QCheckBox(tr("Blink on events"));
  myDockTrayBlinkCheck->setToolTip(tr("Make tray icon blink on unread incoming events."));
  myDockingLayout->addWidget(myDockTrayBlinkCheck, 3, 1);

  // TODO: Move this to Event pages when we get different kinds of popup
  myTrayMsgOnlineNotify = new QCheckBox(tr("Show popup for Online notify"));
  myTrayMsgOnlineNotify->setToolTip(tr("Show balloon popup message when contacts marked for online notify comes online."));
  myDockingLayout->addWidget(myTrayMsgOnlineNotify, 4, 1);

  myDockThemeCombo = new QComboBox();
  // Set the currently available themes
  QString szDockThemesDir = QString::fromLocal8Bit(Licq::gDaemon.shareDir().c_str()) + QTGUI_DIR + DOCK_DIR;
  QDir d(szDockThemesDir);
  d.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
  myDockThemeCombo->addItems(d.entryList());
  connect(myDockDefaultRadio, SIGNAL(toggled(bool)), myDockFortyEightCheck, SLOT(setEnabled(bool)));
  connect(myDockThemedRadio, SIGNAL(toggled(bool)), myDockThemeCombo, SLOT(setEnabled(bool)));
  connect(myDockTrayRadio, SIGNAL(toggled(bool)), myDockTrayBlinkCheck, SLOT(setEnabled(bool)));
  connect(myDockTrayRadio, SIGNAL(toggled(bool)), myTrayMsgOnlineNotify, SLOT(setEnabled(bool)));
  connect(myUseDockCheck, SIGNAL(toggled(bool)), SLOT(useDockToggled(bool)));
  myDockingLayout->addWidget(myDockThemeCombo, 2, 1);


  myPageDockingLayout->addWidget(myDockingBox);
  myPageDockingLayout->addStretch(1);

  return w;
}

QWidget* Settings::General::createPageFonts(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  myPageFontsLayout = new QVBoxLayout(w);
  myPageFontsLayout->setContentsMargins(0, 0, 0, 0);

  myFontBox = new QGroupBox(tr("Fonts"));
  myFontLayout = new QGridLayout(myFontBox);

  // Standard font
  myFontLabel = new QLabel(tr("General:"));
  myFontLabel->setToolTip(tr("Used for normal text."));
  myFontLayout->addWidget(myFontLabel, 0, 0);
  myNormalFontEdit = new FontEdit();
  myNormalFontEdit->setToolTip(myFontLabel->toolTip());
  myFontLabel->setBuddy(myNormalFontEdit);
  myFontLayout->addWidget(myNormalFontEdit, 0, 1);
  connect(myNormalFontEdit, SIGNAL(fontSelected(const QFont&)), SLOT(normalFontChanged(const QFont&)));

  // Edit font
  myEditFontLabel = new QLabel(tr("Editing:"));
  myEditFontLabel->setToolTip(tr("Used in message editor etc."));
  myFontLayout->addWidget(myEditFontLabel, 1, 0);
  myEditFontEdit = new FontEdit();
  myEditFontEdit->setToolTip(myEditFontLabel->toolTip());
  myEditFontLabel->setBuddy(myEditFontEdit);
  myFontLayout->addWidget(myEditFontEdit, 1, 1);

  // History font
  myHistoryFontLabel = new QLabel(tr("History:"));
  myHistoryFontLabel->setToolTip(tr("Used in message history."));
  myFontLayout->addWidget(myHistoryFontLabel, 2, 0);
  myHistoryFontEdit = new FontEdit();
  myHistoryFontEdit->setToolTip(myHistoryFontLabel->toolTip());
  myHistoryFontLabel->setBuddy(myHistoryFontEdit);
  myFontLayout->addWidget(myHistoryFontEdit, 2, 1);

  // Fixed font
  myFixedFontLabel = new QLabel(tr("Fixed:"));
  myFixedFontLabel->setToolTip(tr("Used in file editor and network log."));
  myFontLayout->addWidget(myFixedFontLabel, 3, 0);
  myFixedFontEdit = new FontEdit();
  myFixedFontEdit->setToolTip(myFixedFontLabel->toolTip());
  myFixedFontLabel->setBuddy(myFixedFontEdit);
  myFontLayout->addWidget(myFixedFontEdit, 3, 1);

  myPageFontsLayout->addWidget(myFontBox);
  myPageFontsLayout->addStretch(1);

  return w;
}

void Settings::General::useDockToggled(bool useDock)
{
  if (!useDock)
  {
#ifndef USE_KDE
    myDockThemeCombo->setEnabled(false);
    myDockDefaultRadio->setEnabled(false);
    myDockThemedRadio->setEnabled(false);
    myDockTrayRadio->setEnabled(false);
    myDockFortyEightCheck->setEnabled(false);
    myDockTrayBlinkCheck->setEnabled(false);
    myTrayMsgOnlineNotify->setEnabled(false);
#endif
    myHiddenCheck->setEnabled(false);
    myHiddenCheck->setChecked(false);
    return;
  }
  else
  {
    myHiddenCheck->setEnabled(true);
  }

  // Turned on
#ifndef USE_KDE
  myDockDefaultRadio->setEnabled(true);
  myDockThemedRadio->setEnabled(true);
  myDockTrayRadio->setEnabled(true);
  if (myDockDefaultRadio->isChecked())
  {
    myDockFortyEightCheck->setEnabled(true);
    myDockThemeCombo->setEnabled(false);
    myDockTrayBlinkCheck->setEnabled(false);
    myTrayMsgOnlineNotify->setEnabled(false);
  }
  else if (myDockThemedRadio->isChecked())
  {
    myDockFortyEightCheck->setEnabled(false);
    myDockThemeCombo->setEnabled(true);
    myDockTrayBlinkCheck->setEnabled(false);
    myTrayMsgOnlineNotify->setEnabled(false);
  }
  else if (myDockTrayRadio->isChecked())
  {
    myDockFortyEightCheck->setEnabled(false);
    myDockThemeCombo->setEnabled(false);
    myDockTrayBlinkCheck->setEnabled(true);
    myTrayMsgOnlineNotify->setEnabled(true);
  }
  else
    myDockDefaultRadio->setChecked(true);
#endif
}

void Settings::General::normalFontChanged(const QFont& font)
{
  myEditFontEdit->setFont(font);
  myHistoryFontEdit->setFont(font);
}

void Settings::General::load()
{
  Config::General* generalConfig = Config::General::instance();

  myHiddenCheck->setChecked(generalConfig->mainwinStartHidden());
  myUseDockCheck->setChecked(generalConfig->dockMode() != Config::General::DockNone);
#ifndef USE_KDE
  myDockDefaultRadio->setChecked(generalConfig->dockMode() == Config::General::DockDefault);
  myDockFortyEightCheck->setChecked(generalConfig->defaultIconFortyEight());
  myDockThemedRadio->setChecked(generalConfig->dockMode() == Config::General::DockThemed);
  for (int i = 0; i < myDockThemeCombo->count(); ++i)
  {
    if (myDockThemeCombo->itemText(i) == generalConfig->themedIconTheme())
    {
      myDockThemeCombo->setCurrentIndex(i);
      break;
    }
  }
#endif
  myDockTrayRadio->setChecked(generalConfig->dockMode() == Config::General::DockTray);
  myDockTrayBlinkCheck->setChecked(generalConfig->trayBlink());
  myTrayMsgOnlineNotify->setChecked(generalConfig->trayMsgOnlineNotify());
  useDockToggled(myUseDockCheck->isChecked());

  myNormalFontEdit->setFont(QFont(generalConfig->normalFont()));
  myEditFontEdit->setFont(QFont(generalConfig->editFont()));
  myHistoryFontEdit->setFont(QFont(generalConfig->historyFont()));
  myFixedFontEdit->setFont(QFont(generalConfig->fixedFont()));
}

void Settings::General::apply()
{
  Config::General* generalConfig = Config::General::instance();
  generalConfig->blockUpdates(true);

  generalConfig->setMainwinStartHidden(myHiddenCheck->isChecked());
  Config::General::DockMode newDockMode = Config::General::DockNone;
  if (myUseDockCheck->isChecked())
  {
#ifdef USE_KDE
    newDockMode = Config::General::DockTray;
#else
    if (myDockDefaultRadio->isChecked())
      newDockMode = Config::General::DockDefault;
    else if (myDockThemedRadio->isChecked())
      newDockMode = Config::General::DockThemed;
    else if (myDockTrayRadio->isChecked())
      newDockMode = Config::General::DockTray;
#endif
  }
  generalConfig->setDockMode(newDockMode);
#ifndef USE_KDE
  generalConfig->setDefaultIconFortyEight(myDockFortyEightCheck->isChecked());
  generalConfig->setThemedIconTheme(myDockThemeCombo->currentText());
#endif
  generalConfig->setTrayBlink(myDockTrayBlinkCheck->isChecked());
  generalConfig->setTrayMsgOnlineNotify(myTrayMsgOnlineNotify->isChecked());

  if (myNormalFontEdit->font() == Config::General::instance()->defaultFont())
    generalConfig->setNormalFont(QString::null);
  else
    generalConfig->setNormalFont(myNormalFontEdit->font().toString());

  if (myEditFontEdit->font() == Config::General::instance()->defaultFont())
    generalConfig->setEditFont(QString::null);
  else
    generalConfig->setEditFont(myEditFontEdit->font().toString());

  if (myHistoryFontEdit->font() == Config::General::instance()->defaultFont())
    generalConfig->setHistoryFont(QString::null);
  else
    generalConfig->setHistoryFont(myHistoryFontEdit->font().toString());

  if (myFixedFontEdit->font() == Config::General::instance()->defaultFixedFont())
    generalConfig->setFixedFont(QString::null);
  else
    generalConfig->setFixedFont(myFixedFontEdit->font().toString());

  generalConfig->blockUpdates(false);
}
