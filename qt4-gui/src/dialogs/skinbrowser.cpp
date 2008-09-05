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

#include "skinbrowser.h"

#include <QDir>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QLinkedList>
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

#include "helpers/licqstrings.h"
#include "helpers/support.h"

#include "widgets/skinnablebutton.h"
#include "widgets/skinnablecombobox.h"
#include "widgets/skinnablelabel.h"

#include "views/userview.h"

#include "editfiledlg.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::SkinBrowserDlg */

enum {
  MAX_HEIGHT = 190, /* Height of icon preview widget, this is
                       (number-of-Icons) / 3 * 19.
                       if the result isn't a multiple of 19,
                       round it up to become a multiple of 19 */
  MAX_HEIGHT_SKIN = 130
};

SkinBrowserDlg* SkinBrowserDlg::myInstance = NULL;

void SkinBrowserDlg::showSkinBrowserDlg()
{
  if (myInstance == NULL)
    myInstance = new SkinBrowserDlg();

  myInstance->show();
  myInstance->raise();
}

SkinBrowserDlg::SkinBrowserDlg(QWidget* parent)
  : QDialog(parent)
{
  Support::setWidgetProps(this, "SkinBrowserDialog");
  setWindowTitle(tr("Licq Skin Browser"));

  pmSkin = new QPixmap();
  lstIcons = new QLinkedList<QPixmap>;
  lstExtIcons = new QLinkedList<QPixmap>;
  lstEmoticons = new QLinkedList<QPixmap>;
  lstAIcons = new QStringList();
  lstAExtIcons = new QStringList();

  // Setup a list of previewable icons
  // The strings reflect what we expect to find in the icon files.
  // The result of these two lists is used to load the icons, the order of
  // this list will be the order that the icons get rendered in the preview.
  *lstAIcons << "Online" << "Offline" << "FFC" << "Away" << "NA"
                   << "Occupied" << "DND" << "Private" << "Message" << "Url"
                   << "Chat" << "File" << "SMS" << "Contact" << "Authorize" << "ReqAuthorize"
                   << "SecureOff" << "SecureOn" << "Search" << "Remove" << "History"
                   << "Info" << "AIMOnline" << "AIMOffline" << "AIMAway"
                   << "MSNOnline" << "MSNOffline" << "MSNAway"
                   << "MSNOccupied" << "MSNPrivate";

  *lstAExtIcons << "Collapsed" << "Expanded" << "Birthday" << "Cellular"
                      << "CustomAR" << "Invisible" << "Typing" << "Phone"
                      << "PhoneFollowMeActive" << "PhoneFollowMeBusy"
                      << "ICQphoneActive" << "ICQphoneBusy" << "SharedFiles";

  // Main Box
  QVBoxLayout* toplay = new QVBoxLayout(this);

  QHBoxLayout* layMain = new QHBoxLayout();
  boxSkin = new QGroupBox(tr("Skin selection"));
  QVBoxLayout* laySkin = new QVBoxLayout(boxSkin);
  boxPreview = new QGroupBox(tr("Preview"));
  QHBoxLayout* layPreview = new QHBoxLayout(boxPreview);
  layMain->addWidget(boxSkin);
  layMain->addWidget(boxPreview);

  // Skin and Icons Box
  QLabel* lblSkin = new QLabel(tr("S&kins:"));
  cmbSkin = new QComboBox();
  cmbSkin->setToolTip(tr("Use this combo box to select one of the available skins"));
  lblSkin->setBuddy(cmbSkin);
  laySkin->addWidget(lblSkin);
  laySkin->addWidget(cmbSkin);

  QLabel* lblIcon = new QLabel(tr("&Icons:"));
  cmbIcon = new QComboBox();
  cmbIcon->setToolTip(tr("Use this combo box to select one of the available icon sets"));
  lblIcon->setBuddy(cmbIcon);
  laySkin->addWidget(lblIcon);
  laySkin->addWidget(cmbIcon);

  QLabel* lblExtIcon = new QLabel(tr("E&xtended Icons:"));
  cmbExtIcon = new QComboBox();
  cmbExtIcon->setToolTip(tr("Use this combo box to select one of the available extended icon sets"));
  lblExtIcon->setBuddy(cmbExtIcon);
  laySkin->addWidget(lblExtIcon);
  laySkin->addWidget(cmbExtIcon);

  QLabel* lblEmoticons = new QLabel(tr("E&moticons:"));
  cmbEmoticon = new QComboBox();
  cmbEmoticon->setToolTip(tr("Use this combo box to select one of the available emoticon icon sets"));
    lblEmoticons->setBuddy(cmbEmoticon);
  laySkin->addWidget(lblEmoticons);
  laySkin->addWidget(cmbEmoticon);

  // Preview Box
  QVBoxLayout* layPrevSkin = new QVBoxLayout();
  QLabel* lblPrevSkin = new QLabel(tr("Skin:"));
  lblPrevSkin->setAlignment(Qt::AlignHCenter);
  lblPaintSkin = new QLabel();
  lblPaintSkin->setFixedSize(75, MAX_HEIGHT_SKIN);
  layPrevSkin->addWidget(lblPrevSkin, 0, Qt::AlignHCenter);
  layPrevSkin->addWidget(lblPaintSkin, 0, Qt::AlignHCenter);
  layPrevSkin->addStretch();
  layPreview->addLayout(layPrevSkin);

  QVBoxLayout* layPrevIcon = new QVBoxLayout();
  QLabel* lblPrevIcon = new QLabel(tr("Icons:"));
  lblPrevIcon->setAlignment(Qt::AlignHCenter);
  lblPaintIcon = new SkinBrowserPreviewArea();
  lblPaintIcon->setFixedSize(54, MAX_HEIGHT);
  layPrevIcon->addWidget(lblPrevIcon, 0, Qt::AlignHCenter);
  layPrevIcon->addWidget(lblPaintIcon, 0, Qt::AlignHCenter);
  layPrevIcon->addStretch();
  layPreview->addLayout(layPrevIcon);

  QVBoxLayout* layPrevExtIcon = new QVBoxLayout();
  QLabel* lblPrevExtIcon = new QLabel(tr("Extended Icons:"));
  lblPrevExtIcon->setAlignment(Qt::AlignHCenter);
  lblPaintExtIcon = new SkinBrowserPreviewArea();
  lblPaintExtIcon->setFixedSize(54, MAX_HEIGHT);
  layPrevExtIcon->addWidget(lblPrevExtIcon, 0, Qt::AlignHCenter);
  layPrevExtIcon->addWidget(lblPaintExtIcon, 0, Qt::AlignHCenter);
  layPrevExtIcon->addStretch();
  layPreview->addLayout(layPrevExtIcon);

  QVBoxLayout* layPrevEmoticon = new QVBoxLayout();
  QLabel* lblPrevEmoticon = new QLabel(tr("Emoticons:"));
  lblPrevEmoticon->setAlignment(Qt::AlignHCenter);
  lblPaintEmoticon= new SkinBrowserPreviewArea();
  lblPaintEmoticon->setFixedSize(54, MAX_HEIGHT);
  layPrevEmoticon->addWidget(lblPrevEmoticon, 0, Qt::AlignHCenter);
  layPrevEmoticon->addWidget(lblPaintEmoticon, 0, Qt::AlignHCenter);
  layPrevEmoticon->addStretch();
  layPreview->addLayout(layPrevEmoticon);

  // Buttons
  QHBoxLayout* layButtons = new QHBoxLayout();
  QPushButton* btnEdit = new QPushButton(tr("&Edit Skin"));
  QPushButton* btnOk = new QPushButton(tr("&Ok"));
  btnOk->setDefault(true);
  QPushButton* btnApply = new QPushButton(tr("&Apply"));
  QPushButton* btnCancel = new QPushButton(tr("&Cancel"));
  layButtons->addWidget(btnEdit);
  layButtons->addStretch();
  layButtons->addSpacing(15);
  layButtons->addWidget(btnOk);
  layButtons->addWidget(btnApply);
  layButtons->addWidget(btnCancel);

  toplay->addLayout(layMain);
  toplay->addStretch();
  toplay->addLayout(layButtons);

  // Load up the available packs
  QString szDir = QString::fromLocal8Bit(SHARE_DIR) + QTGUI_DIR;
  QString szDirUser = QString::fromLocal8Bit(BASE_DIR) + QTGUI_DIR;
  QDir dSkins(szDir + SKINS_DIR);
  QDir dSkinsUser(szDirUser + SKINS_DIR);
  dSkins.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
  dSkinsUser.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
  if (!dSkins.count() && !dSkinsUser.count())
  {
    gLog.Error("%sError reading %s directory.\n", L_ERRORxSTR, szDir.toLatin1().data());
    cmbSkin->addItem(tr("Error"));
    cmbSkin->setEnabled(false);
  }
  else
  {
    QStringList::Iterator it;
    QStringList lst = dSkins.entryList();
    QString current(Config::Skin::active()->skinName());
    for (it = lst.begin(); it != lst.end(); ++it)
    {
      cmbSkin->addItem(*it);
      if (current == *it)
        cmbSkin->setCurrentIndex(cmbSkin->count() - 1);
    }
    // check for skins in current base dir, too
    lst = dSkinsUser.entryList();
    for (it = lst.begin(); it != lst.end(); ++it)
    {
      // Check for duplicates
      int num = cmbSkin->count();
      bool dup = false;
      for (int i = 0; i < num; i++)
        if (*it == cmbSkin->itemText(i))
          dup = true;
      if (!dup)
      {
        cmbSkin->addItem(*it);
        if (current == *it)
          cmbSkin->setCurrentIndex(cmbSkin->count() - 1);
      }
    }
  }

  QDir dIcons(szDir + ICONS_DIR);
  QDir dIconsUser(szDirUser + ICONS_DIR);
  dIcons.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
  dIconsUser.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
  if (!dIcons.count() && !dIconsUser.count())
  {
    gLog.Error("%sError reading %s directory.\n", L_ERRORxSTR, szDir.toLatin1().data());
    cmbIcon->addItem(tr("Error"));
    cmbIcon->setEnabled(false);
  }
  else
  {
    QStringList::Iterator it;
    QStringList lst = dIcons.entryList();
    QString current(IconManager::instance()->iconSet());
    for (it = lst.begin(); it != lst.end(); ++it)
    {
      dIcons.cd(*it);
      QString iconsFile = QString("%1/%2.icons").arg(dIcons.path()).arg(*it);
      char sFileName[MAX_FILENAME_LEN] = "";
      CIniFile fIconsConf;
      if (!fIconsConf.LoadFile(iconsFile.toLatin1()))
      {
        WarnUser(this,
            tr("Unable to open icons file\n%1\nIconset '%2' has been disabled.")
            .arg(iconsFile)
            .arg(*it));
        dIcons.cdUp();
        continue;
      }
      fIconsConf.SetSection("icons");
      fIconsConf.ReadStr("Online", sFileName, "");
      QString pmFile = QString("%1/%2").arg(dIcons.path()).arg(sFileName);
      cmbIcon->addItem(QPixmap(pmFile), *it);
      if (current == *it)
        cmbIcon->setCurrentIndex(cmbIcon->count() - 1);
      dIcons.cdUp();
    }
    // check for icons in current base dir, too
    lst = dIconsUser.entryList();
    for (it = lst.begin(); it != lst.end(); ++it)
    {
      dIconsUser.cd(*it);
      QString iconsFile = QString("%1/%2.icons").arg(dIconsUser.path()).arg(*it);
      char sFileName[MAX_FILENAME_LEN] = "";
      CIniFile fIconsConf;
      if (!fIconsConf.LoadFile(iconsFile.toLatin1()))
      {
        WarnUser(this, tr("Unable to open icons file\n%1\nIconset '%2' has been disabled.").arg(iconsFile).arg(*it));
        dIconsUser.cdUp();
        continue;
      }
      fIconsConf.SetSection("icons");
      fIconsConf.ReadStr("Online", sFileName, "");
      QString pmFile = QString("%1/%2").arg(dIconsUser.path()).arg(sFileName);
      // Check for duplicates
      int num = cmbIcon->count();
      bool dup = false;
      for (int i = 0; i < num; i++)
      {
        if (*it == cmbIcon->itemText(i))
        {
          dup = true;
          cmbIcon->setItemIcon(i, QPixmap(pmFile));
          cmbIcon->setItemText(i, *it);
        }
      }
      if (!dup)
      {
        cmbIcon->addItem(QPixmap(pmFile), *it);
        if (current == *it)
          cmbIcon->setCurrentIndex(cmbIcon->count() - 1);
      }
      dIconsUser.cdUp();
    }
  }

  QDir dExtendedIcons(szDir + EXTICONS_DIR);
  QDir dExtendedIconsUser(szDirUser + EXTICONS_DIR);
  dExtendedIcons.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
  dExtendedIconsUser.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
  if (!dExtendedIcons.count() && !dExtendedIconsUser.count())
  {
    gLog.Error("%sError reading %s directory.\n", L_ERRORxSTR, szDir.toLatin1().data());
    cmbExtIcon->addItem(tr("Error"));
    cmbExtIcon->setEnabled(false);
  }
  else
  {
    QStringList::Iterator it;
    QStringList lst = dExtendedIcons.entryList();
    QString current(IconManager::instance()->extendedIconSet());
    for (it = lst.begin(); it != lst.end(); ++it)
    {
      dExtendedIcons.cd(*it);
      QString iconsFile = QString("%1/%2.icons").arg(dExtendedIcons.path()).arg(*it);
      char sFileName[MAX_FILENAME_LEN] = "";
      CIniFile fIconsConf;
      if (!fIconsConf.LoadFile(iconsFile.toLatin1()))
      {
        WarnUser(this,
            tr("Unable to open extended icons file\n%1\n"
              "Extended Iconset '%2' has been disabled.")
            .arg(iconsFile)
            .arg(*it));
        dExtendedIcons.cdUp();
        continue;
      }
      fIconsConf.SetSection("icons");
      fIconsConf.ReadStr("Phone", sFileName, "");
      QString pmFile = QString("%1/%2").arg(dExtendedIcons.path()).arg(sFileName);
      cmbExtIcon->addItem(QPixmap(pmFile), *it);
      if (current == *it)
        cmbExtIcon->setCurrentIndex(cmbExtIcon->count() - 1);
      dExtendedIcons.cdUp();
    }
    // check for ext. icons in current base dir, too
    lst = dExtendedIconsUser.entryList();
    for (it = lst.begin(); it != lst.end(); ++it)
    {
      dExtendedIconsUser.cd(*it);
      QString iconsFile = QString("%1/%2.icons").arg(dExtendedIconsUser.path()).arg(*it);
      char sFileName[MAX_FILENAME_LEN] = "";
      CIniFile fIconsConf;
      if (!fIconsConf.LoadFile(iconsFile.toLatin1()))
      {
        WarnUser(this,
            tr("Unable to open extended icons file\n%1\n"
              "Extended Iconset '%2' has been disabled.")
            .arg(iconsFile)
            .arg(*it));
        dExtendedIconsUser.cdUp();
        continue;
      }
      fIconsConf.SetSection("icons");
      fIconsConf.ReadStr("Phone", sFileName, "");
      QString pmFile = QString("%1/%2").arg(dExtendedIconsUser.path()).arg(sFileName);
      // Check for duplicates
      int num = cmbExtIcon->count();
      bool dup = false;
      for (int i=0; i < num; i++)
      {
        if (*it == cmbExtIcon->itemText(i))
        {
          dup = true;
          cmbExtIcon->setItemIcon(i, QPixmap(pmFile));
          cmbExtIcon->setItemText(i, *it);
        }
      }
      if (!dup)
      {
        cmbExtIcon->addItem(QPixmap(pmFile), *it);
        if (current == *it)
          cmbExtIcon->setCurrentIndex(cmbExtIcon->count() - 1);
      }
      dExtendedIconsUser.cdUp();
    }
  }

  const Emoticons* emoticons = Emoticons::self();
  const QStringList themes = emoticons->themes();
  cmbEmoticon->insertItems(0, themes);

  const QString selected = emoticons->theme();
  const int index = themes.indexOf(selected);
  if (index != -1)
    cmbEmoticon->setCurrentIndex(index);

  // setup connections
  connect(btnEdit, SIGNAL(clicked()), SLOT(slot_edtSkin()));
  connect(btnOk, SIGNAL(clicked()), SLOT(slot_ok()));
  connect(btnApply, SIGNAL(clicked()), SLOT(slot_apply()));
  connect(btnCancel, SIGNAL(clicked()), SLOT(close()));
  connect(cmbSkin, SIGNAL(currentIndexChanged(const QString&)),
      SLOT(slot_loadSkin(const QString&)));
  connect(cmbSkin, SIGNAL(highlighted(const QString&)),
      SLOT(slot_loadSkin(const QString&)));
  connect(cmbIcon, SIGNAL(currentIndexChanged(const QString&)),
      SLOT(slot_loadIcons(const QString&)));
  connect(cmbIcon, SIGNAL(highlighted(const QString&)),
      SLOT(slot_loadIcons(const QString&)));
  connect(cmbExtIcon, SIGNAL(currentIndexChanged(const QString&)),
      SLOT(slot_loadExtIcons(const QString&)));
  connect(cmbExtIcon, SIGNAL(highlighted(const QString&)),
      SLOT(slot_loadExtIcons(const QString&)));
  connect(cmbEmoticon, SIGNAL(currentIndexChanged(const QString&)),
      SLOT(slot_loadEmoticons(const QString&)));
  connect(cmbEmoticon, SIGNAL(highlighted(const QString&)),
      SLOT(slot_loadEmoticons(const QString&)));

  // Create initial preview
  slot_loadSkin(cmbSkin->currentText());
  slot_loadIcons(cmbIcon->currentText());
  slot_loadExtIcons(cmbExtIcon->currentText());
  slot_loadEmoticons(cmbEmoticon->currentText());
}

