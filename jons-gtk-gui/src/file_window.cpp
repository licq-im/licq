#include "licq_gtk.h"

#include "icqd.h"
#include "message.h"
#include "user.h"

void file_accept_window(ICQUser *user, CEventFile *e)
{
	GtkWidget *accept;
	GtkWidget *refuse;
	GtkWidget *v_box;
	GtkWidget *h_box;
	GtkWidget *label;
	struct file_accept *fa = g_new0(struct file_accept, 1);
	const gchar *title = g_strdup_printf("File From %s", user->GetAlias());

	fa->user = user;
	fa->e = e;

	/* Make the window */
	fa->window = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title(GTK_WINDOW(fa->window), title);
	gtk_window_set_position(GTK_WINDOW(fa->window), GTK_WIN_POS_CENTER);

	/* Make the boxes */
	v_box = gtk_vbox_new(FALSE, 5);
	h_box = gtk_hbox_new(FALSE, 5);

	/* The label */
	const gchar *text = g_strdup_printf("File: %s (%ld bytes)",
					    e->Filename(), e->FileSize());
	label = gtk_label_new(text);

	/* Pack the label */
	gtk_box_pack_start(GTK_BOX(v_box), label, FALSE, FALSE, 10);

	/* The buttons */
	accept = gtk_button_new_with_label("Accept");
	refuse = gtk_button_new_with_label("Refuse");

	/* Pack them */
	gtk_box_pack_start(GTK_BOX(h_box), accept, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(h_box), refuse, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 10);

	/* Connect the signals */
	gtk_signal_connect(GTK_OBJECT(fa->window), "destroy",
			   GTK_SIGNAL_FUNC(dialog_close), fa->window);
	gtk_signal_connect(GTK_OBJECT(refuse), "clicked",
			   GTK_SIGNAL_FUNC(refuse_file), fa);
	gtk_signal_connect(GTK_OBJECT(accept), "clicked",
			   GTK_SIGNAL_FUNC(accept_file), fa);

	gtk_container_add(GTK_CONTAINER(fa->window), v_box);
	gtk_widget_show_all(fa->window);
}

void accept_file(GtkWidget *widget, struct file_accept *fa)
{
}

void refuse_file(GtkWidget *widget, struct file_accept *fa)
{
	/* Let's get rid of the file accept window */
	dialog_close(NULL, fa->window);

	/*
	 * Make a new window to refuse the file.  We have the file_accept 
         * structure with everything we need.  So don't make a new structure
	 * for this part. 
	 */

	GtkWidget *v_box;
	GtkWidget *scroll;
	GtkWidget *ok;
	const gchar *title =
		g_strdup_printf("File refusal to %s", fa->user->GetAlias());

	/* Make the window */
	fa->window2 = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title(GTK_WINDOW(fa->window2), title);
	gtk_window_set_position(GTK_WINDOW(fa->window2), GTK_WIN_POS_CENTER);

	/* The box */
	v_box = gtk_vbox_new(FALSE, 0);

	/* The text box in the scrolled window */
	scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
				       GTK_POLICY_NEVER,
				       GTK_POLICY_AUTOMATIC);

	fa->textbox = gtk_text_new(NULL, NULL);
	gtk_text_set_editable(GTK_TEXT(fa->textbox), TRUE);
	gtk_text_set_word_wrap(GTK_TEXT(fa->textbox), TRUE);
	gtk_text_set_line_wrap(GTK_TEXT(fa->textbox), TRUE);

	gtk_container_add(GTK_CONTAINER(scroll), fa->textbox);
	gtk_widget_set_usize(scroll, 150, 150);

	/* Pack it */
	gtk_box_pack_start(GTK_BOX(v_box), scroll, FALSE, FALSE, 5);
	
	/* The button */
	ok = gtk_button_new_with_label("OK");

	/* Pack it */
	gtk_box_pack_start(GTK_BOX(v_box), ok, TRUE, TRUE, 5);

	/* Connect the signals */
	gtk_signal_connect(GTK_OBJECT(fa->window2), "destroy",
			   GTK_SIGNAL_FUNC(dialog_close), fa->window2);
	gtk_signal_connect(GTK_OBJECT(ok), "clicked",
			   GTK_SIGNAL_FUNC(refuse_ok), fa);


	/* Show the window now */
	gtk_container_add(GTK_CONTAINER(fa->window2), v_box);
	gtk_widget_show_all(fa->window2);
}

void refuse_ok(GtkWidget *widget, struct file_accept *fa)
{
	icq_daemon->icqFileTransferRefuse(fa->user->Uin(),
		gtk_editable_get_chars(GTK_EDITABLE(fa->textbox), 0, -1),
		fa->e->Sequence());

	dialog_close(NULL, fa->window2);
}
