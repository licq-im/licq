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

#include "filterruledlg.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QVBoxLayout>

#include <licq/filter.h>
#include <licq/userevents.h>

#include "helpers/support.h"
#include "widgets/mledit.h"
#include "widgets/protocombobox.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::FilterRuleDlg */

FilterRuleDlg::FilterRuleDlg(const Licq::FilterRule* rule, QWidget* parent)
  : QDialog(parent)
{
  Support::setWidgetProps(this, "FilterRule");
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(rule == NULL ? tr("New Event Filter Rule") : tr("Edit Event Filter Rule"));

  QGridLayout* lay = new QGridLayout(this);


  QGroupBox* generalBox = new QGroupBox(tr("General"));
  QVBoxLayout* generalLay = new QVBoxLayout(generalBox);

  myEnableCheck = new QCheckBox(tr("Enable rule"));
  myEnableCheck->setToolTip(tr("Uncheck to disable this rule without removing it."));
  generalLay->addWidget(myEnableCheck);

  myProtocolCombo = new ProtoComboBox(tr("Any protocol"));
  generalLay->addWidget(myProtocolCombo);


  QGroupBox* actionBox = new QGroupBox(tr("Action"));
  QVBoxLayout* actionLay = new QVBoxLayout(actionBox);

  myAcceptRadio = new QRadioButton(tr("Accept normally"));
  myAcceptRadio->setToolTip(tr("Accept event and perform normal on event actions."));
  actionLay->addWidget(myAcceptRadio);

  mySilentRadio = new QRadioButton(tr("Accept silently"));
  mySilentRadio->setToolTip(tr("Add event to history but don't show it or perform any on event actions."));
  actionLay->addWidget(mySilentRadio);

  myIgnoreRadio = new QRadioButton(tr("Ignore"));
  myIgnoreRadio->setToolTip(tr("Ignore event completely."));
  actionLay->addWidget(myIgnoreRadio);


  QGroupBox* eventBox = new QGroupBox(tr("Event Types"));
  QGridLayout* eventLay = new QGridLayout(eventBox);

  int pos = 0;
  for (unsigned i = 0; i < 32; ++i)
  {
    std::string eventName = Licq::UserEvent::eventName(i);
    if (eventName.empty())
    {
      // This event doesn't exist, skip it
      myEventChecks[i] = NULL;
      continue;
    }
    myEventChecks[i] = new QCheckBox(QString::fromLocal8Bit(eventName.c_str()));
    eventLay->addWidget(myEventChecks[i], pos/2, pos%2);
    ++pos;
  }


  QGroupBox* expressionBox = new QGroupBox(tr("Expression"));
  QVBoxLayout* expressionLay = new QVBoxLayout(expressionBox);

  myExpressionEdit = new MLEdit(true);
  myExpressionEdit->setSizeHintLines(3);
  myExpressionEdit->setToolTip(tr("Regular expression to match event text against.\n"
      "Leave empty to match any message, otherwise it must match entire message "
      "(Hint: Use \".*part.*\" to match messages with \"part\" in them).\n"
      "Note: Multibyte characters are not handled so UTF8 characters will be handled as individual bytes."));
  expressionLay->addWidget(myExpressionEdit);


  QDialogButtonBox* buttons = new QDialogButtonBox();
  buttons->addButton(QDialogButtonBox::Ok);
  buttons->addButton(QDialogButtonBox::Cancel);
  connect(buttons, SIGNAL(accepted()), SLOT(accept()));
  connect(buttons, SIGNAL(rejected()), SLOT(reject()));


  lay->addWidget(generalBox, 0, 0);
  lay->addWidget(actionBox, 0, 1);
  lay->addWidget(eventBox, 1, 0, 1, 2);
  lay->addWidget(expressionBox, 2, 0, 1, 2);
  lay->addWidget(buttons, 3, 0, 1, 2);
  lay->setRowStretch(2, 1);

  if (rule == NULL)
  {
    // New rule, load defaults
    myEnableCheck->setChecked(true);
    myProtocolCombo->setCurrentPpid(0);
    myAcceptRadio->setChecked(true);
  }
  else
  {
    myEnableCheck->setChecked(rule->isEnabled);
    myProtocolCombo->setCurrentPpid(rule->protocolId);
    myAcceptRadio->setChecked(rule->action == Licq::FilterRule::ActionAccept);
    mySilentRadio->setChecked(rule->action == Licq::FilterRule::ActionSilent);
    myIgnoreRadio->setChecked(rule->action == Licq::FilterRule::ActionIgnore);
    myExpressionEdit->setText(QString::fromUtf8(rule->expression.c_str()));
    for (unsigned i = 0; i < 32; ++i)
      if (myEventChecks[i] != NULL)
        myEventChecks[i]->setChecked(rule->eventMask & (1 << i));
  }

  show();
}

void FilterRuleDlg::getFilterRule(Licq::FilterRule& rule) const
{
  rule.isEnabled = myEnableCheck->isChecked();
  rule.protocolId = myProtocolCombo->currentPpid();
  rule.expression = myExpressionEdit->toPlainText().toUtf8().constData();

  rule.eventMask = 0;
  for (unsigned i = 0; i < 32; ++i)
    if (myEventChecks[i] != NULL && myEventChecks[i]->isChecked())
      rule.eventMask |= (1 << i);

  if (myIgnoreRadio->isChecked())
    rule.action = Licq::FilterRule::ActionIgnore;
  else if (mySilentRadio->isChecked())
    rule.action = Licq::FilterRule::ActionSilent;
  else
    rule.action = Licq::FilterRule::ActionAccept;
}
