/*
 * Licq GTK GUI Plugin
 *
 * Copyright (C) 2000, Jon Keating <jon@licq.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */		 

#include "licq_gtk.h"
#include "licq_file.h"

#include <gtk/gtk.h>
#include <fstream.h>

// Global variables for use in other files
bool show_offline_users;
bool show_ignored_users;
bool enter_sends;

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
	gtk_widget_show(ow->window);
	gtk_window_set_title(GTK_WINDOW(ow->window), "Licq - Options");

	/* The vertical box, the main window will contain this */
	v_box = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(ow->window), v_box);

	/* The notebook that will contain all the options */
	notebook = gtk_notebook_new();

	/* The table that will be used in all the notebook pages */
	table = gtk_table_new(5, 2, FALSE);

/*********************** FIRST TAB *********************/ 
	
	/* Show ignored users in the contact list? */
	ow->show_ignored = gtk_check_button_new_with_label("Show ignored users");
	gtk_table_attach(GTK_TABLE(table), ow->show_ignored, 0, 1, 0, 1,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GTK_FILL, 3, 3);

	/* Show offline users in the contact list? */
	ow->show_offline = gtk_check_button_new_with_label("Show offline users");
	gtk_table_attach(GTK_TABLE(table), ow->show_offline, 1, 2, 0, 1,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GTK_FILL, 3, 3);
 
	/* Enter key pressed in the convo window send it? */
	ow->enter_sends = gtk_check_button_new_with_label("Enter sends messages");
	gtk_table_attach(GTK_TABLE(table), ow->enter_sends, 0, 1, 1, 2,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GTK_FILL, 3, 3);

	/* Set the check buttons */
	set_options(ow);

	/* Put the table in the notebook */
	label = gtk_label_new("General");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), table, label);

/**************** The second tab: Contact List *******************/
	
	/* Recreate the table */
	table = gtk_table_new(5, 2, FALSE);

	/* The table for the color selection in the "Contact List Colors" frame */
	GtkWidget *clr_table = gtk_table_new(3, 3, FALSE);
	
	/* Colors frame and attach it to the table and also a table for inside **
	 * the frame */
	GtkWidget *color_frame = gtk_frame_new("Contact List Colors");
	gtk_table_attach(GTK_TABLE(table), color_frame, 0, 1, 0, 1,
			 GTK_FILL, GTK_FILL, 3, 3);
	gtk_container_add(GTK_CONTAINER(color_frame), clr_table);

	/* Online color label */
	label = gtk_label_new("Online Color");
	gtk_table_attach(GTK_TABLE(clr_table), label, 0, 1, 0, 1,
			 GTK_FILL, GTK_FILL, 3, 3);
	
	/* Online color browse button */
	int *chg_on_color = new int;
	*chg_on_color = 1;
	
	GtkWidget *online_browse = gtk_button_new_with_label("Browse");
	gtk_signal_connect(GTK_OBJECT(online_browse), "clicked",
				  GTK_SIGNAL_FUNC(show_on_color_dlg),
				  (gpointer)chg_on_color);
	gtk_table_attach(GTK_TABLE(clr_table), online_browse, 2, 3, 0, 1,
			 GTK_FILL, GTK_FILL, 3, 3);

	/* Offline color label */
	label = gtk_label_new("Offline Color");
	gtk_table_attach(GTK_TABLE(clr_table), label, 0, 1, 1, 2,
			 GTK_FILL, GTK_FILL, 3, 3);

	/* Offline color browse button */
	int *chg_off_color = new int;
	*chg_off_color = 2;
	
	GtkWidget *offline_browse = gtk_button_new_with_label("Browse");
	gtk_signal_connect(GTK_OBJECT(offline_browse), "clicked",
			   GTK_SIGNAL_FUNC(show_on_color_dlg),
			   (gpointer)chg_off_color);
	gtk_table_attach(GTK_TABLE(clr_table), offline_browse, 2, 3, 1, 2,
			 GTK_FILL, GTK_FILL, 3, 3);

	label = gtk_label_new("Contact List");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), table, label);

/********************* END OF ALL THE TABS ************************/

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
				     show_ignored_users);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ow->show_offline),
				     show_offline_users);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ow->enter_sends),
				     enter_sends);
}

void done_options(GtkWidget *widget, gpointer data)
{
	// Refresh the contact list
	contact_list_refresh();

	struct options_window *ow = (struct options_window *)data;
	show_offline_users = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(ow->show_offline));
	show_ignored_users = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(ow->show_ignored));
	enter_sends = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ow->enter_sends));
	
	gtk_widget_destroy(ow->window);

	// Save the daemon options
	icq_daemon->SaveConf();

	// Save our options
	char filename[MAX_FILENAME_LEN];
	sprintf(filename, "%s/licq_jons-gtk-gui.conf", BASE_DIR);
	CIniFile licqConf(INI_FxERROR | INI_FxALLOWxCREATE);
	if(!licqConf.LoadFile(filename))
		return;

	licqConf.SetSection("appearance");
	licqConf.WriteNum("ColorOnline_Red", online_color->red);
	licqConf.WriteNum("ColorOnline_Green", online_color->green);
	licqConf.WriteNum("ColorOnline_Blue", online_color->blue);
	licqConf.WriteNum("ColorOnline_Pixel", online_color->pixel);
	licqConf.WriteNum("ColorOffline_Red", offline_color->red);
	licqConf.WriteNum("ColorOffline_Green", offline_color->green);
	licqConf.WriteNum("ColorOffline_Blue", offline_color->blue);
	licqConf.WriteNum("ColorOffline_Pixel", offline_color->pixel);
	licqConf.WriteNum("ColorAway_Red", away_color->red);
	licqConf.WriteNum("ColorAway_Green", away_color->green);
	licqConf.WriteNum("ColorAway_Blue", away_color->blue);
	licqConf.WriteNum("ColorAway_Pixel", away_color->pixel);
	licqConf.WriteBool("ShowOfflineUsers", show_offline_users);
	licqConf.WriteBool("ShowIgnoredUsres", show_ignored_users);
	licqConf.WriteBool("EnterSends", enter_sends);

	licqConf.FlushFile();
	licqConf.CloseFile();
}

