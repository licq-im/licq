#include "licq_gtk.h"

#include <gtk/gtk.h>
#include <fstream.h>

/* Global variables for use in other files */
gushort general_options;
const gushort SHOW_IGN 		= 0x0001;
const gushort SHOW_OFFLINE	= 0x0002;
const gushort ENTER_SENDS	= 0x0004;

/* The "Options" selection under the menu in the main window */
void menu_options_create()
{
	GtkWidget *v_box;
	GtkWidget *notebook;
	GtkWidget *table;
	GtkWidget *label;
	GtkWidget *close;
	struct options_window *ow = g_new0(struct options_window, 1);

	/* Make the window */
	ow->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(ow->window), "Licq - Options");

	/* The vertical box, the main window will contain this */
	v_box = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(ow->window), v_box);

	/* The notebook that will contain all the options */
	notebook = gtk_notebook_new();

	/* The table that will be used in all the notebook pages */
	table = gtk_table_new(5, 2, FALSE);

	/* This will be the first tab.. some general options */
	
	/* Show ignored users in the contact list? */
	ow->show_ignored = gtk_check_button_new_with_label("Show ignored users");
	gtk_table_attach(GTK_TABLE(table), ow->show_ignored, 0, 1, 0, 1,
			 GTK_FILL | GTK_EXPAND, GTK_FILL, 3, 3);

	/* Show offline users in the contact list? */
	ow->show_offline = gtk_check_button_new_with_label("Show offline users");
	gtk_table_attach(GTK_TABLE(table), ow->show_offline, 1, 2, 0, 1,
			 GTK_FILL | GTK_EXPAND, GTK_FILL, 3, 3);
			 
	/* Enter key pressed in the convo window send it? */
	ow->enter_sends = gtk_check_button_new_with_label("Enter sends messages");
	gtk_table_attach(GTK_TABLE(table), ow->enter_sends, 0, 1, 1, 2,
			 GTK_FILL | GTK_EXPAND, GTK_FILL, 3, 3);

	/* Set the check buttons */
	set_options(ow);

	/* Put the table in the notebook */
	label = gtk_label_new("General");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), table, label);

	/* Put the notebook in the window */
	gtk_box_pack_start(GTK_BOX(v_box), notebook, FALSE, FALSE, 0);

	/* The close button */
	close = gtk_button_new_with_label("Done");
	gtk_box_pack_start(GTK_BOX(v_box), close, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(close), "clicked",
			   GTK_SIGNAL_FUNC(done_options), (gpointer)ow);
	
	gtk_widget_show_all(ow->window);
}

void set_options(struct options_window *ow)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ow->show_ignored),
				     general_options & SHOW_IGN);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ow->show_offline),
				     general_options & SHOW_OFFLINE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ow->enter_sends),
				     general_options & ENTER_SENDS);
}

void done_options(GtkWidget *widget, gpointer data)
{
	struct options_window *ow = (struct options_window *)data;

	gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ow->show_ignored)) ?
		general_options |= SHOW_IGN : general_options &= ~SHOW_IGN;

	gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ow->show_offline)) ?
		general_options |= SHOW_OFFLINE : general_options &= ~SHOW_OFFLINE;

	gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ow->enter_sends)) ?
		general_options |= ENTER_SENDS : general_options &= ~ENTER_SENDS;

	gtk_widget_destroy(ow->window);

	save_options();
}

void save_options()
{
	const char *filename = g_strdup_printf("%s/licq_jons-gtk-gui.conf",
					       BASE_DIR);
	ofstream file(filename);

	/* Write to the config file */
	file << "general_options " << general_options << endl;

	/* Close the file now that we're done with it */
	file.close();
}

void load_options()
{
	const char *filename = g_strdup_printf("%s/licq_jons-gtk-gui.conf",
					       BASE_DIR);
	ifstream file(filename);

	/* Read in the variables */
	while(file.get() != ' ');

	/* First one is general options */
	gushort temp;
	file >> temp;

	/* Close the file, we're done with it */
	file.close();

	/* Save the options */
	general_options = temp;
}

void set_default_options()
{
	general_options = SHOW_OFFLINE;
	save_options();
}
