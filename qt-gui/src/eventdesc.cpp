#include <qobject.h>
#include <qstring.h>
#include <qapplication.h>

#include "eventdesc.h"
#include "licq_message.h"
#include "licq_icq.h"


static const int MAX_EVENT = 26;

static const char *szEventTypes[27] =
{ QT_TRANSLATE_NOOP("Event","Plugin Event"),
  QT_TRANSLATE_NOOP("Event","Message"),
  QT_TRANSLATE_NOOP("Event","Chat Request"),
  QT_TRANSLATE_NOOP("Event","File Transfer"),
  QT_TRANSLATE_NOOP("Event","URL"),
  QT_TRANSLATE_NOOP("Event",""),
  QT_TRANSLATE_NOOP("Event","Authorization Request"),
  QT_TRANSLATE_NOOP("Event","Authorization Refused"),
  QT_TRANSLATE_NOOP("Event","Authorization Granted"),
  QT_TRANSLATE_NOOP("Event",""),
  QT_TRANSLATE_NOOP("Event",""),
  QT_TRANSLATE_NOOP("Event",""),
  QT_TRANSLATE_NOOP("Event","Added to Contact List"),
  QT_TRANSLATE_NOOP("Event","Web Panel"),
  QT_TRANSLATE_NOOP("Event","Email Pager"),
  QT_TRANSLATE_NOOP("Event",""),
  QT_TRANSLATE_NOOP("Event",""),
  QT_TRANSLATE_NOOP("Event",""),
  QT_TRANSLATE_NOOP("Event",""),
  QT_TRANSLATE_NOOP("Event","Contact List"),
  QT_TRANSLATE_NOOP("Event",""),
  QT_TRANSLATE_NOOP("Event",""),
  QT_TRANSLATE_NOOP("Event",""),
  QT_TRANSLATE_NOOP("Event",""),
  QT_TRANSLATE_NOOP("Event",""),
  QT_TRANSLATE_NOOP("Event",""),
  QT_TRANSLATE_NOOP("Event","SMS")
};

QString EventDescription(CUserEvent *e)
{
  QString desc;
  if (e->SubCommand() > MAX_EVENT ||
      szEventTypes[e->SubCommand()][0] == '\0')
    desc = qApp->translate("Event", "Unknown Event");
  else
  {
    desc = qApp->translate("Event", szEventTypes[e->SubCommand()]);
    if (e->IsCancelled())
      desc += ' ' + qApp->translate("Event", "(cancelled)");
  }
  return desc;
}