SkinBrowserDlg::~SkinBrowserDlg()
{
  delete pmSkin;
  delete lstIcons;
  delete lstExtIcons;
  delete lstAIcons;
  delete lstAExtIcons;

  myInstance = NULL;
}

/*! \brief Applies skin/iconsets and closes the dialog
 *
 *  slot_ok() applies all selected options that differ from the currently
 *  activated settings. Afterwards it saves the new skin and iconset
 *  settings to the config file and closes the window.
 */
void SkinBrowserDlg::slot_ok()
{
  slot_apply();
  LicqGui::instance()->saveConfig();
  close();
}

/*! \brief Applies settings
 *
 *  Applies selected skin/icons/emoticons.
 */
void SkinBrowserDlg::slot_apply()
{
  IconManager* iconManager = IconManager::instance();

  Config::Skin::active()->loadSkin(cmbSkin->currentText().toLocal8Bit());

  if (!iconManager->loadIcons(cmbIcon->currentText()))
    WarnUser(this, tr("Unable to load icons\n%1.")
        .arg(cmbIcon->currentText().toLocal8Bit().data()));

  if (!iconManager->loadExtendedIcons(cmbExtIcon->currentText()))
    WarnUser(this, tr("Unable to load extended icons\n%1.")
        .arg(cmbExtIcon->currentText().toLocal8Bit().data()));

  Emoticons::self()->setTheme(cmbEmoticon->currentText());
}

