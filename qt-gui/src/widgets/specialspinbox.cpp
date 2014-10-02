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

#include "specialspinbox.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::SpecialSpinBox */

SpecialSpinBox::SpecialSpinBox(int minimum, int maximum, const QString& specialValueText, QWidget* parent)
  : QSpinBox(parent)
{
  setRange(minimum, maximum);
  setSpecialValueText(specialValueText);
  setAccelerated(true);

  // Values are numeric so aligning right makes more sense
  setAlignment(Qt::AlignRight);

  // Let empty value go to special value instead of previous
  setCorrectionMode(QSpinBox::CorrectToNearestValue);
  connect(this, SIGNAL(editingFinished()), SLOT(finished()));
}

void SpecialSpinBox::finished()
{
  // Default to special text if input is bad
  if (!hasAcceptableInput())
    setValue(minimum());
}
