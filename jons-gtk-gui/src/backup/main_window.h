#ifndef __MAIN_WINDOW_H
#define __MAIN_WINDOW_H

#include "icqd.h"

#include <gtk/gtk.h>

GtkWidget *vertical_box;
GtkWidget *contact_list;

GtkWidget *main_window_new(const gchar *title, gint height, gint width);
void main_window_show();

/* From main.h
 * -----------
 */
extern GtkWidget *main_window;


/* From menu.h
 * -----------
 */
extern GtkWidget *menu;
extern void menu_create();

/* From contact_list.h
 * -------------------
 */
extern GtkWidget *contact_list_new(gint height, gint width);
extern GtkWidget *contact_list_refresh();

#endif /* __MAIN_WINDOW_H */
