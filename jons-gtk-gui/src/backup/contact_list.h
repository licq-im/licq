#ifndef __CONTACT_LIST_H
#define __CONTACT_LIST_H

#include "icqd.h"
#include "user.h"

#include <gtk/gtk.h>

GtkWidget *contact_list_new(gint height, gint width);
GtkWidget *contact_list_refresh();

/* From main.h */
extern CICQDaemon *icq_daemon;

/* From main_window.h */
extern GtkWidget *contact_list;

#endif /* __CONTACT_LIST_H */