/*! \brief Creates a new skin editor dialog
 *
 *  Creates a new Dialog which enables the user to edit the currently selected
 *  skin.
 */
void SkinBrowserDlg::slot_edtSkin()
{
  if (cmbSkin->currentText().isEmpty()) return;
  QString f;
  f.sprintf("%s%s%s%s/%s.skin", BASE_DIR, QTGUI_DIR, SKINS_DIR,
            QFile::encodeName(cmbSkin->currentText()).data(),
            QFile::encodeName(cmbSkin->currentText()).data());
  if (!QFile(f).exists())
    f.sprintf("%s%s%s%s/%s.skin", SHARE_DIR, QTGUI_DIR, SKINS_DIR,
              QFile::encodeName(cmbSkin->currentText()).data(),
              QFile::encodeName(cmbSkin->currentText()).data());
  new EditFileDlg(f);
}

/*! \brief Refreshes the skin preview
 *
 *  This slot reloads the skin preview pixmap dynamically. It
 *  loads the currently highlighted skin in the combobox. If
 *  it was successful it sets the new skin-pixmap as preview pixmap.
 */
void SkinBrowserDlg::slot_loadSkin(const QString& skin)
{
  lblPaintSkin->setPixmap(renderSkin(skin));
}

/*! \brief Reloads the current preview icons
 *
 *  This slot reloads all preview icons. It loads the complete
 *  set of icons that is currently highlighted in the relevant combo box.
 *  If it was successful it makes these icons to be rendered in the preview.
 */
