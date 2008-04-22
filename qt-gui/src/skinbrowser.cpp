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

#include <qdir.h>
#include <qframe.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qpixmap.h>
#include <qtoolbutton.h>
#include <qvaluelist.h>
#include <qwhatsthis.h>

#include "editfile.h"
#include "ewidgets.h"
#include "gui-defines.h"
#include "licq_log.h"
#include "licq_constants.h"
#include "mainwin.h"
#include "skin.h"
#include "skinbrowser.h"
#include "gui-strings.h"
#include "emoticon.h"

enum {
  MAX_HEIGHT = 190, /* Height of icon preview widget, this is 
                       (number-of-Icons) / 3 * 19. 
                       if the result isn't a multiple of 19, 
                       round it up to become a multiple of 19 */
  MAX_HEIGHT_SKIN = 130
};

SkinBrowserDlg::SkinBrowserDlg(CMainWindow *_mainwin, QWidget *parent)
	: LicqDialog(parent, "SkinBrowserDialog")
{
	mainwin = _mainwin;
	pmSkin = new QPixmap();
	lstIcons = new QValueList<QPixmap>;
	lstExtIcons = new QValueList<QPixmap>;
	lstEmoticons = new QValueList<QPixmap>;
	lstAIcons = new QStringList();
	lstAExtIcons = new QStringList();

	// Setup a list of previewable icons
	// The strings reflect what we exptect to find in the *.icons files.
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
	QVBoxLayout *toplay = new QVBoxLayout(this);
	QFrame *frmMain = new QFrame(this);
	QFrame *frmButtons = new QFrame(this);
	toplay->addWidget(frmMain);
	toplay->addStretch();
	toplay->addWidget(frmButtons);

	QHBoxLayout *layMain = new QHBoxLayout(frmMain, 8, 4);
	boxSkin = new QGroupBox(1, QGroupBox::Horizontal, tr("Skin selection"), frmMain);
	boxPreview = new QGroupBox(1, QGroupBox::Vertical, tr("Preview"), frmMain);
	layMain->addWidget(boxSkin);
	layMain->addWidget(boxPreview);

	// Skin and Icons Box
	QLabel *lblSkin = new QLabel(tr("S&kins:"), boxSkin);
	cmbSkin = new QComboBox(boxSkin);
	QWhatsThis::add(cmbSkin, tr("Use this combo box to select one of the available skins"));
	lblSkin->setBuddy(cmbSkin);

	QLabel *lblIcon = new QLabel(tr("&Icons:"), boxSkin);
	cmbIcon = new QComboBox(boxSkin);
	QWhatsThis::add(cmbIcon, tr("Use this combo box to select one of the available icon sets"));
	lblIcon->setBuddy(cmbIcon);

	QLabel *lblExtIcon = new QLabel(tr("E&xtended Icons:"), boxSkin);
	cmbExtIcon = new QComboBox(boxSkin);
	QWhatsThis::add(cmbExtIcon, tr("Use this combo box to select one of the available extended icon sets"));
	lblExtIcon->setBuddy(cmbExtIcon);

	QLabel *lblEmoticons = new QLabel(tr("E&moticons:"), boxSkin);
	cmbEmoticon = new QComboBox(boxSkin);
	QWhatsThis::add(cmbEmoticon, tr("Use this combo box to select one of "
	                                "the available emoticon icon sets"));
  	lblEmoticons->setBuddy(cmbEmoticon);

	// Preview Box
	QFrame *frmPrevSkin = new QFrame(boxPreview);
	QVBoxLayout *layPrevSkin = new QVBoxLayout(frmPrevSkin);
	QLabel *lblPrevSkin = new QLabel(tr("Skin:"), frmPrevSkin);
	lblPrevSkin->setAlignment(Qt::AlignHCenter);
	lblPaintSkin = new QLabel(frmPrevSkin);
	lblPaintSkin->setFixedSize(75, MAX_HEIGHT_SKIN);
	layPrevSkin->addWidget(lblPrevSkin, 0, Qt::AlignHCenter);
	layPrevSkin->addWidget(lblPaintSkin, 0, Qt::AlignHCenter);
	layPrevSkin->addStretch();

	QFrame *frmPrevIcon = new QFrame(boxPreview);
	QVBoxLayout *layPrevIcon = new QVBoxLayout(frmPrevIcon);
	QLabel *lblPrevIcon = new QLabel(tr("Icons:"), frmPrevIcon);
	lblPrevIcon->setAlignment(Qt::AlignHCenter);
	lblPaintIcon = new SkinBrowserPreviewArea(frmPrevIcon);
	lblPaintIcon->setFixedSize(54, MAX_HEIGHT);
	layPrevIcon->addWidget(lblPrevIcon, 0, Qt::AlignHCenter);
	layPrevIcon->addWidget(lblPaintIcon, 0, Qt::AlignHCenter);
	layPrevIcon->addStretch();

	QFrame *frmPrevExtIcon = new QFrame(boxPreview);
	QVBoxLayout *layPrevExtIcon = new QVBoxLayout(frmPrevExtIcon);
	QLabel *lblPrevExtIcon = new QLabel(tr("Extended Icons:"), frmPrevExtIcon);
	lblPrevExtIcon->setAlignment(Qt::AlignHCenter);
	lblPaintExtIcon = new SkinBrowserPreviewArea(frmPrevExtIcon);
	lblPaintExtIcon->setFixedSize(54, MAX_HEIGHT);
	layPrevExtIcon->addWidget(lblPrevExtIcon, 0, Qt::AlignHCenter);
	layPrevExtIcon->addWidget(lblPaintExtIcon, 0, Qt::AlignHCenter);
	layPrevExtIcon->addStretch();

	QFrame *frmPrevEmoticon= new QFrame(boxPreview);
	QVBoxLayout *layPrevEmoticon = new QVBoxLayout(frmPrevEmoticon);
	QLabel *lblPrevEmoticon = new QLabel(tr("Emoticons:"), frmPrevEmoticon);
	lblPrevEmoticon->setAlignment(Qt::AlignHCenter);
	lblPaintEmoticon= new SkinBrowserPreviewArea(frmPrevEmoticon);
	lblPaintEmoticon->setFixedSize(54, MAX_HEIGHT);
	layPrevEmoticon->addWidget(lblPrevEmoticon, 0, Qt::AlignHCenter);
	layPrevEmoticon->addWidget(lblPaintEmoticon, 0, Qt::AlignHCenter);
	layPrevEmoticon->addStretch();

	// Buttons
	QHBoxLayout *layButtons = new QHBoxLayout(frmButtons, 8, 4);
	QPushButton *btnEdit = new QPushButton(tr("&Edit Skin"), frmButtons);
	QPushButton *btnOk = new QPushButton(tr("&Ok"), frmButtons);
	btnOk->setDefault(true);
	QPushButton *btnApply = new QPushButton(tr("&Apply"), frmButtons);
	QPushButton *btnCancel = new QPushButton(tr("&Cancel"), frmButtons);
	layButtons->addWidget(QWhatsThis::whatsThisButton(frmButtons), 0, AlignLeft);
	layButtons->addWidget(btnEdit);
	layButtons->addStretch();
	layButtons->addSpacing(15);
	layButtons->addWidget(btnOk);
	layButtons->addWidget(btnApply);
	layButtons->addWidget(btnCancel);

	// Load up the available packs
	QString szDir, szDirUser;
	szDir.sprintf("%s%s", SHARE_DIR, QTGUI_DIR);
	szDirUser.sprintf("%s/%s", BASE_DIR, QTGUI_DIR);
	QDir dSkins(szDir, "skin.*", QDir::Name | QDir::IgnoreCase, QDir::Dirs);
	QDir dSkinsUser(szDirUser, "skin.*", QDir::Name | QDir::IgnoreCase, QDir::Dirs);
	if (!dSkins.count() && !dSkinsUser.count())
	{
		gLog.Error("%sError reading " PLUGIN_NAME " directory %s.\n", L_ERRORxSTR, szDir.latin1());
		cmbSkin->insertItem(tr("Error"));
		cmbSkin->setEnabled(false);
	}
	else
	{
		QStringList::Iterator it;
		QStringList lst = dSkins.entryList();
		QString current(mainwin->skin->szSkinName);
		for (it = lst.begin(); it != lst.end(); ++it)
		{
			cmbSkin->insertItem((*it).mid(5));
			if (current == (*it).mid(5))
				cmbSkin->setCurrentItem(cmbSkin->count() - 1);
		}
		// check for skins in current base dir, too
		lst = dSkinsUser.entryList();
		for (it = lst.begin(); it != lst.end(); ++it)
		{
			// Check for duplicates
			int num = cmbSkin->count();
			bool dup = false;
			for (int i=0; i < num; i++)
				if ((*it).mid(5) == cmbSkin->text(i)) dup = true;
			if (!dup)
			{
				cmbSkin->insertItem((*it).mid(5));
				if (current == (*it).mid(5))
					cmbSkin->setCurrentItem(cmbSkin->count() - 1);
			}
		}
	}

	QDir dIcons(szDir, "icons.*", QDir::Name | QDir::IgnoreCase, QDir::Dirs);
	QDir dIconsUser(szDirUser, "icons.*", QDir::Name | QDir::IgnoreCase, QDir::Dirs);
	if (!dIcons.count() && !dIconsUser.count())
	{
		gLog.Error("%sError reading " PLUGIN_NAME " directory %s.\n", L_ERRORxSTR, szDir.latin1());
		cmbIcon->insertItem(tr("Error"));
		cmbIcon->setEnabled(false);
	}
	else
	{
		QStringList::Iterator it;
		QStringList lst = dIcons.entryList();
		QString current(mainwin->m_szIconSet);
		for (it = lst.begin(); it != lst.end(); ++it)
		{
			QString iconsFile = QString("%1%2icons.%3/%4.icons").arg(SHARE_DIR).arg(QTGUI_DIR).arg((*it).mid(6)).arg((*it).mid(6));
			char sFileName[MAX_FILENAME_LEN] = "";
			CIniFile fIconsConf;
			if (!fIconsConf.LoadFile(iconsFile))
			{
				WarnUser(this, tr("Unable to open icons file\n%1\nIconset '%2' has been disabled.").arg(iconsFile).arg((*it).mid(6)));
				continue;
			}
			fIconsConf.SetSection("icons");
			fIconsConf.ReadStr("Online", sFileName, "");
			QString pmFile = QString("%1%2icons.%3/%4").arg(SHARE_DIR).arg(QTGUI_DIR).arg((*it).mid(6)).arg(sFileName);
			cmbIcon->insertItem(QPixmap(pmFile), (*it).mid(6));
			if (current == (*it).mid(6))
				cmbIcon->setCurrentItem(cmbIcon->count() - 1);
		}
		// check for icons in current base dir, too
		lst = dIconsUser.entryList();
		for (it = lst.begin(); it != lst.end(); ++it)
		{
			QString iconsFile = QString("%1/%2icons.%3/%4.icons").arg(BASE_DIR).arg(QTGUI_DIR).arg((*it).mid(6)).arg((*it).mid(6));
			char sFileName[MAX_FILENAME_LEN] = "";
			CIniFile fIconsConf;
			if (!fIconsConf.LoadFile(iconsFile))
			{
				WarnUser(this, tr("Unable to open icons file\n%1\nIconset '%2' has been disabled.").arg(iconsFile).arg((*it).mid(6)));
				continue;
			}
			fIconsConf.SetSection("icons");
			fIconsConf.ReadStr("Online", sFileName, "");
			QString pmFile = QString("%1/%2icons.%3/%4").arg(BASE_DIR).arg(QTGUI_DIR).arg((*it).mid(6)).arg(sFileName);
			// Check for duplicates
			int num = cmbIcon->count();
			bool dup = false;
			for (int i=0; i < num; i++)
			{
				if ((*it).mid(6) == cmbIcon->text(i))
				{
					dup = true;
					cmbIcon->changeItem(QPixmap(pmFile), (*it).mid(6), i);
				}
			}
			if (!dup)
			{
				cmbIcon->insertItem(QPixmap(pmFile), (*it).mid(6));
				if (current == (*it).mid(6))
					cmbIcon->setCurrentItem(cmbIcon->count() - 1);
			}
		}
	}

	QDir dExtendedIcons(szDir, "extended.icons.*", QDir::Name | QDir::IgnoreCase, QDir::Dirs);
	QDir dExtendedIconsUser(szDirUser, "extended.icons.*", QDir::Name | QDir::IgnoreCase, QDir::Dirs);
	if (!dExtendedIcons.count() && !dExtendedIconsUser.count())
	{
		gLog.Error("%sError reading " PLUGIN_NAME " directory %s.\n", L_ERRORxSTR, szDir.latin1());
		cmbExtIcon->insertItem(tr("Error"));
		cmbExtIcon->setEnabled(false);
	}
	else
	{
		QStringList::Iterator it;
		QStringList lst = dExtendedIcons.entryList();
		QString current(mainwin->m_szExtendedIconSet);
		for (it = lst.begin(); it != lst.end(); ++it)
		{
			QString iconsFile = QString("%1%2extended.icons.%3/%4.icons").arg(SHARE_DIR).arg(QTGUI_DIR).arg((*it).mid(15)).arg((*it).mid(15));
			char sFileName[MAX_FILENAME_LEN] = "";
			CIniFile fIconsConf;
			if (!fIconsConf.LoadFile(iconsFile))
			{
				WarnUser(this, tr("Unable to open extended icons file\n%1\nExtended Iconset '%2' has been disabled.").arg(iconsFile).arg((*it).mid(15)));
				continue;
			}
			fIconsConf.SetSection("icons");
			fIconsConf.ReadStr("Phone", sFileName, "");
			QString pmFile = QString("%1%2extended.icons.%3/%4").arg(SHARE_DIR).arg(QTGUI_DIR).arg((*it).mid(15)).arg(sFileName);
			cmbExtIcon->insertItem(QPixmap(pmFile), (*it).mid(15));
			if (current == (*it).mid(15))
				cmbExtIcon->setCurrentItem(cmbExtIcon->count() - 1);
		}
		// check for ext. icons in current base dir, too
		lst = dExtendedIconsUser.entryList();
		for (it = lst.begin(); it != lst.end(); ++it)
		{
			QString iconsFile = QString("%1/%2extended.icons.%3/%4.icons").arg(BASE_DIR).arg(QTGUI_DIR).arg((*it).mid(15)).arg((*it).mid(15));
			char sFileName[MAX_FILENAME_LEN] = "";
			CIniFile fIconsConf;
			if (!fIconsConf.LoadFile(iconsFile))
			{
				WarnUser(this, tr("Unable to open extended icons file\n%1\nExtended Iconset '%2' has been disabled.").arg(iconsFile).arg((*it).mid(15)));
				continue;
			}
			fIconsConf.SetSection("icons");
			fIconsConf.ReadStr("Phone", sFileName, "");
			QString pmFile = QString("%1/%2extended.icons.%3/%4").arg(BASE_DIR).arg(QTGUI_DIR).arg((*it).mid(15)).arg(sFileName);
			// Check for duplicates
			int num = cmbExtIcon->count();
			bool dup = false;
			for (int i=0; i < num; i++)
			{
				if ((*it).mid(15) == cmbExtIcon->text(i))
				{
					dup = true;
					cmbExtIcon->changeItem(QPixmap(pmFile), (*it).mid(15), i);
				}
			}
			if (!dup)
			{
				cmbExtIcon->insertItem(QPixmap(pmFile), (*it).mid(15));
				if (current == (*it).mid(15))
					cmbExtIcon->setCurrentItem(cmbExtIcon->count() - 1);
			}
		}
	}

  const CEmoticons *emoticons = CEmoticons::self();
  const QStringList themes = emoticons->themes();
  cmbEmoticon->insertStringList(themes, 0);

  const QString selected = emoticons->theme();
  const int index = themes.findIndex(selected);
  if (index != -1)
    cmbEmoticon->setCurrentItem(index);

	// setup connections
	connect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_edtSkin()));
	connect(btnOk, SIGNAL(clicked()), this, SLOT(slot_ok()));
	connect(btnApply, SIGNAL(clicked()), this, SLOT(slot_apply()));
	connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));
	connect(cmbSkin, SIGNAL(highlighted(const QString &)), this, SLOT(slot_loadSkin(const QString &)));
	connect(cmbIcon, SIGNAL(highlighted(const QString &)), this, SLOT(slot_loadIcons(const QString &)));
	connect(cmbExtIcon, SIGNAL(highlighted(const QString &)), this, SLOT(slot_loadExtIcons(const QString &)));
	connect(cmbEmoticon, SIGNAL(highlighted(const QString&)), this, SLOT(slot_loadEmoticons(const QString &)));

	// Create initial preview
	slot_loadSkin(cmbSkin->currentText());
	slot_loadIcons(cmbIcon->currentText());
	slot_loadExtIcons(cmbExtIcon->currentText());
	slot_loadEmoticons(cmbEmoticon->currentText());

	setCaption(tr("Licq Skin Browser"));
	show();
}