// load_options() is only called if the file exists
void load_options()
{
	online_color = new GdkColor;
	offline_color = new GdkColor;
	away_color = new GdkColor;

	CIniFile licqConf;
	licqConf.LoadFile(g_strdup_printf("%s/licq_jons-gtk-gui.conf",
				          BASE_DIR));
	licqConf.SetSection("appearance");

	gLog.Info("%sLoading Jon's GTK+ GUI configuration", L_INITxSTR);

	// Online color
	licqConf.ReadNum("ColorOnline_Red", online_color->red, 8979);
	licqConf.ReadNum("ColorOnline_Green", online_color->green, 27457);
	licqConf.ReadNum("ColorOnline_Blue", online_color->blue, 63052);
	licqConf.ReadNum("ColorOnline_Pixel", online_color->pixel, 0);
	
	// Offline color
	licqConf.ReadNum("ColorOffline_Red", offline_color->red, 59080);
	licqConf.ReadNum("ColorOffline_Green", offline_color->green, 0);
	licqConf.ReadNum("ColorOffline_Blue", offline_color->blue, 1660);
	licqConf.ReadNum("ColorOffline_Pixel", offline_color->pixel, 0);

	// Away color
	licqConf.ReadNum("ColorAway_Red", away_color->red, 0);
	licqConf.ReadNum("ColorAway_Green", away_color->green, 30000);
	licqConf.ReadNum("ColorAway_Blue", away_color->blue, 0);
	licqConf.ReadNum("ColorAway_Pixel", away_color->pixel, 0);

	// General options
	licqConf.ReadBool("ShowOfflineUsers", show_offline_users, true);
	licqConf.ReadBool("ShowIgnoredUsers", show_ignored_users, false);
	licqConf.ReadBool("EnterSends", enter_sends, true);
}

void show_on_color_dlg(GtkWidget *widget, gpointer data)
{
	gdouble color[4];
	int *change = (int *)data;
	
	switch(*change)
	{
	case 1:
		color[0] = (gdouble)online_color->red / 65535;
		color[1] = (gdouble)online_color->green / 65535;
		color[2] = (gdouble)online_color->blue / 65535;
		color[3] = (gdouble)online_color->pixel / 65535;
		break;
	case 2:
		color[0] = (gdouble)offline_color->red / 65535;
		color[1] = (gdouble)offline_color->green / 65535;
		color[2] = (gdouble)offline_color->blue / 65535;
		color[3] = (gdouble)offline_color->pixel / 65535;
		break;
	}

	/* The select color dialog */
	GtkWidget *color_dialog =
		gtk_color_selection_dialog_new("Licq - Choose Color");
	/* Set the int that represents the color to change */
	gtk_object_set_user_data(GTK_OBJECT(color_dialog), (gpointer)change);

	/* Set the default color to the current online color */
	gtk_color_selection_set_color(
		GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(color_dialog)->colorsel),
				      color);

	/* Handle the ok and cancel button on the color dialog */
	gtk_signal_connect(
		GTK_OBJECT(GTK_COLOR_SELECTION_DIALOG(color_dialog)->ok_button),
		"clicked", GTK_SIGNAL_FUNC(color_dlg_ok), (gpointer)color_dialog);
	gtk_signal_connect(
		GTK_OBJECT(GTK_COLOR_SELECTION_DIALOG(color_dialog)->cancel_button),
		"clicked", GTK_SIGNAL_FUNC(color_dlg_cancel),
		(gpointer)color_dialog);

	/* Show the color dialog */
	gtk_widget_show_all(color_dialog);

}

void color_dlg_ok(GtkWidget *widget, gpointer data)
{
	GtkWidget *color_dialog = (GtkWidget *)data;

	/* What are we changing? */
	gint *change = (gint *)gtk_object_get_user_data(GTK_OBJECT(color_dialog));

	/* To store the selected color */
	double new_color[4];

	/* Now get the color that was selected */
	gtk_color_selection_get_color(
		GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(color_dialog)->colorsel),
		new_color);

	switch(*change)
	{
	case 1:
		online_color->red = (guint16)(new_color[0] * 65535.0);
		online_color->green = (guint16)(new_color[1] * 65535.0);
		online_color->blue = (guint16)(new_color[2] * 65535.0);
		online_color->pixel = (gulong)new_color[3];
		break;
	case 2:
		offline_color->red = (guint16)(new_color[0] * 65535.0);
		offline_color->green = (guint16)(new_color[1] * 65535.0);
		offline_color->blue = (guint16)(new_color[2] * 65535.0);
		offline_color->pixel = (gulong)new_color[3];
 		break;
	}

	gtk_widget_destroy(color_dialog);
	delete change;
}

void color_dlg_cancel(GtkWidget *widget, gpointer data)
{
	GtkWidget *color_dialog = (GtkWidget *)data;
	gint *change = (gint *)gtk_object_get_user_data(GTK_OBJECT(color_dialog));
	gtk_widget_destroy(color_dialog);
	delete change;
}