void SkinBrowserDlg::slot_loadIcons(const QString& icon)
{
  // force a sane state and then load all icons into the valuelist
  lstIcons->clear();
  QString iconsFile = QString::fromLocal8Bit(BASE_DIR) + QTGUI_DIR + ICONS_DIR +
    icon + "/" + icon + ".icons";
  char sFileName[MAX_FILENAME_LEN] = "";
  CIniFile fIconsConf;
  if (!fIconsConf.LoadFile(iconsFile.toLatin1()))
  {
    iconsFile = QString::fromLocal8Bit(SHARE_DIR) + QTGUI_DIR + ICONS_DIR +
      icon + "/" + icon + ".icons";
    if (!fIconsConf.LoadFile(iconsFile.toLatin1()))
    {
      WarnUser(this, tr("Unable to open icons file\n%1").arg(iconsFile));
      return;
    }
  }
  fIconsConf.SetSection("icons");
  for (QStringList::Iterator it = lstAIcons->begin(); it != lstAIcons->end(); ++it)
  {
    fIconsConf.ReadStr((*it).toAscii().data(), sFileName, "");
    QString pmFile = iconsFile.left(iconsFile.length()-icon.length()-6) + QString::fromAscii(sFileName);
    QPixmap pm(pmFile);
    if (! pm.isNull())
      lstIcons->append(pm);
  }
  lblPaintIcon->setPixmapList(lstIcons);
}

