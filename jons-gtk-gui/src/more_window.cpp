#include "licq_gtk.h"

#include "icqd.h"
#include "user.h"

#include <gtk/gtk.h>

void list_more_window(GtkWidget *widget, ICQUser *u)
{
	GtkWidget *fix_radios;
	GtkWidget *h_box;
	GtkWidget *v_box;
	GtkWidget *ok;
	GtkWidget *cancel;

	const gchar *title = g_strdup_printf("More Options for %s", u->GetAlias());
	struct more_window *mw = g_new0(struct more_window, 1);

	/* Set the user in the struct */
	mw->user = u;

	/* Make the window */
	mw->window = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title(GTK_WINDOW(mw->window), title);

	/* Make the boxes */
	h_box = gtk_hbox_new(FALSE, 5);	
	v_box = gtk_vbox_new(FALSE, 5);

	/* The online notify */
	mw->notify = gtk_check_button_new_with_label("Online Notify");

	/* Set the state of the online notify button */
	if(mw->user->OnlineNotify())
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mw->notify), TRUE);

	/* The ignore list */
	mw->ignore = gtk_check_button_new_with_label("Ignore List");

	/* Set the state of the ignore button */
	if(mw->user->IgnoreList())
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mw->ignore), TRUE);

	/* Pack them */
	gtk_box_pack_start(GTK_BOX(h_box), mw->notify, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), mw->ignore, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 5);

	/* Make new h_box for the other options */
	h_box = gtk_hbox_new(FALSE, 5);

	fix_radios = gtk_radio_button_new_with_label(NULL, "None");

	/* The visible list */
	mw->visible = gtk_radio_button_new_with_label_from_widget(
		GTK_RADIO_BUTTON(fix_radios), "Visible List");

	/* Set the state of the visible button */
	if(mw->user->VisibleList())
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mw->visible), TRUE);

	/* The invisible list */
	mw->invisible = gtk_radio_button_new_with_label_from_widget(
		GTK_RADIO_BUTTON(fix_radios), "Invisible List");

	/* Set the state of the invisible button */
	if(mw->user->InvisibleList())
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mw->invisible), TRUE);

	/* Pack them */
	gtk_box_pack_start(GTK_BOX(h_box), fix_radios, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), mw->visible, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), mw->invisible, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 5);

	h_box = gtk_hbox_new(TRUE, 15);

	ok = gtk_button_new_with_label("OK");
	cancel = gtk_button_new_with_label("Cancel");

	gtk_box_pack_start(GTK_BOX(h_box), ok, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), cancel, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 5);

	gtk_signal_connect(GTK_OBJECT(cancel), "clicked",
			   GTK_SIGNAL_FUNC(dialog_close), mw->window);
	gtk_signal_connect(GTK_OBJECT(mw->window), "destroy",
			   GTK_SIGNAL_FUNC(dialog_close), mw->window);
	gtk_signal_connect(GTK_OBJECT(ok), "clicked",
			   GTK_SIGNAL_FUNC(more_ok_callback), mw);

	/* Add the v_box to the window */
	gtk_container_add(GTK_CONTAINER(mw->window), v_box);

	/* Show the widgets */
	gtk_widget_show_all(mw->window);
}

void more_ok_callback(GtkWidget *widget, struct more_window *mw)
{
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->notify)))
		mw->user->SetOnlineNotify(TRUE);
	else
		mw->user->SetOnlineNotify(FALSE);

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->ignore)))
		mw->user->SetIgnoreList(TRUE);
	else
		mw->user->SetIgnoreList(FALSE);

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->visible)))
		mw->user->SetVisibleList(TRUE);
	else
		mw->user->SetVisibleList(FALSE);

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->invisible)))
		mw->user->SetInvisibleList(TRUE);
	else
		mw->user->SetInvisibleList(FALSE);

	dialog_close(NULL, mw->window);
}	
