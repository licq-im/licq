#ifndef EVENTDESC_H
#define EVENTDESC_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qstring.h>

class CUserEvent;

QString EventDescription(CUserEvent *e);

#endif
