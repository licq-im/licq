/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

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
#include "licq_log.h"
#include "licq_constants.h"
#include "mainwin.h"
#include "skin.h"
#include "skinbrowser.h"

SkinBrowserDlg::SkinBrowserDlg(CMainWindow *_mainwin, QWidget *parent)
	: LicqDialog(parent, "SkinBrowserDialog")
{
	mainwin = _mainwin;
	pmSkin = new QPixmap();
	lstIcons = new QValueList<QPixmap>;
	lstExtIcons = new QValueList<QPixmap>;
	lstAIcons = new QStringList();
	lstAExtIcons = new QStringList();
	
	// Setup a list of previewable icons
	// The strings reflect what we exptect to find in the *.icons files.
	// The result of these two lists is used to load the icons, the order of 
	// this list will be the order that the icons get rendered in the preview.
	*lstAIcons << "Online" << "Offline" << "FFC" << "Away" << "NA" << "Occupied"
						 << "DND" << "Private" << "Message" << "Url" << "Chat" << "File" 
						 << "SMS" << "Contact" << "Authorize" << "SecureOff" << "SecureOn" 
						 << "History" << "Info";
	*lstAExtIcons << "Collapsed" << "Expanded" << "Birthday" << "Cellular"
								<< "CustomAR" << "Invisible" << "Phone";
	
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

	// Preview Box
	QFrame *frmPrevSkin = new QFrame(boxPreview);
	QVBoxLayout *layPrevSkin = new QVBoxLayout(frmPrevSkin);
	QLabel *lblPrevSkin = new QLabel(tr("Skin:"), frmPrevSkin);
	lblPrevSkin->setAlignment(Qt::AlignHCenter);
	lblPaintSkin = new QLabel(frmPrevSkin);
	lblPaintSkin->setFixedSize(75, 130);
	layPrevSkin->addWidget(lblPrevSkin, 0, Qt::AlignHCenter);
	layPrevSkin->addWidget(lblPaintSkin, 0, Qt::AlignHCenter);
	layPrevSkin->addStretch();
	
	QFrame *frmPrevIcon = new QFrame(boxPreview);
	QVBoxLayout *layPrevIcon = new QVBoxLayout(frmPrevIcon);
	QLabel *lblPrevIcon = new QLabel(tr("Icons:"), frmPrevIcon);
	lblPrevIcon->setAlignment(Qt::AlignHCenter);
	lblPaintIcon = new SkinBrowserPreviewArea(frmPrevIcon);
	lblPaintIcon->setFixedSize(54, 130);
	layPrevIcon->addWidget(lblPrevIcon, 0, Qt::AlignHCenter);
	layPrevIcon->addWidget(lblPaintIcon, 0, Qt::AlignHCenter);
	layPrevIcon->addStretch();
	
	QFrame *frmPrevExtIcon = new QFrame(boxPreview);
	QVBoxLayout *layPrevExtIcon = new QVBoxLayout(frmPrevExtIcon);
	QLabel *lblPrevExtIcon = new QLabel(tr("Extended Icons:"), frmPrevExtIcon);
	lblPrevExtIcon->setAlignment(Qt::AlignHCenter);
	lblPaintExtIcon = new SkinBrowserPreviewArea(frmPrevExtIcon);
	lblPaintExtIcon->setFixedSize(54, 130);
	layPrevExtIcon->addWidget(lblPrevExtIcon, 0, Qt::AlignHCenter);
	layPrevExtIcon->addWidget(lblPaintExtIcon, 0, Qt::AlignHCenter);
	layPrevExtIcon->addStretch();

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
	QString szDir;
	szDir.sprintf("%s%s", SHARE_DIR, QTGUI_DIR);
	QDir dSkins(szDir, "skin.*", QDir::Name | QDir::IgnoreCase, QDir::Dirs);
	if (!dSkins.count())
	{
		gLog.Error("%sError reading qt-gui directory %s.\n", L_ERRORxSTR, szDir.latin1());
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
	}

	QDir dIcons(szDir, "icons.*", QDir::Name | QDir::IgnoreCase, QDir::Dirs);
	if (!dIcons.count())
	{
		gLog.Error("%sError reading qt-gui directory %s.\n", L_ERRORxSTR, szDir.latin1());
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
	}
	
	QDir dExtendedIcons(szDir, "extended.icons.*", QDir::Name | QDir::IgnoreCase, QDir::Dirs);
	if (!dExtendedIcons.count())
	{
		gLog.Error("%sError reading qt-gui directory %s.\n", L_ERRORxSTR, szDir.latin1());
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
	}
	
	// setup connections
	connect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_edtSkin()));
	connect(btnOk, SIGNAL(clicked()), this, SLOT(slot_ok()));
	connect(btnApply, SIGNAL(clicked()), this, SLOT(slot_apply()));
	connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));
	connect(cmbSkin, SIGNAL(highlighted(const QString &)), this, SLOT(slot_loadSkin(const QString &)));
	connect(cmbIcon, SIGNAL(highlighted(const QString &)), this, SLOT(slot_loadIcons(const QString &)));
	connect(cmbExtIcon, SIGNAL(highlighted(const QString &)), this, SLOT(slot_loadExtIcons(const QString &)));
	
	// Create initial preview
	slot_loadSkin(cmbSkin->currentText());
	slot_loadIcons(cmbIcon->currentText());
	slot_loadExtIcons(cmbExtIcon->currentText());
	
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
}