SkinBrowserDlg::~SkinBrowserDlg()
{
	delete pmSkin;
	delete lstIcons;
	delete lstExtIcons;
	delete lstAIcons;
	delete lstAExtIcons;
}

/*!	\brief Applies skin/iconsets and closes the dialog
 *
 *	slot_ok() applies all selected options that differ from the currently
 *	activated settings. Afterwards it saves the new skin and iconset
 *	settings to the config file and closes the window.
 */
void SkinBrowserDlg::slot_ok()
{
	slot_apply();
	mainwin->saveOptions();
	close();
}

/*! \brief Applies Skin/iconset
 *
 *	slot_apply() applies all selected options that differ from the currently
 *	activated settings.
 */
void SkinBrowserDlg::slot_apply()
{
  if (cmbSkin->currentText() != mainwin->skin->szSkinName)
    mainwin->ApplySkin(cmbSkin->currentText().local8Bit());

  if (cmbIcon->currentText() != mainwin->m_szIconSet)
    mainwin->ApplyIcons(cmbIcon->currentText().local8Bit());

  if (cmbExtIcon->currentText() != mainwin->m_szExtendedIconSet)
    mainwin->ApplyExtendedIcons(cmbExtIcon->currentText().local8Bit());

  if (cmbEmoticon->currentText() != CEmoticons::self()->theme())
    CEmoticons::self()->setTheme(cmbEmoticon->currentText());
}