/*! \brief Reloads the current preview extended icons
 *
 *  This slot reloads all preview icons. It loads the complete
 *  set of extended icons that is currently highlighted in the relevant combo box.
 *  If it was successful it makes these icons to be rendered in the preview.
 */
void SkinBrowserDlg::slot_loadExtIcons(const QString& extIcon)
{
  // force a sane state and then load all icons into the valuelist
  lstExtIcons->clear();
  QString iconsFile = QString::fromLocal8Bit(BASE_DIR) + QTGUI_DIR +
    EXTICONS_DIR + extIcon + "/" + extIcon + ".icons";
  char sFileName[MAX_FILENAME_LEN] = "";
  CIniFile fIconsConf;
  if (!fIconsConf.LoadFile(iconsFile.toLatin1()))
  {
    iconsFile = QString::fromLocal8Bit(SHARE_DIR) + QTGUI_DIR +
      EXTICONS_DIR + extIcon + "/" + extIcon + ".icons";
    if (!fIconsConf.LoadFile(iconsFile.toLatin1()))
    {
      WarnUser(this, tr("Unable to open extended icons file\n%1").arg(iconsFile));
      return;
    }
  }
  fIconsConf.SetSection("icons");
  for (QStringList::Iterator it = lstAExtIcons->begin(); it != lstAExtIcons->end(); ++it)
  {
    fIconsConf.ReadStr((*it).toAscii().data(), sFileName, "");
    QString pmFile = iconsFile.left(iconsFile.length()-extIcon.length()-6) + QString::fromAscii(sFileName);
    QPixmap pm(pmFile);
    if (! pm.isNull())
      lstExtIcons->append(pm);
  }
  lblPaintExtIcon->setPixmapList(lstExtIcons);
}
/*! \brief Reloads the current preview emoticons
 *
 *  This slot reloads all preview emoicons. It loads the complete
 *  set of emoticons that is currently highlighted in the relevant combo
 *  box.
 *  If it was successful it makes these icons to be rendered in the preview.
 */