/*!	\brief Creates a new skin editor dialog
 *
 *	Creates a new Dialog which enables the user to edit the currently selected 
 *	skin.
 */
void SkinBrowserDlg::slot_edtSkin()
{
  if (!cmbSkin->currentText()) return;
  QString f;
  f.sprintf("%sqt-gui/skin.%s/%s.skin", SHARE_DIR,
            QFile::encodeName(cmbSkin->currentText()).data(),
            QFile::encodeName(cmbSkin->currentText()).data());
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
	QString iconsFile = QString("%1%2icons.%3/%4.icons").arg(SHARE_DIR).arg(QTGUI_DIR).arg(icon).arg(icon);
	char sFileName[MAX_FILENAME_LEN] = "";
	CIniFile fIconsConf;
	if (!fIconsConf.LoadFile(iconsFile))
	{
		WarnUser(this, tr("Unable to open icons file\n%1").arg(iconsFile));
		return;
	}
	fIconsConf.SetSection("icons");
	for (QStringList::Iterator it = lstAIcons->begin(); it != lstAIcons->end(); ++it)
	{
		fIconsConf.ReadStr((*it).ascii(), sFileName, "");
		QString pmFile = QString("%1%2icons.%3/%4").arg(SHARE_DIR).arg(QTGUI_DIR).arg(icon).arg(sFileName);
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
	QString iconsFile = QString("%1%2extended.icons.%3/%4.icons").arg(SHARE_DIR).arg(QTGUI_DIR).arg(extIcon).arg(extIcon);
	char sFileName[MAX_FILENAME_LEN] = "";
	CIniFile fIconsConf;
	if (!fIconsConf.LoadFile(iconsFile))
	{
		WarnUser(this, tr("Unable to open extended icons file\n%1").arg(iconsFile));
		return;
	}
	fIconsConf.SetSection("icons");
	for (QStringList::Iterator it = lstAExtIcons->begin(); it != lstAExtIcons->end(); ++it)
	{
		fIconsConf.ReadStr((*it).ascii(), sFileName, "");
		QString pmFile = QString("%1%2extended.icons.%3/%4").arg(SHARE_DIR).arg(QTGUI_DIR).arg(extIcon).arg(sFileName);
		QPixmap pm(pmFile);
		if (! pm.isNull())
			lstExtIcons->append(pm);
	}
	lblPaintExtIcon->setPixmapList(lstExtIcons);
}

/*! \brief provide correct repainting when resizing the main widget
 *
 *	This slot is called everytime the mainwidget gets resized. It forces the 
 *	preview areas to be updated so that it looks nice with structured 
 *	backgrounds.
 */
void SkinBrowserDlg::resizeEvent(QResizeEvent *e)
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

void SkinBrowserPreviewArea::paintEvent(QPaintEvent *e)
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
#if QT_VERSION < 300
		w.setBackgroundPixmap(p);
#else
		w.setPaletteBackgroundPixmap(p);
#endif
	}
	else
	{
		setBackgroundMode(PaletteBackground);
#if QT_VERSION >= 300
		unsetPalette();
#endif
	}

	// Group Combo Box
	cmbUserGroups = new CEComboBox(false, &w);
	cmbUserGroups->setNamedBgColor(skin->cmbGroups.color.bg);
	cmbUserGroups->setNamedFgColor(skin->cmbGroups.color.fg);
	cmbUserGroups->setGeometry(skin->borderToRect(&skin->cmbGroups, &w));
	cmbUserGroups->insertItem("All Users");

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
#if QT_VERSION < 300
		lblMsg->setBackgroundPixmap(QPixmap(skin->lblMsg.pixmap));
	}
