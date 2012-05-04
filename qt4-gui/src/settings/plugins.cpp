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

#include "plugins.h"

#include <boost/foreach.hpp>

#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStringList>
#include <QTimer>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <licq/daemon.h>
#include <licq/plugin/generalplugin.h>
#include <licq/plugin/pluginmanager.h>

#include "core/messagebox.h"
#include "dialogs/editfiledlg.h"

#include "settingsdlg.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::Settings::Plugins */

static Licq::GeneralPlugin::Ptr getGeneralPlugin(int id)
{
  Licq::GeneralPluginsList plugins;
  Licq::gPluginManager.getGeneralPluginsList(plugins);
  BOOST_FOREACH(Licq::GeneralPlugin::Ptr plugin, plugins)
  {
    if (plugin->id() == id)
      return plugin;
  }
  return Licq::GeneralPlugin::Ptr();
}

Settings::Plugins::Plugins(SettingsDlg* parent)
  : QObject(parent)
{
  parent->addPage(SettingsDlg::PluginsPage, createPagePlugins(parent),
      tr("Plugins"));

  updatePluginList();
}

QWidget* Settings::Plugins::createPagePlugins(QWidget* parent)
{
  QGroupBox* pluginsBox = new QGroupBox(tr("Plugins"), parent);
  QVBoxLayout* pluginsLayout = new QVBoxLayout(pluginsBox);
  pluginsLayout->setContentsMargins(0, 0, 0, 0);

  myPluginsList = new QTreeWidget();
  QStringList headers;
  headers << tr("Name") << tr("Version") << tr("Enabled") << tr("Description");
  myPluginsList->setHeaderLabels(headers);
  myPluginsList->setIndentation(0);
  myPluginsList->setAllColumnsShowFocus(true);
  pluginsLayout->addWidget(myPluginsList);

  QHBoxLayout* buttons = new QHBoxLayout();

  myLoadButton = new QPushButton(tr("Load"));
  buttons->addWidget(myLoadButton);

  myUnloadButton = new QPushButton(tr("Unload"));
  buttons->addWidget(myUnloadButton);

  myEnableButton = new QPushButton(tr("Enable"));
  buttons->addWidget(myEnableButton);

  myDisableButton = new QPushButton(tr("Disable"));
  buttons->addWidget(myDisableButton);

  QPushButton* refreshButton = new QPushButton(tr("Refresh"));
  buttons->addWidget(refreshButton);

  pluginsLayout->addLayout(buttons);

  connect(myPluginsList, SIGNAL(itemSelectionChanged()), SLOT(updatePluginButtons()));
  connect(myPluginsList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
      SLOT(pluginDoubleClicked(QTreeWidgetItem*,int)));
  connect(myLoadButton, SIGNAL(clicked()), SLOT(loadPlugin()));
  connect(myUnloadButton, SIGNAL(clicked()), SLOT(unloadPlugin()));
  connect(myEnableButton, SIGNAL(clicked()), SLOT(enablePlugin()));
  connect(myDisableButton, SIGNAL(clicked()), SLOT(disablePlugin()));
  connect(refreshButton, SIGNAL(clicked()), SLOT(updatePluginList()));

  return pluginsBox;
}