void SkinBrowserDlg::slot_loadEmoticons(const QString& emoticon)
{
  lstEmoticons->clear();
  const QStringList files = Emoticons::self()->fileList(emoticon);
  for (QStringList::ConstIterator it = files.begin(); it != files.end(); ++it)
  {
    QImage img = QImage(*it);
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
      lstEmoticons->append(pm);
  }
  lblPaintEmoticon->setPixmapList(lstEmoticons);
}

/*! \brief provide correct repainting when resizing the main widget
 *
 *  This slot is called everytime the mainwidget gets resized. It forces the
 *  preview areas to be updated so that it looks nice with structured
 *  backgrounds.
 */
void SkinBrowserDlg::resizeEvent(QResizeEvent* /* e */)
{
  lblPaintSkin->update();
  lblPaintIcon->update();
  lblPaintExtIcon->update();
}

// Helper Class to provide correct repainting
// of the preview widget
SkinBrowserPreviewArea::SkinBrowserPreviewArea(QWidget* parent)
  : QFrame(parent)
{
  setObjectName("SkinBrowserPreviewArea");
  lstPm.clear();
}

/*! \brief Sets the pixmap set for preview
 *
 *  You have to call this slot if you want to set or update the iconset
 *  that is drawn on the widget. It updates the widget itself, no manual update
 *  is necessary.
 */