/*!	\brief Creates a new skin editor dialog
 *
 *	Creates a new Dialog which enables the user to edit the currently selected
 *	skin.
 */
void SkinBrowserDlg::slot_edtSkin()
{
  if (!cmbSkin->currentText()) return;
  QString skinName = QFile::encodeName(cmbSkin->currentText());
  QString f = QTGUI_DIR "skin." + skinName + "/" + skinName + ".skin";
  QString baseF = QString(BASE_DIR) + "/" + f;
  if (QFile(baseF).exists())
    f = baseF;
  else
    f = SHARE_DIR + f;
  (void) new EditFileDlg(f);
}

/*!	\brief Refreshes the skin preview
 *
 *	This slot reloads the skin preview pixmap dynamically. It
 *	loads the currently highlighted skin in the combobox. If
 *	it was successful it sets the new skin-pixmap as preview pixmap.
 */
void SkinBrowserDlg::slot_loadSkin(const QString &skin)
{
	lblPaintSkin->setPixmap(renderSkin(skin));
}

/*!	\brief Reloads the current preview icons
 *
 *	This slot reloads all preview icons. It loads the complete
 *	set of icons that is currently highlighted in the relevant combo box.
 *	If it was successful it makes these icons to be rendered in the preview.
 */
void SkinBrowserDlg::slot_loadIcons(const QString &icon)
{
	// force a sane state and then load all icons into the valuelist
	lstIcons->clear();
	QString iconsFile = QString("%1/%2icons.%3/%4.icons").arg(BASE_DIR).arg(QTGUI_DIR).arg(icon).arg(icon);
	char sFileName[MAX_FILENAME_LEN] = "";
	CIniFile fIconsConf;
	if (!fIconsConf.LoadFile(iconsFile))
	{
		iconsFile = QString("%1%2icons.%3/%4.icons").arg(SHARE_DIR).arg(QTGUI_DIR).arg(icon).arg(icon);
		if (!fIconsConf.LoadFile(iconsFile))
		{
			WarnUser(this, tr("Unable to open icons file\n%1").arg(iconsFile));
			return;
		}
	}
	fIconsConf.SetSection("icons");
	for (QStringList::Iterator it = lstAIcons->begin(); it != lstAIcons->end(); ++it)
	{
		fIconsConf.ReadStr((*it).ascii(), sFileName, "");
		QString pmFile = iconsFile.left(iconsFile.length()-icon.length()-6) + sFileName;
		QPixmap pm(pmFile);
		if (! pm.isNull())
			lstIcons->append(pm);
	}
	lblPaintIcon->setPixmapList(lstIcons);
}

