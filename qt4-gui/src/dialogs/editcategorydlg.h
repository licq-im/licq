/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2004-2012 Licq developers <licq-dev@googlegroups.com>
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

#ifndef CATDLG_H
#define CATDLG_H

#include <QDialog>

#include <licq/icq/user.h>

class QComboBox;
class QLineEdit;

struct SCategory;

namespace LicqQtGui
{

class EditCategoryDlg : public QDialog
{
  Q_OBJECT

public:
  EditCategoryDlg(Licq::UserCat cat, const Licq::UserCategoryMap& category, QWidget* parent = 0);

signals:
  void updated(Licq::UserCat cat, const Licq::UserCategoryMap& category);

private:
  QComboBox* myCats[Licq::MAX_CATEGORIES];
  QLineEdit* myDescr[Licq::MAX_CATEGORIES];
  unsigned short myNumCats;
  Licq::UserCat myUserCat;

  const struct SCategory* (*getEntry)(unsigned short index);

private slots:
  void ok();
  void checkEnabled();
};

} // namespace LicqQtGui

#endif
