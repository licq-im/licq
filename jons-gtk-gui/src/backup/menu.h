#ifndef __MENU_H
#define __MENU_H

#include <gtk/gtk.h>

GtkWidget *menu;

GtkWidget *menu_new_item(GtkWidget *menu, const char *str,
			 GtkSignalFunc s_func);
void menu_create();
void menu_system_quit(GtkWidget *blah, gpointer data);

/* From main.h */

extern GtkWidget *main_window;

/* From main_window.h */

extern GtkWidget *vertical_box;

#endif /* __MENU_H */