/*!	\brief Reloads the current preview extended icons
 *
 *	This slot reloads all preview icons. It loads the complete
 *	set of extended icons that is currently highlighted in the relevant combo box.
 *	If it was successful it makes these icons to be rendered in the preview.
 */
void SkinBrowserDlg::slot_loadExtIcons(const QString &extIcon)
{
	// force a sane state and then load all icons into the valuelist
	lstExtIcons->clear();
	QString iconsFile = QString("%1/%2extended.icons.%3/%4.icons").arg(BASE_DIR).arg(QTGUI_DIR).arg(extIcon).arg(extIcon);
	char sFileName[MAX_FILENAME_LEN] = "";
	CIniFile fIconsConf;
	if (!fIconsConf.LoadFile(iconsFile))
	{
		iconsFile = QString("%1%2extended.icons.%3/%4.icons").arg(SHARE_DIR).arg(QTGUI_DIR).arg(extIcon).arg(extIcon);
		if (!fIconsConf.LoadFile(iconsFile))
		{
			WarnUser(this, tr("Unable to open extended icons file\n%1").arg(iconsFile));
			return;
		}
	}
	fIconsConf.SetSection("icons");
	for (QStringList::Iterator it = lstAExtIcons->begin(); it != lstAExtIcons->end(); ++it)
	{
		fIconsConf.ReadStr((*it).ascii(), sFileName, "");
		QString pmFile = iconsFile.left(iconsFile.length()-extIcon.length()-6) + sFileName;
		QPixmap pm(pmFile);
		if (! pm.isNull())
			lstExtIcons->append(pm);
	}
	lblPaintExtIcon->setPixmapList(lstExtIcons);
}
/*! \brief Reloads the current preview emoticons
 *
 *	This slot reloads all preview emoicons. It loads the complete
 *	set of emoticons that is currently highlighted in the relevant combo
 *	box.
 *	If it was successful it makes these icons to be rendered in the preview.
 */
