#ifndef __LICQDIALOG_H
#define __LICQDIALOG_H

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

#include <qdialog.h>

// This is a wrapper for QDialog since Qt3 has broken XClassHints which are
// used by the WindowManager to decide how to decorate (or not) each window
// on an individual basis.

class LicqDialog : public QDialog
{
  Q_OBJECT

public:
  LicqDialog(QWidget *parent = 0, const char *name = 0, bool modal = false,
             WFlags f = 0);
  virtual ~LicqDialog()	{ }
};

#endif // __LICQDIALOG_H
