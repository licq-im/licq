#include "eventdesc.h"
#include "message.h"
#include "icq-defines.h"


static const int MAX_EVENT = 26;

static const char *szEventTypes[27] =
{ "Saved Event",
  "Message",
  "Chat Request",
  "File Transfer",
  "URL",
  "",
  "Authorization Request",
  "",
  "",
  "",
  "",
  "",
  "Added to Contact List",
  "Web Panel",
  "Email Pager",
  "",
  "",
  "",
  "",
  "Contact List",
  "",
  "",
  "",
  "",
  "",
  "",
  "User Info"
};


const char *EventDescription(CUserEvent *e)
{
  static char desc[128];

  if (e->SubCommand() > MAX_EVENT ||
      szEventTypes[e->SubCommand()][0] == '\0')
    strcpy(desc, "Unknown Event");
  else
  {
    strcpy(desc, szEventTypes[e->SubCommand()]);
    if (e->Command() == ICQ_CMDxTCP_CANCEL)
      strcat(desc, " Cancelled");
  }
  return desc;
}