void SkinBrowserDlg::slot_loadEmoticons(const QString &emoticon)
{
  lstEmoticons->clear();
  const QStringList files = CEmoticons::self()->fileList(emoticon);
  for (QStringList::ConstIterator it = files.begin(); it != files.end(); ++it)
  {
    QImage img = QImage(*it);
    // hack: SkinBrowserPreviewArea only draws the first 16 pixels
    const int max_area = 16;
    QSize size = img.size();
    if (size.isValid() && size.width() > max_area && size.height() > max_area)
      img = img.scale(max_area, max_area, QImage::ScaleFree);

    QPixmap pm(img);
    if (!pm.isNull())
      lstEmoticons->append(pm);
  }
  lblPaintEmoticon->setPixmapList(lstEmoticons);
}

/*! \brief provide correct repainting when resizing the main widget
 *
 *	This slot is called everytime the mainwidget gets resized. It forces the
 *	preview areas to be updated so that it looks nice with structured
 *	backgrounds.
 */
void SkinBrowserDlg::resizeEvent(QResizeEvent* /* e */)
{
	lblPaintSkin->update();
	lblPaintIcon->update();
	lblPaintExtIcon->update();
}

// Helper Class to provide correct repainting
// of the preview widget
SkinBrowserPreviewArea::SkinBrowserPreviewArea(QWidget *parent)
	: QFrame(parent, "SkinBrowserPreviewArea")
{
	lstPm.clear();
}