void Settings::Plugins::updatePluginList()
{
  myPluginsList->clear();

  // Get list of loaded plugins
  Licq::GeneralPluginsList plugins;
  Licq::gPluginManager.getGeneralPluginsList(plugins);
  BOOST_FOREACH(Licq::GeneralPlugin::Ptr plugin, plugins)
  {
    QTreeWidgetItem* item = new QTreeWidgetItem(myPluginsList);
    item->setText(0, plugin->name().c_str());
    item->setText(1, plugin->version().c_str());
    item->setText(2, plugin->isEnabled() ? tr("Yes") : tr("No"));
    item->setText(3, plugin->description().c_str());

    item->setData(0, Qt::UserRole, plugin->id());
    item->setData(2, Qt::UserRole, plugin->isEnabled());
  }

  // Get list of available (not loaded) plugins
  std::list<std::string> unloadedPlugins;
  Licq::gPluginManager.getAvailableGeneralPlugins(unloadedPlugins, false);
  BOOST_FOREACH(std::string plugin, unloadedPlugins)
  {
    QTreeWidgetItem* item = new QTreeWidgetItem(myPluginsList);
    item->setText(0, plugin.c_str());
    item->setText(1, tr("(Not loaded)"));

    item->setData(0, Qt::UserRole, plugin.c_str());
  }

  myPluginsList->resizeColumnToContents(0);
  myPluginsList->resizeColumnToContents(1);
  myPluginsList->resizeColumnToContents(2);
  myPluginsList->resizeColumnToContents(3);
  updatePluginButtons();
}

void Settings::Plugins::updatePluginButtons()
{
  QTreeWidgetItem* item = myPluginsList->currentItem();
  bool isLoaded = (item != NULL && item->data(0, Qt::UserRole).type() == QVariant::Int);
  myLoadButton->setEnabled(item != NULL && !isLoaded);
  myUnloadButton->setEnabled(isLoaded);
  myEnableButton->setEnabled(isLoaded && !item->data(2, Qt::UserRole).toBool());
  myDisableButton->setEnabled(isLoaded && item->data(2, Qt::UserRole).toBool());
}

void Settings::Plugins::loadPlugin()
{
  QTreeWidgetItem* item = myPluginsList->currentItem();
  if (item == NULL)
    return;

  QString pluginName = item->data(0, Qt::UserRole).toString();
  Licq::gPluginManager.startGeneralPlugin(pluginName.toLocal8Bit().constData(), 0, NULL);

  QTimer::singleShot(1000, this, SLOT(updatePluginList()));
}

void Settings::Plugins::unloadPlugin()
{
  QTreeWidgetItem* item = myPluginsList->currentItem();
  if (item == NULL)
    return;

  int index = item->data(0, Qt::UserRole).toInt();
  Licq::GeneralPlugin::Ptr plugin = getGeneralPlugin(index);
  if (plugin.get() == NULL)
    return;

  Licq::gPluginManager.unloadGeneralPlugin(plugin);

  QTimer::singleShot(1000, this, SLOT(updatePluginList()));
}

void Settings::Plugins::enablePlugin()
{
  QTreeWidgetItem* item = myPluginsList->currentItem();
  if (item == NULL)
    return;

  int index = item->data(0, Qt::UserRole).toInt();
  Licq::GeneralPlugin::Ptr plugin = getGeneralPlugin(index);
  if (plugin.get() == NULL)
    return;

  plugin->enable();

  QTimer::singleShot(1000, this, SLOT(updatePluginList()));
}

void Settings::Plugins::disablePlugin()
{
  QTreeWidgetItem* item = myPluginsList->currentItem();
  if (item == NULL)
    return;

  int index = item->data(0, Qt::UserRole).toInt();
  Licq::GeneralPlugin::Ptr plugin = getGeneralPlugin(index);
  if (plugin.get() == NULL)
    return;

  plugin->disable();

  QTimer::singleShot(1000, this, SLOT(updatePluginList()));
}

void Settings::Plugins::pluginDoubleClicked(QTreeWidgetItem* item, int /* index */)
{
  if (item->data(0, Qt::UserRole).type() == QVariant::String)
    return;

  int index = item->data(0, Qt::UserRole).toInt();
  Licq::GeneralPlugin::Ptr plugin = getGeneralPlugin(index);
  if (plugin.get() == NULL)
    return;

  if (plugin->configFile().empty())
  {
    InformUser(dynamic_cast<SettingsDlg*>(parent()),
        tr("Plugin %1 has no configuration file").arg(plugin->name().c_str()));
    return;
  }

  QString f;
  f.sprintf("%s%s", Licq::gDaemon.baseDir().c_str(), plugin->configFile().c_str());
  new EditFileDlg(f);
}
