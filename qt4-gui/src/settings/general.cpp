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

#include <licq_constants.h>

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
  QVBoxLayout* layPageDocking = new QVBoxLayout(w);
  layPageDocking->setContentsMargins(0, 0, 0, 0);

  boxDocking = new QGroupBox(tr("Docking"));
  layDocking = new QGridLayout(boxDocking);

  chkUseDock = new QCheckBox(tr("Use dock icon"), boxDocking);
  chkUseDock->setToolTip(tr("Controls whether or not the dockable icon should be displayed."));
  layDocking->addWidget(chkUseDock, 0, 0);

  chkHidden = new QCheckBox(tr("Start hidden"));
  chkHidden->setToolTip(tr("Start main window hidden. Only the dock icon will be visible."));
  layDocking->addWidget(chkHidden, 0, 1);

  rdbDockDefault = new QRadioButton(tr("Default icon"));
  layDocking->addWidget(rdbDockDefault, 1, 0);

  chkDockFortyEight = new QCheckBox(tr("64 x 48 dock icon"));
  chkDockFortyEight->setToolTip(tr("Selects between the standard 64x64 icon used in the WindowMaker/Afterstep wharf\n"
                                        "and a shorter 64x48 icon for use in the Gnome/KDE panel."));
  layDocking->addWidget(chkDockFortyEight, 1, 1);

  rdbDockThemed = new QRadioButton(tr("Themed icon"));
  layDocking->addWidget(rdbDockThemed, 2, 0);

  rdbDockTray = new QRadioButton(tr("Tray icon"));
  rdbDockTray->setToolTip(tr("Uses the freedesktop.org standard to dock a small icon into the system tray.\n"
                              "Works with many different window managers."));
  layDocking->addWidget(rdbDockTray, 3, 0);

  chkDockTrayBlink = new QCheckBox(tr("Blink on events"));
  chkDockTrayBlink->setToolTip(tr("Make tray icon blink on unread incoming events."));
  layDocking->addWidget(chkDockTrayBlink, 3, 1);

  chkDockTrayMessage = new QCheckBox(tr("Show messages"));
  chkDockTrayMessage->setToolTip(tr("Show balloon popup messages."));
  layDocking->addWidget(chkDockTrayMessage, 4, 1);

  cmbDockTheme = new QComboBox();
  // Set the currently available themes
  QString szDockThemesDir = QString::fromLocal8Bit(SHARE_DIR) + QTGUI_DIR + DOCK_DIR;
  QDir d(szDockThemesDir);
  d.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
  cmbDockTheme->addItems(d.entryList());
  connect(rdbDockDefault, SIGNAL(toggled(bool)), chkDockFortyEight, SLOT(setEnabled(bool)));
  connect(rdbDockThemed, SIGNAL(toggled(bool)), cmbDockTheme, SLOT(setEnabled(bool)));
  connect(rdbDockTray, SIGNAL(toggled(bool)), chkDockTrayBlink, SLOT(setEnabled(bool)));
  connect(chkUseDock, SIGNAL(toggled(bool)), SLOT(slot_useDockToggled(bool)));
  layDocking->addWidget(cmbDockTheme, 2, 1);


  layPageDocking->addWidget(boxDocking);
  layPageDocking->addStretch(1);

  return w;
}

QWidget* Settings::General::createPageFonts(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  layPageFonts = new QVBoxLayout(w);
  layPageFonts->setContentsMargins(0, 0, 0, 0);

  boxFont = new QGroupBox(tr("Fonts"));
  layFont = new QGridLayout(boxFont);

  // Standard font
  lblFont = new QLabel(tr("General:"));
  lblFont->setToolTip(tr("Used for normal text."));
  layFont->addWidget(lblFont, 0, 0);
  myNormalFontEdit = new FontEdit();
  myNormalFontEdit->setToolTip(lblFont->toolTip());
  lblFont->setBuddy(myNormalFontEdit);
  layFont->addWidget(myNormalFontEdit, 0, 1);
  connect(myNormalFontEdit, SIGNAL(fontSelected(const QFont&)), SLOT(normalFontChanged(const QFont&)));

  // Edit font
  lblEditFont = new QLabel(tr("Editing:"));
  lblEditFont->setToolTip(tr("Used in message editor etc."));
  layFont->addWidget(lblEditFont, 1, 0);
  myEditFontEdit = new FontEdit();
  myEditFontEdit->setToolTip(lblFont->toolTip());
  lblFont->setBuddy(myEditFontEdit);
  layFont->addWidget(myEditFontEdit, 1, 1);

  layPageFonts->addWidget(boxFont);
  layPageFonts->addStretch(1);

  return w;
}

