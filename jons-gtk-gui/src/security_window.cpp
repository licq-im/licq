/*
 *
 * Licq GTK Gui Plugin		Copyright (C) 2000, Jon Keating <jon@licq.org>
 *
 * Licensed under the GNU GPL
 *
 */

#include "licq_gtk.h"

#include "licq_events.h"
#include "licq_icqd.h"
#include "licq_user.h"

#include <gtk/gtk.h>

/* Create a global structure, easy way to ensure only one security window is open */
struct security_window
{
	GtkWidget *window;
	GtkWidget *check_auth;
	GtkWidget *check_web;
	GtkWidget *check_hideip;
	GtkTooltips *tooltips;
	CICQEventTag *e_tag;
} *sw;

void menu_security_window(GtkWidget *widget, gpointer data)
{
	GtkWidget *table;
	GtkWidget *h_box;
	GtkWidget *ok;
	GtkWidget *cancel;

	/* If the window is open, we don't need to be in here */
	if(sw)
	{
		gdk_window_raise(sw->window->window);
		return;
	}

	sw = (struct security_window *)g_new0(struct security_window, 1);

	/* Setup the tooltips for this window */
	sw->tooltips = gtk_tooltips_new();

	/* Create the window */
	sw->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(sw->window), "Licq - Security Options");
	gtk_window_set_position(GTK_WINDOW(sw->window), GTK_WIN_POS_CENTER);

	/* Delete signal for the window */
	gtk_signal_connect(GTK_OBJECT(sw->window), "delete_event",
			   GTK_SIGNAL_FUNC(close_security_window), NULL);

	/* Create the table */
	table = gtk_table_new(4, 2, FALSE);
	gtk_container_add(GTK_CONTAINER(sw->window), table);

	/* The first option */
	sw->check_auth = gtk_check_button_new_with_label("Authorization Required");
	gtk_table_attach(GTK_TABLE(table), sw->check_auth, 0, 1, 0, 1,
			 GTK_FILL | GTK_EXPAND, GTK_FILL, 3, 3);
	gtk_tooltips_set_tip(GTK_TOOLTIPS(sw->tooltips), sw->check_auth,
			     "Determines whether regular ICQ clients require your authorization to add you to their contact list.", NULL);

	/* The second option */
	sw->check_web = gtk_check_button_new_with_label("Web Presence");
	gtk_table_attach(GTK_TABLE(table), sw->check_web, 0, 1, 1, 2,
			 GTK_FILL | GTK_EXPAND, GTK_FILL, 3, 3);
	gtk_tooltips_set_tip(GTK_TOOLTIPS(sw->tooltips), sw->check_web,
			     "Web Presence allows users to see if you are online through your web indicator.", NULL);

	/* The third option */
	sw->check_hideip = gtk_check_button_new_with_label("Hide IP");
	gtk_table_attach(GTK_TABLE(table), sw->check_hideip, 0, 1, 2, 3,
			 GTK_FILL | GTK_EXPAND, GTK_FILL, 3, 3);
	gtk_tooltips_set_tip(GTK_TOOLTIPS(sw->tooltips), sw->check_hideip,
			     "Hiding ip is a minor prevention for regular ICQ clients to hide your ip from other users.", NULL);
	
	/* Set the check boxes accordingly */
	ICQOwner *owner = gUserManager.FetchOwner(LOCK_R);
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(sw->check_auth),
				    owner->GetAuthorization());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(sw->check_web),
				    owner->WebAware());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(sw->check_hideip),
				    owner->HideIp());

	gUserManager.DropOwner();

	/* Make the buttons and put them in an hbox */
	h_box = gtk_hbox_new(TRUE, 5);

	ok = gtk_button_new_with_label("OK");
	gtk_box_pack_start(GTK_BOX(h_box), ok, TRUE, TRUE, 0);
	gtk_signal_connect(GTK_OBJECT(ok), "clicked",
			   GTK_SIGNAL_FUNC(ok_security_window), NULL);

	cancel = gtk_button_new_with_label("Cancel");
	gtk_box_pack_start(GTK_BOX(h_box), cancel, TRUE, TRUE, 0);
	gtk_signal_connect(GTK_OBJECT(cancel), "clicked",
			   GTK_SIGNAL_FUNC(close_security_window), NULL);

	gtk_table_attach(GTK_TABLE(table), h_box, 0, 2, 3, 4,
			 GTK_FILL | GTK_EXPAND, GTK_FILL, 3, 3);

	gtk_widget_show_all(sw->window);
}

void ok_security_window(GtkWidget *widget, gpointer data)
{
	sw->e_tag =
	   icq_daemon->icqSetSecurityInfo(
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(sw->check_auth)),
	      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(sw->check_hideip)),
	        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(sw->check_web)));
}

void close_security_window(GtkWidget *widget, gpointer data)
{
	/* Distroy the window and all child widgets */
	gtk_widget_destroy(sw->window);

	/* Free the memory allocated */
	g_free(sw);
	sw = NULL;
}
