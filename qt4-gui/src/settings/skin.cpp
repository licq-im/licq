// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2009 Licq developers
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

#include "skin.h"

#include <QDir>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QMenuBar>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

#include <licq_log.h>
#include <licq_constants.h>

#include "config/iconmanager.h"
#include "config/emoticons.h"
#include "config/skin.h"

#include "core/gui-defines.h"
#include "core/licqgui.h"
#include "core/messagebox.h"

#include "dialogs/editfiledlg.h"

#include "helpers/licqstrings.h"
#include "helpers/support.h"

#include "widgets/skinnablebutton.h"
#include "widgets/skinnablecombobox.h"
#include "widgets/skinnablelabel.h"

#include "views/userview.h"

#include "settingsdlg.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::Settings::Skin */

enum {
  MAX_HEIGHT = 190, /* Height of icon preview widget, this is
                       (number-of-Icons) / 3 * 19.
                       if the result isn't a multiple of 19,
                       round it up to become a multiple of 19 */
  MAX_HEIGHT_SKIN = 130
};

Settings::Skin::Skin(SettingsDlg* parent)
  : QObject(parent)
{
  parent->addPage(SettingsDlg::SkinPage, createPageSkin(parent),
      tr("Skin"), SettingsDlg::ContactListPage);

  load();
}

QWidget* Settings::Skin::createPageSkin(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  myPageSkinLayout = new QVBoxLayout(w);
  myPageSkinLayout->setContentsMargins(0, 0, 0, 0);

  // Setup a list of previewable icons
  // The strings reflect what we expect to find in the icon files.
  // The result of these two lists is used to load the icons, the order of
  // this list will be the order that the icons get rendered in the preview.
  myIconNames << "Online" << "Offline" << "FFC" << "Away" << "NA"
                   << "Occupied" << "DND" << "Private" << "Message" << "Url"
                   << "Chat" << "File" << "SMS" << "Contact" << "Authorize" << "ReqAuthorize"
                   << "SecureOff" << "SecureOn" << "Search" << "Remove" << "History"
                   << "Info" << "AIMOnline" << "AIMOffline" << "AIMAway"
                   << "MSNOnline" << "MSNOffline" << "MSNAway"
                   << "MSNOccupied" << "MSNPrivate";

  myExtIconNames << "Collapsed" << "Expanded" << "Birthday" << "Cellular"
                      << "CustomAR" << "Invisible" << "Typing" << "Phone"
                      << "PhoneFollowMeActive" << "PhoneFollowMeBusy"
                      << "ICQphoneActive" << "ICQphoneBusy" << "SharedFiles";


  QHBoxLayout* layMain = new QHBoxLayout();
  QGroupBox* boxSkin = new QGroupBox(tr("Skin Selection"));
  QVBoxLayout* laySkin = new QVBoxLayout(boxSkin);
  QGroupBox* boxIcons = new QGroupBox(tr("Icon Selection"));
  QHBoxLayout* layIconSets = new QHBoxLayout(boxIcons);
  layMain->addWidget(boxSkin);
  layMain->addWidget(boxIcons);

  // Skin Box
  QLabel* lblSkin = new QLabel(tr("S&kins:"));
  laySkin->addWidget(lblSkin);

  mySkinPreview = new QLabel();
  mySkinPreview->setFixedSize(75, MAX_HEIGHT_SKIN);
  laySkin->addWidget(mySkinPreview, 0, Qt::AlignHCenter);
  laySkin->addStretch();

  mySkinCombo = new QComboBox();
  mySkinCombo->setToolTip(tr("Use this combo box to select one of the available skins"));
  connect(mySkinCombo, SIGNAL(currentIndexChanged(const QString&)),
      SLOT(previewSkin(const QString&)));
  connect(mySkinCombo, SIGNAL(highlighted(const QString&)),
      SLOT(previewSkin(const QString&)));
  lblSkin->setBuddy(mySkinCombo);
  laySkin->addWidget(mySkinCombo);

  // Icons Box

  QVBoxLayout* layIcon = new QVBoxLayout();
  QLabel* lblIcon = new QLabel(tr("Icons:"));
  lblIcon->setAlignment(Qt::AlignHCenter);
  layIcon->addWidget(lblIcon, 0, Qt::AlignHCenter);

  myIconPreview = new SkinBrowserPreviewArea();
  layIcon->addWidget(myIconPreview, 0, Qt::AlignHCenter);

  myIconCombo = new QComboBox();
  myIconCombo->setToolTip(tr("Use this combo box to select one of the available icon sets"));
  connect(myIconCombo, SIGNAL(currentIndexChanged(const QString&)),
      SLOT(previewIcons(const QString&)));
  connect(myIconCombo, SIGNAL(highlighted(const QString&)),
      SLOT(previewIcons(const QString&)));
  lblIcon->setBuddy(myIconCombo);
  layIcon->addWidget(myIconCombo);

  layIcon->addStretch();
  layIconSets->addLayout(layIcon);


  QVBoxLayout* layExtIcon = new QVBoxLayout();
  QLabel* lblExtIcon = new QLabel(tr("Extended Icons:"));
  lblExtIcon->setAlignment(Qt::AlignHCenter);
  layExtIcon->addWidget(lblExtIcon, 0, Qt::AlignHCenter);

  myExtIconPreview = new SkinBrowserPreviewArea();
  layExtIcon->addWidget(myExtIconPreview, 0, Qt::AlignHCenter);

  myExtIconCombo = new QComboBox();
  myExtIconCombo->setToolTip(tr("Use this combo box to select one of the available extended icon sets"));
  connect(myExtIconCombo, SIGNAL(currentIndexChanged(const QString&)),
      SLOT(previewExtIcons(const QString&)));
  connect(myExtIconCombo, SIGNAL(highlighted(const QString&)),
      SLOT(previewExtIcons(const QString&)));
  lblExtIcon->setBuddy(myExtIconCombo);
  layExtIcon->addWidget(myExtIconCombo);

  layExtIcon->addStretch();
  layIconSets->addLayout(layExtIcon);


  QVBoxLayout* layEmoticon = new QVBoxLayout();
  QLabel* lblEmoticon = new QLabel(tr("Emoticons:"));
  lblEmoticon->setAlignment(Qt::AlignHCenter);
  layEmoticon->addWidget(lblEmoticon, 0, Qt::AlignHCenter);

  myEmoticonPreview= new SkinBrowserPreviewArea();
  layEmoticon->addWidget(myEmoticonPreview, 0, Qt::AlignHCenter);

  myEmoticonCombo = new QComboBox();
  myEmoticonCombo->setToolTip(tr("Use this combo box to select one of the available emoticon icon sets"));
  connect(myEmoticonCombo, SIGNAL(currentIndexChanged(const QString&)),
      SLOT(previewEmoticons(const QString&)));
  connect(myEmoticonCombo, SIGNAL(highlighted(const QString&)),
      SLOT(previewEmoticons(const QString&)));
  lblEmoticon->setBuddy(myEmoticonCombo);
  layEmoticon->addWidget(myEmoticonCombo);

  layEmoticon->addStretch();
  layIconSets->addLayout(layEmoticon);

  // Buttons
  QPushButton* btnEdit = new QPushButton(tr("&Edit Skin"));
  connect(btnEdit, SIGNAL(clicked()), SLOT(editSkin()));
  laySkin->addWidget(btnEdit);

  myPageSkinLayout->addLayout(layMain);
  myPageSkinLayout->addStretch(1);

  return w;
}

