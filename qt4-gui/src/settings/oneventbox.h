/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq developers
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

#ifndef SETTINGS_ONEVENTSBOX_H
#define SETTINGS_ONEVENTSBOX_H

#include <config.h>

#include <QGroupBox>

#include <licq/oneventmanager.h>

class QCheckBox;
class QComboBox;


namespace LicqQtGui
{
class FileNameEdit;

class OnEventBox : public QGroupBox
{
  Q_OBJECT

public:
  OnEventBox(bool isGlobal, QWidget* parent = NULL);
  virtual ~OnEventBox() {}

public slots:
  /**
   * Load configuration
   *
   * @param effectiveData Data object to get configuration from
   * @param readData Data object to get default values from (ignored for global data)
   */
  void load(const Licq::OnEventData* effectiveData, const Licq::OnEventData* realData = NULL);

  /**
   * Save configuration
   *
   * @param eventData Data object to write configuration to
   */
  void apply(Licq::OnEventData* eventData);

private:
  bool myIsGlobal;

  QComboBox* myEnabledCombo;
  FileNameEdit* myCommandEdit;
  FileNameEdit* myParameterEdit[Licq::OnEventData::NumOnEventTypes];
  QCheckBox* myAlwaysNotifyCheck;

  QCheckBox* myOverEnabledCheck;
  QCheckBox* myOverCommandCheck;
  QCheckBox* myOverParameterCheck[Licq::OnEventData::NumOnEventTypes];
  QCheckBox* myOverAlwaysNotifyCheck;
};

} // namespace LicqQtGui

#endif
