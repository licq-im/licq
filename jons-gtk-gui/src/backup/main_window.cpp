#include "main_window.h"

#include <gtk/gtk.h>

void main_window_delete_event(GtkWidget *mainwindow, gpointer data)
{
	gtk_widget_hide(mainwindow);
}

GtkWidget* main_window_new(const gchar* window_title,
			   gint height,
			   gint width)
{
	GtkWidget *scroll_bar;

	/* Create the main window */
	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	/* Set the title */
	gtk_window_set_title(GTK_WINDOW(main_window), window_title);
	
	/* Make the window fully resizable */
	gtk_window_set_policy(GTK_WINDOW(main_window), TRUE, TRUE, FALSE);
	
	/* Set the def size which is passed in from LP_Main() in main.cpp */
	gtk_window_set_default_size(GTK_WINDOW(main_window), width, height);

	/* Call main_window_delete_event when the delete_event is called */
	gtk_signal_connect(GTK_OBJECT(main_window), "delete_event",
			   GTK_SIGNAL_FUNC(main_window_delete_event), NULL);

	/* Add the vertical box in */
	vertical_box = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(main_window), vertical_box);
	gtk_widget_show(vertical_box);

	/* Add in the menu */
	menu_create();

	/* Add a scroll bar for the contact list */
	scroll_bar = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_bar),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_widget_set_usize(scroll_bar, width - 77, height - 31);

	/* Add in the contact list */
	contact_list = contact_list_new(height, width - 37);
	
	gtk_container_add(GTK_CONTAINER(scroll_bar), contact_list);
	gtk_box_pack_start(GTK_BOX(vertical_box), scroll_bar, 
			   FALSE, FALSE, 0); 
	contact_list_refresh();

	/* Show the widgets */
	gtk_widget_show(scroll_bar);
	gtk_widget_show(contact_list);

	return main_window;
}

void main_window_show()
{
	gtk_widget_show(main_window);
}