void Settings::Skin::load()
{
  // Load up the available packs
  QDir skinsPath(QString::fromLocal8Bit(SHARE_DIR) + QTGUI_DIR + SKINS_DIR);
  QDir skinsUserPath(QString::fromLocal8Bit(BASE_DIR) + QTGUI_DIR + SKINS_DIR);
  skinsPath.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
  skinsUserPath.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
  if (skinsPath.count() == 0 && skinsUserPath.count() == 0)
  {
    gLog.Error("%sError reading %s directory.\n", L_ERRORxSTR, skinsPath.path().toLatin1().data());
    mySkinCombo->addItem(tr("Error"));
    mySkinCombo->setEnabled(false);
  }
  else
  {
    QStringList lst = skinsPath.entryList();
    QString current(Config::Skin::active()->skinName());
    foreach (const QString& skin, lst)
    {
      mySkinCombo->addItem(skin);
      if (skin == current)
        mySkinCombo->setCurrentIndex(mySkinCombo->count() - 1);
    }
    // check for skins in current base dir, too
    lst = skinsUserPath.entryList();
    foreach (const QString& skin, lst)
    {
      // Check for duplicates
      int num = mySkinCombo->count();
      bool dup = false;
      for (int i = 0; i < num; i++)
        if (mySkinCombo->itemText(i) == skin)
          dup = true;
      if (!dup)
      {
        mySkinCombo->addItem(skin);
        if (skin == current)
          mySkinCombo->setCurrentIndex(mySkinCombo->count() - 1);
      }
    }
  }

  loadIconsetList(ICONS_DIR, myIconCombo, IconManager::instance()->iconSet(), "Online");
  loadIconsetList(EXTICONS_DIR, myExtIconCombo, IconManager::instance()->extendedIconSet(), "Phone");

  const Emoticons* emoticons = Emoticons::self();
  const QStringList themes = emoticons->themes();
  myEmoticonCombo->insertItems(0, themes);

  const QString selected = emoticons->theme();
  const int index = themes.indexOf(selected);
  if (index != -1)
    myEmoticonCombo->setCurrentIndex(index);

  // Create initial preview
  previewSkin(mySkinCombo->currentText());
  previewIcons(myIconCombo->currentText());
  previewExtIcons(myExtIconCombo->currentText());
  previewEmoticons(myEmoticonCombo->currentText());
}

