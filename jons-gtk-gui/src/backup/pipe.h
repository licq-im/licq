#ifndef __PIPE_H
#define __PIPE_H

#include "icqd.h"
#include "icqevent.h"
#include "user.h"
#include "icq-defines.h"

#include <gtk/gtk.h>

CICQSignal *s;
ICQEvent *e;

void pipe_callback(gpointer data, gint _Pipe, GdkInputCondition condition);
void pipe_signal(CICQSignal *sig);
void pipe_event(ICQEvent *eve);

/* From main.h
 * -----------
 */
extern gint _Pipe;
extern CICQDaemon *icq_daemon;

/* From contact_list.h
 * -------------------
 */
extern void contact_list_refresh();

#endif /* __PIPE_H */

