#include <qobject.h>
#include <qstring.h>
#include <qapplication.h>
#include "eventdesc.h"
#include "message.h"
#include "icq-defines.h"


static const int MAX_EVENT = 26;

static const char *szEventTypes[27] =
{ QT_TR_NOOP("Saved Event"),
  QT_TR_NOOP("Message"),
  QT_TR_NOOP("Chat Request"),
  QT_TR_NOOP("File Transfer"),
  QT_TR_NOOP("URL"),
  QT_TR_NOOP(""),
  QT_TR_NOOP("Authorization Request"),
  QT_TR_NOOP(""),
  QT_TR_NOOP(""),
  QT_TR_NOOP(""),
  QT_TR_NOOP(""),
  QT_TR_NOOP(""),
  QT_TR_NOOP("Added to Contact List"),
  QT_TR_NOOP("Web Panel"),
  QT_TR_NOOP("Email Pager"),
  QT_TR_NOOP(""),
  QT_TR_NOOP(""),
  QT_TR_NOOP(""),
  QT_TR_NOOP(""),
  QT_TR_NOOP("Contact List"),
  QT_TR_NOOP(""),
  QT_TR_NOOP(""),
  QT_TR_NOOP(""),
  QT_TR_NOOP(""),
  QT_TR_NOOP(""),
  QT_TR_NOOP(""),
  QT_TR_NOOP("User Info")
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
    if (e->Command() == ICQ_CMDxTCP_CANCEL)
      desc += qApp->translate("Event", " Cancelled");
  }
  return desc;
}