void Settings::Skin::loadIconsetList(const QString& subdir, QComboBox* iconCombo,
    const QString& current, const QString& exampleIcon)
{
  QDir iconsPath(QString::fromLocal8Bit(SHARE_DIR) + QTGUI_DIR + subdir);
  QDir iconsUserPath(QString::fromLocal8Bit(BASE_DIR) + QTGUI_DIR + subdir);
  iconsPath.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
  iconsUserPath.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
  if (iconsPath.count() == 0 && iconsUserPath.count() == 0)
  {
    gLog.Error("%sError reading %s directory.\n", L_ERRORxSTR, iconsPath.path().toLatin1().data());
    iconCombo->addItem(tr("Error"));
    iconCombo->setEnabled(false);
  }
  else
  {
    QStringList lst = iconsPath.entryList();
    foreach (const QString& iconset, lst)
    {
      iconsPath.cd(iconset);
      QString iconsFile = QString("%1/%2.icons").arg(iconsPath.path()).arg(iconset);
      char sFileName[MAX_FILENAME_LEN] = "";
      CIniFile fIconsConf;
      if (!fIconsConf.LoadFile(iconsFile.toLatin1()))
      {
        WarnUser(dynamic_cast<SettingsDlg*>(parent()),
            tr("Unable to open icons file\n%1\nIconset '%2' has been disabled.")
            .arg(iconsFile)
            .arg(iconset));
        iconsPath.cdUp();
        continue;
      }
      fIconsConf.SetSection("icons");
      fIconsConf.ReadStr(exampleIcon.toAscii().data(), sFileName, "");
      QString pmFile = QString("%1/%2").arg(iconsPath.path()).arg(sFileName);
      iconCombo->addItem(QPixmap(pmFile), iconset);
      if (iconset == current)
        iconCombo->setCurrentIndex(iconCombo->count() - 1);
      iconsPath.cdUp();
    }
    // check for icons in current base dir, too
    lst = iconsUserPath.entryList();
    foreach (const QString& iconset, lst)
    {
      iconsUserPath.cd(iconset);
      QString iconsFile = QString("%1/%2.icons").arg(iconsUserPath.path()).arg(iconset);
      char sFileName[MAX_FILENAME_LEN] = "";
      CIniFile fIconsConf;
      if (!fIconsConf.LoadFile(iconsFile.toLatin1()))
      {
        WarnUser(dynamic_cast<SettingsDlg*>(parent()),
            tr("Unable to open icons file\n%1\nIconset '%2' has been disabled.").arg(iconsFile).arg(iconset));
        iconsUserPath.cdUp();
        continue;
      }
      fIconsConf.SetSection("icons");
      fIconsConf.ReadStr(exampleIcon.toAscii().data(), sFileName, "");
      QString pmFile = QString("%1/%2").arg(iconsUserPath.path()).arg(sFileName);
      // Check for duplicates
      int num = iconCombo->count();
      bool dup = false;
      for (int i = 0; i < num; i++)
      {
        if (iconCombo->itemText(i) == iconset)
        {
          dup = true;
          iconCombo->setItemIcon(i, QPixmap(pmFile));
          iconCombo->setItemText(i, iconset);
        }
      }
      if (!dup)
      {
        iconCombo->addItem(QPixmap(pmFile), iconset);
        if (iconset == current)
          iconCombo->setCurrentIndex(iconCombo->count() - 1);
      }
      iconsUserPath.cdUp();
    }
  }
}