void SkinBrowserPreviewArea::setPixmapList(QLinkedList<QPixmap>* _lstPm)
{
  lstPm = *_lstPm;
  this->update();
}

void SkinBrowserPreviewArea::paintEvent(QPaintEvent* /* e */)
{
  QPainter p(this);
  unsigned short int X = 0;
  unsigned short int Y = 0;
  for (QLinkedList<QPixmap>::Iterator it = lstPm.begin(); it != lstPm.end(); ++it)
  {
    p.drawPixmap(X, Y, (*it), 0, 0, 16, 16);
    // determine next drawing position (16px + 3px space, so we need 19pixels space)
    X = ((X+19) <= (this->width()-16)) ? (X+19) : 0;
    Y = (X==0) ? (Y+19) : Y;
  }
  p.end();
}

/*  \brief Renders the dynamic skin preview
 *
 *  This method renders a skin preview in realtime. This is accomplished
 *  by creating a new Widget and applying the Skin &skinName to it. The
 *  widget is never shown, but instead using grabWidget() it is copied
 *  into a pixmap, which afterwards is returned to the caller.
 */
QPixmap SkinBrowserDlg::renderSkin(const QString& skinName)
{
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
  else
  {
    setPalette(QPalette());
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

  userView.setPalette(skin->palette(this));
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
