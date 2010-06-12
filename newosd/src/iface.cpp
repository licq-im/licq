#include "iface.h"

#include <licq/icq.h>
#include <licq/translator.h>
#include <licq/contactlist/usermanager.h>

#include "conf.h"

using namespace std;
using Licq::gUserManager;

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

  FOR_EACH_OWNER_START(LOCK_R)
  {
    ppidTimers[pOwner->PPID()] = time(NULL);
  }
  FOR_EACH_OWNER_END;

  string init = "Licq newosd plugin is initialized.";
  displayLayout(init, false);
}

Iface::~Iface()
{
  delete conf;
  ppidTimers.clear();
  aosd_destroy(aosd);
  pango_layout_unref_aosd(trd.lay);
}

void Iface::processSignal(LicqSignal* sig)
{
  string msg = "";
  bool control = true;

  Licq::User* u = gUserManager.fetchUser(sig->userId(), LOCK_R);
  if (u == NULL)
    return;
  unsigned long ppid = u->PPID();
  gUserManager.DropUser(u);

  switch (sig->Signal())
  {
    case SIGNAL_UPDATExUSER:
    {
      if (filterSignal(sig, ppid))
        break;

      u = gUserManager.fetchUser(sig->userId(), LOCK_R);
      if (u == NULL)
        break;

      msg = u->GetAlias();

      switch (sig->SubSignal())
      {
        case USER_STATUS:
          msg += " changed status to: ";
          msg += u->StatusStr();
          if (sig->Argument() > 0)
            msg += " [logged on]";
          break;

        case USER_EVENTS:
          if (sig->Argument() == 0)
          {
            msg += " checked your auto-response";
            break;
          }

          control = false;

          if (conf->notifyOnly)
          {
            msg = "Message from ";
            msg += u->GetAlias();
            if (conf->markSecure && u->Secure())
              msg += " [secured]";
            msg += ".";
          }
          else
          {
            const CUserEvent* ue = u->EventPeekId(sig->Argument());
            if (ue == NULL)
            {
              msg.clear();
              break;
            }
            if (conf->markSecure && u->Secure())
              msg += " (S)";
            msg += ": ";
            char* trans = Licq::gTranslator.ToUnicode(const_cast<char*>(ue->Text()),
                u->userEncoding().c_str());
            msg += trans;
            delete[] trans;
          }
          break;
      }

      gUserManager.DropUser(u);
      break;
    }

    case SIGNAL_LOGON:
    case SIGNAL_LOGOFF:
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
  pango_layout_set_font_aosd(trd.lay, const_cast<char*>(conf->font.c_str()));

  trd.geom.x_offset = conf->marginHorizontal;
  trd.geom.y_offset = conf->marginVertical;

  trd.back.color = const_cast<char*>(conf->backColor.c_str());
  trd.back.opacity = conf->backOpacity;

  trd.shadow.x_offset = conf->shadowOffset;
  trd.shadow.y_offset = conf->shadowOffset;
  trd.shadow.color = const_cast<char*>(conf->shadowColor.c_str());
  trd.shadow.opacity = conf->shadowOpacity;

  trd.fore.opacity = conf->textOpacity;

  int width = aosd_text_get_screen_wrap_width(aosd, &trd);
  if (conf->wrapWidth == 0 || width < 0)
    pango_layout_set_width(trd.lay, -1);
  else
    pango_layout_set_width(trd.lay,
        (width > conf->wrapWidth ? conf->wrapWidth : width) * PANGO_SCALE);
}

bool Iface::filterSignal(LicqSignal* sig, unsigned long ppid)
{
  switch (sig->SubSignal())
  {
    case USER_STATUS:
    case USER_EVENTS:
      break;

    default:
      return true;
  }

  bool isMessage = (sig->SubSignal() == USER_EVENTS && sig->Argument() > 0);

  if (!isMessage && (ppidTimers[ppid] > time(NULL)))
    return true;

  // TODO status check

  if (gUserManager.isOwner(sig->userId()))
  {
    if (isMessage && conf->showMessage != GROUP_TYPE_NONE)
      return false;

    return true;
  }

  Licq::User* u = gUserManager.fetchUser(sig->userId(), LOCK_R);
  if (u == NULL)
    return true;

  bool ign = u->IgnoreList();
  bool on = u->OnlineNotify();

  gUserManager.DropUser(u);

  if (ign)
    return true;

#define GROUP_DISABLED(type) \
  (conf->type == GROUP_TYPE_NONE || \
  (conf->type == GROUP_TYPE_ONLINE_NOTIFY && !on))

  if (sig->SubSignal() == USER_STATUS)
  {
    if (sig->Argument() == 0 && GROUP_DISABLED(statusChange))
      return true;

    if (GROUP_DISABLED(logonLogoff))
      return true;
  }

  if (sig->SubSignal() == USER_EVENTS)
  {
    if (sig->Argument() < 0)
      return true;

    if (sig->Argument() == 0 && GROUP_DISABLED(autoResponse))
      return true;

    if (GROUP_DISABLED(showMessage))
      return true;
  }
#undef GROUP_DISABLED

  return false;
}

void Iface::displayLayout(string& msg, bool control)
{
  trd.fore.color = control ?
    const_cast<char*>(conf->textControlColor.c_str()) :
    const_cast<char*>(conf->textColor.c_str());

  pango_layout_set_text(trd.lay, const_cast<char*>(msg.c_str()), -1);

  if (conf->maxLines != 0 &&
      pango_layout_get_line_count(trd.lay) > conf->maxLines)
  {
    PangoLayoutLine* line =
      pango_layout_get_line_readonly(trd.lay, conf->maxLines);
    pango_layout_set_text(trd.lay, const_cast<char*>(msg.c_str()),
        line->start_index);
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
