/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2012 Licq developers <licq-dev@googlegroups.com>
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

#ifndef FILTERRULEDLG_H
#define FILTERRULEDLG_H

#include <vector>

#include <QDialog>

class QCheckBox;
class QRadioButton;

namespace Licq
{
struct FilterRule;
typedef std::vector<FilterRule> FilterRules;
}

namespace LicqQtGui
{
class MLEdit;
class ProtoComboBox;

class FilterRuleDlg : public QDialog
{
  Q_OBJECT

public:
  /**
   * Constructor
   *
   * @param rules Rules to edit or NULL to create new rule
   * @param parent Parent object
   */
  FilterRuleDlg(const Licq::FilterRule* rule = NULL, QWidget* parent = NULL);

  /**
   * Get filter rule from dialog
   *
   * @param rule Rule to populate with parameters
   */
  void getFilterRule(Licq::FilterRule& rule) const;

private:
  QCheckBox* myEnableCheck;
  ProtoComboBox* myProtocolCombo;
  QRadioButton* myAcceptRadio;
  QRadioButton* mySilentRadio;
  QRadioButton* myIgnoreRadio;
  QCheckBox* myEventChecks[32];
  MLEdit* myExpressionEdit;
};

} // namespace LicqQtGui

#endif // FILTERRULEDLG_H