#else
		lblMsg->setBackgroundOrigin(w.ParentOrigin);
		lblMsg->setPaletteBackgroundPixmap(p);
		lblMsg->setPixmap(QPixmap(skin->lblMsg.pixmap));
	}
	else if (skin->lblMsg.transparent && skin->frame.pixmap)
	{
		lblMsg->setBackgroundOrigin(w.ParentOrigin);
		lblMsg->setPaletteBackgroundPixmap(p);
	}
#endif
	lblMsg->setGeometry(skin->borderToRect(&skin->lblMsg, &w));
	lblMsg->setText("New Users");

  // Status Label
	lblStatus = new CELabel(skin->lblStatus.transparent, NULL, &w);
	lblStatus->setFrameStyle(skin->lblStatus.frameStyle);
	lblStatus->setIndent(skin->lblStatus.margin);
	lblStatus->setNamedFgColor(skin->lblStatus.color.fg);
	lblStatus->setNamedBgColor(skin->lblStatus.color.bg);
	if (skin->lblStatus.pixmap != NULL)
	{
#if QT_VERSION < 300
		lblStatus->setBackgroundPixmap(p);
	}
#else
		lblStatus->setBackgroundOrigin(w.ParentOrigin);
		lblStatus->setPaletteBackgroundPixmap(p);
		lblStatus->setPixmap(QPixmap(skin->lblStatus.pixmap));
	}
	else if (skin->lblStatus.transparent && skin->frame.pixmap)
	{
		lblStatus->setBackgroundOrigin(w.ParentOrigin);
		lblStatus->setPaletteBackgroundPixmap(p);
	}
#endif
	lblStatus->setGeometry(skin->borderToRect(&skin->lblStatus, &w));
	lblStatus->setText("Online");
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

	userView.setColors(skin->colors.online, skin->colors.away,
											skin->colors.offline, skin->colors.newuser,
											skin->colors.background, skin->colors.gridlines);
	if (skin->frame.transparent)
	{
#if QT_VERSION < 300
	userView.setBackgroundPixmap(p);
#else
	userView.setBackgroundOrigin(w.ParentOrigin);
	userView.setPaletteBackgroundPixmap(p);
#endif
	}
	userView.show();

	QPixmap tmp(QPixmap::grabWidget(&w));
	QPixmap ret;
	ret.convertFromImage(QImage(tmp.convertToImage().smoothScale(75, 130)));

	// Reset origin colors
	userView.setColors(c_online, c_away, c_offline, c_newuser, c_background, c_gridlines);

	delete btnSystem;
	delete lblMsg;
	delete lblStatus;
	delete skin;
	delete menu;
	delete cmbUserGroups;
	
	return ret;
}

#include "skinbrowser.moc"