/*! \brief Sets the pixmap set for preview
 *
 *	You have to call this slot if you want to set or update the iconset
 *	that is drawn on the widget. It updates the widget itself, no manual update
 *	is necessary.
 */
void SkinBrowserPreviewArea::setPixmapList(QValueList<QPixmap> *_lstPm)
{
	lstPm = *_lstPm;
	this->update();
}

void SkinBrowserPreviewArea::paintEvent(QPaintEvent* /* e */)
{
	QPainter p(this);
	unsigned short int X = 0;
	unsigned short int Y = 0;
	for (QValueList<QPixmap>::Iterator it = lstPm.begin(); it != lstPm.end(); ++it)
	{
		p.drawPixmap(X, Y, (*it), 0, 0, 16, 16);
		// determine next drawing position (16px + 3px space, so we need 19pixels space)
		X = ((X+19) <= (this->width()-16)) ? (X+19) : 0;
		Y = (X==0) ? (Y+19) : Y;
	}
	p.end();
}

/*	\brief Renders the dynamic skin preview
 *
 *	This method renders a skin preview in realtime. This is accomplished
 *	by creating a new Widget and applying the Skin &skinName to it. The
 *	widget is never shown, but instead using grabWidget() it is copied
 *	into a pixmap, which afterwards is returned to the caller.
 */