void Settings::Skin::apply()
{
  IconManager* iconManager = IconManager::instance();

  Config::Skin::active()->loadSkin(mySkinCombo->currentText().toLocal8Bit());

  if (!iconManager->loadIcons(myIconCombo->currentText()))
    WarnUser(dynamic_cast<SettingsDlg*>(parent()), tr("Unable to load icons\n%1.")
        .arg(myIconCombo->currentText().toLocal8Bit().data()));

  if (!iconManager->loadExtendedIcons(myExtIconCombo->currentText()))
    WarnUser(dynamic_cast<SettingsDlg*>(parent()), tr("Unable to load extended icons\n%1.")
        .arg(myExtIconCombo->currentText().toLocal8Bit().data()));

  Emoticons::self()->setTheme(myEmoticonCombo->currentText());
}

void Settings::Skin::editSkin()
{
  if (mySkinCombo->currentText().isEmpty()) return;
  QString f;
  f.sprintf("%s%s%s%s/%s.skin", BASE_DIR, QTGUI_DIR, SKINS_DIR,
            QFile::encodeName(mySkinCombo->currentText()).data(),
            QFile::encodeName(mySkinCombo->currentText()).data());
  if (!QFile(f).exists())
    f.sprintf("%s%s%s%s/%s.skin", SHARE_DIR, QTGUI_DIR, SKINS_DIR,
              QFile::encodeName(mySkinCombo->currentText()).data(),
              QFile::encodeName(mySkinCombo->currentText()).data());
  new EditFileDlg(f);
}

void Settings::Skin::previewSkin(const QString& skin)
{
  mySkinPreview->setPixmap(renderSkin(skin));
}

void Settings::Skin::previewIcons(const QString& icon)
{
  myIconPreview->setPixmapList(loadIcons(icon, ICONS_DIR, myIconNames));
}

void Settings::Skin::previewExtIcons(const QString& extIcon)
{
  myExtIconPreview->setPixmapList(loadIcons(extIcon, EXTICONS_DIR, myExtIconNames));
}

IconList Settings::Skin::loadIcons(const QString& iconSet, const QString& subdir,
    const QStringList& iconNames)
{
  IconList icons;
  QString subpath = QTGUI_DIR + subdir + iconSet + "/" + iconSet + ".icons";
  QString iconsFile = QString::fromLocal8Bit(BASE_DIR) + subpath;
  char sFileName[MAX_FILENAME_LEN] = "";
  CIniFile fIconsConf;
  if (!fIconsConf.LoadFile(iconsFile.toLatin1()))
  {
    iconsFile = QString::fromLocal8Bit(SHARE_DIR) + subpath;
    if (!fIconsConf.LoadFile(iconsFile.toLatin1()))
    {
      WarnUser(dynamic_cast<SettingsDlg*>(parent()), tr("Unable to open icons file\n%1").arg(iconsFile));
      return icons;
    }
  }
  fIconsConf.SetSection("icons");
  foreach (const QString& iconName, iconNames)
  {
    fIconsConf.ReadStr(iconName.toAscii().data(), sFileName, "");
    QString pmFile = iconsFile.left(iconsFile.length()-iconSet.length()-6) + QString::fromAscii(sFileName);
    QPixmap pm(pmFile);
    if (! pm.isNull())
      icons.append(pm);
  }
  return icons;
}

void Settings::Skin::previewEmoticons(const QString& emoticon)
{
  IconList icons;
  const QStringList files = Emoticons::self()->fileList(emoticon);
  foreach (const QString& i, files)
  {
    QImage img = QImage(i);
    // hack: SkinBrowserPreviewArea only draws the first 16 pixels
    const int max_area = 16;
    QSize size = img.size();
    if (size.isValid() && (size.width() > max_area || size.height() > max_area))
    {
      if (size.width() > size.height())
        img = img.scaledToWidth(max_area);
      else
        img = img.scaledToHeight(max_area);
    }

    QPixmap pm(QPixmap::fromImage(img));
    if (!pm.isNull())
      icons.append(pm);
  }
  myEmoticonPreview->setPixmapList(icons);
}

void Settings::Skin::resizeEvent(QResizeEvent* /* e */)
{
  mySkinPreview->update();
  myIconPreview->update();
  myExtIconPreview->update();
}