void Settings::General::slot_useDockToggled(bool b)
{
  if (!b)
  {
#ifndef USE_KDE
    cmbDockTheme->setEnabled(false);
    rdbDockDefault->setEnabled(false);
    rdbDockThemed->setEnabled(false);
    rdbDockTray->setEnabled(false);
    chkDockFortyEight->setEnabled(false);
    chkDockTrayBlink->setEnabled(false);
    chkDockTrayMessage->setEnabled(false);
#endif
    chkHidden->setEnabled(false);
    chkHidden->setChecked(false);
    return;
  }
  else
  {
    chkHidden->setEnabled(true);
  }

  // Turned on
#ifndef USE_KDE
  rdbDockDefault->setEnabled(true);
  rdbDockThemed->setEnabled(true);
  rdbDockTray->setEnabled(true);
  if (rdbDockDefault->isChecked())
  {
    chkDockFortyEight->setEnabled(true);
    cmbDockTheme->setEnabled(false);
    chkDockTrayBlink->setEnabled(false);
    chkDockTrayMessage->setEnabled(false);
  }
  else if (rdbDockThemed->isChecked())
  {
    chkDockFortyEight->setEnabled(false);
    cmbDockTheme->setEnabled(true);
    chkDockTrayBlink->setEnabled(false);
    chkDockTrayMessage->setEnabled(false);
  }
  else if (rdbDockTray->isChecked())
  {
    chkDockFortyEight->setEnabled(false);
    cmbDockTheme->setEnabled(false);
    chkDockTrayBlink->setEnabled(true);
    chkDockTrayMessage->setEnabled(true);
  }
  else
    rdbDockDefault->setChecked(true);
#endif
}

void Settings::General::normalFontChanged(const QFont& font)
{
  myEditFontEdit->setFont(font);
}

void Settings::General::load()
{
  Config::General* generalConfig = Config::General::instance();

  chkHidden->setChecked(generalConfig->mainwinStartHidden());
  chkUseDock->setChecked(generalConfig->dockMode() != Config::General::DockNone);
#ifndef USE_KDE
  rdbDockDefault->setChecked(generalConfig->dockMode() == Config::General::DockDefault);
  chkDockFortyEight->setChecked(generalConfig->defaultIconFortyEight());
  rdbDockThemed->setChecked(generalConfig->dockMode() == Config::General::DockThemed);
  for (unsigned short i = 0; i < cmbDockTheme->count(); i++)
  {
    if (cmbDockTheme->itemText(i) == generalConfig->themedIconTheme())
    {
      cmbDockTheme->setCurrentIndex(i);
      break;
    }
  }
#endif
  rdbDockTray->setChecked(generalConfig->dockMode() == Config::General::DockTray);
  chkDockTrayBlink->setChecked(generalConfig->trayBlink());
  chkDockTrayMessage->setChecked(generalConfig->showMessage());
  slot_useDockToggled(chkUseDock->isChecked());

  myNormalFontEdit->setFont(QFont(generalConfig->normalFont()));
  myEditFontEdit->setFont(QFont(generalConfig->editFont()));
}

void Settings::General::apply()
{
  Config::General* generalConfig = Config::General::instance();
  generalConfig->blockUpdates(true);

  generalConfig->setMainwinStartHidden(chkHidden->isChecked());
  Config::General::DockMode newDockMode = Config::General::DockNone;
  if (chkUseDock->isChecked())
  {
#ifdef USE_KDE
    newDockMode = Config::General::DockTray;
#else
    if (rdbDockDefault->isChecked())
      newDockMode = Config::General::DockDefault;
    else if (rdbDockThemed->isChecked())
      newDockMode = Config::General::DockThemed;
    else if (rdbDockTray->isChecked())
      newDockMode = Config::General::DockTray;
#endif
  }
  generalConfig->setDockMode(newDockMode);
#ifndef USE_KDE
  generalConfig->setDefaultIconFortyEight(chkDockFortyEight->isChecked());
  generalConfig->setThemedIconTheme(cmbDockTheme->currentText());
#endif
  generalConfig->setTrayBlink(chkDockTrayBlink->isChecked());
  generalConfig->setShowMessage(chkDockTrayMessage->isChecked());

  if (myNormalFontEdit->font() == Config::General::instance()->defaultFont())
    generalConfig->setNormalFont(QString());
  else
    generalConfig->setNormalFont(myNormalFontEdit->font().toString());

  if (myEditFontEdit->font() == Config::General::instance()->defaultFont())
    generalConfig->setEditFont(QString());
  else
    generalConfig->setEditFont(myEditFontEdit->font().toString());

  generalConfig->blockUpdates(false);
}