QPixmap SkinBrowserDlg::renderSkin(const QString &skinName)
{
	CEButton *btnSystem = NULL;
	CELabel *lblMsg = NULL;
	CELabel *lblStatus = NULL;
	CSkin *skin = NULL;
	QMenuBar *menu = NULL;
	CEComboBox *cmbUserGroups = NULL;

	QWidget w;
	w.setFixedWidth(188); // this is (75x130) * 2.5
	w.setFixedHeight(325);

	skin = new CSkin(skinName);

	// Background
	QPixmap p;
	if (skin->frame.pixmap != NULL)
	{
		p = (QImage(skin->frame.pixmap).smoothScale(w.width(), w.height()));
		w.setPaletteBackgroundPixmap(p);
	}
	else
	{
		setBackgroundMode(PaletteBackground);
		unsetPalette();
	}

	// Group Combo Box
	cmbUserGroups = new CEComboBox(false, &w);
	cmbUserGroups->setNamedBgColor(skin->cmbGroups.color.bg);
	cmbUserGroups->setNamedFgColor(skin->cmbGroups.color.fg);
	cmbUserGroups->setGeometry(skin->borderToRect(&skin->cmbGroups, &w));
	cmbUserGroups->insertItem(Strings::getSystemGroupName(GROUP_ALL_USERS));

	// The Menu Button
	if (!skin->frame.hasMenuBar)
	{
		if (skin->btnSys.pixmapUpNoFocus == NULL)
		{
			btnSystem = new CEButton(skin->btnSys.caption == NULL ? QString("System") : QString::fromLocal8Bit(skin->btnSys.caption), &w);
		}
		else
		{
			btnSystem = new CEButton(new QPixmap(skin->btnSys.pixmapUpFocus),
															new QPixmap(skin->btnSys.pixmapUpNoFocus),
															new QPixmap(skin->btnSys.pixmapDown),
															&w);
		}
		btnSystem->setNamedFgColor(skin->btnSys.color.fg);
		btnSystem->setNamedBgColor(skin->btnSys.color.bg);
		btnSystem->setGeometry(skin->borderToRect(&skin->btnSys, &w));
		menu = NULL;
	}
	else
	{
		menu = new QMenuBar(&w);
		menu->insertItem(skin->btnSys.caption == NULL ? QString("System") : QString::fromLocal8Bit(skin->btnSys.caption));
		skin->AdjustForMenuBar(menu->height());
		menu->show();
		btnSystem = NULL;
	}

	// Message Label
	lblMsg = new CELabel(skin->lblMsg.transparent, NULL, &w);
	lblMsg->setFrameStyle(skin->lblMsg.frameStyle);
	lblMsg->setIndent(skin->lblMsg.margin);
	lblMsg->setNamedFgColor(skin->lblMsg.color.fg);
	lblMsg->setNamedBgColor(skin->lblMsg.color.bg);
	if (skin->lblMsg.pixmap != NULL)
	{
		lblMsg->setBackgroundOrigin(w.ParentOrigin);
		lblMsg->setPaletteBackgroundPixmap(p);
		lblMsg->setPixmap(QPixmap(skin->lblMsg.pixmap));
	}
	else if (skin->lblMsg.transparent && skin->frame.pixmap)
	{
		lblMsg->setBackgroundOrigin(w.ParentOrigin);
		lblMsg->setPaletteBackgroundPixmap(p);
	}

	lblMsg->setGeometry(skin->borderToRect(&skin->lblMsg, &w));
	lblMsg->setText(Strings::getSystemGroupName(GROUP_NEW_USERS));

  // Status Label
	lblStatus = new CELabel(skin->lblStatus.transparent, NULL, &w);
	lblStatus->setFrameStyle(skin->lblStatus.frameStyle);
	lblStatus->setIndent(skin->lblStatus.margin);
	lblStatus->setNamedFgColor(skin->lblStatus.color.fg);
	lblStatus->setNamedBgColor(skin->lblStatus.color.bg);
	if (skin->lblStatus.pixmap != NULL)
	{
		lblStatus->setBackgroundOrigin(w.ParentOrigin);
		lblStatus->setPaletteBackgroundPixmap(p);
		lblStatus->setPixmap(QPixmap(skin->lblStatus.pixmap));
	}
	else if (skin->lblStatus.transparent && skin->frame.pixmap)
	{
		lblStatus->setBackgroundOrigin(w.ParentOrigin);
		lblStatus->setPaletteBackgroundPixmap(p);
	}

	lblStatus->setGeometry(skin->borderToRect(&skin->lblStatus, &w));
	lblStatus->setText(Strings::getStatus(ICQ_STATUS_ONLINE, false));
	lblStatus->setPrependPixmap(CMainWindow::iconForStatus(ICQ_STATUS_ONLINE));

	// Userview
	CUserView userView(NULL, &w);
	userView.setGeometry(skin->frame.border.left, skin->frame.border.top,
												w.width() - skin->frameWidth(), w.height() - skin->frameHeight());
	// Save mainwindow origin colors (why does setColors() affect the mainwindow ???)
	char * c_online     = mainwin->skin->colors.online;
	char * c_away       = mainwin->skin->colors.away;
	char * c_offline    = mainwin->skin->colors.offline;
	char * c_newuser    = mainwin->skin->colors.newuser;
	char * c_background = mainwin->skin->colors.background;
	char * c_gridlines  = mainwin->skin->colors.gridlines;
  char* c_groupBack     = mainwin->skin->colors.groupBack;

  userView.QListView::setPalette(skin->palette(this));
  userView.setColors(skin->colors.online, skin->colors.away,
      skin->colors.offline, skin->colors.newuser,
      skin->colors.background, skin->colors.gridlines, skin->colors.groupBack);
	if (skin->frame.transparent)
	{
	userView.setBackgroundOrigin(w.ParentOrigin);
	userView.setPaletteBackgroundPixmap(p);
	}
	userView.show();

	QPixmap tmp(QPixmap::grabWidget(&w));
	QPixmap ret;
	ret.convertFromImage(QImage(tmp.convertToImage().smoothScale(75, MAX_HEIGHT_SKIN)));

	// Reset origin colors
  userView.setColors(c_online, c_away, c_offline, c_newuser, c_background, c_gridlines, c_groupBack);

	delete btnSystem;
	delete lblMsg;
	delete lblStatus;
	delete skin;
	delete menu;
	delete cmbUserGroups;

	return ret;
}

#include "skinbrowser.moc"