SkinBrowserPreviewArea::SkinBrowserPreviewArea(QWidget* parent)
  : QWidget(parent)
{
  setObjectName("SkinBrowserPreviewArea");
  setFixedSize(54, MAX_HEIGHT);
}

void SkinBrowserPreviewArea::setPixmapList(const IconList& iconList)
{
  myIconList = iconList;
  this->update();
}

void SkinBrowserPreviewArea::paintEvent(QPaintEvent* /* event */)
{
  QPainter p(this);
  unsigned short int X = 0;
  unsigned short int Y = 0;
  foreach (const QPixmap& icon, myIconList)
  {
    p.drawPixmap(X, Y, icon, 0, 0, 16, 16);
    // determine next drawing position (16px + 3px space, so we need 19pixels space)
    X = ((X+19) <= (this->width()-16)) ? (X+19) : 0;
    Y = (X==0) ? (Y+19) : Y;
  }
  p.end();
}

QPixmap Settings::Skin::renderSkin(const QString& skinName)
{
  // This method renders a skin preview in realtime. This is accomplished
  // by creating a new Widget and applying the Skin &skinName to it. The
  // widget is never shown, but instead using grabWidget() it is copied
  // into a pixmap, which afterwards is returned to the caller.

  SkinnableButton* btnSystem = NULL;
  SkinnableLabel* lblMsg = NULL;
  SkinnableLabel* lblStatus = NULL;
  Config::Skin* skin = NULL;
  QMenuBar* menu = NULL;
  SkinnableComboBox* cmbUserGroups = NULL;

  QWidget w;
  w.setFixedWidth(188); // this is (75x130) * 2.5
  w.setFixedHeight(325);

  skin = new Config::Skin(skinName);

  // Background
  QPixmap p;
  if (!skin->frame.pixmap.isNull())
  {
    p = QPixmap::fromImage(skin->frame.pixmap.toImage().scaled(w.width(), w.height()));
    QPalette palette(w.palette());
    palette.setBrush(w.backgroundRole(), QBrush(p));
    w.setPalette(palette);
  }

  // Group Combo Box
  cmbUserGroups = new SkinnableComboBox(skin->cmbGroups, &w);
  cmbUserGroups->setGeometry(skin->cmbGroups.borderToRect(&w));
  cmbUserGroups->addItem(LicqStrings::getSystemGroupName(GROUP_ALL_USERS));

  // The Menu Button
  if (!skin->frame.hasMenuBar)
  {
    btnSystem = new SkinnableButton(skin->btnSys, tr("System"), &w);
    btnSystem->setGeometry(skin->btnSys.borderToRect(&w));
    menu = NULL;
  }
  else
  {
    menu = new QMenuBar(&w);
    menu->addAction(skin->btnSys.caption.isNull() ? tr("System") : skin->btnSys.caption);
    skin->AdjustForMenuBar(menu->height());
    menu->show();
    btnSystem = NULL;
  }

  // Message Label
  lblMsg = new SkinnableLabel(skin->lblMsg, NULL, &w);
  lblMsg->setGeometry(skin->lblMsg.borderToRect(&w));
  lblMsg->setText(LicqStrings::getSystemGroupName(GROUP_NEW_USERS));

  // Status Label
  lblStatus = new SkinnableLabel(skin->lblStatus, NULL, &w);
  lblStatus->setGeometry(skin->lblStatus.borderToRect(&w));
  lblStatus->setText(LicqStrings::getStatus(ICQ_STATUS_ONLINE, false));
  lblStatus->setPrependPixmap(IconManager::instance()->iconForStatus(ICQ_STATUS_ONLINE));

  // Userview
  UserView userView(LicqGui::instance()->contactList(), &w);
  userView.setGeometry(skin->frame.border.left, skin->frame.border.top,
                        w.width() - skin->frameWidth(), w.height() - skin->frameHeight());

  userView.setPalette(skin->palette(dynamic_cast<SettingsDlg*>(parent())));
  userView.setColors(skin->backgroundColor);

  if (skin->frame.transparent)
  {
    QPalette palette(userView.palette());
    palette.setBrush(userView.backgroundRole(), QBrush(p));
    userView.setPalette(palette);
  }
  userView.show();

  QPixmap tmp(QPixmap::grabWidget(&w));

  delete btnSystem;
  delete lblMsg;
  delete lblStatus;
  delete skin;
  delete menu;
  delete cmbUserGroups;

  return QPixmap::fromImage(QImage(tmp.toImage().scaled(75, MAX_HEIGHT_SKIN)));
}
