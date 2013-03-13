/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2013 Licq developers <licq-dev@googlegroups.com>
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

#include "iface.h"

#include <boost/foreach.hpp>

#include <licq/contactlist/owner.h>
#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>
#include <licq/pluginsignal.h>
#include <licq/translator.h>
#include <licq/userevents.h>

#include "conf.h"

using Licq::gUserManager;
using std::string;

Iface::Iface()
{
  g_type_init();

  aosd = aosd_new();
  aosd_set_transparency(aosd, TRANSPARENCY_COMPOSITE);
  aosd_set_renderer(aosd, aosd_text_renderer, &trd);
  aosd_set_names(aosd, "OSD Plugin", "Licq");

  memset(&trd, 0, sizeof(TextRenderData));
  trd.lay = pango_layout_new_aosd();
  pango_layout_set_wrap(trd.lay, PANGO_WRAP_WORD_CHAR);

  conf = new Conf();

  updateTextRenderData();

  {
    Licq::OwnerListGuard ownerList;
    BOOST_FOREACH(const Licq::Owner* owner, **ownerList)
    {
      ppidTimers[owner->protocolId()] = time(NULL);
    }
  }

  string init = "Licq aosd plugin is initialized.";
  displayLayout(init, false);
}

Iface::~Iface()
{
  delete conf;
  ppidTimers.clear();
  aosd_destroy(aosd);
  pango_layout_unref_aosd(trd.lay);
}

void Iface::processSignal(const Licq::PluginSignal* sig)
{
  string msg = "";
  bool control = true;

  unsigned long ppid = sig->userId().protocolId();

  switch (sig->signal())
  {
    case Licq::PluginSignal::SignalUser:
    {
      if (filterSignal(sig, ppid))
        break;

      Licq::UserReadGuard user(sig->userId());
      if (!user.isLocked())
        break;

      msg = user->getAlias();

      switch (sig->subSignal())
      {
        case Licq::PluginSignal::UserStatus:
          msg += " changed status to: ";
          msg += user->statusToString(true);
          if (sig->argument() > 0)
            msg += " [logged on]";
          break;

        case Licq::PluginSignal::UserEvents:
          if (sig->argument() == 0)
          {
            msg += " checked your auto-response";
            break;
          }

          control = false;

          if (conf->notifyOnly)
          {
            msg = "Message from ";
            msg += user->getAlias();
            if (conf->markSecure && user->Secure())
              msg += " [secured]";
            msg += ".";
          }
          else
          {
            const Licq::UserEvent* ue = user->EventPeekId(sig->argument());
            if (ue == NULL)
            {
              msg.clear();
              break;
            }
            if (conf->markSecure && user->Secure())
              msg += " (S)";
            msg += ": ";
            msg += ue->text();
          }
          break;
      }

      break;
    }

    case Licq::PluginSignal::SignalLogon:
    case Licq::PluginSignal::SignalLogoff:
      ppidTimers[ppid] = time(NULL) + conf->quietTimeout;
      break;
  }

  if (!msg.empty())
    displayLayout(msg, control);
}

void Iface::updateTextRenderData()
{
  conf->loadConfig();

  aosd_set_hide_upon_mouse_event(aosd, conf->mouseActive);
  pango_layout_set_font_aosd(trd.lay, conf->font.c_str());

  trd.geom.x_offset = conf->marginHorizontal;
  trd.geom.y_offset = conf->marginVertical;

  trd.back.color = conf->backColor.c_str();
  trd.back.opacity = conf->backOpacity;

  trd.shadow.x_offset = conf->shadowOffset;
  trd.shadow.y_offset = conf->shadowOffset;
  trd.shadow.color = conf->shadowColor.c_str();
  trd.shadow.opacity = conf->shadowOpacity;

  trd.fore.opacity = conf->textOpacity;

  int width = aosd_text_get_screen_wrap_width(aosd, &trd);
  if (conf->wrapWidth == 0 || width < 0)
    pango_layout_set_width(trd.lay, -1);
  else
    pango_layout_set_width(trd.lay,
        (width > conf->wrapWidth ? conf->wrapWidth : width) * PANGO_SCALE);
}

bool Iface::filterSignal(const Licq::PluginSignal* sig, unsigned long ppid)
{
  switch (sig->subSignal())
  {
    case Licq::PluginSignal::UserStatus:
    case Licq::PluginSignal::UserEvents:
      break;

    default:
      return true;
  }

  bool isMessage = (sig->subSignal() == Licq::PluginSignal::UserEvents
                    && sig->argument() > 0);

  if (!isMessage && (ppidTimers[ppid] > time(NULL)))
    return true;

  // TODO status check

  if (sig->userId().isOwner())
  {
    if (isMessage && conf->showMessage != GROUP_TYPE_NONE)
      return false;

    return true;
  }

  bool on;
  {
    Licq::UserReadGuard user(sig->userId());
    if (!user.isLocked())
      return true;

    if (user->IgnoreList())
      return true;
    on = user->OnlineNotify();
  }

#define GROUP_DISABLED(type) \
  (conf->type == GROUP_TYPE_NONE || \
  (conf->type == GROUP_TYPE_ONLINE_NOTIFY && !on))

  if (sig->subSignal() == Licq::PluginSignal::UserStatus)
  {
    if (sig->argument() == 0 && GROUP_DISABLED(statusChange))
      return true;

    if (GROUP_DISABLED(logonLogoff))
      return true;
  }

  if (sig->subSignal() == Licq::PluginSignal::UserEvents)
  {
    if (sig->argument() < 0)
      return true;

    if (sig->argument() == 0 && GROUP_DISABLED(autoResponse))
      return true;

    if (GROUP_DISABLED(showMessage))
      return true;
  }
#undef GROUP_DISABLED

  return false;
}

void Iface::displayLayout(string& msg, bool control)
{
  trd.fore.color = control ? conf->textControlColor.c_str() :
    conf->textColor.c_str();

  pango_layout_set_text(trd.lay, msg.c_str(), -1);

  if (conf->maxLines != 0 &&
      pango_layout_get_line_count(trd.lay) > conf->maxLines)
  {
    PangoLayoutLine* line =
      pango_layout_get_line_readonly(trd.lay, conf->maxLines);
    pango_layout_set_text(trd.lay, msg.c_str(), line->start_index);
  }

  unsigned width, height;

  aosd_text_get_size(&trd, &width, &height);

  aosd_set_position_with_offset(aosd,
      conf->posHorizontal, conf->posVertical,
      width, height, conf->offsetHorizontal, conf->offsetVertical);

  // TODO implement non-waiting mechanism

  aosd_flash(aosd, conf->fadeIn,
      conf->fadeFull + msg.size() * conf->delayPerChar, conf->fadeOut);
}

/* vim: set ts=2 sw=2 et : */
