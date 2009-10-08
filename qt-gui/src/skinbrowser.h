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

#ifndef SKINBROWSER_H
#define SKINBROWSER_H

#include <qcombobox.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qpixmap.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <qwidget.h>

#include "licqdialog.h"
#include "skin.h"

class CMainWindow;
class SkinBrowserPreviewArea;

/*!	\brief Provides a user dialog for selecting Skins and IconSets
 *
 *	A SkinBrowserDlg enables the user to select a skin, iconset and extended 
 *	iconset for Licq. Every gui item is automatically previewed before the 
 *	user makes his final decision.
 */
class SkinBrowserDlg : public LicqDialog
{
Q_OBJECT
public:
	/*! Paints the window and sets initial values for its content */
	SkinBrowserDlg (CMainWindow *_mainwin, QWidget *parent = 0);
	/*! Destroys the SkinBrowserDlg */
	~SkinBrowserDlg ();
	void resizeEvent(QResizeEvent *e);

private:
	/*! This Box holds the Dropdown menus for Skin selection */
	QGroupBox *boxSkin;
	/*! This Box holds the preview area */
	QGroupBox *boxPreview;
	/*! This Combo contains all available skin packs */
	QComboBox *cmbSkin;
	/*! This Combo contains all available icon packs */
	QComboBox *cmbIcon;
	/*! This Combo contains all available extended icon packs */
	QComboBox *cmbExtIcon;
	/*! This Combo contains all available emoticons themes */
	QComboBox *cmbEmoticon;
	/*! This QLabel contains the skin preview pixmap (75 x 130 Pixel)*/
	QLabel *lblPaintSkin;
	/*! This SkinBrowserPreviewArea contains the icon pixmaps (54 x 130 Pixel)*/
	SkinBrowserPreviewArea *lblPaintIcon;
	/*! This SkinBrowserPreviewArea contains the extended icons pixmaps (54 x 130 Pixel)*/
	SkinBrowserPreviewArea *lblPaintExtIcon;
	/*! This SkinBrowserPreviewArea contains the emoticons pixmaps (54x130 Px) */
	SkinBrowserPreviewArea *lblPaintEmoticon;
	/*! Holds the list of possible themeable icons in normal icon sets */
	QStringList *lstAIcons;
	/*! Holds the list of possible themeable icons in extended icon sets */
	QStringList *lstAExtIcons;
	/*! Stores the current QPixmap for the Skin preview */
	QPixmap *pmSkin;
	/*! Stores the list of the current QPixmaps for the Icons preview */
	QValueList<QPixmap> *lstIcons;
	/*! Stores the list of the current QPixmaps for the Extended Icons preview */
	QValueList<QPixmap> *lstExtIcons;
	/*! Stores the list of the current QPixmaps for the emoticons preview */
	QValueList<QPixmap> *lstEmoticons;
	/*! Renders a dynamic skin preview */
	QPixmap renderSkin(const QString &skin);
	CSkin *skin;
	
protected:
	CMainWindow *mainwin;

protected slots:
	void slot_edtSkin();
	void slot_ok();
	void slot_apply();
	
	void slot_loadSkin(const QString &skin);
	void slot_loadIcons(const QString &icon);
	void slot_loadExtIcons(const QString &extIcon);
	void slot_loadEmoticons(const QString &emoticon);
};

/*!	\brief Helper class to provide a preview area for our icons using a modified QFrame
 *
 *	This class is derived from QFrame and extends minor functionality
 *	to provide support for many icons to be painted on this widget.
 *	The main improvement is, to make it redraw the icons when the widget
 *	gets modified by movement, hiding or resizing.
 */
class SkinBrowserPreviewArea : public QFrame
{
Q_OBJECT
public:
	SkinBrowserPreviewArea (QWidget *parent);
	void setPixmapList(QValueList<QPixmap> *_lstPm);

private:
	void paintEvent(QPaintEvent *e);
	/*! Holds a copy of the pixmaps that are drawn on the widget */
	QValueList<QPixmap> lstPm;
};

#endif
