#include "menu.h"

#include <gtk/gtk.h>

GtkWidget *menu_new_item(GtkWidget *_menu, const char *str, GtkSignalFunc s_func)
{
	GtkWidget *menu_item;
	menu_item = gtk_menu_item_new_with_label(str);

	/* If menu is passed into this function, append menu_item to menu */
	if(_menu)
	{
		gtk_menu_append(GTK_MENU(_menu), menu_item);
	}

	gtk_widget_show(menu_item);

	/* If s_func is passed into this function, connect it to the object */
	if(s_func)
	{
		/* Cast s_func WHEN it is passed in, not here */
		gtk_signal_connect(GTK_OBJECT(menu_item),
				   "activate",
				   s_func,
				   NULL);
	}

	return menu_item;
}

void menu_create()
{
	GtkWidget *quit_item;
	GtkWidget *system_item;
	GtkWidget *menu_bar;

	/* This is the main menu...  do not show it */
	menu = gtk_menu_new();

	quit_item = menu_new_item(menu, "Exit",
				  GTK_SIGNAL_FUNC(menu_system_quit));


	/* The root menu text for the System menu */
	system_item = menu_new_item(NULL, "System", NULL);
	gtk_widget_show(system_item);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(system_item), menu);

	/* The menu bar to place the mens */
	menu_bar = gtk_menu_bar_new();
	gtk_box_pack_start(GTK_BOX(vertical_box), menu_bar, FALSE, FALSE, 0);
	gtk_widget_show(menu_bar);

	/* Append the "System" to the menu bar */
	gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), system_item);
}

void menu_system_quit(GtkWidget *blah, gpointer data)
{
	gtk_main_quit();
} 
